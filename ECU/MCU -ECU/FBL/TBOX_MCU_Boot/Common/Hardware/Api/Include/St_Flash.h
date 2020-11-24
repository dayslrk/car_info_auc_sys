#ifndef _ST_FLASH_H
#define _ST_FLASH_H

#include "Flash_Driver.h"

u8 Flash_Write(u32 WriteAddr, u32 *Buffer, u32 index);
void Flash_Read(u32 ReadAddr, u32 *Buffer, u32 index);
u8 Flash_Erase(u32 EraseAddr, u32 index);

#endif /*_ST_FLASH_H*/
