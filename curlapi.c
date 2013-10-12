// communication tools which send a message to or receive a message from swift proxy.The message is Swift API.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <curl/easy.h>

#include "sufs.h"
#include "curlapi.h"

Curl_Data * curl_new_data()
{
	Curl_Data * curldata = (Curl_Data*)malloc(sizeof(Curl_Data));
	if(curldata==NULL)
		return NULL;
	memset(curldata,0,sizeof(Curl_Data));
	curldata->liWrite = fifo_new_list();
	if(curldata->liWrite==NULL)
	{
		return NULL;
	}
	curldata->liRead = fifo_new_list();
	if(curldata->liRead==NULL)
	{
		return NULL;
	}
	if(curl_global_init(CURL_GLOBAL_ALL)!=0)
	{
		return NULL;
	}
	return curldata;
}
void curl_delete_data(Curl_Data* curldata)
{
	if(curldata)
	{
		if(curldata->liRead)
		{
			//printf("delete read fifo:%x\n",curldata->liRead);
			fifo_delete_list(curldata->liRead);
		}
		if(curldata->liWrite)
		{
			//printf("delete write fifo:%x\n",curldata->liWrite);
			fifo_delete_list(curldata->liWrite);
		}
		free(curldata);
	}

	curl_global_cleanup();
}

// all the function DO NOT check the validate of the SwiftData struct data .
void curl_clear_data(Curl_Data * curldata)
{
	fifo_release_data(curldata->liRead);
}
void curl_clear_senddata(Curl_Data * curldata)
{
	fifo_release_data(curldata->liWrite);
}
void curl_print_data(Curl_Data * curldata)
{
	memory_print_data(curldata->liRead);
}
//output the list in the Curl_Data to write the file.
FIFO * curl_pop_fifo_data(Curl_Data * curldata)
{
	FIFO * li = fifo_new_list();
	if(li==NULL)
		return NULL;
	
	FIFO * pop = curldata->liRead;
	curldata->liRead = li;

	return pop;
}
//curl download data function:the data maybe a part of one communication,so fifo memory is needed.
size_t downloadsection(void * buffer,size_t size,size_t count,void * user)
{
	Curl_Data * curldata = (Curl_Data*)user;

	int sum = count*size;
	MemoryBuffer * mem = memory_new_data(sum);
	
	if(mem==NULL)
	{
		printf("memory opt error.\n");
		return 0;
	}
	fifo_push_node(mem,curldata->liRead);
	memcpy(mem->lpBuffer,buffer,sum);
		
	return sum;
}
size_t uploadsection(void * buffer,size_t size,size_t count,void * user) 
{
	Curl_Data * curldata = (Curl_Data*)user;
		
	if(size!=1)
	{
		printf("size is not 1!!!!!!");
		return 0;
	}
	return memory_output_data((char*)buffer,count,curldata->liWrite);
}
//file operations:
size_t download_file_from_swift(void * buffer,size_t size,size_t count,void * user)
{
	return fwrite(buffer,size,count,(FILE*)user);
}
size_t upload_file_to_swift(void * buffer,size_t size,size_t count,void * user) 
{
	return fread(buffer,size,count,(FILE*)user);
}
/*
// common API:
int normal_api_download_data(char * uri,char * method,Curl_Data * curldata)
{
	//common use api,but the header must be writon by yourself,and the end flag is "\r\n".
	curl_clear_data(curldata);	
	
	struct curl_slist * headers = NULL;
	CURL *curl_handle = curl_easy_init();
	curl_easy_reset(curl_handle);
	// set uri:
	curl_easy_setopt(curl_handle,CURLOPT_URL,uri);
	// set the headers can be downloaded.
	curl_easy_setopt(curl_handle,CURLOPT_HEADER,curldata->bHeadValid);
	// set the method:GET,HEAD,DELETE,COPY
	curl_easy_setopt(curl_handle,CURLOPT_CUSTOMREQUEST,method);
	if(method=="GET")
		curl_easy_setopt(curl_handle,CURLOPT_NOBODY,0);
	else
		curl_easy_setopt(curl_handle,CURLOPT_NOBODY,1);
	// set the header parameters:
	
	MemoryBuffer * mem = fifo_pop_node(curldata->liWrite);
	while(mem)
	{
		if(mem->nSize==2)
		{
			//find '\r\n' then the liWrite data is end,PAY ATTENTION:DO NOT add the headers,or error.
			//memory_delete_data(mem);
			break;
		}
		headers = curl_slist_append(headers,mem->lpBuffer);
		//memory_delete_data(mem);
		mem = fifo_pop_node(curldata->liWrite);
	}
	curl_easy_setopt(curl_handle,CURLOPT_HTTPHEADER,headers);	
	
	// set download function info
	curl_easy_setopt(curl_handle,CURLOPT_WRITEFUNCTION,&downloadsection);
	curl_easy_setopt(curl_handle,CURLOPT_WRITEDATA,curldata);

	curl_easy_perform(curl_handle);
	curl_easy_getinfo(curl_handle,CURLINFO_RESPONSE_CODE,&(curldata->nCurlCode));
	curl_slist_free_all(headers);
	curl_easy_cleanup(curl_handle);
	
	return curldata->nCurlCode;
} 

int normal_api_upload_data(char * uri,char * method,Curl_Data * curldata)
{
	curl_clear_data(curldata);
	
	struct curl_slist * headers = NULL;
	CURL *curl_handle = curl_easy_init();
	curl_easy_reset(curl_handle);
	// set uri
	curl_easy_setopt(curl_handle,CURLOPT_URL,uri);
	// set PUT
	if(method=="PUT")
		curl_easy_setopt(curl_handle,CURLOPT_UPLOAD,1);	//=curl_easy_setopt(curl_handle,CURLOPT_PUT,1);
	else
		curl_easy_setopt(curl_handle,CURLOPT_POST,1);
	// set no body data,only the headers data in the returned data.
	curl_easy_setopt(curl_handle,CURLOPT_NOBODY,1);
	// set the header not return.
	curl_easy_setopt(curl_handle,CURLOPT_HEADER,0);
	// set the header parameters:
	MemoryBuffer * mem = fifo_pop_node(curldata->liWrite);
	while(mem)
	{
		if(mem->nSize==2)
		{
			//find '\r\n' then the liWrite data is end,PAY ATTENTION:DO NOT add the headers,or error.
			//memory_delete_data(mem);
			break;
		}
		headers = curl_slist_append(headers,mem->lpBuffer);
		//memory_delete_data(mem);
		mem = fifo_pop_node(curldata->liWrite);
	}
	curl_easy_setopt(curl_handle,CURLOPT_HTTPHEADER,headers);	

	// set upload function info
	curl_easy_setopt(curl_handle,CURLOPT_READFUNCTION,&uploadsection);
	curl_easy_setopt(curl_handle,CURLOPT_READDATA,curldata);
	
	//curl_easy_setopt(curl_handle,CURLOPT_VERBOSE,1);

	curl_easy_perform(curl_handle);
	curl_easy_getinfo(curl_handle,CURLINFO_RESPONSE_CODE,&(curldata->nCurlCode));
	curl_slist_free_all(headers);
	curl_easy_cleanup(curl_handle);

	return curldata->nCurlCode;
}
*/
int curl_get_return_code(Curl_Data * curldata)
{
	return curldata->nCurlCode;
}
//APIs:
int curlapi_authv1(char * user,char * passwd,char * auth_url,Curl_Data * curldata)
{
	curl_clear_data(curldata);

	//gettoken and no check curl error:
	struct curl_slist * headers = NULL;
	CURL *curl_handle = curl_easy_init();
	curl_easy_reset(curl_handle);
	curl_easy_setopt(curl_handle,CURLOPT_URL,auth_url);
	curl_easy_setopt(curl_handle,CURLOPT_HEADER,curldata->bHeadValid);
	headers = curl_slist_append(headers,user);
	headers = curl_slist_append(headers,passwd);
	
	curl_easy_setopt(curl_handle,CURLOPT_HTTPHEADER,headers);
	curl_easy_setopt(curl_handle,CURLOPT_WRITEFUNCTION,&downloadsection);
	curl_easy_setopt(curl_handle,CURLOPT_WRITEDATA,curldata);
	
	curl_easy_perform(curl_handle);
	curl_easy_getinfo(curl_handle,CURLINFO_RESPONSE_CODE,&(curldata->nCurlCode));
	curl_slist_free_all(headers);
	curl_easy_cleanup(curl_handle);
	
	return curldata->nCurlCode;
}
// account api:
int curlapi_get_account_data(char * token,char * url,int limit,char * marker,char * end_marker,Curl_Data * curldata)
{
	char uri[1024];
	char paras[1024];
	char flag = '?';

	curl_clear_data(curldata);
	strcpy(uri,url);
	if(limit)
	{
		sprintf(paras,"%climit=%d",flag,limit);
		flag = '&';
		strcat(uri,paras);
	}
	if(marker)
	{
		sprintf(paras,"%cmarker=%s",flag,marker);
		flag = '&';
		strcat(uri,paras);
	}
	if(end_marker)
	{
		sprintf(paras,"%cend_marker=%s",flag,marker);
		strcat(uri,paras);
	}

	struct curl_slist * headers = NULL;
	CURL *curl_handle = curl_easy_init();
	curl_easy_reset(curl_handle);
	curl_easy_setopt(curl_handle,CURLOPT_URL,uri);
	curl_easy_setopt(curl_handle,CURLOPT_HEADER,curldata->bHeadValid);

	headers = curl_slist_append(headers,token);
	curl_easy_setopt(curl_handle,CURLOPT_HTTPHEADER,headers);
	curl_easy_setopt(curl_handle,CURLOPT_WRITEFUNCTION,&downloadsection);
	curl_easy_setopt(curl_handle,CURLOPT_WRITEDATA,curldata);

	curl_easy_perform(curl_handle);
	curl_easy_getinfo(curl_handle,CURLINFO_RESPONSE_CODE,&(curldata->nCurlCode));
	curl_slist_free_all(headers);
	curl_easy_cleanup(curl_handle);
	
	return curldata->nCurlCode;
}
int curlapi_get_account_metadata(char * token,char * url,Curl_Data * curldata)
{
	curl_clear_data(curldata);
	
	struct curl_slist * headers = NULL;
	CURL *curl_handle = curl_easy_init();
	curl_easy_reset(curl_handle);
	curl_easy_setopt(curl_handle,CURLOPT_URL,url);
	curl_easy_setopt(curl_handle,CURLOPT_HEADER,curldata->bHeadValid);
	curl_easy_setopt(curl_handle,CURLOPT_CUSTOMREQUEST,"HEAD");
	curl_easy_setopt(curl_handle,CURLOPT_NOBODY,1);
	
	headers = curl_slist_append(headers,token);
	curl_easy_setopt(curl_handle,CURLOPT_HTTPHEADER,headers);
	curl_easy_setopt(curl_handle,CURLOPT_WRITEFUNCTION,&downloadsection);
	curl_easy_setopt(curl_handle,CURLOPT_WRITEDATA,curldata);

	curl_easy_perform(curl_handle);
	curl_easy_getinfo(curl_handle,CURLINFO_RESPONSE_CODE,&(curldata->nCurlCode));
	curl_slist_free_all(headers);
	curl_easy_cleanup(curl_handle);
		
	return curldata->nCurlCode;
}
int curlapi_update_account_metadata(char * token,char * url,Curl_Data * curldata)
{
	curl_clear_data(curldata);
	struct curl_slist * headers = NULL;
	CURL *curl_handle = curl_easy_init();
	curl_easy_reset(curl_handle);
	// set uri
	curl_easy_setopt(curl_handle,CURLOPT_URL,url);
	// set POST
	curl_easy_setopt(curl_handle,CURLOPT_POST,1);
	// set no body data,only the headers data in the returned data.CURLOPT_NOBODY=1 will be error.
	//curl_easy_setopt(curl_handle,CURLOPT_NOBODY,1);

	curl_easy_setopt(curl_handle,CURLOPT_HEADER,curldata->bHeadValid);
	// set header:
	headers = curl_slist_append(headers,token);
	headers = curl_slist_append(headers,"Content-Length: 0");
	MemoryBuffer * mem = fifo_pop_node(curldata->liWrite);
	while(mem)
	{
		if(mem->nSize==2)
		{
			//find '\r\n' then the liWrite data is end,PAY ATTENTION:DO NOT add the headers,or error.
			//memory_delete_data(mem);
			break;
		}
		headers = curl_slist_append(headers,mem->lpBuffer);
		//memory_delete_data(mem);
		mem = fifo_pop_node(curldata->liWrite);
	}
	curl_easy_setopt(curl_handle,CURLOPT_HTTPHEADER,headers);
	
	curl_easy_perform(curl_handle);
	curl_easy_getinfo(curl_handle,CURLINFO_RESPONSE_CODE,&(curldata->nCurlCode));
	curl_slist_free_all(headers);
	curl_easy_cleanup(curl_handle);
	
	return curldata->nCurlCode;
}
int curlapi_delete_account_metadata(char * token,char * url,Curl_Data * curldata)
{
	return curlapi_update_account_metadata(token,url,curldata);
}
// container api:
int curlapi_get_container_data(char * token,char * url,char * container,int limit,char * marker,char * end_marker,char * prefix,char * delimiter,char * path,Curl_Data * curldata)
{
	char uri[1024];
	char paras[1024];
	char flag = '?';

	curl_clear_data(curldata);
	sprintf(uri,"%s/%s",url,container);
	if(limit)
	{
		sprintf(paras,"%climit=%d",flag,limit);
		flag = '&';
		strcat(uri,paras);
	}
	if(marker)
	{
		sprintf(paras,"%cmarker=%s",flag,marker);
		flag = '&';
		strcat(uri,paras);
	}
	if(end_marker)
	{
		sprintf(paras,"%cend_marker=%s",flag,marker);
		flag = '&';
		strcat(uri,paras);
	}
	if(prefix)
	{
		sprintf(paras,"%cprefix=%s",flag,prefix);
		flag = '&';
		strcat(uri,paras);
	}
	if(delimiter)
	{
		sprintf(paras,"%cdelimiter=%s",flag,delimiter);
		flag = '&';
		strcat(uri,paras);
	}
	if(path)
	{
		sprintf(paras,"%cpath=%s",flag,path);
		//flag = '&';
		strcat(uri,paras);
	}
	//printf("%s\n",uri);

	struct curl_slist * headers = NULL;	
	CURL *curl_handle = curl_easy_init();

	curl_easy_reset(curl_handle);
	curl_easy_setopt(curl_handle,CURLOPT_URL,uri);
	curl_easy_setopt(curl_handle,CURLOPT_HEADER,curldata->bHeadValid);
	
	headers = curl_slist_append(headers,token);
	curl_easy_setopt(curl_handle,CURLOPT_HTTPHEADER,headers);
	curl_easy_setopt(curl_handle,CURLOPT_WRITEFUNCTION,&downloadsection);
	curl_easy_setopt(curl_handle,CURLOPT_WRITEDATA,curldata);

	curl_easy_perform(curl_handle);
	curl_easy_getinfo(curl_handle,CURLINFO_RESPONSE_CODE,&(curldata->nCurlCode));
	curl_slist_free_all(headers);
	curl_easy_cleanup(curl_handle);
	
	return curldata->nCurlCode;
}
int curlapi_create_container(char * token,char * url,char * container,Curl_Data * curldata)
{
	curl_clear_data(curldata);
	char uri[1024];
	sprintf(uri,"%s/%s",url,container);	
	
	struct curl_slist * headers = NULL;
	CURL *curl_handle = curl_easy_init();
	curl_easy_reset(curl_handle);
	// set uri
	curl_easy_setopt(curl_handle,CURLOPT_URL,uri);
	// set PUT
	curl_easy_setopt(curl_handle,CURLOPT_UPLOAD,1);	
	
	curl_easy_setopt(curl_handle,CURLOPT_HEADER,curldata->bHeadValid);

	// set header:
	headers = curl_slist_append(headers,token);

	MemoryBuffer * mem = fifo_pop_node(curldata->liWrite);
	while(mem)
	{
		if(mem->nSize==2)
		{
			//find '\r\n' then the liWrite data is end,PAY ATTENTION:DO NOT add the headers,or error.
			//memory_delete_data(mem);
			break;
		}
		headers = curl_slist_append(headers,mem->lpBuffer);
		//memory_delete_data(mem);
		mem = fifo_pop_node(curldata->liWrite);
	}
	headers = curl_slist_append(headers,"Content-Length: 0");
	headers = curl_slist_append(headers,"Expect:");
	headers = curl_slist_append(headers,"Connection: close");
	
	curl_easy_setopt(curl_handle,CURLOPT_HTTPHEADER,headers);

	curl_easy_perform(curl_handle);
	curl_easy_getinfo(curl_handle,CURLINFO_RESPONSE_CODE,&(curldata->nCurlCode));
	curl_slist_free_all(headers);
	curl_easy_cleanup(curl_handle);
		
	return curldata->nCurlCode;
}
int curlapi_create_container2(char * token,char * url,char * container,Curl_Data * curldata)
{
	curl_clear_data(curldata);
	char uri[1024];	
	sprintf(uri,"%s/%s",url,container);
	
	struct curl_slist * headers = NULL;
	CURL *curl_handle = curl_easy_init();
	curl_easy_reset(curl_handle);
	curl_easy_setopt(curl_handle,CURLOPT_URL,uri);
	curl_easy_setopt(curl_handle,CURLOPT_HEADER,curldata->bHeadValid);
	curl_easy_setopt(curl_handle,CURLOPT_CUSTOMREQUEST,"PUT");
	curl_easy_setopt(curl_handle,CURLOPT_NOBODY,1);
	
	headers = curl_slist_append(headers,token);
	MemoryBuffer * mem = fifo_pop_node(curldata->liWrite);
	while(mem)
	{
		if(mem->nSize==2)
		{
			//find '\r\n' then the liWrite data is end,PAY ATTENTION:DO NOT add the headers,or error.
			memory_delete_data(mem);
			break;
		}
		headers = curl_slist_append(headers,mem->lpBuffer);
		memory_delete_data(mem);
		mem = fifo_pop_node(curldata->liWrite);
	}
	headers = curl_slist_append(headers,"Content-Length: 0");
	
	curl_easy_setopt(curl_handle,CURLOPT_HTTPHEADER,headers);

	curl_easy_perform(curl_handle);
	curl_easy_getinfo(curl_handle,CURLINFO_RESPONSE_CODE,&(curldata->nCurlCode));
	curl_slist_free_all(headers);
	curl_easy_cleanup(curl_handle);
		
	return curldata->nCurlCode;	
}

int curlapi_create_container3(char * token,char * url,char * container,Curl_Data * curldata)
{
	curl_clear_data(curldata);
	char uri[1024];
	sprintf(uri,"%s/%s",url,container);	
	
	struct curl_slist * headers = NULL;
	CURL *curl_handle = curl_easy_init();
	curl_easy_reset(curl_handle);
	
	curl_easy_setopt(curl_handle,CURLOPT_URL,uri);
	curl_easy_setopt(curl_handle,CURLOPT_UPLOAD,1);

	headers = curl_slist_append(headers,token);
	headers = curl_slist_append(headers,"Expect:");
	MemoryBuffer * mem = fifo_pop_node(curldata->liWrite);
	while(mem)
	{
		if(mem->nSize==2)
		{
			//find '\r\n' then the liWrite data is end,PAY ATTENTION:DO NOT add the headers,or error.
			//memory_delete_data(mem);
			break;
		}
		headers = curl_slist_append(headers,mem->lpBuffer);
		//memory_delete_data(mem);
		mem = fifo_pop_node(curldata->liWrite);
	}
	headers = curl_slist_append(headers,"Connection: close");
	curl_easy_setopt(curl_handle,CURLOPT_HTTPHEADER,headers);
	
	curl_easy_setopt(curl_handle,CURLOPT_READFUNCTION,&upload_file_to_swift);
	curl_easy_setopt(curl_handle,CURLOPT_READDATA,curldata);
	curl_easy_setopt(curl_handle,CURLOPT_HEADER,curldata->bHeadValid);
	curl_easy_setopt(curl_handle,CURLOPT_INFILESIZE,0); 

	curl_easy_perform(curl_handle);
	curl_easy_getinfo(curl_handle,CURLINFO_RESPONSE_CODE,&(curldata->nCurlCode));
	curl_slist_free_all(headers);
	curl_easy_cleanup(curl_handle);

	return curldata->nCurlCode;
}
int curlapi_delete_container(char * token,char * url,char * container,Curl_Data * curldata)
{
	curl_clear_data(curldata);
	char uri[1024];	
	sprintf(uri,"%s/%s",url,container);
	struct curl_slist * headers = NULL;
	CURL *curl_handle = curl_easy_init();
	curl_easy_reset(curl_handle);
	curl_easy_setopt(curl_handle,CURLOPT_URL,uri);
	curl_easy_setopt(curl_handle,CURLOPT_HEADER,curldata->bHeadValid);
	curl_easy_setopt(curl_handle,CURLOPT_CUSTOMREQUEST,"DELETE");
	curl_easy_setopt(curl_handle,CURLOPT_NOBODY,1);
	
	headers = curl_slist_append(headers,token);
	curl_easy_setopt(curl_handle,CURLOPT_HTTPHEADER,headers);
	
	curl_easy_perform(curl_handle);
	curl_easy_getinfo(curl_handle,CURLINFO_RESPONSE_CODE,&(curldata->nCurlCode));
	curl_slist_free_all(headers);
	curl_easy_cleanup(curl_handle);
		
	return curldata->nCurlCode;
}
int curlapi_get_container_metadata(char * token,char * url,char * container,Curl_Data * curldata)
{
	curl_clear_data(curldata);
	char uri[1024];	
	sprintf(uri,"%s/%s",url,container);
	
	struct curl_slist * headers = NULL;
	CURL *curl_handle = curl_easy_init();
	curl_easy_reset(curl_handle);
	curl_easy_setopt(curl_handle,CURLOPT_URL,uri);
	curl_easy_setopt(curl_handle,CURLOPT_HEADER,curldata->bHeadValid);
	curl_easy_setopt(curl_handle,CURLOPT_CUSTOMREQUEST,"HEAD");
	curl_easy_setopt(curl_handle,CURLOPT_NOBODY,1);
	
	headers = curl_slist_append(headers,token);
	curl_easy_setopt(curl_handle,CURLOPT_HTTPHEADER,headers);
	curl_easy_setopt(curl_handle,CURLOPT_WRITEFUNCTION,&downloadsection);
	curl_easy_setopt(curl_handle,CURLOPT_WRITEDATA,curldata);

	curl_easy_perform(curl_handle);
	curl_easy_getinfo(curl_handle,CURLINFO_RESPONSE_CODE,&(curldata->nCurlCode));
	curl_slist_free_all(headers);
	curl_easy_cleanup(curl_handle);
		
	return curldata->nCurlCode;
}
int curlapi_update_container_metadata(char * token,char * url,char * container,Curl_Data * curldata)
{
	curl_clear_data(curldata);
	char uri[1024];
	sprintf(uri,"%s/%s",url,container);	
	
	struct curl_slist * headers = NULL;
	CURL *curl_handle = curl_easy_init();
	curl_easy_reset(curl_handle);
	// set uri
	curl_easy_setopt(curl_handle,CURLOPT_URL,uri);
	// set POST
	curl_easy_setopt(curl_handle,CURLOPT_POST,1);	
	
	curl_easy_setopt(curl_handle,CURLOPT_HEADER,curldata->bHeadValid);
	// set header:
	headers = curl_slist_append(headers,token);
	headers = curl_slist_append(headers,"Content-Length: 0");
	MemoryBuffer * mem = fifo_pop_node(curldata->liWrite);
	while(mem)
	{
		if(mem->nSize==2)
		{
			//find '\r\n' then the liWrite data is end,PAY ATTENTION:DO NOT add the headers,or error.
			//memory_delete_data(mem);
			break;
		}
		headers = curl_slist_append(headers,mem->lpBuffer);
		//memory_delete_data(mem);
		mem = fifo_pop_node(curldata->liWrite);
	}
	curl_easy_setopt(curl_handle,CURLOPT_HTTPHEADER,headers);

	curl_easy_perform(curl_handle);
	curl_easy_getinfo(curl_handle,CURLINFO_RESPONSE_CODE,&(curldata->nCurlCode));
	curl_slist_free_all(headers);
	curl_easy_cleanup(curl_handle);
		
	return curldata->nCurlCode;
}
int curlapi_delete_container_metadata(char * token,char * url,char * container,Curl_Data * curldata)
{	
	return curlapi_update_container_metadata(token,url,container,curldata);
}
// object api:
int curlapi_get_object_data(char * token,char * url,char * container,char * object,Curl_Data * curldata)
{
	curl_clear_data(curldata);

	char uri[1024];
	sprintf(uri,"%s/%s/%s",url,container,object);	
	
	struct curl_slist * headers = NULL;
	CURL *curl_handle = curl_easy_init();
	curl_easy_reset(curl_handle);
	curl_easy_setopt(curl_handle,CURLOPT_URL,uri);
	curl_easy_setopt(curl_handle,CURLOPT_HEADER,curldata->bHeadValid);
	
	headers = curl_slist_append(headers,token);
	curl_easy_setopt(curl_handle,CURLOPT_HTTPHEADER,headers);
	curl_easy_setopt(curl_handle,CURLOPT_WRITEFUNCTION,&downloadsection);
	curl_easy_setopt(curl_handle,CURLOPT_WRITEDATA,curldata);

	curl_easy_perform(curl_handle);
	curl_easy_getinfo(curl_handle,CURLINFO_RESPONSE_CODE,&(curldata->nCurlCode));
	curl_slist_free_all(headers);
	curl_easy_cleanup(curl_handle);
		
	return curldata->nCurlCode;
}
int curlapi_create_object_data2(char * token,char * url,char * container,char * object,Curl_Data * curldata)
{
	curl_clear_data(curldata);
	char uri[1024];
	sprintf(uri,"%s/%s/%s",url,container,object);	
	
	struct curl_slist * headers = NULL;
	CURL *curl_handle = curl_easy_init();
	curl_easy_reset(curl_handle);
	// set uri
	curl_easy_setopt(curl_handle,CURLOPT_URL,uri);
	// set PUT
	curl_easy_setopt(curl_handle,CURLOPT_UPLOAD,1);	
	curl_easy_setopt(curl_handle,CURLOPT_HEADER,curldata->bHeadValid);
	// set header:
	headers = curl_slist_append(headers,token);
	// set length:
	int sum = memory_get_data_size(curldata->liWrite);
	//char length[64];
	//sprinft(length,"Content-Length: %d",sum);
	//headers = curl_slist_append(headers,length);	
	
	headers = curl_slist_append(headers,"Expect:");
	curl_easy_setopt(curl_handle,CURLOPT_HTTPHEADER,headers);
	
	curl_easy_setopt(curl_handle,CURLOPT_READFUNCTION,&uploadsection);
	curl_easy_setopt(curl_handle,CURLOPT_READDATA,curldata);
	curl_easy_setopt(curl_handle,CURLOPT_VERBOSE,1);
	//curl_easy_setopt(curl_handle,CURLOPT_INFILE,curldata->pFile);
	curl_easy_setopt(curl_handle,CURLOPT_HEADER,1);
	// set Content-Length=sum
	curl_easy_setopt(curl_handle,CURLOPT_INFILESIZE,sum);
	
	curl_easy_perform(curl_handle);
	curl_easy_getinfo(curl_handle,CURLINFO_RESPONSE_CODE,&(curldata->nCurlCode));
	curl_slist_free_all(headers);
	curl_easy_cleanup(curl_handle);
		
	return curldata->nCurlCode;
}
int curlapi_create_object_null(char * token,char * url,char * container,char * object,Curl_Data * curldata)
{
	curl_clear_data(curldata);
	char uri[1024];
	sprintf(uri,"%s/%s/%s",url,container,object);	

	struct curl_slist * headers = NULL;
	CURL *curl_handle = curl_easy_init();
	curl_easy_reset(curl_handle);
	curl_easy_setopt(curl_handle,CURLOPT_URL,uri);
	curl_easy_setopt(curl_handle,CURLOPT_HEADER,curldata->bHeadValid);
	curl_easy_setopt(curl_handle,CURLOPT_CUSTOMREQUEST,"PUT");
	curl_easy_setopt(curl_handle,CURLOPT_NOBODY,1);
	
	headers = curl_slist_append(headers,token);
	headers = curl_slist_append(headers,"Content-Length: 0");
	curl_easy_setopt(curl_handle,CURLOPT_HTTPHEADER,headers);
	//curl_easy_setopt(curl_handle,CURLOPT_WRITEFUNCTION,&downloadsection);
	//curl_easy_setopt(curl_handle,CURLOPT_WRITEDATA,curldata);

	curl_easy_perform(curl_handle);
	curl_easy_getinfo(curl_handle,CURLINFO_RESPONSE_CODE,&(curldata->nCurlCode));
	curl_slist_free_all(headers);
	curl_easy_cleanup(curl_handle);
		
	return curldata->nCurlCode;	
}
int curlapi_create_object_data(char * token,char * url,char * container,char * object,char * path,Curl_Data * curldata)
{
	curl_clear_data(curldata);
	char uri[1024];
	sprintf(uri,"%s/%s/%s",url,container,object);	
	//printf("%s\n",uri);
	
	FILE * pf = fopen(path,"rb");
	if(pf==0)
	{
		printf("The file can not open.\n");
		return -111;
	}
	fseek(pf,0,2);
	int sum = ftell(pf);
	rewind(pf);
	//printf("[%d]:%s///////////////////////////////////////////////\n",sum,path);
	if(sum>5000000000)
		return -6;
	//curldata->nFileLength = sum;
	
	struct curl_slist * headers = NULL;
	CURL *curl_handle = curl_easy_init();
	curl_easy_reset(curl_handle);
	curl_easy_setopt(curl_handle,CURLOPT_URL,uri);
	curl_easy_setopt(curl_handle,CURLOPT_UPLOAD,1);
	
	headers = curl_slist_append(headers,token);
	headers = curl_slist_append(headers,"Expect:");
	curl_easy_setopt(curl_handle,CURLOPT_HTTPHEADER,headers);
	
	curl_easy_setopt(curl_handle,CURLOPT_READFUNCTION,&upload_file_to_swift);
	curl_easy_setopt(curl_handle,CURLOPT_READDATA,pf);
	curl_easy_setopt(curl_handle,CURLOPT_VERBOSE,1);
	//MUST NOT USE ,or curl will wait for long..curl_easy_setopt(curl_handle,CURLOPT_INFILE,curldata->pFile);
	curl_easy_setopt(curl_handle,CURLOPT_HEADER,1);
	curl_easy_setopt(curl_handle,CURLOPT_HEADER,curldata->bHeadValid);

	curl_easy_setopt(curl_handle,CURLOPT_INFILESIZE,sum);

	curl_easy_perform(curl_handle);
	curl_easy_getinfo(curl_handle,CURLINFO_RESPONSE_CODE,&(curldata->nCurlCode));
	curl_slist_free_all(headers);
	curl_easy_cleanup(curl_handle);

	fclose(pf);
	
	return curldata->nCurlCode;
}

int curlapi_copy_object_data(char * token,char * url,char * dest_container,char * dest_obj,char * source_container,char * source_obj,Curl_Data * curldata)
{
	curl_clear_data(curldata);
	char dest[1024];
	char source[1024];
	sprintf(dest,"%s/%s/%s",url,dest_container,dest_obj);
	sprintf(source,"X-Copy-From: %s/%s",source_container,source_obj);
	//curldata->nFileLength = 0;
	
	struct curl_slist * headers = NULL;
	CURL *curl_handle = curl_easy_init();
	curl_easy_reset(curl_handle);
	curl_easy_setopt(curl_handle,CURLOPT_URL,dest);
	curl_easy_setopt(curl_handle,CURLOPT_UPLOAD,1);

	headers = curl_slist_append(headers,token);
	headers = curl_slist_append(headers,"Expect:");
	//headers = curl_slist_append(headers,"Content-Type:.application/x-www-form-urlencoded");
	headers = curl_slist_append(headers,source);
	headers = curl_slist_append(headers,"Connection: close");
	curl_easy_setopt(curl_handle,CURLOPT_HTTPHEADER,headers);
	curl_easy_setopt(curl_handle,CURLOPT_READFUNCTION,&upload_file_to_swift);
	curl_easy_setopt(curl_handle,CURLOPT_READDATA,curldata);
	curl_easy_setopt(curl_handle,CURLOPT_HEADER,1);
	curl_easy_setopt(curl_handle,CURLOPT_HEADER,curldata->bHeadValid);

	curl_easy_setopt(curl_handle,CURLOPT_INFILESIZE,0); 

	curl_easy_perform(curl_handle);
	curl_easy_getinfo(curl_handle,CURLINFO_RESPONSE_CODE,&(curldata->nCurlCode));
	curl_slist_free_all(headers);
	curl_easy_cleanup(curl_handle);
	

	//printf("\nerror=%d\n",curldata->nCurlCode);

	return curldata->nCurlCode;
}

int curlapi_delete_object_data(char * token,char * url,char * container,char * object,Curl_Data * curldata)
{
	curl_clear_data(curldata);
	char uri[1024];
	
	sprintf(uri,"%s/%s/%s",url,container,object);
	//curldata->nFileLength = 0;
	
	struct curl_slist * headers = NULL;
	CURL *curl_handle = curl_easy_init();
	curl_easy_reset(curl_handle);
	curl_easy_setopt(curl_handle,CURLOPT_URL,uri);
	curl_easy_setopt(curl_handle,CURLOPT_CUSTOMREQUEST,"DELETE");
	//curl_easy_setopt(curl_handle,CURLOPT_HEADER,1);
	curl_easy_setopt(curl_handle,CURLOPT_HEADER,curldata->bHeadValid);

	headers = curl_slist_append(headers,token);
	//headers = curl_slist_append(headers,"Expect:");
	//headers = curl_slist_append(headers,"Connection: close");
	curl_easy_setopt(curl_handle,CURLOPT_HTTPHEADER,headers);
	
	curl_easy_perform(curl_handle);
	curl_easy_getinfo(curl_handle,CURLINFO_RESPONSE_CODE,&(curldata->nCurlCode));
	curl_slist_free_all(headers);
	curl_easy_cleanup(curl_handle);

	return curldata->nCurlCode;
}

int curlapi_get_object_metadata(char * token,char * url,char * container,char * object,Curl_Data * curldata)
{
	curl_clear_data(curldata);
	char uri[1024];	
	sprintf(uri,"%s/%s/%s",url,container,object);
	//printf("curlapi_get_object_metadata:%s\n",uri);
	struct curl_slist * headers = NULL;
	CURL *curl_handle = curl_easy_init();
	curl_easy_reset(curl_handle);
	curl_easy_setopt(curl_handle,CURLOPT_URL,uri);
	// can receive the header:
	curl_easy_setopt(curl_handle,CURLOPT_HEADER,1);
	curl_easy_setopt(curl_handle,CURLOPT_HEADER,curldata->bHeadValid);
	curl_easy_setopt(curl_handle,CURLOPT_CUSTOMREQUEST,"HEAD");
	curl_easy_setopt(curl_handle,CURLOPT_NOBODY,1);
	
	headers = curl_slist_append(headers,token);
	curl_easy_setopt(curl_handle,CURLOPT_HTTPHEADER,headers);
	curl_easy_setopt(curl_handle,CURLOPT_WRITEFUNCTION,&downloadsection);
	curl_easy_setopt(curl_handle,CURLOPT_WRITEDATA,curldata);

	curl_easy_perform(curl_handle);
	curl_easy_getinfo(curl_handle,CURLINFO_RESPONSE_CODE,&(curldata->nCurlCode));
	curl_slist_free_all(headers);
	curl_easy_cleanup(curl_handle);
	
	return curldata->nCurlCode;
}

int curlapi_update_object_metadata(char * token,char * url,char * container,char * object,Curl_Data * curldata)
{
	curl_clear_data(curldata);
	char uri[1024];
	sprintf(uri,"%s/%s/%s",url,container,object);	
	
	struct curl_slist * headers = NULL;
	CURL *curl_handle = curl_easy_init();
	curl_easy_reset(curl_handle);
	// set uri
	curl_easy_setopt(curl_handle,CURLOPT_URL,uri);
	// set POST
	curl_easy_setopt(curl_handle,CURLOPT_POST,1);	
	curl_easy_setopt(curl_handle,CURLOPT_HEADER,curldata->bHeadValid);
	// set header:
	headers = curl_slist_append(headers,token);
	headers = curl_slist_append(headers,"Content-Length: 0");
	MemoryBuffer * mem = fifo_pop_node(curldata->liWrite);
	while(mem)
	{
		if(mem->nSize==2)
		{
			//find '\r\n' then the liWrite data is end,PAY ATTENTION:DO NOT add the headers,or error.
			//memory_delete_data(mem);
			break;
		}
		headers = curl_slist_append(headers,mem->lpBuffer);
		//memory_delete_data(mem);
		mem = fifo_pop_node(curldata->liWrite);
	}
	curl_easy_setopt(curl_handle,CURLOPT_HTTPHEADER,headers);

	curl_easy_perform(curl_handle);
	curl_easy_getinfo(curl_handle,CURLINFO_RESPONSE_CODE,&(curldata->nCurlCode));
	curl_slist_free_all(headers);
	curl_easy_cleanup(curl_handle);
		
	return curldata->nCurlCode;
}
int curlapi_delete_object_metadata(char * token,char * url,char * container,char * object,Curl_Data * curldata)
{
	return curlapi_update_object_metadata(token,url,container,object,curldata);
}
void test_downloadsection()
{
    MemoryBuffer mem; 
    CURL *curl_handle = curl_easy_init();
    curl_easy_reset(curl_handle);
    curl_easy_setopt(curl_handle,CURLOPT_URL,"http://www.baidu.com");
    curl_easy_setopt(curl_handle,CURLOPT_WRITEFUNCTION,&downloadsection);
    curl_easy_setopt(curl_handle,CURLOPT_WRITEDATA,&mem);
    curl_easy_perform(curl_handle);
    curl_easy_cleanup(curl_handle);
 
    //printf("%s\n",mem.lpBuffer);		
}




