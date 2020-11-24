
#ifndef _TYPES_H_
#define _TYPES_H_

#include "stddef.h"
#include "stm32f4xx.h"

#define E_BUFFER_FULL               (1u)
#define E_BUFFER_EMPTY              (2u)
#define E_NULL_PTR                  (3u)
#define E_OK                        (0u)

#define FALSE                       (0u)
#define TRUE                        (1u)

/*#define NULL                        ((void*)0u)*/

#define DIAG_FUNC                   (0u)
#define DIAG_PHY                    (1u)

#define Tx_SUCCESS                  (0u)
#define Tx_FAILURE                  (1u)

#define BIGLITTLESWAP16(A)          ((((u16)(A) & 0xff00) >> 8) |  (((u16)(A) & 0x00ff) << 8))
#define BIGLITTLESWAP32(A)         ((((u32)(A) & 0xff000000) >> 24) | (((u32)(A) & 0x00ff0000) >> 8) | \
                                   (((u32)(A) & 0x0000ff00) << 8) | (((u32)(A) & 0x000000ff) << 24))

typedef unsigned char bool;
//typedef unsigned char u8;
//typedef volatile unsigned char vu8;
//typedef signed char s8;
//typedef unsigned short u16;
//typedef volatile unsigned short vu16;
//typedef signed short s16;
//typedef unsigned int u32;
//typedef volatile unsigned int vu32;
//typedef signed int s32;


#endif

