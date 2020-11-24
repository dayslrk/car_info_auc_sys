#include "system.h"

/**
 *	@breif	采用如下方法实现执行汇编指令WFI，THUMB指令不支持汇编内联
 *  @param	无
 *	@retval	无
 */
void WFI_SET(void)
{
	asm("WFI");		  
}

/**
 *	@breif	关闭所有中断(但是不包括fault和NMI中断)
 *  @param	无
 *	@retval	无
 */
void INTX_DISABLE(void)
{
	asm("CPSID   I \n"
		"BX		LR");	  
}

/**
 *	@breif	开启所有中断
 *  @param	无
 *	@retval	无
 */
void INTX_ENABLE(void)
{
	asm("CPSIE  I \n"
		"BX		LR"); 
}

/**
 *	@breif	设置栈顶地址
 *  @param	addr:栈顶地址
 *	@retval	无
 */
void MSR_MSP(u32 addr) 
{
	asm("MSR MSP, r0 \n"
		"BX		r14"); 			//set Main Stack value
}

/**
 *	@breif	内核复位
 *  @param	无
 *	@retval	无
 */
void Asm_CoreReset(void)	
{
asm("LDR R0, =0xE000ED0C \n"
	"LDR R1, =0x05FA0001 \n"
	"STR R1, [R0] \n"			//将R1的值传送到地址为R0的内存中
	"loopc_core: \n"
	"B loopc_core");	
}

/**
 *	@breif	系统复位
 *  @param	无
 *	@retval	无
 */
void Asm_SystemReset(void)	//同NVIC_SystemReset
{
asm("LDR R0, =0xE000ED0C \n"
	"LDR R1, =0x05FA0004 \n"
	"STR R1, [R0] \n"			//将R1的值传送到地址为R0的内存中
	  "loopsys_core: \n"
	"B loopsys_core");
}

/**
 *	@breif	系统复位
 *  @param	无
 *	@retval	无
 */
void SystemReset(void)
{
	__set_FAULTMASK(1);//关闭所有中断，避免内核还在工作
	Asm_SystemReset();//复位
}

/**
 *	@breif	内核复位
 *  @param	无
 *	@retval	无
 */
void CoreReset(void)
{
	__set_FAULTMASK(1);//关闭所有中断，避免内核还在工作
	Asm_CoreReset();//复位
}





