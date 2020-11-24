#include "Uart.h"
#include "Logging.h"
#include <string.h>
#include "ComUsart_Driver.h"
#include "ComUsart.h"

extern ComUsartData_t ComUsartData;

u32 Uart_Recv(u8 *pdata, u32 *plen)
{
	u32 ret = 2;		
	int len = 0;

	if(NULL == pdata || NULL == plen || *plen ==0)
	{
/*		Log(LOG_DEBUG, "Param Faild.\n");*/
		return 1;
	}

	memset(pdata, 0x00, *plen);
	len = ComUsartData.RecvLen;

    if (len > 0)
    {
        __disable_irq(); 
        memcpy(pdata, ComUsartData.RecvBuffer, ComUsartData.RecvLen);
        ComUsartData.RecvLen = 0;
        __enable_irq(); 
        
		*plen = len;
		ret = 0;
    }

	return ret;
}


u32 Uart_Send(const u8 * pdata, const u32 len)
{
    ComUsart_SendData((char *)pdata, len);
    
	return 0;
}








