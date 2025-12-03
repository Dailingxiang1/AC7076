#include "includes.h"
#include "timer.h"

void clock_early_init()
{
    if (M2P_LRC24M_MODE) {
        //P11_SYS_CLK_SEL(P11_SYS_CLK_BTOSC_24M);
        //P11_SYS_CLK_SEL(P11_SYS_CLK_LRC24M);
        P11_SYS_CLK_SEL(P11_SYS_CLK_RC16M);

#if 0
        //PB0 OUT
        P11_PORT->PB_DIR &= ~BIT(0);
        P11_PORT->PB_SEL |=  BIT(0);
        //och0 sysclk
        SFR(P11_PORT->OCH_CON0, 0, 4, 7);
        //crossbar och0
        P11_OMAP->P11_PB0_OUT = P11_FO_GP_OCH0;

        while (1);
#endif
    }

    //std12M 输出
    SFR(P11_CLOCK->CLK_CON1, 15, 2, 1); //MSYS_BT24M --> P11_BT24M
    SFR(P11_CLOCK->CLK_CON1, 17, 3, 1); //STD12M sel P11_BT24M input
    SFR(P11_CLOCK->CLK_CON1, 20, 2, 0); // div0
    SFR(P11_CLOCK->CLK_CON1, 22, 2, 1); // div1
}

static void clock_switch_lrc24m()
{
    printf("fun: %s\n", __FUNCTION__);

    P11_SYS_CLK_SEL(P11_SYS_CLK_LRC24M);

    uart_clk_sel(UART_CLK_SYS_CLK);
}

static void clock_msg_handler(void *priv, u8 *msg, u32 len)
{
    switch (msg[0]) {
    case MSG_CLOCK_LRC24M_OK:
        clock_switch_lrc24m();
        timer_set_clock_source(GPTIMER_CLK_SRC_LRC_24M);
        break;
    }
}

REGISTER_M2P_MSG_HANDLER(0, MSG_CLOCK, clock_msg_handler);
