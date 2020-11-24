#include "Crc_Driver.h"
#include "Logging.h"
//本模块使用注意：硬件CRC是32位的，不足32位的数据必须补足32位，不然计算结果会跟PC有差别



#if 1
const u32 POLYNOMIAL = 0xEDB88320;
static u32 table[256] ;


void CRCDriver_Init()
{
    int i, j;
    for (i = 0; i < 256 ; i++)
        for (j = 0, table[i] = i; j < 8; j++)
            table[i] = (table[i]>>1)^((table[i]&1)?POLYNOMIAL:0);
}

void CRCDriver_Reset(void)
{

}


u32 CRC32_Cal_Buffer(u32 crc, u8 *buff, u32 len)
{
    int i = 0;
    crc = ~crc;
    for (i = 0; i < len; i++)
        crc = (crc >> 8) ^ table[(crc ^ buff[i]) & 0xff];
    return ~crc;
}


#else
/**
 *	@brief	CRC外设初始化
 *	@param	无
 *	@retval	无
 */
void CRCDriver_Init(void)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_CRC, ENABLE);
	CRC->CR = CRC_CR_RESET;
}

/**
 *	@brief	CRC寄存器复位
 *	@param	无
 *	@retval	无
 */
void CRCDriver_Reset(void)
{
	CRC->CR = CRC_CR_RESET;
}

/**
 *	@brief	将一个32位数据按位颠倒函数，如0011->1100，效果更rbit指令一样
 *	@param	data:要颠倒的数据
 *	@retval	颠倒后的数据
 */
/*
static u32 Reversebit(u32 data)
{
	u32 uRevData = 0,i = 0;
	uRevData |= ((data >> i) & 0x01);
	for(i = 1; i < 32; i++)
	{
		uRevData <<= 1;
		uRevData |= ((data >> i) & 0x01);
	}
	return uRevData;
}

u32 swap(u32 data) //同REV指令
{
	u8 *p = (u8 *)&data;
	u8 *q = p + 3;
	
	while(p < q)	
	{
		*p = *p^*q;
		*q = *p^*q;
		*p = *p^*q;
		p++;
		q--;
	}
	return data;
}
*/


/**
 *	@brief	rbit指令，按位颠倒数据 //效果
 *	@param	data:要颠倒的数据      //01000001 01000010 01000011 01000100
 *	@retval	颠倒后的数据           //00100010 11000010 00100100 10000010
 */
//u32 RBITx(u32 data)	//后来发现CM4相关头文件中有__rbit()函数，在core_cmInstr.h有宏定义
//{
//	asm("RBIT	r0, r0");
//	asm("BX		LR");
//}

/**
 *	@brief	REV指令，将一个32位数据的字节序转变（大小端转换）,同swap函数
 *	@param	data:要转换字节序的数据
 *	@retval	转换后的数据
 */
//u32 REVx(u32 data)	//后来发现CM4相关头文件中有__rev()函数，在core_cmInstr.h有宏定义
//{
//	asm("REV	r0, r0");
//	asm("BX		LR");
//}

/**
 *	@brief	CRC32计算，同PC转换效果
 *	@param	data:要计算CRC32的数据
 *	@retval	CRC32值
 */
u32 CRC32_Cal(u32 data)
{
	//这样操作是为了同PC上计算的值保持一致，STM32硬件CRC32因字节序等原因，计算结果并不是与PC一致的
	CRC->DR = __RBIT(__REV(data));
	//因CRC寄存器复位值为0xFFFFFFFF,需进行异或操作
	return (__RBIT(CRC->DR)^0xFFFFFFFF);
}

/**
 *	@brief	计算一个buffer的CRC32校验值，同PC转换效果
 *	@param	data:要计算CRC32的buffer
 *	@retval	CRC32值
 */
#define CRC32_POLYNOMIAL ((uint32_t)0xEDB88320)
u32 CRC32_Cal_Buffer(u8 *buffer, u32 length)
{
	u32 i = 0;
	u32 len_u32 = length >>2;

	u32 crc = 0;
	
	for(i = 0; i < len_u32; i++)
	{
		CRC32_Cal(*((u32 *)(&(buffer[i*4]))));
	}

	crc = __RBIT(CRC->DR);

	len_u32 = length &0x03;

	buffer += (i *4);
	
	while (len_u32 --)
	{		
		crc ^= (u32)*buffer++;
		for(i = 0;i < 8;i++)
		 if (crc & 0x1)
		   crc = (crc >> 1) ^ CRC32_POLYNOMIAL;
		 else
		   crc >>= 1;
	}
	
	
	return crc^0xFFFFFFFF;	//返回最后一次计算的值
}
#endif


