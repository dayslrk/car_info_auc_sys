#ifndef MPUPOWERCTRL_H_
#define MPUPOWERCTRL_H_

#include "BaseType.h"
#include "comm_if.h"
#include "CAN.h"

#define RECEIVE                     (0U)
#define SEND                        (1U)

#define TASKBASE_TIME               (10U)

//#define MPUPOWEROFF_TIME            (60000U)
#define MPUPOWEROFF_TIME            (15000U)

#define MPUPOWEROFF_COUNT           (MPUPOWEROFF_TIME / TASKBASE_TIME)
#define MPUPOWERRESUME_TIME         (1000U)
#define MPUPOWERRESUME_COUNT        (MPUPOWERRESUME_TIME / TASKBASE_TIME)
#define MPUPOWEROFFSENDWAIT_TIME    (30000U)
//#define MPUPOWEROFFSENDWAIT_TIME    (120000U)

#define MPUPOWEROFFSENDWAIT_COUNT   (MPUPOWEROFFSENDWAIT_TIME / TASKBASE_TIME)
#define LOWVOLCHECKCOUNT            (10U)
#define POWERMSGLEN                 (4U)
#define MPUFLASHFAILEDRETRYCOUNT    (3U)

typedef enum
{
    KL15VOL = 0,            //KL15状态，MCU --> MPU Manage
    SHUTDOWN,               //关机信号，MCU --> MPU Manage
    SELFFLASHTIME,          //自升级时间，MPU FotaSlave --> MCU          
    SELFFLASHFINISHED,      //自升级完成，MPU FotaSlave --> MCU
    NODEFLASHING,           //开始升级， TBOX MPU FotaMaster --> MCU
    NODEFLASHFINISHED,      //升级完成，TBOX MPU FotaMaster --> MCU
    EXTENDIFRESET,          //扩展板复位, MPU --> MCU
    PWRSIGNALMAX,
}PwrSignalType_EU;

typedef enum
{
    UNKNOW,
    NORMAL,                 //普通状态
    SELF_FLASHING,          //ECU自升级状态
    SELF_FLASHFAILED,       //ECU自升级失败状态
//    NODE_FLASHING,
    SENDPOWEROFF_WAIT,
    POWEROFF_WAIT,
    POWEROFF,
}PwrManageStep_EU;

typedef struct
{
    bool              Type;
    PwrSignalType_EU  SignalType;
    u8                DataLen;
}PwrSignalConf_ST;

#pragma pack(push)
#pragma pack(1)
typedef struct
{
    u8 SignalType;
    u8 DataLen;
    u8 Data[2];
}PwrSignal_ST;
#pragma pack(pop)


void MpuPower_Init(void);
void KL15_VolCheck(void);
void SelfFlashProcess(void);
void MpuPowerOffProcess(void);
void SendPowerOff(void);
void BroadCastFlashSig(void);
void KL15StatusSend();
void App_RxIndication(u8 Channel, pCAN_Frame_Type CANFrame);
void MpuPowerOff(void);
void MpuPowerOn(void);
void MpuFlashInfor(MSG_Queue *buff);

void ExtendIFPowerOn(void);

void ExtendIFPowerOff(void);

#endif

