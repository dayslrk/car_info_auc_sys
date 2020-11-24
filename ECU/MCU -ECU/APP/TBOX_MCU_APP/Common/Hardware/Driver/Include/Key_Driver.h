#ifndef _KEY_DRIVER_H
#define _KEY_DRIVER_H

#include "system.h"

#define KEY_PORT	GPIOF
#define KEY_UP		GPIO_Pin_6
#define KEY_DOWN	GPIO_Pin_7
#define KEY_LEFT	GPIO_Pin_8
#define KEY_RIGHT	GPIO_Pin_9


#define READ_KEY_UP		(KEY_PORT->IDR&KEY_UP)
#define READ_KEY_DOWN	(KEY_PORT->IDR&KEY_DOWN)
#define READ_KEY_LEFT	(KEY_PORT->IDR&KEY_LEFT)
#define READ_KEY_RIGHT	(KEY_PORT->IDR&KEY_RIGHT)


typedef enum
{
	KEY_NOT_PRESS = 0,
	KEY_UP_PRESS = 1,
	KEY_DOWN_PRESS = 2,
	KEY_LEFT_PRESS = 3,
	KEY_RIGHT_PRESS = 4
	
}KeyVal;





#endif /*_KEY_DRIVER_H*/
