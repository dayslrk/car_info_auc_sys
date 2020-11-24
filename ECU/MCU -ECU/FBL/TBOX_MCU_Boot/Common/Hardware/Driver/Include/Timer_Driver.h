#ifndef _TIMER_DRIVER_H
#define _TIMER_DRIVER_H

#include "system.h"



void TIMx_Init(u16 arr, u16 psc);
u32 TIMx_GetCounter(void);


#endif /*_TIMER_DRIVER_H*/

