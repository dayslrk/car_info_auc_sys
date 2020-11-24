//
// Created by Faiz on 2020/3/11 0011.
//

#ifndef _COMMON_H
#define _COMMON_H

#include <string.h>
#ifdef __cplusplus
extern "C"
{
#endif

/*
	common base data Type
*/
typedef unsigned char          u8;                 /*!< Unsigned 8 bit data type */
typedef unsigned short         u16;                /*!< Unsigned 16 bit data type */
typedef unsigned int           u32;                /*!< Unsigned 32 bit data type */
typedef unsigned long long     u64;                /*!< unsigned 64 bit data type*/
typedef signed char            s8;                 /*!< signed 8 bit data type */
typedef signed short           s16;                /*!< signed 16 bit data type */
typedef signed int             s32;                /*!< signed 32 bit data type */
typedef signed long long       s64;                /*!< signed 64 bit data type */
typedef unsigned char          boolen;             /*!< boolen data type */ 
//typedef unsigned long          size_t;             /*!< size_t data type */   



#ifndef TRUE
    #define TRUE !0
#endif

#ifndef FALSE
    #define FALSE 0
#endif

/*
    Generic return value definition
*/

typedef enum 
{
    RETURN_E_OK     = 0,
    RETURN_E_NOTOK  
    
}Std_ReturnDef;

#define FREE(A)		\
do{					\
	if(NULL != A)	\
	{				\
		free(A);	\
		A = NULL;	\
	}				\
}while(0)


void make_crc_table();
u32 crc32_calc( char *buff, int len);

u32 ReadTextFile(char *pFileName, u8 *pdata, u32 len);

int AscToInt(const char * pData);

u32 AscToBcd(const char * pAsc, char *pBcd, u32 *plen);


unsigned long long  GetCurTimeMS();

#ifdef __cplusplus
}
#endif

#endif //FOTACLIENT_COMMON_H

