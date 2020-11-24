#ifndef _SPI_FLASH_H
#define _SPI_FLASH_H

#include "system.h"





void SPI_Flash_Init(void);
void SPI_SetSpeed(u16 SPI_BaudRatePrescaler);
uint8_t SPI_ReadWriteByte(char data);

#endif
