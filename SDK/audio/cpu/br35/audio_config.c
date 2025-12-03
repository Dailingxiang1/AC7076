/*
 ******************************************************************************************
 *							Audio Config
 *
 * Discription: 音频模块与芯片系列相关配置
 *
 * Notes:
 ******************************************************************************************
 */
#include "cpu/includes.h"
#include "media/includes.h"
#include "system/includes.h"
#include "audio_config.h"
#include "clock_manager/clock_manager.h"

const int config_classd_output_mode = TCFG_AUDIO_CLASSD_OUTPUT;
const int config_audio_classd_power_off_enable = 1;
const int config_audio_classd_light_power_off_enable = 0;	//轻量级关闭

const int config_audio_adc_performance_mode = TCFG_ADC_PERFORMANCE_MODE;
//ADC输入模式配置
const int config_audio_adc_input_mode = TCFG_ADC0_MODE;
const int config_audio_adc_input_port = TCFG_ADC0_AIN_SEL;

/*
 *******************************************************************
 *						Audio Codec Config
 *******************************************************************
 */
const int config_audio_dac_mix_enable = 1;

//***********************
//*		AAC Codec       *
//***********************
const int AAC_DEC_MP4A_LATM_ANALYSIS = 1;
const int AAC_DEC_LIB_SUPPORT_24BIT_OUTPUT = 0;
const int WTS_DEC_LIB_SUPPORT_24BIT_OUTPUT = 1;

//***********************
//*		Audio Params    *
//***********************
void audio_adc_param_fill(struct mic_open_param *mic_param, struct adc_platform_cfg *platform_cfg)
{
    mic_param->mic_mode      = platform_cfg->mic_mode;
    mic_param->mic_ain_sel   = platform_cfg->mic_ain_sel;
    mic_param->mic_bias_sel  = platform_cfg->mic_bias_sel;
    if (mic_param->mic_mode == AUDIO_MIC_CAPLESS_MODE) {
        mic_param->mic_bias_rsel = adc_hdl.capless_trim.bias_rsel;
    } else {
        mic_param->mic_bias_rsel = platform_cfg->mic_bias_rsel;
    }
    mic_param->mic_dcc       = platform_cfg->mic_dcc;
}

void audio_linein_param_fill(struct linein_open_param *linein_param, const struct adc_platform_cfg *platform_cfg)
{
    linein_param->linein_mode    = platform_cfg->mic_mode;
    linein_param->linein_ain_sel = platform_cfg->mic_ain_sel;
    linein_param->linein_dcc     = platform_cfg->mic_dcc;
}

//***********************
//*		Audio Clock     *
//***********************
int aud_clock_alloc(const char *name, u32 clk)
{
    y_printf("aud_clock_alloc:%s(%d)\n", name, clk);
    return clock_alloc(name, clk);
}

int aud_clock_free(char *name)
{
    y_printf("aud_clock_alloc:%s\n", name);
    return clock_free(name);

}
