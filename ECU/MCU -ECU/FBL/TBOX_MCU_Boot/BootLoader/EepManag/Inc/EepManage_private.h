#ifndef EEPMANAGE_PRIVATE_H_
#define EEPMANAGE_PRIVATE_H_

#include "BaseType.h"
#include "EepManage_public.h"

typedef struct
{
   u16 Size;
   u16 Len;
   u16 Addr;
}Eeprom_ST;

Eeprom_ST EepromData[EEPROM_INDEXMAX] = 
{
    {EEPROM_CURRENT_SESSION_SIZE             , EEPROM_CURRENT_SESSION_LEN             , EEPROM_CURRENT_SESSION_OFFSET               },
    {EEPROM_DID_0xF183_SIZE                  , EEPROM_DID_0xF183_LEN                  , EEPROM_DID_0xF183_OFFSET                    },
    {EEPROM_DID_0xF187_SIZE                  , EEPROM_DID_0xF187_LEN                  , EEPROM_DID_0xF187_OFFSET                    },
    {EEPROM_DID_0xF18C_SIZE                  , EEPROM_DID_0xF18C_LEN                  , EEPROM_DID_0xF18C_OFFSET                    },
    {EEPROM_DID_0xF190_SIZE                  , EEPROM_DID_0xF190_LEN                  , EEPROM_DID_0xF190_OFFSET                    },
    {EEPROM_DID_0xF1A0_SIZE                  , EEPROM_DID_0xF1A0_LEN                  , EEPROM_DID_0xF1A0_OFFSET                    },
    {EEPROM_DID_0xF1A1_SIZE                  , EEPROM_DID_0xF1A1_LEN                  , EEPROM_DID_0xF1A1_OFFSET                    },
    {EEPROM_DID_Writeable_SIZE               , EEPROM_DID_Writeable_LEN               , EEPROM_DID_Writeable_OFFSET                 },
    {EEPROM_UNLOCKFAILEDCOUNTADDR_SIZE       , EEPROM_UNLOCKFAILEDCOUNTADDR_LEN       , EEPROM_UNLOCKFAILEDCOUNTADDR_OFFSET         },
    {EEPROM_COMCONTROL_NORMALMESSAGEADDR_SIZE, EEPROM_COMCONTROL_NORMALMESSAGEADDR_LEN, EEPROM_COMCONTROL_NORMALMESSAGEADDR_OFFSET  },
    {EEPROM_JUMPTOAPPFAILEDCOUNT_SIZE        , EEPROM_JUMPTOAPPFAILEDCOUNT_LEN        , EEPROM_JUMPTOAPPFAILEDCOUNT_OFFSET          },
    {EEPROM_JUMPTOAPPADDRESS_SIZE            , EEPROM_JUMPTOAPPADDRESS_LEN            , EEPROM_JUMPTOAPPADDRESS_OFFSET              },
    {EEPROM_BACKUPJUMPTOAPPADDRESS_SIZE      , EEPROM_BACKUPJUMPTOAPPADDRESS_LEN      , EEPROM_BACKUPJUMPTOAPPADDRESS_OFFSET        },
};

#endif
