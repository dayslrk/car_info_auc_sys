#include "Routing.h"
#include "CanQueue.h"
#include "comm_if.h"
#include "Trans.h"

#include "string.h"


//3,4,6,8,9,a
const RoutingMessage_Type RoutingFrame[] = 
{
    {0x193, TRUE, 8, 500, PERIODIC, CAN_CHANEEL1, MPU_CHANEEL1},
    {0x194, TRUE, 8, 500, PERIODIC, CAN_CHANEEL1, MPU_CHANEEL1},
    {0x196, TRUE, 8, 500, PERIODIC, CAN_CHANEEL1, MPU_CHANEEL1},
    {0x198, TRUE, 8, 500, PERIODIC, CAN_CHANEEL1, MPU_CHANEEL1},
    {0x199, TRUE, 8, 500, PERIODIC, CAN_CHANEEL1, MPU_CHANEEL1},
    {0x19a, TRUE, 8, 500, PERIODIC, CAN_CHANEEL1, MPU_CHANEEL1},
    {0x000, FALSE, 0, 0, PERIODIC, CAN_CHANEEL1, MPU_CHANEEL1},
};/*路由配置表（帧ID，是否有效，长度，周期，路由类型，receiver，sender）*/
    
static RoutingContent_Type RoutingFrameContent[sizeof(RoutingFrame)/sizeof(RoutingMessage_Type)] = 
{
    {{0, 0, 0, 0, 0, 0, 0, 0}, {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}, 0, 0},
    {{0, 0, 0, 0, 0, 0, 0, 0}, {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}, 0, 0},
    {{0, 0, 0, 0, 0, 0, 0, 0}, {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}, 0, 0},
    {{0, 0, 0, 0, 0, 0, 0, 0}, {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}, 0, 0},
    {{0, 0, 0, 0, 0, 0, 0, 0}, {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}, 0, 0},
    {{0, 0, 0, 0, 0, 0, 0, 0}, {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}, 0, 0},
    {{0, 0, 0, 0, 0, 0, 0, 0}, {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}, 0, 0},
   
};/*路由帧内容（接收值，默认值，丢失次数，周期发送指标）*/

//路由到MPU的报文只有6帧 长度6*（2+1+8）
static u16 RoutingMpuLen = 0x00;
static u8 RoutingMpuBuff[256]= {0};




void Routing_RxIndication(u8 Channel, pCAN_Frame_Type Message)
{
    u8 Index = 0;
    u8 i = 0;

    for (Index = 0; 0x000 != RoutingFrame[Index].CANID; ++Index)/*查找接收数据的配置表索引*/
    {
    	if(Channel != RoutingFrame[Index].ReceiveChannel)
    	{
    		continue;
    	}
    	
        if (Message->ID == RoutingFrame[Index].CANID)
        {
            break;
        }
    }

	if(0x000 == RoutingFrame[Index].CANID)
	{
		return;
	}
	
    switch (RoutingFrame[Index].Type)/*判断路由帧的类型（由于底层判断过该帧是否存在，因此此处无需判断路由帧不存在）*/
    {
        case PERIODIC:/*周期路由类型帧，只需将其接收值保存，并刷新丢失次数值*/
            RoutingFrameContent[Index].LoseCount = 0;
            for (i = 0; i < 8; ++i)
            {
                RoutingFrameContent[Index].RecValue[i] = Message->Byte[i];
            }
            
            break;
        case ON_EVENT:/*事件路由类型，立即路由*/
        
        	break;
        default:
            break;
    }
}


void Routing_MainFunction(void)
{
    u8 Index = 0;
    u8 i = 0;
    CAN_Frame_Type Message;

	RoutingMpuLen = 0;

    for (Index = 0; 0x000 != RoutingFrame[Index].CANID; ++Index)/*遍历配置表，发送周期路由帧*/
    {
        RoutingFrameContent[Index].WaitCount++;
        
        if (PERIODIC == RoutingFrame[Index].Type && TRUE == RoutingFrame[Index].Valid)/*周期路由类型帧*/
        {
            if (RoutingFrameContent[Index].WaitCount * 10 >= RoutingFrame[Index].Cycle)/*周期已达到*/
            {
                RoutingFrameContent[Index].WaitCount = 0;/*计时次数重新开始*/
                RoutingFrameContent[Index].LoseCount++;/*丢失次数自加*/
                
                if (RoutingFrameContent[Index].LoseCount > 3)/*周期路由帧丢失三个周期以上，发送默认值*/
                {
                	RoutingFrameContent[Index].LoseCount = 3;		//防止溢出
                	
                    for (i = 0; i < RoutingFrame[Index].Length; ++i)
                    {
                        Message.Byte[i] = RoutingFrameContent[Index].Default[i];
                    }
                }
                else/*周期路由帧未丢失三个周期以上，发送最后一次接收值*/
                {
                    for (i = 0; i < RoutingFrame[Index].Length; ++i)
                    {
                        Message.Byte[i] = RoutingFrameContent[Index].RecValue[i];
                    }
                }
                Message.ID = RoutingFrame[Index].CANID;/*填充发送数据内容*/
                Message.Len = RoutingFrame[Index].Length;

				if(MPU_CHANEEL1 == RoutingFrame[Index].SendChannel)
				{
					RoutingMpuBuff[RoutingMpuLen++] = Message.ID >> 8;
					RoutingMpuBuff[RoutingMpuLen++] = Message.ID;
					RoutingMpuBuff[RoutingMpuLen++] = Message.Len;
					memcpy(RoutingMpuBuff+RoutingMpuLen , Message.Byte, 8);
					RoutingMpuLen += 8;
					
                }
                else
                {
                	CAN_SendData(RoutingFrame[Index].SendChannel, &Message);
                }
            }
            else
            {
                ;
            }
        }
        else
        {
            ;
        }
    }
    //数据发送
    if(RoutingMpuLen > 0)
    {
    	SendData(0x00, EN_MCU_APP2, EN_MPU_APP2, RoutingMpuLen, RoutingMpuBuff);
    }
}
