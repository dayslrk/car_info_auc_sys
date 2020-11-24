#include "BaseType.h"
#include "ISO14229_1.h"
#include "Timer.h"
#include "St_Flash.h"
#include "task.h"
#include "system.h"
#include "ISO15765_2.h"
#include "CanQueue.h"
#include "Rng_Driver.h"
#include "Crc_Driver.h"
#include "Timer_Driver.h"
#include "Logging.h"
#include "Eeprom.h"
#include "delay.h"
#include "Trans.h"
#include "string.h"


static const SessionService_type ServiceList[DIAG_SERVICE_NUMBER] = 
{
    {TRUE, SESSION_CONTROL      , ZERO_LEVEL     , ZERO_LEVEL      , ZERO_LEVEL     , ZERO_LEVEL     , ZERO_LEVEL     , ZERO_LEVEL      , TRUE , 0x07, UDS_Service0x10Handle},
    {TRUE, RESET_ECU            , UNSUPPORT_LEVEL, ZERO_LEVEL      , ZERO_LEVEL     , ZERO_LEVEL     , ZERO_LEVEL     , ZERO_LEVEL      , TRUE , 0x01, UDS_Service0x11Handle},
    {TRUE, SECURITY_ACCESS      , UNSUPPORT_LEVEL, ZERO_LEVEL      , ZERO_LEVEL     , UNSUPPORT_LEVEL, UNSUPPORT_LEVEL, UNSUPPORT_LEVEL , FALSE, 0xFF, UDS_Service0x27Handle},
    {TRUE, COMMUNICATION_CONTROL, UNSUPPORT_LEVEL, UNSUPPORT_LEVEL , EXTENDED_LEVEL , UNSUPPORT_LEVEL, UNSUPPORT_LEVEL, EXTENDED_LEVEL  , TRUE , 0x0F, UDS_Service0x28Handle},
    {TRUE, TESTERPRESENT        , ZERO_LEVEL     , ZERO_LEVEL      , ZERO_LEVEL     , ZERO_LEVEL     , ZERO_LEVEL     , ZERO_LEVEL      , TRUE , 0x01, UDS_Service0x3EHandle},
    {TRUE, READ_DATA_BY_ID      , ZERO_LEVEL     , ZERO_LEVEL      , ZERO_LEVEL     , ZERO_LEVEL     , ZERO_LEVEL     , ZERO_LEVEL      , FALSE, 0xFF, UDS_Service0x22Handle},
    {TRUE, WRITE_DATA_BY_ID     , UNSUPPORT_LEVEL, ZERO_LEVEL      , ZERO_LEVEL     , UNSUPPORT_LEVEL, UNSUPPORT_LEVEL, UNSUPPORT_LEVEL , FALSE, 0xFF, UDS_Service0x2EHandle},
    {TRUE, ROUTINE_CONTROL      , UNSUPPORT_LEVEL, ZERO_LEVEL      , ZERO_LEVEL     , UNSUPPORT_LEVEL, UNSUPPORT_LEVEL, UNSUPPORT_LEVEL , FALSE, 0xFF, UDS_Service0x31Handle},
    {TRUE, REQUEST_DOWNLOAD     , UNSUPPORT_LEVEL, PROGRAMING_LEVEL, UNSUPPORT_LEVEL, UNSUPPORT_LEVEL, UNSUPPORT_LEVEL, UNSUPPORT_LEVEL , FALSE, 0xFF, UDS_Service0x34Handle},
    {TRUE, TRANSMIT_DATA        , UNSUPPORT_LEVEL, PROGRAMING_LEVEL, UNSUPPORT_LEVEL, UNSUPPORT_LEVEL, UNSUPPORT_LEVEL, UNSUPPORT_LEVEL , FALSE, 0xFF, UDS_Service0x36Handle},
    {TRUE, REQUEST_TRANSFER_EXIT, UNSUPPORT_LEVEL, PROGRAMING_LEVEL, UNSUPPORT_LEVEL, UNSUPPORT_LEVEL, UNSUPPORT_LEVEL, UNSUPPORT_LEVEL , FALSE, 0xFF, UDS_Service0x37Handle},
};/*每个服务的配置表*/
static const u8 CommunicationControl_MessageSupport = 0x01;/*通信控制报文类型：普通报文、网络管理报文、普通和网络管理报文*/
static const SecurityUnlock_Type UnlockList[SECURITYLEVEL_COUNT] = 
{
    {TRUE, EXTENDED_LEVEL   , 0x01, 0x02, 0x06, 0x0A},
    {TRUE, DEVELOPMENT_LEVEL, 0x03, 0x04, 0x06, 0x0A},
    {TRUE, PROGRAMING_LEVEL , 0x05, 0x06, 0x06, 0x0A}
};

const DIDConfigure_Type DIDList[DID_COUNT] = 
{
    {0xF183, 10, READONLY , 0x0001, ZERO_LEVEL      , 0x000B},
    {0xF187, 5 , READONLY , 0x000C, ZERO_LEVEL      , 0x0011},
    {0xF18C, 16, READWRITE, 0x0012, ZERO_LEVEL      , 0x0022},
	{0xF190, 17, READWRITE, 0x0023, EXTENDED_LEVEL  , 0x0034},
    {0xF191, 4 , READONLY , 0x0035, ZERO_LEVEL      , 0x0039},
    {0xF1A0, 6 , READONLY , 0x003A, ZERO_LEVEL      , 0x0040},
	{0xF1A1, 6 , READONLY , 0x0041, ZERO_LEVEL      , 0x0047}
};
/*static const u8 ECUBootloaderSoftwareReferenceNumber[10] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static const u8 ECUHardwareNumber[4] = {0x00, 0x00, 0x00, 0x00};*/
static RoutineControl_Type RoutineList[ROUTINE_COUNT] = 
{
    {0x0203, 0, ZERO_LEVEL     , ZERO_LEVEL      , ZERO_LEVEL     , 0x05, UDS_CheckProgrammingPreConditions, 0xFF, TRUE},
    {0xFF00, 8, UNSUPPORT_LEVEL, PROGRAMING_LEVEL, UNSUPPORT_LEVEL, 0x05, UDS_EraseMemory                  , 0xFF, TRUE},
    {0xFF01, 4, UNSUPPORT_LEVEL, PROGRAMING_LEVEL, UNSUPPORT_LEVEL, 0x05, UDS_CheckApplicationDataValid    , 0xFF, TRUE},
    {0x6666, 0, EXTENDED_LEVEL , EXTENDED_LEVEL  , EXTENDED_LEVEL , 0x05, UDS_APPRollBack                  , 0xFF, TRUE}
};


UDS_Msg_Type HandlingMsg;/*当前正在处理的诊断消息内容*/
PostiveResponse_Type PosRespBuffer;/*正响应数据buffer*/
DiagTimer_Type ResponsePendingTimer;/*响应超时Timer*/
DiagTimer_Type ResponseTimeoutTimer;/*连续发送0x78NRC响应Timer*/
static u8 ResponsePendingCount = 0;/*发送0x78NRC次数*/
/*static bool DealTimeout = FALSE;*//*0x78发送超时，诊断数据处理超时，应及时退出*/
bool SuppressResponse = FALSE;/*抑制诊断服务正响应*/
bool ResponsePending = FALSE;/*响应处理中*/
static SecurityLevel_enum CurrentSecurityLevel = ZERO_LEVEL;/*当前安全等级*/
static NegativeResposeCode_enum NRC = PR;/*需要发送的否定响应码*/
u8 CurrentSession = DEFAULT_SESSION;/*当前会话模式*/
DiagTimer_Type SessionTimer;/*会话超时Timer*/
bool Reprogram_WaitConfirm = TRUE;/*进入编程会话确认，10 02响应发送完毕*/
bool Reset_WaitConfirm = TRUE;/*重启确认，11 01响应发送完毕*/
static u8 ResetType = HARD_RESET;
static SecurityUnlockStep_enum UnlockStep = WAIT_REQ_SEED;/*27服务当前解锁步骤*/
static u8 UnlockFailedCounter = 0;
DiagTimer_Type UnlockFailedTimer;/*延时机制Timer*/
static u8 Seed[4] = {0};/*27服务发送的种子*/
static u8 Key[4] = {0};/*27服务根据种子生成的钥匙*/
static u32 RandNext = 0;/*产生随机数种子*/
u32 Personalkey = 0x65332941;/*27服务内置计算种子*/
extern bool NormalMessageRecEnable;/*普通消息接收使能*/
extern bool NormalMessageSendEnable;/*普通消息发送使能*/
static u32 EraseMemoryAddress = 0;/*31服务指定擦除数据地址*/
static u32 EraseMemoryLength = 0;/*31服务指定擦除数据大小*/
static u32 ReprogramMemoryAddress = 0;/*34服务指定下载数据地址*/
static u32 ReprogramMemoryLength = 0;/*34服务指定下载数据大小*/
static u32 ProgramReceivedLength = 0;/*已接收程序长度*/
static u8 BlockIndex = 0;/*36服务block索引*/
static bool Downloading = FALSE;/*下载状态，TRUE：下载中*/
u32 ReceiveCrc = 0xFFFFFFFF;/*存储接收的CRC*/
u8 AppSoftwareNumber[6] = {0, 0, 5, 0x20, 07, 9};/*存储接收的软件版本号*/


/*static TaskHandle_t Task_CheckProgrammingPreConditions;
static TaskHandle_t Task_EraseMemory;
static TaskHandle_t Task_CheckApplicationDataValid;*/

static TaskStatues_enum Sta_CheckProgrammingPreConditions;
static TaskStatues_enum Sta_EraseMemory;
static TaskStatues_enum Sta_CheckApplicationDataValid;
static TaskStatues_enum Sta_APPRollBack;
static TaskStatues_enum Sta_CopyAPPCode;

static TaskHandle_t Task_UDSTimeOperation;

unsigned char ReprogramSessionDelay = 0;


/*******************************************************************************
* 函数名称: UDS_Init
* 功能描述: 14229初始化函数
* 输入参数: 无
* 输出参数: 无
* 返回参数: 无
* 备    注： 
*******************************************************************************/
void UDS_Init(void)
{
    u16 i = 0;
    u8 NormalMessageEnableStatues = 0;
    u8 FlashAppSoftwareNumber[6] = {0};
    u8 Index = 0;
    bool BootSoftNumRight = FALSE;
    
    HandlingMsg.SID = 0x00;
    for (i = 0; i < MAX_UDS_RCV_Buff_Size; ++i)
    {
        HandlingMsg.Data[i] = 0;
    }
    HandlingMsg.Len = 0;
    HandlingMsg.FrameType = DIAG_PHY;

    for (i = 0; i < MAX_UDS_RES_Buff_Size; ++i)
    {
        PosRespBuffer.Data[i] = 0;
    }
    PosRespBuffer.Len = 0;
    
    ResponsePendingTimer.TimerCounter = 0;
    ResponsePendingTimer.Valid = FALSE;
    ResponseTimeoutTimer.TimerCounter = 0;
    ResponseTimeoutTimer.Valid = FALSE;
    SessionTimer.TimerCounter = 0;
    SessionTimer.Valid = FALSE;
    UnlockFailedTimer.TimerCounter = 0;
    UnlockFailedTimer.Valid = FALSE;
    
    ResponsePendingCount = 0;
    /*DealTimeout = FALSE;*/
    SuppressResponse = FALSE;
    ResponsePending = FALSE;
    CurrentSecurityLevel = ZERO_LEVEL;
    NRC = PR;
    CurrentSession = DEFAULT_SESSION;
    Reprogram_WaitConfirm = TRUE;
    Reset_WaitConfirm = TRUE;
    UnlockStep = WAIT_REQ_SEED;
    UnlockFailedCounter = 0;
    RandNext = 0;
    Personalkey = 0x65332941;
    EraseMemoryAddress = 0;
    EraseMemoryLength = 0;
    ReprogramMemoryAddress = 0;
	
    ReprogramMemoryLength = 0;
    ProgramReceivedLength = 0;
    BlockIndex = 0;
    Downloading = FALSE;
    ReceiveCrc = 0xFFFFFFFF;
    
    for (i = 0; i < 4; ++i)
    {
        Seed[i] = 0;
        Key[i] = 0;
    }

    /*Flash_Write(DIDList[1].EEPromAddr, (u32 *)ECUBootloaderSoftwareReferenceNumber, (u32)10);*//*写ECU BootLoader软件号，不能通过2E服务写*/
    /*Flash_Write(DIDList[3].EEPromAddr, (u32 *)ECUHardwareNumber, (u32)4);*//*写ECU硬件号，不能通过2E服务写*/
    AT24Cxx_Read(COMCONTROL_NORMALMESSAGEADDR, &NormalMessageEnableStatues, (u16)1);/*从内存中读取0x28服务的控制情况*/
    /*普通应用报文接收使能或禁止*/
    NormalMessageRecEnable = ((ENABLERX_TX == NormalMessageEnableStatues || ENABLERX_DISABLETX == NormalMessageEnableStatues))?TRUE:
        (((DISABLE_RXTX == NormalMessageEnableStatues || DISABLERX_ENABLETX == NormalMessageEnableStatues))?FALSE:NormalMessageRecEnable);
    /*普通应用报文发送使能或禁止*/
    NormalMessageSendEnable = ((ENABLERX_TX == NormalMessageEnableStatues || DISABLERX_ENABLETX == NormalMessageEnableStatues))?TRUE:
        (((DISABLE_RXTX == NormalMessageEnableStatues || ENABLERX_DISABLETX == NormalMessageEnableStatues))?FALSE:NormalMessageRecEnable);

    for (Index = 0; Index < DID_COUNT; ++Index)/*检查App软件版本号*/
    {
        if (0xF183 == DIDList[Index].ID)
        {
            AT24Cxx_Read(DIDList[Index].EEPromAddr, FlashAppSoftwareNumber, (u16)(DIDList[Index].Size));
            for (i = 0; i < DIDList[Index].Size; ++i)
            {
                BootSoftNumRight = (FlashAppSoftwareNumber[i] == AppSoftwareNumber[i])?TRUE:FALSE;
                if (FALSE == BootSoftNumRight)
                {
                    break;
                }
                else
                {
                    continue;
                }
            }
            
            if (FALSE == BootSoftNumRight)
            {
                AT24Cxx_Write(DIDList[Index].EEPromAddr, AppSoftwareNumber, (u16)(DIDList[Index].Size));
            }
            else
            {
                ;
            }
        }
        else
        {
            continue;
        }
    }
}


/*******************************************************************************
* 函数名称: UDS_RxIndication
* 功能描述: 将接收的数据压入UDS的处理队列
* 输入参数: Data：接收的诊断数据 Len：接收数据的长度 
*           FrameType：当前接收诊断帧的类型（物理寻址/功能寻址）
*			Channel: 诊断指令接受通道 1-CAN /2-MPU
* 输出参数: 无
* 返回参数: 无
* 备    注： tp层接收到数据后，调用此函数
*******************************************************************************/
void UDS_RxIndication(u8 *Data, u16 Len, u8 FrameType, u8 Channel, u8 apptype)
{
    if (NULL != Data)
    {
        u16 i = 0;
        UDS_Msg_Type ReceiveBuffer;
        
        ReceiveBuffer.SID = Data[0];
        ReceiveBuffer.Len = Len - 1;
        ReceiveBuffer.FrameType = FrameType;
        ReceiveBuffer.Channel = Channel;
        ReceiveBuffer.apptype = apptype;

        for (i = 0; i < (Len - 1); ++i)
        {
            ReceiveBuffer.Data[i] = Data[i + 1];
        }
        
        UDS_Queue_Push(&ReceiveBuffer);/*压入UDS的消息处理队列*/
    }
    else
    {
        ;/*数据指针为空*/
    }
}

/*******************************************************************************
* 函数名称: UDS_TxConfirmation
* 功能描述: TP层的发送确认函数
* 输入参数: SID：当前发送结束的SID Subfunction：当前发送结束的子功能
*           Result：发送结果
* 输出参数: 无
* 返回参数: 无
* 备    注： ISO15765_SendPacket中被调用
*******************************************************************************/
void UDS_TxConfirmation(u8 SID, u8 Subfunction, u8 Result)
{
    if (0x10 + 0x40 == SID && 0x02 == Subfunction && Tx_SUCCESS == Result)
    {
        /*10 02的正响应已发送结束，可以重启*/
        Reprogram_WaitConfirm = FALSE;
    }
    else if (0x11 + 0x40 == SID && 0x01 == Subfunction && Tx_SUCCESS == Result)
    {
        /*11 01的正响应已发送结束，可以重启*/
        Reset_WaitConfirm = FALSE;
    }
    else
    {
        ;/*！！！目前对于需要重启的指令，正响应发送失败则不会进行重启操作*/
    }
}
/*******************************************************************************
* 函数名称: UDS_MainFunction
* 功能描述: UDS的主函数，查询若有数据需要处理，调用处理函数进行处理
* 输入参数: 无
* 输出参数: 无
* 返回参数: 无
* 备    注： UDS的主函数被周期调用
*******************************************************************************/
void UDS_MainFunction(void)
{
    u8 SendFailedCount = 0;
    u8 Channel = 0;
    u8 apptype = 0;
    
    UDS_TaskClose();/*检测31服务和34服务创建task运行情况，运行完成则关闭task*/
    
    /*若有消息需要处理*/
    if (UDS_Queue_GetRecCount() && !ResponsePending)
    {
        UDS_TimerRun(&ResponsePendingTimer, RESPONSE_TIMEOUT, UDS_ResponseTimeout);
        ResponsePending = TRUE;
        /*DealTimeout = FALSE;*/
            
        UDS_Queue_Pop(&HandlingMsg);/*取出数据*/

		Channel = HandlingMsg.Channel;
		apptype = HandlingMsg.apptype;
        
        /*UDS的服务处理函数*/
        UDS_ServiceHandle(HandlingMsg.SID, HandlingMsg.Len, HandlingMsg.FrameType, HandlingMsg.Data);

		UDS_TimerStop(&ResponsePendingTimer);
		UDS_TimerStop(&ResponseTimeoutTimer);

        if (DIAG_FUNC == HandlingMsg.FrameType && (SNS == NRC || SFNS == NRC || ROOR == NRC || SFNSIAS == NRC))
        {
            ;/*功能寻址的服务不支持、子功能不支持、请求超出范围、当前子功能状态下不支持否定响应码，均不给予响应*/
        }
        else if (TRUE == SuppressResponse && PR == NRC)
        {
            ;/*正响应抑制位*/
        }
        else if (PR == NRC && 0 != PosRespBuffer.Len)
        {
        	if( 1 == Channel)
        	{
		        while (SendFailedCount < 3)
		        {
		    	    if (ISO15765_MakePacket(DIAGCANTXID, PosRespBuffer.Data, PosRespBuffer.Len))/*无否定响应码，发送正响应数据*/
		            {
		                break;
		            }
		            else
		            {
		                SendFailedCount++;
		            }
		        }
            }

            if(2 == Channel)
            {
                SendData_EN_MCU_APP1((EN_APPS)apptype, DIAGCANTXID, PosRespBuffer.Len , PosRespBuffer.Data);
                
            }
            
            ResponsePending = FALSE;
            ResponsePendingCount = 0;
        }
        else if (PR != NRC)
        {
            /*填充否定响应数据*/
            PosRespBuffer.Data[0] = 0x7F;
            PosRespBuffer.Data[1] = HandlingMsg.SID;
            PosRespBuffer.Data[2] = NRC;
			
			PosRespBuffer.Len = 3;

            if(1 == Channel)
            {
	            SendFailedCount = 0;
	            while (SendFailedCount < 3)
	            {
	        	    if (ISO15765_MakePacket(DIAGCANTXID, PosRespBuffer.Data, 3))/*发送否定响应数据*/
	                {
	                    break;
	                }
	                else
	                {
	                    SendFailedCount++;
	                }
	            }
	      	}

            if(2 == Channel)
            {
                SendData_EN_MCU_APP1((EN_APPS)apptype, DIAGCANTXID, PosRespBuffer.Len , PosRespBuffer.Data);
            }
            
            ResponsePending = FALSE;
            ResponsePendingCount = 0;
        }
        else
        {
            ;
        }

        PosRespBuffer.Len = 0;

        if (DEFAULT_SESSION != CurrentSession)
        {
            UDS_TimerRun(&SessionTimer, SESSIONTIMEOUT, UDS_SessionTimeout);/*非默认会话模式下，每收到一条诊断指令，会刷新会话管理的timer*/
        }
    }
    else
    {
        ;
    }

    if (!Reprogram_WaitConfirm)
    {
	  	ReprogramSessionDelay++;
		if (ReprogramSessionDelay == 20)
		{
			UDS_EnterReprogramSession();/*执行进入编程会话操作操作*/
			Reprogram_WaitConfirm = TRUE;
			ReprogramSessionDelay = 0;
		}
    }
    else
    {
        ;
    }

    if (!Reset_WaitConfirm)
    {
	 	ReprogramSessionDelay++;
		if (ReprogramSessionDelay == 20)
		{
			UDS_Reset();
			Reset_WaitConfirm = TRUE;
			ReprogramSessionDelay = 0;
		}
    }
    else
    {
        ;
    }
}


/*******************************************************************************
* 函数名称: UDS_ServiceHandle
* 功能描述: 查询当前诊断服务是否支持，以及当前会话和安全等级是否满足要求
* 输入参数: SID：当前处理的诊断服务 Len：需要处理数据的长度 
*           FrameType：诊断帧的类型 Data：需要处理的数据
* 输出参数: 无
* 返回参数: 无
* 备    注： 在UDS的主函数中被调用
*******************************************************************************/
void UDS_ServiceHandle(u8 SID, u16 Len, u8 FrameType, u8 *Data)
{
    u8 ServiceIndex = 0;
    bool ServiceValid = FALSE;/*当前处理的诊断服务是否支持*/

    if (NULL!= Data)
    {
        /*查询诊断服务配置表当前诊断服务是否支持*/  
        for (ServiceIndex = 0; ServiceIndex < DIAG_SERVICE_NUMBER; ++ServiceIndex)
        {
            if (ServiceList[ServiceIndex].ServiceName == SID)
            {
                if (ServiceList[ServiceIndex].Support)
                {
                    ServiceValid = TRUE;
                    break;
                }
                else
                {
                    ServiceValid = FALSE;
                    continue;
                }
            } 
            else
            {
                ;
            }
        }

        if (TRUE == ServiceValid)/*支持当前处理诊断服务*/
        {
            if (DIAG_PHY == FrameType)/*物理寻址*/
            {
                SuppressResponse = FALSE;
                switch (CurrentSession)
                {
                    case DEFAULT_SESSION:/*当前为默认会话*/
                        if (UNSUPPORT_LEVEL == ServiceList[ServiceIndex].PHYDefaultSession_Security)
                        {
                            NRC = SNSIAS;/*当前会话下服务不支持*/
                        }
                        else
                        {
                            /*当前安全等级为配置表中当前诊断服务要求的安全等级*/
                            if (CurrentSecurityLevel == ServiceList[ServiceIndex].PHYDefaultSession_Security || 
                                ZERO_LEVEL == ServiceList[ServiceIndex].PHYDefaultSession_Security)
                            {
                                ServiceList[ServiceIndex].SIDHandle(Len, FrameType, Data, ServiceIndex);
                            }
                            else
                            {
                                NRC = SAD;/*安全访问未通过*/
                            }
                        }
                        break;
                    case PROGRAM_SESSION:/*当前为编程会话*/
                        if (UNSUPPORT_LEVEL == ServiceList[ServiceIndex].PHYProgramSession_Security)
                        {
                            NRC = SNSIAS;
                        }
                        else
                        {
                            if (CurrentSecurityLevel == ServiceList[ServiceIndex].PHYProgramSession_Security || 
                                ZERO_LEVEL == ServiceList[ServiceIndex].PHYProgramSession_Security)
                            {
                                ServiceList[ServiceIndex].SIDHandle(Len, FrameType, Data, ServiceIndex);
                            }
                            else
                            {
                                NRC = SAD;
                            }
                        }
                        break;
                    case EXTENDED_SESSION:/*当前为扩展会话*/
                        if (UNSUPPORT_LEVEL == ServiceList[ServiceIndex].PHYExtendedSession_Security)
                        {
                            NRC = SNSIAS;
                        }
                        else
                        {
                            if (CurrentSecurityLevel == ServiceList[ServiceIndex].PHYExtendedSession_Security || 
                                ZERO_LEVEL == ServiceList[ServiceIndex].PHYExtendedSession_Security)
                            {
                                ServiceList[ServiceIndex].SIDHandle(Len, FrameType, Data, ServiceIndex);
                            }
                            else
                            {
                                NRC = SAD;
                            }
                        }
                        break;
                }
            }
            else if (DIAG_FUNC == FrameType)/*功能寻址*/
            {
                switch (CurrentSession)
                {
                    case DEFAULT_SESSION:/*当前为默认会话*/
                        if (UNSUPPORT_LEVEL == ServiceList[ServiceIndex].FUNCDefaultSession_Security)
                        {
                            NRC = SNSIAS;
                        }
                        else
                        {
                            if (CurrentSecurityLevel == ServiceList[ServiceIndex].FUNCDefaultSession_Security || 
                                ZERO_LEVEL == ServiceList[ServiceIndex].FUNCDefaultSession_Security)
                            {
                                ServiceList[ServiceIndex].SIDHandle(Len, FrameType, Data, ServiceIndex);
                            }
                            else
                            {
                                NRC = SAD;
                            }
                        }
                        break;
                    case PROGRAM_SESSION:/*当前为编程会话*/
                        if (UNSUPPORT_LEVEL == ServiceList[ServiceIndex].FUNCProgramSession_Security)
                        {
                            NRC = SNSIAS;
                        }
                        else
                        {
                            if (CurrentSecurityLevel == ServiceList[ServiceIndex].FUNCProgramSession_Security || 
                                ZERO_LEVEL == ServiceList[ServiceIndex].FUNCProgramSession_Security)
                            {
                                ServiceList[ServiceIndex].SIDHandle(Len, FrameType, Data, ServiceIndex);
                            }
                            else
                            {
                                NRC = SAD;
                            }
                        }
                        break;
                    case EXTENDED_SESSION:/*当前为扩展会话*/
                        if (UNSUPPORT_LEVEL == ServiceList[ServiceIndex].FUNCExtendedSession_Security)
                        {
                            NRC = SNSIAS;
                        }
                        else
                        {
                            if (CurrentSecurityLevel == ServiceList[ServiceIndex].FUNCExtendedSession_Security || 
                                ZERO_LEVEL == ServiceList[ServiceIndex].FUNCExtendedSession_Security)
                            {
                                ServiceList[ServiceIndex].SIDHandle(Len, FrameType, Data, ServiceIndex);
                            }
                            else
                            {
                                NRC = SAD;
                            }
                        }
                        break;
                }
            }
        }
        else/*服务不支持*/
        {
            if (DIAG_PHY == FrameType)/*功能寻址服务不支持，无需响应*/
            {
                NRC = SNS;/*服务不支持*/
                SuppressResponse = FALSE;
            }
            else
            {
                ;
            }
        }
    }
    else
    {
        ;/*数据指针为空*/
    }
}


/*******************************************************************************
* 函数名称: UDS_ResponseTimeout
* 功能描述: 诊断数据处理超过500ms还未发送，则进入此函数发送0x78否定响应
* 输入参数: 无
* 输出参数: 无
* 返回参数: 无
* 备    注： 
*******************************************************************************/
void UDS_ResponseTimeout(void)
{
    u8 SendFailedCount = 0;

	u8 TimeOutRes[3]= {0};
    
    ResponsePendingCount++;
    
    if (ResponsePendingCount > NRC0x78SEND_MAXCOUNT)/*发送0x78NRC超过规定次数，防止进入发送0x78NRC的死循环*/
    {
        UDS_TimerStop(&ResponsePendingTimer);
        UDS_TimerStop(&ResponseTimeoutTimer);
        ResponsePending = FALSE;
        ResponsePendingCount = 0;

        /*DealTimeout = TRUE;*/
    }
    else
    {
        /*填充否定响应数据*/
        TimeOutRes[0] = 0x7F;
        TimeOutRes[1] = HandlingMsg.SID;
        TimeOutRes[2] = RCRRP;
        
        while (SendFailedCount < 3)
        {
    	    if (ISO15765_MakePacket(DIAGCANTXID, TimeOutRes, 3))/*发送否定响应0x78数据*/
            {
                break;
            }
            else
            {
                SendFailedCount++;
            }
        }
        
        UDS_TimerRun(&ResponseTimeoutTimer, NRC0x78Interval_TIME, UDS_ResponseTimeout);/*没间隔1.5s发送一次0x78NRC，直到诊断帧处理完毕发送响应*/
    }
}


/*******************************************************************************
* 函数名称: UDS_TimeOperation
* 功能描述: 采用创建task形式，执行耗时操作
* 输入参数: 无
* 输出参数: 无
* 返回参数: 无
* 备    注： 
*******************************************************************************/
void UDS_TimeOperation(void)
{
    if (RUNNING == Sta_CheckApplicationDataValid)
    {
        UDS_CheckApplicationDataValid();
    }
    else if (RUNNING == Sta_CheckProgrammingPreConditions)
    {
        UDS_CheckProgrammingPreConditions();
    }
    else if (RUNNING == Sta_CopyAPPCode)
    {
        UDS_BackupAPPCode();
    }
    else if (RUNNING == Sta_EraseMemory)
    {
        UDS_EraseMemory();
    }
    else if (RUNNING == Sta_APPRollBack)
    {
        UDS_APPRollBack();
    }

    while(1);
}


/*******************************************************************************
* 函数名称: UDS_TaskClose
* 功能描述: 检测31服务和34服务创建的task是否允许完毕，运行完毕则关闭task
* 输入参数: 无
* 输出参数: 无
* 返回参数: 无
* 备    注： 
*******************************************************************************/
void UDS_TaskClose(void)
{
    bool TaskOperation = FALSE;
    
    if (FINISHED == Sta_CheckApplicationDataValid)
    {
        TaskOperation = TRUE;
        Sta_CheckApplicationDataValid = NOTRUN;/*修改Task状态为未运行*/
    }
    else if (FINISHED == Sta_CheckProgrammingPreConditions)
    {
        TaskOperation = TRUE;
        Sta_CheckProgrammingPreConditions = NOTRUN;/*修改Task状态为未运行*/
    }
    else if (FINISHED == Sta_CopyAPPCode)
    {
        TaskOperation = TRUE;
        Sta_CopyAPPCode = NOTRUN;/*修改Task状态为未运行*/
    }
    else if (FINISHED == Sta_EraseMemory)
    {
        TaskOperation = TRUE;
        Sta_EraseMemory = NOTRUN;/*修改Task状态为未运行*/
    }
    else if (FINISHED == Sta_APPRollBack)
    {
        TaskOperation = TRUE;
        Sta_APPRollBack = NOTRUN;
    }

    if (TaskOperation)
    {
        taskENTER_CRITICAL();/*进入临界区*/
        vTaskDelete(Task_UDSTimeOperation);
        taskEXIT_CRITICAL();/*退出临界区*/
    }
    else
    {
        ;
    }
}


/*******************************************************************************
* 函数名称: UDS_Service0x10Handle
* 功能描述: 会话控制（0x10）服务的处理函数
* 输入参数: Len：接收诊断数据长度 FrameType：接收诊断帧类型 Data：诊断数据
*           ServiceIndex：当前服务在配置表中的索引
* 输出参数: 无
* 返回参数: 无
* 备    注： 
*******************************************************************************/
void UDS_Service0x10Handle(u16 Len, u8 FrameType, u8 *Data, u8 ServiceIndex)
{
    u8 Subfunction = 0;
    u8 SuppressSupportBit = 0;
    
    NRC = PR;

    if (NULL == Data)
    {
        return;
    }
    else
    {
        ;
    }
    
    if (1 == Len)
/*长度正确*/
    {
        if (TRUE == ServiceList[ServiceIndex].SuppressSupport)/*10服务支持抑制正响应bit*/
        {
            Subfunction = Data[0] & 0x7f;
            SuppressSupportBit = Data[0] & 0x80;
        }
        else
        {
            Subfunction = Data[0];
            SuppressSupportBit = FALSE;
        }

        switch (Subfunction)/*检查请求子功能是否支持*/
        {
            case DEFAULT_SESSION:
                NRC = (!(ServiceList[ServiceIndex].SubfunctionSupport & 0x01))?SFNS:NRC;
                break;
            case PROGRAM_SESSION:
                if (!(ServiceList[ServiceIndex].SubfunctionSupport & 0x02))
                {
                    NRC = SFNS;/*子功能不支持*/
                }
                else
                {
                    /*不允许从默认会话直接切换到编程会话*/
                    NRC = ((DEFAULT_SESSION == CurrentSession) && (!SERVICE0x10_01TO02SUPPORT))?SFNSIAS:NRC;
                }
                break;
            case EXTENDED_SESSION:
                if (!(ServiceList[ServiceIndex].SubfunctionSupport & 0x04))
                {
                    NRC = SFNS;/*子功能不支持*/
                }
                else
                {
                    /*不允许从编程会话直接切换到扩展会话*/
                    NRC = ((PROGRAM_SESSION == CurrentSession) && (!SERVICE0x10_02TO03SUPPORT))?SFNSIAS:NRC;
                }
                break;
            default:
                NRC = SFNS;/*子功能不支持*/
                break;
        }
    }
    else
    {
        NRC = IMLOIF;/*报文长度错误*/
    }

    /*无错误码且正响应抑制bit为1*/
    SuppressResponse = (SuppressSupportBit && PR == NRC)?TRUE:((PR == NRC)?FALSE:TRUE);

    if (!SuppressSupportBit && PR == NRC)/*正响应无抑制且无错误码，填充响应数据*/
    {
        PosRespBuffer.Data[0] = ServiceList[ServiceIndex].ServiceName + 0x40;
        PosRespBuffer.Data[1] = Subfunction;
        PosRespBuffer.Data[2] = (u8)(P2SERVERMAX >> 8);
        PosRespBuffer.Data[3] = (u8)(P2SERVERMAX);
        PosRespBuffer.Data[4] = (u8)(P2PSERVERMAX >> 8);
        PosRespBuffer.Data[5] = (u8)(P2PSERVERMAX);

        PosRespBuffer.Len = 6;
    }
    else
    {
        ;
    }

    if (PR == NRC)
    {
        UDS_GoToSession(Subfunction);/*执行会话切换操作*/
    }
}


/*******************************************************************************
* 函数名称: UDS_GoToSession
* 功能描述: 执行会话切换的操作
* 输入参数: Session：需要切换的会话
* 输出参数: 无
* 返回参数: 无
* 备    注： 
*******************************************************************************/
void UDS_GoToSession(u8 Session)
{
    CurrentSession = Session;
    CurrentSecurityLevel = ZERO_LEVEL;

    /*非默认会话均需维持5.2s定时器，超时切换为默认会话*/
    if (DEFAULT_SESSION != CurrentSession)
    {
        UDS_TimerRun(&SessionTimer, SESSIONTIMEOUT, UDS_SessionTimeout);
    }
    else
    {
        ;
    }

    if (PROGRAM_SESSION == CurrentSession)
    {
        Reprogram_WaitConfirm = SuppressResponse?FALSE:TRUE;/*若无需响应，则无需等待，否则复位flag，等待响应完成*/
    }
    else
    {
        ;
    }

    UnlockStep = (UnlockStep != WAIT_DELAY)?WAIT_REQ_SEED:UnlockStep;

}


/*******************************************************************************
* 函数名称: UDS_SessionTimeout
* 功能描述: 非默认会话超时，切换到默认会话
* 输入参数: 无
* 输出参数: 无
* 返回参数: 无
* 备    注： 
*******************************************************************************/
void UDS_SessionTimeout(void)
{
    UDS_GoToSession(DEFAULT_SESSION);
}


/*******************************************************************************
* 函数名称: UDS_EnterReprogramSession
* 功能描述: 进入编程会话
* 输入参数: 无
* 输出参数: 无
* 返回参数: 无
* 备    注： 
*******************************************************************************/
void UDS_EnterReprogramSession(void)
{
    u8 WriteSession = 0x02;
    AT24Cxx_Write(CURRENT_SESSION, &WriteSession, (u16)1);
	SystemReset();
}


/*******************************************************************************
* 函数名称: UDS_Service0x11Handle
* 功能描述: ECU服务（0x11）服务的处理函数
* 输入参数: Len：接收诊断数据长度 FrameType：接收诊断帧类型 Data：诊断数据
*           ServiceIndex：当前服务在配置表中的索引
* 输出参数: 无
* 返回参数: 无
* 备    注： 
*******************************************************************************/
void UDS_Service0x11Handle(u16 Len, u8 FrameType, u8 *Data, u8 ServiceIndex)
{
    u8 Subfunction = 0;
    u8 SuppressSupportBit = 0;
    
    NRC = PR;

    if (NULL == Data)
    {
        return;
    }
    else
    {
        ;
    }

    if (1 == Len)
/*长度正确*/
    {
        if (TRUE == ServiceList[ServiceIndex].SuppressSupport)/*10服务支持抑制正响应bit*/
        {
            Subfunction = Data[0] & 0x7f;
            SuppressSupportBit = Data[0] & 0x80;
        }
        else
        {
            Subfunction = Data[0];
            SuppressSupportBit = FALSE;
        }

        switch (Subfunction)
        {
            case HARD_RESET:
                NRC = (!(ServiceList[ServiceIndex].SubfunctionSupport & 0x01))?SFNS:NRC;
                break;
            case KEYOFFON_RESET:
                NRC = (!(ServiceList[ServiceIndex].SubfunctionSupport & 0x02))?SFNS:NRC;
                break;
            case SOFT_RESET:
                NRC = (!(ServiceList[ServiceIndex].SubfunctionSupport & 0x04))?SFNS:NRC;
                break;
            case ENABLERAPIDPOWERSHUTDOWN:
                NRC = (!(ServiceList[ServiceIndex].SubfunctionSupport & 0x08))?SFNS:NRC;
                break;
            case DISENABLERAPIDPOWERSHUTDOWN:
                NRC = (!(ServiceList[ServiceIndex].SubfunctionSupport & 0x10))?SFNS:NRC;
                break;
            default:
                NRC = SFNS;
                break;
        }
    }
    else
    {
        NRC = IMLOIF;
    }

    /*无错误码且正响应抑制bit为1*/
    SuppressResponse = (SuppressSupportBit && PR == NRC)?TRUE:((PR == NRC)?FALSE:TRUE);
    
    if (!SuppressSupportBit && PR == NRC)/*正响应无抑制且无错误码，填充响应数据*/
    {
        PosRespBuffer.Data[0] = ServiceList[ServiceIndex].ServiceName + 0x40;
        PosRespBuffer.Data[1] = Subfunction;

        PosRespBuffer.Len = 2;
    }
    else
    {
        ;
    }

    if (PR == NRC)
    {
		ResetType = Subfunction;
			
        if(SuppressResponse == FALSE)/*需要正响应,等待响应结束*/
        {
            Reset_WaitConfirm = TRUE;
        }
        else/*不需要正响应时直接复位*/
        {
            UDS_Reset();
        }
    }
}


/*******************************************************************************
* 函数名称: UDS_Reset
* 功能描述: 执行Reset操作
* 输入参数: ResetType：复位方式
* 输出参数: 无
* 返回参数: 无
* 备    注： 
*******************************************************************************/
void UDS_Reset(void)
{
    switch (ResetType)
    {
        case HARD_RESET:
            SystemReset();
            break;
        case KEYOFFON_RESET:
            break;
        case SOFT_RESET:
            break;
        case ENABLERAPIDPOWERSHUTDOWN:
            break;
        case DISENABLERAPIDPOWERSHUTDOWN:
            break;
        default:
            break;
    }
}


/*******************************************************************************
* 函数名称: UDS_Service0x27Handle
* 功能描述: 安全访问（0x27）服务的处理函数
* 输入参数: Len：接收诊断数据长度 FrameType：接收诊断帧类型 Data：诊断数据
*           ServiceIndex：当前服务在配置表中的索引
* 输出参数: 无
* 返回参数: 无
* 备    注： 
*******************************************************************************/
void UDS_Service0x27Handle(u16 Len, u8 FrameType, u8 *Data, u8 ServiceIndex)
{
    u8 Subfunction;
    u8 SuppressSupportBit = 0;
    u8 Index = 0;
    bool SubfunctionSupport = FALSE;/*当前子功能是否支持*/
    bool SubfunctionValid = FALSE;/*根据当前会话模式和寻址方式判断子功能是否支持*/
    
    NRC = PR;

    if (NULL == Data)
    {
        return;
    }
    else
    {
        ;
    }

    if (Len >= 1)
/*内部判断具体长度*/
    {
        if (TRUE == ServiceList[ServiceIndex].SuppressSupport)/*10服务支持抑制正响应bit*/
        {
            Subfunction = Data[0] & 0x7f;
            SuppressSupportBit = Data[0] & 0x80;
        }
        else
        {
            Subfunction = Data[0];
            SuppressSupportBit = FALSE;
        }

        /*由于27服务可能每个子功能在不同寻址方式和会话模式下支持情况不同，所以不查询ServerList中的SubfunctionSupport，单独配置UnlockList.Support*/
        for (Index = 0; Index < 3; ++Index)/*查询配置表，当前子功能请求的安全等级是否支持*/
        {
            if (TRUE == UnlockList[Index].Valid && (UnlockList[Index].SeedID == Subfunction|| UnlockList[Index].KeyID == Subfunction))
            {
                SubfunctionValid = TRUE;
                switch (CurrentSession)
                {
                    case DEFAULT_SESSION:
                        /*判断物理寻址和功能寻址的默认会话下是否支持*/
                        SubfunctionValid = (DIAG_PHY == FrameType)?(UnlockList[Index].Support & (0x01 << 0)?TRUE:FALSE):
                            (UnlockList[Index].Support & (0x01 << 3)?TRUE:FALSE);
                        break;
                    case PROGRAM_SESSION:
                        /*判断物理寻址和功能寻址的编程会话下是否支持*/
                        SubfunctionValid = (DIAG_PHY == FrameType)?(UnlockList[Index].Support & (0x01 << 1)?TRUE:FALSE):
                            (UnlockList[Index].Support & (0x01 << 4)?TRUE:FALSE);
                        break;
                    case EXTENDED_SESSION:
                        /*判断物理寻址和功能寻址的扩展会话下是否支持*/
                        SubfunctionValid = (DIAG_PHY == FrameType)?(UnlockList[Index].Support & (0x01 << 2)?TRUE:FALSE):
                            (UnlockList[Index].Support & (0x01 << 5)?TRUE:FALSE);
                        break;
                    default:
                        break;
                }

                if (TRUE == SubfunctionValid)
                {
                    break;/*查询到支持且有效的子功能*/
                }
                else
                {
                    continue;
                }
            }
        }

        if (SubfunctionValid)/*子功能支持且有效*/
        {
            if (Subfunction == UnlockList[Index].SeedID)/*请求种子*/
            {
                if (1 == Len)/*长度正确*/
                {
                    if (WAIT_DELAY == UnlockStep)/*钥匙错误超过两次，激活延时机制，未超过延时时间*/
                    {
                        NRC = RTDNE;
                    }
                    /*请求安全等级已经解锁*/
                    else if (UNLOCKED == UnlockStep && CurrentSecurityLevel == UnlockList[Index].Level)
                    {
                        PosRespBuffer.Data[0] = ServiceList[ServiceIndex].ServiceName + 0x40;
                        PosRespBuffer.Data[1] = Subfunction;
                        PosRespBuffer.Data[2] = 0x00;
                        PosRespBuffer.Data[3] = 0x00;
                        PosRespBuffer.Data[4] = 0x00;
                        PosRespBuffer.Data[5] = 0x00;

                        PosRespBuffer.Len = 6;
                    }
                    else
                    {
                        UDS_GeneratSeed(Seed);/*生成随机数种子*/
                        PosRespBuffer.Data[0] = ServiceList[ServiceIndex].ServiceName + 0x40;
                        PosRespBuffer.Data[1] = Subfunction;
                        PosRespBuffer.Data[2] = Seed[0];
                        PosRespBuffer.Data[3] = Seed[1];
                        PosRespBuffer.Data[4] = Seed[2];
                        PosRespBuffer.Data[5] = Seed[3];

                        PosRespBuffer.Len = 6;
                        UnlockStep = WAIT_REC_KEY;/*解锁步骤进入等待钥匙阶段*/
						UDS_GenerateKey(Seed, Key);/*根据种子采用内部算法生成钥匙*/
                    }
                }
                else/*长度不正确*/
                {
                    NRC = IMLOIF;
                }
            }
            else if (Subfunction == UnlockList[Index].KeyID)/*验证钥匙*/
            {
                if (5 == Len)/*长度正确*/
                {
                    if (WAIT_REC_KEY == UnlockStep)/*正在等待钥匙的验证*/
                    {
                        
                        AT24Cxx_Read(UNLOCKFAILEDCOUNTADDR, &UnlockFailedCounter, (u16)1);/*读取EEProm中验证钥匙失败次数*/
                        if (Data[1] == Key[0] && Data[2] == Key[1] && Data[3] == Key[2] && Data[4] == Key[3])/*验证成功*/
                        {
                            UnlockStep = UNLOCKED;
                            CurrentSecurityLevel = UnlockList[Index].Level;/*当前安全等级切换*/
                            UnlockFailedCounter = 0;/*验证成功后，验证失败次数清零*/
                            PosRespBuffer.Data[0] = ServiceList[ServiceIndex].ServiceName + 0x40;
                            PosRespBuffer.Data[1] = Subfunction;

                            PosRespBuffer.Len = 2;
                        }
                        else/*验证失败*/
                        {
                            UnlockFailedCounter++;

                            if (UnlockFailedCounter > 2)/*超过验证失败次数，激活延时机制*/
                            {
                                NRC = ENOA;
                                UnlockStep = WAIT_DELAY;/*解锁步骤进入延时机制*/
                                UDS_TimerRun(&UnlockFailedTimer, UnlockList[Index].FailedDelayTime, UDS_UnlockFailedTimerTimeout);/*启动定时器*/
                            }
                            else/*未超过验证失败次数*/
                            {
                                NRC = IK;
                                UnlockStep = WAIT_REQ_SEED;
                            }
                        }

                        AT24Cxx_Write(UNLOCKFAILEDCOUNTADDR, &UnlockFailedCounter, (u16)1);/*将验证失败次数写入EEProm*/
                    }
                    else if (WAIT_DELAY == UnlockStep)/*延时机制生效中，需等待延时完毕*/
                    {
                        NRC = RTDNE;
                    }
                    else/*当前解锁步骤为等待种子请求或已解锁，请求序列不正确*/
                    {
                        NRC = RSE;
                    }
                }
                else/*数据长度不正确*/
                {
                    NRC = IMLOIF;
                }
            }
            else/*既不是请求种子也不是验证钥匙，不存在*/
            {
                ;
            }
        }
        else/*子功能不支持或无效*/
        {
            NRC = !SubfunctionSupport?SFNS:(!SubfunctionValid?SFNSIAS:NRC);
        }
    }
    else
    {
        NRC = IMLOIF;/*长度不正确*/
    }

    SuppressResponse = (SuppressSupportBit && PR == NRC)?TRUE:((PR == NRC)?FALSE:TRUE);/*是否抑制正响应*/
}


/*******************************************************************************
* 函数名称: UDS_GeneratSeed
* 功能描述: 用于生成解锁安全等级的随机数种子
* 输入参数: Seed：保存种子的数组指针
* 输出参数: 无
* 返回参数: 无
* 备    注： 
*******************************************************************************/
void UDS_GeneratSeed(u8 *Seed)
{
    if (NULL != Seed)
    {
        /*u32 RandNum = RngDriver_GetRandomNum();

        Seed[0] = (RandNum >> 0) & 0xFF;
        Seed[1] = (RandNum >> 8) & 0xFF;
        Seed[2] = (RandNum >> 16) & 0xFF;
        Seed[3] = (RandNum >> 24) & 0xFF;*/
        RandNext = TIMx_GetCounter();
        
        Seed[0] = UDS_rand();
        Seed[1] = UDS_rand();
        Seed[2] = UDS_rand();
        Seed[3] = UDS_rand();
    }
    else
    {
        ;
    }
}


/*******************************************************************************
* 函数名称: UDS_rand
* 功能描述: 生成随机数
* 输入参数: 无
* 输出参数: 无
* 返回参数: 无
* 备    注： 
*******************************************************************************/
u8 UDS_rand(void)
{
    RandNext = (RandNext * 12345) + 12345; 
    return ((RandNext / 65535) % 255);
}


/*******************************************************************************
* 函数名称: UDS_GenerateKey
* 功能描述: 根据种子生成钥匙
* 输入参数: 无
* 输出参数: 无
* 返回参数: 无
* 备    注： 
*******************************************************************************/
void UDS_GenerateKey(const u8 *pSeed, u8 *pKey)
{	
	u8 i = 0;
	u32 Seed = 0;

	if(NULL != pSeed && NULL != pKey)
	{
    	Seed  = pSeed[0] * 0x1000000;
    	Seed += pSeed[1] * 0x10000;
    	Seed += pSeed[2] * 0x100;
    	Seed += pSeed[3];

    	for(i = 0; i < 35; ++i)
    	{
    		if(Seed & 0x80000000)
    		{
    			Seed = Seed<<1;
    			Seed = Seed ^ Personalkey;
    		}
    		else
    		{
    			Seed = Seed <<1;
    		}
    	}

    	pKey[0] = Seed /0x1000000;
    	pKey[1] = Seed /0x10000;	
    	pKey[2] = Seed /0x100;
    	pKey[3] = Seed ;
    }
    else
    {
        ;/*指针为空，错误*/
    }
}


/*******************************************************************************
* 函数名称: UDS_UnlockFailedTimerTimeout
* 功能描述: 解锁失败timer超时，修改解锁步骤，可以进行下一次27服务认证了。
* 输入参数: 无
* 输出参数: 无
* 返回参数: 无
* 备    注： 
*******************************************************************************/
void UDS_UnlockFailedTimerTimeout(void)
{
    UnlockStep = WAIT_REQ_SEED;
}


/*******************************************************************************
* 函数名称: UDS_Service0x28Handle
* 功能描述: 通讯控制（0x28）服务的处理函数
* 输入参数: Len：接收诊断数据长度 FrameType：接收诊断帧类型 Data：诊断数据
*           ServiceIndex：当前服务在配置表中的索引
* 输出参数: 无
* 返回参数: 无
* 备    注： 
*******************************************************************************/
void UDS_Service0x28Handle(u16 Len, u8 FrameType, u8 *Data, u8 ServiceIndex)
{

    u8 Subfunction;
    u8 ControlType;
    u8 SuppressSupportBit = 0;
    
    NRC = PR;

    if (NULL == Data)
    {
        return;
    }
    else
    {
        ;
    }

    if (2 == Len)
/*长度正确*/
    {
        if (TRUE == ServiceList[ServiceIndex].SuppressSupport)/*10服务支持抑制正响应bit*/
        {
            Subfunction = Data[0] & 0x7f;
            SuppressSupportBit = Data[0] & 0x80;
        }
        else
        {
            Subfunction = Data[0];
            SuppressSupportBit = FALSE;
        }

        ControlType = Data[1];

        switch (Subfunction)/*查询子功能是否支持*/
        {
            case ENABLERX_TX:
                NRC = (!(ServiceList[ServiceIndex].SubfunctionSupport & 0x01))?SFNS:NRC;
                break;
            case ENABLERX_DISABLETX:
                NRC = (!(ServiceList[ServiceIndex].SubfunctionSupport & 0x02))?SFNS:NRC;
                break;
            case DISABLERX_ENABLETX:
                NRC = (!(ServiceList[ServiceIndex].SubfunctionSupport & 0x04))?SFNS:NRC;
                break;
            case DISABLE_RXTX:
                NRC = (!(ServiceList[ServiceIndex].SubfunctionSupport & 0x08))?SFNS:NRC;
                break;
            default:
                NRC = SFNS;
                break;
        }

        switch (ControlType)/*查询控制报文类型是否支持*/
        {
            case NORMALMESSAGE:
                NRC = (!(CommunicationControl_MessageSupport & 0x01))?ROOR:NRC;
                break;
            case NETWORKMANAGEMENTMESSAGE:
                NRC = (!(CommunicationControl_MessageSupport & 0x02))?ROOR:NRC;
                break;
            case NORMAL_NETWORKMANAGEMENTMESSAGE:
                NRC = (!(CommunicationControl_MessageSupport & 0x03))?ROOR:NRC;
                break;
            default:
                NRC = ROOR;
                break;
        }
    }
    else/*长度不正确*/
    {
        NRC = IMLOIF;
    }

    SuppressResponse = (SuppressSupportBit && PR == NRC)?TRUE:((PR == NRC)?FALSE:TRUE);/*是否抑制正响应*/

    if (PR == NRC)/*填充响应数据*/
    {
        PosRespBuffer.Data[0] = ServiceList[ServiceIndex].ServiceName + 0x40;
        PosRespBuffer.Data[1] = Subfunction;

        PosRespBuffer.Len = 2;

        UDS_CommunicationControl(Subfunction, ControlType);/*设置接收发送报文使能状态*/
    }
    else
    {
        ;
    }
}


/*******************************************************************************
* 函数名称: UDS_CommunicationControl
* 功能描述: 会话控制（0x28）服务的处理函数
* 输入参数: Subfunction：28服务的子功能 ControlType：28服务的控制类型
* 输出参数: 无
* 返回参数: 无
* 备    注： 
*******************************************************************************/
void UDS_CommunicationControl(u8 Subfunction, u8 ControlType)
{
    /*普通应用报文接收使能或禁止*/
    NormalMessageRecEnable = ((ENABLERX_TX == Subfunction || ENABLERX_DISABLETX == Subfunction) && NORMALMESSAGE == ControlType)?TRUE:(((DISABLE_RXTX == Subfunction || DISABLERX_ENABLETX == Subfunction) && NORMALMESSAGE == ControlType)?FALSE:NormalMessageRecEnable);
    /*普通应用报文发送使能或禁止*/
    NormalMessageSendEnable = ((ENABLERX_TX == Subfunction || DISABLERX_ENABLETX == Subfunction) && NORMALMESSAGE == ControlType)?TRUE:(((DISABLE_RXTX == Subfunction || ENABLERX_DISABLETX == Subfunction) && NORMALMESSAGE == ControlType)?FALSE:NormalMessageRecEnable);

    AT24Cxx_Write(COMCONTROL_NORMALMESSAGEADDR, &Subfunction, (u16)1);/*存储通信控制状态，断电仍保持*/
}


/*******************************************************************************
* 函数名称: UDS_Service0x3EHandle
* 功能描述: 诊断工具在线（0x3E）服务的处理函数
* 输入参数: Len：接收诊断数据长度 FrameType：接收诊断帧类型 Data：诊断数据
*           ServiceIndex：当前服务在配置表中的索引
* 输出参数: 无
* 返回参数: 无
* 备    注： 
*******************************************************************************/
void UDS_Service0x3EHandle(u16 Len, u8 FrameType, u8 *Data, u8 ServiceIndex)
{

    u8 Subfunction;
    u8 SuppressSupportBit = 0;
    
    NRC = PR;

    if (NULL == Data)
    {
        return;
    }
    else
    {
        ;
    }

    if (1 == Len)
/*长度正确*/
    {
        if (TRUE == ServiceList[ServiceIndex].SuppressSupport)/*10服务支持抑制正响应bit*/
        {
            Subfunction = Data[0] & 0x7f;
            SuppressSupportBit = Data[0] & 0x80;
        }
        else
        {
            Subfunction = Data[0];
            SuppressSupportBit = FALSE;
        }

        NRC = !(ServiceList[ServiceIndex].SubfunctionSupport & 0x01)?SFNS:NRC;/*子功能不支持*/
    }
    else
    {
        NRC = IMLOIF;/*长度不正确*/
    }

    SuppressResponse = (SuppressSupportBit && PR == NRC)?TRUE:((PR == NRC)?FALSE:TRUE);/*是否抑制正响应*/

    if (PR == NRC)
    {
        PosRespBuffer.Data[0] = ServiceList[ServiceIndex].ServiceName + 0x40;
        PosRespBuffer.Data[1] = Subfunction;

        PosRespBuffer.Len = 2;/*正响应*/
        UDS_TimerRun(&SessionTimer, SESSIONTIMEOUT, UDS_SessionTimeout);/*重置timer*/
    }
    else
    {
        ;
    }
}


/*******************************************************************************
* 函数名称: UDS_Service0x22Handle
* 功能描述: 通过DID读数据（0x22）服务的处理函数
* 输入参数: Len：接收诊断数据长度 FrameType：接收诊断帧类型 Data：诊断数据
*           ServiceIndex：当前服务在配置表中的索引
* 输出参数: 无
* 返回参数: 无
* 备    注： 
*******************************************************************************/
void UDS_Service0x22Handle(u16 Len, u8 FrameType, u8 *Data, u8 ServiceIndex)
{
    u8 Index = 0;
    u16 DID = (Data[0] << 8) + Data[1];/*取出DID*/
    u8 SuppressSupportBit = FALSE;/*22服务不支持正响应抑制*/
    bool DIDExist = FALSE;
    
    NRC = PR;

    if (NULL == Data)
    {
        return;
    }
    else
    {
        ;
    }

    if (2 == Len)
/*长度正确*/
    {
        for (Index = 0; Index < DID_COUNT; ++Index)
        {
            if (DIDList[Index].ID == DID)
            {
                DIDExist = TRUE;/*DIDList中查找是否存在请求的DID*/
                break;
            }
            else
            {
                continue;
            }
        }

        if (TRUE == DIDExist)/*DID存在*/
        {
            if (READONLY == DIDList[Index].RWAttribute || READWRITE == DIDList[Index].RWAttribute)/*DID属性为可读*/
            {
                PosRespBuffer.Data[0] = ServiceList[ServiceIndex].ServiceName + 0x40;
                PosRespBuffer.Data[1] = Data[0];
                PosRespBuffer.Data[2] = Data[1];
                /* + 4 - 1是为了向上取整*/
                AT24Cxx_Read(DIDList[Index].EEPromAddr, &(PosRespBuffer.Data[3]), (u16)(DIDList[Index].Size));/*从内存中读取DID的值到正响应buffer*/

                PosRespBuffer.Len = 3 + DIDList[Index].Size;
            }
            else
            {
                NRC = ROOR;/*属性为不可读*/
            }
        }
        else
        {
            NRC = ROOR;/*DID不存在*/
        }
    }
    else
    {
        NRC = IMLOIF;/*长度不正确*/
    }

    SuppressResponse = (SuppressSupportBit && PR == NRC)?TRUE:((PR == NRC)?FALSE:TRUE);/*是否抑制正响应*/
}


/*******************************************************************************
* 函数名称: UDS_Service0x2EHandle
* 功能描述: 通过DID写数据（0x2E）服务的处理函数
* 输入参数: Len：接收诊断数据长度 FrameType：接收诊断帧类型 Data：诊断数据
*           ServiceIndex：当前服务在配置表中的索引
* 输出参数: 无
* 返回参数: 无
* 备    注： 
*******************************************************************************/
void UDS_Service0x2EHandle(u16 Len, u8 FrameType, u8 *Data, u8 ServiceIndex)
{
    u8 Index = 0;
    u16 DID = (Data[0] << 8) + Data[1];/*取出DID*/
    u8 SuppressSupportBit = FALSE;/*2E服务不支持正响应抑制*/
    bool DIDExist = FALSE;
    u8 DIDWritable = 1;
    bool WriteResult = FALSE;
    
    NRC = PR;

    if (NULL == Data)
    {
        return;
    }
    else
    {
        ;
    }

    if (Len >= 2)
/*长度正确*/
    {
        for (Index = 0; Index < DID_COUNT; ++Index)
        {
            if (DIDList[Index].ID == DID)
            {
                DIDExist = TRUE;/*DIDList中查找是否存在请求的DID*/
                break;
            }
            else
            {
                continue;
            }
        }

        if (TRUE == DIDExist)/*DID存在*/
        {
            if (DIDList[Index].SecurityLevel == CurrentSecurityLevel || ZERO_LEVEL == DIDList[Index].SecurityLevel)/*满足写DID要求的安全等级*/
            {
                if (WRITEONLY == DIDList[Index].RWAttribute || READWRITE == DIDList[Index].RWAttribute)/*该DID可写*/
                {
                    if (DIDList[Index].Size + 2 == Len)/*长度正确*/
                    {
                        AT24Cxx_Read(DIDList[Index].WritableAddr, &DIDWritable, (u16)1);/*从EEProm中读取flag，查询该DID是否可写（有的DID只能写一次）*/
                        if (0 != DIDWritable)
/*该DID仍可写*/
                        {
                            /* + 4 - 1是为了向上取整*/
                            /*WriteResult = */AT24Cxx_Write(DIDList[Index].EEPromAddr, &Data[2], (u16)(DIDList[Index].Size));

                            if (FALSE == WriteResult)/*写入成功*/
                            {
                                PosRespBuffer.Data[0] = ServiceList[ServiceIndex].ServiceName + 0x40;
                                PosRespBuffer.Data[1] = Data[0];
                                PosRespBuffer.Data[2] = Data[1];

                                PosRespBuffer.Len = 3;
                            }
                            else/*写入失败*/
                            {
                                NRC = GR;
                            }
                            
                            if (0xF18C == DID || 0xF190 == DID)/*只能写一次的DID*/
                            {
                                DIDWritable = 0;
                                AT24Cxx_Write(DIDList[Index].WritableAddr, &DIDWritable, (u16)1);/*将本DID的可写修改为不可写*/
                            }
                            else
                            {
                                ;
                            }
                        }
                        else/*超过可写次数*/
                        {
                            NRC = ENOA;
                        }
                    }
                    else/*长度不正确*/
                    {
                        NRC = IMLOIF;
                    }
                }
                else/*该DID属性为不可写*/
                {
                    NRC = ROOR;
                }
            }
            else/*安全等级不满足*/
            {
                NRC = SAD;
            }
        }
        else/*DID不存在*/
        {
            NRC = ROOR;
        }
    }
    else/*长度不正确*/
    {
        NRC = IMLOIF;
    }

    SuppressResponse = (SuppressSupportBit && PR == NRC)?TRUE:((PR == NRC)?FALSE:TRUE);/*是否抑制正响应*/
}


/*******************************************************************************
* 函数名称: UDS_Service0x31Handle
* 功能描述: 例程控制（0x31）服务的处理函数
* 输入参数: Len：接收诊断数据长度 FrameType：接收诊断帧类型 Data：诊断数据
*           ServiceIndex：当前服务在配置表中的索引
* 输出参数: 无
* 返回参数: 无
* 备    注： 
*******************************************************************************/
void UDS_Service0x31Handle(u16 Len, u8 FrameType, u8 *Data, u8 ServiceIndex)
{
    u8 Subfunction = 0;
    u16 RoutineID;
    u8 SuppressSupportBit = 0;
    u8 Index = 0;
    bool RoutineIDExist = FALSE;
	BaseType_t xReturn = pdPASS;

    NRC = PR;

    if (NULL == Data)
    {
        return;
    }
    else
    {
        ;
    }

    if (Len >= 3)
/*长度正确*/
    {
        RoutineID = (Data[1] << 8) + Data[2];/*获取例程ID*/
    
        if (TRUE == ServiceList[ServiceIndex].SuppressSupport)/*10服务支持抑制正响应bit*/
        {
            Subfunction = Data[0] & 0x7f;
            SuppressSupportBit = Data[0] & 0x80;
        }
        else
        {
            Subfunction = Data[0];
            SuppressSupportBit = FALSE;
        }

        for (Index = 0; Index < ROUTINE_COUNT; ++Index)/*查询每个例程是否支持以及需要的参数长度是否满足要求*/
        {
            if (RoutineList[Index].ID == RoutineID)
            {
                RoutineIDExist = TRUE;
                break;
            }
            else
            {
                continue;
            }
        }

        switch (CurrentSession)
        {
            case DEFAULT_SESSION:
                /*判断当前会话和安全等级下是否支持*/
                NRC = (RoutineList[Index].DefaultSessionSecurity == UNSUPPORT_LEVEL)?SNSIAS:
                        ((RoutineList[Index].DefaultSessionSecurity != CurrentSecurityLevel 
                        && ZERO_LEVEL != RoutineList[Index].DefaultSessionSecurity)?SAD:NRC);
                break;
            case PROGRAM_SESSION:
                 NRC = (RoutineList[Index].ProgramSessionSecurity == UNSUPPORT_LEVEL)?SNSIAS:
                        ((RoutineList[Index].ProgramSessionSecurity != CurrentSecurityLevel 
                        && ZERO_LEVEL != RoutineList[Index].ProgramSessionSecurity)?SAD:NRC);
                break;
            case EXTENDED_SESSION:
                 NRC = (RoutineList[Index].ExtendedSessionSecuruty == UNSUPPORT_LEVEL)?SNSIAS:
                        ((RoutineList[Index].ExtendedSessionSecuruty != CurrentSecurityLevel 
                        && ZERO_LEVEL != RoutineList[Index].ExtendedSessionSecuruty)?SAD:NRC);
                break;
            default:
                break;
        }
        
        switch (Subfunction)/*查询子功能是否支持*/
        {
            case STARTROUTINE:
                NRC = (!(RoutineList[Index].RoutineControlTypeSupport & 0x01))?SFNS:NRC;
                break;
            case STOPROUTINE:
                NRC = (!(RoutineList[Index].RoutineControlTypeSupport & 0x02))?SFNS:NRC;
                break;
            case REQUESTROUTINERESULTS:
                NRC = (!(RoutineList[Index].RoutineControlTypeSupport & 0x04))?SFNS:NRC;
                break;
            default:
                NRC = SFNS;
                break;
        }

        NRC = (FALSE == RoutineIDExist)?ROOR:NRC;/*例程不存在？*/

        if (PR == NRC && TRUE == RoutineIDExist)
        {
            switch (Subfunction)
            {
                case STARTROUTINE:/*启动一个例程*/
                    if (RoutineList[Index].ParameterLen + 3 == Len)
                    {
                        /*填充正响应*/
                        PosRespBuffer.Data[0] = ServiceList[ServiceIndex].ServiceName + 0x40;
                        PosRespBuffer.Data[1] = Subfunction;
                        PosRespBuffer.Data[2] = Data[1];
                        PosRespBuffer.Data[3] = Data[2];
                        
                        PosRespBuffer.Len = 4;
                    }
                    else
                    {
                        NRC = IMLOIF;
                    }

                    RoutineList[Index].Result = 0xFF;
                    RoutineList[Index].Execute_Finished = FALSE;

                    if (NOTRUN == Sta_CheckApplicationDataValid && NOTRUN == Sta_CheckProgrammingPreConditions && 
                        NOTRUN == Sta_CopyAPPCode && NOTRUN == Sta_EraseMemory && NOTRUN == Sta_APPRollBack)
                    {
                        taskENTER_CRITICAL();          /*进入临界区*/
                        switch (Index)/*创建任务运行例程，防止程序阻塞*/
                        {
                            case CheckProgrammingPreconditions:
                                Sta_CheckProgrammingPreConditions = RUNNING;/*修改Task状态为运行中*/
                                
                                break;
                            case EraseMemory:
								EraseMemoryAddress = (Data[3] << 24) | (Data[4] << 16) | (Data[5] << 8) | Data[6];
								EraseMemoryLength = (Data[7] << 24) | (Data[8] << 16) | (Data[9] << 8) | Data[10];
								
                                /*下载地址在可用物理内存范围内，且满足最大程序大小限制*/
                                if (EraseMemoryAddress >= MEMORY_SATRTADDRESS && (EraseMemoryAddress + EraseMemoryLength) <= MEMORY_ENDADDRESS)
                                {
                                
                                    Sta_EraseMemory = RUNNING;
                                }
                                else
                                {
                                    NRC = ROOR;
                                }
                                
                                break;
                            case CheckValidateApplication:
                                CRCDriver_Reset();
                                ReceiveCrc = (Data[3] << 24) | (Data[4] << 16) | (Data[5] << 8) | (Data[6] << 0);
                                /*for (i = 0; i < 6; ++i)
                                {
                                    SoftwareNumber[i] = Data[i + 7];
                                }*/
                                Sta_CheckApplicationDataValid = RUNNING;
                                    
                                break;
                            case APPRollBack:
                                Sta_APPRollBack = RUNNING;
                            
                                break;
                            default:
                                break;
                        }

                        xReturn = xTaskCreate((TaskFunction_t)UDS_TimeOperation,        //任务函数
                    						  (const char*	)"UDSTask",                 //任务名称
                    						  (uint16_t		)0x200,                     //任务栈大小
                    						  (void*		)NULL,                      //任务函数的参数
                    						  (UBaseType_t	)2,                         //任务优先级
                    						  (TaskHandle_t*)&Task_UDSTimeOperation);   //任务控制块
						  
    					if(xReturn == pdPASS)
    					{
    						Log(LOG_DEBUG, "0x31 Task Create Success\n");
    					}
    					else
    					{
    						Log(LOG_DEBUG, "0x31 Task Create Failed\n");
    					}
                        
    					taskEXIT_CRITICAL();         //退出临界区 
                    }
                    else
                    {
                        NRC = BRR;
                    }
					
                    break;
                case STOPROUTINE:/*暂停一个例程*/
                    break;
                case REQUESTROUTINERESULTS:/*查询例程结果*/
                    
                    if (RoutineList[Index].Execute_Finished)
                    {
                        /*填充正响应*/
                        PosRespBuffer.Data[0] = ServiceList[ServiceIndex].ServiceName + 0x40;
                        PosRespBuffer.Data[1] = Subfunction;
                        PosRespBuffer.Data[2] = Data[1];
                        PosRespBuffer.Data[3] = Data[2];
                        PosRespBuffer.Data[4] = RoutineList[Index].Result;

                        PosRespBuffer.Len = 5;
                    }
                    else
                    {
                        NRC = BRR;
                    }
                    break;
                default:
                    break;
            }
        }
        else
        {
            ;
        }
    }
    else/*长度不正确*/
    {
        NRC = IMLOIF;
    }

    SuppressResponse = (SuppressSupportBit && PR == NRC)?TRUE:((PR == NRC)?FALSE:TRUE);/*是否抑制正响应*/
}


/*******************************************************************************
* 函数名称: UDS_CheckProgrammingPreConditions
* 功能描述: 检查刷新前提条件例程
* 输入参数: 无
* 输出参数: 无
* 返回参数: 无
* 备    注： 
*******************************************************************************/
void UDS_CheckProgrammingPreConditions(void)
{
    RoutineList[0].Result = 0;
    RoutineList[0].Execute_Finished = TRUE;
    Sta_CheckProgrammingPreConditions = FINISHED;
}


/*******************************************************************************
* 函数名称: UDS_EraseMemory
* 功能描述: 擦除flash例程
* 输入参数: 无
* 输出参数: 无
* 返回参数: 无
* 备    注： 
*******************************************************************************/
void UDS_EraseMemory(void)
{
    u8 AppAddress[8] = {0};
	u8 JumpAPPFailedCount = 0;
  
    UDS_BackupAPPCode(); 
	
	AT24Cxx_Read(JUMPTOAPPADDRESS, AppAddress, 4);
    AT24Cxx_Write(BACKUPJUMPTOAPPADDRESS, AppAddress, 4);/*存入备份APP的跳转地址*/

    JumpAPPFailedCount = 0;
    AT24Cxx_Write(JUMPTOAPPFAILEDCOUNT, &JumpAPPFailedCount, (u16)1);/*设置APP程序启动失败次数为0，待刷新完毕后，每次重启失败+1*/
    
    Sta_EraseMemory = FINISHED;/*修改Task状态为运行完毕*/
    RoutineList[1].Result = Flash_Erase(EraseMemoryAddress, EraseMemoryLength);
    RoutineList[1].Execute_Finished = TRUE;
}


/*******************************************************************************
* 函数名称: UDS_ByteConvert
* 功能描述: 转换u8数组顺序
* 输入参数: 无
* 输出参数: 无
* 返回参数: 无
* 备    注： 在计算crc时需强转为u32，为达到CRC计算结果一样，需转换u32的大小端格式
*******************************************************************************/
void UDS_ByteConvert(u8 *Data, u16 Len)
{
    u16 i = 0;
    u32 *DataU32 = (u32 *)Data;

    if (Data != NULL && Len % 4 == 0)/*传入大小必须为4的整数倍，程序本身大小确实为4的整数倍*/
    {
        for (i = 0; i < Len / 4; ++i)
        {
            DataU32[i] = BIGLITTLESWAP32(DataU32[i]);
        }
    }
    else
    {
        ;
    }
}


/*******************************************************************************
* 函数名称: UDS_CheckApplicationDataValid
* 功能描述: 检查36服务后的程序完整有效性例程
* 输入参数: 无
* 输出参数: 无
* 返回参数: 无
* 备    注： 
*******************************************************************************/
void UDS_CheckApplicationDataValid(void)
{
    u32 ProcessedSize = 0;/*已处理程序大小*/
    u8 Data[400] = {0};/*存储从flash读取出来的程序数据*/
    u16 Size = 0;/*当前从flash读取大小*/
    u32 LocalCRC = CRC_INITVALUE;/*CRC初始值*/

    while (ProcessedSize < ReprogramMemoryLength)/*CRC还未计算完毕*/
    {
         if (ProcessedSize + 400 > ReprogramMemoryLength)/*剩余未处理数据不足400字节，只读取剩余字节数*/
         {
             Size = ReprogramMemoryLength - ProcessedSize;
         }
         else
         {
             Size = 400;/*剩余未处理数据大于400字节，每次读取400字节*/
         }

         Flash_Read(ReprogramMemoryAddress + ProcessedSize, (u32 *)Data, (u32)Size / 4);/*读取成功*/
         UDS_ByteConvert(Data, Size);
         LocalCRC = CRC32_Cal_Buffer((u32 *)Data, (u32)Size / 4);/*调用硬件CRC计算接口*/
		 
		 
		 ProcessedSize += Size;
    }

    LocalCRC ^= ReceiveCrc;

    RoutineList[2].Result = LocalCRC;
    RoutineList[2].Execute_Finished = TRUE;
    Sta_CheckApplicationDataValid = FINISHED;/*修改Task状态为运行完毕*/
}


/*******************************************************************************
* 函数名称: UDS_APPRollBack
* 功能描述: APP代码一键回滚到上个版本，从备份区将代码拷贝到APP代码区
* 输入参数: 无
* 输出参数: 无
* 返回参数: 无
* 备    注： 
*******************************************************************************/
void UDS_APPRollBack(void)
{
    u8 JumpAPPFailedCount = 0;
    u8 Result = 0;
    u8 AppAddress8[4] = {0};

    AT24Cxx_Read(BACKUPJUMPTOAPPADDRESS, AppAddress8, 4);
    AT24Cxx_Write(JUMPTOAPPADDRESS, AppAddress8, 4);
    
    Result = Flash_Write(MEMORY_BACKUPSTARTADDRESS, (u32 *)MEMORY_BACKUPSTARTADDRESS, (u32)((256 * 1024) / 4));
    
    JumpAPPFailedCount = 0;
    AT24Cxx_Write(JUMPTOAPPFAILEDCOUNT, &JumpAPPFailedCount, (u16)1);
    
    SystemReset();

    RoutineList[3].Result = Result;
    RoutineList[3].Execute_Finished = TRUE;
    Sta_CheckApplicationDataValid = FINISHED;/*修改Task状态为运行完毕*/
}


/*******************************************************************************
* 函数名称: UDS_Service0x34Handle
* 功能描述: 请求下载（0x34）服务的处理函数
* 输入参数: Len：接收诊断数据长度 FrameType：接收诊断帧类型 Data：诊断数据
*           ServiceIndex：当前服务在配置表中的索引
* 输出参数: 无
* 返回参数: 无
* 备    注： 
*******************************************************************************/
void UDS_Service0x34Handle(u16 Len, u8 FrameType, u8 *Data, u8 ServiceIndex)
{
    u8 SuppressSupportBit = FALSE;
    u8 AppAddress[4] = {0};

    NRC = PR;

    if (NULL == Data)
    {
        return;
    }
    else
    {
        ;
    }
    
    if (Len >= 4)/*大于要求最小字节数*/
    {
        /*无压缩方法、无加密方法、程序大小占位4字节、程序下载地址占位4字节*/
        if ((No_Comress == (Data[0] >> 4) || No_Encrypting == (Data[0] & 0x0f)) && 
            (TRANSFER_MEMORYSIZEBYTE == (Data[1] >> 4) && TRANSFER_ADDRESSBYTE == (Data[1] & 0x0f)))
        {
            if (10 == Len)/*长度正确*/
            {
                ReprogramMemoryAddress = (Data[2] << 24) + (Data[3] << 16) + (Data[4] << 8) + Data[5];
                ReprogramMemoryLength = (Data[6] << 24) + (Data[7] << 16) + (Data[8] << 8) + Data[9];

                /*下载地址在可用物理内存范围内，且满足最大程序大小限制*/
                if (ReprogramMemoryAddress >= MEMORY_SATRTADDRESS && (ReprogramMemoryAddress + ReprogramMemoryLength) <= MEMORY_ENDADDRESS)
                {
                    AppAddress[0] = (ReprogramMemoryAddress >> 24) & 0xFF;
                    AppAddress[1] = (ReprogramMemoryAddress >> 16) & 0xFF;
                    AppAddress[2] = (ReprogramMemoryAddress >> 8) & 0xFF;
                    AppAddress[3] = (ReprogramMemoryAddress >> 0) & 0xFF;
                    AT24Cxx_Write(JUMPTOAPPADDRESS, AppAddress, 4);/*存入APP跳转地址*/

                    BlockIndex = 0;
                    Downloading = TRUE;/*开始*/

                    /*填充正响应*/
                    PosRespBuffer.Data[0] = ServiceList[ServiceIndex].ServiceName + 0x40;
                    PosRespBuffer.Data[1] = 0x20;
                    PosRespBuffer.Data[2] = (DOWNLOAD_MAXBUFSIZE >> 8) & 0xff;
                    PosRespBuffer.Data[3] = DOWNLOAD_MAXBUFSIZE & 0xff;

                    PosRespBuffer.Len = 4;

                    #if 0
                    if (NOTRUN == Sta_CheckApplicationDataValid && NOTRUN == Sta_CheckProgrammingPreConditions && 
                        NOTRUN == Sta_CopyAPPCode && NOTRUN == Sta_EraseMemory && NOTRUN == Sta_APPRollBack)
                    {
                        taskENTER_CRITICAL();          /*进入临界区*/
                        
                        Sta_CopyAPPCode = RUNNING;
                        xReturn = xTaskCreate((TaskFunction_t)UDS_TimeOperation,        //任务函数
                    						  (const char*	)"UDSTask",                 //任务名称
                    						  (uint16_t		)0x200,                     //任务栈大小
                    						  (void*		)NULL,                      //任务函数的参数
                    						  (UBaseType_t	)2,                         //任务优先级
                    						  (TaskHandle_t*)&Task_UDSTimeOperation);   //任务控制块
						  
    					if(xReturn == pdPASS)
    					{
    						Log(LOG_DEBUG, "0x34 Task Create Success\n");
    					}
    					else
    					{
    						Log(LOG_DEBUG, "0x34 Task Create Failed\n");
    					}
                        
    					taskEXIT_CRITICAL();         //退出临界区 
                    }
                    else
                    {
                        ReprogramMemoryAddress = 0;
                        ReprogramMemoryLength = 0;
                        
                        NRC = BRR;
                    }
                    #endif
                }
                else/*下载地址不正确或程序长度有问题*/
                {
                    NRC = UDNA;
                }
            }/*长度不正确*/
            else
            {
                NRC = IMLOIF;
            }
        }/*压缩加密或程序长度地址不正确*/
        else
        {
            NRC = ROOR;
        }
    }
    else/*长度小于最小要求长度*/
    {
        NRC = IMLOIF;
    }

    SuppressResponse = (SuppressSupportBit && PR == NRC)?TRUE:((PR == NRC)?FALSE:TRUE);/*是否抑制正响应*/
}


/*******************************************************************************
* 函数名称: UDS_BackupAPPCode
* 功能描述: 备份APP到备份区，防止刷新失败
* 输入参数: 无
* 输出参数: 无
* 返回参数: 无
* 备    注： 
*******************************************************************************/
void UDS_BackupAPPCode(void)
{
    Flash_Write(MEMORY_BACKUPSTARTADDRESS, (u32 *)MEMORY_SATRTADDRESS, (u32)((256 * 1024) / 4));
    /*Sta_CopyAPPCode = FINISHED;*/
}


/*******************************************************************************
* 函数名称: UDS_Service0x36Handle
* 功能描述: 数据传输（0x36）服务的处理函数
* 输入参数: Len：接收诊断数据长度 FrameType：接收诊断帧类型 Data：诊断数据
*           ServiceIndex：当前服务在配置表中的索引
* 输出参数: 无
* 返回参数: 无
* 备    注： 
*******************************************************************************/
void UDS_Service0x36Handle(u16 Len, u8 FrameType, u8 *Data, u8 ServiceIndex)
{
    u8 SuppressSupportBit = FALSE;
    
    NRC = PR;

    if (NULL == Data)
    {
        return;
    }
    else
    {
        ;
    }
    
    if (Len >= 2 && (Len - 2) <= DOWNLOAD_MAXBUFSIZE && 0 == (Len - 2) % 4)/*长度大于最小要求长度，且小于34服务发送的最大长度*/
    {
        if (TRUE == Downloading)/*34服务已成功请求*/
        {
            if (Data[0] == (u8)(BlockIndex + 1))/*BlockIndex正确*/
            {
                BlockIndex = Data[0];/*更新BlockIndex*/
                
                if ((ProgramReceivedLength + Len - 2) <= ReprogramMemoryLength)/*已下载长度未超过34服务指定下载长度*/
                {
                    /*接收数据的检验和校验成功，且写入数据成功*/
                    if (!UDS_CheckSum(&Data[1], Len - 1) && !Flash_Write(ReprogramMemoryAddress + ProgramReceivedLength, (u32 *)&Data[1], (u32)(Len - 2) / 4))
                    {
                        /*填充正响应*/
                        PosRespBuffer.Data[0] = ServiceList[ServiceIndex].ServiceName + 0x40;
                        PosRespBuffer.Data[1] = BlockIndex;

                        PosRespBuffer.Len = 2;

                        ProgramReceivedLength += Len - 2;
                    }
                    else/*校验和校验失败或写入数据失败*/
                    {
                        BlockIndex--;/*BlockIndex回退，等待重试*/
                        NRC = GPF;
                    }
                }
                else/*超出指定下载长度*/
                {
                    BlockIndex--;
                    NRC = TDS;
                }
            }
            else if (Data[0] == BlockIndex)/*接收到重复36服务，直接给正响应*/
            {
                PosRespBuffer.Data[0] = ServiceList[ServiceIndex].ServiceName + 0x40;
                PosRespBuffer.Data[1] = BlockIndex;

                PosRespBuffer.Len = 2;
            }
            else/*BlockIndex不正确*/
            {
                NRC = WBSC;
            }
        }
        else/*34服务没有正确请求*/
        {
            NRC = RSE;
        }
    }
    else/*长度不满足要求*/
    {
        NRC = IMLOIF;
    }

    SuppressResponse = (SuppressSupportBit && PR == NRC)?TRUE:((PR == NRC)?FALSE:TRUE);/*是否抑制正响应*/
}


/*******************************************************************************
* 函数名称: UDS_Service0x37Handle
* 功能描述: 请求传输退出（0x37）服务的处理函数
* 输入参数: Len：接收诊断数据长度 FrameType：接收诊断帧类型 Data：诊断数据
*           ServiceIndex：当前服务在配置表中的索引
* 输出参数: 无
* 返回参数: 无
* 备    注： 
*******************************************************************************/
void UDS_Service0x37Handle(u16 Len, u8 FrameType, u8 *Data, u8 ServiceIndex)
{
    u8 SuppressSupportBit = FALSE;

    NRC = PR;

    if (NULL == Data)
    {
        return;
    }
    else
    {
        ;
    }

    if (0 == Len)/*SID后无数据*/
    {
        if (TRUE == Downloading)/*34服务已正确请求*/
        {
            Downloading = FALSE;
            ProgramReceivedLength = 0;
            BlockIndex = 0;

            PosRespBuffer.Data[0] = ServiceList[ServiceIndex].ServiceName + 0x40;

            PosRespBuffer.Len = 1;
        }
        else
        {
            NRC =RSE;
        }
    }
    else/*长度不正确*/
    {
        NRC = IMLOIF;
    }

    SuppressResponse = (SuppressSupportBit && PR == NRC)?TRUE:((PR == NRC)?FALSE:TRUE);/*是否抑制正响应*/
}


/*******************************************************************************
* 函数名称: UDS_CheckSum
* 功能描述: 检查数据的校验和
* 输入参数: Data：需要校验的数据指针 Len：需要校验数据的长度
* 输出参数: 无
* 返回参数: 0：校验和为0，数据接收正确 非0：数据接收有误 FF：参数错误
* 备    注： 由36服务每次接受数据时调用，进行校验。
*******************************************************************************/
u8 UDS_CheckSum(u8 *Data, u8 Len)
{ 
    u8 i = 0;
    u8 ret = 0;

    if (NULL != Data)
    {
        for(i = 0; i < Len; ++i)
        {
            ret ^= *(Data++);
        }
        
        return ret;
    }
    else
    {
        return 0xFF;
    }
}
