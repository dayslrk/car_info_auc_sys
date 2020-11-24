#ifndef _MAIN_H
#define _MAIN_H


#include <stdio.h>
#include <string.h>

#include "stm32f4xx.h"
#include "system.h"
#include "delay.h"

#include "Led_Driver.h"
#include "DbgUsart_Driver.h"
#include "Logging.h"
#include "ComUsart.h"
#include "Wdg_Driver.h"
#include "Timer_Driver.h"
#include "Crc_Driver.h"
#include "Flash_Driver.h"


typedef void (*pFunc)(void);			//用于跳转函数


#endif
