#include "delay.h"

static u8 fac_us=0;		//us延时倍乘数
static u8 fac_ms=0;
			   
/**
 *	@brief	初始化延迟函数
 *	@param	SYSCLK,系统时钟频率
 *	@retval	无
 */
void delay_init(u8 SYSCLK)
{
#if(SYSTEM_SUPPORT_OS == 1)
	u32 reload = 0;
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);
	fac_us = SYSCLK;
	reload = SYSCLK;
	reload *= 1000000/configTICK_RATE_HZ;
	
	fac_ms = 1000/configTICK_RATE_HZ;
	SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;	//开启中断
	SysTick->LOAD = reload;						//每1/configTICK_RATE_HZ中断一次
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;	//开启SYSTICK
#else
	SysTick->CTRL&=~(1<<2);	//SYSTICK使用外部时钟源	 
	fac_us=SYSCLK/8;		//不论是否使用OS,fac_us都需要使用
#endif
}

/**
 *	@brief	延时nus，用OS时不产生任务调度
 *	@param	nus为要延时的us数
 *	@retval	无
 */
void delay_us(u32 nus)
{
#if(SYSTEM_SUPPORT_OS == 1)
	u32 ticks = 0;
	u32 told, tnow, tcnt = 0;
	u32 reload = SysTick->LOAD;
	
	ticks = nus*fac_us;
	told = SysTick->VAL;
	while(1)
	{
		tnow = SysTick->VAL;
		if(tnow != told)
		{
			if(tnow < told)	tcnt += told - tnow;
			else 			tcnt += reload-tnow+told;
			told = tnow;
			if(tcnt >= ticks) break;
		}
	}
#else
	u32 temp;	    	 
	SysTick->LOAD=nus*fac_us; 				//时间加载	  		 
	SysTick->VAL=0x00;        				//清空计数器
	SysTick->CTRL=0x01 ;      				//开始倒数 	 
	do
	{
		temp=SysTick->CTRL;
	}while((temp&0x01)&&!(temp&(1<<16)));	//等待时间到达   
	SysTick->CTRL=0x00;      	 			//关闭计数器
	SysTick->VAL =0X00;       				//清空计数器 
#endif
}

/**
 *	@brief	延时nms，使用OS时产生任务调度
 *	@param	nms:0~65535
 *	@retval	无
 */
void delay_ms(u16 nms)
{
#if(SYSTEM_SUPPORT_OS == 1)
	if(xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)//系统已经运行
	{
		if(nms >= fac_ms)
		{
			vTaskDelay(nms/fac_ms);	//freertos的延时
		}
		nms %= fac_ms;	//OS无法提供更小的延时，采用普通方式延时
	}
	delay_us((u32)(nms*1000));
#else
	u32 i;
	for(i=0;i<nms;i++) delay_us(1000);
#endif
} 

/**
 *	@brief	延时nms，普通延时不产生任务调度,用于OS运行之前的延时
 *	@param	nms:0~65535
 *	@retval	无
 */
void delay_xms(u16 nms)
{
	u32 i;
	for(i=0;i<nms;i++) delay_us(1000);
}

/**
 *	@brief	滴答定时器中断，产生时钟节拍
 *	@param	无
 *	@retval	无
 */
extern void xPortSysTickHandler(void);
void SysTick_Handler(void)
{	
	if(xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)//OS开始跑了,才执行正常的调度处理
	{
		xPortSysTickHandler();
	}
}



