#ifndef	LOG_H
#define LOG_H

#include "stdio.h"
#include "stdarg.h"
#include "common.h"
#include "stdlib.h"
#include "string.h"



//日志接口封装
typedef enum
{
	DEBUG,
	INFO,
	WARRN,
	ERR,
	HMI
}EN_LOG;

#define MIN_LOG      DEBUG //打印改等级及其以上等级的日志

#define __FILENAME__ (strrchr(__FILE__, '/') ? (strrchr(__FILE__, '/') + 1):__FILE__)


#include "time.h"
#if 1
#define LOG(a,format, ... )								\
do{														\
	struct tm *t;										\
	time_t tt;											\
	time(&tt);											\
	t = localtime(&tt);									\
	if(a < MIN_LOG) break;								\
	if(HMI == a){											\
		printf(format,##__VA_ARGS__);						\
	}														\
	else													\
	{														\
		if(ERR == a) printf("\033[1;31;40m");				\
		else if(WARRN == a)	printf("\033[1;33;40m");		\
		else if(DEBUG == a) printf("\033[1;32;40m");		\
		else 	printf("\033[0m");							\
		printf("%02d-%02d %02d:%02d:%02d ",  t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);	\
		printf("[%*s] %*d %*s  %*s  L%04d:  ",5,#a, 5,111111 ,16,__FILENAME__, 16,__FUNCTION__, __LINE__); 		\
		printf(format, ##__VA_ARGS__);						\
		printf("\033[0m");									\
	}														\
}while(0)	
#else

#include "pthread.h"

int LogInit();


//该函数不允许直接调用
int Logwrite(char *pLog);

int LogClose();


#define MAX_S_LOG			4096

#define LOG(a,format, ... )								\
do{														\
	struct tm *t;										\
	time_t tt;											\
	time(&tt);											\
	t = localtime(&tt);									\
	if(a < MIN_LOG) break;								\
	if(HMI == a){											\
		printf(format,##__VA_ARGS__);						\
	}														\
	else													\
	{														\
		if(ERR == a) printf("\033[1;31;40m");				\
		else if(WARRN == a)	printf("\033[1;33;40m");		\
		else if(DEBUG == a) printf("\033[1;32;40m");		\
		else 	printf("\033[0m");							\
		char *pBuff = (char *)malloc(MAX_S_LOG);			\
		memset(pBuff, 0x00, MAX_S_LOG);						\
		sprintf(pBuff+strlen(pBuff),"%02d-%02d %02d:%02d:%02d ",  t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);	\
		sprintf(pBuff+strlen(pBuff),"[%*s] %*d %*s  %*s  L%04d:  ",5,#a, 5,(int)-pthread_self() ,16,__FILENAME__, 16,__FUNCTION__, __LINE__); 		\
		sprintf(pBuff+strlen(pBuff),format, ##__VA_ARGS__);						\
		printf("%s",pBuff);									\
		Logwrite(pBuff);									\
		printf("\033[0m");									\
	}														\
}while(0)	



#endif 

#endif	/* LOG_H */
