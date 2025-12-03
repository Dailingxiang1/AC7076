#ifndef _GPCNT_H
#define _GPCNT_H

#include "typedef.h"


enum gpcnt_gss_css_src {
    GPCNT_SRC_SYSPLL_D1P0 =  1,
    GPCNT_SRC_RINGCLK    =  2,
    GPCNT_SRC_RC16M      =  3,
    GPCNT_SRC_RC250K     =  4,
    GPCNT_SRC_LRC_TRC    =  5,
    GPCNT_SRC_PATCLK     =  6,
    GPCNT_SRC_LRC24M     =  7,
    GPCNT_SRC_BTOSC24M   =  8,
    GPCNT_SRC_BTOSC48M   =  9,
    GPCNT_SRC_P33_DBG    = 10,
    GPCNT_SRC_HSBCLK     = 11,
    GPCNT_SRC_LSBCLK     = 12,
    GPCNT_SRC_PLL96M     = 13,
    GPCNT_SRC_STD48M     = 14,
    GPCNT_SRC_STD24M     = 15,
    GPCNT_SRC_ASS_DBG    = 16,
    GPCNT_SRC_P11_DBG    = 17,
    GPCNT_SRC_WATCLK     = 18
};

#define     CSS_CLK_SRC GPCNT_SRC_PLL96M
#define     CSS_CLK_FREQ    96000000

int gpcnt_trim_early_init();

void gpcnt_trim_cfg(u32 gss_ck, u32 css_ck, u32 gts_mul);
u32 gpcnt_trim_wait_pnd(u32 sync);
u32 gpcnt_trim_num2freq(u32 css_num, u32 gts_mul, u32 css_freq);
u32 gpcnt_trim_freq(u32 gss_ck, u32 css_ck, u32 gts_mul, u32 css_freq);

u32 gpcnt_trim_num2cssfreq(u32 css_num, u32 gts_mul, u32 gss_freq);
u32 gpcnt_trim_cssfreq(u32 gss_ck, u32 css_ck, u32 gts_mul, u32 gss_freq);


// avg 实际返回的是中心值，不是算术平均值
u32 rc250k_trim_get_avg();
u32 rc250k_trim_get_latest();
void rc250k_trim_dump();
u32 rc250k_trim_get_drift();
u32 rc250k_trim_get_high();
u32 rc250k_trim_get_low();

u32 rc16m_trim_get_avg();
u32 rc16m_trim_get_latest();
void rc16m_trim_dump();
u32 rc16m_trim_get_drift();
u32 rc16m_trim_get_high();
u32 rc16m_trim_get_low();

u32 lrc24m_trim_get_avg();
u32 lrc24m_trim_get_latest();
void lrc24m_trim_dump();
u32 lrc24m_trim_get_drift();
u32 lrc24m_trim_get_high();
u32 lrc24m_trim_get_low();
void lrc24m_init(u32 ftis, u32 ftcs, u32 ftrs);

u32 lrc200k_trim_get_avg();
u32 lrc200k_trim_get_center();
u32 lrc200k_trim_get_latest();
void lrc200k_trim_dump();
u32 lrc200k_trim_get_drift();
u32 lrc200k_trim_get_high();
u32 lrc200k_trim_get_low();

void m2p_lrc_update(u32 osc_hz);

#endif

