#ifndef _RTCTIME_DRIVER_H
#define _RTCTIME_DRIVER_H


#include "system.h"

u8 RTCTime_Init(void);
ErrorStatus RTC_Set_Date(u8 year,u8 month,u8 date,u8 week);
ErrorStatus RTC_Set_Time(u8 hour,u8 min,u8 sec,u8 ampm);
void RTC_Set_WakeUp(u32 wksel,u16 cnt);
void RTC_WKUP_IRQHandler(void);

#endif /*_RTCTIME_DRIVER_H*/

