#include "DbgUsart_Driver.h"

/**
 *	@brief	UART初始化
 *	@param	baudrate,波特率
 *	@retval	无
 */
void DebugUsart_Init(uint32_t baudrate)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef Usart_InitStructure;
#if (ENABLE_RX == 1)
	NVIC_InitTypeDef NVIC_InitStructure;
#endif
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9|GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	Usart_InitStructure.USART_Mode = USART_Mode_Rx|USART_Mode_Tx;
	Usart_InitStructure.USART_Parity = USART_Parity_No;
	Usart_InitStructure.USART_StopBits = USART_StopBits_1;
	Usart_InitStructure.USART_BaudRate = baudrate;
	Usart_InitStructure.USART_WordLength = USART_WordLength_8b;
	Usart_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_Init(USART1, &Usart_InitStructure);
	USART_Cmd(USART1, ENABLE);
	
	//中断配置
#if(ENABLE_RX == 1)
	USART_ClearFlag(USART1, USART_FLAG_TC);
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);         //开启相关中断

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;	// 抢断优先级为1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;			// 子优先级为1
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				// 使能中断 
	NVIC_Init(&NVIC_InitStructure);								//初始化配置NVIC
#endif
}

/**
 *	@brief	发送单个字符
 *	@param	ch,字符
 *	@retval	无
 */

void Debug_SendChar(char ch)
{
    USART1->DR = ch;
	while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
}

/**
 *	@brief	发送字符串
 *	@param	ch,字符串
 *	@retval	无
 */
void Debug_SendMsg(const char *ch)
{
	while(*ch != '\0')
	{
		Debug_SendChar(*ch);
		ch++;
	}
      
}


/**
 *	@brief	串口接收中断
 *	@param	无
 *	@retval	无
 */
#if(ENABLE_RX == 1)
void USART1_IRQHandler(void)
{

}
#endif


