#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <termios.h>
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <sys/time.h>
#include <dirent.h> 
#include "TG.h"
#define INFO_LENGTH 100

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
	*(pstr+length-1) = '\0';
//	printf("pstr=%s.\n",pstr);
	strcpy(dest,pstr);
	fclose(fp);
	return 0;
}

/*****************************************************************  
** @brief       get file's line num
** @author      xuhc
** @param1      <in>  <char *>          path of file
** @exception
** @return     -1:fail
**             else:line num
** @note       
** @see 
** @date       2017/09/21
******************************************************************/
static int get_line_num(char *path)
{
	FILE *fp;
	int i = 0,c;
	fp=fopen(path ,"r"); 
	if (fp!=NULL)
	{
		while ((c=fgetc(fp))!=EOF)
		{ 
			if(c=='\n')
				i++;
		}
	}
	else
		return -1;
	return i;
	fclose(fp);
}


/*****************************************************************  
** @brief       write one line to a specific file
** @author      xuhc
** @param1      <in>  <const char *>          path of file
** @param1      <in>  <char *>          	  write context
** @exception
** @return     -1 : fail
**             0  : success
** @note       
** @see 
** @date       2017/10/14
******************************************************************/
static int write_one_line(const char *path,char *str)     
{  
	FILE *fp;   	                                //定义文件指针  
	if ((fp=fopen(path,"ai+"))==NULL)         //打开指定文件，如果文件不存在则新建该文件  
	{  
		printf("Open Failed.\n");  
		return -1;  
	}
	fprintf(fp,"%s\n",str);                 //格式化写入文件（追加至文件末尾）  
	fclose(fp);      //关闭文件 
	return 0;
}

/*****************************************************************  
** @brief       parse a line to get num1 num2 score
** @author      xuhc
** @param1      <in>   <char *>          string to parse
** @param2      <out>  <char *>          name_1
** @param3      <out>  <char *>          name_2
** @param4      <out>  <float *>          score
** @exception
** @return     
** @note       
** @see 
** @date       2017/09/21
******************************************************************/
static void parse_line(char *str,char *name_1,char *name_2,float *score)
{
	int i = 0;
	int count = 0;
	char temp[INFO_LENGTH] = {0};
	while(*(str+count) == ' ')
		count++;
	while(*(str+count) != ' ')
	{
		name_1[i] = *(str+count);
		i++;
		count++;
	}
	name_1[i] = '\0';

	i = 0;
	while(*(str+count) == ' ')
		count++;
	while(*(str+count) != ' ')
	{
		name_2[i] = *(str+count);
		i++;
		count++;
	}
	name_2[i] = '\0';

	i = 0;
	while(*(str+count) == ' ')
		count++;
	while('\n' != *(str+count))
	{
		temp[i] = *(str+count);
		i++;
		count ++;
	}
	temp[i] = '\0';
	*score = atof(temp);
}

/*****************************************************************  
** @brief       cut tail of two strings,and cmpare
** @author      xuhc
** @param1      <in>  <char *>          string 1
** @param2      <in>  <char *>          string 2
** @exception
** @return     0    : same strings
** 			   else : diff strings
** @note       
** @see 
** @date       2017/10/14
******************************************************************/
static int cut_cmp_str(char *name_1,char *name_2)
{
	int len_1,len_2;
	len_1 = strlen(name_1);
	len_2 = strlen(name_2);
	name_1[len_1-5] = '\0';
	name_2[len_2-5] = '\0';
	return strcmp(name_1,name_2);
}

/*****************************************************************  
** @brief       sort increase float 
** @author      xuhc
** @param1      <in><out>  <char *>       float array
** @param2      <in> 	   <int>          0
** @param3      <in> 	   <int>          length-1
** @exception
** @return     void
** @note       
** @see 
** @date       2017/09/21
******************************************************************/
static void sort_float(float *a, int left, int right)
{
	int i;
	int j ;
	float key ;
    i = left;
    j = right;
    key = a[left];
	if(left >= right)
        return ;
    while(i < j)                             
    {
        while(i < j && key <= a[j])
            j--;
        a[i] = a[j];
        while(i < j && key >= a[i])
            i++;
        a[j] = a[i];
    }
    a[i] = key;
    sort_float(a, left, i - 1);
    sort_float(a, i + 1, right);
}

/*****************************************************************  
** @brief       calculate pass rate and value rate 
** @author      xuhc
** @param1      <in>  <float *>          same kind score list
** @param2      <in>  <float *>          different kind score list
** @param3      <in>  <int>          same kind num
** @param4      <in>  <int>          different kind num
** @param5      <in>  <int>          wrong num target 
** @param6      <in>  <int>          total num target
** @param7      <out>  <float *>          pass rate
** @param8      <out>  <float *>          value score
** @exception
** @return     void
** @note       
** @see 
** @date       2017/09/21
******************************************************************/
int cal_rate(float *same_score,float *diff_score,int same_num,int diff_num,int mis_num,int total_num,float *pass_rate,float *value_rate)
{
	int real_mis_num;
	int mis_num_count = 0;
	int i;
	if( diff_num < total_num)
		return -1;
	real_mis_num = mis_num*(diff_num/total_num);//real_mis_num
	*value_rate =  diff_score[ diff_num - real_mis_num -1];
//	printf("111 %d/%d  %f %f  \n",1,100,pass_rate,value_rate);
	for(i = 0;i<same_num;i++)
	{
		if(same_score[i] > *value_rate)
			break;
	}
	*pass_rate = 1 - (float)i/same_num;
//	printf("222 %d/%d  %f %f  \n",1,100,pass_rate,value_rate);
	return 0;
}

int main(int argc, const char *argv[])
{
	int line_num;
	char name_1[PATH_LEN],name_2[PATH_LEN];
	float score;
	int same_num = 0,diff_num = 0;
	float pass_rate,value_rate;
	int i;

	float *same_score;
	float *diff_score;
	char info[INFO_LENGTH] = {0}; 
	char str_temp[100] = {0};

	char match_result_path[PATH_LEN];
	char rate_result_path[PATH_LEN];
	FILE *fp;
	
	if(get_config(match_result_path,3))
	{
		printf("error,match_result_path do not exist\n");
		return -1;
	}
	if(get_config(rate_result_path,4))
	{
		printf("error,rate_result_path do not exist\n");
		return -1;
	}

	if (!access(rate_result_path, F_OK))
		remove(rate_result_path);
	line_num = get_line_num(match_result_path);
	printf("总比对次数:%d\n",line_num-1);
	printf("---------------------------------------------------------\n");
	if(-1 == line_num)
		return -1;
	line_num--;
	same_score = (float *)malloc(line_num * sizeof(float));
	memset(same_score,0,line_num*sizeof(float));
	diff_score = (float *)malloc(line_num * sizeof(float));
	memset(diff_score,0,line_num*sizeof(float));

	fp = fopen(match_result_path,"r");
	
	for(i = 0;i<line_num;i++)
	{
		memset(info,0,INFO_LENGTH);
		fgets(info,INFO_LENGTH,fp);
		parse_line(info,name_1,name_2,&score);
#if TG_DEBUG
//		printf("name_1 = %s,name_2 = %s,score = %lf\n",name_1,name_2,score);
#endif
		if(cut_cmp_str(name_1,name_2))
		{
			*(diff_score + diff_num) = score; 
			diff_num++;
		}
		else
		{
			*(same_score + same_num) = score; 
			same_num++;
		}

	}	
	sort_float(same_score,0, same_num-1);
	sort_float(diff_score,0, diff_num-1);
#if TG_DEBUG
	for(i = 0;i < 20;i++)
		printf("same_score = %f\n",*(same_score + i));
	for(i = diff_num - 20;i < diff_num;i++)
		printf("diff_score = %f\n",*(diff_score + i));
	printf("same_num = %d,diff_num = %d total_num = %d\n",same_num,diff_num,line_num);
#endif	

/* 										结果信息								 		   */
	write_one_line(rate_result_path,"           误识率           通过率           阈值   ");
	write_one_line(rate_result_path,"---------------------------------------------------------");
	printf("           误识率           通过率           阈值   \n");
	printf("---------------------------------------------------------\n");
	memset(str_temp,0,100);
	if(cal_rate(same_score,diff_score,same_num,diff_num,1,100,&pass_rate,&value_rate))
		sprintf(str_temp,"            num less than 100                ");
	else
		sprintf(str_temp,"            1/100         %f           %2.2f  ",pass_rate,value_rate);
	write_one_line(rate_result_path,str_temp);
	printf("%s\n",str_temp);

	memset(str_temp,0,100);
	if(cal_rate(same_score,diff_score,same_num,diff_num,1,1000,&pass_rate,&value_rate))
		sprintf(str_temp,"            num less than 1000                ");
	else
		sprintf(str_temp,"           1/1000         %f           %2.2f  ",pass_rate,value_rate);
	write_one_line(rate_result_path,str_temp);
	printf("%s\n",str_temp);

	memset(str_temp,0,100);
	if(cal_rate(same_score,diff_score,same_num,diff_num,1,10000,&pass_rate,&value_rate))
		sprintf(str_temp,"            num less than 10000                ");
	else
		sprintf(str_temp,"          1/10000         %f           %2.2f  ",pass_rate,value_rate);
	write_one_line(rate_result_path,str_temp);
	printf("%s\n",str_temp);

	memset(str_temp,0,100);
	if(cal_rate(same_score,diff_score,same_num,diff_num,5,100000,&pass_rate,&value_rate))
		sprintf(str_temp,"            num less than 100000                ");
	else
		sprintf(str_temp,"         5/100000         %f           %2.2f  ",pass_rate,value_rate);
	write_one_line(rate_result_path,str_temp);
	printf("%s\n",str_temp);

	memset(str_temp,0,100);
	if(cal_rate(same_score,diff_score,same_num,diff_num,1,100000,&pass_rate,&value_rate))	
		sprintf(str_temp,"            num less than 100000                ");
	else
		sprintf(str_temp,"         1/100000         %f           %2.2f  ",pass_rate,value_rate);
	write_one_line(rate_result_path,str_temp);
	printf("%s\n",str_temp);

	memset(str_temp,0,100);
	if(cal_rate(same_score,diff_score,same_num,diff_num,1,1000000,&pass_rate,&value_rate))
		sprintf(str_temp,"            num less than 1000000                ");
	else
		sprintf(str_temp,"        1/1000000         %f           %2.2f  ",pass_rate,value_rate);
	write_one_line(rate_result_path,str_temp);
	printf("%s\n",str_temp);

	memset(str_temp,0,100);
	if(cal_rate(same_score,diff_score,same_num,diff_num,1,10000000,&pass_rate,&value_rate))
		sprintf(str_temp,"            num less than 10000000                ");
	else
		sprintf(str_temp,"        1/10000000         %f           %2.2f  ",pass_rate,value_rate);
	write_one_line(rate_result_path,str_temp);
	printf("%s\n",str_temp);


	printf("---------------------------------------------------------\n");
	write_one_line(rate_result_path,"---------------------------------------------------------");
	return 0;
}
