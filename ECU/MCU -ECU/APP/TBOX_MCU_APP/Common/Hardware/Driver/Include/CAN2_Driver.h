#ifndef _CAN2_DRIVER_H
#define _CAN2_DRIVER_H

#include "system.h"

#define CAN2_INT_ENABLE 	1u



void CAN2Driver_Init(u8 mode);
u8 CAN2Driver_SendData(CanTxMsg *TxMessage);
u8 CAN2Driver_RecvData(u8 *data);


#endif /*_CAN2_DRIVER_H*/


