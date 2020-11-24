#ifndef _WDG_DRIVER_H
#define _WDG_DRIVER_H

#include "system.h"

#define USE_xWDG	0u

void IWDG_Init(u8 prer, u16 reloadVal);
void IWDG_Feed(void);
void WWDG_Init(u8 tr,u8 wr,u32 fprer);


#endif /*_WDG_DRIVER_H*/
