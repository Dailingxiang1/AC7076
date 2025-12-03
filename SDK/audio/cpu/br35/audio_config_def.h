#ifndef _AUDIO_CONFIG_DEF_H_
#define _AUDIO_CONFIG_DEF_H_
/*
 *******************************************************************
 *						Audio CPU Definitions
 *
 *Note(s):
 *		 Only macro definitions can be defined here.
 *******************************************************************
 */

// #include "app_config.h"

//**************************************
// 			ASS通用配置
//**************************************
#define MEDIA_24BIT_ENABLE		0//TCFG_AUDIO_BIT_WIDTH
#define AUD_DAC_TRIM_ENABLE		0
// ADC性能测试模式说明:
// 1.使能 AUD_ADC_TEST_DEMO
// 2.使能 audio_adc_demo.c中的ADC_DEBUG_BUF
// 3.将sdk_config.h中的TCFG_ADC0_BIAS_SEL配置为0
// 4.关闭lib_system_config.c中的config_printf_time
// 5.防止pcm数据打印出错,降低串口波特率为1000000
// 打开测试模式后会等ADC数据稳定并将ADC中数据通过打印出来用作分析
#define AUD_ADC_TEST_DEMO       0

#define AUD_MEDIA_EFx_SCENE_SWITCH_ENABLE	0	//媒体音效场景切换使能
#define AUD_MIC_EFx_SCENE_SWITCH_ENABLE		0	//麦克风音效场景切换使能

//**************************************
// 			音频模块链接配置
//**************************************
/*通话语音处理算法*/
#define AUDIO_CVP_TEXT_AT_RAM	    	0	//COMMON TEXT
#define AUDIO_CVP_AEC_AT_RAM	    	0	//AEC
#define AUDIO_CVP_NLP_AT_RAM	    	0	//NLP
#define AUDIO_CVP_NS_AT_RAM		    	0	//ANS/下行降噪
#define AUDIO_CVP_COMMON_AT_RAM	    	0	//COMMON
#define AUDIO_CVP_DNS_AT_RAM	    	0	//DNS
#define AUDIO_CVP_AGC_AT_RAM	    	0	//AGC
#define AUDIO_CVP_DMS_AT_RAM	    	0	//双MIC DMS
#define AUDIO_CVP_SMS_AT_RAM		    0	//单MIC SMS_TDE
#define AUDIO_CVP_PREP_AT_RAM		    0	//COMMON 预处理
#define AUDIO_CVP_WN_AT_RAM			    0	//抗风噪
#define AUDIO_CVP_THIRD_AT_RAM		    0	//3MIC
#define AUDIO_ENCODER_AT_RAM		    0	//MSBC/CVSD 解码

#define AUD_AAC_DEC_AT_RAM		        0   //AAC解码

#define AUD_SBC_DEC_AT_RAM              0
#define AUD_MSBC_CODEC_AT_RAM           0

//**************************************
// 			模块使能控制
//**************************************
#define AUD_PDM_LINK_ENABLE		0

//***************End********************
#define AUDIO_ADC_IRQ_POINTS 256

#if TCFG_BT_VOL_SYNC_ENABLE
#define TCFG_MAX_VOL_PROMPT						 0
#else
#define TCFG_MAX_VOL_PROMPT						 1
#endif
/*
 *br36 支持24bit位宽输出
 * */
#define TCFG_AUDIO_DAC_24BIT_MODE           0
#define AUDIO_DAC_MAX_SAMPLE_RATE           48000


/*
 *该配置适用于没有音量按键的产品，防止打开音量同步之后
 *连接支持音量同步的设备，将音量调小过后，连接不支持音
 *量同步的设备，音量没有恢复，导致音量小的问题
 *默认是没有音量同步的，将音量设置到最大值，可以在vol_sync.c
 *该宏里面修改相应的设置。
 */
#define TCFG_VOL_RESET_WHEN_NO_SUPPORT_VOL_SYNC	 0 //不支持音量同步的设备默认最大音量

#define TCFG_1T2_VOL_RESUME_WHEN_NO_SUPPORT_VOL_SYNC	0	//1T2不支持音量同步的设备则恢复上次设置的音量值

/*省电容mic模块使能*//*省电容mic模块使能*/
#if (TCFG_ADC0_ENABLE && (TCFG_ADC0_MODE == 2))
#define TCFG_SUPPORT_MIC_CAPLESS        1
#else
#define TCFG_SUPPORT_MIC_CAPLESS        0
#endif

/*省电容mic校准方式选择*/
#define MC_BIAS_ADJUST_DISABLE			0	//省电容mic偏置校准关闭
#define MC_BIAS_ADJUST_ONE			 	1	//省电容mic偏置只校准一次（跟dac trim一样）
#define MC_BIAS_ADJUST_POWER_ON		 	2	//省电容mic偏置每次上电复位都校准(Power_On_Reset)
#define MC_BIAS_ADJUST_ALWAYS		 	3	//省电容mic偏置每次开机都校准(包括上电复位和其他复位)
/*
 *省电容mic偏置电压自动调整(因为校准需要时间，所以有不同的方式)
 *1、烧完程序（完全更新，包括配置区）开机校准一次
 *2、上电复位的时候都校准,即断电重新上电就会校准是否有偏差(默认)
 *3、每次开机都校准，不管有没有断过电，即校准流程每次都跑
 */
#if TCFG_SUPPORT_MIC_CAPLESS
#define TCFG_MC_BIAS_AUTO_ADJUST	 	MC_BIAS_ADJUST_ONE
#else
#define TCFG_MC_BIAS_AUTO_ADJUST	 	MC_BIAS_ADJUST_DISABLE
#endif/*TCFG_SUPPORT_MIC_CAPLESS*/
#define TCFG_MC_CONVERGE_PRE			0  //省电容mic预收敛
#define TCFG_MC_CONVERGE_TRACE			0  //省电容mic收敛值跟踪
/*
 *省电容mic收敛步进限制
 *0:自适应步进调整, >0:收敛步进最大值
 *注：当mic的模拟增益或者数字增益很大的时候，mic_capless模式收敛过程,
 *变化的电压放大后，可能会听到哒哒声，这个时候就可以限制住这个收敛步进
 *让收敛平缓进行(前提是预收敛成功的情况下)
 */
#define TCFG_MC_DTB_STEP_LIMIT			3  //最大收敛步进值
/*
 *省电容mic使用固定收敛值
 *可以用来测试默认偏置是否合理：设置固定收敛值7000左右，让mic的偏置维持在1.5v左右即为合理
 *正常使用应该设置为0,让程序动态收敛
 */
#define TCFG_MC_DTB_FIXED				0

#define TCFG_ESCO_PLC					1  	//通话丢包修复(1T2已修改为节点)
#define TCFG_AEC_ENABLE					1	//通话回音消除使能

#define MAX_ANA_VOL               (3)	// 系统最大模拟音量,范围: 0 ~ 3
//#define MAX_COM_VOL             (16)    // 数值应该大于等于16，具体数值应小于联合音量等级的数组大小 (combined_vol_list)
//#define MAX_DIG_VOL             (16)    // 数值应该大于等于16，因为手机是16级，如果小于16会导致某些情况手机改了音量等级但是小机音量没有变化

/*#if (SYS_VOL_TYPE == VOL_TYPE_DIGITAL)
#define SYS_MAX_VOL             16
#define SYS_DEFAULT_VOL         16
#define SYS_DEFAULT_TONE_VOL    10
#define SYS_DEFAULT_SIN_VOL    	8

#elif (SYS_VOL_TYPE == VOL_TYPE_DIGITAL_HW)
#define SYS_MAX_VOL             MAX_DIG_VOL
#define SYS_DEFAULT_VOL         SYS_MAX_VOL
#define SYS_DEFAULT_TONE_VOL    10
#define SYS_DEFAULT_SIN_VOL    	8

#elif (SYS_VOL_TYPE == VOL_TYPE_ANALOG)
#define SYS_MAX_VOL             MAX_ANA_VOL
#define SYS_DEFAULT_VOL         SYS_MAX_VOL
#define SYS_DEFAULT_TONE_VOL    10
#define SYS_DEFAULT_SIN_VOL    	8

#elif (SYS_VOL_TYPE == VOL_TYPE_AD)
#define SYS_MAX_VOL             MAX_COM_VOL
#define SYS_DEFAULT_VOL         SYS_MAX_VOL
#define SYS_DEFAULT_TONE_VOL    14
#define SYS_DEFAULT_SIN_VOL    	8
#else
#error "SYS_VOL_TYPE define error"
#endif*/

/*数字音量最大值定义*/
#define DEFAULT_DIGITAL_VOLUME  16384
#define IDLE_DEFAULT_MAX_VOLUME  16     //音量调节IDLE 状态的最大音量

#if TCFG_AUDIO_ANC_ENABLE
#define ANC_MODE_DIG_VOL_LIMIT	(0.0f)
#endif/*TCFG_AUDIO_ANC_ENABLE*/


#define BT_MUSIC_VOL_LEAVE_MAX	16		/*高级音频音量等级*/
#define BT_CALL_VOL_LEAVE_MAX	15		/*通话音量等级*/
// #define BT_CALL_VOL_STEP		(-2.0f)	[>通话音量等级衰减步进<]

/*
 *audio state define
 */
#define APP_AUDIO_STATE_IDLE        0
#define APP_AUDIO_STATE_MUSIC       1
#define APP_AUDIO_STATE_CALL        2
#define APP_AUDIO_STATE_WTONE       3
#define APP_AUDIO_CURRENT_STATE     4

#define TONE_BGM_FADEOUT            0   //播叠加提示音时是否将背景音淡出

#define VOL_TAB_CUSTOM_EN           1  //使能音量表功能

#define TCFG_AUDIO_MIC_DUT_ENABLE	0   //麦克风测试和传递函数测试
//**************************************
// 			EFFECT配置
//**************************************
#define AUDIO_EFFECTS_REVERB_AT_RAM     0
#define AUDIO_EFFECTS_ECHO_AT_RAM       0
#define AUDIO_EFFECTS_EQ_AT_RAM         0
#define AUDIO_EFFECTS_DRC_AT_RAM        0
#define AUDIO_EFFECTS_AFC_AT_RAM        0
#define AUDIO_EFFECTS_NOISEGATE_AT_RAM  0
#define AUDIO_EFFECTS_GAIN_AT_RAM       0
#define AUDIO_EFFECTS_DYNAMIC_EQ_AT_RAM 0
#define AUDIO_EFFECTS_MIC_EFFECT_AT_RAM 0
#define AUDIO_EFFECTS_MIC_STREAM_AT_RAM 0
#define AUDIO_EFFECTS_DEBUG_AT_RAM      0
#define AUDIO_EFFECTS_VBASS_AT_RAM      0
#define AUDIO_EFFECTS_SURROUND_AT_RAM   0

#define TCFG_CODE_RUN_RAM_FM_MODE       0

//**************************************
// 			音效模块使能配置
//**************************************

//limiter运行模式使能控制，支持多种位宽时使用或配置
#define AUDIO_LIMITER_RUN_MODE          (EFx_BW_32t16 | LIMITER_PRECISION_MAX)

#endif/*_AUDIO_CONFIG_DEF_H_*/
