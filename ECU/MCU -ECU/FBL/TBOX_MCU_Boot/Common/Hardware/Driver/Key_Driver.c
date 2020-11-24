#include "Key_Driver.h"
#include "delay.h"

/**
 *	@brief	按键初始化
 *	@param	无
 *	@retval	无
 */
void KeyDriver_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);    //使能GPIOF时钟
	
	GPIO_InitStructure.GPIO_Pin = KEY_UP|KEY_DOWN|KEY_LEFT|KEY_RIGHT; //KEY0 KEY1 KEY2 KEY3对应引脚
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;             //普通输入模式
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;             //上拉
	GPIO_Init(KEY_PORT, &GPIO_InitStructure);               
}

/**
 *	@brief	按键扫描
 *	@param	mode:0,不支持连续按，1支持连按
 *	@retval	KeyVal,按键值
 */
KeyVal KeyDriver_Scan(u8 mode)
{
	static u8 keyflag = 1;
	if(mode) keyflag = 1;
	if(keyflag&&(READ_KEY_UP == 0 || READ_KEY_DOWN == 0 || READ_KEY_LEFT == 0 || READ_KEY_RIGHT == 0))
	{
		delay_ms(10); //消抖
		keyflag = 0;
		if(READ_KEY_UP == 0) 			return KEY_UP_PRESS;
		else if(READ_KEY_DOWN == 0)		return KEY_DOWN_PRESS;
		else if(READ_KEY_LEFT == 0)		return KEY_LEFT_PRESS;
		else if(READ_KEY_RIGHT == 0)	return KEY_RIGHT_PRESS;
	}
	else if(READ_KEY_UP == 1 && READ_KEY_DOWN == 1 && READ_KEY_LEFT == 1 && READ_KEY_RIGHT == 1)
	{
		keyflag = 1;
	}
	return KEY_NOT_PRESS; //没有按键按下
}
