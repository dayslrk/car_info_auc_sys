#ifndef _CAN1_DRIVER_H
#define _CAN1_DRIVER_H


#include "system.h"


#define CAN1_INT_ENABLE		1u



void CAN1Driver_Init(u8 mode);
u8 CAN1Driver_SendData(CanTxMsg *TxMessage);
u8 CAN1Driver_RecvData(u8 *data);


#endif /*_CAN1_DRIVER_H*/


