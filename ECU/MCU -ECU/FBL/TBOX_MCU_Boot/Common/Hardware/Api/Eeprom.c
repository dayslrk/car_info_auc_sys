#include "Eeprom.h"
#include "delay.h"

static u8 AT24Cxx_Check(void);

/**
 *	@brief	AT24Cxx初始化
 *	@param	无
 *	@retval	0,成功；非0,失败
 */
u8 AT24Cxx_Init(void)
{
	IIC_Init();
	if(AT24Cxx_Check())	return 1;
	else return 0;
}

static u8 AT24Cxx_Check(void)
{
	u8 temp;
	temp = AT24Cxx_RecvByte(255);//避免每次开机都写AT24CXX			   
	if(temp == 0x55) return 0;		   
	else//排除第一次初始化的情况
	{
		AT24Cxx_WriteByte(255, 0x55);
	    temp=AT24Cxx_RecvByte(255);	  
		if(temp == 0x55) return 0;
	}
	return 1;
}

/**
 *	@brief	AT24Cxx读一个字节
 *	@param	RecvAddr,开始读数的地址
 *	@retval	读到的数据
 */
u8 AT24Cxx_RecvByte(u16 RecvAddr)
{
	u8 Temp = 0;
	IIC_Start();
	if(EEPROM_TYPE > AT24C16)
	{
		IIC_SendByte(0xA0);	/*发送写命令*/
		IIC_WaitAck();
		IIC_SendByte(RecvAddr>>8);	/*发送高地址*/
	}
	else IIC_SendByte(0xA0 + ((RecvAddr/256)<<1));/*发送器件地址0XA0,写数据*/ 	 
	IIC_WaitAck();
	IIC_SendByte(RecvAddr%256);//发送低地址
	IIC_WaitAck();
	IIC_Start();
	IIC_SendByte(0xA1);
	IIC_WaitAck();
	Temp = IIC_RecvByte(0);
	IIC_Stop();
	return Temp;
}

/**
 *	@brief	AT24Cxx写一个字节
 *	@param	WriteAddr,开始写数的地址；DataToWrite，要写的数据
 *	@retval	无
 */
void AT24Cxx_WriteByte(u16 WriteAddr, u8 DataToWrite)
{
	IIC_Start();  
	if(EEPROM_TYPE>AT24C16)
	{
		IIC_SendByte(0xA0);	    //发送写命令
		IIC_WaitAck();
		IIC_SendByte(WriteAddr>>8);//发送高地址	  
	}else IIC_SendByte(0xA0+((WriteAddr/256)<<1));   //发送器件地址0XA0,写数据 	 
	IIC_WaitAck();	   
    IIC_SendByte(WriteAddr%256);   //发送低地址
	IIC_WaitAck(); 	 										  		   
	IIC_SendByte(DataToWrite);     //发送字节							   
	IIC_WaitAck();  		    	   
	IIC_Stop();//产生一个停止条件 
	delay_xms(8);
}

/**
 *	@brief	在AT24CXX里面的指定地址开始写入长度为Len的数据,该函数用于写入16bit或者32bit的数据
 *	@param	WriteAddr,开始写数的地址；DataToWrite，要写的数据；Len,长度2或4
 *	@retval	无
 */
void AT24Cxx_WriteLenByte(u16 WriteAddr,u32 DataToWrite,u8 Len)
{  	
	u8 t;
	for(t=0; t<Len; t++)
	{
		AT24Cxx_WriteByte(WriteAddr+t,(DataToWrite>>(8*t))&0xff);
	}										    
}

/**
 *	@brief	在AT24CXX里面的指定地址开始读出长度为Len的数据,该函数用于读出16bit或者32bit的数据
 *	@param	ReadAddr,开始读出的数据；DataToWrite,要读的数据；Len,长度2或4
 *	@retval	无
 */
u32 AT24Cxx_ReadLenByte(u16 ReadAddr, u8 Len)
{  	
	u8 t;
	u32 temp=0;
	for(t=0;t<Len;t++)
	{
		temp<<=8;
		temp+=AT24Cxx_RecvByte(ReadAddr+Len-t-1); 	 				   
	}
	return temp;												    
}

/**
 *	@brief	在AT24CXX里面的指定地址开始读出指定个数的数据
 *	@param	ReadAddr,开始读出的地址,对24c02为0~255；pBuffer数据数组首地址；NumToRead:要读出数据的个数
 *	@retval	无
 */
void AT24Cxx_Read(u16 ReadAddr,u8 *pBuffer,u16 NumToRead)
{
	while(NumToRead)
	{
		*pBuffer++=AT24Cxx_RecvByte(ReadAddr++);	
		NumToRead--;
	}
}  


/**
 *	@brief	在AT24CXX里面的指定地址开始写入指定个数的数据
 *	@param	WriteAddr,开始写入的地址,对24c02为0~255；pBuffer数据数组首地址；NumToRead:要读出数据的个数
 *	@retval	无
 */
void AT24Cxx_Write(u16 WriteAddr,u8 *pBuffer,u16 NumToWrite)
{
	while(NumToWrite--)
	{
		AT24Cxx_WriteByte(WriteAddr,*pBuffer);
		WriteAddr++;
		pBuffer++;
	}
}



