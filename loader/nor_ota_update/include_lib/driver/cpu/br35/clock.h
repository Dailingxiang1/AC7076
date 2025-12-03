#ifndef  __CLOCK_H__
#define  __CLOCK_H__

#include "typedef.h"


#define MHz (1000000L)

#define OSC_FREQ    24000000

#define SYS_CLK     48000000


enum {
    UART_CLOCK_IN_DISABLE = 0,
    UART_CLOCK_IN_STD48M,
    UART_CLOCK_IN_STD24M,
    UART_CLOCK_IN_EXT,
    UART_CLOCK_IN_LSB,
};
#define UART_CLOCK_IN(x)        SFR(JL_LSBCLK->PRP_CON0,  12,  3,  x)

#define BT_CLOCK_IN(x)          //SFR(JL_CLOCK->CLK_CON1,  14,  2,  x)
//for MACRO - BT_CLOCK_IN
enum {
    BT_CLOCK_IN_PLL48M = 0,
    BT_CLOCK_IN_HSB,
    BT_CLOCK_IN_LSB,
    BT_CLOCK_IN_DISABLE,
};

void lrc_init(void);

void sys_clk_init(u32 osc_freq, u32 sys_clk);

void btosc_upgrade_cfg(void *cfg);

void sys_clk_reinit(void *clk_argv);

u32 clk_get(const char *clk);


#define CLR_DVDD_POR_PND()		JL_HSBCLK->PWR_CON |= BIT(11)
#endif

