#include "EepManage_private.h"
#include "EepManage_public.h"
#include "Eeprom.h"

#include "string.h"

bool Eeprom_Write(Eeprom_Index_EU Index,u8 *pBuffer,u16 Len)
{
    bool Retval = FALSE;

    if (Len <= EepromData[Index].Size && Index < EEPROM_INDEXMAX)
    {
        AT24Cxx_Write(EepromData[Index].Addr, pBuffer, Len);
        Retval = TRUE;
    }
    else
    {
        Retval = FALSE;
    }

    return Retval;
}

bool Eeprom_Read(Eeprom_Index_EU Index,u8 *pBuffer,u16 Len)
{
    bool Retval = FALSE;

    if (Len <= EepromData[Index].Size && Index < EEPROM_INDEXMAX)
    {
        AT24Cxx_Read(EepromData[Index].Addr, pBuffer, Len);
        Retval = TRUE;
    }
    else
    {
        Retval = FALSE;
    }

    return Retval;
}

void Eeprom_Init( )
{
    u8 Data[4] = {0};
    u16 len = 0;

    
    AT24Cxx_Read(0, Data, 4);
    if(0 != memcmp(Data, EEPROM_DATA_VERSION, 4))
    {
        AT24Cxx_Write(0, EEPROM_DATA_VERSION, 4);

        switch(EEPROM_TYPE)
        {
            case AT24C01:
                len = 128 -4;
                break;
            case AT24C02:
                len = 256 -4;        //4字节头
                break;
            default:
                break;
        }

        //清空EEPROM数据
        for(u16 index = 0; index < len; index ++)
        {
            AT24Cxx_WriteByte(4+index,0xff);
        }
        
    }
    
}

