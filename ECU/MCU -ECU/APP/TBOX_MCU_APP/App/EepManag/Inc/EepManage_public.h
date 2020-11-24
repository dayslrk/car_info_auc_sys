#ifndef EEPMANAGE_PUBLIC_H_
#define EEPMANAGE_PUBLIC_H_

//命名规则，第一个字节ECU诊断CANID低字节，后三个字节日期
#define EEPROM_DATA_VERSION                                "\x60\x20\x10\x10"           

#define EEPROM_CURRENT_SESSION_SIZE                       (8U)
#define EEPROM_CURRENT_SESSION_LEN                        (1U)
#define EEPROM_CURRENT_SESSION_OFFSET                     (4U)
#define EEPROM_DID_0xF183_SIZE                            (16U)
#define EEPROM_DID_0xF183_LEN                             (14U)
#define EEPROM_DID_0xF183_OFFSET                          (EEPROM_CURRENT_SESSION_OFFSET + EEPROM_CURRENT_SESSION_SIZE)
#define EEPROM_DID_0xF187_SIZE                            (16U)
#define EEPROM_DID_0xF187_LEN                             (9U)
#define EEPROM_DID_0xF187_OFFSET                          (EEPROM_DID_0xF183_OFFSET + EEPROM_DID_0xF183_SIZE)
#define EEPROM_DID_0xF18C_SIZE                            (24U)
#define EEPROM_DID_0xF18C_LEN                             (20U)
#define EEPROM_DID_0xF18C_OFFSET                          (EEPROM_DID_0xF187_OFFSET + EEPROM_DID_0xF187_SIZE)
#define EEPROM_DID_0xF190_SIZE                            (24U)
#define EEPROM_DID_0xF190_LEN                             (21U)
#define EEPROM_DID_0xF190_OFFSET                          (EEPROM_DID_0xF18C_OFFSET + EEPROM_DID_0xF18C_SIZE)
#define EEPROM_DID_0xF191_SIZE                            (8U)
#define EEPROM_DID_0xF191_LEN                             (8U)
#define EEPROM_DID_0xF191_OFFSET                          (EEPROM_DID_0xF190_OFFSET + EEPROM_DID_0xF190_SIZE)
#define EEPROM_DID_0xF1A0_SIZE                            (16U)
#define EEPROM_DID_0xF1A0_LEN                             (10U)
#define EEPROM_DID_0xF1A0_OFFSET                          (EEPROM_DID_0xF191_OFFSET + EEPROM_DID_0xF191_SIZE)
#define EEPROM_DID_0xF1A1_SIZE                            (16U)
#define EEPROM_DID_0xF1A1_LEN                             (10U)
#define EEPROM_DID_0xF1A1_OFFSET                          (EEPROM_DID_0xF1A0_OFFSET + EEPROM_DID_0xF1A0_SIZE)
#define EEPROM_DID_Writeable_SIZE                         (8U)
#define EEPROM_DID_Writeable_LEN                          (7U)
#define EEPROM_DID_Writeable_OFFSET                       (EEPROM_DID_0xF1A1_OFFSET + EEPROM_DID_0xF1A1_SIZE)
#define EEPROM_UNLOCKFAILEDCOUNTADDR_SIZE                 (1U)
#define EEPROM_UNLOCKFAILEDCOUNTADDR_LEN                  (1U)
#define EEPROM_UNLOCKFAILEDCOUNTADDR_OFFSET               (EEPROM_DID_Writeable_OFFSET + EEPROM_DID_Writeable_SIZE)
#define EEPROM_COMCONTROL_NORMALMESSAGEADDR_SIZE          (1U)
#define EEPROM_COMCONTROL_NORMALMESSAGEADDR_LEN           (1U)
#define EEPROM_COMCONTROL_NORMALMESSAGEADDR_OFFSET        (EEPROM_UNLOCKFAILEDCOUNTADDR_OFFSET + EEPROM_UNLOCKFAILEDCOUNTADDR_SIZE)
#define EEPROM_JUMPTOAPPFAILEDCOUNT_SIZE                  (1U)
#define EEPROM_JUMPTOAPPFAILEDCOUNT_LEN                   (1U)
#define EEPROM_JUMPTOAPPFAILEDCOUNT_OFFSET                (EEPROM_COMCONTROL_NORMALMESSAGEADDR_OFFSET + EEPROM_COMCONTROL_NORMALMESSAGEADDR_SIZE)

#define EEPROM_SELFFLASHING_SIZE                          (4U)
#define EEPROM_SELFFLASHING_LEN                           (4U)
#define EEPROM_SELFFLASHING_OFFSET                        (EEPROM_JUMPTOAPPFAILEDCOUNT_OFFSET + EEPROM_JUMPTOAPPFAILEDCOUNT_SIZE)

#define EEPROM_JUMPTOAPPADDRESS_SIZE                      (8U)
#define EEPROM_JUMPTOAPPADDRESS_LEN                       (4U)
#define EEPROM_JUMPTOAPPADDRESS_OFFSET                    (EEPROM_SELFFLASHING_OFFSET + EEPROM_SELFFLASHING_SIZE)

#define EEPROM_BACKUPJUMPTOAPPADDRESS_SIZE                (8U)
#define EEPROM_BACKUPJUMPTOAPPADDRESS_LEN                 (4U)
#define EEPROM_BACKUPJUMPTOAPPADDRESS_OFFSET              (EEPROM_JUMPTOAPPADDRESS_OFFSET + EEPROM_JUMPTOAPPADDRESS_SIZE)


typedef enum
{
    EEPROM_CURRENT_SESSION,
    EEPROM_DID_0xF183,
    EEPROM_DID_0xF187,
    EEPROM_DID_0xF18C,
    EEPROM_DID_0xF190,
    EEPROM_DID_0xF191,
    EEPROM_DID_0xF1A0,
    EEPROM_DID_0xF1A1,
    EEPROM_DID_Writeable,
    EEPROM_UNLOCKFAILEDCOUNTADDR,
    EEPROM_COMCONTROL_NORMALMESSAGEADDR,
    EEPROM_JUMPTOAPPFAILEDCOUNT,

   
    EEPROM_SELFFLASHING,

    EEPROM_JUMPTOAPPADDRESS,
    EEPROM_BACKUPJUMPTOAPPADDRESS,
    EEPROM_INDEXMAX,
}Eeprom_Index_EU;

bool Eeprom_Write(Eeprom_Index_EU Index,u8 *pBuffer,u16 Len);
bool Eeprom_Read(Eeprom_Index_EU Index,u8 *pBuffer,u16 Len);

void Eeprom_Init( );

#endif

