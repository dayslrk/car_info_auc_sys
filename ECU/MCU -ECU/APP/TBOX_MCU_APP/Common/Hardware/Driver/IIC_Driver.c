#include "IIC_Driver.h"
#include "delay.h"

/**
 *	@brief	�ⲿEEPROM��ʼ��
 *	@param	��
 *	@retval	��
 */
void IIC_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);	//����GPIOʱ��
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
 *	@brief	EEPROM I2C��ʼ�ź�
 *	@param	��
 *	@retval	��
 */
void IIC_Start(void)
{
	IIC_SDA_OUT();	/*SDAΪ���*/
	IIC_SDA_OUT_HIGH;
	IIC_SCL_OUT_HIGH;
	delay_us(4);
	IIC_SDA_OUT_LOW;
	delay_us(4);
	IIC_SCL_OUT_LOW;
}

/**
 *	@brief	EEPROM I2Cֹͣ�ź�
 *	@param	��
 *	@retval	��
 */
void IIC_Stop(void)
{
	IIC_SDA_OUT();		/*SDAΪ���*/
	IIC_SCL_OUT_LOW;
	IIC_SDA_OUT_LOW;
	delay_us(4);
	IIC_SCL_OUT_HIGH;
	delay_us(4);
	IIC_SDA_OUT_HIGH;	/*����I2C���߽����ź�*/
}

/**
 *	@brief	�ȴ�EEPROM I2CӦ���ź�
 *	@param	��
 *	@retval	u8,1:Ӧ��ʧ�ܣ�0��Ӧ��ɹ�
 */
u8 IIC_WaitAck(void)
{
	u8 ErrorTime = 0;
	IIC_SDA_IN();		/*SDAΪ����*/
	
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
	IIC_SCL_OUT_LOW;/*ʱ�����0*/
	return 0;
}

/**
 *	@brief	����EEPROM I2CӦ���ź�
 *	@param	��
 *	@retval	��
 */
void IIC_Ack(void)	
{
	IIC_SCL_OUT_LOW;
	IIC_SDA_OUT();		/*SDAΪ���*/
	IIC_SDA_OUT_LOW;
	delay_us(2);
	IIC_SCL_OUT_HIGH;
	delay_us(5);
	IIC_SCL_OUT_LOW;
}

/**
 *	@brief	������EEPROM I2CӦ���ź�
 *	@param	��
 *	@retval	��
 */
void IIC_NoAck(void)
{
	IIC_SCL_OUT_LOW;
	IIC_SDA_OUT();		/*SDAΪ���*/
	IIC_SDA_OUT_HIGH;
	delay_us(2);
	IIC_SCL_OUT_HIGH;
	delay_us(5);
	IIC_SCL_OUT_LOW;
}

/**
 *	@brief	EEPROM I2C����һ���ֽ�
 *	@param	Tx:Ҫ���͵�����
 *	@retval	��
 */
void IIC_SendByte(u8 Tx)
{
	u8 i;
	IIC_SDA_OUT();		/*SDAΪ���*/
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
 *	@brief	EEPROM I2C����һ���ֽ�
 *	@param	Ack:1,����ack;0������ack
 *	@retval	u8,���յ�����
 */
u8 IIC_RecvByte(u8 Ack)
{
	u8 i, RecvTemp = 0;
	IIC_SDA_IN();		/*SDAΪ����*/
	for(i = 0; i <8; i++)
	{
		IIC_SCL_OUT_LOW;
		delay_us(2);
		IIC_SCL_OUT_HIGH;
		RecvTemp <<= 1;
		if(IIC_SDA_READ) RecvTemp++;
		delay_us(1);
	}
	if(!Ack)	IIC_NoAck();	/*������ACK*/
	else		IIC_Ack();	/*����ACK*/
	return RecvTemp;
}






