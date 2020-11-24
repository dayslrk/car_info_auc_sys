#ifndef _COMUSART_DRIVER_H
#define _COMUSART_DRIVER_H

#include "system.h"

#define COM_USART_USE_DMA	1u


#define COMUSART_DMA_BUFFER_SIZE	1024u
#define COMUSART_BUFFER_LENGTH		COMUSART_DMA_BUFFER_SIZE*2
#define COMUSART_DMA_ADDRESS		(USART3_BASE+4)

extern char DMA_RecvBuffer[COMUSART_DMA_BUFFER_SIZE];
typedef struct
{
	char SendBuffer[COMUSART_DMA_BUFFER_SIZE*3];
	char RecvBuffer[COMUSART_DMA_BUFFER_SIZE*3];
	u16 RecvLen;
	char SendFinishFlag;
}ComUsartData_t;

extern ComUsartData_t ComUsartData;

void ComUsartDriver_Init(uint32_t baudrate);
void ComUsartDriver_DMA_Rx_Init(uint8_t *lteBuff , uint16_t  lens);
void ComUsartDriver_DMA_Tx_Init(void);
void ComUsartDriver_SendStr(char *ch);
void ComUsartDriver_SendChar(char ch);
void ComUsartDriver_SendData(char *buffer, int length);
u16 ComUsartDriver_RecvPosition(void);

#endif /*_COMUSART_DRIVER_H*/

