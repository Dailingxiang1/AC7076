#include "app_config.h"

// *INDENT-OFF*

#if EXPORT_CONFIG_USB_ENABLE
//usb slave
#if TCFG_USB_SLAVE_ENABLE
c_SRC_FILES += \
	apps/common/device/usb/usb_config.c \
	apps/common/device/usb/device/descriptor.c \
	apps/common/device/usb/device/usb_device.c \
	apps/common/device/usb/device/user_setup.c \
	apps/common/device/usb/device/task_pc.c \

#endif

// mass storage
#if TCFG_USB_SLAVE_MSD_ENABLE
c_SRC_FILES += \
	apps/common/device/usb/device/msd.c \

#endif

c_SRC_FILES += \
	apps/common/device/usb/device/msd_upgrade.c \

// hid
#if TCFG_USB_SLAVE_HID_ENABLE
c_SRC_FILES += \
	apps/common/device/usb/device/hid.c \

#endif

#if TCFG_USB_CUSTOM_HID_ENABLE
c_SRC_FILES += \
	apps/common/device/usb/device/custom_hid.c \
	apps/common/device/usb/device/rcsp_hid_inter.c \

#endif

#if TCFG_USB_WEBUSB_ENABLE
c_SRC_FILES += \
	apps/common/device/usb/device/webusb.c \

#endif

// audio
#if TCFG_USB_SLAVE_AUDIO_SPK_ENABLE || TCFG_USB_SLAVE_AUDIO_MIC_ENABLE
c_SRC_FILES += \
	apps/common/device/usb/device/uac_stream.c

#if (USB_AUDIO_VERSION == USB_AUDIO_VERSION_1_0)
c_SRC_FILES += \
	apps/common/device/usb/device/uac1.c
#endif

#if (USB_AUDIO_VERSION == USB_AUDIO_VERSION_2_0)
c_SRC_FILES += \
	apps/common/device/usb/device/uac2.c
#endif

#endif



#if TCFG_USB_SLAVE_CDC_ENABLE
c_SRC_FILES += \
    apps/common/device/usb/device/cdc.c \

#endif


// usb host
#if TCFG_USB_HOST_ENABLE
c_SRC_FILES += \
	apps/common/device/usb/usb_host_config.c \
	apps/common/device/usb/host/usb_bulk_transfer.c \
	apps/common/device/usb/host/usb_ctrl_transfer.c \
	apps/common/device/usb/host/usb_host.c \

#endif


// udisk
#if TCFG_UDISK_ENABLE
c_SRC_FILES += \
	apps/common/device/usb/host/usb_storage.c

#endif

// gamepad
#if TCFG_ADB_ENABLE
c_SRC_FILES += \
	apps/common/device/usb/host/adb.c \

#endif

#if TCFG_HOST_UVC_ENABLE
 c_SRC_FILES += \
	 apps/common/device/usb/host/usb_video.c \
	 apps/common/device/usb/host/uvc_host.c \
	 apps/common/device/usb/host/video_device.c \
	 apps/common/device/usb/host/uvc_jpeg_demo.c \
	 apps/common/device/usb/host/videobuf.c
#endif


#if TCFG_AOA_ENABLE
c_SRC_FILES += \
    apps/common/device/usb/host/aoa.c \

#endif


#if TCFG_HID_HOST_ENABLE
c_SRC_FILES += \
    apps/common/device/usb/host/hid.c \

#endif



#if TCFG_USB_SLAVE_ENABLE || TCFG_USB_HOST_ENABLE
c_SRC_FILES += \
    apps/common/device/usb/usb_epbuf_manager.c \
    apps/common/device/usb/usb_task.c \

#endif



#endif



// *INDENT-OFF*



c_SRC_FILES += \
	apps/common/third_party_profile/jieli/online_db/spp_online_db.c \
	apps/common/third_party_profile/jieli/online_db/online_db_deal.c \
	apps/common/third_party_profile/jieli/trans_data_demo/spp_trans_data.c \
	apps/common/third_party_profile/jieli/trans_data_demo/le_trans_data.c \
	apps/common/third_party_profile/wechat_sport/wechat_sport_demo.c\
    apps/common/third_party_profile/multi_protocol_common.c \
    apps/common/third_party_profile/multi_protocol_event.c \
    apps/common/third_party_profile/multi_protocol_main.c \


c_SRC_FILES += \
	apps/common/third_party_profile/common/3th_profile_api.c \
	apps/common/third_party_profile/common/custom_cfg.c \
	apps/common/third_party_profile/common/mic_rec.c \


c_SRC_FILES += \
	apps/common/third_party_profile/jieli/rcsp/ble_rcsp_multi_common.c \
	apps/common/third_party_profile/jieli/rcsp/ble_rcsp_server.c \
	apps/common/third_party_profile/jieli/rcsp/ble_rcsp_multi_client.c \
	apps/common/third_party_profile/jieli/rcsp/adv/ble_rcsp_adv.c \
	apps/common/third_party_profile/jieli/rcsp/rcsp_functions/rcsp.c \
	apps/common/third_party_profile/jieli/rcsp/rcsp_functions/rcsp_config.c \
	apps/common/third_party_profile/jieli/rcsp/rcsp_functions/rcsp_task.c \
	apps/common/third_party_profile/jieli/rcsp/rcsp_functions/rcsp_event.c \
	apps/common/third_party_profile/jieli/rcsp/rcsp_functions/rcsp_manage.c \
	apps/common/third_party_profile/jieli/rcsp/rcsp_functions/rcsp_bt_manage.c \



c_SRC_FILES += \
	apps/common/third_party_profile/jieli/rcsp/server/rcsp_cmd_recieve.c \
	apps/common/third_party_profile/jieli/rcsp/server/rcsp_cmd_recieve_no_respone.c \
	apps/common/third_party_profile/jieli/rcsp/server/rcsp_cmd_respone.c \
	apps/common/third_party_profile/jieli/rcsp/server/rcsp_data_recieve.c \
	apps/common/third_party_profile/jieli/rcsp/server/rcsp_data_recieve_no_respone.c \
	apps/common/third_party_profile/jieli/rcsp/server/rcsp_data_respone.c \
	apps/common/third_party_profile/jieli/rcsp/server/rcsp_cmd_user.c \
	apps/common/third_party_profile/jieli/rcsp/server/rcsp_command.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_switch_device.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/rcsp_setting_opt.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/rcsp_setting_sync.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/rcsp_adv_bluetooth.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/rcsp_screen_box_earphone_info.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/rcsp_eq_setting.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/rcsp_high_low_vol_setting.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/rcsp_music_info_setting.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/rcsp_vol_setting.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/rcsp_color_led_setting.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/rcsp_karaoke_eq_setting.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/rcsp_karaoke_setting.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/adv_anc_voice.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/adv_anc_voice_key.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/adv_hearing_aid_setting.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/adv_bt_name_setting.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/adv_key_setting.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/adv_led_setting.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/adv_mic_setting.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/adv_time_stamp_setting.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/adv_work_setting.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/adv_adaptive_noise_reduction.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/adv_ai_no_pick.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/adv_scene_noise_reduction.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/adv_wind_noise_detection.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/adv_voice_enhancement_mode.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/adv_1t2_setting.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/rcsp_misc_setting.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/rcsp_misc_reverbration_setting.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_setting_opt/settings/rcsp_misc_drc_setting.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/device_info/rcsp_device_feature.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/device_info/rcsp_device_status.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/device_info/device_status_cmd/rcsp_fm_func.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/device_info/device_status_cmd/rcsp_rtc_func.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/device_info/device_status_cmd/rcsp_music_func.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/device_info/device_status_cmd/rcsp_linein_func.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/device_info/device_status_cmd/rcsp_bt_func.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_update/rcsp_update.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_update/rcsp_update_tws.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/rcsp_update/rcsp_ch_loader_download.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/external_flash/rcsp_extra_flash_cmd.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/external_flash/rcsp_extra_flash_opt.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/file_transfer/file_transfer.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/file_transfer/file_delete.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/file_transfer/dev_format.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/file_transfer/file_trans_back.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/file_transfer/file_bluk_trans_prepare.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/file_transfer/file_simple_transfer.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/sensors/sensors_data_opt.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/sensors/sport_data_func.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/sensors/sensor_log_notify.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/sensors/nfc_data_opt.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/sensors/sport_info_opt.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/sensors/sport_info_sync.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/browser/rcsp_browser.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/watch_expand/rcsp_watch_info.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/watch_expand/rcsp_watch_device_config.c \
	apps/common/third_party_profile/jieli/rcsp/server/functions/watch_expand/rcsp_common_info_res_file_handler.c \


c_SRC_FILES += \
	apps/common/third_party_profile/jieli/rcsp/client/rcsp_c_cmd_recieve.c \
	apps/common/third_party_profile/jieli/rcsp/client/rcsp_c_cmd_recieve_no_response.c \
	apps/common/third_party_profile/jieli/rcsp/client/rcsp_c_cmd_response.c \
	apps/common/third_party_profile/jieli/rcsp/client/rcsp_m_update/rcsp_update_master.c \

c_SRC_FILES += \
	apps/common/third_party_profile/jieli/screen_trans/screen_ear_protocol_received.c \
	apps/common/third_party_profile/jieli/screen_trans/smartbox_user_app.c \


c_SRC_FILES += \
    apps/common/third_party_profile/swift_pair/swift_pair_protocol.c \


c_SRC_FILES += \
    apps/common/third_party_profile/custom_protocol_demo/custom_protocol.c \


#if EXPORT_FMNA_ENABLE
c_SRC_FILES += \
    apps/common/third_party_profile/bt_fmy/ble_fmy.c \
    apps/common/third_party_profile/bt_fmy/ble_fmy_fmna.c \
    apps/common/third_party_profile/bt_fmy/ble_fmy_ota.c \
    apps/common/third_party_profile/bt_fmy/ble_fmy_modet.c \

#endif

c_SRC_FILES += \
    apps/common/third_party_profile/interface/app_protocol_api.c \
    apps/common/third_party_profile/interface/app_protocol_ota.c \
    apps/common/third_party_profile/interface/app_protocol_common.c \
	apps/common/third_party_profile/interface/app_protocol_gma.c \
	apps/common/third_party_profile/interface/app_protocol_dma.c \
	apps/common/third_party_profile/interface/app_protocol_mma.c \
	apps/common/third_party_profile/interface/app_protocol_tme.c \


c_SRC_FILES += \
	apps/common/third_party_profile/alipay/le_alipay.c \






// *INDENT-OFF*



c_SRC_FILES += \
      audio/framework/plugs/source/a2dp_file.c \
      audio/framework/plugs/source/a2dp_streamctrl.c \
      audio/framework/plugs/source/esco_file.c \
      audio/framework/plugs/source/adc_file.c \
      audio/framework/nodes/esco_tx_node.c \
      audio/framework/nodes/plc_node.c \
      audio/framework/nodes/volume_node.c \

#if TCFG_NS_NODE_ENABLE
c_SRC_FILES += \
      audio/framework/nodes/ns_node.c
#endif

#if TCFG_CHANNEL_SWAP_NODE_ENABLE
c_SRC_FILES += \
      audio/framework/nodes/channle_swap_node.c
#endif

#if TCFG_EFFECT_DEV0_NODE_ENABLE
c_SRC_FILES += \
      audio/framework/nodes/effect_dev0_node.c
#endif
#if TCFG_EFFECT_DEV1_NODE_ENABLE
c_SRC_FILES += \
      audio/framework/nodes/effect_dev1_node.c
#endif
#if TCFG_EFFECT_DEV2_NODE_ENABLE
c_SRC_FILES += \
      audio/framework/nodes/effect_dev2_node.c
#endif
#if TCFG_EFFECT_DEV3_NODE_ENABLE
c_SRC_FILES += \
      audio/framework/nodes/effect_dev3_node.c
#endif
#if TCFG_EFFECT_DEV4_NODE_ENABLE
c_SRC_FILES += \
      audio/framework/nodes/effect_dev4_node.c
#endif
#if TCFG_SINK_DEV0_NODE_ENABLE
c_SRC_FILES += \
      audio/framework/nodes/sink_dev0_node.c
#endif
#if TCFG_SINK_DEV1_NODE_ENABLE
c_SRC_FILES += \
      audio/framework/nodes/sink_dev1_node.c
#endif

#if TCFG_SOURCE_DEV0_NODE_ENABLE
c_SRC_FILES += \
      audio/framework/plugs/source/source_dev0_file.c
#endif
#if TCFG_SOURCE_DEV1_NODE_ENABLE
c_SRC_FILES += \
      audio/framework/plugs/source/source_dev1_file.c
#endif

#if TCFG_AGC_NODE_ENABLE
c_SRC_FILES += \
      audio/framework/nodes/agc_node.c
#endif

#if TCFG_SURROUND_DEMO_NODE_ENABLE
c_SRC_FILES += \
      audio/framework/nodes/surround_demo_node.c
#endif

#if TCFG_AUDIO_CVP_SMS_ANS_MODE || TCFG_AUDIO_CVP_SMS_DNS_MODE
c_SRC_FILES += \
      audio/framework/nodes/cvp_sms_node.c
#endif

c_SRC_FILES += \
      audio/framework/nodes/cvp_dms_node.c

#if TCFG_AUDIO_CVP_3MIC_MODE
c_SRC_FILES += \
      audio/framework/nodes/cvp_3mic_node.c
#endif

#if TCFG_AUDIO_CVP_DEVELOP_ENABLE
c_SRC_FILES += \
      audio/framework/nodes/cvp_develop_node.c
#endif

#if TCFG_UART_NODE_ENABLE
c_SRC_FILES += \
      audio/framework/nodes/uart_node.c
#endif

#if TCFG_DNS_NODE_ENABLE
c_SRC_FILES += \
      audio/framework/nodes/dns_node.c
#endif

#if TCFG_AI_TX_NODE_ENABLE
c_SRC_FILES += \
      audio/framework/nodes/ai_tx_node.c \
      audio/framework/nodes/a2dp_tx_node.c
#endif

#if TCFG_AI_RX_NODE_ENABLE
c_SRC_FILES += \
      audio/framework/nodes/ai_rx_file.c \
	  audio/interface/player/ai_rx_player.c
#endif

#if TCFG_VIDEO_DEC_NODE_ENABLE
c_SRC_FILES += \
      audio/framework/nodes/avi_audio_file.c \
	  audio/interface/player/avi_audio_player.c
#endif

#if TCFG_DATA_EXPORT_NODE_ENABLE
c_SRC_FILES += \
      audio/framework/nodes/data_export_node.c
#endif


#if EXPORT_PLATFORM_AUDIO_PDM_ENABLE
#if TCFG_PDM_NODE_ENABLE
c_SRC_FILES += \
    audio/framework/plugs/source/pdm_mic_file.c
#endif
#endif

#if EXPORT_PLATFORM_AUDIO_SPDIF_ENABLE
#if TCFG_SPDIF_ENABLE
c_SRC_FILES += \
	  audio/framework/plugs/source/spdif_file.c
#endif
#endif


// Audio Test Tool Kit
#if TCFG_AUDIO_MIC_DUT_ENABLE
c_SRC_FILES += \
	  audio/test_tools/mic_dut_process.c
#endif

#if AUDIO_ENC_MPT_SELF_ENABLE
c_SRC_FILES += \
	  audio/test_tools/audio_enc_mpt_self.c \
	  audio/test_tools/audio_enc_mpt_cvp_ctr.c
#endif

#if TCFG_AEC_TOOL_ONLINE_ENABLE
c_SRC_FILES += \
	  audio/test_tools/cvp_tool.c
#endif

#if TCFG_AUDIO_DUT_ENABLE
c_SRC_FILES += \
	  audio/test_tools/audio_dut_control.c \
	  audio/test_tools/audio_dut_control_old.c
#endif

// Audio Common
c_SRC_FILES += \
	  audio/common/audio_node_config.c \
	  audio/common/audio_dvol.c \
	  audio/common/audio_general.c \
	  audio/common/audio_build_needed.c \
	  audio/common/online_debug/aud_data_export.c \
	  audio/common/online_debug/audio_online_debug.c \
	  audio/common/online_debug/audio_capture.c \
	  audio/common/audio_plc.c \
	  audio/common/audio_noise_gate.c \
	  audio/common/audio_ns.c \
	  audio/common/audio_utils.c \
	  audio/common/audio_export_demo.c \
	  audio/common/amplitude_statistic.c \
	  audio/common/frame_length_adaptive.c \
	  audio/common/bt_audio_energy_detection.c \
	  audio/common/audio_event_handler.c \
	  audio/common/debug/audio_debug.c \
	  audio/common/audio_volume_mixer.c \

#if TCFG_AUDIO_MIC_DUT_ENABLE
c_SRC_FILES += \
	  audio/common/online_debug/aud_mic_dut.c
#endif

#if (TCFG_AUDIO_DATA_EXPORT_DEFINE == AUDIO_DATA_EXPORT_VIA_UART)
c_SRC_FILES += \
	  audio/common/uartPcmSender.c
#endif


#if EXPORT_PLATFORM_AUDIO_WM8978_ENABLE
#if 0
c_SRC_FILES += \
	  audio/common/wm8978/wm8978.c \
	  audio/common/wm8978/iic.c
#endif
#endif


// Audio Player
c_SRC_FILES += \
	  audio/interface/player/tone_player.c \
	  audio/interface/player/ring_player.c \
	  audio/interface/player/a2dp_player.c \
	  audio/interface/player/esco_player.c \
	  audio/interface/player/key_tone_player.c \
	  audio/interface/player/dev_flow_player.c \
	  audio/interface/player/adda_loop_player.c \
	  audio/interface/player/reference_time.c \

// Audio Recoder
c_SRC_FILES += \
	  audio/interface/recoder/esco_recoder.c \
	  audio/interface/recoder/ai_voice_recoder.c \
	  audio/interface/recoder/dev_flow_recoder.c \


// Audio Effects
c_SRC_FILES += \
      audio/effect/eq_config.c \
	  audio/effect/spk_eq.c \
	  audio/effect/audio_voice_changer_api.c \
	  audio/effect/esco_ul_voice_changer.c \
	  audio/effect/bass_treble.c \
	  audio/effect/audio_dc_offset_remove.c \
	  audio/effect/effects_adj.c \
	  audio/effect/effects_default_param.c \
	  audio/effect/node_param_update.c \

// ICSD
#if EXPORT_PLATFORM_ICSD_ENABLE
#if TCFG_AUDIO_ANC_ENABLE
c_SRC_FILES += \
	audio/common/icsd/adt/icsd_adt.c \
	audio/common/icsd/adt/icsd_adt_app.c \
	audio/common/icsd/adt/icsd_adt_config.c \
	audio/common/icsd/adt/icsd_adt_alg.c \
	audio/common/icsd/adt/icsd_adt_demo.c \
	audio/common/icsd/anc/icsd_anc_app.c \
	audio/common/icsd/anc/icsd_anc_board.c \
	audio/common/icsd/anc/icsd_anc_data.c \
	audio/common/icsd/anc/icsd_anc_interactive.c \
	audio/common/icsd/common/icsd_common.c \
	audio/common/icsd/dot/icsd_dot_app.c \
	audio/common/icsd/dot/icsd_dot.c \
	audio/common/icsd/common_v2/icsd_common_v2.c \
	audio/common/icsd/common_v2/icsd_common_v2_app.c \
	audio/common/icsd/anc_v2/icsd_anc_v2.c \
	audio/common/icsd/anc_v2/icsd_anc_v2_app.c \
	audio/common/icsd/anc_v2/icsd_anc_v2_interactive.c \
	audio/common/icsd/config/icsd_anc_v2_config.c \
	audio/common/icsd/rt_anc/rt_anc.c \
	audio/common/icsd/rt_anc/rt_anc_app.c \
	audio/common/icsd/rt_anc/rt_anc_config.c \
	audio/common/icsd/tool/anc_ext_tool.c \
	audio/common/icsd/tool/anc_ext_tool_file.c \
	audio/common/icsd/aeq/icsd_aeq_app.c \
	audio/common/icsd/aeq/icsd_aeq.c \
	audio/common/icsd/aeq/icsd_aeq_config.c \
	audio/common/icsd/afq/icsd_afq_app.c \
	audio/common/icsd/afq/icsd_afq.c \
	audio/common/icsd/afq/icsd_afq_config.c \
	audio/common/icsd/cmp/icsd_cmp_app.c \
	audio/common/icsd/cmp/icsd_cmp_config.c \
	audio/common/icsd/demo/icsd_demo.c \
	audio/common/icsd/ein/icsd_ein_config.c \
	audio/common/icsd/vdt/icsd_vdt_config.c \
	audio/common/icsd/wat/icsd_wat_config.c \
	audio/common/icsd/wind/icsd_wind_config.c \
	audio/common/icsd/avc/icsd_avc_config.c \

#endif
#endif

// Audio ANC
#if EXPORT_PLATFORM_ANC_ENABLE
#if TCFG_AUDIO_ANC_ENABLE
c_SRC_FILES += \
	  audio/anc/audio_anc_fade_ctr.c \
	  audio/anc/audio_anc_common_plug.c \
	  audio/anc/audio_anc_debug_tool.c \
	  audio/anc/audio_anc_mult_scene.c \
	  audio/framework/nodes/anc_music_dynamic_gain_node.c \

#endif
#endif

// Smart Voice
#if EXPORT_PLATFORM_AUDIO_SMART_VOICE_ENABLE
#if TCFG_SMART_VOICE_ENABLE
c_SRC_FILES += \
	audio/smart_voice/smart_voice_core.c \
	audio/smart_voice/smart_voice_config.c \
	audio/smart_voice/jl_kws_platform.c \
	audio/smart_voice/user_asr.c \
	audio/smart_voice/voice_mic_data.c \
	audio/smart_voice/kws_event.c \
	audio/smart_voice/nn_vad.c
#endif
#endif

// KWS(yes/no)
#if TCFG_KWS_VOICE_RECOGNITION_ENABLE
c_SRC_FILES += \
	audio/jl_kws/jl_kws_main.c \
	audio/jl_kws/jl_kws_audio.c \
	audio/jl_kws/jl_kws_algo.c \
	audio/jl_kws/jl_kws_event.c \

#endif


#if EXPORT_PLATFORM_AUDIO_SPATIAL_EFFECT_ENABLE
#if TCFG_SPATIAL_AUDIO_ENABLE
c_SRC_FILES += \
	audio/framework/nodes/spatial_effects_node.c \
	audio/common/online_debug/aud_spatial_effect_dut.c \

c_SRC_FILES += \
	audio/effect/spatial_effect/spatial_effect.c \
	audio/effect/spatial_effect/spatial_effect_imu.c \
	audio/effect/spatial_effect/spatial_effect_tws.c \
	audio/effect/spatial_effect/spatial_imu_trim.c \
	audio/effect/spatial_effect/spatial_effects_process.c \

#endif
#endif

// Clear Voice Process(AEC/NLP/NS/AGC...)
c_SRC_FILES += \
	  audio/CVP/audio_cvp.c \
	  audio/CVP/audio_cvp_dms.c \
	  audio/CVP/audio_cvp_3mic.c \
	  audio/CVP/audio_cvp_online.c \
	  audio/CVP/audio_cvp_demo.c \
	  audio/CVP/audio_cvp_develop.c \
	  audio/CVP/audio_cvp_sync.c \
	  audio/CVP/audio_cvp_ais_3mic.c \
	  audio/CVP/audio_cvp_ref_task.c \

#if TCFG_USER_TWS_ENABLE
c_SRC_FILES += \
	  audio/interface/player/tws_tone_player.c
#endif

// ALINK BUILD
#if EXPORT_PLATFORM_AUDIO_ALINK_ENABLE
#if TCFG_IIS_NODE_ENABLE
c_SRC_FILES += \
	  audio/framework/nodes/iis_node.c \
	  audio/framework/plugs/source/iis_file.c
#endif
#endif

// SPDIF BUILD
#if EXPORT_PLATFORM_AUDIO_SPDIF_ENABLE
#if TCFG_SPDIF_MASTER_NODE_ENABLE
c_SRC_FILES += \
	  audio/framework/nodes/spdif_node.c
#endif
#endif

// SPDIF BUILD
#if EXPORT_PLATFORM_AUDIO_FM_ENABLE
c_SRC_FILES += \
	  audio/interface/player/fm_player.c \
	  audio/framework/plugs/source/fm_file.c \

#endif


#if TCFG_APP_MUSIC_EN
c_SRC_FILES += \
      audio/interface/player/file_player.c \

#endif

#if TCFG_APP_RECORD_EN || TCFG_MIX_RECORD_ENABLE
c_SRC_FILES += \
      audio/interface/recoder/file_recorder.c \

#endif

#if TCFG_APP_LINEIN_EN
c_SRC_FILES += \
	  audio/framework/plugs/source/linein_file.c
#endif

#if TCFG_USB_SLAVE_AUDIO_SPK_ENABLE
c_SRC_FILES += \
	  audio/framework/plugs/source/pc_spk_file.c \
	  audio/interface/player/pc_spk_player.c \

#endif

#if TCFG_USB_SLAVE_AUDIO_MIC_ENABLE
c_SRC_FILES += \
	  audio/framework/nodes/pc_mic_node.c \
	  audio/interface/recoder/pc_mic_recoder.c \

#endif

#if TCFG_PITCH_SPEED_NODE_ENABLE
c_SRC_FILES += \
	  audio/effect/audio_pitch_speed_api.c \

#endif



// Audio Effects params update demo
#if EXPORT_PLATFORM_AUDIO_EFFECT_DEMO_ENABLE
#if 0
c_SRC_FILES += \
      audio/effect/demo/autotune_demo.c \
      audio/effect/demo/bass_treble_demo.c \
      audio/effect/demo/chorus_demo.c \
      audio/effect/demo/crossover_demo.c \
      audio/effect/demo/drc_demo.c \
      audio/effect/demo/dynamic_eq_demo.c \
      audio/effect/demo/echo_demo.c \
      audio/effect/demo/energy_detect_demo.c \
      audio/effect/demo/eq_demo.c \
      audio/effect/demo/frequency_shift_howling_demo.c \
      audio/effect/demo/gain_demo.c \
      audio/effect/demo/harmonic_exciter_demo.c \
      audio/effect/demo/noisegate_demo.c \
      audio/effect/demo/notch_howling_demo.c \
      audio/effect/demo/pitch_speed_demo.c \
      audio/effect/demo/reverb_advance_demo.c \
      audio/effect/demo/reverb_demo.c \
      audio/effect/demo/spectrum_demo.c \
      audio/effect/demo/stereo_widener_demo.c \
      audio/effect/demo/surround_demo.c \
      audio/effect/demo/virtual_bass_demo.c \
      audio/effect/demo/voice_changer_demo.c \

#endif
#endif





// *INDENT-OFF*

c_SRC_FILES += \
	  audio/cpu/br35/audio_setup.c \
	  audio/cpu/br35/audio_config.c \
	  audio/cpu/br35/media_memory_manager.c \
	  audio/cpu/br35/audio_dai/audio_pdm.c \

/* #if TCFG_AUDIO_ANC_ENABLE */
/* objs += \ */
	  /* audio/cpu/br35/audio_anc.c \ */
	  /* audio/cpu/br35/icsd_anc_user.c */
/* #endif */


//Audio Accelerator
/* objs += \ */
	  /* audio/cpu/br35/audio_accelerator/hw_fft.c \ */

/* #if TCFG_SMART_VOICE_ENABLE */
/* objs += \ */
	/* audio/cpu/br35/audio_vad/vad_mic.c \ */
	/* audio/cpu/br35/audio_vad/vad_clock_trim.c */
/* #endif */

//Audio Demo Files
c_SRC_FILES += \
	  audio/cpu/br35/audio_demo/audio_adc_demo.c \
	  audio/cpu/br35/audio_demo/audio_dac_demo.c


// *INDENT-OFF*

c_SRC_FILES += \
    apps/watch/ble/bt_ble.c \
    apps/watch/ble/ble_adv.c


c_SRC_FILES += \
    apps/watch/battery/charge.c \
    apps/watch/battery/charge_device_handle.c

#if TCFG_SYS_LVD_EN
c_SRC_FILES += \
    apps/watch/battery/battery_level.c \
    apps/watch/battery/battery_offset.c
#endif


#if (TCFG_CHARGESTORE_ENABLE || TCFG_TEST_BOX_ENABLE)
c_SRC_FILES += \
    apps/watch/battery/charge_store.c
#endif



#if TCFG_KWS_VOICE_RECOGNITION_ENABLE
c_SRC_FILES += \
	apps/watch/mode/bt/bt_call_kws_handler.c \
    apps/watch/mode/bt/kws_voice_event_deal.c
#endif


#if TCFG_MIC_EFFECT_ENABLE
c_SRC_FILES += \
    apps/watch/mode/mic_effect/mic_effect.c \

#endif

#if TCFG_APP_PC_EN
c_SRC_FILES += \
    apps/watch/mode/pc/pc.c \
    apps/watch/mode/pc/pc_app_msg_handler.c \
    apps/watch/mode/pc/pc_key_msg_table.c \

#endif

#if TCFG_APP_RECORD_EN
c_SRC_FILES += \
    apps/watch/mode/record/record.c \
    apps/watch/mode/record/record_app_msg_handler.c \
    apps/watch/mode/record/record_key_msg_table.c \

#endif

#if TCFG_APP_RTC_EN
c_SRC_FILES += \
    apps/watch/mode/rtc/rtc.c \
    apps/watch/mode/rtc/rtc_app_msg_handler.c \
    apps/watch/mode/rtc/rtc_key_msg_table.c \
    apps/watch/mode/rtc/alarm_api.c \
    apps/watch/mode/rtc/alarm_user.c \

#endif


