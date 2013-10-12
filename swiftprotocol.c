#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sufs.h"
#include "swiftprotocol.h"

int __get_http_code(char * buf,char * info);
int __get_user_directory(char *buffer,User_Directory * ud);
int __reset_user_dir(User_Directory * ud);
int __get_object_type(char * obj);
void __safe_memcpy(char * dest,char * source,int size);

/* Auth functions:*/
Swift_Auth_Data * swift_new_auth()
{
	Swift_Auth_Data * auth = (Swift_Auth_Data*)malloc(sizeof(Swift_Auth_Data));
	if(auth==NULL)
		return NULL;
	memset(auth,0,sizeof(Swift_Auth_Data));
	return auth;
}
void swift_delete_auth(Swift_Auth_Data * auth)
{
	if(auth)
		free(auth);
	auth = NULL;
}
void swift_print_auth(Swift_Auth_Data * auth)
{
	if(auth)
	{
		if(auth->bAuth)
			printf("regist OK.\n");
		else
			printf("No registry.\n");
		printf("%s\n",auth->strUser);
		printf("%s\n",auth->strPassword);	
		printf("%s\n",auth->strStorageToken);	
		printf("%s\n",auth->strAuthToken);	
		printf("%s\n",auth->strAuthUrl);	
		printf("%s\n",auth->strUrl);
	}
	else
		printf("No auth data.\n");
}
void swift_auth_init(char * user,char *passwd,char * host,Swift_Auth_Data * auth)
{
	sprintf(auth->strUser,"X-storage-user: %s",user);
	sprintf(auth->strPassword,"X-storage-pass: %s",passwd);
	sprintf(auth->strAuthUrl,"http://%s:8080/auth/v1.0",host);
	auth->bAuth = 0;
	auth->strStorageToken[0] = '\0';
	auth->strAuthToken[0] = '\0';
	auth->strUrl[0] = '\0';
}
int swiftapi_get_auth(FIFO * li,Swift_Auth_Data * auth)
{
	MemoryBuffer * mem = fifo_get_head(li);
	char info[256];
	int code = __get_http_code(mem->lpBuffer,info);
	
	if(code>=300)
	{
		plog(mem->lpBuffer);		
		return -4;
	}
	else
	{
		char * p;				
		while(mem)
		{
			mem = fifo_get_next(li);
			if(mem->nSize==2)
				break;			
			//MUST DELETE the charactors "\r\n" which will stop the http headers and return error 411.
			p = strstr(mem->lpBuffer,"X-Auth-Token");
			if(p)
			{
				memcpy(auth->strAuthToken,mem->lpBuffer,mem->nSize-2);
				auth->bAuth = 1;
			}
			p = strstr(mem->lpBuffer,"X-Storage-Token");
			if(p)
			{
				memcpy(auth->strStorageToken,mem->lpBuffer,mem->nSize-2);
			}
			p = strstr(mem->lpBuffer,"X-Storage-Url:");
			if(p)
			{
				memcpy(auth->strUrl,mem->lpBuffer+15,mem->nSize-17);//delete \r\n
			}
		}
	}
		
	return 0;
}

/* Account functions:* /
Swift_Account_Metadata * swift_new_account_metadata()
{
	Swift_Account_Metadata * metadata = (Swift_Account_Metadata*)malloc(sizeof(Swift_Account_Metadata));
	if(metadata==NULL)
		return NULL;
	memset(metadata,0,sizeof(Swift_Account_Metadata));

	return metadata;
}
void swift_delete_account_metadata(Swift_Account_Metadata * metadata)
{
	if(metadata)
		free(metadata);
	metadata = NULL;
}
int swift_get_account_data(FIFO * li,Swift_Account_Metadata * metadata)
{
	//the headers is end.
	if(metadata->pDetail==NULL)
	{
		metadata->pDetail = (Swift_Account_List_Detail*)malloc(sizeof(Swift_Account_List_Detail)*metadata->nAccountContainerCount);
		if(metadata->pDetail==NULL)
			return -1;
		memset(metadata->pDetail,0,sizeof(Swift_Account_List_Detail)*metadata->nAccountContainerCount);
		metadata->nContainterLoc = 0;
	}
	
	MemoryBuffer * mem = fifo_get_next(li);
	if(mem)
	{
		char * p = strtok(mem->lpBuffer,"\r\n");
		while(p)
		{
			if(metadata->nContainerLoc==metadata->nAccountContainerCount)
				break;
			__safe_memcpy(metadata->pDetail[metadata->nContainerLoc]->strName,p,strlen(p));
			metadata->nContainerLoc = metadata->nContainerLoc+1;
			p = strtok(NULL,"\r\n");		
		}
	}
	
	return metadata->nContainerLoc;
}
int swift_get_account_metadata(FIFO * li,Swift_Account_Metadata * metadata)
{
	MemoryBuffer * mem = fifo_get_head(li);
	if(mem==NULL)
	{
		return -2;
	}
	char info[256];
	int code = __get_http_code(mem->lpBuffer,info);
	
	if(code>=300)
	{
		//This is an error,reset the user dir.
		//__reset_user_dir(swiftdata->pUserDirectory);
		//memory_print_data(li);
		return -3;
	}		
	char *p;	
	while(mem)
	{
		mem = fifo_get_next(li);
		if(mem->nSize==2)
			break;
		p = strstr(mem->lpBuffer,"X-Account-Object-Count:");
		if(p)
		{
			sscanf(mem->lpBuffer,"%*s%d",&(metadata->nAccountObjectCount));
			continue;
		}
		p = strstr(mem->lpBuffer,"X-Account-Bytes-Used:");
		if(p)
		{
			sscanf(mem->lpBuffer,"%*s%ld",&(metadata->lAccountBytesUsed));
			continue;
		}
		p = strstr(mem->lpBuffer,"X-Account-Container-Count:");
		if(p)
		{
			sscanf(mem->lpBuffer,"%*s%d",&(metadata->nAccountContainerCount));
			continue;
		}
		p = strstr(mem->lpBuffer,"Content-Length:");
		if(p)
		{
			sscanf(mem->lpBuffer,"%*s%d",&(metadata->nContentLength));
			continue;
		}	
	}
		
	return 0;
}
*/
/*Container functions*/
Swift_Container_Metadata * swift_new_container_metadata()
{
	Swift_Container_Metadata * data = (Swift_Container_Metadata*)malloc(sizeof(Swift_Container_Metadata));
	if(data==NULL)
		return NULL;
	memset(data,0,sizeof(Swift_Container_Metadata));
	
	return data;
}
void swift_delete_container_metadata(Swift_Container_Metadata * data)
{
	if(data)
		free(data);
	data = NULL;
}
int swift_get_container_data(FIFO * li,FIFO* liObjects)
{
	MemoryBuffer * mem = fifo_get_head(li);
	char * p;
	int ret = 0;
	int len;
	while(mem)
	{
		p = strtok(mem->lpBuffer,"\n");
		while(p)
		{
			len = strlen(p);
			MemoryBuffer * obj = memory_new_data(len);
			if(obj)
			{
				__safe_memcpy(obj->lpBuffer,p,len);
				fifo_push_node(obj,liObjects);
			}
			else
				return -1;
			
			p = strtok(NULL,"\n");	
			ret = ret+1;
		}	
		mem = fifo_get_next(li);
	}
	return ret;	
}
/*
int swift_get_container_detail(FIFO * li,Swift_Interface_Data * swiftdata)
{
	int ret = swift_get_container_metadata(li,swiftdata);	
	if(ret==0)
	{		
		//the headers is end.Do with a list of detail for json or xml.
		// Do with json NO CODE.
	}
	return ret;
}
int swift_create_container(FIFO * li,Swift_Interface_Data * swiftdata)
{
	MemoryBuffer * mem = fifo_pop_node(li);
	if(mem==NULL)
	{
		return -2;
	}
	char info[256];
	int code = __get_http_code(mem->lpBuffer,info);
	
	if(code>=300)
	{
		//This is an error,reset the user dir.
		//__reset_user_dir(swiftdata->pUserDirectory);
		//memory_print_data(li);	
		return -3;
	}		
	
	return 0;
}
int swift_delete_container(FIFO * li,Swift_Interface_Data * swiftdata)
{
	memory_print_data(li);
	fifo_release_data(li);	
	return 0;
}*/
int swift_get_container_metadata(FIFO * li,Swift_Container_Metadata * metadata)
{
	MemoryBuffer * mem = fifo_get_head(li);
	if(mem==NULL)
	{
		return -2;
	}
	char info[256];
	int code = __get_http_code(mem->lpBuffer,info);
	
	if(code>=300)
	{
		//This is an error,reset the user dir.		
		return -3;
	}		
	char *p;
	while(mem)
	{
		mem = fifo_get_next(li);
		if(mem->nSize==2)
			break;
		p = strstr(mem->lpBuffer,"X-Container-Object-Count:");
		if(p)
		{
			sscanf(mem->lpBuffer,"%*s%d",&(metadata->nContainerObjectCount));
			continue;
		}
		p = strstr(mem->lpBuffer,"Content-Length:");
		if(p)
		{
			sscanf(mem->lpBuffer,"%*s%d",&(metadata->nContentLength));
			continue;
		}	
		p = strstr(mem->lpBuffer,"Date:");
		if(p)
		{
			//sscanf(mem->lpBuffer,"%*s%d",&(swiftdata->pContainerMetadata->tMakeTime));
			struct tm tv;
			metadata->tMakeTime = gettimefromstring(mem->lpBuffer+6,&tv);
			continue;
		}	
		p = strstr(mem->lpBuffer,"X-Container-Bytes-Used:");
		if(p)
		{
			sscanf(mem->lpBuffer,"%*s%ld",&(metadata->lContainerBytesUsed));
			continue;
		}
		p = strstr(mem->lpBuffer,"X-Container-Read:");
		if(p)
		{
			//sscanf(mem->lpBuffer,"%*s%d",&(swiftdata->pContainerMetadata->bContainerRead));
			metadata->bContainerRead = 1;
			continue;
		}
		/*p = strstr(mem->lpBuffer,"X-Container-Meta-Web-Index:");
		if(p)
		{
			sscanf(mem->lpBuffer,"%*s%d",&(metadata->strContainerMetaWebIndex));
			continue;
		}*/	
	}	
	
	return 0;
}
/*
int swift_create_container_metadata(FIFO * li,Swift_Interface_Data * swiftdata)
{		
	memory_print_data(li);
	fifo_release_data(li);	
	return 0;
}
int swift_delete_container_metadata(FIFO * li,Swift_Interface_Data * swiftdata)
{
	memory_print_data(li);
	fifo_release_data(li);	
	return 0;
}





//memory APIs:
Swift_Interface_Data * swift_new_data()
{
	Swift_Interface_Data * swiftdata = (Swift_Interface_Data*)malloc(sizeof(Swift_Interface_Data));
	if(swiftdata==NULL)
		return NULL;
	memset(swiftdata,0,sizeof(Swift_Interface_Data));
	int bError = 0;
	
	swiftdata->pAuthData = (Swift_Auth_Data*)malloc(sizeof(Swift_Auth_Data));
	if(swiftdata->pAuthData==NULL)
	{
		bError = 1;
	}	
	swiftdata->pAccountMetadata = (Swift_Account_Metadata*)malloc(sizeof(Swift_Account_Metadata));
	if(swiftdata->pAccountMetadata==NULL)
	{
		bError = 1;
	}	
	swiftdata->pAccountListDetail = (Swift_Account_List_Detail*)malloc(sizeof(Swift_Account_List_Detail));
	if(swiftdata->pAccountListDetail==NULL)
	{
		bError = 1;
	}	
	swiftdata->pContainerMetadata = (Swift_Container_Metadata*)malloc(sizeof(Swift_Container_Metadata));
	if(swiftdata->pContainerMetadata==NULL)
	{
		bError = 1;
	}	
	swiftdata->pObjectMetadata = (Swift_Object_Metadata*)malloc(sizeof(Swift_Object_Metadata));
	if(swiftdata->pObjectMetadata==NULL)
	{
		bError = 1;
	}	
	swiftdata->pUserDirectory = (User_Directory*)malloc(sizeof(User_Directory));
	if(swiftdata->pUserDirectory==NULL)
	{
		bError = 1;
	}
	
	if(bError==1)
	{
		swift_delete_data(swiftdata);
		return NULL;
	}
	
	memset(swiftdata->pAuthData,0,sizeof(Swift_Auth_Data));
	memset(swiftdata->pAccountMetadata,0,sizeof(Swift_Account_Metadata));
	memset(swiftdata->pAccountListDetail,0,sizeof(Swift_Account_List_Detail));
	memset(swiftdata->pContainerMetadata,0,sizeof(Swift_Container_Metadata));
	memset(swiftdata->pObjectMetadata,0,sizeof(Swift_Object_Metadata));
	memset(swiftdata->pUserDirectory,0,sizeof(User_Directory));
	
	swift_init_data(swiftdata);
	
	return swiftdata;
}
void swift_delete_data(Swift_Interface_Data *swiftdata)
{
	if(swiftdata->pAuthData)
		free(swiftdata->pAuthData);
	if(swiftdata->pAccountMetadata)
		free(swiftdata->pAccountMetadata);
	if(swiftdata->pAccountListDetail)
		free(swiftdata->pAccountListDetail);
	if(swiftdata->pContainerMetadata)
		free(swiftdata->pContainerMetadata);
	if(swiftdata->pObjectMetadata)
		free(swiftdata->pObjectMetadata);
	if(swiftdata->pUserDirectory)
		free(swiftdata->pUserDirectory);
	swiftdata->pAuthData = NULL;
	swiftdata->pAccountMetadata = NULL;
	swiftdata->pAccountListDetail = NULL;
	swiftdata->pContainerMetadata = NULL;
	swiftdata->pObjectMetadata = NULL;
	swiftdata->pUserDirectory = NULL;
	
	if(swiftdata)
		free(swiftdata);
	swiftdata = NULL;
}
void swift_init_data(Swift_Interface_Data *swiftdata)
{
	swiftdata->pUserDirectory->nLimit = 50;
	//swiftdata-.pAuthData->bAuth = 0;	
}
void swift_print_data(Swift_Interface_Data *swiftdata)
{
	if(swiftdata->pAuthData->bAuth==0)
	{
		printf("auth failed.\n");
		return;
	}
	Swift_Account_Metadata *am = NULL;
	//Swift_Account_List_Detail * ald = NULL;
	Swift_Container_Metadata * cm = NULL;
	Swift_Object_Metadata * om = NULL;
	User_Directory * ud = NULL;
	
	am = swiftdata->pAccountMetadata;	
	cm = swiftdata->pContainerMetadata;
	om = swiftdata->pObjectMetadata;
	ud = swiftdata->pUserDirectory;	
	
	if(am)
	{
		printf("[account metadata]object=%d,container=%d,length=%d,bytes=%ld\n",am->nAccountObjectCount,am->nAccountContainerCount,am->nContentLength,am->lAccountBytesUsed);
	}
	if(cm)
	{
		printf("[container metadata]object=%d,bytes=%ld,length=%d,tm=%ld\n",cm->nContainerObjectCount,cm->lContainerBytesUsed,cm->nContentLength,(long)cm->tMakeTime);
	}
	if(om)
	{
		printf("[object metadata]length=%d,tlast=%ld,mtime=%ld\n",om->nContentLength,om->tLastModified,om->tMakeTime);
	}
	if(ud)
	{
		printf("dir=%s\n",ud->strCurrentDir);
		int i;
		for(i=0;i<ud->nValidCount;i++)
		{
			printf("[loc=%d]%s\n",i,ud->strFiles[i]);		
		}	
	}	
}
*/





/*Object functions*/
/*
int swift_get_object_data(FIFO * li,FIFO * liObject)
{
	
	int ret = swift_get_object_metadata(li,swiftdata);	
	if(ret==0)
	{		
		//the data is not done with in the function...
	}
	return ret;	
}*/

Swift_Object_Metadata * swift_new_object_metadata()
{
	Swift_Object_Metadata * metadata = (Swift_Object_Metadata*)malloc(sizeof(Swift_Object_Metadata));
	if(metadata==NULL)
		return NULL;
	memset(metadata,0,sizeof(Swift_Object_Metadata));
	
	return metadata;
}
void swift_delete_object_metadata(Swift_Object_Metadata * metadata)
{
	if(metadata)
		free(metadata);
	metadata = NULL;
}
int swift_get_object_metadata(FIFO * li,Swift_Object_Metadata * metadata)
{
	MemoryBuffer * mem = fifo_get_head(li);
	if(mem==NULL)
	{
		return -2;
	}
	char info[256];
	int code = __get_http_code(mem->lpBuffer,info);
	
	if(code>=300)
	{
		//This is an error,reset the user dir.		
		return -3;
	}		
	
	struct tm tv;
	char *p;
	while(mem)
	{
		mem = fifo_get_next(li);
		if(mem->nSize==2)
			break;
		p = strstr(mem->lpBuffer,"Last-Modified:");
		if(p)
		{
			//sscanf(mem->lpBuffer,"%*s%d",&(swiftdata->pObjectMetadata->tLastModified));
			metadata->tLastModified = gettimefromstring(mem->lpBuffer+15,&tv);
			continue;
		}
		p = strstr(mem->lpBuffer,"Content-Length:");
		if(p)
		{
			sscanf(mem->lpBuffer,"%*s%d",&(metadata->nContentLength));
			continue;
		}	
		p = strstr(mem->lpBuffer,"Date:");
		if(p)
		{
			metadata->tMakeTime = gettimefromstring(mem->lpBuffer+6,&tv);
			continue;
		}	

		/*
		p = strstr(mem->lpBuffer,"Etag:");
		if(p)
		{
			sscanf(mem->lpBuffer,"%*s%s",&(metadata->strEtag));
			continue;
		}	
		p = strstr(mem->lpBuffer,"Content-Type:");
		if(p)
		{
			sscanf(mem->lpBuffer,"%*s%s",&(metadata->strContentType));
			continue;
		}	
		*/		
	}	

	return 0;
}

/*inner used functions:*/
int __get_http_code(char * buf,char * info)
{
	//HTTP/1.1 200 OK
	//HTTP/1.1 204 No Content
	//HTTP/1.1 401 Unauthorized
	//
	int ret;
	sscanf(buf,"%*s %d %s",&ret,info);
	
	return ret;
}
int __get_user_directory(char *buffer,User_Directory * ud)
{
	ud->nValidCount = 0;
	char * p = strtok(buffer,"\r\n");
	
	while(p)
	{
		if(ud->nValidCount>=ud->nLimit)
			break;
		__safe_memcpy(ud->strFiles[ud->nValidCount],p,strlen(p));
		ud->nValidCount++;
		p = strtok(NULL,"\r\n");
	}
	
	return ud->nValidCount;
}
int __reset_user_dir(User_Directory * ud)
{
	int i;
	ud->strCurrentDir[0] = '/';
	ud->strCurrentDir[1] = '\0';
	for(i=0;i<50;i++)
		ud->strFiles[i][0] = '\0';
	ud->nValidCount = 0;
	ud->nLimit = 50;
	ud->strMarker[0] = '\0';
	ud->strEndMarker[0] = '\0';
	ud->strPrefix[0] = '\0';
	ud->strDelimiter[0] = '\0';
	
	return 0;
}
int __get_object_type(char * obj)
{
	int size = sizeof(obj);
	if(size<=0)
		return SWIFT_OBJECT_TYPE_IFERROR;
	if(obj[size-1]=='/')
		return SWIFT_OBJECT_TYPE_IFDIR;
	else
		return SWIFT_OBJECT_TYPE_IFFILE;
}
void __safe_memcpy(char * dest,char * source,int size)
{
	if(size>0)
		memcpy(dest,source,size);
	dest[size] = '\0';
}




