#include "CAN2_Driver.h"
#include "Logging.h"
#include "CAN.h"

/**
 *	@brief	CAN1初始化配置
 *	@param	mode:CAN1工作模式
 *	@retval	无
 */
void CAN2Driver_Init(u8 mode)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	CAN_InitTypeDef CAN_InitStructure;
	CAN_FilterInitTypeDef CAN_FilterInitStructure;
#if (CAN2_INT_ENABLE == 1) 
   	NVIC_InitTypeDef  NVIC_InitStructure;
#endif
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);	//使能GPIOB时钟	                   											 
  	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1 | RCC_APB1Periph_CAN2, ENABLE);	//使用CAN2的时候也要使能CAN1时钟	
	
	/*初始化GPIO*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12| GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;			//复用功能
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;			//推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;		//100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;			//上拉
    GPIO_Init(GPIOB, &GPIO_InitStructure);					//初始化PB12,PB13
	
	/*引脚复用映射配置*/
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource12, GPIO_AF_CAN2); //GPIOB12复用为CAN2
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_CAN2); //GPIOB13复用为CAN2
	
	/*CAN单元设置*/
   	CAN_InitStructure.CAN_TTCM = DISABLE;	//非时间触发通信模式   
  	CAN_InitStructure.CAN_ABOM = ENABLE;	//软件自动离线管理	  
  	CAN_InitStructure.CAN_AWUM = DISABLE;	//睡眠模式通过软件唤醒(清除CAN->MCR的SLEEP位)
  	CAN_InitStructure.CAN_NART = ENABLE;	//禁止报文自动传送 
  	CAN_InitStructure.CAN_RFLM = DISABLE;	//报文不锁定,新的覆盖旧的  
  	CAN_InitStructure.CAN_TXFP = DISABLE;	//优先级由报文标识符决
	
	CAN_InitStructure.CAN_Mode = mode;	 	//模式设置 
  	CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;//重新同步跳跃宽度(Tsjw)为tsjw+1个时间单位 CAN_SJW_1tq~CAN_SJW_4tq
  	CAN_InitStructure.CAN_BS1 = CAN_BS1_7tq;//时间段1的时间单元.  Tbs1范围CAN_BS1_1tq ~CAN_BS1_16tq
  	CAN_InitStructure.CAN_BS2 = CAN_BS2_6tq;//时间段2的时间单元.  Tbs2范围CAN_BS2_1tq ~	CAN_BS2_8tq
  	CAN_InitStructure.CAN_Prescaler = 6;  	//分频系数(Fdiv)为brp+1	
	CAN_Init(CAN2, &CAN_InitStructure);   	// 初始化CAN1 
	
	/*配置过滤器*/
 	CAN_FilterInitStructure.CAN_FilterNumber = 14;	  					//过滤器0
  	CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask; 
  	CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;	//32位 
  	CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0000;					//32位ID
  	CAN_FilterInitStructure.CAN_FilterIdLow = 0x0000;
  	CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000;				//32位MASK
  	CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x0000;
   	CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_Filter_FIFO0;//过滤器0关联到FIFO0
  	CAN_FilterInitStructure.CAN_FilterActivation = ENABLE; 				//激活过滤器0
  	CAN_FilterInit(&CAN_FilterInitStructure);							//滤波器初始化
	
#if (CAN2_INT_ENABLE == 1)
	NVIC_InitStructure.NVIC_IRQChannel = CAN2_RX0_IRQn;
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;     // 主优先级为1
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;            // 次优先级为0
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  	NVIC_Init(&NVIC_InitStructure);
	CAN_ITConfig(CAN2, CAN_IT_FMP0, ENABLE);//FIFO0消息挂号中断允许.		    
#endif
}

/**
 *	@brief	can2发送一组数据
 *	@param	TxMessage:发送数据集
 *	@retval	0，成功； 1失败
 */
u8 CAN2Driver_SendData(CanTxMsg *TxMessage)
{
	u8 mbox;
	u32 i = 0;
	mbox= CAN_Transmit(CAN2, TxMessage);   
	while((CAN_TransmitStatus(CAN2, mbox)!=CAN_TxStatus_Ok)&&(i<0XFFF))i++;	//等待发送结束
	if (i >= 0xFFF)
	{
		/*Log(LOG_DEBUG, "SendMessage = %s\n", TxMessage->Data);*/
	}
	else
	{
		/*Log(LOG_DEBUG,"%s Success\n", __func__);*/
	}
	
	if(i >= 0xFFF)return 1;
	return 0;
}

/**
 *	@brief	CAN2查询接收数据
 *	@param	data:数据缓存区
 *	@retval	0，无数据接收； 其他值，接收到的数据长度
 */
u8 CAN2Driver_RecvData(u8 *data)
{		   		   
 	u32 i;
	CanRxMsg RxMessage;
	//没有接收到数据,直接退出
    if( CAN_MessagePending(CAN2, CAN_FIFO0) == 0)
		return 0;		 
    CAN_Receive(CAN2, CAN_FIFO0, &RxMessage);//读取数据	
    for(i = 0;i < RxMessage.DLC; i++)
	{
		data[i]=RxMessage.Data[i];
	}		
	return RxMessage.DLC;	
}

/**
 *	@brief	CAN2中断接收数据
 *	@param	无
 *	@retval	无
 */
#if CAN2_INT_ENABLE	//使能RX0中断			    
void CAN2_RX0_IRQHandler(void)
{
  	CanRxMsg RxMessage;
    CAN_Receive(CAN2, 0, &RxMessage);
    CAN_RX1_IRQHandler(2, &RxMessage);
}
#endif


