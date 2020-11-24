#include "Wdg_Driver.h"
#include "Led_Driver.h"

/**
 *	@brief	独立看门狗初始化
 *	@param	prer:分频系数，reloadVal:计数器初始值
 *	@retval	无
 */
#if(USE_xWDG == 0)
void IWDG_Init(u8 prer, u16 reloadVal)
{
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable); 	//使能对IWDG->PR IWDG->RLR的写
	IWDG_SetPrescaler(prer); 						//设置IWDG分频系数
	IWDG_SetReload(reloadVal);   					//设置IWDG装载值
	IWDG_ReloadCounter(); 							//reload
	IWDG_Enable();       							//使能看门狗
}

/**
 *	@brief	独立看门狗喂狗操作
 *	@param	无
 *	@retval	int
 */
void IWDG_Feed(void)
{
	IWDG_ReloadCounter();	//reload
}


/**
 *	@brief	窗口看门狗初始化
 *	@param	tr:T[6:0],计数器值 ,wr:W[6:0],窗口值, fprer:时钟分频系数
 *	@retval	无
 */
#else
u8 WWDG_CNT=0X7F;
void WWDG_Init(u8 tr,u8 wr,u32 fprer)
{
	NVIC_InitTypeDef NVIC_InitStructure;
 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_WWDG,ENABLE); //使能窗口看门狗时钟
	
	WWDG_CNT = tr&WWDG_CNT;   	//初始化WWDG_CNT. 
	WWDG_SetPrescaler(fprer); 	//设置分频值
	WWDG_SetWindowValue(wr); 	//设置窗口值
	WWDG_Enable(WWDG_CNT);  	//开启看门狗
	
	NVIC_InitStructure.NVIC_IRQChannel = WWDG_IRQn;  //窗口看门狗中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;  //抢占优先级为2
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;			//子优先级为3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  //使能窗口看门狗
	NVIC_Init(&NVIC_InitStructure);
	
	WWDG_ClearFlag();	//清除提前唤醒中断标志位
	WWDG_EnableIT();	//开启提前唤醒中断
}

/**
 *	@brief	窗口看门狗中断服务程序，中断里喂狗
 *	@param	无
 *	@retval	无
 */
void WWDG_IRQHandler(void)
{
	WWDG_SetCounter(WWDG_CNT);	//重设窗口看门狗值
	WWDG_ClearFlag();			//清除提前唤醒中断标志位
	LED2_TOGGLE;
}
#endif

