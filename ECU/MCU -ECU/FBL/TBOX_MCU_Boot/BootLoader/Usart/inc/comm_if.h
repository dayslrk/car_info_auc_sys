#ifndef COMM_IF_H
#define COMM_IF_H

#include "BaseType.h"

/*************************************************/
#define MSGQ_MAX_BUF  			4096


typedef struct
{
   u32 mlen;
   u8 apptype;					//����ʱ��������ߣ�����ʱ��������
   u8 mbuff[MSGQ_MAX_BUF];
}MSG_QueueBuf;


typedef struct
{
   long mtype;
   MSG_QueueBuf    mdata;
}MSG_Queue;


/*************************************************/
typedef enum
{
	EN_UNKNOW_APP = 0,
	EN_MCU_APP1 = 0x02,			//MCU���
	EN_MCU_APP2 = 0x04,			//CAN����ָ���·�
	EN_MCU_APP3 = 0x06,			//MPU�汾���·�
	EN_MCU_APP4 = 0x08,			//��Դ����
	EN_MCU_APP5 = 0x0A,

	EN_MPU_APP1 = 0x80,			//Master->ˢд����
	EN_MPU_APP2 = 0x82,			//Master->CAN�źŽ���
	EN_MPU_APP3 = 0x84,			//Master->DID�ɼ�
	EN_MPU_APP4 = 0x86,			//MPU_Manage
	EN_MPU_APP5 = 0x88,			//��Դ����

}EN_APPS;



#endif /* COMM_IF_H */

