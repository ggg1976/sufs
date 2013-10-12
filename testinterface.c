#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include "sufs.h"
#include "curlapi.h"
#include "swiftprotocol.h"
#include "interface.h"

Interface_File_System_Data * ifs = NULL;

int main()
{
	int fuse_ret = ifs_new_data();
	if(fuse_ret==-1)
	{
		printf("Memory error.\n");
		ifs_delete_data();
		return -1;
	}
	ifs_init_data("test:tester","testing","192.168.8.162","/root/samba","myfiles");
	
	fuse_ret = ifs_get_auth();
	if(fuse_ret==0)
	{
		printf("start fuse.\n");
		struct stat m_stat;
		
		//ifs_api_getattr("/curl/upload",&m_stat);//41f8
		//ifs_api_getattr("/curl/upload/",&m_stat);//41f8
		//ifs_api_getattr("/curl/upload/test1.txt",&m_stat);//81f8
		//printf("getattr:%x\n",m_stat.st_mode);
		
		//printf("****READDIR:\n");
		fuse_ret = ifs_api_readdir("/");
		ifs_print_data(0,0,0,0,1,0);
		
		//printf("****MKDIR:\n");
		//ifs_api_mkdir("/a/b/c/d/e/f");
		//ifs_print_data(0,0,0,0,1,0);
		//ifs_api_readdir("/a/b/c/");
		//ifs_print_data(0,0,0,0,1,0);
		
		//printf("****DELETE FILE:\n");
		//ifs_api_unlink("/a/b/c/d/.");
		//ifs_api_readdir("/a/");
		//ifs_print_data(0,0,0,0,1,0);
		//ifs_api_readdir("/a/b/");
		//ifs_print_data(0,0,0,0,1,0);
		//ifs_api_readdir("/a/b/c/");
		//ifs_print_data(0,0,0,0,1,0);
		//ifs_api_unlink("/a/.");		
		//ifs_api_readdir("/a/");
		//ifs_print_data(0,0,0,0,1,0);
		
		//printf("****DELETE DIR:\n");
		//ifs_api_mkdir("/a/b/c/d/e/f");
		//ifs_api_rmdir("/a/b/c/d");
		//ifs_api_readdir("/a/b/c/");
		//ifs_print_data(0,0,0,0,1,0);
		
		/*char buf[1024];
		fuse_ret = ifs_api_read("/curl/upload/test1.txt",buf,1024);
		if(fuse_ret>0)
		{
			printf("%s\n",buf);
		}
		else
			printf("return count:%d\n",fuse_ret);
		ifs_api_release("/curl/upload/test1.txt");
		*/
		/*
		char wbuf[1024] = "this is a file to test write to swift.\nby gdf.\n2013-08-29.";
		fuse_ret = ifs_api_write("/test_write/1.txt",wbuf,strlen(wbuf));
		if(fuse_ret>0)
		{
			printf("%d\n",fuse_ret);
		}
		else
			printf("return count:%d\n",fuse_ret);
		ifs_api_release("/test_write/1.txt");
		*/
		
		
		
		printf("stop fuse.\n");
	}
	else
	{
		printf("auth error:%d\n",fuse_ret);
	}
	ifs_delete_data();
	
	return 0;
}




