#ifndef  __TIMER_H__
#define  __TIMER_H__


#include "typedef.h"


u32 tmr_2ms_cnt_get(void);

void sys_tmr_init(void (*cb)(void));

void sys_tmr_close(void);



#endif

