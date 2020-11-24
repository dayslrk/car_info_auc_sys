#include "ComUsart_Driver.h"
#include <string.h>
#include "Led_Driver.h"

ComUsartData_t ComUsartData;


#if(COM_USART_USE_DMA == 1)
char DMA_RecvBuffer[COMUSART_DMA_BUFFER_SIZE] = {0};
#endif
/**
 *	@brief	UART初始化
 *	@param	baudrate,波特率
 *	@retval	无
 */
void ComUsartDriver_Init(uint32_t baudrate)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef Usart_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_USART3);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_USART3);
	
	//中断配置
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;	// 抢断优先级为1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;			// 子优先级为1
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				// 使能中断 
	NVIC_Init(&NVIC_InitStructure);	//初始化配置NVIC
	
	Usart_InitStructure.USART_Mode = USART_Mode_Rx|USART_Mode_Tx;
	Usart_InitStructure.USART_Parity = USART_Parity_No;
	Usart_InitStructure.USART_StopBits = USART_StopBits_1;
	Usart_InitStructure.USART_BaudRate = baudrate;
	Usart_InitStructure.USART_WordLength = USART_WordLength_8b;
	Usart_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_Init(USART3, &Usart_InitStructure);
	

#if (COM_USART_USE_DMA == 0)
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
#else
	USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);
	
	ComUsartDriver_DMA_Tx_Init();
    ComUsartDriver_DMA_Rx_Init(NULL, 0);
#endif

	USART_Cmd(USART3, ENABLE);
}

/**
 *	@brief	通信串口接收相关DMA外设初始化
 *	@param	无
 *	@retval	无
 */
#if (COM_USART_USE_DMA == 1)
void ComUsartDriver_DMA_Rx_Init(uint8_t *lteBuff , uint16_t  lens)
{
	DMA_InitTypeDef	DMA_InitStructure;
	NVIC_InitTypeDef NVIC_Initstucture;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
	
	DMA_DeInit(DMA1_Stream1);
	while (DMA_GetCmdStatus(DMA1_Stream1) != DISABLE);
	
	DMA_InitStructure.DMA_Channel = DMA_Channel_4;	//通道4
	DMA_InitStructure.DMA_PeripheralBaseAddr = COMUSART_DMA_ADDRESS; //串口数据地址
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)DMA_RecvBuffer; //DMA buffer
	DMA_InitStructure.DMA_BufferSize = COMUSART_DMA_BUFFER_SIZE;	//buffer大小
    //DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)lteBuff; //DMA buffer
	//DMA_InitStructure.DMA_BufferSize = lens;	//buffer大小
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory; //传输放向

	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable; //禁止FIFO
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
	
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; 
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
	
	DMA_Init(DMA1_Stream1, &DMA_InitStructure);
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	NVIC_Initstucture.NVIC_IRQChannel = DMA1_Stream1_IRQn;
	NVIC_Initstucture.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_Initstucture.NVIC_IRQChannelSubPriority = 0;
	NVIC_Initstucture.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_Initstucture);
	
	DMA_ClearFlag(DMA1_Stream1, DMA_FLAG_TCIF1);
	DMA_ITConfig(DMA1_Stream1, DMA_IT_TE|DMA_IT_HT, ENABLE);
	
	DMA_Cmd(DMA1_Stream1,ENABLE);
	while(DMA_GetCmdStatus(DMA1_Stream1) != ENABLE);
	USART_DMACmd(USART3, USART_DMAReq_Rx, ENABLE);
}
#endif

/**
 *	@brief	通信串口接收相关DMA外设初始化
 *	@param	无
 *	@retval	无
 */
#if (COM_USART_USE_DMA == 1)
void ComUsartDriver_DMA_Tx_Init(void)
{
	DMA_InitTypeDef	DMA_InitStructure;
	NVIC_InitTypeDef NVIC_Initstucture;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
	
	DMA_DeInit(DMA1_Stream3);
	while (DMA_GetCmdStatus(DMA1_Stream3) != DISABLE);
	
	DMA_InitStructure.DMA_Channel = DMA_Channel_4;	//通道4
	DMA_InitStructure.DMA_PeripheralBaseAddr = COMUSART_DMA_ADDRESS; //串口数据地址
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)ComUsartData.SendBuffer; //DMA buffer
	DMA_InitStructure.DMA_BufferSize = COMUSART_DMA_BUFFER_SIZE;	//buffer大小
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral; //传输放向

	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable; //禁止FIFO
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
	
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; 
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
	
	DMA_Init(DMA1_Stream3, &DMA_InitStructure);
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	NVIC_Initstucture.NVIC_IRQChannel = DMA1_Stream3_IRQn;
	NVIC_Initstucture.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_Initstucture.NVIC_IRQChannelSubPriority = 0;
	NVIC_Initstucture.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_Initstucture);
	
	DMA_ClearFlag(DMA1_Stream3, DMA_FLAG_TCIF3);
	DMA_ITConfig(DMA1_Stream3, DMA_IT_TE|DMA_IT_TC, ENABLE);
	
	DMA_Cmd(DMA1_Stream3, ENABLE);
	while(DMA_GetCmdStatus(DMA1_Stream3) != ENABLE);
	USART_DMACmd(USART3, USART_DMAReq_Tx, ENABLE);
}
#endif

/**
 *	@brief	发送单个字符
 *	@param	ch,字符
 *	@retval	无
 */
#if(COM_USART_USE_DMA == 0)
void ComUsartDriver_SendChar(char ch)
{
	USART_SendData(USART3, ch);
	while(USART_GetFlagStatus(USART3, USART_FLAG_TC)==RESET);
}
#endif

/**
 *	@brief	发送数据
 *	@param	buffer，数据缓存区，使用DMA时可以填NULL
 *	@param	length，长度
 *	@retval	无
 */
void ComUsartDriver_SendData(char *buffer, int length)
{
#if(COM_USART_USE_DMA == 0)
	int index = 0;
	for(index = 0; index <length; index++)
	{
		ComUsartDriver_SendChar(buffer[index]);
	}
#else
	DMA_Cmd(DMA1_Stream3, DISABLE);
	while(DMA_GetCmdStatus(DMA1_Stream3) != DISABLE);
	DMA_SetCurrDataCounter(DMA1_Stream3, length);
	DMA_Cmd(DMA1_Stream3, ENABLE);
#endif	
}

/**
 *	@brief	发送字符串
 *	@param	ch,字符串
 *	@retval	无
 */
#if(COM_USART_USE_DMA == 0)
void ComUsartDriver_SendStr(char *ch)
{
	while(*ch != '\0')
	{
		ComUsartDriver_SendChar(*ch);
		ch++;
	}
}
#endif

/**
 *	@brief	DMA接收数据
 *	@param	data,数据存储位置， length,长度
 *	@retval	无
 */
void ComUsartDriver_RecvData(void)
{
    u16 RecLen = 0;
  
#if (COM_USART_USE_DMA == 1)
	DMA_Cmd(DMA1_Stream1, DISABLE);
	DMA_ClearFlag(DMA1_Stream1, DMA_FLAG_FEIF1);
	RecLen += COMUSART_DMA_BUFFER_SIZE - DMA_GetCurrDataCounter(DMA1_Stream1);
    if (ComUsartData.RecvLen <= 1024 * 3)
    {
        memcpy(ComUsartData.RecvBuffer + ComUsartData.RecvLen, DMA_RecvBuffer, RecLen);
        ComUsartData.RecvLen += RecLen;
    }
    else
    {
        ;
    }
    
	DMA_SetCurrDataCounter(DMA1_Stream1, COMUSART_DMA_BUFFER_SIZE);
	DMA_Cmd(DMA1_Stream1, ENABLE);
#endif
}

/**
 *	@brief	拷贝数据，返回位置
 *	@param	无
 *	@retval	位置
 */
//u16 ComUsartDriver_RecvPosition(void)
//{
//	static u16 recv_pos = 0;
//	u16 remain = 0;
//	if(ComUsartData.RecvLen > 0)
//	{
//		if(recv_pos + ComUsartData.RecvLen <= COMUSART_BUFFER_LENGTH)
//		{
//			memcpy(ComUsartData.RecvBuffer + recv_pos, DMA_RecvBuffer, ComUsartData.RecvLen);
//			recv_pos += ComUsartData.RecvLen;
//			if(recv_pos == COMUSART_BUFFER_LENGTH)
//				recv_pos = 0;
//		}
//		else
//		{
//			memcpy(ComUsartData.RecvBuffer + recv_pos, DMA_RecvBuffer, COMUSART_BUFFER_LENGTH - recv_pos);
//			remain = ComUsartData.RecvLen - (COMUSART_BUFFER_LENGTH - recv_pos);
//			memcpy(ComUsartData.RecvBuffer, DMA_RecvBuffer + (COMUSART_BUFFER_LENGTH - recv_pos), remain);
//			recv_pos = remain;
//		}
//	}
//	return recv_pos;
//}


/**
 *	@brief	串口接收中断
 *	@param	无
 *	@retval	无
 */
void USART3_IRQHandler(void)
{
	u32 xReturn = 0;
	
#if (COM_USART_USE_DMA == 0)
	char ch = 0;
	xReturn = taskENTER_CRITICAL_FROM_ISR();	// 进入临界段，临界段可以嵌套
	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
	{
		USART_ClearITPendingBit(USART3, USART_IT_RXNE); // 清除标志位
		ch = USART_ReceiveData(USART3);
		
		//if(ch == 'c')
		LED2_TOGGLE;
		ComUsartDriver_SendChar(ch);
	}
#else


	xReturn = taskENTER_CRITICAL_FROM_ISR();	// 进入临界段，临界段可以嵌套
	if(USART_GetITStatus(USART3, USART_IT_IDLE) != RESET)
	{
		/*必须读一次这两个寄存器，相当于串口IDLE清除标志，不然会一直出发串口IDLE中断*/
		(void)USART3->SR;//相当清除flag
		(void)USART3->DR;//相当于调一次USART_ReceiveData函数
		
		USART_ClearITPendingBit(USART3, USART_IT_IDLE); // 清除标志位
		ComUsartDriver_RecvData();
	}
	
#endif
	taskEXIT_CRITICAL_FROM_ISR(xReturn);// 退出临界段 
}

/**
 *	@brief	串口DMA接收中断,占时没有用
 *	@param	无
 *	@retval	无
 */
#if (COM_USART_USE_DMA == 1)
void DMA1_Stream1_IRQHandler(void)
{
	if(DMA_GetITStatus(DMA1_Stream1, DMA_IT_TEIF1))
	{
		DMA_ClearITPendingBit(DMA1_Stream1, DMA_IT_TEIF1);
	
	}
	if(DMA_GetITStatus(DMA1_Stream1, DMA_IT_HTIF1))
	{
		DMA_ClearITPendingBit(DMA1_Stream1, DMA_IT_HTIF1);
		ComUsartDriver_RecvData();
	}
}


#endif

/**
 *	@brief	串口DMA发送完成中断，当用串口DMA发数据时，发完进一次
 *	@param	无
 *	@retval	无
 */
#if (COM_USART_USE_DMA == 1)
void DMA1_Stream3_IRQHandler(void)
{
	u32 xReturn = 0;
	xReturn = taskENTER_CRITICAL_FROM_ISR();	// 进入临界段，临界段可以嵌套
	if(DMA_GetFlagStatus(DMA1_Stream3, DMA_IT_TCIF3))
	{
		LED2_TOGGLE;
		ComUsartData.SendFinishFlag = 1;	//用了清零,不用也没关系
		DMA_ClearITPendingBit(DMA1_Stream3, DMA_IT_TCIF3);
		//用于发送数据的DMA必须在发送完成后禁用，不然会一直发送
		DMA_Cmd(DMA1_Stream3, DISABLE); 
	}
	taskEXIT_CRITICAL_FROM_ISR(xReturn);// 退出临界段 
}
#endif

