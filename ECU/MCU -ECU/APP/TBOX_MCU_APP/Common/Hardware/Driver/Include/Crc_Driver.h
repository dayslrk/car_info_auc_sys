#ifndef _CRC_DRIVER_H
#define _CRC_DRIVER_H

#include "system.h"


void CRCDriver_Init(void);
void CRCDriver_Reset(void);

u32 CRC32_Cal_Buffer( u32 crc,u8 *buff, u32 len);

#endif /*_CRC_DRIVER_H*/

