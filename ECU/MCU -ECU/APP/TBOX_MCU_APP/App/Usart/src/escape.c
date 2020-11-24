#include "escape.h"
#include "Logging.h"
#include <string.h>

const ST_ESCAPE st_escape[] = 
{
	{ 0x7F, 	ESCAPE_CH, 	0x01 },
	{ HEAD_H, 	ESCAPE_CH, 	0x02 },
	{ HEAD_L, 	ESCAPE_CH, 	0x03 },
	{ TAIL_H, 	ESCAPE_CH, 	0x04 },
	{ TAIL_L, 	ESCAPE_CH, 	0x05 },
	{ INF_1, 	ESCAPE_CH, 	0x06 },
	{ 0x00, 	0x00, 		0x00 }			//空数据
};


// 在此处增加 0x55AA 的头，以及 0x0A0D 的尾
u32 escape_encode(u8 *pIn, const u32 Inlen, u8 *pOut, u32 *pOutlen)
{
	u32 ret = 0;
	u32 index = 0,i= 0, j = 0;

	if(NULL == pIn || NULL == pOut || NULL == pOutlen || 0 == Inlen || *pOutlen < Inlen)
	{
/*        Log(LOG_DEBUG, "Param Faild.\n");*/
		return 1;
	}

	do{
		memset(pOut, 0x00, *pOutlen);
		
		pOut[index++] = HEAD_H;
		pOut[index++] = HEAD_L;

		//在此处填充数据
		for(j = 0; j < Inlen; j++ )
		{
			for(i= 0; 0x00 != st_escape[i].dst_l; i++)
			{
				if( st_escape[i].src == pIn[j] )
				{
					pOut[index++] = st_escape[i].dst_h;
					pOut[index++] = st_escape[i].dst_l;
					break;
				}
			}

			if(0x00 == st_escape[i].dst_l)
			{
				pOut[index++] = pIn[j];
			}

			if(index + (Inlen - j) + 2 > *pOutlen)		//2个字节的尾
			{
				ret = 2;
/*                Log(LOG_DEBUG, "Out Buff too small.\n");*/
				break;
			}
		}

		if(0 != ret)
		{
			break;
		}

		pOut[index++] = TAIL_H;
		pOut[index++] = TAIL_L;
		*pOutlen = index;
		
	}while(0);

	return ret;
}

//反转义预处理，去掉干扰字符
u32 pre_decode(u8 *pIn, const u32 Inlen, u8 *pOut, u32 *pOutlen)
{
	u32 ret = 0;
	u32 i = 0,j = 0;
	
	if(NULL == pIn || NULL == pOut || NULL == pOutlen )
	{
/*        Log(LOG_DEBUG, "Param Faild.\n");*/
		return 1;
	}

	for(i = 0; i< Inlen; i++)
	{
		if(INF_1 == pIn[i])
		{
			continue;
		}
		if(j >= *pOutlen)
		{
/*			Log(LOG_DEBUG, "Out Buff too small, OutBufflen: %d, curlen:%d, i:%d, Inlen:%d\n", *pOutlen, j, i,Inlen);*/
			ret = 2;
			break;
		}
		pOut[j++] = pIn[i];
	}
    
    *pOutlen = j;

	return ret;
}
u32 escape_decode(u8 * pIn, const u32 Inlen, u8 * pOut, u32 * pOutlen)
{
	u32 ret = 2;		//默认未知错误
	u32 index = 0, i = 0, j = 0;

	u8 sta = 0x00;	
	u8 flag = 0x00;		//转义字符 标志

	

	if(NULL == pIn || NULL == pOut || NULL == pOutlen || Inlen <=4)
	{
/*		Log(LOG_DEBUG, "Param Faild.\n");*/
		return 1;
	}

	do{
		for(i = 0; i< Inlen; i++)
		{
			if(0 == sta)
			{
				if(HEAD_H == pIn[i] && HEAD_L == pIn[i+1])
				{
					sta =1;		//找到报文头
					i++;	
				}
				else
				{
					//异常情况处理
				}
			}
			else if(1 ==sta)
			{
				if(1 == flag)
				{
					//转义字符处理
					//遍历表，dst-L 然后赋值
					for(index = 0; 0x00 != st_escape[index].dst_l; index++)
					{
						if(pIn[i] == st_escape[index].dst_l)
						{
							pOut[j++] = st_escape[index].src;
                            flag = 0;
							break;
						}
					}
					if(0x00 == st_escape[index].dst_l)
					{
/*						Log(LOG_DEBUG, "deCode Faild.\n");*/
						break;
					}
				}
				else
				{
					//字符判断
					if(ESCAPE_CH == pIn[i] )
					{
						flag = 1;
						continue;
					}
					else if(TAIL_H == pIn[i])
					{
						//数据处理完成
						*pOutlen = j;
						ret = 0;
						break;
					}
					else
					{
						pOut[j++] = pIn[i];
					}
				}
			}		
				
			
		}
		
	}while (0);
	

	return ret;
}

