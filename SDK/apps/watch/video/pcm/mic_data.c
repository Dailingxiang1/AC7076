#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".mic_data.data.bss")
#pragma data_seg(".mic_data.data")
#pragma const_seg(".mic_data.text.const")
#pragma code_seg(".mic_data.text")
#endif
/*****************************************************************
>file name : voice_mic_data.c
>author : lichao
>create time : Mon 01 Nov 2021 11:33:32 AM CST
*****************************************************************/
#include "app_config.h"
#include "app_main.h"
#include "adc_file.h"
#include "effects/convert_data.h"
#include "circular_buf.h"
#include "media/audio_general.h"
#include "audio_splicing.h"
#include "mic_data.h"

extern struct audio_adc_hdl adc_hdl;

#define MAIN_ADC_GAIN               8
#define MAIN_ADC_BUF_NUM            3

struct main_adc_context {
    struct audio_adc_output_hdl dma_output;
    struct adc_mic_ch mic_ch;
    s16 *dma_buf;
    s16 *mic_sample_data;;
    u8 adc_ch_num;  //打开的adc ch数量
    u8 adc_seq;     //记录当前用的那个mic
    u8 bit_width;
};
/*
 * Mic 数据接收buffer(循环buffer，动态大小)
 */
struct voice_mic_data {
    u8 open;
    u8 source;
    struct main_adc_context *main_adc;
    void *vad_mic;
    struct list_head head;
    cbuffer_t cbuf;
    u8 buf[0];
};

struct voice_mic_capture_channel {
    void *priv;
    void (*output)(void *priv, s16 *data, int len);
    struct list_head entry;
};

static struct voice_mic_data *voice_handle = NULL;
extern const u8 const_adc_async_en;
#define __this      (voice_handle)
int mic_data_write(void *mic, void *data, int len);
static int voice_mic_data_output(void *priv, s16 *data, int len)
{
    struct voice_mic_data *voice = (struct voice_mic_data *)priv;
    int wlen = 0;
    {
        struct voice_mic_capture_channel *ch;
        list_for_each_entry(ch, &voice->head, entry) {
            if (ch->output) {
                ch->output(ch->priv, data, len);
            }
        }
        wlen = mic_data_write(priv, data, len);
    }
    return wlen;
}

static void audio_main_adc_dma_data_handler(void *priv, s16 *data, int len)
{
    struct voice_mic_data *voice = (struct voice_mic_data *)priv;
    if (!voice || voice->source != VOICE_MCU_MIC) {
        return;
    }

    s16 *pcm_data = NULL;
    if (voice->main_adc->bit_width != ADC_BIT_WIDTH_16) {
        s32 *s32_src = (s32 *)data;
        s32 *s32_dst = (s32 *)voice->main_adc->mic_sample_data;
        if (voice->main_adc->adc_ch_num > 1) {
            for (int i = 0; i < len / 4; i++) {
                s32_dst[i] = s32_src[i * voice->main_adc->adc_ch_num + voice->main_adc->adc_seq];
            }
            audio_convert_data_32bit_to_16bit_round(s32_dst, (s16 *)s32_dst, len / 4);
        } else {
            audio_convert_data_32bit_to_16bit_round(s32_src, (s16 *)s32_dst, len / 4);
        }
        pcm_data = (s16 *)s32_dst;
        len >>= 1;
    } else {
        s16 *s16_src = (s16 *)data;
        s16 *s16_dst = (s16 *)voice->main_adc->mic_sample_data;
        if (voice->main_adc->adc_ch_num > 1) {
            for (int i = 0; i < len / 2; i++) {
                s16_dst[i] = s16_src[i * voice->main_adc->adc_ch_num + voice->main_adc->adc_seq];
            }
            pcm_data = (s16 *)s16_dst;

        } else {
            pcm_data = (s16 *)data;
        }
    }
    /* putchar('M'); */
    voice_mic_data_output(voice, pcm_data, len);
}

static int audio_main_adc_mic_open(struct voice_mic_data *voice, int sample_rate)
{
    int ret = 0;
    if (voice->main_adc) {
        printf("audio main adc mic already  open !!!");
        return 0;
    }
    voice->main_adc = zalloc(sizeof(struct main_adc_context));

    if (!voice->main_adc) {
        return -ENOMEM;
    }

    if (const_adc_async_en) {
        /*是否4个adc通道都打开，音箱使用*/
        voice->main_adc->adc_ch_num = 4;
    } else {
        /*默认打开一个adc通道做语音识别，耳机使用*/
        voice->main_adc->adc_ch_num = 1;
    }

    voice->main_adc->bit_width = adc_hdl.bit_width;
    /*设置adc buf大小*/
    int dma_len = 0;
    if (voice->main_adc->bit_width == ADC_BIT_WIDTH_16) {
        dma_len = VOICE_MIC_DATA_PERIOD_FRAMES * sizeof(short) * MAIN_ADC_BUF_NUM * voice->main_adc->adc_ch_num;
    } else {
        dma_len = VOICE_MIC_DATA_PERIOD_FRAMES * sizeof(int) * MAIN_ADC_BUF_NUM * voice->main_adc->adc_ch_num;
    }
    voice->main_adc->dma_buf = zalloc(dma_len);
    printf("adc dma_len %d ", dma_len);

    /*设置缓存buf大小*/
    int buf_len = 0;
    if (voice->main_adc->bit_width == ADC_BIT_WIDTH_16) {
        if (voice->main_adc->adc_ch_num > 1) {
            buf_len = VOICE_MIC_DATA_PERIOD_FRAMES * sizeof(short);
        }
    } else {
        if (voice->main_adc->adc_ch_num > 1) {
            buf_len = VOICE_MIC_DATA_PERIOD_FRAMES * sizeof(int);
        } else {
            buf_len = VOICE_MIC_DATA_PERIOD_FRAMES * sizeof(short);
        }
    }
    printf("mic sample data len %d", buf_len);
    if (buf_len) {
        voice->main_adc->mic_sample_data = zalloc(buf_len);
    }

    /*配置使用哪个mic做语音识别*/
#ifdef TCFG_SMART_VOICE_MIC_CH_SEL
    u8 ch = TCFG_SMART_VOICE_MIC_CH_SEL;
#else
    u8 ch = AUDIO_ADC_MIC_0;
#endif
    printf("%s:%d", __func__, ch);
    audio_adc_file_init();
    if (ch & AUDIO_ADC_MIC_0) {
        adc_file_mic_open(&voice->main_adc->mic_ch, AUDIO_ADC_MIC_0);
        audio_adc_mic_set_gain(&voice->main_adc->mic_ch, AUDIO_ADC_MIC_0, MAIN_ADC_GAIN);
    }
    if (ch & AUDIO_ADC_MIC_1) {
        adc_file_mic_open(&voice->main_adc->mic_ch, AUDIO_ADC_MIC_1);
        audio_adc_mic_set_gain(&voice->main_adc->mic_ch, AUDIO_ADC_MIC_1, MAIN_ADC_GAIN);
    }
    if (ch & AUDIO_ADC_MIC_2) {
        adc_file_mic_open(&voice->main_adc->mic_ch, AUDIO_ADC_MIC_2);
        audio_adc_mic_set_gain(&voice->main_adc->mic_ch, AUDIO_ADC_MIC_2, MAIN_ADC_GAIN);
    }
    if (ch & AUDIO_ADC_MIC_3) {
        adc_file_mic_open(&voice->main_adc->mic_ch, AUDIO_ADC_MIC_3);
        audio_adc_mic_set_gain(&voice->main_adc->mic_ch, AUDIO_ADC_MIC_3, MAIN_ADC_GAIN);
    }

    audio_adc_mic_set_sample_rate(&voice->main_adc->mic_ch, sample_rate);
    ret =  audio_adc_mic_set_buffs(&voice->main_adc->mic_ch, voice->main_adc->dma_buf,
                                   VOICE_MIC_DATA_PERIOD_FRAMES * 2, MAIN_ADC_BUF_NUM);
    if (ret && voice->main_adc->dma_buf) {
        /*已经设置过buf了，并不需要在set buf*/
        free(voice->main_adc->dma_buf);
        voice->main_adc->dma_buf = NULL;
    }
    voice->main_adc->dma_output.priv = voice;
    voice->main_adc->dma_output.handler = audio_main_adc_dma_data_handler;
    audio_adc_add_output_handler(&adc_hdl, &voice->main_adc->dma_output);
    audio_adc_mic_start(&voice->main_adc->mic_ch);
    voice->main_adc->adc_seq = get_adc_seq(&adc_hdl, ch); //查询模拟mic对应的ADC通道,打开4个adc通道时使用
    printf("adc_seq %d", voice->main_adc->adc_seq);

    return 0;
}

static void audio_main_adc_mic_close(struct voice_mic_data *voice, u8 all_channel)
{
    if (voice && voice->main_adc) {
        if (all_channel) {
            audio_adc_mic_close(&voice->main_adc->mic_ch);
        }
        audio_adc_del_output_handler(&adc_hdl, &voice->main_adc->dma_output);
        if (voice->main_adc->dma_buf) {
            /*mic close时会自动释放内存?*/
            /* free(voice->main_adc->dma_buf); */
            voice->main_adc->dma_buf = NULL;
        }
        if (voice->main_adc->mic_sample_data) {
            free(voice->main_adc->mic_sample_data);
            voice->main_adc->mic_sample_data = NULL;
        }
        free(voice->main_adc);
        voice->main_adc = NULL;
    }
}


void *mic_data_open(u8 source, int buffer_size, int sample_rate)
{
    if (__this && __this->open) {
        return __this;
    }
    void *mic_data_cbuf_init(int buffer_size);
    __this = mic_data_cbuf_init(buffer_size);
    __this->source = source;
    INIT_LIST_HEAD(&__this->head);

    audio_main_adc_mic_open(__this, sample_rate);

    __this->open = 1;
    return __this;
}
void mic_data_close(void *mic)
{
    struct voice_mic_data *voice = (struct voice_mic_data *)mic;

    if (voice) {
        audio_main_adc_mic_close(voice, 1);
        free(voice);
    }
    /* int mic_data_cbuf_deinit(void *mc); */
    /* mic_data_cbuf_deinit(__this); */
    __this = NULL;
}

void *mic_data_cbuf_init(int buffer_size)
{
    if (!__this) {
        __this = zalloc(sizeof(struct voice_mic_data) + buffer_size);
    }
    cbuf_init(&__this->cbuf, __this->buf, buffer_size);
    return __this;
}

int mic_data_cbuf_deinit(void *mic)
{
    struct voice_mic_data *fb = (struct voice_mic_data *)mic;
    if (fb) {
        free(fb);
    }
    return 0;
}

int mic_data_write(void *mic, void *data, int len)
{
    struct voice_mic_data *fb = (struct voice_mic_data *)mic;
    int wlen = 0;
    wlen = cbuf_write(&fb->cbuf, data, len);
    //如果有较多丢包，要开下读的putchar打印，评估是否需要加大cbuf
    if (wlen < len) {
        putchar('D');
    }
    return wlen;
}
int mic_data_read(void *mic, void *data, int len)
{
    struct voice_mic_data *fb = (struct voice_mic_data *)mic;
    /* printf("%s. %d, %d", __func__, len, cbuf_get_data_len(&fb->cbuf)); */
    if (cbuf_get_data_len(&fb->cbuf) < len) {
        /* putchar('B'); */
        return 0;
    } else {
        int wlen = cbuf_read(&fb->cbuf, data, len);
        /* putchar('G'); */
        return wlen;
    }
}

int mic_data_buffered_samples(void *mic)
{
    struct voice_mic_data *fb = (struct voice_mic_data *)mic;

    return cbuf_get_data_len(&fb->cbuf) >> 1;
}

void mic_data_clear(void *mic)
{
    struct voice_mic_data *fb = (struct voice_mic_data *)mic;

    cbuf_clear(&fb->cbuf);
}

