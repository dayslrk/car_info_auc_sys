#include "UDS_Diag_Client.h"
#include "ISO15765_2.h"
#include "Timer_Driver.h"
#include "string.h"
#include "Trans.h"

#include "Logging.h"


#define MAX_DIAG_TASK_NUM         2
static ST_UDS_DIAG_TASK diagtask[MAX_DIAG_TASK_NUM] ={0};

#define MAX_TIMEOUT         2000            //超时时间2秒，1个tick 1ms        



//添加从MPU来的诊断服务请求
void AddUdsDiagTaskFromMpu(const u32 CanId, const u16 len, const u8 *pdata, u8 apptype)
{
    int i = 0;
    for(i = 0; i < MAX_DIAG_TASK_NUM; i++)
    {
        if(0 == diagtask[i].TaskSta)
        {
            break;
        }
    }

    if(i >= MAX_DIAG_TASK_NUM)
    {
        //TODO:打印日志，当前buff已满
        return;
    }

    diagtask[i].TaskSta = 1;        //有任务
    diagtask[i].CanId = CanId;
    diagtask[i].SID = pdata[0];
    diagtask[i].apptype = apptype;
    diagtask[i].DataLen = len;
   
  
    memcpy(diagtask[i].Data, pdata, len);
}


 void UdsDiagTaskInit()
 {
     memset(diagtask, 0x00, sizeof(diagtask));
 }


void GetTaskSta(ST_UDS_DIAG_TASK *ptask)
{
    EN_CHAN min_chan=EN_CHAN_UNKNOW, max_chan=EN_CHAN_UNKNOW;
    u8 Flag= 1;        //处理完成
    
    //TODO：根据task的canid获取其他ECU的ID
    
    switch(ptask->CanId)
    {
        case DIAG_FUNC_ADDR:
            //检查除7df，711以外的所有buff
            min_chan = EN_CHAN_GW_RES;
            max_chan = EN_CHAN_BCM_RES;
            break;
        case DIAG_GW_ADDR:
            min_chan = EN_CHAN_GW_RES;
            max_chan = EN_CHAN_GW_RES;
            break;
        case DIAG_IPK_ADDR:
            min_chan = EN_CHAN_IPK_RES;
            max_chan = EN_CHAN_IPK_RES;
            break;

        case DIAG_IVI_ADDR:
            min_chan = EN_CHAN_IVI_RES;
            max_chan = EN_CHAN_IVI_RES;
            break;
        case DIAG_BCM_ADDR:
            min_chan = EN_CHAN_BCM_RES;
            max_chan = EN_CHAN_BCM_RES;
            break;
        default:
            break;
    }

    while((min_chan <= max_chan) && (min_chan < EN_CHAN_MAX_NUM ))
    {
        //检查每个是否处理完成
        if( 2 != CAN_Channel.ChanRecv[min_chan].Sta & 
            3 != CAN_Channel.ChanRecv[min_chan].Sta &
            4 != CAN_Channel.ChanRecv[min_chan].Sta)
        {
            Flag = 0;
        }

        if(2 == CAN_Channel.ChanRecv[min_chan].Sta)
        {
            //如果处理完成
            //发送应答数据给MPU
            SendData_EN_MCU_APP1((EN_APPS)ptask->apptype, CAN_Channel.ChanRecv[min_chan].CanId, CAN_Channel.ChanRecv[min_chan].len, CAN_Channel.ChanRecv[min_chan].Data );

            CAN_Channel.ChanRecv[min_chan].Sta = 5;
        }
        
        min_chan++;
    }

    if((TIMx_GetCounter() >= (ptask->Tick + MAX_TIMEOUT)) || (1 == Flag))
    {
        //超时或者接受完成,回归初始状态
        CAN_Channel.ChanSend.Sta = 0;		//超时，清空发送队列
        ptask->TaskSta = 0;    				//超时，删除任务
      
    }
    
}


//UDS诊断任务主循环,次函数中不可处理有延时的函数，应异步处理
void UdsDiagTaskMain()
{
    int i = 0;

    for(i = 0; i < MAX_DIAG_TASK_NUM; i++)
    {
            switch(diagtask[i].TaskSta)
            {
                case 1:            //有任务
                        //在此发送任务
                        if(TRUE == ISO15765_MakePacket(diagtask[i].CanId,diagtask[i].Data, diagtask[i].DataLen ))
                        {
                            //任务添加成功
                            diagtask[i].TaskSta = 2;
                        }

                        break;
                case 2:            //任务采集中
                        //在此检查TP的接收队列，检查是否诊断完成，诊断完成接收数据，并将状态设置为3
                        if(4 == CAN_Channel.ChanSend.Sta )
                        {
                            //发送完成，等待接收
                            diagtask[i].TaskSta = 3;
                            diagtask[i].Tick = TIMx_GetCounter();
                        }
                        break;
                case 3:
                        //判断任务是否接受完成，如果接受完成，则应答给MPU。
                           
                        GetTaskSta(&(diagtask[i]));
                        
                        break;
                default:        //未知状态，不予处理
                        break;
            }
    }
    
}

