#ifndef _LED_DRIVER_H
#define _LED_DRIVER_H

#include "system.h"

#define LED0_PORT	GPIOE
#define LED1_PORT	GPIOE
#define LED2_PORT	GPIOG

#define LED0_PIN	GPIO_Pin_3
#define LED1_PIN	GPIO_Pin_4
#define LED2_PIN	GPIO_Pin_9

#define BEEP_PORT	GPIOG
#define BEEP_PIN	GPIO_Pin_7

#define	digitalHi(p,i)			{p->BSRRL=i;}		//设置为高电平
#define digitalLo(p,i)			{p->BSRRH=i;}		//输出低电平
#define digitalToggle(p,i)	 	{p->ODR ^=i;}		//输出反转状态

#define LED0_ON		digitalLo(LED0_PORT, LED0_PIN)
#define LED0_OFF	digitalHi(LED0_PORT, LED0_PIN)
#define LED0_TOGGLE	digitalToggle(LED0_PORT, LED0_PIN)	

#define LED1_ON		digitalLo(LED1_PORT, LED1_PIN)
#define LED1_OFF	digitalHi(LED1_PORT, LED1_PIN)
#define LED1_TOGGLE	digitalToggle(LED1_PORT, LED1_PIN)

#define LED2_ON		digitalLo(LED2_PORT, LED2_PIN)
#define LED2_OFF	digitalHi(LED2_PORT, LED2_PIN)
#define LED2_TOGGLE	digitalToggle(LED2_PORT, LED2_PIN)

#define BEEP_ON		digitalHi(BEEP_PORT, BEEP_PIN)
#define BEEP_OFF	digitalLo(BEEP_PORT, BEEP_PIN)
#define BEEP_TOGGLE	digitalToggle(BEEP_PORT, BEEP_PIN)

void LED_GPIO_Init(void);



#endif /*_LED_DRIVER_H*/


