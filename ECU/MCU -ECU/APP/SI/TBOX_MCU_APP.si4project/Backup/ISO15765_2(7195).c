#include "ISO15765_2.h"
#include "Timer.h"
#include "ISO14229_1.h"
//#include "Com.h"
#include "Logging.h"
#include "string.h"

u8 SendFinishedFlag = TRUE;
DiagTimer_Type TxMsgTimer;
CAN_Channel_Type   CAN_Channel;/*CAN总线：包含总线参数接收和发送的frame数据*/
/*static u8 FlowControlTimeout = 250;等待流控帧250 * 20ms，仍无流控帧接收到，则放弃连续帧的发送*/

void ISO15765_CheckFrame(CHANNEL_RECV *pRecv);

/*******************************************************************************
* 函数名称: ISO15765_Init
* 功能描述: ISO15765协议初始化，用于初始化相关的参数
* 输入参数: 无 
* 输出参数: 无
* 返回参数: 无
* 备    注：
*******************************************************************************/
void ISO15765_Init(void)
{
    /*为CAN总线上的参数赋值（发送CANID、接收CANID、总线状态）*/
    CAN_Channel.Channel = CAN_CHANNEL1;
    CAN_Channel.CANRID = DIAGCANRXID;
    CAN_Channel.FUNCID = DIAGFUNCRXID;
    CAN_Channel.Statue = CAN_INIT_OK;

     /*CAN总线的接收发送buffer清空*/
    memset(&(CAN_Channel.ChanSend),0x00, sizeof(CAN_Channel.ChanSend) );
    memset(&(CAN_Channel.ChanRecv), 0x00, sizeof(CAN_Channel.ChanRecv));
    

    SendFinishedFlag = TRUE;
    /*FlowControlTimeout = 250;*/
} 


/*******************************************************************************
* 函数名称: ISO15765_ReceivePacket
* 功能描述: ISO5765数据链路层数据包的构造函数,并将数据包接收到buffer
* 输入参数: nChannel 通道名
* 输出参数: 无
* 返回参数: 无
*******************************************************************************/
void ISO15765_ReceivePacket(u8 Channel, pCAN_Frame_Type pCANFrame)
{
    u8 i = 0;
    CAN_Frame_Type CANFrame;
    EN_CHAN en_chan_addr = EN_CHAN_UNKNOW;
    u32 FC_CanId = 0;

    if (NULL == pCANFrame)
    {
        return;
    }
    else
    {
        CANFrame = *pCANFrame;/*防止参数被修改*/
    }

    do{
    
        /*总线初始化成功，且接收诊断帧来自诊断CAN总线*/
        if (CAN_INIT_OK != CAN_Channel.Statue || Channel != CAN_Channel.Channel)
        {
            break;
        }

        //Check CANID 
        switch(pCANFrame->ID)
        {
            case DIAG_FUNC_ADDR:
                /* 功能地址诊断请求 */
                en_chan_addr = EN_CHAN_FUNC_REQ;
                FC_CanId = DIAG_TBOX_ADDR | 0x08;
                CAN_Channel.ChanRecv[en_chan_addr].FC_Flag = 1;        //需要应答流控
                break;

            case DIAG_TBOX_ADDR:    
                /* TBOX诊断请求 */
                en_chan_addr = EN_CHAN_TBOX_REQ;
                FC_CanId = DIAG_TBOX_ADDR | 0x08;
                CAN_Channel.ChanRecv[en_chan_addr].FC_Flag = 1;     //需要应答流控
                break;

            case (DIAG_GW_ADDR | 0x08):
                /* 网关诊断应答 */
                en_chan_addr = EN_CHAN_GW_RES;
                FC_CanId = DIAG_GW_ADDR;
                if((4 ==CAN_Channel.ChanSend.Sta) && 
                   ((pCANFrame->ID & 0x7F7) == CAN_Channel.ChanSend.CanId))
                {
                    CAN_Channel.ChanRecv[en_chan_addr].FC_Flag = 1;     //需要应答流控
                }
                
                break;

            case (DIAG_IPK_ADDR | 0x08):
                /* 仪表诊断应答 */
                en_chan_addr = EN_CHAN_IPK_RES;
                FC_CanId = DIAG_IPK_ADDR;
                if((4 ==CAN_Channel.ChanSend.Sta) && 
                   ((pCANFrame->ID & 0x7F7) == CAN_Channel.ChanSend.CanId))
                {
                    CAN_Channel.ChanRecv[en_chan_addr].FC_Flag = 1;     //需要应答流控
                }
                break;

            case (DIAG_IVI_ADDR | 0x08):
                en_chan_addr = EN_CHAN_IVI_RES;
                FC_CanId = DIAG_IVI_ADDR;
                if((4 ==CAN_Channel.ChanSend.Sta) && 
                   ((pCANFrame->ID & 0x7F7) == CAN_Channel.ChanSend.CanId))
                {
                    CAN_Channel.ChanRecv[en_chan_addr].FC_Flag = 1;     //需要应答流控
                }
                break;

            case (DIAG_BCM_ADDR | 0x08):
                en_chan_addr = EN_CHAN_BCM_RES;
                FC_CanId = DIAG_BCM_ADDR;
                if((4 ==CAN_Channel.ChanSend.Sta) && 
                   ((pCANFrame->ID & 0x7F7) == CAN_Channel.ChanSend.CanId))
                {
                    CAN_Channel.ChanRecv[en_chan_addr].FC_Flag = 1;     //需要应答流控
                }
                break;

            default:
                en_chan_addr = EN_CHAN_UNKNOW;
                break;
            
                
        }

        if(EN_CHAN_UNKNOW == en_chan_addr)
        {
            //未知CAN ID，退出
            break;
        }

        //在此处对特殊信息进行屏蔽，例如3E 80、7F XX 78等
        if((CANFrame.Byte[0] < 0x08) &&
            (CANFrame.Byte[1] == 0x7F) &&
            (CANFrame.Byte[3] == 0x78))
        {
            //TODO:在此更新相关定时器
            break;    //此包不予处理
        }

        if((CANFrame.Byte[0] < 0x08) &&
            (CANFrame.Byte[1] == 0x3E))
        {
            //TODO:在此更新相关定时器
            break;    //此包不予处理
        }
        
        
        switch (CANFrame.Byte[0] >> 4)
        {
            case SF_FLAG:
                CAN_Channel.ChanRecv[en_chan_addr].CanId = CANFrame.ID;
                CAN_Channel.ChanRecv[en_chan_addr].Sta = 2;        //单帧，已接收完成
                CAN_Channel.ChanRecv[en_chan_addr].len = CANFrame.Byte[0] & 0x0F;

                memcpy(CAN_Channel.ChanRecv[en_chan_addr].Data, &(CANFrame.Byte[1]), 7);
             
                /*数据接收完成后,检查是否需要tbox处理，如需处理则上报请求到uds层*/
                ISO15765_CheckFrame(&(CAN_Channel.ChanRecv[en_chan_addr]));
                break;
            case FF_FLAG:
                CAN_Channel.ChanRecv[en_chan_addr].CanId = CANFrame.ID;
                CAN_Channel.ChanRecv[en_chan_addr].Sta = 1;        //首帧，正在接收中
                CAN_Channel.ChanRecv[en_chan_addr].len = ((CANFrame.Byte[0] & 0x0F) << 8) | CANFrame.Byte[1];
          
                memcpy(CAN_Channel.ChanRecv[en_chan_addr].Data , &(CANFrame.Byte[2]), 6);
                CAN_Channel.ChanRecv[en_chan_addr].CurIndex = 6;        //首帧只有6个字节。
                CAN_Channel.ChanRecv[en_chan_addr].CurSN = 0;            //当前第0包

                if(1 != CAN_Channel.ChanRecv[en_chan_addr].FC_Flag)
                {
                    //不需要应答流控帧
                    break;
                }
                
               
                /*发送流控帧*/
                for (i = 0; i < 8; ++i)
                {
                    CANFrame.Byte[i] = 0;/*清空CANFrame中的数据*/
                }

                CANFrame.ID = FC_CanId;
               
                CANFrame.Byte[0] = 0x30;
                CANFrame.Byte[1] = 0x00;
                CANFrame.Byte[2] = 0x05;
                CANFrame.Len = 8;

                i = 0;
                while (CAN_SendData(CAN_Channel.Channel, &CANFrame) && i < FC_RESEND_COUNT)
                {
                     i++;
                }
                break;
                
            case CF_FLAG:
                if( 1 != CAN_Channel.ChanRecv[en_chan_addr].Sta)
                {
                    /* 如果当前通道非正在接收中，则丢弃该数据 */
                    break;
                }
            
                CAN_Channel.ChanRecv[en_chan_addr].CurSN ++;
                
                /* 接收CANFrame的BS_SN和当前接收的SN不相等，接收错误有漏帧的情况，丢弃该帧 */
                if (CANFrame.CF.CF_SN != (CAN_Channel.ChanRecv[en_chan_addr].CurSN & 0x0f))
                {
                    //数据接收失败，停止接收数据
                    CAN_Channel.ChanRecv[en_chan_addr].Sta = 3;        
                }
                else/*SN正确*/
                {
                    //第一个字节非有效数据，从第二个数据开始
                    for (i = 1; i <= 7; ++i)
                    {
                        /*当前CAN_Channel的接收buffer已满*/
                        if(CAN_Channel.ChanRecv[en_chan_addr].CurIndex >= MAX_UDS_RCV_Buff_Size)
                        {
                            //当前接收失败
                            CAN_Channel.ChanRecv[en_chan_addr].Sta = 3;
                            break; 
                        }
                        
                        CAN_Channel.ChanRecv[en_chan_addr].Data[CAN_Channel.ChanRecv[en_chan_addr].CurIndex] = CANFrame.CF.Bytes[i];
                       
                        CAN_Channel.ChanRecv[en_chan_addr].CurIndex++;

                        if(CAN_Channel.ChanRecv[en_chan_addr].CurIndex == CAN_Channel.ChanRecv[en_chan_addr].len)
                        {
                            CAN_Channel.ChanRecv[en_chan_addr].Sta = 2;
                            break;
                        }
                    }

                    if(1 != CAN_Channel.ChanRecv[en_chan_addr].Sta)
                    {
#if 1   //测试代码             
                if(0x74 == CAN_Channel.ChanRecv[en_chan_addr].Data[0])
                {
                    Log(LOG_INFO,"\n");
                    break;
                }
#endif   
                           //当前处理结束，酌情通知上层
                        ISO15765_CheckFrame(&(CAN_Channel.ChanRecv[en_chan_addr]));
                    }
                }
                break;
            case FC_FLAG:
                //TODO：检查是否和当前发送的数据匹配
                if(3 != CAN_Channel.ChanSend.Sta)
                {
                    //如果当前不是等待流控，则不处理
                }

                if((CAN_Channel.ChanSend.CanId == CANFrame.ID | 0x08) ||
                    (CANFrame.ID == CAN_Channel.ChanSend.CanId))
                {
                    CAN_Channel.ChanSend.Sta = 2;    //已收到流控信息，继续发送

                    CAN_Channel.ChanSend.BlockSize =  CANFrame.FC.FC_BS;
                    CAN_Channel.ChanSend.sTime = CANFrame.FC.FC_STmin;

                       CAN_Channel.ChanSend.SN = 1;
                 
                }
                
                break;
            default:
                /*接收诊断帧出错*/
                break;
        }
            
        
    
    }while(0);


}


/*******************************************************************************
* 函数名称: ISO15765_CheckFrame
* 功能描述: 一帧数据接收完毕，检查接收数据的完整性并将数据发送给上层
* 输入参数: 无
* 输出参数: 无
* 返回参数: 无
*******************************************************************************/
void ISO15765_CheckFrame(CHANNEL_RECV *pRecv)
{
    u8 FrameType = 0;
    
    if( (pRecv->CanId == DIAG_TBOX_ADDR || pRecv->CanId == DIAG_FUNC_ADDR) &&
        (pRecv->Sta == 2) )
    {
        FrameType = pRecv->CanId == DIAG_TBOX_ADDR ? DIAG_PHY: DIAG_FUNC;

        /* 从CAN线来的诊断请求 ,CAN总线来的数据，apptype随意填*/
        UDS_RxIndication(pRecv->Data, pRecv->len, FrameType, 1, 0);
    }

}


/*******************************************************************************
* 函数名称: ISO15765_SendPacket
* 功能描述: ISO5765数据链路层数据包的构造函数,并将数据包发送到总线
* 输入参数: nChannel 通道名
* 输出参数: 无
* 返回参数: 无
*******************************************************************************/
void ISO15765_SendPacket(void)
{
    u8 i = 0;
    CAN_Frame_Type CANFrame;/*存储发送数据的零时变量，交给CAN底层发送*/
    /*u8 TxResult = Tx_FAILURE;*/

    /*清空发送数据结构体*/
    for (i = 0; i < 8; ++i)
    {
        CANFrame.Byte[i] = 0;
    }
    
    do{
        if(CANINITOK != CAN_Channel.Statue)
        {
            break;
        }

        if(1 != CAN_Channel.ChanSend.Sta && 2 != CAN_Channel.ChanSend.Sta)
        {
            //如果当前未就绪或者未在正在发送中，则本次不处理
            break;
        }

        if(1 == CAN_Channel.ChanSend.Sta)
        {
            //清空对应的接收缓冲区状态，
            CAN_Channel.ChanSend.Index = 0;
            CAN_Channel.ChanSend.SN = 1;

            //清空获取接收缓冲区状态
            switch(CAN_Channel.ChanSend.CanId)
            {
                case DIAG_FUNC_ADDR:
                    //发送7DF的诊断请求，清空所有接收缓存区
                    memset(CAN_Channel.ChanRecv, 0x00, sizeof(CAN_Channel.ChanRecv));
                    
                    break;
                case (DIAG_TBOX_ADDR | 0x08):
                    //发送TBOX的诊断应答，清空TBOX的诊断请求缓冲区
                    memset(&(CAN_Channel.ChanRecv[EN_CHAN_TBOX_REQ]), 0x00, sizeof(CAN_Channel.ChanRecv[EN_CHAN_TBOX_REQ]));

                    break;
                case DIAG_GW_ADDR:
                    //发送GW的诊断请求，清空GW的接收缓存区
                    memset(&(CAN_Channel.ChanRecv[EN_CHAN_GW_RES]), 0x00, sizeof(CAN_Channel.ChanRecv[EN_CHAN_GW_RES]));
                    break;
                case DIAG_IPK_ADDR:
                    //发送IPK的诊断请求，清空IPK的接收缓存区
                    memset(&(CAN_Channel.ChanRecv[EN_CHAN_IPK_RES]), 0x00, sizeof(CAN_Channel.ChanRecv[EN_CHAN_IPK_RES]));
                    break;
                case DIAG_IVI_ADDR:
                    //发送IVI的诊断请求，清空IVI的接收缓存区
                    memset(&(CAN_Channel.ChanRecv[EN_CHAN_IVI_RES]), 0x00, sizeof(CAN_Channel.ChanRecv[EN_CHAN_IVI_RES]));
                    break;
                case DIAG_BCM_ADDR:
                    //发送BCM的诊断请求，清空BCM的接收缓存区
                    memset(&(CAN_Channel.ChanRecv[EN_CHAN_BCM_RES]), 0x00, sizeof(CAN_Channel.ChanRecv[EN_CHAN_BCM_RES]));
                    break;
                default:
                    break;
                    
            }

            //开始发送首帧            
            if(CAN_Channel.ChanSend.len > 7)
            {
                //发送首帧
                CANFrame.Len = 8;
                CANFrame.Byte[0] = 0x10 + ((CAN_Channel.ChanSend.len >> 8) & 0x0f);
                CANFrame.Byte[1] = CAN_Channel.ChanSend.len & 0xff;
                memcpy(&(CANFrame.Byte[2]), CAN_Channel.ChanSend.Data, 6);
                CAN_Channel.ChanSend.Index = 6;
                CANFrame.ID = CAN_Channel.ChanSend.CanId;

                if (CAN_SendData(CAN_Channel.Channel, &CANFrame))
                {
                   //发送失败
                       CAN_Channel.ChanSend.RetryTimes++;
                       if(CAN_Channel.ChanSend.RetryTimes > MAX_RETRYTIMES)
                    {
                        CAN_Channel.ChanSend.Sta = 4;        //发送失败，发送完成
                        CAN_Channel.ChanSend.RetryTimes = 0;
                    }
                    
                }
                else
                {
                    //发送成功
                    CAN_Channel.ChanSend.RetryTimes = 0;
                    CAN_Channel.ChanSend.Sta = 3;            //等待流控帧
                } 
            }
            else
            {
                //发送单帧
                CANFrame.Len = 8;
                
                CANFrame.Byte[0] = CAN_Channel.ChanSend.len & 0x0f;
                memcpy(&(CANFrame.Byte[1]), CAN_Channel.ChanSend.Data, CANFrame.Byte[0] );
                CAN_Channel.ChanSend.Index = CANFrame.Byte[0];
                CANFrame.ID = CAN_Channel.ChanSend.CanId;

                if (CAN_SendData(CAN_Channel.Channel, &CANFrame))
                {
                   //发送失败
                       CAN_Channel.ChanSend.RetryTimes++;
                    
                    if(CAN_Channel.ChanSend.RetryTimes > MAX_RETRYTIMES)
                    {
                        CAN_Channel.ChanSend.Sta = 4;        //发送失败，发送完成
                        CAN_Channel.ChanSend.RetryTimes = 0;
                        UDS_TxConfirmation(CAN_Channel.ChanSend.CanId,CAN_Channel.ChanSend.Data[0],CAN_Channel.ChanSend.Data[1], Tx_FAILURE);
                    }
                }
                else
                {
                    //单帧发送成功，
                    CAN_Channel.ChanSend.RetryTimes = 0;
                    UDS_TxConfirmation(CAN_Channel.ChanSend.CanId,CAN_Channel.ChanSend.Data[0],CAN_Channel.ChanSend.Data[1], Tx_SUCCESS);
                    CAN_Channel.ChanSend.Sta = 4;            //已发送完成
                }

                
              
            }
            
        }
        else
        {
            //非首次发送，本次直接发送所有后续帧，直到发送失败（可能是CAN发送邮箱已满）
            while(CAN_Channel.ChanSend.Index < CAN_Channel.ChanSend.len)
            {
                //循环发送，直到写满can发送缓存区，此部分不可加延时
                CANFrame.Len = 8;

                CANFrame.Byte[0]= 0x20 + (CAN_Channel.ChanSend.SN & 0x0F);
                memcpy(&(CANFrame.Byte[1]), &(CAN_Channel.ChanSend.Data[CAN_Channel.ChanSend.Index]), 7);
                CANFrame.ID = CAN_Channel.ChanSend.CanId;
                if (CAN_SendData(CAN_Channel.Channel, &CANFrame))
                {
                    //发送失败
                    CAN_Channel.ChanSend.RetryTimes++;
                    
                    if(CAN_Channel.ChanSend.RetryTimes > MAX_RETRYTIMES)
                    {
                        CAN_Channel.ChanSend.Sta = 4;        //发送失败，发送完成
                        CAN_Channel.ChanSend.RetryTimes = 0;
                        UDS_TxConfirmation(CAN_Channel.ChanSend.CanId,CAN_Channel.ChanSend.Data[0],CAN_Channel.ChanSend.Data[1], Tx_FAILURE);
                    }
                    break;
                }
                else
                {
                
                    //发送成功Index才增加
                    CAN_Channel.ChanSend.Index += 7;    
                    CAN_Channel.ChanSend.SN++;
                    
                    CAN_Channel.ChanSend.RetryTimes = 0;
                }
            }

            if(CAN_Channel.ChanSend.Index >= CAN_Channel.ChanSend.len)
            {
                //发送完成
                UDS_TxConfirmation(CAN_Channel.ChanSend.CanId,CAN_Channel.ChanSend.Data[0],CAN_Channel.ChanSend.Data[1], Tx_SUCCESS);
                CAN_Channel.ChanSend.Sta = 4;        //发送完成
            }
        }

        //tbox被诊断的应答任务，发送完成后自动回归空闲状态
        if( (DIAG_TBOX_ADDR|0x08) == CAN_Channel.ChanSend.CanId)
        {
            if(4 == CAN_Channel.ChanSend.Sta )
            {
                CAN_Channel.ChanSend.Sta = 0;
            }
        }
        
        
    }while(0);
    
}




/*******************************************************************************
* 函数名称: ISO15765_Keepalive
* 功能描述: ISO15765数据链路层保持连接函数,由定时器管理函数调用
* 输入参数: Channel:需要初始化的通道
* 输出参数: 无
* 返回参数: 无
*******************************************************************************/
void ISO15765_Keepalive(u8 nChannel)
{
    /*诊断工具在线，3E 80*/
    u8 KeepAlive[2] = {0x3E, 0x80}; //0x3e 0x00  0x3e 0x80
    u8 SendFailedCount = 0;
    
    while(SendFailedCount < 3)
    {
        if (ISO15765_MakePacket(DIAGCANTXID, KeepAlive, 2))
        {
            break;
        }
        else
        {
            SendFailedCount++;
        }
    }
}

/*******************************************************************************
* 函数名称: ISO15765_MakePacket
* 功能描述: ISO5765数据链路层数据包的构造函数,并将数据包发送到总线
* 输入参数: SId:发送诊断服务SID   Data：发送的数据 Len：Data长度
* 输出参数: 无
* 返回参数: TRUE:开始发送 FALSE:发送失败，当前有数据正在发送
*******************************************************************************/
bool ISO15765_MakePacket(u32 CanId, u8 *Data, u16 Len)
{
    if (NULL != Data && 0 == CAN_Channel.ChanSend.Sta && Len <= MAX_UDS_RCV_Buff_Size )
    {

           CAN_Channel.ChanSend.CanId = CanId;
       
        CAN_Channel.ChanSend.len = Len;
        memcpy(CAN_Channel.ChanSend.Data, Data, Len);

        CAN_Channel.ChanSend.Sta = 1;        //数据就绪，可以发送
        
        return TRUE;
    }
    else
    {
        return FALSE;/*指针为空或当前有数据正在发送*/
    }
}


