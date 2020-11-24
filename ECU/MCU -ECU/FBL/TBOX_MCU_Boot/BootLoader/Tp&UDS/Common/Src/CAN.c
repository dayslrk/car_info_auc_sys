#include "CAN.h"
#include "CanQueue.h"
#include "ISO15765_2.h"
#include "CAN1_Driver.h"
#include "CAN2_Driver.h"
//#include "InforCollect.h"

bool NormalMessageRecEnable = TRUE;
bool NormalMessageSendEnable = TRUE;

#if ROUTINGMESSAGECOUNT
extern const RoutingMessage_Type RoutingFrame[ROUTINGMESSAGECOUNT];
#endif

#if APPMESSAGECOUNT
const AppMessage_Type AppFrame[APPMESSAGECOUNT] = 
{
    {0x301, TRUE, 1, 1},
    {0x305, TRUE, 5, 1},
};
#endif



/*******************************************************************************
* 函数名称: CAN_MainFunction
* 功能描述: CAN主函数，从CanBuffer读取数据进行接收
* 输入参数: 无
* 输出参数: 无
* 返回参数: 无
*******************************************************************************/
void CAN_MainFunction(void)
{
    CAN_Msg_Type CANRxMsg;
    CAN_Frame_Type CANFrame;
    u8 Channel;

    u16 i = 0;

	while(CAN_Queue_GetRecCount())
    {
        if (!CAN_Queue_Pop(&CANRxMsg))
        {
            CANFrame.ID = CANRxMsg.ID;
            CANFrame.Len = CANRxMsg.Len;
            Channel = CANRxMsg.Channel;

            for (i = 0; i < CANRxMsg.Len; ++i)
            {
                CANFrame.Byte[i] = CANRxMsg.Data[i];
            }

            for (i = i; i < 8; ++i)/*防止局部变量未赋值数据不为0*/
            {
                CANFrame.Byte[i] = 0;
            }
            
            if (CAN_CHANNEL1 == Channel)
            {
                if( DIAGFUNCRXID == CANRxMsg.ID || DIAGCANRXID == CANRxMsg.ID)
                {
                        //当前ECU的诊断报文
                        ISO15765_ReceivePacket(Channel, &CANFrame);
                }
                else if( (0x708 == (CANRxMsg.ID & 0x708)) && (DIAGCANRXID != (CANRxMsg.ID & DIAGCANRXID)))
                {
                        //其他ECU的诊断应答报文
                        ISO15765_ReceivePacket(Channel, &CANFrame);
                }
                else if(0x700 == (CANRxMsg.ID & 0x700))
                {
                        //当前CAN总线上的其他ECU的诊断请求，忽略。可以监听处理
                }
                else
                {
                        //非诊断请求
                        CAN_RecFiltration(Channel, &CANFrame);/*CAN1非诊断帧接收*/
                }			
            }
            else
            {
                ;
            }
        }
        else
        {
            break;/*取出数据失败*/
        }
    }
   
}


/*******************************************************************************
* 函数名称: CAN_RecFiltration
* 功能描述: 判断应用报文是路由接受方式（路由或者应用）
* 输入参数: Channel:判断数据接收的can通道 CANFrame:接收的完整数据
* 输出参数: 无
* 返回参数: 无
*******************************************************************************/
void CAN_RecFiltration(u8 Channel, pCAN_Frame_Type CANFrame)
{
#if APPMESSAGECOUNT
    u8 Index = 0;

    for (Index = 0; Index < APPMESSAGECOUNT; ++Index)
    {
        if (CANFrame->ID == AppFrame[Index].CANID && TRUE == AppFrame[Index].Valid && Channel == AppFrame[Index].ReceiveChannel
            && CANFrame->Len == AppFrame[Index].Length)
        {
//            App_RxIndication(Channel, CANFrame);/*应用程序数据接收*/
            break;
        }
        else
        {
             continue;
        }
    }
#endif
}


/*******************************************************************************
* 函数名称: CAN_RX1_IRQHandler
* 功能描述: 从can的缓存中读取数据，并压入队列
* 输入参数: 无 
* 输出参数: 无
* 返回参数: 无
* 备    注：中断函数编写要精简，不易过于冗长
*******************************************************************************/
void CAN_RX1_IRQHandler(u8 Channel, CanRxMsg *RxMsg)
{
    if ((FALSE == NormalMessageRecEnable && 0x710 != RxMsg->StdId) || NULL == RxMsg)
    {
        ;/*不接收*/
    }
    else
    {
        CAN_Queue_Push(RxMsg, Channel);
    }
}


/*******************************************************************************
* 函数名称: Can_Send_Data
* 功能描述: can 链路层的发送函数，接收网络层发送过来的数据包，并发送出去
* 输入参数: 无 
* 输出参数: 无
* 返回参数: 0,成功;
         其他,失败;
* 备    注：在网络层由ISO15765_SendPacket调用
*******************************************************************************/
u8 CAN_SendData(u8 nChannel, pCAN_Frame_Type CANFrame)
{
    u16 i = 0;
    CanTxMsg TxMsg;
    u8 Result;

    if (FALSE == NormalMessageSendEnable && DIAGCANTXID != CANFrame->ID)
    {
        ;/*不发送*/
    }
    else
    {
        TxMsg.StdId = CANFrame->ID;
        TxMsg.ExtId = 0x00;

        TxMsg.IDE = CAN_Id_Standard;
        TxMsg.RTR = CAN_RTR_Data;
        TxMsg.DLC = CANFrame->Len;
        for (i = 0 ; i < CANFrame->Len; ++i)
        {
            TxMsg.Data[i] = CANFrame->Byte[i];
        }
        
        if (CANFrame->Len < 8)
        {
            for (i = i; i < 8; ++i)
            {
                TxMsg.Data[i] = 0;
            }
        }
        else
        {
            ;
        }

        if (CAN_CHANNEL1 == nChannel)
        {
            Result = CAN1Driver_SendData(&TxMsg);/*发送*/
        }
        else if (CAN_CHANNEL2 == nChannel)

        {
            Result = CAN2Driver_SendData(&TxMsg);/*发送*/
        }
    }
    
    return Result;
}


/*******************************************************************************
* 函数名称: Get_Can_Status
* 功能描述: 获取当前Can总线状态
* 输入参数: 无 
* 输出参数: 无
* 返回参数: 当前Can总线状态
* 备    注：
*******************************************************************************/
u8 Get_CAN_Status(u8 nChannel)
{
    return 0;
}


