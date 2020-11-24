#ifndef _IIC_DRIVER_H
#define _IIC_DRIVER_H

#include "system.h"

#define IIC_PORT		GPIOB
#define IIC_SCL_PIN		GPIO_Pin_8
#define IIC_SDA_PIN		GPIO_Pin_9


#define IIC_SCL_OUT_LOW		IIC_PORT->BSRRH=IIC_SCL_PIN
#define IIC_SCL_OUT_HIGH	IIC_PORT->BSRRL=IIC_SCL_PIN
		
#define IIC_SDA_OUT_LOW		IIC_PORT->BSRRH=IIC_SDA_PIN
#define IIC_SDA_OUT_HIGH	IIC_PORT->BSRRL=IIC_SDA_PIN
		
#define IIC_SDA_READ		IIC_PORT->IDR&IIC_SDA_PIN

//IO方向设置
#define IIC_SDA_IN()  {IIC_PORT->MODER&=~(3<<(9*2));IIC_PORT->MODER|=0<<9*2;} //PH5输入模式
#define IIC_SDA_OUT() {IIC_PORT->MODER&=~(3<<(9*2));IIC_PORT->MODER|=1<<9*2;} //PH5输出模式

void IIC_Init(void);
void IIC_Start(void);
void IIC_Stop(void);
u8 IIC_WaitAck(void);
void IIC_Ack(void);
void IIC_NoAck(void);
void IIC_SendByte(u8 Tx);
u8 IIC_RecvByte(u8 Ack);

#endif /*_IIC_DRIVER_H*/
