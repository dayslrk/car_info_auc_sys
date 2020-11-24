#include"ipcCom.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "stdio.h"
#include "stdlib.h"


#include "dev_manage_config.h"

#include "log.h"

/*  msg id*/
static u32 msgqid = 0;
static IPC_Msg ipcMsg;



static void UpdateFreshValue( EN_APPS app)
{
	unsigned long long curtime;
	u8 FilePath[128] = {0};
	FILE *pfile = NULL;

	
	sprintf(FilePath, "%s%d", TMP_FP_MSGQ, app);
	curtime = GetCurTimeMS();

	pfile = fopen(FilePath, "wb");
	if(NULL == pfile)
	{
		LOG(WARRN, "File open faild,Path: %s.\n", FilePath);
		return ;
	}

	fwrite((void *)&curtime, sizeof(curtime), 1, pfile);

	fflush(pfile);

	fclose(pfile);	
}


void ipcCom_Init(void)
{

    key_t key;    
	key = ftok(MSGQ_FTOK_PATH, MSGQ_FTOK_PROJECTID);
	perror("ftok");
	msgqid = msgget(key, IPC_CREAT|0666);
	perror("msgget");

    UpdateFreshValue(CUR_APP);
}

unsigned int  RecvData(EN_APPS	en_app ,EN_APPS *pEn_Sender ,unsigned char *Buff)
{
	int i = 0;
	IPC_Msg MSG_Recv ={0};

	if(NULL == pEn_Sender || NULL == Buff)
	{
		LOG(ERR, "Param faild.\n");
		return -1;
	}
	//在此处对传入的en_app进行检查
	UpdateFreshValue(en_app);
	if (msgrcv(msgqid, &MSG_Recv, sizeof(MSG_QueueBuf), en_app, IPC_NOWAIT) == -1)//IPC_NOWAIT
	{
		//perror("msgrcv");
		return 0;
	}

	printf("MsgRecv:");
	while(i < (MSG_Recv.mdata.mlen))
	{
		printf("0x%02x  ",MSG_Recv.mdata.mbuff[i]);
		i++;
	}
	printf("\n");

	*pEn_Sender = MSG_Recv.mdata.apptype;
	memcpy(Buff, (const unsigned char *)&MSG_Recv.mdata.mbuff[0], MSG_Recv.mdata.mlen);
	return MSG_Recv.mdata.mlen;
}


char SendData(const EN_APPS	en_sender ,const EN_APPS en_recver , const unsigned char *Buff, const unsigned int Length)
{	
	
	IPC_Msg MSG_Send = {0};

    //该应用较为特殊，基本无需接收应答，因此需做处理，防止消息发送失败
	UpdateFreshValue(en_sender);
	
	LOG(DEBUG, "SendData len:%d.\n",Length);
	MSG_Send.mtype = en_sender+1;	//和ComMPU通信协议
	memcpy(&MSG_Send.mdata.mbuff[0], Buff, Length);
	MSG_Send.mdata.mlen = Length ;
	MSG_Send.mdata.apptype = en_recver;
	if (!msgsnd(msgqid, &MSG_Send, sizeof(MSG_QueueBuf), 0))
	{
#if 1
		int i = Length;
		LOG(DEBUG,"MsgSend: ");
		while(i > 0)
		{
			printf("0x%02x  ",MSG_Send.mdata.mbuff[Length  - i]);
			i--;
		}
		printf("\n");
#endif
		return 0;
	}
	else
	{
		perror("msgsnd");
		return -1;
	}
	
	return -1;
}


