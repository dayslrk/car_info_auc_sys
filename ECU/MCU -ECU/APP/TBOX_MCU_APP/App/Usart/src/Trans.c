#include "Trans.h"
#include "Uart.h"
#include "Logging.h"
#include "escape.h"
#include <string.h>

#include "ISO14229_1.h"
#include "ISO15765_2.h"

#define MAX_QUQUE_LEN            6        


typedef enum{
    EN_TSTA_UNKNOW = 0,
    EN_TSTA_IDEL,            //空闲
    EN_TSTA_S1 = 2,                //第一次发送
    EN_TSTA_S2,                //第二次发送
    EN_TSTA_S3,                //第三次发送
    EN_TSTA_FAIL,            //发送失败
    EN_TSTA_SUCC             //发送成功
}EN_TSTA;

typedef struct{
    u8 SN;                        //消息流水
    u8 Sender;                    //消息发送者
    u8 Recver;                    //消息接受者
    u16 DLen;                    //有效数据长度
    u8 Data[MAX_S_MSG_DATA];        //有效数据内容
}ST_S_MSG;

typedef struct{
    u8 TSN;                        //传输层SN
    EN_TSTA ENState;            //消息状态
    u8 WTime;                    //超时时间                    
    ST_S_MSG msg;                    //传输层消息体
}ST_S_MSG_NODE;


typedef struct
{
    ST_S_MSG_NODE Node[MAX_QUQUE_LEN];
    u8 write;
    u8 read;
    u8 len;
}ST_SEND_QUQUE;

typedef struct
{
    u8 Sender;                //此条消息发送者
    u16 DLen;
    u8 Data[MAX_R_MSG_DATA];
}ST_R_MSG_NODE;


typedef struct
{
    ST_R_MSG_NODE Node[MAX_QUQUE_LEN];
    u8 write;
    u8 read;
    u8 len;
}ST_RECV_QUQUE;


/***********************************************************/
static ST_SEND_QUQUE send_quque[SEND_PRI_NUM];


/*
TODO:recv_quque做为一个优化项，更改为，每个队列都是指针，
根据实际数据大小指向不同的buff。例如预留4个1K，4个512，8个256,16个128的
或者实现一个动态内存分配。 
*/

static ST_RECV_QUQUE recv_quque[RECV_NUM];

static u8 TSN = 0x00;



u8 datatmp[30] = {0};
/*********************************************************************************/

u32 SendData_EN_MCU_APP1(const EN_APPS Recver, const u32 CanID, u16 DLen, const u8 *pdata)
{
    
    u32 ret = 1;

    u8 pri = 0;        //默认优先级为0
    
    do{

        if( NULL == pdata )
        {
/*            Log(LOG_DEBUG, "Param Faild.\n");*/
            ret = 1;
            break;
        }
    
        if(DLen > MAX_S_MSG_DATA)
        {
/*            Log(LOG_DEBUG, "send Buffer too small, DLen: %d, Buff len: %d.\n", DLen, MAX_MSG_DATA);*/
            ret = 1;
            break;
        }
    
        if(send_quque[pri].len >= MAX_QUQUE_LEN)
        {
/*            Log(LOG_DEBUG, "send quque too small.\n");*/
            ret = 2;
            break;
        }

        //填充待发送数据
        send_quque[pri].Node[send_quque[pri].write].ENState = EN_TSTA_IDEL;        //数据就绪状态    
        send_quque[pri].Node[send_quque[pri].write].msg.SN = 0;
        send_quque[pri].Node[send_quque[pri].write].msg.Sender = EN_MCU_APP1;
        send_quque[pri].Node[send_quque[pri].write].msg.Recver = Recver;

        if(DLen > 7)
        {
            send_quque[pri].Node[send_quque[pri].write].msg.DLen = DLen + 2+2;    //CANID,长度信息
            send_quque[pri].Node[send_quque[pri].write].msg.Data[0] = CanID >> 8;
            send_quque[pri].Node[send_quque[pri].write].msg.Data[1] = CanID;
            send_quque[pri].Node[send_quque[pri].write].msg.Data[2] = 0x10 + ((DLen>>8) &0x0F);
            send_quque[pri].Node[send_quque[pri].write].msg.Data[3] = DLen;
            
            memcpy(send_quque[pri].Node[send_quque[pri].write].msg.Data +4, pdata, DLen);
        }
        else
        {
            send_quque[pri].Node[send_quque[pri].write].msg.DLen = DLen + 2+1;    //CANID,长度信息
            send_quque[pri].Node[send_quque[pri].write].msg.Data[0] = CanID >> 8;
            send_quque[pri].Node[send_quque[pri].write].msg.Data[1] = CanID;
            send_quque[pri].Node[send_quque[pri].write].msg.Data[2] = (DLen & 0x0F);
            
            memcpy(send_quque[pri].Node[send_quque[pri].write].msg.Data +3, pdata, DLen);
        }
        

        send_quque[pri].write ++;
        send_quque[pri].len ++;    

        send_quque[pri].write = send_quque[pri].write % MAX_QUQUE_LEN;
        
        ret = 0;
    }while(0);

     
      memset(datatmp, 0x00, sizeof(datatmp));
      for(int j = 0; j < 10 && j < DLen; j++)
      {
          sprintf((char *)(&(datatmp[j*3])), "%02x ", pdata[j]);
      }
      
//      Log(LOG_DEBUG, "Send Data len: %d, Data: \r\n%s\n", DLen, datatmp);
      
    return ret;
}


u32 SendData(const u8 SN, const EN_APPS Sender, const EN_APPS Recver, const u16 DLen, const u8 * pdata)
{
    u32 ret = 1;

    u8 pri = 0;        //默认优先级为0

    
    
    do{

        if( NULL == pdata )
        {
/*            Log(LOG_DEBUG, "Param Faild.\n");*/
            ret = 1;
            break;
        }
    
        if(DLen > MAX_S_MSG_DATA)
        {
/*            Log(LOG_DEBUG, "send Buffer too small, DLen: %d, Buff len: %d.\n", DLen, MAX_MSG_DATA);*/
            ret = 1;
            break;
        }
    
        if(send_quque[pri].len >= MAX_QUQUE_LEN)
        {
/*            Log(LOG_DEBUG, "send quque too small.\n");*/
            ret = 2;
            break;
        }

        //填充待发送数据
        send_quque[pri].Node[send_quque[pri].write].ENState = EN_TSTA_IDEL;        //数据就绪状态
        send_quque[pri].Node[send_quque[pri].write].msg.DLen = DLen;
        send_quque[pri].Node[send_quque[pri].write].msg.SN = SN;
        send_quque[pri].Node[send_quque[pri].write].msg.Sender = Sender;
        send_quque[pri].Node[send_quque[pri].write].msg.Recver = Recver;
        memcpy(send_quque[pri].Node[send_quque[pri].write].msg.Data, pdata, DLen);

        send_quque[pri].write ++;
        send_quque[pri].len ++;    

        send_quque[pri].write = send_quque[pri].write % MAX_QUQUE_LEN;
        
        ret = 0;
    }while(0);

     
      memset(datatmp, 0x00, sizeof(datatmp));
      for(int j = 0; j < 10 && j < DLen; j++)
      {
          sprintf((char *)(&(datatmp[j*3])), "%02x ", pdata[j]);
      }
      
      Log(LOG_DEBUG, "Send Data len: %d, Data: \r\n%s\n", DLen, datatmp);
      
    return ret;
}

//0-数据读取成功，1-当前队列无数据，2-参数异常，
u8 ReadData(const EN_APPS Recver, EN_APPS *pSender,u32 *plen, u8 *pdata)
{
    u32 ret = 0;
    u8 index = 0;

    do{
        if(Recver < EN_MCU_APP1 || Recver > (EN_MCU_APP1 + (RECV_NUM * 2))|| NULL == plen || NULL == pdata || NULL == pSender)
        {
/*            Log(LOG_DEBUG, "Param Faild.\n");*/
            ret = 2;
            break;
        }

        index = (Recver - EN_MCU_APP1) / 2;
        
        if( recv_quque[index].len <= 0)
        {

            //当前队列无数据
            
            ret = 1;
            break;
        }

        if(recv_quque[index].Node[recv_quque[index].read].DLen > *plen)
        {
/*            Log(LOG_DEBUG, "Recv Buff too small, DLen: %d, Bufflen: %d.\n",recv_quque[index].Node[recv_quque[index].read].DLen, *plen );*/
            ret = 3;
            break;
        }

        *pSender = (EN_APPS)recv_quque[index].Node[recv_quque[index].read].Sender;
        memcpy(pdata, recv_quque[index].Node[recv_quque[index].read].Data, recv_quque[index].Node[recv_quque[index].read].DLen);
        *plen = recv_quque[index].Node[recv_quque[index].read].DLen;

        recv_quque[index].len --;
        recv_quque[index].read ++;
        recv_quque[index].read = recv_quque[index].read % MAX_QUQUE_LEN;
                
//                Log(LOG_DEBUG, "Recv Data len: %d", *plen);
        
    }while(0);

        
    return ret;
}
/*********************************************************************************/


static u8 TempBuff1[MAX_SEND_DATA];
static u8 TempBuff2[MAX_SEND_DATA];

u32 TransUartSend(const ST_S_MSG_NODE *pNode )
{
    u32 ret =0;
    u32 index = 0;
    u32 i = 0;
    u32 len = 0;

    memset(TempBuff1, 0x00, sizeof(TempBuff1));
    memset(TempBuff2, 0x00, sizeof(TempBuff2));

    TempBuff1[index++] = FRAME_TYPE_SEND;            //报文类型-发送
    TempBuff1[index++] = pNode->TSN;    
    TempBuff1[index++] = pNode->msg.Sender;    
    TempBuff1[index++] = pNode->msg.Recver;        
    TempBuff1[index++] = pNode->msg.DLen / 0x100;
    TempBuff1[index++] = pNode->msg.DLen ;

    memcpy(TempBuff1 +index, pNode->msg.Data, pNode->msg.DLen);
    index += pNode->msg.DLen;

    //填充CS
    while(i < index)
    {
        TempBuff1[index] +=TempBuff1[i];
        i++;
    }

    len = sizeof(TempBuff2);
    escape_encode(TempBuff1, index, TempBuff2, &len);

    Uart_Send(TempBuff2, len);
    
    return ret;
}

u32 TransUartSendAck(const u8 SN)
{
    u32 ret =0;
    u32 index = 0;
    u8  AckBuffer[100] = {0};

    u32 len = 0;

    memset(TempBuff1, 0x00, sizeof(TempBuff1));
    memset(AckBuffer, 0x00, sizeof(AckBuffer));

    TempBuff1[index++] = FRAME_TYPE_ACK;            //报文类型-发送
    TempBuff1[index++] = SN;    
    
    len = sizeof(AckBuffer);
    escape_encode(TempBuff1, index, AckBuffer, &len);

    Uart_Send(AckBuffer, len);
    
    return ret;
}

//买个周期只能发送一条报文
u32 AutoSend()
{
    u32 ret = 0;
    u32 i = 0, j =0, index =0;

    u8 flag =0;


    
    for(i = 0; i< SEND_PRI_NUM; i++)
    {
        if(send_quque[i].len > 0)
        {
            for(j = 0; j<send_quque[i].len; j++ )
            {
                index = j + send_quque[i].read;
                index = index % MAX_QUQUE_LEN;
                
                if(EN_TSTA_IDEL == send_quque[i].Node[index].ENState)
                {
                    //在此处发送消息
                    TSN ++;
                    send_quque[i].Node[index].TSN = TSN;
                    send_quque[i].Node[index].WTime = 0;
                    send_quque[i].Node[index].ENState = EN_TSTA_S1;

                    TransUartSend(&(send_quque[i].Node[index]));
//                    Log(LOG_DEBUG, "send SN:%d, send times:%d\n", send_quque[i].Node[index].TSN, send_quque[i].Node[index].ENState - 2);
                    flag = 1;
                    break;
                }
                else if((EN_TSTA_S1 <= send_quque[i].Node[index].ENState) && (EN_TSTA_S3 >= send_quque[i].Node[index].ENState))
                {
                    //在此处进行重发。不可在此处进行时间累加处理。
                    if(send_quque[i].Node[index].WTime >= TIMEOUT_SEND)
                    {
                        send_quque[i].Node[index].ENState += 1;
                        send_quque[i].Node[index].WTime = 0;        //重新记录超时时间
                        
                        if(EN_TSTA_S3 >= send_quque[i].Node[index].ENState)
                        {
                            TransUartSend(&(send_quque[i].Node[index]));
                            Log(LOG_DEBUG, "Uart Retry send SN:%d, send times:%d\n", send_quque[i].Node[index].TSN, send_quque[i].Node[index].ENState - 2);
                            flag = 1;
                            break;
                        }
                    }
                }

            }
        }
        if(1 == flag)
        {
            break;
        }
    }

    return ret;
}


u32 UpdateTime()
{
    u32 ret = 0;
    u32 i = 0, j =0, index =0;
    
    for(i = 0; i< SEND_PRI_NUM; i++)
    {
        if(send_quque[i].len > 0)
        {
            for(j = 0; j<send_quque[i].len; j++ )
            {
                index = j + send_quque[i].read;
                index = index % MAX_QUQUE_LEN;
                
                if(EN_TSTA_S1 <= send_quque[i].Node[index].ENState && send_quque[i].Node[index].ENState <= EN_TSTA_S3 )
                {
                    send_quque[i].Node[index].WTime ++;                
                }

            }
        }
        
    }

    return ret;

}


typedef struct
{
    u8 data[1024 * 10];			//串口数据缓冲区10K足够，已去掉无效数据，例如0x00
    u32 read;
    u32 write;
    u32 len;
}ST_POP;

static ST_POP RecvQueBuff;

static u32 RecvQuePush(const u8 * pdata, u32 len)
{
    u32 ret = 0;
    u32 i = 0;

    if((len + RecvQueBuff.len) >= sizeof(RecvQueBuff.data))
    {
        return 2;        //buff too small
    }

    for(i = 0; i< len; i++)
    {
        RecvQueBuff.len ++;
        RecvQueBuff.data[RecvQueBuff.write] = pdata[i];    
        RecvQueBuff.write = (RecvQueBuff.write + 1) % sizeof( RecvQueBuff.data);
    }
    

    return ret;
}

//实现读取单帧数据，如果数据不够一帧，则不出队列
static u32 RecvQuePop_SF(u8 *pdata, u32 * plen)
{
    u32 ret = 2;
    u32 flag = 0;
    u32 index = 0,i = 0;
    static char FindHeadFlag = 0;

    do{

        if(NULL == pdata || NULL == plen || *plen <= 5)
        {
/*            Log(LOG_DEBUG, "Param faild.\n");*/
            ret = 1;
            break;
        }
    
        //找到报文头
        while(RecvQueBuff.len > 0)
        {
            if(HEAD_H == RecvQueBuff.data[RecvQueBuff.read] )
            {
                flag = 1;
                FindHeadFlag = 1;
                break;
            }
            RecvQueBuff.len --;
            RecvQueBuff.read ++;
            RecvQueBuff.read = RecvQueBuff.read % sizeof(RecvQueBuff.data);
            
        }

        if(0 == FindHeadFlag && 1 != flag )
        {
            break;
        }
            
        //尝试找到报文尾，不出队列
        index = RecvQueBuff.read;
        i = RecvQueBuff.len;
        while(i > 0)
        {
            if(TAIL_L == RecvQueBuff.data[index] )
            {
                flag = 2;
                FindHeadFlag = 0;
                break;
            }
            i --;
            index ++;
            index = index % sizeof(RecvQueBuff.data);
        }
        
        if(2 != flag)
        {
            break;
        }
        
        if((index - RecvQueBuff.read) > *plen && (RecvQueBuff.read + (RecvQueBuff.len - i)) > sizeof(RecvQueBuff.data) && 
                (index + sizeof(RecvQueBuff.data) - RecvQueBuff.read) > *plen)
        {
/*            Log(LOG_DEBUG, "RecvBuff too small.\n");*/
            ret = 3;
            break;
        }

        //找到了报文尾，出队列
        index = 0;
        while(RecvQueBuff.len > 0)
        {
            pdata[index] = RecvQueBuff.data[RecvQueBuff.read];
            RecvQueBuff.len --;
            RecvQueBuff.read ++;
            RecvQueBuff.read = RecvQueBuff.read % sizeof(RecvQueBuff.data);
            
            if(TAIL_L == pdata[index++] )
            {
                ret = 0;
                break;
            }
            
        }
        
        *plen = index;
        
    }while(0);

    return ret;
}


u32 FrameAnalysis(const u8 *pdata, const u32 datalen)
{
    u32 ret = 0;
    u32 index = 0,i = 0,j = 0;
    u8 SN = 0,Recver = 0, Sender = 0;
    u16 len = 0;
    u8 flag = 0;
    
    SN = pdata[1];
    Sender = pdata[2];
        
    if(FRAME_TYPE_ACK == pdata[0])
    {
        //在此处更新发送队列，报文发送成功。
        //遍历发送队列，比较SN，如果SN一致则认为发送成功
        for(i = 0; (0 == flag) && (i < SEND_PRI_NUM); i++ )
        {
            for(j = 0; j < send_quque[i].len; j++)
            {
                index = send_quque[i].read + j;
                index %= MAX_QUQUE_LEN;

                //正在接受消息状态，且SN相同
                if( send_quque[i].Node[index].ENState >= EN_TSTA_S1 &&
                    send_quque[i].Node[index].ENState <= EN_TSTA_S3 &&
                    SN == send_quque[i].Node[index].TSN )
                {
                    send_quque[i].Node[index].ENState = EN_TSTA_SUCC;
                    /*send_quque[i].len--;*/
                    flag = 1;
                    Log(LOG_DEBUG,"ACK SN:%d\n", SN);
                    break;
                }
            }
        }    
    }
    else
    {
        //在此处发送ACK信息，
        TransUartSendAck(SN);

        //在此处将信息分别存储在各个接收队列中
        Recver = pdata[3];
        len = pdata[4] * 0x100 +pdata[5];

        if(6+len > datalen)
        {
            //TODO:异常处理
        }

        switch(Recver)
        {
            case EN_MCU_APP1:
            case EN_MCU_APP2:
            case EN_MCU_APP3:
            case EN_MCU_APP4:
//            case EN_MCU_APP5:     接收该APP的报文会导致MCU Reset，受限于 recv_quque大小
                //复用Recver
                Recver = (Recver - EN_MCU_APP1) / 2;

                if(    recv_quque[Recver].len > MAX_QUQUE_LEN)
                {
/*                    Log(LOG_DEBUG, "recv_quque data full, recver: %d\n", Recver);*/
                    break;
                }

                
                recv_quque[Recver].Node[recv_quque[Recver].write].DLen = len;
                recv_quque[Recver].Node[recv_quque[Recver].write].Sender = Sender;
                memcpy(recv_quque[Recver].Node[recv_quque[Recver].write].Data, &pdata[6], len);

                recv_quque[Recver].write ++;
                recv_quque[Recver].write = recv_quque[Recver].write % MAX_QUQUE_LEN;
                
                recv_quque[Recver].len++;
                break;

            default:
/*                Log(LOG_DEBUG, "Recver faild, recver: %d.\n", Recver);*/
                break;
        }
    }

    return ret;
}


u32 AutoRecv()
{
    u32 ret = 0;
    u32 len1 = 0, len2 = 0;

    memset(TempBuff1, 0x00, sizeof(TempBuff1));
    memset(TempBuff2, 0x00, sizeof(TempBuff2));

    len1 = sizeof(TempBuff1);

    //数据接收
    if(0 == Uart_Recv(TempBuff1, &len1))
    {
        len2 = sizeof(TempBuff2);
        pre_decode(TempBuff1, len1, TempBuff2, &len2);

        //Temp2数据缓存入队列
        RecvQuePush(TempBuff2, len2);

        //下次循环使用
        len1 = sizeof(TempBuff1);
    }
    else
    {
        ret = 1;
        return ret;
    }

    //报文处理
    memset(TempBuff1, 0x00, sizeof(TempBuff1));
    len1 = sizeof(TempBuff1);
    while( 0 == RecvQuePop_SF(TempBuff1, &len1))
    {
        if(len1 > 1024)
        {
            len2 = 10;
        }
        len2 = sizeof(TempBuff2);
        escape_decode(TempBuff1, len1, TempBuff2, &len2);

        //在此处进行报文解析，并应答ACK
        FrameAnalysis(TempBuff2, len2);

        //下次循环使用
        len1 = sizeof(TempBuff1);
    }
    
    return ret;
}


//自动整理发送队列
u32 AutoDefragSendQue()
{
    int index = 0, i = 0, j = 0, k = 0,k_index = 0;

    u8 sque_info[SEND_PRI_NUM] = {0};
    u32 Canid = 0x00;
    
    /* 第一次遍历，将发送成功、发送失败信息通知给Sender，
    并记录每个队列可以整理的数据情况 */
    for(i = 0; i < SEND_PRI_NUM; i++)
    {
        for(j = 0; j< send_quque[i].len; j++)
        {
            index = send_quque[i].read +j;
            index %= MAX_QUQUE_LEN;

            Canid = (send_quque[i].Node[index].msg.Data[0] <<8) +send_quque[i].Node[index].msg.Data[1];

            if(EN_TSTA_SUCC == send_quque[i].Node[index].ENState)
            {
                //TODO：发送成功-通知发送者

                //TODO:在此处判断该报文的发送者是不是MCUAPP1，
                if(EN_MCU_APP1 == send_quque[i].Node[index].msg.Sender)
                {
                    if(send_quque[i].Node[index].msg.Data[2] >= 0x10)        //多帧
                    {
                        UDS_TxConfirmation(Canid,send_quque[i].Node[index].msg.Data[4], send_quque[i].Node[index].msg.Data[5],Tx_SUCCESS );
                    }
                    else
                    {
                        UDS_TxConfirmation(Canid,send_quque[i].Node[index].msg.Data[3], send_quque[i].Node[index].msg.Data[4],Tx_SUCCESS );
                    }
                }
                
                sque_info[i] += 1;
            }

            if(EN_TSTA_FAIL == send_quque[i].Node[index].ENState)
            {
                if(EN_MCU_APP1 == send_quque[i].Node[index].msg.Sender)
                {
                    if(send_quque[i].Node[index].msg.Data[2] >= 0x10)        //多帧
                    {
                        UDS_TxConfirmation(Canid,send_quque[i].Node[index].msg.Data[4], send_quque[i].Node[index].msg.Data[5],Tx_FAILURE );
                    }
                    else
                    {
                        UDS_TxConfirmation(Canid,send_quque[i].Node[index].msg.Data[3], send_quque[i].Node[index].msg.Data[4],Tx_FAILURE );
                    }
                }
                
                //TODO:发送失败-通知发送者
                sque_info[i] += 1;
/*                Log(LOG_DEBUG, "Send Data Faild.\n");*/
            }
            
        }
    }
    
    /* 第二次遍历，出队列，整理数据 */
    for(i = 0; i < SEND_PRI_NUM; i++)
    {
        //当前队列无需整理
        if( 0 == send_quque[i].len  || 0 == sque_info[i])
        {
            continue;
        }
        
        for(j = send_quque[i].len - 1; j >= 0 ; j--)
        {
            index = send_quque[i].read + j;
            index %= MAX_QUQUE_LEN;

            if(EN_TSTA_SUCC == send_quque[i].Node[index].ENState ||
               EN_TSTA_FAIL == send_quque[i].Node[index].ENState )
               {
                   //需要处理
                   for( k = j; k >= 0; k--)
                   {
                       k_index = send_quque[i].read + k;
                    k_index %= MAX_QUQUE_LEN;
                    
                       if(EN_TSTA_IDEL <= send_quque[i].Node[k_index].ENState &&
                         send_quque[i].Node[k_index].ENState <= EN_TSTA_S3)
                       {
                        break;                       
                       }
                   }

                   if(k >= 0)
                   {
                       //找到了，可以执行拷贝操作
                       memcpy(&(send_quque[i].Node[index]), &(send_quque[i].Node[k_index]), sizeof(ST_S_MSG_NODE));
                       send_quque[i].Node[k_index].ENState = EN_TSTA_UNKNOW;
                   }
                   else
                   {
                       //未找到，说明该队列后续已空，直接出队列
                       send_quque[i].len -= (j+1);
                       send_quque[i].read += (j+1);
                       send_quque[i].read %= MAX_QUQUE_LEN;

#if 1    //错误检测
                    if(j +1 != sque_info[i])
                    {
                        //发现异常
/*                        Log(LOG_DEBUG, "\n");    */
                    }
#endif                        
                       break;
                   }
               }
        
        }
    }
    
    return 0;
}

u32 TransInit()
{
    u32 ret = 0;
    

    memset(send_quque, 0x00, sizeof(send_quque));
    memset(recv_quque, 0x00, sizeof(recv_quque));

    TSN = 0;

    memset(TempBuff1, 0x00, sizeof(TempBuff1));
    memset(TempBuff2, 0x00, sizeof(TempBuff2));
    
    memset(&RecvQueBuff, 0x00, sizeof(RecvQueBuff));
    
    

    return ret;
}


//将MCU接收的数据分发给MCU上的各个应用
u32 msgq_dist()
{
    u32 ret = 0;
        u32 CanId = 0, len = 0, index = 0;

    MSG_Queue recv_buff;

    long type;


    for(type = EN_MCU_APP1; type < EN_MCU_APP1 + (RECV_NUM * 2); type += 2)
    {
        recv_buff.mdata.mlen = sizeof(recv_buff.mdata.mbuff);
        
        while(0 == ReadData((EN_APPS)type,(EN_APPS *)(&(recv_buff.mdata.apptype)), &(recv_buff.mdata.mlen), recv_buff.mdata.mbuff ))
        {
            switch(type)
            {
                //接收到数据.
                case EN_MCU_APP1:
					//收到诊断指令。 
					CanId = recv_buff.mdata.mbuff[index++] * 0x100;    //ECU Diag ID
					CanId += recv_buff.mdata.mbuff[index++];

					len = recv_buff.mdata.mbuff[index++];
					if(len >= 0x10)
					{
					      len -= 0x10;
					      len *= 0x100;
					      len += recv_buff.mdata.mbuff[index++];
					}

					Log(LOG_INFO, "EN_MCU_APP1 Recv data,SID: 0x%02x.\n", recv_buff.mdata.mbuff[index]);
					//TODO:可选，在此处检验长度信息是否匹配。

                    if(DIAGCANRXID == CanId)
					{
					      //只诊断TBOX自身
					      UDS_RxIndication(recv_buff.mdata.mbuff + index, recv_buff.mdata.mlen-index,DIAG_PHY , 2,recv_buff.mdata.apptype);
					      
					}
					else if(DIAGFUNCRXID == CanId)
					{
					      //诊断所有ECU
					      UDS_RxIndication(recv_buff.mdata.mbuff + index, recv_buff.mdata.mlen-index,DIAG_FUNC , 2,recv_buff.mdata.apptype);

					}
                            
					else
					{
					      //该报文不予处理
					}
                    break;
                case EN_MCU_APP2:
                    if ((CALLBACK)APPCALLBACK2 != (CALLBACK)NULL)
                    {
                        ((CALLBACK)APPCALLBACK2)(&recv_buff);
                    }
                    else
                    {
                        ;
                    }
                    break;
                case EN_MCU_APP3:
					UDS_SaveMpuVersion(recv_buff.mdata.mbuff, recv_buff.mdata.mlen);                   
                    break;
                case EN_MCU_APP4:
                    if ((CALLBACK)APPCALLBACK4 != (CALLBACK)NULL)
                    {
                        ((CALLBACK)APPCALLBACK4)(&recv_buff);
                    }
                    else
                    {
                        ;
                    }
                    break;
                case EN_MCU_APP5:
                    if ((CALLBACK)APPCALLBACK5 != (CALLBACK)NULL)
                    {
                        ((CALLBACK)APPCALLBACK5)(&recv_buff);
                    }
                    else
                    {
                        ;
                    }
                    break;
                default:
                    Log(LOG_DEBUG, "Recver faild, recver: %d.\n", type);
                    break;
            }
        }

    }

    return ret;
}


u32 TransMain()
{
    u32 ret = 0;

    //在此处接收数据
    ret = AutoRecv();
    
    //在此处处理所有的待发送数据
    AutoSend();

    //在此处更新超时时间。
    UpdateTime();

    //在此处整理发送队列
    AutoDefragSendQue();
    
    return ret;
}





