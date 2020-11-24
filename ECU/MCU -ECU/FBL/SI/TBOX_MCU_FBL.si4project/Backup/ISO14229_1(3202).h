
#ifndef _ISO14229_1_H_
#define _ISO14229_1_H_

#include "BaseType.h"

#define MAX_UDS_RES_Buff_Size           (100U)

#define DIAG_SERVICE_NUMBER         (11U)/*诊断服务个数*/
#define P2SERVERMAX                 (0x32U)/*ECU在收到请求和给出响应之间的时间间隔*/
#define P2PSERVERMAX                (0x1F4U)/*ECU给出NRC 78(等待)之后生效*/
#define SERVICE0x10_01TO02SUPPORT   (FALSE)/*不允许从默认会话直接切换到编程会话*/
#define SERVICE0x10_02TO03SUPPORT   (FALSE)/*不允许从编程会话直接切换到扩展会话*/
#define SECURITYLEVEL_COUNT         (3U)/*安全等级数量，用于声明UnlockList结构体*/
#define DID_COUNT                   (7U)/*DID数量*/
#define ROUTINE_COUNT               (4U)/*例程数量*/
#define RESPONSE_TIMEOUT            (50U)/*响应超时，50ms未响应发送0x78NRC*/
#define TRANSFER_MEMORYSIZEBYTE     (4U)/*34服务，接收的程序大小字节数*/
#define TRANSFER_ADDRESSBYTE        (4U)/*34服务，接收的物理下载地址字节数*/
#define MEMORY_SATRTADDRESS         (0x08020000U)/*APP可用物理内存起始地址*/
#define MEMORY_ENDADDRESS           (0x08060000U)/*APP可用物理内存结束地址*/
#define MEMORY_BACKUPSTARTADDRESS   (0x080C0000U)/*APP备份区域起始地址*/
#define MEMORY_BACKUPENDADDRESS     (0x080FFFFFU)/*APP备份区域结束地址*/
#define UNLOCKFAILEDCOUNTADDR		(0x0048U)/*0x27服务解锁失败次数在flash中的存储地址*/
#define COMCONTROL_NORMALMESSAGEADDR (0x0049U)/*0x28服务普通报文控制情况在flash中的存储地址，取值按照子功能取值*/
#define CURRENT_SESSION             (0x0000U)/*当前会话存储地址，在应用程序请求10 02后写入，每次上电BootLoader中读取该值，确认是否进入APP*/
#define JUMPTOAPPFAILEDCOUNT        (0x004AU)/*存储跳转到APP失败次数，若超过3次则写入备份区的APP程序*/
#define JUMPTOAPPADDRESS            (0x004B)/*存储APP跳转的地址*/
#define BACKUPJUMPTOAPPADDRESS      (0x004F)/*存储备份APP跳转的地址*/
#define APP_MAXSIZE                 (1024 * 256U)/*下载程序最大字节数*/
#define DOWNLOAD_MAXBUFSIZE         (1024)/* 一次36最多传输字节数（受限于15765和14229两个的buffer大小）*/
#define CRC_INITVALUE               (0xFFFFFFFFU)
#define CRC_MULTINOMIAL             (0x04C11DB7U)
#define CRC_MULTIRESERVED           (0xEDB88320U)
#define NRC0x78SEND_MAXCOUNT        (30U)/*最多发送0x78NRC次数*/
#define NRC0x78Interval_TIME        (1500U)/*0x78NRC连续发送时间间隔*/
#define SESSIONTIMEOUT              (5200U)/*会话超时时间*/

typedef enum
{
    SESSION_CONTROL = 0x10,
	RESET_ECU = 0x11,
	SECURITY_ACCESS = 0x27,
	COMMUNICATION_CONTROL = 0x28,
	TESTERPRESENT = 0x3E,
	READ_DATA_BY_ID = 0x22,
	WRITE_DATA_BY_ID = 0x2E,
	ROUTINE_CONTROL = 0x31,
	REQUEST_DOWNLOAD = 0x34,
	TRANSMIT_DATA = 0x36,
	REQUEST_TRANSFER_EXIT = 0x37
}Service_enum;/*诊断服务*/

typedef enum{
    PR      = 0x00,/*postive response*/
	GR      = 0x10,/*general reject*/
	SNS     = 0x11,/*service not supported*/
	SFNS    = 0x12,/*sub-function not supported*/
	IMLOIF  = 0x13,/*incorrect message length or invalid format*/
	RTL     = 0x14,/*response too long*/
	BRR     = 0x21,/*busy repeat request*/
	CNC     = 0x22,/*condifitons not correct*/
	RSE     = 0x24,/*request sequence error*/
	FPEORA  = 0x26,/*failure Prevents Execution Of Requested Action*/
	ROOR    = 0x31,/*reqeust out of range*/
	SAD     = 0x33,/*security access denied*/
	IK      = 0x35,/*invalid key*/
	ENOA    = 0x36,/*exceed number of attempts*/
	RTDNE   = 0x37,/*required time delay not expired*/
	UDNA    = 0x70,/*upload download not accepted*/
	TDS     = 0x71,/*transfer data suspended*/
	GPF     = 0x72,/*general programming failure*/
	WBSC    = 0x73,/*wrong block sequence coutner*/
	RCRRP   = 0x78,/*request correctly received-respone pending*/
	SFNSIAS = 0x7e,/*sub-function not supported in active session*/
	SNSIAS  = 0x7F,/*service not supported in active session*/
	EIR     = 0x83,/*engine is running*/
	EINR    = 0x84,/*engine is not running*/
	VSTH    = 0x88,/*vehicle speed too high*/
	VSTL    = 0x89,/*vehicle speed too low*/
	PTL     = 0x8A,/*throttle/peda too low*/
	PTH     = 0x8B,/*throttle/peda too high*/
	BSNC    = 0x8F,/*brake switch not closed(brake pedal not pressed or not applied)*/
	SLNIP   = 0x90,/*shifter lever not in park*/
	VTH     = 0x92,/*voltage too high*/
	VTL     = 0x93,/*voltage too low*/
}NegativeResposeCode_enum;/*诊断NRC*/

typedef enum
{
    UNSUPPORT_LEVEL = 0,
    ZERO_LEVEL = 2,
    EXTENDED_LEVEL = 1,
    DEVELOPMENT_LEVEL = 3,
    PROGRAMING_LEVEL = 5
}SecurityLevel_enum;/*安全等级*/

typedef enum
{
    DEFAULT_SESSION = 1,
    PROGRAM_SESSION,
    EXTENDED_SESSION
}Session_enum;/*会话模式*/

typedef enum
{
    HARD_RESET = 1,
    KEYOFFON_RESET,
    SOFT_RESET,
    ENABLERAPIDPOWERSHUTDOWN,
    DISENABLERAPIDPOWERSHUTDOWN
}Reset_enum;/*Reset类型*/

typedef enum
{
    WAIT_REQ_SEED = 0,
    WAIT_REC_KEY,
    WAIT_DELAY,
    UNLOCKED
}SecurityUnlockStep_enum;/*安全解锁步骤*/

typedef enum
{
    ENABLERX_TX = 0,
    ENABLERX_DISABLETX,
    DISABLERX_ENABLETX,
    DISABLE_RXTX
}CommunicationCtrol_enum;/*通信控制类型*/

typedef enum
{
    NORMALMESSAGE = 1,
    NETWORKMANAGEMENTMESSAGE,
    NORMAL_NETWORKMANAGEMENTMESSAGE
}MessageControlType_enum;/*通信控制消息类型*/

typedef enum
{
    UNSUPPORT = 0,
    READONLY,
    WRITEONLY,
    READWRITE
}DIDAttribute_enum;/*DID属性*/

typedef enum
{
    STARTROUTINE = 1,
    STOPROUTINE,
    REQUESTROUTINERESULTS
}RoutineControl_enum;/*例程控制子功能*/

typedef enum
{
    CheckProgrammingPreconditions = 0,
    EraseMemory,
    CheckValidateApplication,
    APPRollBack
}RoutineName_enum;/*例程名字*/

typedef enum
{
    No_Comress
}CompressionMethod_enum;/*程序压缩类型*/

typedef enum
{
    No_Encrypting
}EncryptingMethod_enum;/*程序加密类型*/

typedef enum
{
    NOTRUN = 0,
    RUNNING,
    FINISHED
}TaskStatues_enum;/*31启动的各个任务状态*/

typedef void (*ServiceHandle)(u16 Len, u8 FrameType, u8 *Data, u8 ServiceIndex);
typedef void (*RoutineHandle)(void);


#pragma pack(push)
#pragma pack(1)

struct SessionService
{
    bool                Support;/*当前诊断服务是否支持*/
    Service_enum        ServiceName;/*当前服务名字*/
    SecurityLevel_enum  PHYDefaultSession_Security;/*物理寻址默认会话下是否支持以及要求的安全等级*/
    SecurityLevel_enum  PHYProgramSession_Security;/*物理寻址编程会话下是否支持以及要求的安全等级*/
    SecurityLevel_enum  PHYExtendedSession_Security;/*物理寻址扩展会话下是否支持以及要求的安全等级*/
    SecurityLevel_enum  FUNCDefaultSession_Security;/*功能寻址默认会话下是否支持以及要求的安全等级*/
    SecurityLevel_enum  FUNCProgramSession_Security;/*功能寻址编程会话下是否支持以及要求的安全等级*/
    SecurityLevel_enum  FUNCExtendedSession_Security;/*功能寻址扩展会话下是否支持以及要求的安全等级*/
    bool                SuppressSupport;/*是否支持正响应抑制bit*/
    u8                  SubfunctionSupport;/*按照14229文档子功能顺序，每bit代表其是否支持此子功能*/
    ServiceHandle       SIDHandle;/*当前诊断服务的处理函数指针*/
};/*每个诊断服务的配置表*/
typedef struct SessionService SessionService_type, *pSessionService_type;


struct DIDConfigure
{
    u16                 ID;/*DID的ID*/
    u8                  Size;/*DID的大小*/
    DIDAttribute_enum   RWAttribute;/*DID的属性*/
    u16                 EEPromAddr;/*DID在物理内存中存储的地址*/
    SecurityLevel_enum  SecurityLevel;/*写DID需要的安全等级*/
    u16                  WritableAddr;/*是否可写，有的DID只能写一次*/
};
typedef struct DIDConfigure DIDConfigure_Type, *pDIDConfigure_Type;

struct PostiveResponse
{
    u16 Len;
    u8  Data[MAX_UDS_RES_Buff_Size];
};
typedef struct PostiveResponse PostiveResponse_Type, *pPostiveResponse_Type;

struct SecurityUnlock
{
    bool                Valid;/*是否有效*/
    SecurityLevel_enum  Level;/*安全等级*/
    u8                  SeedID;/*当前等级请求种子ID*/
    u8                  KeyID;/*当前等级接收钥匙ID*/
    u8                  Support;/*当前等级在哪些模式下支持，bit0~bit5代表：物理寻址默认会话、物理编程、物理扩展、功能默认、功能编程、功能扩展*/
    u8                  FailedDelayTime;/*超过解锁失败次数后延时参数*/
};
typedef struct SecurityUnlock SecurityUnlock_Type, *pSecurityUnlock_Type;

struct RoutineControlConfig
{
    u16                 ID;/*例程ID*/
    u8                  ParameterLen;/*例程需要的参数长度*/
    SecurityLevel_enum  DefaultSessionSecurity;
    SecurityLevel_enum  ProgramSessionSecurity;
    SecurityLevel_enum  ExtendedSessionSecuruty;
    u8                  RoutineControlTypeSupport;/*bit0~bit2：代表01、02、03是否支持（1：支持，0：不支持）。*/
    RoutineHandle       Routine/*该例程运行的程序指针*/;
    u8                  Result;/*例程执行结果*/
    bool                Execute_Finished;/*例程是否执行完毕*/
};
typedef struct RoutineControlConfig RoutineControl_Type, *pRoutineControl_Type;

#pragma pack(pop)


void UDS_Init(void);
void UDS_RxIndication(u8 *Data, u16 Len, u8 FrameType, u8 Channel, u8 apptype);

void UDS_TxConfirmation(u32 Canid,u8 SID, u8 Subfunction, u8 Result);

void UDS_MainFunction(void);
void UDS_ServiceHandle(u8 SID, u16 Len, u8 FrameType, u8 *Data);
void UDS_ResponseTimeout(void);
void UDS_TimeOperation(void);
void UDS_TaskClose(void);
void UDS_Service0x10Handle(u16 Len, u8 FrameType, u8 *Data, u8 ServiceIndex);
void UDS_GoToSession(u8 Session);
void UDS_SessionTimeout(void);
void UDS_EnterReprogramSession(void);
void UDS_Service0x11Handle(u16 Len, u8 FrameType, u8 *Data, u8 ServiceIndex);
void UDS_Reset(void);
void UDS_Service0x27Handle(u16 Len, u8 FrameType, u8 *Data, u8 ServiceIndex);
void UDS_GeneratSeed(u8 *Seed);
u8 UDS_rand(void);
void UDS_GenerateKey(const u8 *pSeed, u8 *pKey);
void UDS_UnlockFailedTimerTimeout(void);
void UDS_Service0x28Handle(u16 Len, u8 FrameType, u8 *Data, u8 ServiceIndex);
void UDS_CommunicationControl(u8 Subfunction, u8 ControlType);
void UDS_Service0x3EHandle(u16 Len, u8 FrameType, u8 *Data, u8 ServiceIndex);
void UDS_Service0x22Handle(u16 Len, u8 FrameType, u8 *Data, u8 ServiceIndex);
void UDS_Service0x2EHandle(u16 Len, u8 FrameType, u8 *Data, u8 ServiceIndex);
void UDS_Service0x31Handle(u16 Len, u8 FrameType, u8 *Data, u8 ServiceIndex);
void UDS_CheckProgrammingPreConditions(void);
void UDS_EraseMemory(void);
void UDS_ByteConvert(u8 *Data, u16 Len);
void UDS_CheckApplicationDataValid(void);
void UDS_APPRollBack(void);
void UDS_Service0x34Handle(u16 Len, u8 FrameType, u8 *Data, u8 ServiceIndex);
void UDS_BackupAPPCode(void);
void UDS_Service0x36Handle(u16 Len, u8 FrameType, u8 *Data, u8 ServiceIndex);
void UDS_Service0x37Handle(u16 Len, u8 FrameType, u8 *Data, u8 ServiceIndex);
u8 UDS_CheckSum(u8 *Data, u8 Len);

#endif

