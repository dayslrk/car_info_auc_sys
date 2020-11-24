#include "Eeprom.h"
#include "delay.h"

static u8 AT24Cxx_Check(void);

/**
 *	@brief	AT24Cxx��ʼ��
 *	@param	��
 *	@retval	0,�ɹ�����0,ʧ��
 */
u8 AT24Cxx_Init(void)
{
	IIC_Init();
	if(AT24Cxx_Check())	return 1;
	else return 0;
}

static u8 AT24Cxx_Check(void)
{
	u8 temp;
	temp = AT24Cxx_RecvByte(255);//����ÿ�ο�����дAT24CXX			   
	if(temp == 0x55) return 0;		   
	else//�ų���һ�γ�ʼ�������
	{
		AT24Cxx_WriteByte(255, 0x55);
	    temp=AT24Cxx_RecvByte(255);	  
		if(temp == 0x55) return 0;
	}
	return 1;
}

/**
 *	@brief	AT24Cxx��һ���ֽ�
 *	@param	RecvAddr,��ʼ�����ĵ�ַ
 *	@retval	����������
 */
u8 AT24Cxx_RecvByte(u16 RecvAddr)
{
	u8 Temp = 0;
	IIC_Start();
	if(EEPROM_TYPE > AT24C16)
	{
		IIC_SendByte(0xA0);	/*����д����*/
		IIC_WaitAck();
		IIC_SendByte(RecvAddr>>8);	/*���͸ߵ�ַ*/
	}
	else IIC_SendByte(0xA0 + ((RecvAddr/256)<<1));/*����������ַ0XA0,д����*/ 	 
	IIC_WaitAck();
	IIC_SendByte(RecvAddr%256);//���͵͵�ַ
	IIC_WaitAck();
	IIC_Start();
	IIC_SendByte(0xA1);
	IIC_WaitAck();
	Temp = IIC_RecvByte(0);
	IIC_Stop();
	return Temp;
}

/**
 *	@brief	AT24Cxxдһ���ֽ�
 *	@param	WriteAddr,��ʼд���ĵ�ַ��DataToWrite��Ҫд������
 *	@retval	��
 */
void AT24Cxx_WriteByte(u16 WriteAddr, u8 DataToWrite)
{
	IIC_Start();  
	if(EEPROM_TYPE>AT24C16)
	{
		IIC_SendByte(0xA0);	    //����д����
		IIC_WaitAck();
		IIC_SendByte(WriteAddr>>8);//���͸ߵ�ַ	  
	}else IIC_SendByte(0xA0+((WriteAddr/256)<<1));   //����������ַ0XA0,д���� 	 
	IIC_WaitAck();	   
    IIC_SendByte(WriteAddr%256);   //���͵͵�ַ
	IIC_WaitAck(); 	 										  		   
	IIC_SendByte(DataToWrite);     //�����ֽ�							   
	IIC_WaitAck();  		    	   
	IIC_Stop();//����һ��ֹͣ���� 
	delay_xms(8);
}

/**
 *	@brief	��AT24CXX�����ָ����ַ��ʼд�볤��ΪLen������,�ú�������д��16bit����32bit������
 *	@param	WriteAddr,��ʼд���ĵ�ַ��DataToWrite��Ҫд�����ݣ�Len,����2��4
 *	@retval	��
 */
void AT24Cxx_WriteLenByte(u16 WriteAddr,u32 DataToWrite,u8 Len)
{  	
	u8 t;
	for(t=0; t<Len; t++)
	{
		AT24Cxx_WriteByte(WriteAddr+t,(DataToWrite>>(8*t))&0xff);
	}										    
}

/**
 *	@brief	��AT24CXX�����ָ����ַ��ʼ��������ΪLen������,�ú������ڶ���16bit����32bit������
 *	@param	ReadAddr,��ʼ���������ݣ�DataToWrite,Ҫ�������ݣ�Len,����2��4
 *	@retval	��
 */
u32 AT24Cxx_ReadLenByte(u16 ReadAddr, u8 Len)
{  	
	u8 t;
	u32 temp=0;
	for(t=0;t<Len;t++)
	{
		temp<<=8;
		temp+=AT24Cxx_RecvByte(ReadAddr+Len-t-1); 	 				   
	}
	return temp;												    
}

/**
 *	@brief	��AT24CXX�����ָ����ַ��ʼ����ָ������������
 *	@param	ReadAddr,��ʼ�����ĵ�ַ,��24c02Ϊ0~255��pBuffer���������׵�ַ��NumToRead:Ҫ�������ݵĸ���
 *	@retval	��
 */
void AT24Cxx_Read(u16 ReadAddr,u8 *pBuffer,u16 NumToRead)
{
	while(NumToRead)
	{
		*pBuffer++=AT24Cxx_RecvByte(ReadAddr++);	
		NumToRead--;
	}
}  


/**
 *	@brief	��AT24CXX�����ָ����ַ��ʼд��ָ������������
 *	@param	WriteAddr,��ʼд��ĵ�ַ,��24c02Ϊ0~255��pBuffer���������׵�ַ��NumToRead:Ҫ�������ݵĸ���
 *	@retval	��
 */
void AT24Cxx_Write(u16 WriteAddr,u8 *pBuffer,u16 NumToWrite)
{
	while(NumToWrite--)
	{
		AT24Cxx_WriteByte(WriteAddr,*pBuffer);
		WriteAddr++;
		pBuffer++;
	}
}



