#ifndef _RNG_DRIVER_H
#define _RNG_DRIVER_H

#include "system.h"




void RngDriver_Init(void);
u32 RngDriver_GetRandomNum(void);
u32 RngDriver_GetRandomRange(u32 min, u32 max);

#endif /*_RNG_DRIVER_H*/

