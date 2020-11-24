#ifndef _W25QXX_H
#define _W25QXX_H

#include "Spi_Driver.h"


#define W25QXX_CS_LOW		GPIOG->BSRRH=GPIO_Pin_8
#define W25QXX_CS_HIGH		GPIOG->BSRRL=GPIO_Pin_8

typedef enum
{
	W25Q80 = 0xEF13,
	W25Q16 = 0xEF14,
	W25Q32 = 0xEF15,
	W25Q64 = 0xEF16,
	W25Q128 = 0xEF17,
	W25Q256 = 0xEF18
	
}W25QXX_ID;

extern W25QXX_ID W25QXX_TYPE;	//默认是W25Q256


//指令表
#define W25X_WriteEnable		0x06 
#define W25X_WriteDisable		0x04 
#define W25X_ReadStatusReg1		0x05 
#define W25X_ReadStatusReg2		0x35 
#define W25X_ReadStatusReg3		0x15 
#define W25X_WriteStatusReg1    0x01 
#define W25X_WriteStatusReg2    0x31 
#define W25X_WriteStatusReg3    0x11 
#define W25X_ReadData			0x03 
#define W25X_FastReadData		0x0B 
#define W25X_FastReadDual		0x3B 
#define W25X_PageProgram		0x02 
#define W25X_BlockErase			0xD8 
#define W25X_SectorErase		0x20 
#define W25X_ChipErase			0xC7 
#define W25X_PowerDown			0xB9 
#define W25X_ReleasePowerDown	0xAB 
#define W25X_DeviceID			0xAB 
#define W25X_ManufactDeviceID	0x90 
#define W25X_JedecDeviceID		0x9F 
#define W25X_Enable4ByteAddr    0xB7
#define W25X_Exit4ByteAddr      0xE9


u8 W25Qxx_Init(void);
u8 W25QXX_ReadRegState(u8 regno);
void W25QXX_WriteRegState(u8 regno, u8 sr);
void W25QXX_Write_Enable(void);
void W25QXX_Write_Disable(void);
void W25QXX_Read(u8* pBuffer,u32 ReadAddr,u16 NumByteToRead);
void W25QXX_Write_Page(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite);
void W25QXX_Write_NoCheck(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite);
void W25QXX_Write(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite);
void W25QXX_Erase_Chip(void);
void W25QXX_Erase_Sector(u32 Dst_Addr);
void W25QXX_Wait_Busy(void);
void W25QXX_PowerDown(void);
void W25QXX_WAKEUP(void);

#endif
