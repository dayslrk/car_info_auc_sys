
#ifndef _ISO15765_2_h_
#define _ISO15765_2_h_


#include "BaseType.h"
#include "CAN.h"
#include "CanQueue.h"


//#define MAX_SED_Buff_Size           (1024+64)
//#define MAX_RCV_Buff_Size           (1024+64)

#define SF_FLAG                     (0x00U)
#define FF_FLAG                     (0x01U)
#define CF_FLAG                     (0x02U)
#define FC_FLAG                     (0x03U)

#define RESEND_COUNT                (100U)
#define FC_RESEND_COUNT             (10U)

#define CAN_CHANNEL1                (1U)
#define CAN_CHANNEL2                (2U)



#define DIAG_FUNC_ADDR				(0x7dfu)
#define DIAG_TBOX_ADDR 				(0x711u)
#define DIAG_GW_ADDR				(0x710u)
#define DIAG_IPK_ADDR				(0x760u)
#define DIAG_IVI_ADDR				(0x761u)
#define DIAG_BCM_ADDR				(0x740u)

#define DIAGFUNCRXID                    (DIAG_FUNC_ADDR)
#define DIAGCANRXID                     (DIAG_TBOX_ADDR)
#define DIAGCANTXID                     (DIAGCANRXID | 0x08)


/*#pragma anon_unions*/
#pragma pack(push)
#pragma pack(1)


/*__packed*/

typedef struct
{
	
	u32 CanId;						//CANID		
	u16 len;						//长度
	u16 CurIndex;					//当前已接受的数据
	u8 Sta;							//状态,0-空闲，1-接收中，2-已接收完成，3接收失败，4-处理完成
	u8 CurSN;						//当前包序号
	u8 Data[MAX_UDS_RCV_Buff_Size];

	u8 FC_Flag; 					//是否需要应答流控
}CHANNEL_RECV;

//功能寻址、TBOX、CGW、IPK、IVI、BCM
typedef enum
{
	EN_CHAN_FUNC_REQ= 0,
	EN_CHAN_PHY_REQ,
	EN_CHAN_GW_RES,
	EN_CHAN_IPK_RES,
	EN_CHAN_IVI_RES,
	EN_CHAN_BCM_RES,
	EN_CHAN_MAX_NUM,
	EN_CHAN_UNKNOW
}EN_CHAN;

#define MAX_RETRYTIMES 		3			//CAN最大重发次数3次

typedef struct
{
	u8 Sta;								//状态，0-空闲中，1-待发送,2-发送中，3-等待流控，4-已发送完成		
	u32 CanId;				
	u16 len;					
	u8 Data[MAX_UDS_RCV_Buff_Size];		                        //转发从MPU收到的诊断指令
	u16 Index;							//当前已发送数量
	u8 SN;								//当前流水号
	vu8     BlockSize;								/* 接收FC中指定的BlockSize大小 */
    vu8     BS_SN ;									/* 接收FC中指定的BS_SN */
    vu8     sTime;									/* 接收FC中指定的sTime */
	u8 RetryTimes;						//发送失败重试次数
}CHANNEL_SEND;
			


typedef struct 
{
    u8      Channel;								/* CAN通道编号 */
    vu8     Statue;									/* CAN通道状态 */
    u32     CANRID;									/* 接收自身诊断帧的CANID */
    u32     FUNCID;									/* 功能寻址自身诊断帧的CANID */
  	CHANNEL_RECV  ChanRecv[EN_CHAN_MAX_NUM];		/* 目前所有的接收缓存区均为1K以上，以后可以进行优化，只将本ECU的诊断请求711的赋值为1K */
    
    CHANNEL_SEND  ChanSend;							/* 数据发送缓冲区 */

}CAN_Channel_Type;

typedef CAN_Channel_Type* pCAN_Channel_Type;

#pragma pack(pop)


enum _CAN_STATUS_enum
{
	 CAN_CLOSED = 0,       /*总线关闭*/
	 CAN_INIT_OK = 1,       	 
};

extern CAN_Channel_Type   CAN_Channel;/*CAN总线：包含总线参数接收和发送的frame数据*/


void ISO15765_Init(void);
void ISO15765_ReceivePacket(u8 Channel, pCAN_Frame_Type pCANFrame);
void ISO15765_SendPacket(void);
void ISO15765_Keepalive(u8 nChannel);
bool ISO15765_MakePacket(u32 CanId, u8 *Data, u16 Len);

#endif

