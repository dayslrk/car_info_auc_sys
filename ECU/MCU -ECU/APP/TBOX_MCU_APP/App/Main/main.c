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
#include "EepManage_public.h"

#include "MpuPowerCtrl.h"
#include "Routing.h"
#include"sm3.h"

extern const DIDConfigure_Type DIDList[DID_COUNT];
extern u8 CurrentSession;

/*ÈÎÎñ¾ä±ú*/
static TaskHandle_t MainTaskHandle;
static TaskHandle_t Task1Handle;
static TaskHandle_t MonitorHandle;
static TaskHandle_t AppUserHandle;

static void AppMainTask(void);
static void AppTask1(void);
static void AppMonitorTask(void);
static void AppUserTask(void);


static void DevicesInit(void);

/**
 *    @brief    设备初始化函数，所有设备在这里初始化
 *    @param    无
 *    @retval    无
 */
static void DevicesInit(void)
{
    delay_init(168);
    Logging_Init();
    LED_GPIO_Init();
    ComUsart_Init(115200);    /*通信串口初始化后，就会去接收数据了*/
    IWDG_Init(IWDG_Prescaler_64, 2500);//溢出时间为一秒，超过1秒没喂狗则复位
    RngDriver_Init();
    TIMx_Init(9, 8399);
    CRCDriver_Init();
    AT24Cxx_Init();

    Eeprom_Init();
    
    delay_xms(100);
    
    CAN_Queue_Init();
    UDS_Init();
    ISO15765_Init();
    MpuPower_Init();
	
	CAN1Driver_Init(CAN_Mode_Normal);
	CAN2Driver_Init(CAN_Mode_Normal);
	
	Log(LOG_DEBUG,"%s Success\n", __func__);
}


/**
 *    @brief    main函数
 *    @param    无
 *    @retval    int
 */
int main(void)
{
    BaseType_t xReturn = pdPASS;
    
    /*硬件初始化后创建任务启动系统*/
    //NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x20000); //重定向向量表到APP的地址空间
    SCB->VTOR = FLASH_BASE | 0x20000;    //重定向向量表到APP的地址空间
    
    DevicesInit();
    
    xReturn = xTaskCreate((TaskFunction_t)AppMainTask,        //任务函数
                          (const char*    )"MainTask",        //任务名称
                          (uint16_t        )0x200,             //任务栈大小
                          (void*        )NULL,              //任务函数的参数
                          (UBaseType_t    )1,                 //任务优先级
                          (TaskHandle_t*)&MainTaskHandle);  //任务控制块
                          
    if(xReturn == pdPASS)
    {
        Log(LOG_DEBUG, "Main Task Create Success\n");
        Log(LOG_DEBUG,"App OS start\n");
        
        vTaskStartScheduler();    /*启动任务，开始调度*/
    }
    else    return -1;
    /*正常情况下，以下代码不在执行*/
}

/**
 *    @brief    开始任务函数
 *    @param    无
 *    @retval    无
 */
static void AppMainTask(void)
{
    BaseType_t xReturn = pdPASS;
    taskENTER_CRITICAL();          //进入临界区
    
    /*这里可以创建信号量、消息队列、timer等IPC*/
    
    /*创建各个实际任务，完成后将删除MainTask任务*/
    xReturn = xTaskCreate((TaskFunction_t)AppTask1,       //任务函数
                          (const char*    )"Task1",         //任务名称
                          (uint16_t        )0x400,           //任务栈大小
                          (void*        )NULL,            //任务函数的参数
                          (UBaseType_t    )2,               //任务优先级
                          (TaskHandle_t*)&Task1Handle);    //任务控制块
                          
    if(xReturn == pdPASS)
        Log(LOG_DEBUG, "Task1 Create Success\n");

    
    xReturn = xTaskCreate((TaskFunction_t)AppMonitorTask,   //任务函数
                          (const char*    )"Monitor",          //任务名称
                          (uint16_t        )0x80,               //任务栈大小
                          (void*        )NULL,                //任务函数的参数
                          (UBaseType_t    )4,                   //任务优先级
                          (TaskHandle_t*)&MonitorHandle);    //任务控制块
                          
    if(xReturn == pdPASS)
        Log(LOG_DEBUG, "MonitorTask Create Success\n");

    
    xReturn = xTaskCreate((TaskFunction_t)AppUserTask,             //用户线程
                          (const char*    )"User",                 
                          (uint16_t        )0xA00,                  
                          (void*        )NULL,                   
                          (UBaseType_t    )2,                      
                          (TaskHandle_t*)&AppUserHandle);       
                
    if(xReturn == pdPASS)
        Log(LOG_DEBUG, "User Task Create Success\n");
    
    vTaskDelete(MainTaskHandle); //删除MainTask任务
    taskEXIT_CRITICAL();         //退出临界区
}

/**
 *    @brief    任务1函数
 *    @param    无
 *    @retval    无
 */
static void AppTask1(void)
{
    u8 JumpAPPFailedCount = 0;
    int i = 0;

    __enable_irq(); //APP在没有创建任务之前使能所有中断会进入硬件错误,
    Eeprom_Read(EEPROM_JUMPTOAPPFAILEDCOUNT, &JumpAPPFailedCount, (u16)1);

    if (JumpAPPFailedCount != 0xFF)
    {
        JumpAPPFailedCount = 0xFF;
        Eeprom_Write(EEPROM_JUMPTOAPPFAILEDCOUNT, &JumpAPPFailedCount, (u16)1);
    }
    else
    {
        ;
    }
    
    for(;;)
    {
        UDS_WaitTimeout();
        CAN_MainFunction();     //从CanBuffer读取数据进行接收
        ISO15765_SendPacket();    //CAN Tp层自动发送
        TransMain();            //MCU、MPU通信处理服务
        i++;
        if(0 == (i % 500))
        {
          LED1_TOGGLE;
          i = 0;
        }
        delay_ms(2);
    }
}

/**
 *    @brief    系统监视任务函数
 *    @param    无
 *    @retval    无
 */
static void AppMonitorTask(void)
{
    for(;;)
    {
        IWDG_Feed();
        delay_ms(900);
    }
}

static void AppUserTask(void)
{  
    static TickType_t TaskTime;
    portTickType xFrequency;
		
    /*获取当前的系统时间*/
    TaskTime = xTaskGetTickCount();
    xFrequency = pdMS_TO_TICKS(10);
    
    while(1)
    {
        UDS_MainFunction();        //UDS诊断服务服务

        msgq_dist();            //分发接收的MPU的数据


        KL15_VolCheck();

        Routing_MainFunction();
		
        HMac_Calc();         //计算并发送数据
                
        LED2_TOGGLE;
        //delay_ms(10);
        /*10ms绝对延时*/
        vTaskDelayUntil(&TaskTime, xFrequency);
    }
}
