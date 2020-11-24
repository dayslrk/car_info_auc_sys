#include "Rng_Driver.h"

/**
 *	@brief	RNG外设初始化函数
 *	@param	无
 *	@retval	无
 */
void RngDriver_Init(void)
{
	RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_RNG, ENABLE); //开启产生随机数的硬件时钟
	RNG_ITConfig(DISABLE); //中断不使能，随机数外设只有一个错误中断，出错才会中断
	
	RNG_Cmd(ENABLE);//使能，在不需要随机数的时候请调用RNG_DeInit函数关闭
}

/**
 *	@brief	获取随机数，32位数据
 *	@param	无
 *	@retval	产生的随机数
 */
u32 RngDriver_GetRandomNum(void)
{
	while(RNG_GetFlagStatus(RNG_FLAG_DRDY) == RESET); 
	return RNG->DR;	//这里返回产生的32位原数据
}

/**
 *	@brief	获取一个min到max的随机数
 *	@param	x:随机数最大值减一
 *	@retval	所需的随机值
 */
u32 RngDriver_GetRandomRange(u32 min, u32 max) 
{
	while(RNG_GetFlagStatus(RNG_FLAG_DRDY) == RESET);
	return ((RNG->DR%(max - min +1))+min);
}

