#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <termios.h>
#include <fcntl.h> 
#include <pthread.h>
#include <sys/stat.h> 
#include <sys/types.h> 
#include <sys/time.h>
#include <dirent.h> 
#include "TG.h"
#include"TGFingerVeinApi.h"

typedef struct{
	int id;
	char name[256];
	char feature[SIZE_FEATURE];
}user_data;

#define WRITE_MATCH_COUNT (100000)

/*****************************************************************  
** @brief       read file
** @author      xuhc
** @param1      <out>  <unsigned char *>           pointor of buf
** @param2      <in> <int>         length of buf
** @param3      <in> <int>         file path
** @exception
** @return     0 :successed
**             -1:fail
** @note       
** @see 
** @date       2017/09/19
******************************************************************/
static int read_data_hex(unsigned char *buf,int length,char *path)
{
    FILE *fp;
    fp = fopen(path,"rb");
    if(NULL == fp)
    {
        printf("file open Fail!\n");
		return -1;
    }
	fread(buf,sizeof(unsigned char),length,fp);
    fclose(fp);
	return 0;
}
/*****************************************************************  
** @brief      write file
** @author     xuhc
** @param1      <in>  <unsigned char *>         pointor of buf
** @param2     <in> <int>         length of buf
** @param3      <in> <char *>         file path
** @exception
** @return     0 :successed
**             -1:fail
** @note       
** @see 
** @date       2017/09/19
******************************************************************/
static int write_data_hex(unsigned char * array,int length,char *path)
{
    FILE *fp;	
    fp = fopen(path,"wb+");	
    if(NULL == fp)	
    {       
        printf("write file open Fail!\n");  
        fclose(fp);
        return -1;  
    }   
    fwrite(array,sizeof(unsigned char),length,fp);
    fclose(fp);
	return 0;
}



char **Make2DArray_uint8(int row,int col){
	char **a;
	int i;
	
	a=(char **)calloc(row,sizeof(char *));
	
	for(i=0;i<row;i++)
	{
		a[i]=(char *)calloc(col,sizeof(char));
	}
	
	return a;
}

void Clean2DArray_uint8(char **a,int row,int col){
	int i;
	for(i=0;i<row;i++)
		memset(a[i],0,col);
}

void Free2DArray_uint8(char **a,int row){
	
	int i;
	for(i=0;i<row;i++)
	{
		free(a[i]);
		a[i] = NULL;
	}
	free(a);
	a = NULL;
	//printf("part");
}


/*****************************************************************  
** @brief       get config param
** @author      xuhc
** @param1      <out>  <char *>      config buf
** @param2      <in>   <int>         the line_num of config file ,from 1
** @exception
** @return     0 :successed
**             -1:fail
** @note       
** @see 
** @date       2017/10/11
******************************************************************/
static int get_config(char *dest,int line_num)
{
	FILE *fp=fopen(CONFIG_PATH, "r+");
	int i;
	int length;
	char str[256]; 
	char *pstr = NULL;
	line_num --;
	if (fp==NULL )
		return -1;
	for( i=0;i<line_num;i++ ) 
	{
		fgets(str,sizeof(str),fp);
	}
	fgets(str,sizeof(str),fp);
	pstr = strstr(str,"=");
	pstr++;
	length = strlen(pstr);
	*(pstr+length-1) = 0;
//	printf("pstr=%s.\n",pstr);
	strcpy(dest,pstr);
	fclose(fp);
	return 0;
}

static int get_match_result(char **array,int index,char *name_1,char *name_2,float score)     //参数：文件全名，写入内容  
{  
	int cut_pos;
	cut_pos = strlen(name_1);
	name_1[cut_pos-4] = '\0';
	strcat(name_1,".bmp");
	cut_pos = strlen(name_2);
	name_2[cut_pos-4] = '\0';
	strcat(name_2,".bmp");
	sprintf(*(array+index),"%s %s %.2f\n",name_1,name_2,score);                 //格式化写入文件（追加至文件末尾）  
	return 0;
}

static int write_match_result(char *path,char **array,int index)     //参数：文件全名，写入内容  
{  
	FILE *fp;   	                                //定义文件指针  
	int i;
	if ((fp=fopen(path,"ai+"))==NULL)         //打开指定文件，如果文件不存在则新建该文件  
	{  
		printf("Open Failed.\n");  
		return -1;  
	}
	for(i = 0;i < index;i++)
	{
//		printf("%s",*(array+i));
		fputs(*(array+i),fp);	
	}
	fclose(fp);      //关闭文件 
	return 0;
}

static int write_match_one_line(const char *path,int total_num,long total_time)     
{  
	FILE *fp;   	                                //定义文件指针  
	total_num = total_num*(total_num-1)/2;
	if ((fp=fopen(path,"ai+"))==NULL)         //打开指定文件，如果文件不存在则新建该文件  
	{  
		printf("Open Failed.\n");  
		return -1;  
	}
	fprintf(fp,"总比对次数:%d  总比对时间:%ldms  平均比对时间:%ldms\n",total_num,total_time,total_time/total_num);                 //格式化写入文件（追加至文件末尾）  
	fclose(fp);      //关闭文件 
	return 0;
}

static int get_file_num(char *dir_path)
{
	int num = 0;
	DIR *dirptr = NULL;
	struct dirent *entry;
	if(NULL == (dirptr = opendir(dir_path)))
	{
		printf("open dir failed \n");
		return -1;
	}
	while((entry = readdir(dirptr)))
	{
		if(8 == entry->d_type)
			num++;
	}
	closedir(dirptr);
	return num;
}


int main( int argc, char** argv )
{
	int i,j,k;
	int total_pic = 0;
	long start_time = 0;
    long end_time = 0;
    struct timeval tv;


	char **result;
	char dat_path[PATH_LEN];
	char match_result_path[PATH_LEN];
	char dat_dir[PATH_LEN];
	user_data * p_user_data_1;
	user_data * p_user_data_2;
	float score;


	if(get_config(match_result_path,3))
	{
		printf("error,match_result_path do not exist\n");
		return -1;
	}
	if(get_config(dat_dir,7))
	{
		printf("error,dat_dir do not exist\n");
		return -1;
	}
	total_pic =  get_file_num(dat_dir);
	total_pic = total_pic/2;
	result =(char **)Make2DArray_uint8(WRITE_MATCH_COUNT,RESULT_LEN);
	p_user_data_1 = (user_data*)malloc(total_pic * sizeof(user_data));
	p_user_data_2 = (user_data*)malloc(total_pic * sizeof(user_data));

	if (!access(match_result_path, F_OK))
		remove(match_result_path);

	int num_count_1 = 0;
	int num_count_2 = 0;
	int cut_pos;
	DIR *dirptr = NULL;
	struct dirent *entry;
	if(NULL == (dirptr = opendir(dat_dir)))
	{
		printf("open dir failed \n");
		return -1;
	}
	while((entry = readdir(dirptr)))
	{
		if(8 == entry->d_type)
		{
			if(strstr(entry->d_name,"_1.dat"))
			{
				sprintf(dat_path,"%s/%s",dat_dir,entry->d_name);
				read_data_hex((unsigned char *)(p_user_data_1+num_count_1)->feature,SIZE_FEATURE, dat_path);
				strcpy((p_user_data_1+num_count_1)->name,entry->d_name);
				(p_user_data_1 + num_count_1)->id = num_count_1+1;
//				printf("dat1_path =  %s\n",dat_path);
//				printf("dat1_name =  %s\n",entry->d_name);
				num_count_1++;
			}
			else{
				sprintf(dat_path,"%s/%s",dat_dir,entry->d_name);
				read_data_hex((unsigned char *)(p_user_data_2+num_count_2)->feature,SIZE_FEATURE, dat_path);
				strcpy((p_user_data_2+num_count_2)->name,entry->d_name);
				(p_user_data_2 + num_count_2)->id = num_count_2+1;
//				printf("dat2_path =  %s\n",dat_path);
//				printf("dat2_name =  %s\n",entry->d_name);
				num_count_2++;
			}
		}
	}
	closedir(dirptr);

	printf("A num = %d,B num = %d\n",num_count_1,num_count_2);	
	k = 0;
	gettimeofday(&tv,NULL);
    	start_time = tv.tv_sec*1000+tv.tv_usec/1000;
	for (i = 0;i < num_count_1;i++)
	{
		for(j = 0;j < num_count_2;j++)
		{
			score  = vapis_match(NULL,(p_user_data_1 + i)->feature,1,(p_user_data_2 + j)->feature,1,NULL);
			get_match_result(result,k,(p_user_data_1 + i)->name,(p_user_data_2 + j)->name,score);
			k++;

			if(k == WRITE_MATCH_COUNT)
			{
				k = 0;
				write_match_result(match_result_path,result,WRITE_MATCH_COUNT);     //参数：文件全名，写入内容  
				Clean2DArray_uint8(result,WRITE_MATCH_COUNT,RESULT_LEN);
			}
//			printf("%d  %d  %f\n",(p_user_data_1 + i)->id,(p_user_data_2 + j)->id,*(score + i));
		}
	}
	write_match_result(match_result_path,result,k);     //参数：文件全名，写入内容  
	gettimeofday(&tv,NULL);
   	end_time = tv.tv_sec*1000+tv.tv_usec/1000;
	
	write_match_one_line(match_result_path,num_count_1+num_count_2,end_time-start_time);     
	printf("总个数:%d 总比对时间: %ldms\n",total_pic,end_time-start_time);
	
	Free2DArray_uint8(result,WRITE_MATCH_COUNT);
	free(p_user_data_2);
	p_user_data_2  = NULL;
	free(p_user_data_1);
	p_user_data_1  = NULL;
    return 0;
}
