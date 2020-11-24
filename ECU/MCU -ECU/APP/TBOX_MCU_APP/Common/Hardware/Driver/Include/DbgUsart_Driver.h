#ifndef _DBGUSART_DRIVER_H
#define _DBGUSART_DRIVER_H

#include "system.h"


#define ENABLE_RX		0u


void DebugUsart_Init(uint32_t baudrate);
void Debug_SendMsg(const char *ch);
void Debug_SendChar(char ch);

#endif /*_DBGUSART_DRIVER_H*/

