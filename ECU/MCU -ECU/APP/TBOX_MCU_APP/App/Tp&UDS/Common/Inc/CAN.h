
#ifndef _CAN_H_
#define _CAN_H_

#include "BaseType.h"
#include "stm32f4xx_can.h"


#define CAN_Id_Standard             ((uint32_t)0x00000000)  /*!< Standard Id */
#define CAN_Id_Extended             ((uint32_t)0x00000004)  /*!< Extended Id */

#define CAN_RTR_Data                ((uint32_t)0x00000000)  /*!< Data frame */
#define CAN_RTR_Remote              ((uint32_t)0x00000002)  /*!< Remote frame */

#define CAN_CHANEEL1                         (1U)
#define CAN_CHANEEL2                         (2U)
#define MPU_CHANEEL1						 (3U)		//Â·ÓÉµ½MPU
#define APPMESSAGECOUNT                      (1u)


#pragma pack(push)
#pragma pack(1)

struct Single_Frame
{
		 u8  SF_DL  :4; /*1-7*/
		 u8  FLAG   :4;
		 u8  Bytes[7];
};

struct First_Frame
{
		 u8  FF_DL:4; /*8-fff*/
		 u8  FLAG :4;
         u8  FF_DL2;
		 u8  Bytes[6];
};

struct Consecutive_Frame
{
		 u8  CF_SN  :4; /*0x00-0x0f*/
		 u8  FLAG   :4;
		 u8  Bytes[7];	   
};

struct Flow_Control
{
		 u8  FC_FS  :4;	   /*0--0x02*/
		 u8  FLAG   :4;
		 u8  FC_BS;		   /*0x00-0xff*/
		 u8  FC_STmin;	   /*0x00-0xff*/
		 u8  Bytes[5];
};

typedef struct Single_Frame Single_Frame_Type, * pSingle_Frame_Type;
typedef struct First_Frame First_Frame_Type, * pFirst_Frame_Type;
typedef struct Consecutive_Frame Consecutive_Frame_Type, * pConsecutive_Frame_Type;
typedef struct Flow_Control Flow_Control_Type, * pFlow_Control_Type;

struct CAN_Frame
{
	 u32 ID;
     u8 Len;
	 union
	 {
	     u8  Byte[8];
		 Single_Frame_Type SF;
		 First_Frame_Type  FF ;
		 Consecutive_Frame_Type CF;
		 Flow_Control_Type	  FC;
	 };
};
typedef struct CAN_Frame  CAN_Frame_Type, * pCAN_Frame_Type;

typedef void (*pAPPMSGHANDLE)(pCAN_Frame_Type pCanFrame);

struct AppMessage
{
    u16                 CANID;
    bool                Valid;
    u8                  Length;
    u8                  ReceiveChannel;
    pAPPMSGHANDLE       pAppMsgHandle;
};
typedef struct AppMessage AppMessage_Type, *pAppMessage_Type;

#pragma pack(pop)


void CAN_MainFunction(void);
void CAN_RX1_IRQHandler(u8 Channel, CanRxMsg *RxMsg);
u8 CAN_SendData(u8 nChannel, pCAN_Frame_Type CANFrame);
u8 Get_CAN_Status(u8 nChannel);
void CAN_RecFiltration(u8 Channel, pCAN_Frame_Type CANFrame);



#endif

