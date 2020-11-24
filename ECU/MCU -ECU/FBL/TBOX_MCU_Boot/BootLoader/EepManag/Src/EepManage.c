#include "EepManage_private.h"
#include "EepManage_public.h"
#include "Eeprom.h"


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

