#include "main.h"
#include "CAN.h"
#include "ISO14229_1.h"
#include "CanQueue.h"
#include "stm32f4xx_can.h"
#include "CAN1_Driver.h"
#include "ISO15765_2.h"
#include "CAN2_Driver.h"
#include "Rng_Driver.h"
#include "St_Flash.h"
#include "Timer.h"
#include "FreeRTOS.h"
#include "Eeprom.h"
#include "Trans.h"


#include "UDS_Diag_Client.h"

extern const DIDConfigure_Type DIDList[DID_COUNT];
extern u8 CurrentSession;

/*ÈÎÎñ¾ä±ú*/
static TaskHandle_t MainTaskHandle;
static TaskHandle_t Task1Handle;
static TaskHandle_t MonitorTaskHandle;
static TaskHandle_t UserHandle;

/*¶þÖµÐÅºÅÁ¿¾ä±ú*/
//SemaphoreHandle_t BinarySemaphore;

/*ÈÎÎñº¯Êý*/
static void BootLoaderMainTask(void);
static void BootLoaderTask1(void);
static void BootLoaderMonitorTask(void);
static void BootLoaderUserTask(void);

/*BootLoaderµ½APPµÄÌø×ªº¯Êý*/
static void OTA_BootLoaderToApp(u32 AppAddr);

static void DevicesInit(void)
{
	__enable_irq();
	delay_init(168);
	Logging_Init();
	LED_GPIO_Init();
	ComUsart_Init(115200);
	IWDG_Init(IWDG_Prescaler_64, 2500);//Òç³öÊ±¼äÎªÒ»Ãë£¬³¬¹ý1ÃëÃ»Î¹¹·Ôò¸´Î»
	RngDriver_Init();
    TIMx_Init(9, 8399);
	CRCDriver_Init();
    AT24Cxx_Init();
	delay_xms(100);
	
	CAN_Queue_Init();
	UDS_Init();
	ISO15765_Init();

	UdsDiagTaskInit();
	
	
	CAN1Driver_Init(CAN_Mode_Normal);
	CAN2Driver_Init(CAN_Mode_Normal);
	
	Log(LOG_DEBUG,"%s Success\n", __func__);
}


/**
 *	@brief	mainº¯Êý
 *	@param	ÎÞ
 *	@retval	int
 */
int main(void)
{
	BaseType_t xReturn = pdPASS;
    
	/*Ó²¼þ³õÊ¼»¯ºó´´½¨ÈÎÎñÆô¶¯ÏµÍ³*/
	DevicesInit();
    
	xReturn = xTaskCreate((TaskFunction_t)BootLoaderMainTask,  //ÈÎÎñº¯Êý
						  (const char*	)"MainTask",           //ÈÎÎñÃû³Æ
						  (uint16_t		)0x200,                //ÈÎÎñÕ»´óÐ¡
						  (void*		)NULL,                 //ÈÎÎñº¯ÊýµÄ²ÎÊý
						  (UBaseType_t	)1,                    //ÈÎÎñÓÅÏÈ¼¶
						  (TaskHandle_t*)&MainTaskHandle);     //ÈÎÎñ¿ØÖÆ¿é
						  
	if(xReturn == pdPASS)
	{
		Log(LOG_DEBUG, "Main Task Create Success\n");
		Log(LOG_DEBUG,"BootLoader OS start\n");
		vTaskStartScheduler();	/*Æô¶¯ÈÎÎñ£¬¿ªÊ¼µ÷¶È*/
	}
	else	return -1;
	/*Õý³£Çé¿öÏÂ£¬ÒÔÏÂ´úÂë²»ÔÚÖ´ÐÐ*/
}

static void BootLoaderMainTask(void)
{
	BaseType_t xReturn = pdPASS;
	taskENTER_CRITICAL();          //½øÈëÁÙ½çÇø
	
	/*ÕâÀï¿ÉÒÔ´´½¨ÐÅºÅÁ¿¡¢ÏûÏ¢¶ÓÁÐ¡¢timerµÈIPC*/
	//´´½¨¶þÖµÐÅºÅÁ¿,ÍÆ¼öÊ¹ÓÃÈÎÎñÍ¨Öª´úÌæ¶þÖµÐÅºÅÁ¿£¬¸ü¿ìËÙ£¬ÏûºÄ¸üÉÙ×ÊÔ´£¨RAM£©
	//BinarySemaphore = xSemaphoreCreateBinary();	
	
	/*´´½¨¸÷¸öÊµ¼ÊÈÎÎñ£¬Íê³Éºó½«É¾³ýMainTaskÈÎÎñ*/
	xReturn = xTaskCreate((TaskFunction_t)BootLoaderTask1,	//ÈÎÎñº¯Êý
						  (const char*	)"Task1",			//ÈÎÎñÃû³Æ
						  (uint16_t		)0x400,				//ÈÎÎñÕ»´óÐ¡
						  (void*		)NULL,				//ÈÎÎñº¯ÊýµÄ²ÎÊý
						  (UBaseType_t	)2,					//ÈÎÎñÓÅÏÈ¼¶
						  (TaskHandle_t*)&Task1Handle);		//ÈÎÎñ¿ØÖÆ¿é
						  
	if(xReturn == pdPASS)
		Log(LOG_DEBUG, "Task1 Create Success\n");
	
	xReturn = xTaskCreate((TaskFunction_t)BootLoaderMonitorTask,  //ÈÎÎñº¯Êý
						  (const char*	)"Monitor",              //ÈÎÎñÃû³Æ
						  (uint16_t		)0x80,                  //ÈÎÎñÕ»´óÐ¡
						  (void*		)NULL,                   //ÈÎÎñº¯ÊýµÄ²ÎÊý
						  (UBaseType_t	)4,                      //ÈÎÎñÓÅÏÈ¼¶
						  (TaskHandle_t*)&MonitorTaskHandle);    //ÈÎÎñ¿ØÖÆ¿é
				
	if(xReturn == pdPASS)
		Log(LOG_DEBUG, "Monitor Task Create Success\n");
	
	xReturn = xTaskCreate((TaskFunction_t)BootLoaderUserTask,     //ÈÎÎñº¯Êý
						  (const char*	)"User",                 //ÈÎÎñÃû³Æ
                          (uint16_t        )0x600,                  
						  (void*		)NULL,                   //ÈÎÎñº¯ÊýµÄ²ÎÊý
						  (UBaseType_t	)2,                      //ÈÎÎñÓÅÏÈ¼¶
						  (TaskHandle_t*)&UserHandle);           //ÈÎÎñ¿ØÖÆ¿é
				
	if(xReturn == pdPASS)
		Log(LOG_DEBUG, "User Task Create Success\n");
	
	vTaskDelete(MainTaskHandle); //É¾³ýMainTaskÈÎÎñ
	taskEXIT_CRITICAL();         //ÍË³öÁÙ½çÇø
}

/**
 *	@brief	ÈÎÎñº¯Êý1
 *	@param	void
 *	@retval	ÎÞ
 */
static void BootLoaderTask1(void)
{
    u8 JumpAPPFailedCount = 0;
    u8 AppAddress[4] = {0};
    u32 JumpToAppAddress = 0;

	/*?麓?毛?贸?垄??拢卢陆酶?毛APP拢卢bootloader碌????茅?枚?锚潞贸?酶碌陆APP*/
	delay_ms(500);
    Eeprom_Read(EEPROM_JUMPTOAPPFAILEDCOUNT, &JumpAPPFailedCount, (u16)1);
    
    if ((JumpAPPFailedCount >= 3 && 0xFF != JumpAPPFailedCount) || CurrentSession == 2)
    {
        /*10ms?????禄麓?*/
        while(1)
        {
            LED0_TOGGLE;
            UDS_WaitTimeout();
            CAN_MainFunction();
           
            ISO15765_SendPacket();	//CAN Tp层自动发送
            
            TransMain();			//MCU、MPU通信处理服务
            
            delay_ms(2);
        }
    }
    else
    {
        if (0xFF != JumpAPPFailedCount)
        {
            JumpAPPFailedCount += (JumpAPPFailedCount < 3)?1:0;;
            Eeprom_Write(EEPROM_JUMPTOAPPFAILEDCOUNT, &JumpAPPFailedCount, (u16)1);
        }
        else
        {
            ;
        }

        Eeprom_Read(EEPROM_JUMPTOAPPADDRESS, AppAddress, 4);
        JumpToAppAddress = (u32)((AppAddress[0] << 24) |(AppAddress[1] << 16) |(AppAddress[2] << 8) |(AppAddress[3]));
        JumpToAppAddress = (JumpToAppAddress < MEMORY_SATRTADDRESS || JumpToAppAddress > MEMORY_ENDADDRESS)?MEMORY_SATRTADDRESS:JumpToAppAddress;
        
        OTA_BootLoaderToApp(JumpToAppAddress);	
    
		/*????麓煤?毛???媒鲁拢?茅驴枚???露虏禄?麓??碌?*/
		while(1);
    }
}


/**
 *	@brief	系统监视任务函数
 *	@param	无
 *	@retval	无
 */
static void BootLoaderMonitorTask(void)
{
	for(;;)
	{
		IWDG_Feed();
//		Flash_COM_Main();
		delay_ms(10);
	}
}

/**
 *	@brief	APPÆô¶¯º¯Êý
 *	@param	AppAddr:APP´æ·ÅµØÖ·
 *	@retval	ÎÞ
 */
static void OTA_BootLoaderToApp(u32 AppAddr)
{
	pFunc JumpToApp;
	
	Log(LOG_DEBUG, "Start App\n");
	if(((*(vu32 *)(AppAddr+4))&0xFF000000) == 0x08000000)//¼ì²éFLASHµØÖ·ÊÇ·ñºÏ·¨
	{
		if(((*(vu32*)AppAddr)&0x2FFE0000)==0x20000000)	//¼ì²éÕ»¶¥µØÖ·ÊÇ·ñºÏ·¨,·ÀÖ¹Ã»ÓÐÏÂ³ÌÐòÊ±Ö±½ÓÌø×ª£¬µ¼ÖÂ³ÌÐòÅÜ·É
		{ 
			/*¹Ø±ÕÊ¹ÓÃ¹ýµÄÖÐ¶Ï£¬²»È»Ìøµ½APPºó£¬Ã»ÓÐÊ¹ÓÃÏà¹ØÖÐ¶Ï£¬µ«ÒòBootLoaderÊ¹ÄÜÁË£¬APPÒòÕÒ²»µ½ÖÐ¶ÏÈë¿Ú¶øËÀ»ú*/
			USART_ITConfig(USART3, USART_IT_IDLE, DISABLE);
			DMA_ITConfig(DMA1_Stream1, DMA_IT_TE, DISABLE);
			DMA_ITConfig(DMA1_Stream3, DMA_IT_TE|DMA_IT_TC, DISABLE);
			TIM_ITConfig(TIM3,TIM_IT_Update,DISABLE);
			CAN_ITConfig(CAN1, CAN_IT_FMP0, DISABLE);
			CAN_ITConfig(CAN2, CAN_IT_FMP0, DISABLE);
			
			JumpToApp=(pFunc)*(vu32*)(AppAddr+4);		//ÓÃ»§´úÂëÇøµÚ¶þ¸ö×ÖÎª³ÌÐò¿ªÊ¼µØÖ·(¸´Î»µØÖ·)		
			MSR_MSP(*(vu32*)AppAddr);					//³õÊ¼»¯APP¶ÑÕ»Ö¸Õë(ÓÃ»§´úÂëÇøµÄµÚÒ»¸ö×ÖÓÃÓÚ´æ·ÅÕ»¶¥µØÖ·)
			__disable_irq(); 							//¹Ø±Õ×ÜÖÐ¶Ï£¬²»È»APP»áÓöµ½ÖÐ¶Ï´íÎó£¬µ¼ÖÂËÀ»ú
			__set_CONTROL(0);							//ÔÚRTOS¹¤³Ì£¬ÕâÌõÓï¾äºÜÖØÒª£¬ÉèÖÃÎªÌØÈ¨¼¶Ä£Ê½£¬Ê¹ÓÃMSPÖ¸Õë
			
			Log(LOG_DEBUG, "Jump To App Success\n");
			RCC_DeInit(); //¹Ø±ÕÍâÉè
			
			JumpToApp(); 							//Ìø×ªµ½APP£¬ÓÀ²»·µ»Ø
		}
		else Log(LOG_ERROR, "SP Addr Error\n");
	}
	else Log(LOG_ERROR, "Flash Addr Error\n");
}


static void BootLoaderUserTask(void)
{  
    while(1)/*10msÔËÐÐÒ»´Î*/
    {

	UDS_MainFunction();		//UDS诊断服务服务
      
        msgq_dist();			//分发接收的MPU的数据

		UdsDiagTaskMain();		//诊断程序
       
		LED2_TOGGLE;
        delay_ms(10);
    }
}
