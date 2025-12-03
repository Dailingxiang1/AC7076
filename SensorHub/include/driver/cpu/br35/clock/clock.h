#ifndef __CLOCK_H__
#define __CLOCK_H__

void clock_early_init();
u32 lrc_get_avg_freq(void);

enum {
    MSG_CLOCK_LRC24M_OK = 1,
};



#endif
