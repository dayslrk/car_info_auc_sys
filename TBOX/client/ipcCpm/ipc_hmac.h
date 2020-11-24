#ifndef IPC_HMAC_H
#define IPC_HMAC_H

#include "common.h"

u32 ipc_send( u8 *pReq, const u32 req_len );


u32 ipc_read(u8 *pRes, u32 *pres_len);


#endif /* IPC_HMAC_H */

