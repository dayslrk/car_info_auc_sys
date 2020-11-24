#ifndef _TIMER_H_
#define _TIMER_H_

#include "BaseType.h"

#define TIMER_DEVIATION         (5U)

#pragma pack(push)
#pragma pack(1)

struct Timer
{
    u32 TimerCounter;
    bool Valid;
    void (*Func) (void);
};
typedef struct Timer DiagTimer_Type, *pDiagTimer_Type;

#pragma pack(pop)


void UDS_TimerRun(pDiagTimer_Type Timer, u32 TimeCount, void (*Func) (void));
void UDS_TimerStop(pDiagTimer_Type Timer);
bool UDS_CheckTimeout(pDiagTimer_Type Timer);
void UDS_WaitTimeout(void);

#endif
