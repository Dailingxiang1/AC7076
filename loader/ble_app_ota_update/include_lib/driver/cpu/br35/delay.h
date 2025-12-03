#ifndef __DELAY_H__
#define __DELAY_H__

#include "typedef.h"

void udelay(u32 usec);

void mdelay(u32 msec);

void delay(volatile u32 t);

void rc_udelay(u32 usec);

void rc_mdelay(u32 msec);


#endif
