#include "Timer_Driver.h"

volatile unsigned int TicksCounter = 0UL;

/**
 *	@brief	定时器初始化
 *	@param	arr自动重装值，psc时钟预分频数
 *	@retval	无
 */
void TIMx_Init(u16 arr, u16 psc)//84000000   8399 10000 9
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);

	TIM_TimeBaseInitStructure.TIM_Period = arr; 	//自动重装载值
	TIM_TimeBaseInitStructure.TIM_Prescaler = psc;  //定时器分频
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; //向上计数模式
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1; 
	
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStructure);//初始化TIM3
	
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE); //允许定时器3更新中断
	TIM_Cmd(TIM3,ENABLE); //使能定时器3
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn; //定时器3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x08; //抢占优先级1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00; //子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

u32 TIMx_GetCounter(void)
{
	return TicksCounter;
}


//定时器3中断服务函数
void TIM3_IRQHandler(void)
{
	u32 xReturn = 0;
	xReturn = taskENTER_CRITICAL_FROM_ISR();	// 进入临界段，临界段可以嵌套
	if(TIM_GetITStatus(TIM3,TIM_IT_Update)==SET) //溢出中断
	{
		TicksCounter++;
	}
	TIM_ClearITPendingBit(TIM3,TIM_IT_Update);  //清除中断标志位
	taskEXIT_CRITICAL_FROM_ISR(xReturn);// 退出临界段 
}


