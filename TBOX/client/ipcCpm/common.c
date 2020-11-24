#include "common.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "log.h"

#include <sys/time.h>


const u32 POLYNOMIAL = 0xEDB88320;
static u32 table[256] ;

void make_crc_table()
{
    int i, j;
    for (i = 0; i < 256 ; i++)
        for (j = 0, table[i] = i; j < 8; j++)
            table[i] = (table[i]>>1)^((table[i]&1)?POLYNOMIAL:0);
}


u32 crc32_calc( char *buff, int len)
{
    int i = 0;
    u32 crc = 0xFFFFFFFF;
    for (i = 0; i < len; i++)
        crc = (crc >> 8) ^ table[(crc ^ buff[i]) & 0xff];
    return ~crc;
}

//文本文件读取
u32 ReadTextFile(char *pFileName, u8 *pdata, u32 len)
{
	FILE *pFile = NULL;

//	LOG(DEBUG, "satrt read,file:%s, data:%s\n", pFileName, pdata);
	if(NULL == pFileName || NULL == pdata )
	{
		return 1;
	}

	

	pFile = fopen(pFileName, "rt");
	if(NULL == pFile)
	{
		return 2;
	}

	fgets(pdata, len, pFile);

	fclose(pFile);

	LOG(DEBUG, "read succ, file:%s, data:%s\n", pFileName, pdata);
	return 0;	
}

int AscToInt(const char * pData)
{
	int Data = 0;
	int i = 0, len = 0;
	int sign = 0;

	u8 base = 10;		//默认基数是10
	

	if( NULL != pData)
	{
		len = strlen((const char *)pData );
		for( i = 0; i < len; i++)
		{
			if((' ' == pData[i]) &&(0 == sign))
			{
				;			//跳过
			}
			if('x' == pData[i])
			{
				base = 0x10;			//切换为16进制
				Data = 0;				//x前边的数据清空
			}
			else if((0==sign) && ('-' == pData[i]))
			{
				sign = -1;				//第一个符号位
			}
			
			else if((0 == sign) && (('0' <= pData[i]) && ('9' >= pData[i])))
			{
				sign = 1;				//第一个数字
				Data = pData[i]-'0';
			}
			else if((0!=sign) && (('0' <= pData[i]) && ('9' >= pData[i])))
			{
				Data *= base;					//第N个数字
				Data += pData[i]- '0';
			}

			else if((0x10 ==base) &&(('a'<= pData[i]) && ('f' >= pData[i])))
			{
				//只有16进制才有字母
				Data *= base;
				Data += pData[i] -'a'+10;
			}
			else if((0x10 ==base) &&(('A'<= pData[i]) && ('F' >= pData[i])))
			{
				//只有16进制才有字母
				Data *= base;
				Data += pData[i] -'A'+10;
			}
			
			else
			{
				break;				//数字结束
			}
		}

		Data *= sign;
	
	}
	else
	{
		LOG(ERR, "Param Faild.\n");
	}

	if(0 == i)
	{
	    LOG(ERR, "Param Faild.\n");
	}

	return Data;
}

u32 AscToBcd(const char * pAsc, char *pBcd, u32 *plen)
{
	int i = 0, len = 0;

	u8 base = 0x16;		
	u32 offset = 0;

	if( NULL != pAsc && NULL != pBcd)
	{
		len = strlen((const char *)pAsc );
		
		for( i = 0; i < len; i++)
		{
			if('x' == pAsc[i])
			{
			    if(0 == ((len - i -1) % 2))
			        offset = 0;
			    else 
			        offset = 1; //奇数位数字，补0补前边
			  
				pBcd[0] = 0;
			}
			else if(('0' <= pAsc[i]) && ('9' >= pAsc[i]))
			{
			    if(0 ==(offset%2))
			    	pBcd[offset/2] = (pAsc[i]-'0')<<4;
			    else
			        pBcd[offset/2] |= (pAsc[i]-'0');

			    offset++;
			}
			else if(('a'<= pAsc[i]) && ('f' >= pAsc[i]))
			{
				if(0 ==(offset%2))
			    	pBcd[offset/2] = (pAsc[i]-'a' +0x0a)<<4;
			    else
			        pBcd[offset/2] |= (pAsc[i]-'a' +0x0a);

			    offset++;     
			}
			else if(('A'<= pAsc[i]) && ('F' >= pAsc[i]))
			{
				if(0 ==(offset%2))
			    	pBcd[offset/2] = (pAsc[i]-'A' +0x0a)<<4;
			    else
			        pBcd[offset/2] |= (pAsc[i]-'A' +0x0a);

			    offset++;     
			}
			else
			{
				break;				//数字结束
			}
		}

		pBcd[offset/2 +1]= 0;
		*plen = offset/2+1;
	
	}
	else
	{
		LOG(ERR, "Param Faild.\n");
	}

	return 0;
}


//返回当前毫秒时间
unsigned long long  GetCurTimeMS()
{
	unsigned long long cur_time;
	struct timeval tv;

	gettimeofday(&tv, NULL);
	cur_time = tv.tv_sec;
	cur_time *= 1000;
	cur_time += (tv.tv_usec / 1000);


	return cur_time;
}

