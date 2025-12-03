#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".audio_general.data.bss")
#pragma data_seg(".audio_general.data")
#pragma const_seg(".audio_general.text.const")
#pragma code_seg(".audio_general.text")
#endif

#include "system/init.h"
#include "media/audio_general.h"
#include "app_config.h"
#include "media/audio_def.h"
#include "jlstream.h"
#include "uartPcmSender.h"
#include "debug/audio_debug.h"
#include "audio_config_def.h"

/*音频配置在线调试配置*/
const int config_audio_cfg_debug_online = TCFG_CFG_TOOL_ENABLE;

#if TCFG_USER_TWS_ENABLE
const int config_media_tws_en = 1;
#else
const int config_media_tws_en = 0;
#endif

#ifdef AUDIO_LIMITER_RUN_MODE
const int limiter_run_mode = AUDIO_LIMITER_RUN_MODE;
#else
const int limiter_run_mode = 0xFFFF;
#endif

const int config_audio_eq_hp_enable = 1;		//High Pass
const int config_audio_eq_lp_enable = 1;		//Low Pass
const int config_audio_eq_bp_enable = 1;		//Band Pass(Peaking)
const int config_audio_eq_hs_enable = 1;		//High Shelf
const int config_audio_eq_ls_enable = 1;		//Low Shelf
const int config_audio_eq_hs_q_enable = 0;		//High Shelf Q
const int config_audio_eq_ls_q_enable = 0;		//Low Shelf Q
const int config_audio_eq_hp_adv_enable = 0;	//High Pass Advance：对应工具上阶数可选的Hp
const int config_audio_eq_lp_adv_enable = 0;	//Low Pass Advance：对应工具上阶数可选的Lp

const int config_audio_dac_dma_buf_realloc_enable = 1;


/*
 *******************************************************************
 *						Audio SYNCTS Config
 *******************************************************************
 */

const float FRAME_DURATION_THREAD = 1.5f;	//范围1.5f~2,采样率和时间戳抖动阈值倍数(丢帧检测阈值,时间戳间隔超过1.5帧，判定丢帧)

/*
 *******************************************************************
 *						Audio Mic Capless Config
 *******************************************************************
 */
/*
 *          用于debug需要配置多长的mic_capless_delay
 * 部分mic 需要更多延时才能达到稳定的电压值
 * debug步骤：
 * 1、打开 const_mic_capless_open_delay_debug，可打印打开adc后，mic_bias需要多长时间才能达到/接近稳定的电压值
 * 2、将该延时配置到 open_delay_ms 中
 * 3、关闭 const_mic_capless_open_delay_debug

 * 4、打开 const_mic_capless_trim_delay_debug，可打印第一次trim后，mic_bias需要多长时间才能达到/接近稳定的电压值
 * 5、将该延时配置到 trim_delay_ms 中
 * 6、关闭 const_mic_capless_trim_delay_debug
 */
const u8 const_mic_capless_open_delay_debug = 0;
const u8 const_mic_capless_trim_delay_debug = 0;

__attribute__((weak))
int get_system_stream_bit_width(void *par)
{
    return 0;
}
__attribute__((weak))
int get_media_stream_bit_width(void *par)
{
    return 0;
}
__attribute__((weak))
int get_esco_stream_bit_width(void *par)
{
    return 0;
}
__attribute__((weak))
int get_mic_eff_stream_bit_width(void *par)
{
    return 0;
}

static struct audio_general_params audio_general_param = {0};

struct audio_general_params *audio_general_get_param(void)
{
    return &audio_general_param;
}
/*
 *输出设备硬件位宽
 * */
int audio_general_out_dev_bit_width()
{
    if (audio_general_param.system_bit_width || audio_general_param.media_bit_width ||
        audio_general_param.esco_bit_width || audio_general_param.mic_bit_width) {
        return DATA_BIT_WIDE_24BIT;
    }
    return DATA_BIT_WIDE_16BIT;
}
/*
 *输入设备硬件位宽
 * */
int audio_general_in_dev_bit_width()
{
    if (audio_general_param.media_bit_width || audio_general_param.mic_bit_width) {
        return DATA_BIT_WIDE_24BIT;
    }
    return DATA_BIT_WIDE_16BIT;
}

int audio_general_init()
{
#if ((defined TCFG_AUDIO_DATA_EXPORT_DEFINE) && (TCFG_AUDIO_DATA_EXPORT_DEFINE == AUDIO_DATA_EXPORT_VIA_UART))
    /* uartSendInit(); */
#endif/*TCFG_AUDIO_DATA_EXPORT_DEFINE*/

#if TCFG_AUDIO_CONFIG_TRACE
    audio_config_trace_setup(TCFG_AUDIO_CONFIG_TRACE_INTERVAL);
#endif/*TCFG_AUDIO_CONFIG_TRACE*/

#if MEDIA_24BIT_ENABLE
    struct stream_bit_width stream_par = {0};
    if (get_system_stream_bit_width(&stream_par)) {
        audio_general_param.system_bit_width = stream_par.bit_width;
    }

    if (get_media_stream_bit_width(&stream_par)) {
        audio_general_param.media_bit_width = stream_par.bit_width;
    }

    if (get_esco_stream_bit_width(&stream_par)) {
        audio_general_param.esco_bit_width = stream_par.bit_width;
    }

    if (get_mic_eff_stream_bit_width(&stream_par)) {
        audio_general_param.mic_bit_width = stream_par.bit_width;
    }
#endif

#if defined(TCFG_AUDIO_GLOBAL_SAMPLE_RATE) &&TCFG_AUDIO_GLOBAL_SAMPLE_RATE
    audio_general_param.sample_rate = TCFG_AUDIO_GLOBAL_SAMPLE_RATE;
#endif
    return 0;
}
