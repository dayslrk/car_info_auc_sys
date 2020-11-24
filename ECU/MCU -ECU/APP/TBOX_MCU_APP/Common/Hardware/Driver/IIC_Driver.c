#include "IIC_Driver.h"
#include "delay.h"

/**
 *	@brief	外部EEPROM初始化
 *	@param	无
 *	@retval	无
 */
void IIC_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);	//开启GPIO时钟
	GPIO_InitStructure.GPIO_Pin = IIC_SCL_PIN|IIC_SDA_PIN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_Init(IIC_PORT, &GPIO_InitStructure);
	
	IIC_SCL_OUT_HIGH;
	IIC_SDA_OUT_HIGH;
}

/**
 *	@brief	EEPROM I2C起始信号
 *	@param	无
 *	@retval	无
 */
void IIC_Start(void)
{
	IIC_SDA_OUT();	/*SDA为输出*/
	IIC_SDA_OUT_HIGH;
	IIC_SCL_OUT_HIGH;
	delay_us(4);
	IIC_SDA_OUT_LOW;
	delay_us(4);
	IIC_SCL_OUT_LOW;
}

/**
 *	@brief	EEPROM I2C停止信号
 *	@param	无
 *	@retval	无
 */
void IIC_Stop(void)
{
	IIC_SDA_OUT();		/*SDA为输出*/
	IIC_SCL_OUT_LOW;
	IIC_SDA_OUT_LOW;
	delay_us(4);
	IIC_SCL_OUT_HIGH;
	delay_us(4);
	IIC_SDA_OUT_HIGH;	/*发送I2C总线结束信号*/
}

/**
 *	@brief	等待EEPROM I2C应答信号
 *	@param	无
 *	@retval	u8,1:应答失败；0，应答成功
 */
u8 IIC_WaitAck(void)
{
	u8 ErrorTime = 0;
	IIC_SDA_IN();		/*SDA为输入*/
	
	IIC_SDA_OUT_HIGH;
	delay_us(1);
	IIC_SCL_OUT_HIGH;
	delay_us(1);
	while(IIC_SDA_READ)
	{
		if(ErrorTime++ > 250)
		{
			IIC_Stop();
			return 1;
		}
	}
	IIC_SCL_OUT_LOW;/*时钟输出0*/
	return 0;
}

/**
 *	@brief	产生EEPROM I2C应答信号
 *	@param	无
 *	@retval	无
 */
void IIC_Ack(void)	
{
	IIC_SCL_OUT_LOW;
	IIC_SDA_OUT();		/*SDA为输出*/
	IIC_SDA_OUT_LOW;
	delay_us(2);
	IIC_SCL_OUT_HIGH;
	delay_us(5);
	IIC_SCL_OUT_LOW;
}

/**
 *	@brief	不产生EEPROM I2C应答信号
 *	@param	无
 *	@retval	无
 */
void IIC_NoAck(void)
{
	IIC_SCL_OUT_LOW;
	IIC_SDA_OUT();		/*SDA为输出*/
	IIC_SDA_OUT_HIGH;
	delay_us(2);
	IIC_SCL_OUT_HIGH;
	delay_us(5);
	IIC_SCL_OUT_LOW;
}

/**
 *	@brief	EEPROM I2C发送一个字节
 *	@param	Tx:要发送的数据
 *	@retval	无
 */
void IIC_SendByte(u8 Tx)
{
	u8 i;
	IIC_SDA_OUT();		/*SDA为输出*/
	IIC_SCL_OUT_LOW;
	for(i = 0; i < 8; i++)
	{
		if((Tx&0x80)>>7)	IIC_SDA_OUT_HIGH;
		else 				IIC_SDA_OUT_LOW;
		Tx <<= 1;
		delay_us(2);
		IIC_SCL_OUT_HIGH;
		delay_us(2);
		IIC_SCL_OUT_LOW;
		delay_us(2);
	}
}

/**
 *	@brief	EEPROM I2C发送一个字节
 *	@param	Ack:1,发送ack;0不发送ack
 *	@retval	u8,接收的数据
 */
u8 IIC_RecvByte(u8 Ack)
{
	u8 i, RecvTemp = 0;
	IIC_SDA_IN();		/*SDA为输入*/
	for(i = 0; i <8; i++)
	{
		IIC_SCL_OUT_LOW;
		delay_us(2);
		IIC_SCL_OUT_HIGH;
		RecvTemp <<= 1;
		if(IIC_SDA_READ) RecvTemp++;
		delay_us(1);
	}
	if(!Ack)	IIC_NoAck();	/*不发送ACK*/
	else		IIC_Ack();	/*发送ACK*/
	return RecvTemp;
}






