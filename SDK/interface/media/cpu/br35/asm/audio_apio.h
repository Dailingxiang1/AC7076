#ifndef _AUDIO_APIO_H_
#define _AUDIO_APIO_H_

#include "generic/typedef.h"
#include "generic/atomic.h"
#include "os/os_api.h"
#include "circular_buf.h"

#define AUDIO_APIO_DSM_CLK_384K     0
#define AUDIO_APIO_DSM_CLK_768K     1

#define AUDIO_APIO_PWM_MODE0        0
#define AUDIO_APIO_PWM_MODE1        1
#define AUDIO_APIO_PWM_MODE2        2

/************************************
             apio性能模式
************************************/
// TCFG_AUDIO_APIO_PERFORMANCE_MODE
#define	APIO_MODE_HIGH_PERFORMANCE      (0)
#define	APIO_MODE_LOW_POWER		        (1)


struct apio_platform_data {
    u8 dcc_level;
    u8 pwm_mode;
    u8 pwm_mute;
    u8 dsm_clk_mode;
    u8 fade_en;
    u8 fade_slow;
    u8 fade_fast;
    u8 pwm_clk_mode;
    u8 classd_vbg_value;
    u8 apa_en;					// APA模块使能
    u8 apio_en;					// APIO模块使能
    u16 syspll_clk_mode;

};

struct audio_apio_hdl {
    struct apio_platform_data *pd;
    s16 *output_buf;
    u16 output_buf_len;
    s16 *fifo;
    cbuffer_t cbuf;
    u32 sample_rate;
    volatile u8 state;              /*apio运行状态*/
    u8 power_on;
    OS_MUTEX mutex;
};

enum CLASSD_GPIO_STATE {
    CLASSD_OUTPUT_LOGIC0 = 0,
    CLASSD_OUTPUT_LOGIC1,
    CLASSD_OUTPUT_HIGHZ,
};

enum CLASSD_GPIO_PWM_SEL {
    CLASSD_MCPWM0_H = 29,
    CLASSD_MCPWM1_H,
    CLASSD_MCPWM0_L,
    CLASSD_MCPWM1_L,
};
void audio_apio_irq_handler(struct audio_apio_hdl *apio);
int audio_apio_init(struct audio_apio_hdl *apio, struct apio_platform_data *pd);
int audio_apio_set_sample_rate(struct audio_apio_hdl *apio, int sample_rate);
int audio_apio_get_sample_rate(struct audio_apio_hdl *apio);
int audio_apio_try_power_on(struct audio_apio_hdl *apio);
int audio_apio_start(struct audio_apio_hdl *apio);
int audio_apio_write(struct audio_apio_hdl *apio, void *buf, int len);
int audio_apio_close(struct audio_apio_hdl *apio);

// -- CLASSD IO FUNCTION -- //
// param: *apio     -- apio_hdl地址
//        classd_ch -- 操作的CLASSD通道(CLASSD_IO_OUTPUT_P or CLASSD_IO_OUTPUT_N or (CLASSD_IO_OUTPUT_P | CLASSD_IO_OUTPUT_N))
//        io_state  -- 设置IO状态(enum CLASSD_GPIO_STATE)
// ----------------------- //
void audio_classd_io_output_open(struct audio_apio_hdl *apio, u8 classd_ch);
void audio_classd_io_output_set(struct audio_apio_hdl *apio, u8 classd_ch, enum CLASSD_GPIO_STATE io_state);
// -- CLASSD IO PWM FUNCTION -- //
//注意：在该模式时，要将对应模块先行配置起来!!!
// param: *apio       -- apio_hdl地址
//        classd_ch   -- 操作的CLASSD通道(CLASSD_IO_OUTPUT_P or CLASSD_IO_OUTPUT_N or (CLASSD_IO_OUTPUT_P | CLASSD_IO_OUTPUT_N))
//        pwm_src_sel -- 设置PWM信号源: //apa_och1_sel, 0~7: gp_och0~7	8~13: sd0	14~18: spi1		19~23: spi2		24~25: iic0		26~28: uart0	29~32: mcpwm0	33~36:scan chain
// ----------------------- //
void audio_classd_io_output_set_pwm(struct audio_apio_hdl *apio, u8  classd_ch, enum CLASSD_GPIO_PWM_SEL  pwm_src_sel);
void audio_classd_io_output_close(struct audio_apio_hdl *apio);
#endif
