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
#include <sys/time.h>
#include <dirent.h>
#include "TG.h"
#include "TGFingerVeinApi.h"

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
	line_num--;
	if (fp==NULL )
		return -1;
	for(i = 0;i<line_num;i++ ) 
	{
		fgets(str,sizeof(str),fp);
	}
	fgets(str,sizeof(str),fp);
	pstr = strstr(str,"=");
	pstr++;
	length = strlen(pstr);
	*(pstr+length-1) = '\0';
//	printf("pstr=%s.\n",pstr);
	strcpy(dest,pstr);
	fclose(fp);
	return 0;
}

/*****************************************************************  
** @brief       get file num in a dir
** @author      xuhc
** @param1      <in>  <char *>      dir path
** @exception
** @return     -1   : fail
**             else : file num
** @note       
** @see 
** @date       2017/10/14
******************************************************************/
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


static int write_extract_result(const char *path,char *bmp_name)  
{  
	FILE *fp;   	                             
	if ((fp=fopen(path,"ai+"))==NULL)       
	{  
		printf("Open Failed.\n");  
		return -1;  
	}
	fprintf(fp,"%s transfer\n",bmp_name);            
	fclose(fp);     
	return 0;
}

static int write_extract_one_line(const char *path,int success_num,int fail_num,long time)     
{  
	FILE *fp;   	                          
	long avg_time;
	int total_num = success_num + fail_num;
	avg_time = time/total_num;
	if ((fp=fopen(path,"ai+"))==NULL)        
	{  
		printf("Open Failed.\n");  
		return -1;  
	}
	fprintf(fp,"总转换次数:%d 转换成功次数:%d 转换失败次数:%d 总转换时间:%ldms 平均转换时间:%ldms\n",total_num,success_num,fail_num,time,avg_time);         
	fclose(fp);      
	return 0;
}

int main( int argc, char** argv )
{
	int ret;
	int total_pic = 0;
	long start_time = 0;
    long end_time = 0;
	int success_count = 0,fail_count = 0; 
    struct timeval tv;
	int cut_pos = 0;
	DIR *dirptr = NULL;
	struct dirent *entry;
	
	char pic_path[PATH_LEN];
	char dat_path[PATH_LEN];
	char feature [SIZE_FEATURE];

	char extract_result_path_a[PATH_LEN];
	char extract_result_path_b[PATH_LEN];
	char bmp_dir_a[PATH_LEN];
	char bmp_dir_b[PATH_LEN];
	char dat_dir[PATH_LEN];

	Image * img = (Image *)malloc(sizeof(Image));
	img->image_data = (unsigned char*)malloc(SIZE_PIC);

	if(get_config(extract_result_path_a,1))
	{
		printf("error,extract_result_path_a do not exist\n");
		return -1;
	}
	if(get_config(extract_result_path_b,2))
	{
		printf("error,extract_result_path_b do not exist\n");
		return -1;
	}
	if(get_config(bmp_dir_a,5))
	{
		printf("error,bmp_dir_a do not exist\n");
		return -1;
	}
	if(get_config(bmp_dir_b,6))
	{
		printf("error,bmp_dir_b do not exist\n");
		return -1;
	}
	if(get_config(dat_dir,7))
	{
		printf("error,dat_dir do not exist\n");
		return -1;
	}

	if (!access(extract_result_path_a, F_OK))
		remove(extract_result_path_a);
	if (!access(extract_result_path_b, F_OK))
		remove(extract_result_path_b);
/*********************************bmp_a*********************************************/
	gettimeofday(&tv,NULL);
   	start_time = tv.tv_sec*1000+tv.tv_usec/1000;
	if(NULL == (dirptr = opendir(bmp_dir_a)))
	{
		printf("open dir failed \n");
		return -1;
	}
	while((entry = readdir(dirptr)))
	{
		if(8 == entry->d_type)
		{
			total_pic++;
			sprintf(pic_path, "%s/%s",bmp_dir_a,entry->d_name);
//			printf("pic_path = %s\n",pic_path);
			read_data_hex(img->image_data,SIZE_PIC, pic_path);
			sprintf(dat_path, "%s/%s",dat_dir,entry->d_name);
			cut_pos = strlen(dat_path);
			dat_path[cut_pos-4] = '\0';
			strcat(dat_path,".dat");
//			printf("dat_path = %s\n",dat_path);
			write_extract_result(extract_result_path_a,entry->d_name);  		
			ret = vapis_make_feature(NULL, img, 1, feature, 1, NULL);
			if(5776 == ret)
			{
				success_count++;
				write_data_hex((unsigned char *)feature, SIZE_FEATURE,dat_path);
			}
			else
				fail_count++;
		}
	}
	closedir(dirptr);

	gettimeofday(&tv,NULL);
    end_time = tv.tv_sec*1000+tv.tv_usec/1000;
	write_extract_one_line(extract_result_path_a,success_count,fail_count,end_time-start_time);     
	printf("BMP_A 总转换次数:%d 转换成功次数:%d 转换失败次数:%d 总转换时间:%ldms\n",total_pic,success_count,fail_count,end_time-start_time);         
/***********************************************************************************/


/*********************************bmp_a*********************************************/
	total_pic = 0;
	success_count = 0;
	fail_count = 0; 
	gettimeofday(&tv,NULL);
   	start_time = tv.tv_sec*1000+tv.tv_usec/1000;
	if(NULL == (dirptr = opendir(bmp_dir_b)))
	{
		printf("open dir failed \n");
		return -1;
	}
	while((entry = readdir(dirptr)))
	{
		if(8 == entry->d_type)
		{
			total_pic++;
			sprintf(pic_path, "%s/%s",bmp_dir_b,entry->d_name);
//			printf("pic_path = %s\n",pic_path);
			read_data_hex(img->image_data,SIZE_PIC, pic_path);
			sprintf(dat_path, "%s/%s",dat_dir,entry->d_name);
			cut_pos = strlen(dat_path);
			dat_path[cut_pos-4] = '\0';
			strcat(dat_path,".dat");
//			printf("dat_path = %s\n",dat_path);
			write_extract_result(extract_result_path_b,entry->d_name);  		
			ret = vapis_make_feature(NULL, img, 1, feature, 1, NULL);
			if(5776 == ret)
			{
				success_count++;
				write_data_hex((unsigned char *)feature, SIZE_FEATURE,dat_path);
			}
			else
				fail_count++;
		}
	}
	closedir(dirptr);

	gettimeofday(&tv,NULL);
    end_time = tv.tv_sec*1000+tv.tv_usec/1000;
	write_extract_one_line(extract_result_path_b,success_count,fail_count,end_time-start_time);     
	printf("BMP_B 总转换次数:%d 转换成功次数:%d 转换失败次数:%d 总转换时间:%ldms\n",total_pic,success_count,fail_count,end_time-start_time);         
/***********************************************************************************/
	free(img->image_data);
	img->image_data = NULL;
	free(img);
	img = NULL;
    return 0;
}
