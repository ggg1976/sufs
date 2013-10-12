#define FUSE_USE_VERSION 26
//#ifdef HAVE_CONFIG_H
//#include <config.h>
//#endif

#ifdef linux
#define XOPEN_SOURCE 700
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/vfs.h>


#include <fuse.h>
#include <fuse_opt.h>

#include "sufs.h"
#include "swiftprotocol.h"
#include "curlapi.h"
#include "interface.h"


static int swift_getattr(const char *path, struct stat *stbuf) 
{
	return ifs_api_getattr(path,stbuf);
}
static int swift_access(const char *path, int mask)
{	
	// all permit to access.
	return 0;
}

static int swift_readlink(const char *path, char *buf, size_t size)
{
	//plog(".readlink");
	return 0;
}

static int swift_readdir(const char *path, void *buf, fuse_fill_dir_t filler,off_t offset, struct fuse_file_info *fi)
{
	if(ifs_api_readdir(path)>0)
	{
		FIFO * li = ifs_get_dir_list();
		int node = 10000;
		MemoryBuffer * mem = fifo_get_head(li);
		//cut the parent dir:
		int len = strlen(path);
		plog((char*)path);
		if(len==1)
			len = 0;
			
		while(mem)
		{
			struct stat st;
			memset(&st, 0, sizeof(st));
			st.st_ino = node;
			if(mem->lpBuffer[mem->nSize-1]=='/')
			{
				st.st_mode = S_IFDIR|0777;
				mem->lpBuffer[mem->nSize-1] = '\0';
				mem->nSize -= 1;				
			}
			else
			{
				get_file_stat(mem->lpBuffer,&st);
				//st.st_mode = S_IFREG|0777;				
			}
			st.st_uid = SUFS_USER_ID;
			st.st_gid = SUFS_GROUP_ID;
			plog(mem->lpBuffer+len);
			if(mem->lpBuffer[mem->nSize-1]=='/')
			{
				mem->lpBuffer[mem->nSize-1] = '\0';
				mem->nSize -= 1;
			}
				
			
			if (filler(buf, mem->lpBuffer+len, &st, 0))
				break;	
			node = node+1;
			mem = fifo_get_next(li);
		}	
	}
/*	DIR *dp;
	struct dirent *de;
	plog("swift_readdir\n");
	(void) offset;
	(void) fi;

	dp = opendir(path);
	if (dp == NULL)
		return -errno;

	while ((de = readdir(dp)) != NULL) {
		struct stat st;
		memset(&st, 0, sizeof(st));
		st.st_ino = de->d_ino;
		st.st_mode = de->d_type << 12;
		if (filler(buf, de->d_name, &st, 0))
			break;
	}

	closedir(dp);
*/	return 0;
}
static int swift_mknod(const char *path, mode_t mode, dev_t rdev)
{	
	int res;

	if (S_ISREG(mode)) {
		res = ifs_api_mknod(path);
	} else if(S_ISDIR(mode))
	{
		res = ifs_api_mkdir(path);
	}
	else
	{
		errno = -1;
		res = -1;
	}
	
	if (res == -1)
		return -errno;

	return 0;



	return ifs_api_mknod(path);
}
static int swift_mkdir(const char *pathname, mode_t mode)
{
	return ifs_api_mkdir(pathname);
}
static int swift_unlink( const char * pathname)
{
	return ifs_api_unlink(pathname);
}
static int swift_rmdir( const char *dirname)
{
	//delete all objects in the dir..
	ifs_api_rmdir(dirname);
	return 0;
}
static int swift_symlink(const char *from, const char *to)
{
	return 0;
}
static int swift_rename(const char *oldpath, const char *newpath)
{
	return 0;
}
static int swift_link(const char *from, const char *to)
{
	return 0;
}
static int swift_chmod( const char *filename, mode_t pmode)
{
	return 0;
}
static int swift_chown(const char * path, uid_t owner, gid_t group)
{
	return 0;
}
static int swift_truncate(const char *path, off_t size)
{
	return 0;
}
static int swift_utime(const char * filename,struct utimbuf * buf)
{
	return 0;
}
static int swift_open(const char *pathname, struct fuse_file_info *fi)
{
	return 0;//ifs_api_open(pathname,fi->flags);
}
static int swift_read(const char *path, char *buf, size_t size, off_t offset,struct fuse_file_info *fi)
{
	char buffer[1024];
	int ret = ifs_api_read(path,buf,size);
	sprintf(buffer,"READ:%d,%d,[%d],%s",ret,(int)offset,errno,buf);
	plog(buffer);
	return ret;
}
static int swift_write(const char *path, const char *buf, size_t size,off_t offset, struct fuse_file_info *fi)
{
	return ifs_api_write(path,buf,size);
}
static int swift_statfs(const char *path, struct statvfs *buf)
{
	return 0;
}
static int swift_release(const char *path, struct fuse_file_info *fi)
{
	return ifs_api_release(path);
}
static int swift_fsync(const char *path, int isdatasync,struct fuse_file_info *fi)
{
	return 0;
}
static int swift_fallocate(const char *path, int mode,off_t offset, off_t length, struct fuse_file_info *fi)
{
	return 0;
}
static int swift_setxattr(const char *path, const char *name, const char *value,size_t size, int flags)
{
	return 0;
}

static int swift_getxattr(const char *path, const char *name, char *value,size_t size)
{
	return 0;
}

static int swift_listxattr(const char *path, char *list, size_t size)
{
	return 0;
}

static int swift_removexattr(const char *path, const char *name)
{
	return 0;
}
static struct fuse_operations swift_operation = {
	.getattr	= swift_getattr,
	.access		= swift_access,
	.readlink	= swift_readlink,
	.readdir	= swift_readdir,
	.mknod		= swift_mknod,
	.mkdir		= swift_mkdir,
	.symlink	= swift_symlink,
	.unlink		= swift_unlink,
	.rmdir		= swift_rmdir,
	.rename		= swift_rename,
	.link		= swift_link,
	.chmod		= swift_chmod,
	.chown		= swift_chown,
	.truncate	= swift_truncate,
#ifdef HAVE_UTIMENSAT
	.utimens	= swift_utimens,
#endif
	.open		= swift_open,
	.read		= swift_read,
	.write		= swift_write,
	.statfs		= swift_statfs,
	.release	= swift_release,
	.fsync		= swift_fsync,
#ifdef HAVE_POSIX_FALLOCATE
	.fallocate	= swift_fallocate,
#endif
#ifdef HAVE_SETXATTR
	.setxattr	= swift_setxattr,
	.getxattr	= swift_getxattr,
	.listxattr	= swift_listxattr,
	.removexattr	= swift_removexattr,
#endif
};

int main(int argc, char *argv[]) 
{
	umask(0);
	int fuse_ret = ifs_new_data();
	if(fuse_ret==-1)
	{
		ifs_delete_data();
		return -1;
	}
	{
		//Get the parameters from the config file.
		ifs_init_data("test:tester","testing","192.168.8.162","/root/samba","myfiles");
	}
	
	fuse_ret = ifs_get_auth();
	if(fuse_ret==0)
	{
		printf("start fuse\n");		
		fuse_ret = fuse_main(argc,argv,&swift_operation,NULL);		
	}
	else
	{
		printf("error:%d\n",fuse_ret);
	}
	ifs_delete_data();
	
	return 0;
}
