#ifndef COMM_IF_H
#define COMM_IF_H

#include "BaseType.h"

/*************************************************/
#define MSGQ_MAX_BUF  			2048


typedef struct
{
   u32 mlen;
   u8 apptype;					//发送时代表接收者，接收时代表发送者
   u8 mbuff[MSGQ_MAX_BUF];
}MSG_QueueBuf;


typedef struct
{
   long mtype;
   MSG_QueueBuf    mdata;
}MSG_Queue;


/*************************************************/
//APP类型为偶数，可直接 & 0xFE
typedef enum
{
	EN_UNKNOW_APP = 0,
	EN_MCU_APP1 = 0x02,			//MCU诊断，谁触发应答给谁
	EN_MCU_APP2 = 0x04,			//CAN报文路由，IDService 复用
	EN_MCU_APP3 = 0x06,			//MPU版本号下发,
	EN_MCU_APP4 = 0x08,			//电源管理，KL15、关机信号发给EN_MPU_APP6,升级等和EN_MPU_APP5通信
	EN_MCU_APP5 = 0x0A,

	EN_MPU_APP1 = 0x80,			//Master->刷写引擎
	EN_MPU_APP2 = 0x82,			//Master->CAN信号接收,can报文路由
	EN_MPU_APP3 = 0x84,			//Master->DID采集
	EN_MPU_APP4 = 0x86,			//Master->开始/结束升级通知
	EN_MPU_APP5 = 0x88,			//Slave ->MPU_Version，节点自升级
	EN_MPU_APP6 = 0x8A,			//Manage->KL15、关机信号 
	EN_MPU_APP7 = 0x8C,         
	EN_MPU_APP8 = 0x8E,         
	EN_MPU_APP9 = 0x90,         //通用，无需接收应答的
}EN_APPS;




#endif /* COMM_IF_H */

