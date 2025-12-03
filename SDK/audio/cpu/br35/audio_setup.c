/*
 ******************************************************************
 *					      Audio Setup
 *
 * Discription: 音频模块初始化，配置，调试等
 *
 * Notes:
 ******************************************************************
 */
#include "cpu/includes.h"
#include "media/includes.h"
#include "system/includes.h"
#include "app_config.h"
#include "audio_config.h"
#include "asm/audio_adc.h"
#include "asm/audio_apio.h"
#include "media/audio_energy_detect.h"
#include "adc_file.h"
#include "linein_file.h"
#include "asm/audio_common.h"
#include "gpio_config.h"
#include "audio_demo/audio_demo.h"
#include "gpadc.h"
#include "update.h"
#include "media/audio_general.h"

#if (SYS_VOL_TYPE == VOL_TYPE_DIGITAL)
#include "audio_dvol.h"
#endif

#if TCFG_AUDIO_DUT_ENABLE
#include "audio_dut_control.h"
#endif/*TCFG_AUDIO_DUT_ENABLE*/

#if TCFG_SMART_VOICE_ENABLE
#include "smart_voice.h"
#include "user_asr.h"
#endif /*TCFG_SMART_VOICE_ENABLE*/

struct audio_dac_hdl dac_hdl;
struct audio_adc_hdl adc_hdl;
struct audio_apio_hdl apio_hdl;

typedef struct {
    u8 audio_inited;
    /* atomic_t ref; */
} audio_setup_t;
audio_setup_t audio_setup = {0};
#define __this      (&audio_setup)


void audio_fade_in_fade_out(u8 left_vol, u8 right_vol);
extern u32 read_capless_DTB(void);
extern struct dac_platform_data dac_data;
extern struct apio_platform_data apio_data;

int get_dac_channel_num(void)
{
    return dac_hdl.channel;
}

static u8 audio_dac_hpvdd_check()
{
#if 0
    u8 res;
    u16 hpvdd = adc_get_voltage_blocking(AD_CH_AUDIO_HPVDD);
    /* printf("HPVDD: %d\n", hpvdd); */
    if (hpvdd > 1500) {
        res = DAC_HPVDD_18V;
        puts("DAC_HPVDD: 1.8V");
    } else {
        res = DAC_HPVDD_12V;
        puts("DAC_HPVDD: 1.2V");
    }
    SFR(JL_ADDA->ADDA_CON0,  0,  1,  0);					// AUDIO到SARADC的总测试通道使能
    SFR(JL_ADDA->ADDA_CON0,  3,  1,  0);					// DAC测试通道总使能
    SFR(JL_ADDA->ADDA_CON0,  4,  3,  0);					// DAC待测试信号选择位
    return res;
#else
    return 0;
#endif
}

/*
 * DAC开关状态回调，需要的时候，"#if 0" 改成 "#if 1"
 */
#if 0
void audio_dac_power_state(u8 state)
{
    switch (state) {
    case DAC_ANALOG_OPEN_PREPARE:
        //DAC打开前，即准备打开
        printf("DAC power_on start\n");
        break;
    case DAC_ANALOG_OPEN_FINISH:
        //DAC打开后，即打开完成
        printf("DAC power_on end\n");
        break;
    case DAC_ANALOG_CLOSE_PREPARE:
        //DAC关闭前，即准备关闭
        printf("DAC power_off start\n");
        break;
    case DAC_ANALOG_CLOSE_FINISH:
        //DAC关闭后，即关闭完成
        printf("DAC power_off end\n");
        break;
    case DAC_ANALOG_START:
        //DAC开始播放，硬件已经打开
        printf("DAC start\n");
        break;
    case DAC_ANALOG_STOP:
        //DAC停止播放，硬件还未关闭
        printf("DAC stop\n");
        break;
    }
}
#endif

static void wl_audio_clk_on(void)
{
    // ... ass_clk select ...
    SFR(JL_LSBCLK->PRP_CON2, 0, 2, 1);
    SFR(JL_LSBCLK->PRP_CON2, 2, 2, 2);
    SFR(JL_LSBCLK->PRP_CON2, 4, 2, 2);
    SFR(JL_LSBCLK->PRP_CON2, 8, 2, 1);
    udelay(100);
    SFR(JL_WL_AUD->CON0, 0, 1, 1);
    SFR(JL_LSBCLK->PRP_CON1,  0,  2,  1);   // ass_clk=?, 0: 0      1: std_48m      2: hsb_clk      3: pat_clk
    SFR(JL_LSBCLK->PRP_CON1, 26,  1,  1);   // ass_reg_rstn
    udelay(100);    // must delay !!!
    // ... audio clk enable ...
    SFR(JL_AUD->AUD_CON0,  0,  1,  1);      // aud_cken
}
void audio_dac_initcall(void)
{
    printf("audio_dac_initcall\n");


#if (SYS_VOL_TYPE == VOL_TYPE_DIGITAL)
    audio_digital_vol_init(NULL, 0);
#endif
    dac_data.max_sample_rate    = AUDIO_DAC_MAX_SAMPLE_RATE;
    //dac_data.hpvdd_sel = audio_dac_hpvdd_check();
    dac_data.bit_width = audio_general_out_dev_bit_width();

    apio_data.syspll_clk_mode = clk_get("bt_pll") / 1000000;
    apio_hdl.pd = &apio_data;
    audio_dac_init(&dac_hdl, &dac_data);

    /* u8 mode = TCFG_AUDIO_DAC_DEFAULT_VOL_MODE; */
    /* if (1 != syscfg_read(CFG_VOLUME_ENHANCEMENT_MODE, &mode, 1)) { */
    /* printf("vm no CFG_VOLUME_ENHANCEMENT_MODE !\n"); */
    /* } */
    /* printf("enter audio_init.c %d,%d\n",mode,__LINE__); */
    //app_audio_dac_vol_mode_set(mode);

#if defined(TCFG_AUDIO_DAC_24BIT_MODE) && TCFG_AUDIO_DAC_24BIT_MODE
    audio_dac_set_bit_mode(&dac_hdl, 1);
#endif


    audio_dac_set_analog_vol(&dac_hdl, 0);

#if AUD_DAC_TRIM_ENABLE
    struct audio_dac_trim dac_trim = {0};
    int len = syscfg_read(CFG_DAC_TRIM_INFO, (void *)&dac_trim, sizeof(dac_trim));
    if (len != sizeof(dac_trim)) {
        struct trim_init_param_t trim_init = {0};
        trim_init.precision = 1; //DAC trim的收敛精度(-precision, +precision)
        audio_dac_do_trim(&dac_hdl, &dac_trim, &trim_init);
        syscfg_write(CFG_DAC_TRIM_INFO, (void *)&dac_trim, sizeof(dac_trim));
    }
    audio_dac_set_trim_value(&dac_hdl, &dac_trim);
#endif

    audio_dac_set_fade_handler(&dac_hdl, NULL, audio_fade_in_fade_out);

    /*硬件SRC模块滤波器buffer设置，可根据最大使用数量设置整体buffer*/
    /* audio_src_base_filt_init(audio_src_hw_filt, sizeof(audio_src_hw_filt)); */

#if AUDIO_OUTPUT_AUTOMUTE
    mix_out_automute_open();
#endif  //#if AUDIO_OUTPUT_AUTOMUTE
}

static u8 audio_init_complete()
{
    if (!__this->audio_inited) {
        return 0;
    }
    return 1;
}

REGISTER_LP_TARGET(audio_init_lp_target) = {
    .name = "audio_init",
    .is_idle = audio_init_complete,
};

extern void dac_analog_power_control(u8 en);
void audio_fast_mode_test()
{
    audio_dac_set_volume(&dac_hdl, app_audio_get_volume(APP_AUDIO_CURRENT_STATE));
    /* dac_analog_power_control(1);////将关闭基带，不开可发现，不可连接 */
    audio_dac_start(&dac_hdl);
    audio_adc_mic_demo_open(AUDIO_ADC_MIC_CH, 10, 16000, 1);

}

struct audio_adc_private_param adc_private_param = {
    .mic_ldo_vsel   = TCFG_AUDIO_MIC_LDO_VSEL,
    .performance_mode = TCFG_ADC_PERFORMANCE_MODE,
};

#if TCFG_AUDIO_ADC_ENABLE
const struct adc_platform_cfg adc_platform_cfg_table[AUDIO_ADC_MAX_NUM] = {
#if TCFG_ADC0_ENABLE
    [0] = {
        .mic_mode           = TCFG_ADC0_MODE,
        .mic_ain_sel        = TCFG_ADC0_AIN_SEL,
        .mic_bias_sel       = TCFG_ADC0_BIAS_SEL,
        .mic_bias_rsel      = TCFG_ADC0_BIAS_RSEL,
        .power_io           = TCFG_ADC0_POWER_IO,
        .mic_dcc            = TCFG_ADC0_DCC_LEVEL,
    },
#endif
};
#endif
void audio_input_initcall(void)
{
    printf("audio_input_initcall\n");

    audio_adc_init(&adc_hdl, &adc_private_param);
    adc_hdl.bit_width = audio_general_in_dev_bit_width();
    audio_adc_file_init();

#if TCFG_SUPPORT_MIC_CAPLESS
    adc_hdl.capless_trim.bias_rsel = 8;
    adc_hdl.capless_param.trigger_threshold = 50; //电压差校准触发阈值(单位:mV)
    /*
     * 以下两个delay需要根据
     * const_mic_capless_open_delay_debug、const_mic_capless_trim_delay_debug 的结果配置
     */
    adc_hdl.capless_param.open_delay_ms = 150; //adc上电等待稳定延时
    adc_hdl.capless_param.trim_delay_ms = 50; //偏置调整等待稳定延时
    int ret = 0;
    struct adc_platform_cfg *platform_cfg = audio_adc_platform_get_cfg();
    if (platform_cfg != NULL) {
        for (u8 i = 0; i < AUDIO_ADC_MAX_NUM; i++) {
            audio_adc_param_fill(&adc_hdl.mic_param[i], &platform_cfg[i]);
        }
    }
#if (TCFG_MC_BIAS_AUTO_ADJUST == MC_BIAS_ADJUST_ONE)
    int len = syscfg_read(CFG_MC_BIAS, &adc_hdl.capless_trim, sizeof(struct mic_capless_trim_result));
    if (len != sizeof(struct mic_capless_trim_result)) {
        ret = audio_mic_bias_adjust(&adc_hdl.capless_trim, &adc_hdl.capless_param);
        if (ret == 0) {
            syscfg_write(CFG_MC_BIAS, &adc_hdl.capless_trim, sizeof(struct mic_capless_trim_result));
        }
    }
#elif (TCFG_MC_BIAS_AUTO_ADJUST == MC_BIAS_ADJUST_ALWAYS)
    ret = audio_mic_bias_adjust(&adc_hdl.capless_trim, &adc_hdl.capless_param);
    if (ret == 0) {
        syscfg_write(CFG_MC_BIAS, &adc_hdl.capless_trim, sizeof(struct mic_capless_trim_result));
    }
#endif
    printf("BIAS_RSEL: %d\n", adc_hdl.capless_trim.bias_rsel);
#endif
#if TCFG_AUDIO_DUT_ENABLE
    audio_dut_init();
#endif /*TCFG_AUDIO_DUT_ENABLE*/

#if TCFG_AUDIO_LINEIN_ENABLE
    audio_linein_file_init();
#endif/*TCFG_AUDIO_LINEIN_ENABLE*/
}

/*
 *MIC电源管理
 *设置mic vdd对应port的状态
 */
void audio_mic_pwr_ctl(audio_mic_pwr_t state)
{
    int i;
    struct adc_file_cfg *cfg = audio_adc_file_get_cfg();
    struct adc_platform_cfg *platform_cfg = audio_adc_platform_get_cfg();
    switch (state) {
    case MIC_PWR_OFF:
        if (audio_adc_is_active()) {
            printf("MIC_IO_PWR AUDIO_ADC is useing !\n");
            return;
        }
        printf("MIC_IO_PWR close\n");
        break;
    case MIC_PWR_INIT:
        /*mic供电IO配置：输出0*/
        for (i = 0; i < AUDIO_ADC_MIC_MAX_NUM; i++) {
            if (cfg->mic_en_map & BIT(i)) {
                if ((platform_cfg[i].mic_bias_sel == 0) && (platform_cfg[i].power_io != 0)) {
                    u32 gpio = uuid2gpio(platform_cfg[i].power_io);
                    gpio_set_mode(IO_PORT_SPILT(gpio), PORT_OUTPUT_LOW);
                }
            }
        }

#if (defined TCFG_AUDIO_PLNK_PWR_PORT) && (TCFG_AUDIO_PLNK_PWR_PORT != NO_CONFIG_PORT)
        gpio_set_mode(IO_PORT_SPILT(TCFG_AUDIO_PLNK_PWR_PORT), PORT_OUTPUT_LOW);
#endif
        /*
        //测试发现 少数MIC在开机的时候，需要将MIC_IN拉低，否则工作异常
        gpio_set_mode(IO_PORT_SPILT(IO_PORTA_01), PORT_OUTPUT_LOW);	//MIC0
        gpio_set_mode(IO_PORT_SPILT(IO_PORTA_03), PORT_OUTPUT_LOW);	//MIC1
        gpio_set_mode(IO_PORT_SPILT(IO_PORTG_08), PORT_OUTPUT_LOW);	//MIC2
        gpio_set_mode(IO_PORT_SPILT(IO_PORTG_06), PORT_OUTPUT_LOW);	//MIC3
         */
        break;

    case MIC_PWR_ON:
        /*mic供电IO配置：输出1*/
        for (i = 0; i < AUDIO_ADC_MIC_MAX_NUM; i++) {
            if (cfg->mic_en_map & BIT(i)) {
                if ((platform_cfg[i].mic_bias_sel == 0) && (platform_cfg[i].power_io != 0)) {
                    u32 gpio = uuid2gpio(platform_cfg[i].power_io);
                    gpio_set_mode(IO_PORT_SPILT(gpio), PORT_OUTPUT_HIGH);
                }
            }
        }
#if (defined TCFG_AUDIO_PLNK_PWR_PORT) && (TCFG_AUDIO_PLNK_PWR_PORT != NO_CONFIG_PORT)
        gpio_set_mode(IO_PORT_SPILT(TCFG_AUDIO_PLNK_PWR_PORT), PORT_OUTPUT_HIGH);
#endif
        break;

    case MIC_PWR_DOWN:
        break;
    }
}


#ifndef TCFG_AUDIO_DAC_LDO_VOLT_HIGH
#define TCFG_AUDIO_DAC_LDO_VOLT_HIGH 0
#endif

struct dac_platform_data dac_data = {//临时处理
    .output         = DAC_OUTPUT_MONO_L,               //DAC输出配置，和具体硬件连接有关，需根据硬件来设置
    .dma_buf_time_ms = TCFG_AUDIO_CLASSD_BUFFER_TIME_MS,
};

struct apio_platform_data apio_data = {//临时处理
    .dcc_level = 14,
    .pwm_mode = 1,
    .pwm_mute = 0,
    .dsm_clk_mode = 1,
    .fade_en = 1,
    .fade_slow = 4,
    .fade_fast = 1,
    .syspll_clk_mode = 0,
    .pwm_clk_mode = 1,
    .classd_vbg_value = 7,
    .apa_en = TCFG_AUDIO_CLASSD_OUTPUT & 0b01,
    .apio_en = (TCFG_AUDIO_CLASSD_OUTPUT >> 1) & 0b01,
};


static int audio_init()
{
    wl_audio_clk_on();
    audio_general_init();

    audio_common_param_t common_param = {0};
    common_param.power_level = TCFG_AUDIO_VCM_LEVEL;
    //audio vbg trim配置
    audio_vbg_trim_t vbg_trim = {0};
    int len = syscfg_read(CFG_VBG_TRIM, (void *)&vbg_trim, sizeof(audio_vbg_trim_t));
    if (len != sizeof(audio_vbg_trim_t)) {
        u8 ret = audio_common_power_trim(&vbg_trim, common_param.power_level);
        if (ret == 0) {
            syscfg_write(CFG_VBG_TRIM, (void *)&vbg_trim, sizeof(audio_vbg_trim_t));
        }
    }
    common_param.aud_vbg_value = vbg_trim.aud_vbg_value;
    common_param.pmu_vbg_value = vbg_trim.pmu_vbg_value;

    common_param.vcm_cap_en = TCFG_AUDIO_VCM_CAP_EN;
    common_param.clock_mode = AUDIO_COMMON_CLK_DIG_SINGLE;
    /* common_param.clock_mode = AUDIO_COMMON_CLK_DIF_XOSC; */
    /* common_param.clock_mode = AUDIO_COMMON_CLK_DIG_XOSC; //低功耗配置时钟 */
    audio_common_init(&common_param);
    /* audio_common_clock_open(common_param.clock_mode); */
    audio_input_initcall();
    audio_dac_initcall();

#if TCFG_SMART_VOICE_ENABLE
#if TCFG_AUDIO_ASR_DEVELOP == ASR_CFG_USER_DEFINED
    user_platform_asr_open();
#elif TCFG_AUDIO_ASR_DEVELOP ==DISABLE_THIS_MOUDLE
    audio_smart_voice_detect_init(NULL);
#endif
#endif /* #if TCFG_SMART_VOICE_ENABLE */
    __this->audio_inited = 1;

#if AUD_ADC_TEST_DEMO
    extern void audio_adc_mic_demo_open(u8 mic_idx, u8 gain, int sr, u8 mic_2_dac);
    audio_adc_mic_demo_open(BIT(0), 0, 44100, 0);
#endif
    return 0;
}
platform_initcall(audio_init);

static void audio_uninit()
{
    dac_power_off();
}
platform_uninitcall(audio_uninit);

/*关闭audio相关模块使能*/
static void audio_disable_all(void)
{
    printf("audio_disable_all\n");
#if 0
    //DAC:DACEN
    JL_AUD->AUD_CON0 &= ~BIT(2);
    //ADC:ADCEN
    JL_AUD->AUD_CON0 &= ~(BIT(1) | BIT(4) | BIT(9));
    //EQ:
    JL_EQ->CON0 &= ~BIT(1);
    //FFT:
    //JL_FFT->CON = BIT(1);//置1强制关闭模块，不管是否已经运算完成
    //SRC:
    JL_SRC0->CON1 |= BIT(22);
    JL_SRC1->CON0 |= BIT(10);

#endif

}

REGISTER_UPDATE_TARGET(audio_update_target) = {
    .name = "audio",
    .driver_close = audio_disable_all,
};

/*音频模块寄存器跟踪*/
void audio_adda_dump(void) //打印所有的dac,adc寄存器
{
    printf("JL_WL_AUD CON0:%x", JL_WL_AUD->CON0);
    printf("ADC_CON0:%x", JL_AUDADC->ADC_CON0);
    printf("ADC_CON1:%x", JL_AUDADC->ADC_CON1);
    printf("ADC_CON2:%x", JL_AUDADC->ADC_CON2);
    printf("APA_VL0:%x", JL_APA->APA_VL0);
    printf("AUD_CON 0:%x    1:%x,   2:%x\n", JL_AUD->AUD_CON0, JL_AUD->AUD_CON1, JL_AUD->AUD_CON2);
    printf("APA_CON 0:%x 	1:%x,	2:%x,	3:%x    4:%x,   5:%x\n", JL_APA->APA_CON0, JL_APA->APA_CON1, JL_APA->APA_CON2, JL_APA->APA_CON3, JL_APA->APA_CON4, JL_APA->APA_CON5);
    printf("ADA_CON 0:%x	1:%x	2:%x	3:%x	4:%x\n", JL_ADDA->ADA_CON0, JL_ADDA->ADA_CON1, JL_ADDA->ADA_CON2, JL_ADDA->ADA_CON3, JL_ADDA->ADA_CON4);
    printf("ADDA_CON 0:%x\n", JL_ADDA->ADDA_CON0);
    printf("STD_CON 0:%x\n", JL_LSBCLK->STD_CON0);
    printf("PRP_CON 1:%x\n", JL_LSBCLK->PRP_CON1);
}

/*音频模块增益跟踪*/
void audio_config_dump()
{
    u8 adc_bit_width = ((JL_AUDADC->ADC_CON0 & BIT(20)) ? 24 : 16);
    int classd_dgain_max = 16384;
    int mic_gain_max = 5;
    u8 classd_dcc = (JL_APA->APA_CON0 >> 12) & 0xF;
    u8 mic0_dcc = JL_AUDADC->ADC_CON1 & 0xF;
    u32 classd_dgain = JL_APA->APA_VL0 & 0xFFFF;
    u8 mic0_0_6 = ((JL_ADDA->ADA_CON0 & BIT(24)) ? 1 : 0);
    u8 mic0_gain = (JL_ADDA->ADA_CON1 >> 24) & 0x7;

    printf("[ADC]BitWidth:%d,DCC:%d\n", adc_bit_width, mic0_dcc);
    printf("[ADC]Gain(Max:%d):%d,6dB_Boost:%d,\n", mic_gain_max, mic0_gain, mic0_0_6);

    printf("[CLASSD]DCC:%d\n", classd_dcc);
    printf("[CLASSD]DGain(Max:%d):%d\n", classd_dgain_max, classd_dgain);
}
