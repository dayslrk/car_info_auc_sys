#include "Led_Driver.h"

/**
 *	@brief	Led初始化函数
 *	@param	无
 *	@retval	无
 */
void LED_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE|RCC_AHB1Periph_GPIOG, ENABLE);	//开启GPIO时钟
	
	GPIO_InitStructure.GPIO_Pin = LED0_PIN|LED1_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_High_Speed;
	GPIO_Init(LED0_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = LED2_PIN|BEEP_PIN;
	GPIO_Init(LED2_PORT, &GPIO_InitStructure);
	
	LED0_OFF;
	LED1_OFF;
	LED2_OFF;
	BEEP_OFF;
}


