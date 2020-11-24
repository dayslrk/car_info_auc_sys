#include <stdarg.h>
#include "Logging.h"




/**
 *	@brief	Logging模块初始化
 *	@param	无
 *	@retval	无
 */
void Logging_Init(void)
{
	DebugUsart_Init(115200);
	RTCTime_Init();
}

/**
 *	@brief	打印函数，同printf,此文件使用
 *	@param	
 *	@retval	无
 */
void Logging(const char *format, ...)
{
	char s_buffer[BUFFER_LENGTH+1] = {0};
    va_list arg;
	va_start(arg, format);
	vsnprintf(s_buffer, BUFFER_LENGTH, format, arg);
	va_end(arg);
	Debug_SendMsg(s_buffer);
}

#if 0
/**
 *	@brief	打印函数
 *	@param	level,等级
 *	@param	format,格式串; ... 不定参数列表
 *	@retval	无
 */
void Log(LOGGING_LEVEL level, char const *format, ...)
{
	char s_buffer[BUFFER_LENGTH] = {0};
	u16 msec = 0;
	RTC_TimeTypeDef RTC_TimeStruct;
	va_list arg;
	if(level >= LOG_INFO) return;
	va_start(arg, format);
	vsnprintf(s_buffer, BUFFER_LENGTH, format, arg);
	va_end(arg);
	RTC_GetTime(RTC_Format_BIN, &RTC_TimeStruct);
	msec = 1000 - (u32)((u32)RTC_GetSubSecond()*1000)/(u32)0x3FF;
	
	switch(level)
	{
		case LOG_ERROR:
			Logging("FBL_[%05d] %s-%d ERR:   %s", msec, __FUNCTION__,__LINE__,s_buffer);
			break;
		case LOG_WARN:
			Logging("FBL_[%05d] %s-%d WARN:  %s",msec, __FUNCTION__,__LINE__, s_buffer);
			break;
		case LOG_DEBUG:
			Logging("FBL_[%05d] %s-%d DEBUG: %s", msec, __FUNCTION__,__LINE__, s_buffer);
			break;
		default:
			Debug_SendMsg(s_buffer);
			break;
	}
}

#endif 


