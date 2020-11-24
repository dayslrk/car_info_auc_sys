#include "St_Flash.h"


/**
 *	@brief	从指定地址开始写入指定长度的数据
 *  @param	WriteAddr,起始地址；Buffer,数据指针；index，范围
 *  @retval	0:写入成功，1:写入失败
 */
u8 Flash_Write(u32 WriteAddr, u8 *Buffer, u32 index)
{
	if(FlashDriver_Write(WriteAddr, Buffer, index) != 0)	return 1;
	else	return 0;
}

/**
 *	@brief	从指定地址开始读出指定长度的数据
 *  @param	WriteAddr,起始地址；Buffer,数据指针；index，范围
 *  @retval	无
 */
void Flash_Read(u32 ReadAddr, u8 *Buffer, u32 index)
{
	FlashDriver_Read(ReadAddr, Buffer, index);
}

/**
 *	@brief	擦除FLASH扇区，
 *  @param	EraseAddr,起始地址；Index，范围
 *  @retval	0成功，非0失败
 *	@note	不管是写入还是擦除，使用本函数前请先对flash解锁，用完请加锁，加解锁不能嵌套,FLASH_Unlock();	FLASH_Lock();			
 *	@note	不管是写入还是擦除，使用本函数前请禁止数据缓存，用完请解除，同样不能嵌套	FLASH_DataCacheCmd(DISABLE);FLASH_DataCacheCmd(ENABLE);
 */
u8 Flash_Erase(u32 EraseAddr, u32 index)
{
	u8 retval = 0;
	FLASH_Unlock();
	FLASH_DataCacheCmd(DISABLE);
	retval = FlashDriver_Erase(EraseAddr, index);
	FLASH_DataCacheCmd(ENABLE);
	FLASH_Lock();
	return retval;
}




