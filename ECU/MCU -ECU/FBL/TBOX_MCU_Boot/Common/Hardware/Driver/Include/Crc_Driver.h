#ifndef _CRC_DRIVER_H
#define _CRC_DRIVER_H

#include "system.h"


void CRCDriver_Init(void);
void CRCDriver_Reset(void);
//u32 RBITx(u32 data);
//u32 REVx(u32 data);
u32 CRC32_Cal(u32 x);
u32 CRC32_Cal_Buffer(u32 *buffer, u32 length);

#endif /*_CRC_DRIVER_H*/

