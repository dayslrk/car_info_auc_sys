#ifndef _COMUSART_H
#define _COMUSART_H


#include "ComUsart_Driver.h"






void ComUsart_Init(uint32_t baudrate);
void ComUsart_SendData(char *data, uint16_t length);


#endif /*_COMUSART_H*/


