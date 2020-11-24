#ifndef TRANS_H
#define TRANS_H

#include "comm_if.h"
//#include "Com.h"

#define MAX_S_MSG_DATA			(256)
#define MAX_R_MSG_DATA			(1024+64)

#define MAX_SEND_DATA 			(MAX_R_MSG_DATA*2)

#define SEND_PRI_NUM			1		//发送优先级数量
#define RECV_NUM				3		//接收者数量

#define TIMEOUT_SEND			100

#define FRAME_TYPE_SEND			0x00
#define FRAME_TYPE_ACK			0x01

typedef void (*CALLBACK) (MSG_Queue *);



#if 0
#define APPCALLBACK1            (UART_Recv_CallBack)			//MCU诊断
#define APPCALLBACK2            (NULL)
#define APPCALLBACK3            (UART_MpuToMcuVehInforGet)		//
#define APPCALLBACK4            (UART_MpuVersionWrite)
#define APPCALLBACK5            (NULL)

#else
#define APPCALLBACK1            (NULL)							//MCU诊断
#define APPCALLBACK2            (NULL)
#define APPCALLBACK3            (NULL)			//MPU版本号接收
#define APPCALLBACK4            (NULL)							//电源管理
#define APPCALLBACK5            (NULL)
#endif 

u32 SendData_EN_MCU_APP1(const EN_APPS Recver, const u32 CanID, u16 DLen, const u8 *pdata);

u32 SendData(const u8 SN, const EN_APPS Sender, const EN_APPS Recver, const u16 DLen, const u8 * pdata);
u8 ReadData(const EN_APPS Recver, EN_APPS *pSender,u32 *plen, u8 *pdata);

u32 msgq_dist();


u32 TransInit();

u32 TransMain();

#endif

