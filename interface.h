//#include <fcntl.h>
//#include <dirent.h>
#include <sys/stat.h>

#ifndef __FS_INTERFACE_H__
#define __FS_INTERFACE_H__

typedef struct FILE_OPERATE_DATA
{
	int nCheck;		//check whether the FileSystemData data is validate.
	int nSize;		//file total size 
	int nUserLoc;	//file size that user opterate.
	int nCurlLoc;	//
	FIFO * pFifo;
	int bRead;
	int bWrite;
	char strPath[1024];
}File_Operate_Data;

typedef struct INTERFACE_FILE_SYSTEM_DATA
{
	char strMountPath[256];		//root/samba/
	char strContainer[256];		//guodaofeng
	char strLock[1024];			//if it is not null,lock the write or dir operation.
	char strCurrentDir[1024];	//the current directory.
	
	Curl_Data * lpCurlData;			//
	Swift_Auth_Data * lpAuthData;	//
	Swift_Container_Metadata * lpContainerMetadata;	//
	Swift_Object_Metadata * lpObjectMetadata;		//
	FIFO * lpCurrentDirectoryList;	//
	File_Operate_Data * lpFileOptData[1024];
}Interface_File_System_Data;

int ifs_new_data();
void ifs_delete_data();
void ifs_init_data(char * user,char *passwd,char * host,char * mount,char * container);
int ifs_get_auth();
int ifs_api_getattr(const char *pathname, struct stat *stbuf);
int ifs_api_readdir(const char *pathname);
int ifs_api_mkdir(const char *pathname);
int ifs_api_unlink(const char *pathname);
int ifs_api_rmdir(const char * pathname);
int ifs_api_rename(const char *oldpath, const char *newpath);
int ifs_api_open(const char * pathname,int flags);
int ifs_api_read(const char *pathname, char *buf, size_t size);
int ifs_api_write(const char *pathname, const char *buf, size_t size);
int ifs_api_release(const char *pathname);
FIFO * ifs_get_dir_list();
void getlasterror(int ret);
int ifs_api_mknod(const char * pathname);







#endif
