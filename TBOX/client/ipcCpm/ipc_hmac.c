
#include "ipcCom.h"



u32 ipc_send( u8 *pReq, const u32 req_len )
{
    u32 ret = 1;
    
    do{
        if(NULL == pReq || 0 == req_len )
        {
            ret = 1;
            LOG(WARRN, "Param Faild.\n");
            break;
        }
    
        SendData(EN_MPU_APP7, EN_MCU_APP1, pReq, req_len );
        ret = 0;
    }while(0);
    return ret;
}



u32 ipc_read(u8 *pRes, u32 *pres_len)
{
    u32 ret = 1;
    u8 RecvBuff[1024] = {0};
    u32 recv_time = 0;
    EN_APPS en_sender;
    u32 recv_len = 0;
    u32 offset = 0;
    
    do{
        if( NULL == pRes || NULL == pres_len)
        {
            ret = 1;
            LOG(WARRN, "Param Faild,point is null.\n");
            break;
        }
        if( 0 == *pres_len)
        {
            ret = 1;
            LOG(WARRN, "recv_buff len is zero.\n");
            break;
        }
 
        do{
            usleep(100*1000);       //延时100ms
            recv_time ++;
            
            recv_len = RecvData(EN_MPU_APP7, &en_sender, RecvBuff);
            if(recv_len < 49 )
            {
            	continue;
            }

			if(recv_len > *pres_len)
			{
				LOG(WARRN, "RecvLen bigger than RecvBuffLen.\n");
				break;
			}

			*pres_len = recv_len;
            memcpy(pRes, RecvBuff, recv_len);
			
            ret = 0;
            break;
        }while(recv_time < 5);      //超时时间500ms
        
       
    }while(0);

    return ret;
}

