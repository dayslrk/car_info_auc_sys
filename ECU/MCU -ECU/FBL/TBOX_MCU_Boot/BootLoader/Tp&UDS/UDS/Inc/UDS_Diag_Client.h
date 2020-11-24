#ifndef UDS_DIAG_CLIENT_H
#define UDS_DIAG_CLIENT_H

#include "stm32f4xx.h"


typedef struct
{
	u8 TaskSta;				//0-状态未知，1-有任务， 2-任务tp发送中，3-等待接收处理
	u32 Tick;				//任务发送成功后，开始计时
	u32 CanId;				//诊断服务请求CANID
	u8 SID;					//诊断服务SID
	u8 apptype;				//MPU 应用
	u16 DataLen;			//诊断信息长度,
	u8 Data[1024+48];		//诊断数据	数据从SID开始
	
}ST_UDS_DIAG_TASK;


void AddUdsDiagTaskFromMpu(const u32 CanId, const u16 len, const u8 *pdata, u8 apptype);


void UdsDiagTaskInit();

void UdsDiagTaskMain();


#endif /* UDS_DIAG_CLIENT_H */

