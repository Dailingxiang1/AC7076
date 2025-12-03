#ifndef  __CRC_H__
#define  __CRC_H__

#include "typedef.h"


u16 chip_crc16(void *ptr, u32 len);

u16 chip_crc16_with_init(void *ptr, u32 len, u16 init);

void CrcDecode(void  *buf, u16 len);

void crc_encode(void *buf, u32 len, const u8 *crckey, u32 offset);


#define CRC16(x,y)  chip_crc16(x,y)



#endif

