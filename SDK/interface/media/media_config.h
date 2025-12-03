#ifndef LIB_MEDIA_CONFIG_H
#define LIB_MEDIA_CONFIG_H


extern const int config_audio_cfg_debug_online;

extern const int CONFIG_DAC_CACHE_MSEC;
extern const int CONFIG_JLSTREAM_MULTI_THREAD_ENABLE;
extern const int CONFIG_MULTI_THREAD_SELF_ADAPTION_ENABLE;
extern const int config_media_24bit_enable;
extern const int config_audio_dac_dma_buf_realloc_enable;
extern const int config_audio_adc_performance_mode;
extern const int config_audio_adc_input_mode;
extern const int config_audio_adc_input_port;
extern const int CONFIG_SEAMLESS_RECORDER_ENABLE;
extern const int CONFIG_LOG_OUTPUT_ENABLE;
extern const int CONFIG_MEDIA_MEM_DEBUG;


extern const int config_classd_output_mode;
extern const int config_audio_classd_power_off_enable;
extern const int config_audio_classd_light_power_off_enable;//轻量级关闭

//*********************************************************************
//						Audio Effects Configs
//*********************************************************************
/*EQ*/
extern const int config_audio_eq_hp_enable;		//High Pass
extern const int config_audio_eq_lp_enable;		//Low Pass
extern const int config_audio_eq_bp_enable;		//Band Pass(Peaking)
extern const int config_audio_eq_hs_enable;		//High Shelf
extern const int config_audio_eq_ls_enable;		//Low Shelf
extern const int config_audio_eq_hs_q_enable;	//High Shelf Q
extern const int config_audio_eq_ls_q_enable;	//Low Shelf Q
extern const int config_audio_eq_hp_adv_enable;	//High Pass Advance：对应工具上阶数可选的Hp
extern const int config_audio_eq_lp_adv_enable;	//Low Pass Advance：对应工具上阶数可选的Lp

/*
 *******************************************************************
 *						Audio Mic Capless Config
 *******************************************************************
 */
extern const u8 const_mic_capless_open_delay_debug;
extern const u8 const_mic_capless_trim_delay_debug;












#endif
