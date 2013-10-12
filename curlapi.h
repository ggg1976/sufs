#ifndef __CURL_API_H__
#define __CURL_API_H__

// curl工具的数据体：
typedef struct CURL_DATA
{
	FIFO * liWrite;	// 向swift发送数据使用的FIFO，这里可以包含头数据，但是需要用空的"\r\n"来分界。
	FIFO * liRead;	// 接收swift数据使用的FIFO，如果有http头，则需要以空的"\r\n"分界，头数据以"\r\n"来分行，而容器列表和对象列表是以"\n"来分界的。
	int bHeadValid;	// 向curl表明是否需要在接收函数中接收http头数据。
	int nCurlCode;	// curl的返回值。
	void * hCurl;	// 这个参数暂时不用，它是CURL*，curl_easy_init的返回值。
	//int nMethod;	// 这是当前curl应该使用的方法的指针，指定strMethod数组中的一个字符串。
	//char strMethod[10][10];	// 使用的http方法，主要包括:GET,PUT,POST,HEAD,COPY,DELETE等。
	//char strHead[20][128];	//
	//int nHeadSize;	//
	//FILE* pFile;	//the point of the file which is read/writon  
	//int nFileLength;//the file length fro upload.
}Curl_Data;

//alloc and free memory:
Curl_Data * curl_new_data();
void curl_delete_data(Curl_Data* curldata);
void curl_clear_data(Curl_Data * curldata);
void curl_clear_senddata(Curl_Data * curldata);
void curl_print_data(Curl_Data * curldata);
FIFO * curl_pop_fifo_data(Curl_Data * curldata);

//swift api for curl,if the error happens ,check the curl code.

/* Account APIs:*/
int curl_get_return_code(Curl_Data * curldata);
int curlapi_authv1(char * user,char * passwd,char * auth_url,Curl_Data * curldata);
int curlapi_get_account_data(char * token,char * url,int limit,char * marker,char * end_marker,Curl_Data * curldata);
int curlapi_get_account_metadata(char * token,char * url,Curl_Data * curldata);
int curlapi_update_account_metadata(char * token,char * url,Curl_Data * curldata);
int curlapi_delete_account_metadata(char * token,char * url,Curl_Data * curldata);
/* Container APIs:*/
int curlapi_get_container_data(char * token,char * url,char * container,int limit,char * marker,char * end_marker,char * prefix,char * delimiter,char * path,Curl_Data * curldata);
int curlapi_create_container(char * token,char * url,char * container,Curl_Data * curldata);
int curlapi_delete_container(char * token,char * url,char * container,Curl_Data * curldata);
int curlapi_get_container_metadata(char * token,char * url,char * container,Curl_Data * curldata);
int curlapi_update_container_metadata(char * token,char * url,char * container,Curl_Data * curldata);
int curlapi_delete_container_metadata(char * token,char * url,char * container,Curl_Data * curldata);
/* Object APIs:*/
int curlapi_get_object_data(char * token,char * url,char * container,char * object,Curl_Data * curldata);
int curlapi_create_object_data(char * token,char * url,char * container,char * object,char * path,Curl_Data * curldata);
int curlapi_create_object_null(char * token,char * url,char * container,char * object,Curl_Data * curldata);
int curlapi_copy_object_data(char * token,char * url,char * dest_container,char * dest_obj,char * source_container,char * source_obj,Curl_Data * curldata);
int curlapi_delete_object_data(char * token,char * url,char * container,char * object,Curl_Data * curldata);
int curlapi_get_object_metadata(char * token,char * url,char * container,char * object,Curl_Data * curldata);
int curlapi_update_object_metadata(char * token,char * url,char * container,char * object,Curl_Data * curldata);
int curlapi_delete_object_metadata(char * token,char * url,char * container,char * object,Curl_Data * curldata);





#endif
