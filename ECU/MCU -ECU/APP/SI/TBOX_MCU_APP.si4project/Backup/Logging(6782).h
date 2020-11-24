#ifndef _LOGGING_H
#define _LOGGING_H

#include "RtcTime_Driver.h"
#include "DbgUsart_Driver.h"

#define BUFFER_LENGTH   80u	//MyPrintf打印的最大长度


typedef enum
{
	LOG_ERROR = 0,
	LOG_WARN,
	LOG_DEBUG,
	LOG_INFO,
	LOG_CLOSE
	
}LOGGING_LEVEL;


void Logging_Init(void);

void Logging(const char *format, ...);

extern volatile unsigned int TicksCounter;


#define Log(a,format,...)				\
do{										\
	switch(a)							\
	{									\
		case LOG_ERROR:					\
			Logging("APP_[%05d] %s-%d ERR: ", TicksCounter, __FUNCTION__,__LINE__);		\
			break;																\
		case LOG_WARN:															\
			Logging("APP_[%05d] %s-%d WARN: ",TicksCounter, __FUNCTION__,__LINE__);		\
			break;																\
		case LOG_DEBUG:															\
			Logging("APP_[%05d] %s-%d DEBUG: ", TicksCounter, __FUNCTION__,__LINE__);	\
			break;																\
		default:																\
			break;																\
	}																			\
	Logging( format, ##__VA_ARGS__);											\
	Logging("\r\n");															\
}while(0)




#endif /*_LOGGING_H*/

