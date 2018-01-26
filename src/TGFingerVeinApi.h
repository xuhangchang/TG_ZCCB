#ifndef _TGFINGERVEINAPI_H_
#define _TGFINGERVEINAPI_H_

/**************************************************************************************
*以下为Linux和Windows下接口函数调用方式
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
*以下为PC指静脉算法接口函数
**************************************************************************************/
#ifdef __cplusplus 
extern "C" {
#endif
typedef struct
{
	char * vendor; 			   // 厂商代码
	char * lib_version;        // 当前 lib 库的版本
	char * release_date;       // yyyy-MM-dd HH:mm:ss 格式
	int	   bio_type;           // 支持的生物类型
	char * tmpl_version;       // 模板格式版本
	int    max_tmpl_size;      // 最大模板大小
	char * feature_version;    // 特征格式版本
	int    max_feature_size;   // 最大特征大小
	char * extra_version_info; // 预留给厂商，由厂商需要说明的其它版本信息
}Vapis_Version;

typedef struct
{
	unsigned char * image_data; // 图像数据(带格式头)
	int             image_size; // 图像数据的长度
							    // 图片的附加其它信息
}Image;
/*
?	说明
	用于初始化一个上下文句柄。对于不支持或不需要句柄的算法，返回 0 即可。应用程序并不关心句柄的内容，在调用算法时，将会把初始化出的句柄原样传递给相关函数。
	初始化的句柄在算法库的整个运行期间，不会进行释放。仅当应用程序结束时，才会调用句柄释放接口，但不保证一定会调用句柄释放接口。在应用程序异常终止并导致未调用释放句柄时，在应用程序下次程序时，可正常重新初始化新的句柄。原句柄应自动释放。
	初始化出的句柄应可跨线程使用。
	该方法支持多次调用，即多次调用该接口，每次应返回一个新的句柄。
?	参数
?	handle ：输出，算法调用的相关句柄，不支持或不需要句柄的算法，返回 0。
?	lib_file_path ：输入，算法库文件所在的位置，算法库需要加载其它附加资源或配置时，可从这个目录下去找相关的文件并进行加载。
?	返回值
?	>=0 表示初始化成功
?	<0 表示初始化出错，返回错误码。
*/
int API vapis_init_handle(void * handle,char * lib_file_path);

/*
?	说明
	释放申请的算法句柄。重复释放时，函数正常返回，不返回错误。
?	参数
?	handle 通过初始化句柄函数初始化出的句柄
?	返回值
?	>=0 表示调用成功
?	<0 表示返回错误码

*/
int API vapis_release_handle(void * handle);

/*
?	说明
	用指定的图片提取一个或多个特征。
	图片的数量为1 个。
?	参数
?	Handle ：输入，通过初始化句柄得到的算法调用句柄
?	images ：输入，用来提取特征的图片
?	image_size ：输入，用来提取特征的图片的数量>=1
?	feature ：输出，提取出的特征，该特征为标准数据格式，格式定义参见格式说明
?	feature_size ：输入，传入的特征区域的大小
?	options [输入] 预留的其它选项
?	返回值
?	>=0 表示提取出的特征的大小
?	0 表示错误码
*/
int API vapis_make_feature(void * handle, Image images[],int image_size,char * feature,int feature_size,char * options);

/*
?	说明
	从指定的图片中提供或合成一个模板。输出的模板为标准格式。格式定义参见格式说明。
	图片的数量为 1-n 张。当输入图片中有不合格的图片时，应予以忽略，但需要在标准错误输出中打印警告信息。
?	参数
?	handle ：	输入，通过初始化函数得到的句柄
?	images ：	输入，用于合成模板的图像数组，数量最小为一个
?	image_size：输入，用于合成模板的图像数组的大小。
?	tmpl ：		输出，合成出的模板
?	options ：	输入，合成模板时的附加选项
?	返回值
?	>=0 表示模板的大小
?	<0 表示错误码

*/
int API vapis_make_tmpl(const void * handle,Image images[],int image_size,char * tmpl,char * options);

/*
?	说明
	用指定的模板和指定的特征进行相似度比对，并给出相似度。特征或模板不是合法的特征或模板时，返回错误。
?	参数
?	handle ：输入，算法上下文句柄
?	tmpl ：输入，之前注册的模板
?	tmpl_size ：输入，模板区域的大小
?	feature ：输入，要验证的特征
?	feature_size ：输入，特征区域的大小
?	options ：输入，以\0 结尾的附加选项字符串，UTF-8 编码
?	返回值
?	[0-100] 精度不超过 5 位小数的相似度
?	0 时返回整数形式的错误码
*/
double API vapis_match(void * handle,char * tmpl,int tmpl_size,char * feature,int feature_size,char * options);

/*
?	说明
	获取当前算法库的版本信息，包含库的版本，格式版本等
?	参数
?	version_info [输出] 版本信息，详见版本信息定义
?	返回值
?	>=0 表示函数调用成功
?	<0 表示函数调用失败并返回错误码
*/
int API vapis_get_version_info(Vapis_Version * version_info);


//double API vapis_match1(void * handle, char * feature1, int feature1_size, char * feature2, int feature2_size, char * options);

#ifdef __cplusplus 
}
#endif

#endif