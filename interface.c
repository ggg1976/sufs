#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#include "sufs.h"
#include "curlapi.h"
#include "swiftprotocol.h"
#include "interface.h"

void getlasterror(int ret)
{
	char buf[32];
	sprintf(buf,"[%d,%d]\n",ret,errno);
	plog(buf);
}
// 测试使用中文
Interface_File_System_Data * g_pIFSData = NULL;

// Curl api wraps:
int wrap_get_container_data(int limit,char * marker,char * end_marker,char * prefix,char * delimiter)
{
	g_pIFSData->lpCurlData->bHeadValid = 0;
	return curlapi_get_container_data(g_pIFSData->lpAuthData->strAuthToken,g_pIFSData->lpAuthData->strUrl,g_pIFSData->strContainer,limit,marker,end_marker,prefix,delimiter,NULL,g_pIFSData->lpCurlData);
} 
int wrap_get_container_metadata()
{
	g_pIFSData->lpCurlData->bHeadValid = 1;
	return curlapi_get_container_metadata(g_pIFSData->lpAuthData->strAuthToken,g_pIFSData->lpAuthData->strUrl,g_pIFSData->strContainer,g_pIFSData->lpCurlData);
}
int wrap_create_container()
{
	g_pIFSData->lpCurlData->bHeadValid = 0;
	return curlapi_create_container(g_pIFSData->lpAuthData->strAuthToken,g_pIFSData->lpAuthData->strUrl,g_pIFSData->strContainer,g_pIFSData->lpCurlData);
}
int wrap_get_object_data(char * object)
{
	g_pIFSData->lpCurlData->bHeadValid = 0;
	return curlapi_get_object_data(g_pIFSData->lpAuthData->strAuthToken,g_pIFSData->lpAuthData->strUrl,g_pIFSData->strContainer,object,g_pIFSData->lpCurlData);
}
int wrap_create_object_null(char * object)
{
	g_pIFSData->lpCurlData->bHeadValid = 0;
	return curlapi_create_object_null(g_pIFSData->lpAuthData->strAuthToken,g_pIFSData->lpAuthData->strUrl,g_pIFSData->strContainer,object,g_pIFSData->lpCurlData);
}
int wrap_create_object_data(char * object)
{
	//use uploadsection function
	g_pIFSData->lpCurlData->bHeadValid = 0;
	return curlapi_create_object_data2(g_pIFSData->lpAuthData->strAuthToken,g_pIFSData->lpAuthData->strUrl,g_pIFSData->strContainer,object,g_pIFSData->lpCurlData);
}
int wrap_copy_object_data(char * dest_container,char * dest_obj,char * source_container,char * source_obj)
{
	g_pIFSData->lpCurlData->bHeadValid = 0;
	return curlapi_copy_object_data(g_pIFSData->lpAuthData->strAuthToken,g_pIFSData->lpAuthData->strUrl,dest_container,dest_obj,source_container,source_obj,g_pIFSData->lpCurlData);
}
int wrap_delete_object_data(char * object)
{
	g_pIFSData->lpCurlData->bHeadValid = 0;
	return curlapi_delete_object_data(g_pIFSData->lpAuthData->strAuthToken,g_pIFSData->lpAuthData->strUrl,g_pIFSData->strContainer,object,g_pIFSData->lpCurlData);
}
int wrap_get_object_metadata(char * object)
{
	g_pIFSData->lpCurlData->bHeadValid = 1;
	return curlapi_get_object_metadata(g_pIFSData->lpAuthData->strAuthToken,g_pIFSData->lpAuthData->strUrl,g_pIFSData->strContainer,object,g_pIFSData->lpCurlData);
}
int wrap_update_object_metadata(char * object)
{
	g_pIFSData->lpCurlData->bHeadValid = 1;
	return curlapi_update_object_metadata(g_pIFSData->lpAuthData->strAuthToken,g_pIFSData->lpAuthData->strUrl,g_pIFSData->strContainer,object,g_pIFSData->lpCurlData);
}
int wrap_delete_object_metadata(char * object)
{
	g_pIFSData->lpCurlData->bHeadValid = 0;
	return curlapi_delete_object_metadata(g_pIFSData->lpAuthData->strAuthToken,g_pIFSData->lpAuthData->strUrl,g_pIFSData->strContainer,object,g_pIFSData->lpCurlData);
}

/* file system interface tools functions:*/
void ifs_print_data(int bCurldata,int bAuthdata,int bContainer,int bObject,int bDir,int bFod)
{
	if(bCurldata)
	{
		curl_print_data(g_pIFSData->lpCurlData);
	}
	if(bAuthdata)
	{
		swift_print_auth(g_pIFSData->lpAuthData);
	}
	if(bContainer)
	{
	
	}
	if(bObject)
	{
	
	}
	if(bDir)
	{
		memory_print_data(g_pIFSData->lpCurrentDirectoryList);
	}
	if(bFod)
	{
		
	}
}
void safe_memcpy(char * dest,char * source,int size)
{
	if(size>0)
		memcpy(dest,source,size);
	dest[size] = '\0';
}
// file id operations:
int open_file_id()
{
	int i;
	for(i=0;i<1024;i++)
	{
		if(g_pIFSData->lpFileOptData[i]==NULL)
			break;
	}
	if(i>=1024)
		return -1;
	g_pIFSData->lpFileOptData[i] = (File_Operate_Data*)malloc(sizeof(File_Operate_Data));
	if(g_pIFSData->lpFileOptData[i]==NULL)
		return -1;
	memset(g_pIFSData->lpFileOptData[i],0,sizeof(File_Operate_Data));
	
	return i;
}
void close_file_id(int fd)
{
	if(g_pIFSData->lpFileOptData[fd])
		free(g_pIFSData->lpFileOptData[fd]);
	g_pIFSData->lpFileOptData[fd] = NULL;
}
int get_file_id(const char * pathname)
{
	int i;
	for(i=0;i<1024;i++)
	{
		if(g_pIFSData->lpFileOptData[i]==NULL)
			continue;
		if(strcmp(g_pIFSData->lpFileOptData[i]->strPath,pathname)==0)
			break;	
	}
	if(i==1024)
		return -1;
	return i;
}
// create an object with the zero length.
int create_null_object(char * path)
{
	int ret = wrap_create_object_null(path);
	if(ret<300)
		return 0;
	
	return 0;
}
// create an "." object instead of create dir.
int make_dir(char * dirname)
{
	char path[1024];
	int ret = strlen(dirname);
	if(dirname[ret-1]=='/')
		sprintf(path,"%s.",dirname);
	else
		sprintf(path,"%s/.",dirname);
	
	ret = wrap_get_object_metadata(path);
	if(ret==404)
	{
		//找不到对象：
		create_null_object(path);
	}

	return 0;
}
int rename_object(const char * source,const char * dest)
{
	int ret = wrap_get_object_metadata((char*)source);
	if(ret==404)
		return -12;
	if(ret>300)
		return -ret;
		
	ret = wrap_copy_object_data(g_pIFSData->strContainer,(char*)dest,g_pIFSData->strContainer,(char*)source);
	ret = wrap_delete_object_data((char*)source);

	return 0;
}
// remove the dir and all objects in the dir.
int remove_dir(char * dir)
{
	int limit = 50;
	char * tmppath = (char*)malloc(limit*1024);
	int count;
	int ret;
	FIFO * objList = fifo_new_list();
	if(objList==NULL)
		return -1;
	ret = wrap_get_container_data(limit,NULL,NULL,dir,"/");
	count = swift_get_container_data(g_pIFSData->lpCurlData->liRead,objList);
	if(count==-1)
	{
		fifo_delete_list(objList);
		return -1;
	}
	
	//delete:
	MemoryBuffer * mem;
	//char path[1024];
	int len;
	
	mem = fifo_get_head(objList);
	while(mem)
	{
		//sprintf(path,"%s%s",dir,mem->lpBuffer);
		if(mem->lpBuffer[mem->nSize-1]=='/')
		{
			ret = remove_dir(mem->lpBuffer);
			if(ret<0)
			{
				fifo_delete_list(objList);
				return -1;
			}
		}
		else
		{
			ret = ifs_api_unlink(mem->lpBuffer);
			if(ret<0)
			{
				fifo_delete_list(objList);
				return ret;
			}
		}	
		mem = fifo_get_next(objList);
	}
	fifo_release_data(objList);
		
	while(count==limit)
	{
		ret = wrap_get_container_data(limit,NULL,NULL,dir,"/");
		count = swift_get_container_data(g_pIFSData->lpCurlData->liRead,objList);
		if(count==-1)
		{
			fifo_delete_list(objList);
			return -1;
		}
		mem = fifo_get_head(objList);
		while(mem)
		{
			//sprintf(path,"%s%s",dir,mem->lpBuffer);
			if(mem->lpBuffer[mem->nSize-1]=='/')
			{
				ret = remove_dir(mem->lpBuffer);
				if(ret<0)
				{
					fifo_delete_list(objList);
					return -1;
				}
			}
			else
			{
				ret = ifs_api_unlink(mem->lpBuffer);
				if(ret<0)
				{
					fifo_delete_list(objList);
					return ret;
				}
			}	
			mem = fifo_get_next(objList);
		}
		fifo_release_data(objList);
	}
	fifo_delete_list(objList);
	
	return 0;
}
int get_file_stat(char * path,struct stat *stbuf)
{
	int ret = wrap_get_object_metadata(path);
	if(ret==200)
	{
		ret = swift_get_object_metadata(g_pIFSData->lpCurlData->liRead,g_pIFSData->lpObjectMetadata);
		if(ret!=0)
		{
			slog("do with the file metadata error:%d",ret);
			ret = -1;
		}
		stbuf->st_mode = S_IFREG|0777;
		stbuf->st_ino = 0;
		stbuf->st_dev = 0;
		stbuf->st_rdev = 0;
		stbuf->st_nlink = 1;
		stbuf->st_uid = SUFS_USER_ID;
		stbuf->st_gid = SUFS_GROUP_ID;
		stbuf->st_size = g_pIFSData->lpObjectMetadata->nContentLength;
		stbuf->st_atime = time(0);//tLastModified
		stbuf->st_mtime = stbuf->st_atime;//tMakeTime
		stbuf->st_ctime = stbuf->st_atime;
		stbuf->st_blksize = 0;
		stbuf->st_blocks = 0;	
		
		ret = 0;
	}
	else
	{
		slog("get file metadata error:%d",ret);
		ret = -1;
	}		
	return ret;
}
/* Interface APIs:*/
// memory functions:
int ifs_new_data()
{
	g_pIFSData = (Interface_File_System_Data*)malloc(sizeof(Interface_File_System_Data));
	if(g_pIFSData)
	{
		g_pIFSData->lpCurlData = curl_new_data();
		if(g_pIFSData->lpCurlData==NULL)
			return -1;
		g_pIFSData->lpAuthData = swift_new_auth();
		if(g_pIFSData->lpAuthData==NULL)
			return -1;
		g_pIFSData->lpContainerMetadata = swift_new_container_metadata();
		if(g_pIFSData->lpContainerMetadata==NULL)
			return -1;
			
		g_pIFSData->lpObjectMetadata = swift_new_object_metadata();
		if(g_pIFSData->lpObjectMetadata==NULL)
			return -1;
	}
	else
		return -1;

	return 0;
}
void ifs_delete_data()
{
	if(g_pIFSData)
	{
		if(g_pIFSData->lpCurlData)
			curl_delete_data(g_pIFSData->lpCurlData);
		
		if(g_pIFSData->lpAuthData)
			swift_delete_auth(g_pIFSData->lpAuthData);
			
		if(g_pIFSData->lpContainerMetadata)
			swift_delete_container_metadata(g_pIFSData->lpContainerMetadata);
		
		if(g_pIFSData->lpObjectMetadata)
			swift_delete_object_metadata(g_pIFSData->lpObjectMetadata);
			
		free(g_pIFSData);
	}
}
// initalize functios:
void ifs_init_data(char * user,char *passwd,char * host,char * mount,char * container)
{
	swift_auth_init(user,passwd,host,g_pIFSData->lpAuthData);	
	strcpy(g_pIFSData->strMountPath,mount);
	strcpy(g_pIFSData->strContainer,container);
}
// auth function:
int ifs_get_auth()
{
	g_pIFSData->lpCurlData->bHeadValid = 1;
	int ret = curlapi_authv1(g_pIFSData->lpAuthData->strUser,g_pIFSData->lpAuthData->strPassword,g_pIFSData->lpAuthData->strAuthUrl,g_pIFSData->lpCurlData);
	
	if(ret>300)
	{
		// get error curl code:
		return -4;
	}
	
	ret = swiftapi_get_auth(g_pIFSData->lpCurlData->liRead,g_pIFSData->lpAuthData);
	if(g_pIFSData->lpAuthData->bAuth!=1)
	{
		return -13;
	}
	
	ret = wrap_get_container_metadata();
	if(ret>300)
	{
		//ret=401 auth error,
		if(ret==401)
			return -4;
		else if(ret==404)
		{
			//No container named with g_pIFSData->strContainer
			ret = wrap_create_container();
			if(ret>300)
			{
				return -ret;
			}			
		}
		else
			return -ret;
	}
	else
	{
		ret = swift_get_container_metadata(g_pIFSData->lpCurlData->liRead,g_pIFSData->lpContainerMetadata);
		if(ret<0)
			printf("get container metadata failed.\n");
		else
		{
			//printf("ob count=%d,bytes=%ld,mktime=%ld\n",g_pIFSData->lpContainerMetadata->nContainerObjectCount,g_pIFSData->lpContainerMetadata->lContainerBytesUsed,g_pIFSData->lpContainerMetadata->tMakeTime);
		}
	}
	//init success..
	g_pIFSData->strCurrentDir[0] = '/';
	g_pIFSData->strCurrentDir[1] = '\0';
	errno = 0;
	
	return 0;
}

/* fuse_operations APIs:*/
int ifs_api_getattr(const char *pathname, struct stat *stbuf)
{
	/*
		/
		/curl
		/test/1.txt
		
		[2013-09-03 09:56:19] readdir:path=/
		[2013-09-03 09:56:19] getattrib:path=/a
		[2013-09-03 09:56:19] size=2,len=2,a/
		[2013-09-03 09:56:19] mode:16895,err:0
		[2013-09-03 09:56:19] getattrib:path=/curl
		[2013-09-03 09:56:19] size=5,len=5,curl/
		[2013-09-03 09:56:19] mode:16895,err:0
		[2013-09-03 09:56:19] getattrib:path=/test
		[2013-09-03 09:56:19] size=5,len=5,test/
		[2013-09-03 09:56:19] mode:16895,err:0
		[2013-09-03 09:56:19] getattrib:path=/test1.txt
		[2013-09-03 09:56:19] size=9,len=10,test1.txt
		[2013-09-03 09:56:19] ret:200,err:2
		[2013-09-03 09:56:26] getattrib:path=/

	*/
	slog("getattrib:path=%s",(char*)pathname);
	
	char path[1024];
	int len = strlen(pathname); 
	safe_memcpy(path,(char*)pathname+1,len-1);
	
	if(len==1)
	{
		// 在根目录时需要这段代码把根目录去掉，否则如果根目录有文件，则出错。
		stbuf->st_mode = S_IFDIR|0777;
		stbuf->st_ino = 0;
		stbuf->st_dev = 0;
		stbuf->st_rdev = 0;
		stbuf->st_nlink = 1;
		stbuf->st_uid = SUFS_USER_ID;
		stbuf->st_gid = SUFS_GROUP_ID;
		stbuf->st_size = 4096;
		stbuf->st_atime = time(0);
		stbuf->st_mtime = stbuf->st_atime;
		stbuf->st_ctime = stbuf->st_atime;
		stbuf->st_blksize = 0;
		stbuf->st_blocks = 0;
		//slog("error:0");
		
		return 0;
	}
	
	int ret = wrap_get_container_data(0,NULL,NULL,path,"/");
	if(errno==115)
		errno = 0;
		
	if(ret==200)
	{
		// 不一定返回一行:这是以\n分隔的数据。
		FIFO * li = fifo_new_list();
		if(li==NULL)
		{
			errno = 105;
			slog("ret:%d,err:%d",ret,errno);
			
			return -errno;
		}
		if(swift_get_container_data(g_pIFSData->lpCurlData->liRead,li)>0)
		{
			MemoryBuffer * mem = fifo_get_head(li);			
			while(mem)
			{
				slog("size=%d,len=%d,%s",mem->nSize,len,mem->lpBuffer);
				if(mem->nSize==len)
				{
					if(mem->lpBuffer[mem->nSize-1]=='/')
					{
						stbuf->st_mode = S_IFDIR|0777;
						stbuf->st_ino = 0;
						stbuf->st_dev = 0;
						stbuf->st_rdev = 0;
						stbuf->st_nlink = 1;
						stbuf->st_uid = SUFS_USER_ID;
						stbuf->st_gid = SUFS_GROUP_ID;
						stbuf->st_size = 4096;
						stbuf->st_atime = time(0);
						stbuf->st_mtime = stbuf->st_atime;
						stbuf->st_ctime = stbuf->st_atime;
						stbuf->st_blksize = 0;
						stbuf->st_blocks = 0;					
						break;
					}
				}
				else if((mem->nSize+1)==len)
				{	
					// 获得文件的细节：文件长度为0可能是导致无法读取文件的原因,注意存在curlapi嵌套！！！
					// 在真正运行版本要改正这个问题。这里后面已经不再使用curlapi的数据了，所以没有问题。
					ret = get_file_stat(mem->lpBuffer,stbuf);
					/*
					ret = wrap_get_object_metadata(mem->lpBuffer);
					if(ret==200)
					{
						ret = swift_get_object_metadata(g_pIFSData->lpCurlData->liRead,g_pIFSData->lpObjectMetadata);
						if(ret!=0)
						{
							slog("do with the file metadata error:%d",ret);
						}
						stbuf->st_mode = S_IFREG|0777;
						stbuf->st_ino = 0;
						stbuf->st_dev = 0;
						stbuf->st_rdev = 0;
						stbuf->st_nlink = 1;
						stbuf->st_uid = 0;
						stbuf->st_gid = 0;
						stbuf->st_size = g_pIFSData->lpObjectMetadata->nContentLength;
						stbuf->st_atime = time(0);//tLastModified
						stbuf->st_mtime = stbuf->st_atime;//tMakeTime
						stbuf->st_ctime = stbuf->st_atime;
						stbuf->st_blksize = 0;
						stbuf->st_blocks = 0;							
					}
					else
					{
						slog("get file metadata error:%d",ret);
					}	
					*/
					break;					
				}
				mem = fifo_get_next(li);
			}
			fifo_delete_list(li);
			if(errno==115)
				errno = 0;
			if(stbuf->st_mode)
			{				
				//slog("mode:%d,err:%d",(int)stbuf->st_mode,errno);
				return 0;			
			}
			else
			{
				// 没有找到接收数据
				errno = 2;
				memset(stbuf,0,sizeof(stbuf));
				slog("ret:%d,err:%d",ret,errno);
				return -errno;
			}
		}
		else
		{
			//没有找到数据：
			errno = 14;
			fifo_delete_list(li);
			slog("ret:%d,err:%d",ret,errno);
			return -errno;		
		}
	}
	else if(ret==204)
	{
		errno = 2;
		memset(stbuf,0,sizeof(stbuf));
		slog("ret:%d,err:%d",ret,errno);
		return -errno;	
	}
	else
	{
		errno = 5;
		slog("ret:61,err:%d",errno);
		return -61;// No data available 
	}
}
int ifs_api_readdir(const char *pathname)
{
	// 对于pathname要先去掉第一个字节/,然后如果最后一个字节不是/，要手动增加上/，表示目录，这样就与swift协议相符了。
	slog("readdir:path=%s",(char*)pathname);
	
	char path[1024];
	int len = strlen(pathname)-1; 
	safe_memcpy(path,(char*)pathname+1,len);
	if(len>0)
	{
		if(path[len-1]!='/')
		{
			path[len] = '/';
			path[len+1] = '\0';
		}
	}

	int limit = 50;
	int ret = wrap_get_container_data(limit,NULL,NULL,path,"/");

	FIFO * objList = fifo_new_list();
	if(objList==NULL)
		return -1;
	ret = swift_get_container_data(g_pIFSData->lpCurlData->liRead,objList);
	if(ret==-1)
	{
		fifo_delete_list(objList);
		return -1;
	}
	int sum = ret;
	while(ret==limit)
	{
		MemoryBuffer * mem = fifo_get_last(objList);
		
		wrap_get_container_data(limit,mem->lpBuffer,NULL,path,"/");
	
		ret = swift_get_container_data(g_pIFSData->lpCurlData->liRead,objList);
		if(ret==-1)
		{
			fifo_delete_list(objList);
			return -1;
		}
		sum += ret;
	}
	
	//change current dir:
	safe_memcpy(g_pIFSData->strCurrentDir,path,strlen(path));
	if(g_pIFSData->lpCurrentDirectoryList)
	{
		fifo_delete_list(g_pIFSData->lpCurrentDirectoryList);
	}
	g_pIFSData->lpCurrentDirectoryList = objList;
	
	
	MemoryBuffer * lp = fifo_get_head(objList);
	while(lp)
	{
		sprintf(path,"[size:%d,loc:%d]--  %s",lp->nSize,lp->nLoc,lp->lpBuffer);
		plog(path);
		lp = fifo_get_next(objList);
	}	
	return sum;
}
int ifs_api_mkdir(const char *pathname)
{	
	slog("mkdir:path=%s",(char*)pathname);
	char path[1024];

	sprintf(path,"%s/.",(char*)pathname+1);	
	int ret = create_null_object(path);
	if(ret==201)
	{
	
	}
	else if(ret==422)
	{
	
	}
	else if(ret==411)
	{
	
	}
	
	return 0;
}
int ifs_api_unlink(const char *pathname)
{
	//注意：对于目录来说，得到的文件路径第一个字节不包含"/"，而普通文件的第一个字节包含"/".
	slog("unlink:path=%s",(char*)pathname);
	
	char path[1024];
	int len = strlen(pathname); 
	if(pathname[0]=='/')
	{
		//这是一个文件：
		safe_memcpy(path,(char*)pathname+1,len-1);
	}
	else
	{
		//这是一个目录：
		safe_memcpy(path,(char*)pathname,len);
	}
	
	int ret = wrap_delete_object_data(path);
	
	if(ret==204)
	{
	
	
	}
	else if(ret==404)
	{
	
	
	}
	
	return 0;
}
int ifs_api_rmdir(const char * pathname)
{
	slog("rmdir:path=%s",(char*)pathname);
	
	char path[1024];
	int len = strlen(pathname)-1; 
	safe_memcpy(path,(char*)pathname+1,len);
	
	if(len==0)
	{
		return remove_dir("");
	}	
	if(path[len-1]!='/')
	{
		path[len] = '/';
		path[len+1] = '\0';
	}
	
	return remove_dir(path);	
}
int ifs_api_rename(const char *oldpath, const char *newpath)
{
	slog("rename:old=%s,new=%s",(char*)oldpath,(char*)newpath);
	
	int len = strlen(oldpath);
	if(oldpath[len-1]=='/')
		return 0;
		
	len = strlen(newpath);
	if(newpath[len-1]=='/')
		return 0;

	return rename_object(newpath+1,oldpath+1);
}
void ifs_api_close(int fd)
{
	if(g_pIFSData->lpFileOptData[fd])
	{
		close_file_id(fd);
		g_pIFSData->lpFileOptData[fd] = NULL;	
	}
}
int ifs_api_open(const char * pathname,int flags)
{
	slog("open:path=%s,flags=%d",(char*)pathname,flags);
	
	char path[1024];
	int len = strlen(pathname)-1; 
	safe_memcpy(path,(char*)pathname+1,len);	
	
	if(pathname[len]=='/')
		return -1;

	int fd = open_file_id();
	if(fd==-1)
		return -1;

	//DO NOT use "O_RDONLY&flags",because O_RDONLY=0.
	if(O_RDONLY==flags)
	{
		int ret = wrap_get_object_metadata(path);	
		if(ret==404)
		{
			close_file_id(fd);
			return -14;
		}
		swift_get_object_metadata(g_pIFSData->lpCurlData->liRead,g_pIFSData->lpObjectMetadata);	
		g_pIFSData->lpFileOptData[fd]->nSize = g_pIFSData->lpObjectMetadata->nContentLength;
		g_pIFSData->lpFileOptData[fd]->bRead = 1;
	}
	else
	{
		g_pIFSData->lpFileOptData[fd]->bWrite = 1;
	}
	safe_memcpy(g_pIFSData->lpFileOptData[fd]->strPath,path,strlen(path));
	g_pIFSData->lpFileOptData[fd]->nCheck = 1111;
	
	return fd;
}

int ifs_api_read(const char *pathname, char *buf, size_t size)
{
	slog("read:path=%s",(char*)pathname);
	
	char path[1024];
	int len = strlen(pathname)-1; 
	safe_memcpy(path,(char*)pathname+1,len);	

	int fd = get_file_id(path);
	if(fd==-1)
	{
		fd = ifs_api_open(pathname,O_RDONLY);
		if(fd==-1)
			return 0;
	}
	
	if(g_pIFSData->lpFileOptData[fd]==NULL)
		return -1;
	if(g_pIFSData->lpFileOptData[fd]->nCheck!=1111)
		return -1;
	if(g_pIFSData->lpFileOptData[fd]->nSize==0)
	{
		// no data:
		return 0;	
	}
	slog("read fd=%d,loc=%d",fd,g_pIFSData->lpFileOptData[fd]->nCurlLoc);
	if(g_pIFSData->lpFileOptData[fd]->nCurlLoc==0)
	{
		wrap_get_object_data(g_pIFSData->lpFileOptData[fd]->strPath);
		g_pIFSData->lpFileOptData[fd]->nCurlLoc = memory_get_data_size(g_pIFSData->lpCurlData->liRead);
		if(g_pIFSData->lpFileOptData[fd]->nCurlLoc!=g_pIFSData->lpFileOptData[fd]->nSize)
		{
			plog("read length error\n");
			return -1;
		}
		g_pIFSData->lpFileOptData[fd]->pFifo = curl_pop_fifo_data(g_pIFSData->lpCurlData);
		if(g_pIFSData->lpFileOptData[fd]->pFifo==NULL)
		{
			return -1;
		}
	}
	int ret = memory_output_data((char*)buf,size,g_pIFSData->lpFileOptData[fd]->pFifo);
	if(errno==115)
		errno = 0;
	return ret;
}
int ifs_api_write(const char *pathname, const char *buf, size_t size)
{
	slog("write:path=%s,size=%d",(char*)pathname,(int)size);
	
	char path[1024];
	int len = strlen(pathname)-1; 
	safe_memcpy(path,(char*)pathname+1,len);

	int fd = get_file_id(path);
	if(fd==-1)
	{
		fd = ifs_api_open(pathname,O_WRONLY);
		getlasterror(fd);
		if(fd==-1)
			return 0;
	}
	if(g_pIFSData->lpFileOptData[fd]==NULL)
		return -1;
	if(g_pIFSData->lpFileOptData[fd]->nCheck!=1111)
		return -1;
	
	if(g_pIFSData->lpFileOptData[fd]->pFifo==NULL)
	{
		g_pIFSData->lpFileOptData[fd]->pFifo = fifo_new_list();
		if(g_pIFSData->lpFileOptData[fd]->pFifo==NULL)
			return -1;
	}
	fd = memory_input_data(buf,size,g_pIFSData->lpFileOptData[fd]->pFifo);
	if(errno==115)
		errno = 0;
	return fd;
}
int ifs_api_release(const char *pathname)
{
	slog("release:path=%s",(char*)pathname);
	
	int fd = get_file_id((char*)pathname+1);
	if(fd==-1)
		return 0;
	
	if(g_pIFSData->lpFileOptData[fd]==NULL)
		return 0;
	if(g_pIFSData->lpFileOptData[fd]->nCheck!=1111)
		return 0;
		
	if(g_pIFSData->lpFileOptData[fd]->bRead==1)
	{
		//do nothing:	
		plog("read...");
	}
	else if(g_pIFSData->lpFileOptData[fd]->bWrite==1)
	{
		plog("write...");
		fifo_delete_list(g_pIFSData->lpCurlData->liWrite);
		g_pIFSData->lpCurlData->liWrite = g_pIFSData->lpFileOptData[fd]->pFifo;
		g_pIFSData->lpFileOptData[fd]->pFifo = NULL;
		wrap_create_object_data(g_pIFSData->lpFileOptData[fd]->strPath);	
		if(errno==115)
			errno = 0;
	}
	ifs_api_close(fd);
	return 0;
}
// other functions:
FIFO * ifs_get_dir_list()
{
	return g_pIFSData->lpCurrentDirectoryList;
}

int ifs_api_mknod(const char * pathname)
{
	char path[1024];
	int len = strlen(pathname)-1; 
	safe_memcpy(path,(char*)pathname+1,len);
	
	create_null_object(path);
	return 0;
}

