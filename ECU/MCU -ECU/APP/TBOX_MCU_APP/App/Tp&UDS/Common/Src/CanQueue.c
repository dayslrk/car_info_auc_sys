#include "CanQueue.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "delay.h"


CAN_Queue_Type CAN_Queue_Rec;
UDS_Queue_Type UDS_Queue_Rec;


/*******************************************************************************
* 函数名称: CAN_Queue_Init
* 功能描述: CAN Buffer队列的初始化函数
* 输入参数: 无 
* 输出参数: 无
* 返回参数: 无
* 备    注：
*******************************************************************************/
void CAN_Queue_Init(void)
{
    u16 i = 0;
    u16 j = 0;

    CAN_Queue_Rec.front = 0;
    CAN_Queue_Rec.rear = 0;

    for (i = 0; i < CAN_QUEUE_BUFFER_MAXSIZE; ++i)
    {
        CAN_Queue_Rec.Buffer[i].ID = 0;
        CAN_Queue_Rec.Buffer[i].Len = 0;
        for (j = 0 ; j < 8; ++j)
        {
            CAN_Queue_Rec.Buffer[i].Data[j] = 0;
        }
    }
}


/*******************************************************************************
* 函数名称: CAN_Queue_Push
* 功能描述: CAN Buffer的压入操作
* 输入参数: Data：需要压入数据的指针
* 输出参数: 无
* 返回参数: 0（E_OK）：压入数据成功                 1（E_BUFFER_FULL）：压人数据失败，队列已满
* 备    注：
*******************************************************************************/
u8 CAN_Queue_Push(const CanRxMsg *Data, u8 Channel)
{
    u16 i = 0;
    u8 Retval = 0;
    
    /*检查当前队列是否已满，若已满，则放弃继续压入数据*/
    if ((CAN_Queue_Rec.rear + 1) % CAN_QUEUE_BUFFER_MAXSIZE == CAN_Queue_Rec.front)
    {
        Retval = E_BUFFER_FULL;
    }
    else if (NULL == Data)
    {
        Retval = E_NULL_PTR;
    }
    else
    {
        for (i = 0; i < Data->DLC; ++i)
        {
            CAN_Queue_Rec.Buffer[CAN_Queue_Rec.rear].Data[i] = Data->Data[i];
        }

        CAN_Queue_Rec.Buffer[CAN_Queue_Rec.rear].ID = Data->StdId;
        CAN_Queue_Rec.Buffer[CAN_Queue_Rec.rear].Len = Data->DLC;
        CAN_Queue_Rec.Buffer[CAN_Queue_Rec.rear].Channel = Channel;

        /*每压入一次数据，队尾向后挪动1*/
        CAN_Queue_Rec.rear = (CAN_Queue_Rec.rear + 1) % CAN_QUEUE_BUFFER_MAXSIZE;

        Retval = E_OK;
    }

    return Retval;
}


/*******************************************************************************
* 函数名称: CAN_Queue_Pop
* 功能描述: CAN Buffer的取出操作
* 输入参数: Data：存储取出数据的指针
* 输出参数: 无
* 返回参数: 0（E_OK）：取出数据成功   2（E_BUFFER_EMPTY）：取出数据失败，队列为空
* 备    注：
*******************************************************************************/
u8 CAN_Queue_Pop(pCAN_Msg_Type Data)
{
    u8 Retval = 0;
    
    /*判断当前队列是否为空，若为空，则放弃取出数据操作*/
    if (CAN_Queue_Rec.front == CAN_Queue_Rec.rear)
    {
        Retval = E_BUFFER_EMPTY;
    }
    else if (NULL == Data)
    {
        Retval = E_NULL_PTR;
    }
    else
    {
        
        *Data = CAN_Queue_Rec.Buffer[CAN_Queue_Rec.front];
        /* 取出一次数据，队头前后挪动1 */
        CAN_Queue_Rec.front = (CAN_Queue_Rec.front + 1) % CAN_QUEUE_BUFFER_MAXSIZE;

        Retval = E_OK;
    }

    return Retval;
}


/*******************************************************************************
* 函数名称: CAN_Queue_GetRecCount
* 功能描述: 获取当前队列已压入数据的数量
* 输入参数: 无 
* 输出参数: 无
* 返回参数: 当前队列已压入数据的数量
* 备    注：
*******************************************************************************/
u16 CAN_Queue_GetRecCount(void)
{
    return (CAN_Queue_Rec.rear >= CAN_Queue_Rec.front)?(CAN_Queue_Rec.rear - CAN_Queue_Rec.front)
        :(CAN_QUEUE_BUFFER_MAXSIZE - (CAN_Queue_Rec.front - CAN_Queue_Rec.rear));
}


/*******************************************************************************
* 函数名称: UDS_Queue_Init
* 功能描述: UDS Buffer队列的初始化函数
* 输入参数: 无 
* 输出参数: 无
* 返回参数: 无
* 备    注：
*******************************************************************************/
void UDS_Queue_Init(void)
{
    u16 i = 0;
    u16 j = 0;

    UDS_Queue_Rec.front = 0;
    UDS_Queue_Rec.rear = 0;

    for (i = 0; i < UDS_QUEUE_BUFFER_MAXSIZE; ++i)
    {
        UDS_Queue_Rec.Buffer[i].SID = 0;
        UDS_Queue_Rec.Buffer[i].Len = 0;
        UDS_Queue_Rec.Buffer[i].FrameType = DIAG_PHY;
        for (j = 0 ; j < MAX_UDS_RCV_Buff_Size; ++j)
        {
            UDS_Queue_Rec.Buffer[i].Data[j] = 0;
        }
    }
}


/*******************************************************************************
* 函数名称: UDS_Queue_Push
* 功能描述: UDS Buffer的压入操作
* 输入参数: Data：需要压入数据的指针
* 输出参数: 无
* 返回参数: 0（E_OK）：压入数据成功                 1（E_BUFFER_FULL）：压人数据失败，队列已满
* 备    注：
*******************************************************************************/
u8 UDS_Queue_Push(const pUDS_Msg_Type Data)
{
    u16 i = 0;
    u8 Retval = 0;

    /*检查当前队列是否已满，若已满，则放弃继续压入数据*/
    if ((UDS_Queue_Rec.rear + 1) % UDS_QUEUE_BUFFER_MAXSIZE == UDS_Queue_Rec.front)
    {
        Retval = E_BUFFER_FULL;
    }
    else if (NULL == Data)
    {
        Retval = E_NULL_PTR;
    }
    else
    {        
        for (i = 0; i < Data->Len; ++i)
        {
            UDS_Queue_Rec.Buffer[UDS_Queue_Rec.rear].Data[i] = Data->Data[i];
        }

        UDS_Queue_Rec.Buffer[UDS_Queue_Rec.rear].SID = Data->SID;
        UDS_Queue_Rec.Buffer[UDS_Queue_Rec.rear].Len = Data->Len;
        UDS_Queue_Rec.Buffer[UDS_Queue_Rec.rear].FrameType = Data->FrameType;
        UDS_Queue_Rec.Buffer[UDS_Queue_Rec.rear].Channel = Data->Channel;
        UDS_Queue_Rec.Buffer[UDS_Queue_Rec.rear].apptype = Data->apptype;

        /*每压入一次数据，队尾向后挪动1*/
        UDS_Queue_Rec.rear = (UDS_Queue_Rec.rear + 1) % UDS_QUEUE_BUFFER_MAXSIZE;

        Retval = E_OK;
    }

    return Retval;
}


/*******************************************************************************
* 函数名称: UDS_Queue_Pop
* 功能描述: UDS Buffer的取出操作
* 输入参数: Data：存储取出数据的指针
* 输出参数: 无
* 返回参数: 0（E_OK）：取出数据成功   2（E_BUFFER_EMPTY）：取出数据失败，队列为空
* 备    注：
*******************************************************************************/
u8 UDS_Queue_Pop(pUDS_Msg_Type Data)
{
    u8 Retval = 0;

    /*判断当前队列是否为空，若为空，则放弃取出数据操作*/
    if (UDS_Queue_Rec.front == UDS_Queue_Rec.rear)
    {
        Retval = E_BUFFER_EMPTY;
    }
    else if (NULL == Data)
    {
        Retval = E_NULL_PTR;
    }
    else
    {    
        *Data = UDS_Queue_Rec.Buffer[UDS_Queue_Rec.front];
        /*取出一次数据，队头前后挪动1*/
        UDS_Queue_Rec.front = (UDS_Queue_Rec.front + 1) % UDS_QUEUE_BUFFER_MAXSIZE;

        Retval = E_OK;
    }

    return Retval;
}


/*******************************************************************************
* 函数名称: UDS_Queue_GetRecCount
* 功能描述: 获取当前队列已压入数据的数量
* 输入参数: 无 
* 输出参数: 无
* 返回参数: 当前队列已压入数据的数量
* 备    注：
*******************************************************************************/
u16 UDS_Queue_GetRecCount(void)
{
    return (UDS_Queue_Rec.rear >= UDS_Queue_Rec.front)?(UDS_Queue_Rec.rear - UDS_Queue_Rec.front)
        :(UDS_QUEUE_BUFFER_MAXSIZE - (UDS_Queue_Rec.front - UDS_Queue_Rec.rear));
}

