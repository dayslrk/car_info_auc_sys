
#ifndef _QUEUE_H_
#define _QUEUE_H_

#include "BaseType.h"
#include "stm32f4xx_can.h"


#define CAN_QUEUE_BUFFER_MAXSIZE        (100U)
#define UDS_QUEUE_BUFFER_MAXSIZE        (3U)
#define MAX_UDS_RCV_Buff_Size               (1024+64U)

#define FRAME_TYPE_DIAG				(1U)
#define FRAME_TYPE_NORMAL			(2U)


#pragma pack(push)
#pragma pack(1)

struct CAN_Msg
{
    u32 ID;
    u8 Data[8];
    u8 Len;
    u8 Channel;
};
typedef struct CAN_Msg CAN_Msg_Type, *pCAN_Msg_Type;

struct CAN_Queue
{
    u16 front;
    u16 rear;

    CAN_Msg_Type Buffer[CAN_QUEUE_BUFFER_MAXSIZE];
};
typedef struct CAN_Queue CAN_Queue_Type, *pCAN_Queue_Type;


struct UDS_Msg
{
    u8  SID;
    u8  Data[MAX_UDS_RCV_Buff_Size];
    u16 Len;
    u8  FrameType;
    u8 	Channel;			//消息通道。1-CAN  ，2-MPU
    u8  apptype;			//MPU时有效
};
typedef struct UDS_Msg UDS_Msg_Type, *pUDS_Msg_Type;

struct UDS_Queue
{
    u16 front;
    u16 rear;

    UDS_Msg_Type Buffer[UDS_QUEUE_BUFFER_MAXSIZE];
};
typedef struct UDS_Queue UDS_Queue_Type, *pUDS_Queue_Type;

#pragma pack(pop)


void  CAN_Queue_Init(void);
u8    CAN_Queue_Push(const CanRxMsg *Data, u8 Channel);
u8    CAN_Queue_Pop(pCAN_Msg_Type Data);
u16   CAN_Queue_GetRecCount(void);


void  UDS_Queue_Init(void);
u8    UDS_Queue_Push(const pUDS_Msg_Type Data);
u8    UDS_Queue_Pop(pUDS_Msg_Type Data);
u16   UDS_Queue_GetRecCount(void);

#endif
