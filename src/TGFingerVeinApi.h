#ifndef _TGFINGERVEINAPI_H_
#define _TGFINGERVEINAPI_H_

/**************************************************************************************
*����ΪLinux��Windows�½ӿں������÷�ʽ
**************************************************************************************/
#ifdef linux
#define API 
#endif

#ifdef WIN32
#ifndef API
#define   API __declspec(dllimport) __stdcall
#endif
#endif
/**************************************************************************************
*����ΪPCָ�����㷨�ӿں���
**************************************************************************************/
#ifdef __cplusplus 
extern "C" {
#endif
typedef struct
{
	char * vendor; 			   // ���̴���
	char * lib_version;        // ��ǰ lib ��İ汾
	char * release_date;       // yyyy-MM-dd HH:mm:ss ��ʽ
	int	   bio_type;           // ֧�ֵ���������
	char * tmpl_version;       // ģ���ʽ�汾
	int    max_tmpl_size;      // ���ģ���С
	char * feature_version;    // ������ʽ�汾
	int    max_feature_size;   // ���������С
	char * extra_version_info; // Ԥ�������̣��ɳ�����Ҫ˵���������汾��Ϣ
}Vapis_Version;

typedef struct
{
	unsigned char * image_data; // ͼ������(����ʽͷ)
	int             image_size; // ͼ�����ݵĳ���
							    // ͼƬ�ĸ���������Ϣ
}Image;
/*
?	˵��
	���ڳ�ʼ��һ�������ľ�������ڲ�֧�ֻ���Ҫ������㷨������ 0 ���ɡ�Ӧ�ó��򲢲����ľ�������ݣ��ڵ����㷨ʱ������ѳ�ʼ�����ľ��ԭ�����ݸ���غ�����
	��ʼ���ľ�����㷨������������ڼ䣬��������ͷš�����Ӧ�ó������ʱ���Ż���þ���ͷŽӿڣ�������֤һ������þ���ͷŽӿڡ���Ӧ�ó����쳣��ֹ������δ�����ͷž��ʱ����Ӧ�ó����´γ���ʱ�����������³�ʼ���µľ����ԭ���Ӧ�Զ��ͷš�
	��ʼ�����ľ��Ӧ�ɿ��߳�ʹ�á�
	�÷���֧�ֶ�ε��ã�����ε��øýӿڣ�ÿ��Ӧ����һ���µľ����
?	����
?	handle ��������㷨���õ���ؾ������֧�ֻ���Ҫ������㷨������ 0��
?	lib_file_path �����룬�㷨���ļ����ڵ�λ�ã��㷨����Ҫ��������������Դ������ʱ���ɴ����Ŀ¼��ȥ����ص��ļ������м��ء�
?	����ֵ
?	>=0 ��ʾ��ʼ���ɹ�
?	<0 ��ʾ��ʼ���������ش����롣
*/
int API vapis_init_handle(void * handle,char * lib_file_path);

/*
?	˵��
	�ͷ�������㷨������ظ��ͷ�ʱ�������������أ������ش���
?	����
?	handle ͨ����ʼ�����������ʼ�����ľ��
?	����ֵ
?	>=0 ��ʾ���óɹ�
?	<0 ��ʾ���ش�����

*/
int API vapis_release_handle(void * handle);

/*
?	˵��
	��ָ����ͼƬ��ȡһ������������
	ͼƬ������Ϊ1 ����
?	����
?	Handle �����룬ͨ����ʼ������õ����㷨���þ��
?	images �����룬������ȡ������ͼƬ
?	image_size �����룬������ȡ������ͼƬ������>=1
?	feature ���������ȡ����������������Ϊ��׼���ݸ�ʽ����ʽ����μ���ʽ˵��
?	feature_size �����룬�������������Ĵ�С
?	options [����] Ԥ��������ѡ��
?	����ֵ
?	>=0 ��ʾ��ȡ���������Ĵ�С
?	0 ��ʾ������
*/
int API vapis_make_feature(void * handle, Image images[],int image_size,char * feature,int feature_size,char * options);

/*
?	˵��
	��ָ����ͼƬ���ṩ��ϳ�һ��ģ�塣�����ģ��Ϊ��׼��ʽ����ʽ����μ���ʽ˵����
	ͼƬ������Ϊ 1-n �š�������ͼƬ���в��ϸ��ͼƬʱ��Ӧ���Ժ��ԣ�����Ҫ�ڱ�׼��������д�ӡ������Ϣ��
?	����
?	handle ��	���룬ͨ����ʼ�������õ��ľ��
?	images ��	���룬���ںϳ�ģ���ͼ�����飬������СΪһ��
?	image_size�����룬���ںϳ�ģ���ͼ������Ĵ�С��
?	tmpl ��		������ϳɳ���ģ��
?	options ��	���룬�ϳ�ģ��ʱ�ĸ���ѡ��
?	����ֵ
?	>=0 ��ʾģ��Ĵ�С
?	<0 ��ʾ������

*/
int API vapis_make_tmpl(const void * handle,Image images[],int image_size,char * tmpl,char * options);

/*
?	˵��
	��ָ����ģ���ָ���������������ƶȱȶԣ����������ƶȡ�������ģ�岻�ǺϷ���������ģ��ʱ�����ش���
?	����
?	handle �����룬�㷨�����ľ��
?	tmpl �����룬֮ǰע���ģ��
?	tmpl_size �����룬ģ������Ĵ�С
?	feature �����룬Ҫ��֤������
?	feature_size �����룬��������Ĵ�С
?	options �����룬��\0 ��β�ĸ���ѡ���ַ�����UTF-8 ����
?	����ֵ
?	[0-100] ���Ȳ����� 5 λС�������ƶ�
?	0 ʱ����������ʽ�Ĵ�����
*/
double API vapis_match(void * handle,char * tmpl,int tmpl_size,char * feature,int feature_size,char * options);

/*
?	˵��
	��ȡ��ǰ�㷨��İ汾��Ϣ��������İ汾����ʽ�汾��
?	����
?	version_info [���] �汾��Ϣ������汾��Ϣ����
?	����ֵ
?	>=0 ��ʾ�������óɹ�
?	<0 ��ʾ��������ʧ�ܲ����ش�����
*/
int API vapis_get_version_info(Vapis_Version * version_info);


//double API vapis_match1(void * handle, char * feature1, int feature1_size, char * feature2, int feature2_size, char * options);

#ifdef __cplusplus 
}
#endif

#endif