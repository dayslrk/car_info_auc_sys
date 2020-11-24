#include "BaseType.h"
#include "Timer.h"
#include "Timer_Driver.h"

extern DiagTimer_Type SessionTimer;
extern DiagTimer_Type UnlockFailedTimer;
extern DiagTimer_Type ResponsePendingTimer;
extern DiagTimer_Type ResponseTimeoutTimer;
extern DiagTimer_Type TxMsgTimer;


/*******************************************************************************
* 函数名称: UDS_TimerRun
* 功能描述: 启动一个定时器，设置定时器参数
* 输入参数: Timer：需要启动的定时器 TimeCount：定时时间
* 输出参数: 无
* 返回参数: 无
* 备    注：
*******************************************************************************/
void UDS_TimerRun(pDiagTimer_Type Timer, u32 TimeCount, void (*Func) (void))
{
    if (NULL != Timer && NULL != Func)
    {
        u32 SystemTickCount = TIMx_GetCounter();
        Timer->TimerCounter = (TimeCount + SystemTickCount) & 0xFFFFFFFF;
        Timer->Valid = TRUE;
        Timer->Func = Func;
    }
    else
    {
        ;/*指针为空*/
    }
}


/*******************************************************************************
* 函数名称: UDS_TimerStop
* 功能描述: 停止一个定时器
* 输入参数: Timer：需要停止的定时器
* 输出参数: 无
* 返回参数: 无
* 备    注：
*******************************************************************************/
void UDS_TimerStop(pDiagTimer_Type Timer)
{
    if (NULL != Timer)
    {
        Timer->Valid = FALSE;
    }
    else
    {
        ;/*指针为空*/
    }
}


/*******************************************************************************
* 函数名称: UDS_CheckTimeout
* 功能描述: 检测定时器是否已经超时
* 输入参数: Timer：需要检测的定时器
* 输出参数: 无
* 返回参数: TRUE：超时 FALSE：未超时
* 备    注：
*******************************************************************************/
bool UDS_CheckTimeout(pDiagTimer_Type Timer)
{
    if (NULL != Timer)
    {
        u32 SystemTickCount = TIMx_GetCounter();

        if (TRUE == Timer->Valid)
        {
            if (0 == Timer->TimerCounter)
            {
                Timer->Valid = FALSE;
                return TRUE;
            }
            else if ((SystemTickCount - Timer->TimerCounter) < TIMER_DEVIATION || (Timer->TimerCounter - SystemTickCount) < TIMER_DEVIATION)
            {
                Timer->TimerCounter = 0;
                Timer->Valid = FALSE;
                return TRUE;
            }
            else
            {
                return FALSE;
            }
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
        return FALSE;/*指针为空*/
    }
}


/*******************************************************************************
* 函数名称: UDS_WaitTimeout
* 功能描述: 循环检测定时器是否超时
* 输入参数: Timer：
* 输出参数: 无
* 返回参数: 无
* 备    注：
*******************************************************************************/
void UDS_WaitTimeout(void)

{
    if (UDS_CheckTimeout(&SessionTimer))
    {
        UDS_TimerStop(&SessionTimer);
        if (NULL != SessionTimer.Func)
        {
            SessionTimer.Func();
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
    
    if (UDS_CheckTimeout(&UnlockFailedTimer))
    {
        UDS_TimerStop(&UnlockFailedTimer);
        if (NULL != UnlockFailedTimer.Func)
        {
            UnlockFailedTimer.Func();
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

    if (UDS_CheckTimeout(&ResponsePendingTimer))
    {
        UDS_TimerStop(&ResponsePendingTimer);
        if (NULL != ResponsePendingTimer.Func)
        {
            ResponsePendingTimer.Func();
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

    if (UDS_CheckTimeout(&ResponseTimeoutTimer))
    {
        UDS_TimerStop(&ResponseTimeoutTimer);
        if (NULL != ResponseTimeoutTimer.Func)
        {
            ResponseTimeoutTimer.Func();
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
    
    if (UDS_CheckTimeout(&TxMsgTimer))
    {
        UDS_TimerStop(&TxMsgTimer);
        if (NULL != TxMsgTimer.Func)
        {
            TxMsgTimer.Func();
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
