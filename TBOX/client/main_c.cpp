#include <iostream>
#include<dlfcn.h>
#include <cstring>
#include<stdio.h>
#include "client.h"

#include "ipcCom.h"
#include "ipc_hmac.h"

#include "log.h"
ssl_tls tls;
std::shared_ptr<client> c = NULL;

void get_hmac_from_server(const u8* input,const u32 input_len ,u8* ouput, u32 *poutput_len)
{  
	do{
		if(NULL == input || input_len <= 0 || NULL == ouput || NULL == poutput_len || 32 > *poutput_len)
		{
			break;
		}
	
	   	int ret = c->send_msg((const char *)input, input_len);
        if (ret < 0)
        {
            std::cout << "send_msg error!" << '\n';
            break;
        }
       
        ret = c->recv_msg(ouput, *poutput_len);
        if (ret < 0)
        {
            std::cout << "recv_msg error!" << '\n';
            break;
        }

		*poutput_len = ret;
     
		LOG(INFO, "ret len: %d.\n", ret);
		for(int i = 0; i< ret; i++)
		{
			printf("%02X ", ouput[i]);
			if(i % 0x10 == 0x0f)
				printf("\n");
		}
	}while(0);
		    

}


void com_server_init(const char* ip)
{
	tls.two_way_auth = true;
#if 0
    tls.ca_certf = std::string("etc/centos7/ca.crt");
    tls.certf    = std::string("etc/centos7/client.crt");
    tls.private_key  = std::string("etc/centos7/client.key");
    tls.passwd  = std::string("uway123");
#else
    tls.ca_certf = std::string("/etc/ubuntu18/cacert.pem");
    tls.certf    = std::string("/etc/ubuntu18/client.crt");
    tls.private_key  = std::string("/etc/ubuntu18/client.key");
    tls.passwd  = std::string("yyp123");
#endif
    tls.meth = TLSv1_client_method();

	c = std::make_shared<client>(ip, 9999, tls);
	// std::shared_ptr<client> c = std::make_shared<client>("127.0.0.1", 9999);
	if (c->initial() == false)
	{
		std::cout << "client initial error!" << '\n';
		return ;
	}
}


int main(int argc,char** argv)
{
	int i = 0;
	unsigned char Challenge[8] = {0};
	unsigned char SendData[128] = {0};
	unsigned char RecvData[128] = {0};
	unsigned char EcuID[32] = {0};
	unsigned char Hmac[64] = {0};
	

	
	u32 RecvLen = 0;
	
    if(argc!=2)
	{
	    std::cout<<"error number argc,try again"<<'\n';
	    return 0;
	}
	
	com_server_init((const char*)argv[1]);
	
	ipcCom_Init();

	do{
		std::cin>>i;

		printf("Start Generate Challenge: ");
		srand(GetCurTimeMS());
		for(i = 0; i < sizeof(Challenge);i++)
		{
			Challenge[i] = rand();
			printf("0x%02X ", Challenge[i]);
		}
		printf("\n");
		
		
	//在此处和MCU通信
		ipc_send(Challenge, sizeof(Challenge));
	
		RecvLen = sizeof(RecvData);
		ipc_read(RecvData, &RecvLen);

		if(RecvLen < 49)
		{
			//TODO:打印异常日志
			break;
		}

		//保存接收到的数据
		memcpy(EcuID, RecvData, 17);
		LOG(INFO,"EcuID: %s.\n", EcuID);
		memcpy(Hmac, RecvData+17, 32 );

		//构造通服务器通信数据
		memcpy(SendData, EcuID, 17);
		memcpy(SendData+17, Challenge, 8);

		RecvLen = sizeof(RecvData);
		get_hmac_from_server(SendData, 25, RecvData, &RecvLen);
		
		if(0 == memcmp(Hmac, RecvData, 32))
		{
			printf("hmac check successfully.\n");
		}
		else
		{
			printf("hamc check failed.\n");
		}
		
	    
	}
	while(1);
    return 0;
}
