#include <time.h>

#ifndef __UTIL_H_
#define __UTIL_H_

int slog(const char *fmt,...);
int plog(char * str);

int getmonth(char * month);
time_t gettimefromstring(char * buffer,struct tm * tmbuf);







#endif
