#include "Flash_Driver.h"

static u32 FlashDriver_ReadWord(u32 ReadAddr);

/**
 *	@brief	读取一个字（32位数据）
 *  @param	addr,必须为4的倍数
 *  @retval	对应得16位数据
 */
static u32 FlashDriver_ReadWord(u32 ReadAddr)
{
	return *(vu32 *)ReadAddr;
}

/**
 *	@brief	获取某个地址所在的flash扇区
 *  @param	addr:flash地址
 *  @retval	0~11,即addr所在的扇区
 */
uint16_t FlashDriver_GetFlashSector(u32 addr)
{
	if(addr<ADDR_FLASH_SECTOR_1)return FLASH_Sector_0;
	else if(addr<ADDR_FLASH_SECTOR_2)return FLASH_Sector_1;
	else if(addr<ADDR_FLASH_SECTOR_3)return FLASH_Sector_2;
	else if(addr<ADDR_FLASH_SECTOR_4)return FLASH_Sector_3;
	else if(addr<ADDR_FLASH_SECTOR_5)return FLASH_Sector_4;
	else if(addr<ADDR_FLASH_SECTOR_6)return FLASH_Sector_5;
	else if(addr<ADDR_FLASH_SECTOR_7)return FLASH_Sector_6;
	else if(addr<ADDR_FLASH_SECTOR_8)return FLASH_Sector_7;
	else if(addr<ADDR_FLASH_SECTOR_9)return FLASH_Sector_8;
	else if(addr<ADDR_FLASH_SECTOR_10)return FLASH_Sector_9;
	else if(addr<ADDR_FLASH_SECTOR_11)return FLASH_Sector_10; 
	return FLASH_Sector_11;	
}

/**
 *	@brief	从指定地址开始写入指定长度的数据
 *  @param	WriteAddr,起始地址；pBuffer,数据指针；NumToWrite，范围
 *  @retval	0:写入成功，1:写入失败
 */
u8 FlashDriver_Write(u32 WriteAddr,u32 *pBuffer,u32 NumToWrite)	
{ 
	u8 status = 0;
	u32 endaddr=0;
	
	if(WriteAddr<STM32_FLASH_BASE||WriteAddr%4)	return 1;	//非法地址
	FLASH_Unlock();										//解锁 
	FLASH_DataCacheCmd(DISABLE);						//FLASH擦除期间,必须禁止数据缓存

	endaddr = WriteAddr+NumToWrite*4;	//写入的结束地址
	status = FlashDriver_Erase(WriteAddr, NumToWrite);
	if(status==0)
	{
		while(WriteAddr < endaddr)//写数据
		{
			if(FLASH_ProgramWord(WriteAddr,*pBuffer) != FLASH_COMPLETE)//写入数据
			{ 
				break;	//写入异常
			}
			WriteAddr += 4;
			pBuffer++;
		}
	}
	FLASH_DataCacheCmd(ENABLE);	//FLASH擦除结束,开启数据缓存
	FLASH_Lock();//上锁
	return 0;
} 

/**
 *	@brief	从指定地址开始读出指定长度的数据
 *  @param	WriteAddr,起始地址；pBuffer,数据指针；NumToWrite，范围
 *  @retval	无
 */
void FlashDriver_Read(u32 ReadAddr, u32 *pBuffer, u32 NumToRead)   	
{
	u32 i;
	for(i=0;i < NumToRead;i++)
	{
		pBuffer[i] = FlashDriver_ReadWord(ReadAddr);//读取4个字节.
		ReadAddr += 4;//偏移4个字节.	
	}
}

/**
 *	@brief	擦除FLASH扇区，
 *  @param	EraseAddr,起始地址；Index，范围
 *  @retval	0成功，非0失败
 *	@note	不管是写入还是擦除，使用本函数前请先对flash解锁，用完请加锁，加解锁不能嵌套,
 *	@note	不管是写入还是擦除，使用本函数前请禁止数据缓存，用完请解除，同样不能嵌套
 */
u8 FlashDriver_Erase(u32 EraseAddr, u32 Index)
{
	u32 StartAddr = EraseAddr;
	u32 EndAddr = EraseAddr+Index*4;
	FLASH_Status Status = FLASH_COMPLETE;
	
	if(EraseAddr < STM32_FLASH_BASE||EraseAddr%4)	return 1;	//非法地址
	if(EraseAddr < 0x1FFF0000)//主存储区
	{
		while(StartAddr < EndAddr)
		{
			if(FlashDriver_ReadWord(EraseAddr) != 0xFFFFFFFF) //如果flash值不是0xFFFFFFFF就擦除
			{
				Status = FLASH_EraseSector(FlashDriver_GetFlashSector(StartAddr), VoltageRange_3);
				if(Status != FLASH_COMPLETE)	return 1;		//发生错误了
			}
			else StartAddr += 4;
		}
	}
	return 0;
}



