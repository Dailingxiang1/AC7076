#include "app_config.h"
#include "system/includes.h"
#include "system/timer.h"
#include "media/includes.h"
#include "asm/audio_adc.h"
#include "asm/dac.h"
#include "ac7076a3_demo.h"

#if AC7076A3_DEMO_ENABLE && (DEMO_MIC_ENABLE || DEMO_SPEAKER_ENABLE)
extern struct audio_adc_hdl adc_hdl;
extern struct audio_dac_hdl dac_hdl;

#if DEMO_MIC_ENABLE
static struct adc_mic_ch demo_mic;
static struct audio_adc_output_hdl mic_output;
static s16 *mic_buf;
static volatile u32 mic_blocks, mic_peak, mic_mean;
static u32 mic_report_at, last_blocks;
static int mic_running;

static void mic_output_handler(void *priv, s16 *data, int len)
{
    u32 sum = 0, peak = 0;
    int count = len / (adc_hdl.bit_width ? 4 : 2);
    (void)priv;
    if (count <= 0) {
        return;
    }
    for (int i = 0; i < count; ++i) {
        /* Convert signed 24-bit-in-32 samples to the same scale as s16. */
        s32 sample = adc_hdl.bit_width ? ((s32 *)data)[i] / 256 : data[i];
        u32 value = sample < 0 ? -sample : sample;
        if (value > peak) {
            peak = value;
        }
        sum += value;
    }
    mic_peak = peak;
    mic_mean = sum / count;
    ++mic_blocks;
}

int ac7076a3_demo_mic_start(void)
{
    if (mic_running) {
        return 0;
    }
    /* H3: two-wire MIC on PA4, capless with internal bias on PA4. */
    struct mic_open_param param = {
        .mic_ain_sel = AUDIO_MIC0_CH0,
        .mic_bias_sel = AUDIO_MIC_BIAS_CH0,
        .mic_bias_rsel = TCFG_ADC0_BIAS_RSEL,
        .mic_mode = AUDIO_MIC_CAPLESS_MODE,
        .mic_dcc = TCFG_ADC0_DCC_LEVEL,
    };
    audio_adc_mic_set_sample_rate(&demo_mic, 16000);
    int ret = audio_adc_mic_open(&demo_mic, AUDIO_ADC_MIC_0, &adc_hdl, &param);
    if (ret) {
        return ret;
    }
    audio_adc_mic_set_gain(&demo_mic, AUDIO_ADC_MIC_0, DEMO_MIC_GAIN);
    int block_bytes = 160 * (adc_hdl.bit_width ? 4 : 2);
    mic_buf = zalloc(block_bytes * 2);
    if (!mic_buf) {
        audio_adc_mic_close(&demo_mic);
        return -1;
    }
    ret = audio_adc_mic_set_buffs(&demo_mic, mic_buf, block_bytes, 2);
    if (ret) {
        /* SDK mic_close releases a buffer once attached by set_buffs. */
        audio_adc_mic_close(&demo_mic);
        return ret;
    }
    mic_output.handler = mic_output_handler;
    audio_adc_add_output_handler(&adc_hdl, &mic_output);
    ret = audio_adc_mic_start(&demo_mic);
    if (ret) {
        audio_adc_del_output_handler(&adc_hdl, &mic_output);
        audio_adc_mic_close(&demo_mic);
        return ret;
    }
    mic_running = 1;
    printf("[BRINGUP] MIC PA4 capless 16kHz gain=%u, speak/tap to vary level\n", DEMO_MIC_GAIN);
    return 0;
}
#endif

#if DEMO_SPEAKER_ENABLE
/* 1 kHz / 16 kHz, reduced PCM level; no stored tone/UI resources required. */
static const s16 sine16[16] = {
    0, 784, 1448, 1892, 2048, 1892, 1448, 784,
    0, -784, -1448, -1892, -2048, -1892, -1448, -784
};
static union { s16 s16_data[160 * 2]; s32 s32_data[160 * 2]; } speaker_buf;
static int speaker_active, speaker_offset, speaker_bytes;
static u32 speaker_frames_left, speaker_deadline, speaker_drain_at;

void ac7076a3_demo_beep_start(void)
{
    if (speaker_active || !dac_hdl.pd) {
        return;
    }
    audio_dac_set_sample_rate(&dac_hdl, 16000);
    audio_dac_set_volume(&dac_hdl, DEMO_SPEAKER_VOLUME);
    if (audio_dac_start(&dac_hdl)) {
        printf("[BRINGUP] DAC start failed\n");
        return;
    }
    audio_dac_channel_start(NULL);
    int channels = dac_hdl.channel;
    if (channels < 1 || channels > 2) {
        audio_dac_channel_close(NULL);
        audio_dac_stop(&dac_hdl);
        printf("[BRINGUP] DAC unexpected channel count=%d\n", channels);
        return;
    }
    for (int i = 0; i < 160; ++i) {
        for (int ch = 0; ch < channels; ++ch) {
            if (dac_hdl.pd->bit_width) {
                speaker_buf.s32_data[i * channels + ch] = (s32)sine16[i & 15] * 256;
            } else {
                speaker_buf.s16_data[i * channels + ch] = sine16[i & 15];
            }
        }
    }
    speaker_bytes = 160 * channels * (dac_hdl.pd->bit_width ? 4 : 2);
    speaker_offset = 0;
    speaker_frames_left = (DEMO_SPEAKER_DURATION_MS + 9) / 10;
    speaker_drain_at = 0;
    speaker_deadline = sys_timer_get_ms() + 1500;
    speaker_active = 1;
    printf("[BRINGUP] DAC 1kHz tone start, %ums, volume=%u\n",
           DEMO_SPEAKER_DURATION_MS, DEMO_SPEAKER_VOLUME);
}

static void speaker_stop(void)
{
    audio_dac_channel_close(NULL);
    audio_dac_stop(&dac_hdl);
    speaker_active = 0;
    printf("[BRINGUP] DAC stopped\n");
}
#endif

void ac7076a3_demo_audio_poll(void)
{
    u32 now = sys_timer_get_ms();
#if DEMO_MIC_ENABLE
    if (mic_running && (s32)(now - mic_report_at) >= 0) {
        mic_report_at = now + 1000;
        u32 count, peak, mean;
        local_irq_disable();
        count = mic_blocks;
        peak = mic_peak;
        mean = mic_mean;
        local_irq_enable();
        printf("[BRINGUP] MIC blocks=%u peak=%u mean_abs=%u %s\n",
               count, peak, mean, count == last_blocks ? "NO_NEW_DATA" : "");
        last_blocks = count;
    }
#endif
#if DEMO_SPEAKER_ENABLE
    if (!speaker_active) {
        return;
    }
    if ((s32)(now - speaker_deadline) >= 0) {
        printf("[BRINGUP] DAC timeout\n");
        speaker_stop();
        return;
    }
    if (speaker_frames_left) {
        int remaining = speaker_bytes - speaker_offset;
        int written = audio_dac_write(&dac_hdl, (u8 *)&speaker_buf + speaker_offset, remaining);
        if (written < 0 || written > remaining) {
            printf("[BRINGUP] DAC write error=%d\n", written);
            speaker_stop();
            return;
        }
        speaker_offset += written; /* byte offset, not sample offset */
        if (speaker_offset == speaker_bytes) {
            speaker_offset = 0;
            if (--speaker_frames_left == 0) {
                speaker_drain_at = now + 100;
            }
        }
    } else if ((s32)(now - speaker_drain_at) >= 0) {
        speaker_stop();
    }
#endif
}
#endif
