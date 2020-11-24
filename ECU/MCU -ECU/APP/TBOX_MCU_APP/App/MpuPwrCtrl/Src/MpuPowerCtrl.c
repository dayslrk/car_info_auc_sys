#include "MpuPowerCtrl.h"
#include "Trans.h"
#include "Timer_Driver.h"
#include "EepManage_public.h"


PwrSignalConf_ST PwrSignalCofTable[PWRSIGNALMAX] = 
{
    {SEND   , KL15VOL           , 0x01},
    {SEND   , SHUTDOWN          , 0x00},
    {RECEIVE, SELFFLASHTIME     , 0x02},
    {RECEIVE, SELFFLASHFINISHED , 0x00},
    {RECEIVE, NODEFLASHING      , 0x00},
    {RECEIVE, NODEFLASHFINISHED , 0x00},
    {RECEIVE, EXTENDIFRESET,    0x02},          
};
bool NodeFlashing = FALSE;
bool SelfFlashing = FALSE;
static u16 SelfFlashWaitTime = 0;/*自身Mpu需要的升级时间*/
static u16 SelfFlashWaitCount = 0;/*自身升级已花费时间*/
static u16 PowerResumeWaitCount = 0;/*断电3秒,当前等待时间*/
static u8  FlashFailedCount = 0;/*Mpu刷新失败次数*/
static u16 PowerOffWaitCount = 0;/*KL15下电，无消息1分钟后断电*/
static u16 PowerOffSendWaitCount = 0;/*KL15下电2min后发送关机信号*/
static PwrManageStep_EU State = UNKNOW;

static bool MpuResetKeyPush = FALSE;            /* 复位按钮--用于升级重启 */        
static bool MpuOnoffKeyPush = FALSE;            /* 开机键按钮--用于开机过程 */        

static bool MpuPowerFlag = FALSE;                    /* 是否需要按开机键, 默认不需要，防止每次MCU复位都重启MPU*/

const u32 McuResetPowerStaThreshold = 500;       /* unit:1ms  MCU复位后500ms内如果检测到KL15为0则直接切断 
                                                    MPU mini与MPU Pro电源
                                                    注意，该值必须大于200 */

static u32 ExtendIFResetTime = 0;                                                    

/*
    MPU的复位按钮为低电平有效，按键时间必须在30us以上，因此可以在每次执行KL15_VolCheck时设置MPU复位按钮，需要复位时清0。即可保证复位效果

    各IO口说明：
        B0        MPU-mini板电源控制                            输出            高电平供电
        B1        ACC 信号采集                                输入
        B3        MPU-mini ON/OFF控制                        输出            低电平开/关机
        B4        MPU-mini Reset控制                        输出            低电平复位
        C5        扩展接口/MPU-Pro 电源控制                        输出            高电平供电


    复位XXX时间内，如果kl15为低电平，直接进入断电流程

*/

void MpuPower_Init(void)
{
    u8 DataBuff[4] = {0};
    
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);    //开启GPIO时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);    //开启GPIO时钟
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_3 | GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_High_Speed;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_High_Speed;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_High_Speed;
    GPIO_Init(GPIOC, &GPIO_InitStructure);


    GPIO_SetBits(GPIOB, GPIO_Pin_0);
    GPIO_SetBits(GPIOB, GPIO_Pin_3);
    GPIO_SetBits(GPIOB, GPIO_Pin_4);
    GPIO_SetBits(GPIOC, GPIO_Pin_5);

	//其余ECU无需记录NodeFlash状态
    NodeFlashing = FALSE;
    State = NORMAL;
	
#if 1
    Eeprom_Read(EEPROM_SELFFLASHING, DataBuff , 4);
    SelfFlashing = DataBuff[0];
    SelfFlashWaitCount = DataBuff[1];
    SelfFlashWaitTime = (DataBuff[2] << 8 | DataBuff[3]) * 1000;
    
#else    
    SelfFlashing = FALSE;
    SelfFlashWaitTime = 0;
	PowerOffWaitCount = 0;
#endif

    SelfFlashWaitCount = 0;
    PowerResumeWaitCount = 0;
    FlashFailedCount = 0;

}

void KL15_VolCheck(void)
{ 
 
#if 0  	//仅TBOX发送NodeFlash信号
    BroadCastFlashSig();
#endif 	

    KL15StatusSend();
    
    if (MpuResetKeyPush)    /* reset按钮辅助Mpu重启 */
    {
        GPIO_SetBits(GPIOB, GPIO_Pin_4);        /* Reset按键10ms足以 */
        MpuResetKeyPush = FALSE;    
    }

    if(MpuOnoffKeyPush)
    {
        if (PowerResumeWaitCount == MPUPOWERRESUME_COUNT)
        {
            GPIO_SetBits(GPIOB, GPIO_Pin_3);
            MpuOnoffKeyPush = FALSE;
            PowerResumeWaitCount = 0;
        }
        else
        {
            PowerResumeWaitCount++;
        }
    }

    
    if (SelfFlashing)/*自升级*/
    {
        SelfFlashProcess();
    }
    else if (NodeFlashing || (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1)) ) /* 其他节点升级 或者ACC上电中 */
    {
        if (POWEROFF_WAIT == State)/*当MPU自己执行关机后，先断电10ms，再上电使其重启*/
        {
            MpuPowerOff();
        }
        else
        {
            MpuPowerOn();                       //启动MPU
            ExtendIFPowerOn();                  //启动扩展接口供电-大屏、4G模组等
        }
        
        State = NORMAL;
        PowerOffWaitCount = 0;
        PowerOffSendWaitCount = 0;
    }
    else if (!GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1))/*KL15断电*/
    {
        MpuPowerOffProcess();
    }
    else
    {
       ;
    }

    //按照调度时间更新时间标识
    if(ExtendIFResetTime >= 10)
    {
        ExtendIFResetTime -= 10;
    }
    else
    {
        ExtendIFResetTime = 0;
    }
}

void SelfFlashProcess(void)
{
    /*Mpu升级过程失败断电机制*/
    SelfFlashWaitCount = (SELF_FLASHING == State)?(SelfFlashWaitCount + 1):SelfFlashWaitCount;
    if (SelfFlashWaitCount * 10 == SelfFlashWaitTime)/*升级预计时间超时*/
    {
        //模拟Reset按钮
        GPIO_ResetBits(GPIOB, GPIO_Pin_1);
        MpuResetKeyPush = TRUE;

        SelfFlashWaitCount = 0;
        State = SELF_FLASHFAILED;
    }
    else if (SELF_FLASHFAILED == State)/*升级失败，重启MPU*/
    {
        FlashFailedCount++;
        u8 Data[2] = {0};
        Data[0] = SelfFlashing;
        Data[1] = FlashFailedCount;

        Eeprom_Write(EEPROM_SELFFLASHING, Data, 2);      //写入重试次数

        if (FlashFailedCount == MPUFLASHFAILEDRETRYCOUNT)/*达到升级重试次数*/
        {
            SelfFlashing = FALSE;
            Eeprom_Write(EEPROM_SELFFLASHING, &SelfFlashing, 1);
            
            SelfFlashWaitTime = 0;
            SelfFlashWaitCount = 0;
            FlashFailedCount = 0;
            PowerOffWaitCount = 0;
            PowerOffSendWaitCount = 0;
            State = NORMAL;
        }
        else
        {
            State = SELF_FLASHING;
        }
    }
}

void MpuPowerOffProcess(void)
{
    if(McuResetPowerStaThreshold >= TIMx_GetCounter())
    {
        ExtendIFPowerOff();                     //发送关机指令后，立即关闭扩展接口
        MpuPowerOff();

        State = POWEROFF;
    }
    else
    {

        State = (NORMAL == State)?SENDPOWEROFF_WAIT:State;
        
        PowerOffSendWaitCount = (SENDPOWEROFF_WAIT == State)?(PowerOffSendWaitCount + 1):PowerOffSendWaitCount;
        PowerOffWaitCount = (POWEROFF_WAIT == State)?(PowerOffWaitCount + 1):PowerOffWaitCount;
        
        if (PowerOffSendWaitCount == MPUPOWEROFFSENDWAIT_COUNT)/*延时两分钟发送关机信号*/
        {
            State = POWEROFF_WAIT;
            PowerOffSendWaitCount = 0;
            PowerOffWaitCount = 0;

            SendPowerOff();
            ExtendIFPowerOff();                     //发送关机指令后，立即关闭扩展接口
        }
        else if (PowerOffWaitCount == MPUPOWEROFF_COUNT)/*延时一分钟断电*/
        {
            State = POWEROFF;
            MpuPowerOff();
        }
    }
}

void SendPowerOff(void)
{
    u8 Data[POWERMSGLEN] = {0};

    Data[0] = PwrSignalCofTable[SHUTDOWN].SignalType;
    Data[1] = PwrSignalCofTable[SHUTDOWN].DataLen;
    
    SendData(0x00, EN_MCU_APP4, EN_MPU_APP6, POWERMSGLEN, Data);
}

void BroadCastFlashSig(void)
{
    CAN_Frame_Type CANFrame;
    
    CANFrame.ID = 0x306;
    CANFrame.Len = 1;

    if (NodeFlashing)/*有节点正在升级，广播信息*/
    {
        CANFrame.Byte[0] = 1;
    }
    else
    {
        CANFrame.Byte[0] = 0;
    }

    CAN_SendData(CAN_CHANEEL1, &CANFrame);
}

void KL15StatusSend()
{
    static u8 Count = 0;
    u8 Data[POWERMSGLEN] = {0};

    if (100 == Count)
    {
        Data[0] = PwrSignalCofTable[KL15VOL].SignalType;
        Data[1] = PwrSignalCofTable[KL15VOL].DataLen;
        Data[2] = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1)?TRUE:FALSE;
        Count = 0;

        SendData(0x00, EN_MCU_APP4, EN_MPU_APP6, POWERMSGLEN, Data);/*周期发送KL15状态*/
    }
    else
    {
        Count++;
    }
}


void App_RxIndication(u8 Channel, pCAN_Frame_Type CANFrame)     /* 应用程序数据接收 */
{
    if(NULL != CANFrame)
    {
        switch(CANFrame->ID)
        {
            case 0x306:
                NodeFlashing = (CAN_CHANEEL1 == Channel && CANFrame->Byte[0])?TRUE:FALSE;   /* 从CAN上接收是否有节点正在升级信号 */
                break;
                
            default:
                break;
        
        }
    }
}


void MpuPowerOff(void)
{
    GPIO_ResetBits(GPIOB, GPIO_Pin_0);
    MpuPowerFlag = TRUE;
}


void MpuPowerOn(void)
{
    GPIO_SetBits(GPIOB, GPIO_Pin_0);
    
    if( MpuPowerFlag  )
    {
        GPIO_ResetBits(GPIOB, GPIO_Pin_3);
        MpuOnoffKeyPush = TRUE;
        PowerResumeWaitCount = 0;

        MpuPowerFlag = FALSE;
      }
    

}

void ExtendIFPowerOn(void)
{
        
    if(ExtendIFResetTime > 0)
    {
        GPIO_ResetBits(GPIOC, GPIO_Pin_5);
    }
    else
    {
        GPIO_SetBits(GPIOC, GPIO_Pin_5);
    }
}

void ExtendIFPowerOff(void)
{
    GPIO_ResetBits(GPIOC, GPIO_Pin_5);
}


void MpuFlashInfor(MSG_Queue *buff)/*接收到MPU发送的升级状态*/
{
    u8 MsgType = 0;
    u8 MsgLen = 0;

    MsgType = buff->mdata.mbuff[0];
    MsgLen = buff->mdata.mbuff[1];

    if (SELFFLASHTIME == MsgType || SELFFLASHFINISHED == MsgType || NODEFLASHING == MsgType || NODEFLASHFINISHED == MsgType)
    {
        SelfFlashWaitCount = 0;/*状态切换，所有计时器清零*/
        FlashFailedCount = 0;
        PowerResumeWaitCount = 0;
        PowerOffWaitCount = 0;
        PowerOffSendWaitCount = 0;
        SelfFlashWaitTime = 0;
    }
    else
    {
        ;
    }

    switch (MsgType)
    {
        case SELFFLASHTIME:
            if (2 == MsgLen && NodeFlashing)/*自升级之前必须先调用节点升级接口*/
            {
                SelfFlashWaitTime = (buff->mdata.mbuff[2] << 8 | buff->mdata.mbuff[3]) * 1000;
                SelfFlashing = TRUE;
                SelfFlashWaitCount = 0;
                State = SELF_FLASHING;

                u8 Data[4] = {0};

                Data[0] = SelfFlashing;
                Data[1] = 0x00;         //重试次数
                Data[2] = buff->mdata.mbuff[2];
                Data[3] = buff->mdata.mbuff[3];

                Eeprom_Write(EEPROM_SELFFLASHING, Data, 4);
              
            }
            else
            {
                ;
            }
            
            break;
        case SELFFLASHFINISHED:
            if (NodeFlashing)/*自升级之前必须先调用节点升级接口*/
            {
                SelfFlashing = FALSE;
                State = NORMAL;
            }
            else
            {
                ;
            }

            Eeprom_Write(EEPROM_SELFFLASHING, &SelfFlashing, 1);
                        
            break;
#if 0		//仅TBOX从Flash接收NodeFlash信息	
        case NODEFLASHING:
        
            NodeFlashing = TRUE;

            Eeprom_Write(EEPROM_NODEFLASHING, &NodeFlashing, 1);
        
            break;
        case NODEFLASHFINISHED:
            NodeFlashing = FALSE;
        
            Eeprom_Write(EEPROM_NODEFLASHING, &NodeFlashing, 1);
            break;
#endif 			
        case EXTENDIFRESET:
            //重置扩展接口

            //原始数据单位ms。
            ExtendIFResetTime = buff->mdata.mbuff[2] << 8;
            ExtendIFResetTime |= buff->mdata.mbuff[3];
            
            break;
       
        default:
            break;
    }
}
