
#ifndef _ROUTING_H_
#define _ROUTING_H_

#include "BaseType.h"
#include "CAN.h"

#define ROUTINGMESSAGECOUNT (13u)

typedef enum
{
    PERIODIC = 0,
    ON_EVENT,
    IF_ACTIVE,
    PERIODIC_ONEVENT,
    PERIODIC_IFACTIVE
}RoutingType_enum;
    

struct RoutingMessage
{
    u16                 CANID;/*帧ID*/
    bool                Valid;/*是否有效*/
    u8                  Length;/*长度*/
    u16                 Cycle;/*周期*/
    RoutingType_enum    Type;/*路由类型*/
    u8                  ReceiveChannel;/*receiver*/
    u8                  SendChannel;/*sender*/
};
typedef struct RoutingMessage RoutingMessage_Type, *pRoutingMessage_Type;

struct RoutingContent
{
    u8 RecValue[8];/*存放接收的can数据*/
    u8 Default[8];/*存放帧的默认值*/
    u8 LoseCount;/*丢失次数*/
    u16 WaitCount;/*用于周期发送的指标*/
};
typedef struct RoutingContent RoutingContent_Type;

void Routing_RxIndication(u8 Channel, pCAN_Frame_Type Message);
void Routing_MainFunction(void);


#endif

