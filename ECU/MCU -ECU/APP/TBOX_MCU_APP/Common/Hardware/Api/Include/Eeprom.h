#ifndef _EEPROM_H
#define _EEPROM_H

#include "IIC_Driver.h"

#define AT24C01		127
#define AT24C02		255
#define AT24C04		511
#define AT24C08		1023
#define AT24C16		2047
#define AT24C32		4095
#define AT24C64	    8191
#define AT24C128	16383
#define AT24C256	32767  

//STM32F429BI板子使用的是24c02，所以定义EEPROM_TYPE为AT24C02
#define EEPROM_TYPE 	AT24C02

u8 AT24Cxx_Init(void);
u8 AT24Cxx_RecvByte(u16 RecvAddr);
void AT24Cxx_WriteByte(u16 WriteAddr, u8 DataToWrite);
void AT24Cxx_WriteLenByte(u16 WriteAddr,u32 DataToWrite,u8 Len);
u32 AT24Cxx_ReadLenByte(u16 ReadAddr, u8 Len);
void AT24Cxx_Read(u16 ReadAddr,u8 *pBuffer,u16 NumToRead);
void AT24Cxx_Write(u16 WriteAddr,u8 *pBuffer,u16 NumToWrite);


#endif /*_EEPROM_H*/
