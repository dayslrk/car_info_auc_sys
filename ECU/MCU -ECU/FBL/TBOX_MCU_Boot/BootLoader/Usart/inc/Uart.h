#ifndef UART_H
#define UART_H

#include "BaseType.h"

u32 Uart_Recv(u8 *pdata, u32 *plen);

u32 Uart_Send(const u8 * pdata, const u32 len);


#endif /* UART_H */

