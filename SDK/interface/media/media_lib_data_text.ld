#include "cvp/audio_cvp_lib_data_text.ld"

        media_data_code_start = .;
        . = ALIGN(4);
        /*使用动态加载时*/

        /*libkwscommin.a*/
        *(.jlsp_kws_common_code)
        *(.jlsp_kws_common_const)

#ifdef CONFIG_CODE_MOVABLE_ENABLE
#if AUDIO_ENCODER_AT_RAM
		*(.msbc_enc)
		*(.cvsd_codec)
#endif
#if AUD_AAC_DEC_AT_RAM
		aac_dec_code_begin = .;
		*(.bt_aac_dec_code)
		*(.bt_aac_dec_sparse_code)
		aac_dec_code_end = .;
		aac_dec_code_size  = aac_dec_code_end - aac_dec_code_begin ;

		. = ALIGN(4);
		bt_aac_dec_const_begin = .;
		*(.bt_aac_dec_const)
		*(.bt_aac_dec_sparse_const)
		bt_aac_dec_const_end = .;
		bt_aac_dec_const_size = bt_aac_dec_const_end -  bt_aac_dec_const_begin ;

		. = ALIGN(4);
		*(.aac_mem)
		*(.aac_ctrl_mem)
#endif
#endif /*CONFIG_CODE_MOVABLE_ENABLE*/ 

#if (AUD_SBC_DEC_AT_RAM)
		*(.sbc_eng_code)
		*(.sbc_code)
		*(.sbc_const)
        *(.sbc_dec.text.cache.L2.const)
        *(.sbc_dec.text.cache.L2.run)
#endif
#if (AUD_MSBC_CODEC_AT_RAM)
		*(.msbc_enc)
		*(.msbc_code)
		*(.msbc_const)
		*(.msbc_codec_code)
		*(.msbc_codec_const)
        *(.msbc_codec.cache.L2.const)
        *(.msbc_codec.cache.L2.code)
#endif
#if (TCFG_CODE_RUN_RAM_FM_MODE)
        *(.pcm_code)
        *(.pcm_const)
		*(.audio_dec_pcm_code)
		*(.audio_dec_pcm_const)
		*(.stream_code_nor_run)			// 数据流普通输出处理
		*(.mixer_code_nor_run)			// mixer数据流直通输出
		*(.audio_codec_code)			// 解码任务流程
		*(.audio_codec_code_read_file)	// 解码读文件
		*(.audio_src_code_nor_run)		// audio_src运行处理
		*(.src_base_code_nor_run)		// src_base运行处理
		*(.audio_dac_code_nor_run)		// audio_dac普通输出运行处理
		*(.audio_cfifo_code_nor_run)	// audio_cfifo普通运行处理
		*(.audio_track_code)
		*(.audio_track_const)
#endif /*(TCFG_CODE_RUN_RAM_FM_MODE)*/

#if AUDIO_EFFECTS_SURROUND_AT_RAM 
		*(.audio_surround_code)
		*(.audio_surround_const)
		*(.sur_cal_common_code)
		*(.sur_TYPE0_cal_code)
	    *(.sur_TYPE1_cal_code)
		*(.sur_TYPE0_cal_const)
	    *(.sur_TYPE1_cal_const)
		/* *(.sur_LRDRIFT2_cal_code) */
		/* *(.sur_LRDRIFT2_cal_const) */
		/* *(.sur_ROTATE_cal_code) */
		/* *(.sur_ROTATE_cal_const) */
        /* *(.sur_LRDRIFT_cal_code) */
        /* *(.sur_LRDRIFT_cal_const) */
#endif



        . = ALIGN(4);
        media_data_code_end = .;



