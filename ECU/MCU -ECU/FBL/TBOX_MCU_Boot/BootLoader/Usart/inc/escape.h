#ifndef ESCAPE_H
#define ESCAPE_H

#include "BaseType.h"

#define HEAD_H		0x55
#define HEAD_L		0xAA

#define TAIL_H		0x0A
#define TAIL_L 		0x0D

#define INF_1		0x00		//干扰字符1

#define ESCAPE_CH	0x7F 		//转义符		

typedef struct
{
	u8 src;
	u8 dst_h;
	u8 dst_l;
}ST_ESCAPE;

u32 escape_encode(u8 *pIn, const u32 Inlen, u8 *pOut, u32 *pOutlen);

u32 pre_decode(u8 *pIn, const u32 Inlen, u8 *pOut, u32 *pOutlen);

u32 escape_decode(u8 * pIn, const u32 Inlen, u8 * pOut, u32 * pOutlen);


#endif /* ESCAPE_H */

