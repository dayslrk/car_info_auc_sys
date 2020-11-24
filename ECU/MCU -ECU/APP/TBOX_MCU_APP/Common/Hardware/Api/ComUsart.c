#include "ComUsart.h"
#include <string.h>


/**
 *	@brief	通信串口初始化
 *	@param	baudrate,波特率
 *	@retval	无
 */
void ComUsart_Init(uint32_t baudrate)
{
	ComUsartDriver_Init(baudrate);
}


/**
 *	@brief	通信串口初始化
 *	@param	baudrate,波特率
 *	@retval	无
 */
void ComUsart_SendData(char *data, uint16_t length)
{
#if(COM_USART_USE_DMA == 1)
	memcpy(ComUsartData.SendBuffer, data, length);
	ComUsartDriver_SendData(NULL, length);
#else
	int i = 0;
	for(i = 0; i < length; i++)
	{
		ComUsartDriver_SendChar(data[i]);
	}
#endif
}

/**
 *	@brief	通信串口获取数据
 *	@param	baudrate,
 *	@retval	无
 */
void ComUsart_RecvData(char *data)
{
	if(ComUsartData.RecvLen > 0)
	{
		memcpy(data, ComUsartData.RecvBuffer, ComUsartData.RecvLen);
		ComUsartData.RecvLen = 0;
		memset(ComUsartData.RecvBuffer, 0, ComUsartData.RecvLen);
	}
	
}







