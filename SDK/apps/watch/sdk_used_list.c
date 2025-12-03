#include "app_config.h"

#if (!TCFG_AUDIO_DISABLE)
source_node_adapter
#endif

#if TCFG_MIXER_NODE_ENABLE
mixer_node_adapter
#endif

#if TCFG_DAC_NODE_ENABLE
dac_node_adapter
#endif

#if TCFG_CLASSD_DRIVER_NODE_ENABLE
classd_node_adapter
#endif

#if (!TCFG_AUDIO_DISABLE)
clk_sync_node_adapter
decoder_node_adapter
resample_node_adapter
bt_audio_sync_node_adapter
#endif

#if TCFG_ADC_NODE_ENABLE
adc_file_plug
#endif

#if (!TCFG_AUDIO_DISABLE)
tone_file_plug
ring_file_plug
key_tone_file_plug
sbc_decoder_plug
#endif

#ifdef CONFIG_SBC_CODEC_HW
sbc_hwaccel
#endif

#if TCFG_BT_SUPPORT_AAC || TCFG_DEC_AAC_ENABLE
aac_dec_plug
#endif

#if (!TCFG_AUDIO_DISABLE)
sine_dec_plug
pcm_dec_plug
#endif

#if TCFG_ZERO_ACTIVE_NODE_ENABLE
zero_file_plug
#endif

#if TCFG_BT_SUPPORT_LHDC
lhdc_dec_plug
#endif

#if TCFG_BT_SUPPORT_LDAC
ldac_dec_plug
#endif

#if TCFG_ENC_MSBC_ENABLE
#if (defined CONFIG_CPU_BR35)
msbc_encoder_soft_plug
#else
msbc_encoder_hw_plug
#endif
msbc_decoder_plug
#endif

#if TCFG_ENC_SBC_ENABLE
sbc_encoder_soft_plug
#endif

#if TCFG_ENC_CVSD_ENABLE
cvsd_encoder_plug
cvsd_decoder_plug
#endif

#if TCFG_ENC_MP3_ENABLE
mp3_encoder_plug
#endif

#if TCFG_ENC_ADPCM_ENABLE
wav_encoder_plug
#endif

#if TCFG_IIS_NODE_ENABLE
iis_node_adapter
iis1_node_adapter
#endif

#if TCFG_AUDIO_LINEIN_ENABLE
linein_file_plug
#endif

#if TCFG_AUDIO_FM_ENABLE
fm_file_plug
#endif

#if TCFG_APP_MUSIC_EN
music_file_plug
#endif

#if TCFG_ENERGY_DETECT_NODE_ENABLE
energy_detect_node_adapter
#endif


#if TCFG_CONVERT_NODE_ENABLE
convert_node_adapter
convert_data_round_node_adapter
#endif

#if TCFG_PITCH_SPEED_NODE_ENABLE
pitch_speed_node_adapter
#endif

#if TCFG_3BAND_MERGE_ENABLE
three_band_merge_node_adapter
#endif

#if TCFG_2BAND_MERGE_ENABLE
two_band_merge_node_adapter
#endif

#if TCFG_BASS_TREBLE_NODE_ENABLE
bass_treble_eq_node_adapter
#endif


#if TCFG_SOUND_SPLITTER_NODE_ENABLE
sound_splitter_node_adapter
#endif

#if TCFG_VOCAL_TRACK_SYNTHESIS_NODE_ENABLE
vocal_track_synthesis_node_adapter
#endif

#if TCFG_VOCAL_TRACK_SEPARATION_NODE_ENBALE
vocal_track_separation_node_adapter
#endif

#if TCFG_CROSSOVER_NODE_ENABLE
crossover_node_adapter
crossover_2band_node_adapter
#endif

#if TCFG_WDRC_NODE_ENABLE
wdrc_node_adapter
#endif


#if TCFG_STEROMIX_NODE_ENABLE
steromix_node_adapter
#endif

#if TCFG_DYNAMIC_EQ_NODE_ENABLE
dynamic_eq_node_adapter
#endif
#if TCFG_DYNAMIC_EQ_EXT_DETECTOR_NODE_ENABLE
dynamic_eq_ext_detector_node_adapter
#endif

#if TCFG_SURROUND_NODE_ENABLE
surround_node_adapter
#endif

#if TCFG_VOICE_CHANGER_NODE_ENABLE
voice_changer_node_adapter
#endif

#if TCFG_VOICE_CHANGER_ADV_NODE_ENABLE
voice_changer_adv_node_adapter
#endif

#if  TCFG_NOISEGATE_NODE_ENABLE
noisegate_node_adapter
#endif

#if TCFG_FREQUENCY_SHIFT_HOWLING_NODE_ENABLE
frequency_shift_node_adapter
#endif

#if TCFG_NOTCH_HOWLING_NODE_ENABLE
howling_suppress_node_adapter
#endif

#if TCFG_PLATE_REVERB_NODE_ENABLE
plate_reverb_node_adapter
#endif

#if TCFG_PLATE_REVERB_ADVANCE_NODE_ENABLE
plate_reverb_advance_node_adapter
#endif

#if TCFG_ECHO_NODE_ENABLE
echo_node_adapter
#endif

#if TCFG_SPECTRUM_NODE_ENABLE
spectrum_node_adapter
#endif

#if TCFG_AUTOTUNE_NODE_ENABLE
autotune_node_adapter
#endif

#if TCFG_VOCAL_REMOVER_NODE_ENABLE
vocal_remover_node_adapter
#endif

#if TCFG_SPEAKER_EQ_NODE_ENABLE
spk_eq_node_adapter
#endif

#if TCFG_EQ_ENABLE
eq_node_adapter
#endif

#if TCFG_GAIN_NODE_ENABLE
gain_node_adapter
#endif

#if TCFG_VBASS_NODE_ENABLE
vbass_node_adapter
#endif

#if TCFG_NS_NODE_ENABLE
ns_node_adapter
#endif

#if TCFG_DNS_NODE_ENABLE
dns_node_adapter
#endif

#if TCFG_DEC_WTG_ENABLE
g729_dec_plug
#endif

#if TCFG_DEC_MTY_ENABLE
mty_dec_plug
#endif

#if TCFG_DEC_WAV_ENABLE
wav_dec_plug
#endif

#if TCFG_DEC_AIFF_ENABLE
aiff_dec_plug
#endif
#if TCFG_DEC_OPUS_ENABLE
opus_dec_plug
#endif
#if TCFG_DEC_OGG_VORBIS_ENABLE
ogg_dec_plug
#endif

#if TCFG_DEC_AMR_ENABLE
amr_dec_plug
#endif

#if TCFG_DEC_DTS_ENABLE
dts_dec_plug
#endif

#if TCFG_DEC_MP3_ENABLE
mp3_dec_plug
#endif

#if TCFG_DEC_STREAM_MP3_ENABLE
mp3_stream_dec_plug
#endif

#if TCFG_DEC_F2A_ENABLE
f2a_dec_plug
#endif

#if TCFG_DEC_WMA_ENABLE
wma_dec_plug
#endif

#if TCFG_DEC_FLAC_ENABLE
flac_dec_plug
#endif

#if TCFG_DEC_M4A_ENABLE
m4a_dec_plug
#endif

#if TCFG_DEC_ALAC_ENABLE
alac_dec_plug
#endif

#if TCFG_DEC_APE_ENABLE
ape_dec_plug
#endif

#if TCFG_DEC_SPEEX_ENABLE
speex_dec_plug
#endif

#if TCFG_DEC_WTS_ENABLE
wts_dec_plug
#endif

#if CONFIG_RES_SDFILE_FS_ENABLE
res_sdfile_vfs_ops
#endif

#if CONFIG_FATFS_ENABLE
fat_vfs_ops
#endif

#if TCFG_VIRFAT_FLASH_ENABLE || TCFG_VIRFAT_INSERT_FLASH_ENABLE
fat_sdfile_fat_ops
#endif

#if VFS_ENABLE
fat_sdfile_fat_ops
sdfile_resfile_ops
sdfile_vfs_ops
ex_sdfile_vfs_ops
#endif

#if TCFG_NORFLASH_DEV_ENABLE && VFS_ENABLE
nor_fs_vfs_ops
nor_sdfile_vfs_ops
#endif

#if FLASH_INSIDE_REC_ENABLE
inside_nor_fs_vfs_ops
#endif

#if TCFG_STEREO_WIDENER_NODE_ENABLE
stereo_widener_node_adapter
#endif

#if TCFG_HARMONIC_EXCITER_NODE_ENABLE
harmonic_exciter_node_adapter
#endif

#if TCFG_INDICATOR_NODE_ENABLE
indicator_node_adapter
#endif

#if TCFG_CHORUS_NODE_ENABLE
chorus_node_adapter
#endif

#if TCFG_CHANNEL_EXPANDER_NODE_ENABLE
channel_expander_node_adapter
#endif

#if TCFG_CHANNEL_MERGE_NODE_ENABLE
channel_merge_node_adapter
#endif

#if TCFG_WDRC_DETECTOR_NODE_ENABLE
wdrc_detector_node_adapter
#endif

#if TCFG_WDRC_ADVANCE_NODE_ENABLE
wdrc_advance_node_adapter
#endif


#if TCFG_PCM_DELAY_NODE_ENABLE
pcm_delay_node_adapter
#endif

#if TCFG_AUTODUCK_NODE_ENABLE
autoduck_trigger_node_adapter
autoduck_node_adapter
#endif

#if TCFG_EFFECT_DEV0_NODE_ENABLE
effect_dev0_node_adapter
#endif

#if TCFG_EFFECT_DEV1_NODE_ENABLE
effect_dev1_node_adapter
#endif

#if TCFG_EFFECT_DEV2_NODE_ENABLE
effect_dev2_node_adapter
#endif
#if TCFG_EFFECT_DEV3_NODE_ENABLE
effect_dev3_node_adapter
#endif
#if TCFG_EFFECT_DEV4_NODE_ENABLE
effect_dev4_node_adapter
#endif

#if TCFG_FILE_PACKAGE_NODE_ENABLE
packager_adapter
#endif

#if TCFG_WRITE_FILE_NODE_ENABLE
write_file_adapter
#endif

#if TCFG_ENC_ADPCM_ENABLE
wav_encoder_plug
wav_package
#endif

#if TCFG_ENC_PCM_ENABLE
pcm_encoder_plug
#endif

#if TCFG_ENC_OPUS_ENABLE
opus_encoder_plug
#endif

#if TCFG_ENC_SPEEX_ENABLE
speex_encoder_plug
#endif

#if TCFG_DATA_SATURATION_NODE_ENABLE
data_saturation_node_adapter
#endif

#if TCFG_DYNAMIC_EQ_PRO_NODE_ENABLE
dynamic_eq_pro_node_adapter
#endif

#if TCFG_DYNAMIC_EQ_PRO_EXT_DETECTOR_NODE_ENABLE
dynamic_eq_pro_ext_detector_node_adapter
#endif

#if TCFG_AUTO_WAH_NODE_ENABLE
autowah_node_adapter
#endif

#if TCFG_PING_PONG_PCM_DELAY_NODE_ENABLE
pingpong_pcm_delay_node_adapter
#endif

#if TCFG_THREE_D_EFFECT_NODE_ENABLE
three_d_node_adapter
#endif

#if TCFG_LLNS_NODE_ENABLE
llns_node_adapter
#endif

#if TCFG_FADE_NODE_ENABLE
fade_node_adapter
#endif

#if TCFG_AGC_NODE_ENABLE
agc_node_adapter
#endif

#if TCFG_SWITCH_NODE_ENABLE
switch_node_adapter
#endif

#if TCFG_SOFWARE_EQ_NODE_ENABLE
sof_eq_node_adapter
#endif

#if TCFG_LIMITER_NODE_ENABLE
limiter_node_adapter
#endif

a2dp_1sbc_codec_private
#if TCFG_BT_SUPPORT_AAC
a2dp_2aac_sink_codec
#endif
#if TCFG_NOISEGATE_NODE_ENABLE
noisegate_node_adapter
#endif
