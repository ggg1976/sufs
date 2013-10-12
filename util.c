#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <stdarg.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include "sufs.h"

static int x = 0;
static int xx = 0;
int plog(char * str)
{
	if(strlen(str)==0)
	{
		return 0;
	}
	//char buf[1024];
	//sprintf(buf,"%s\r\n",str);
	//printf(buf);
	//return 0;
	int res;
	int fd = open("/root/sufs/log",O_WRONLY|O_APPEND|O_CREAT);
	if(fd==-1)
		return -errno;
	char buffer[4096];
	time_t tv = time(NULL);
	sprintf(buffer,"**%s%s\r\n",ctime(&tv),str);

	res = pwrite(fd,buffer,strlen(buffer),x);
	close(fd);
	if (res==-1)
		return -errno;
	x = x+res;
	return res;	
}
int slog(const char *fmt,...)
{
	// slog("%s,%d","ffff",45);
	va_list ap;
	char tmp[4096] = { 0 };
	char buf[4096] = { 0 };
	char ftime[64] = { 0 };
	struct timeval tv;
	time_t curtime;

	gettimeofday(&tv, NULL);
	curtime = tv.tv_sec;
	strftime(ftime, sizeof(ftime), "%F %T", localtime(&curtime));
	va_start(ap, fmt);
	vsnprintf(tmp, sizeof(tmp), fmt, ap);
	va_end(ap);
	snprintf(buf, sizeof(buf), "[%s] %s\r\n", ftime, tmp);
	int fd = open("/root/sufs/slog",O_WRONLY|O_APPEND|O_CREAT);
	if(fd==-1)
		return -errno;
	int res = pwrite(fd,buf,strlen(buf),xx);
	close(fd);
	if(res==-1)
		return -errno;
	xx = xx+res;

	return res;
}

// time functions:
int getmonth(char * month)
{
	if(strstr(month,"Jan"))
		return 0;
	if(strstr(month,"Feb"))
		return 1;
	if(strstr(month,"Mar"))
		return 2;
	if(strstr(month,"Apr"))
		return 3;
	if(strstr(month,"May"))
		return 4;
	if(strstr(month,"Jun"))
		return 5;
	if(strstr(month,"Jul"))
		return 6;
	if(strstr(month,"Aug"))
		return 7;
	if(strstr(month,"Sep"))
		return 8;
	if(strstr(month,"Oct"))
		return 9;
	if(strstr(month,"Nov"))
		return 10;
	if(strstr(month,"Dec"))
		return 11;

	return -1;
}
time_t gettimefromstring(char * buffer,struct tm * tmbuf)
{
	// Wed, 05 Jun 2013 02:55:34 GMT,DO NOT do with GMT,It is very complicated.
	char *buf = buffer;
	while(buf[0]==' ')
		buf =buf+1;
	char m[10],z[10];
	sscanf(buf,"%*s%d %3s %d %d:%d:%d %3s",&(tmbuf->tm_mday),m,&(tmbuf->tm_year),&(tmbuf->tm_hour),&(tmbuf->tm_min),&(tmbuf->tm_sec),z);
	tmbuf->tm_mon = getmonth(m);
	tmbuf->tm_year -= 1900;
	
	return mktime(tmbuf);
}


