#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".intelligent_duer_voice.data.bss")
#pragma data_seg(".intelligent_duer_voice.data")
#pragma const_seg(".intelligent_duer_voice.text.const")
#pragma code_seg(".intelligent_duer_voice.text")
#endif
#include "duer_ai_rx_get_frame.h"
#include "ai_rx_player.h"
#include "cpu/includes.h"

#if INTELLIGENT_DUER

#define FIX_LEN 1024
enum stream_node_state ai_rx_get_frame_duer(struct ai_rx_file_handle *hdl, struct stream_frame **pframe)
{
    int available = net_cbuf_data_len();
    int read_size = MIN(available, FIX_LEN);
    if (read_size <= 0) {
        *pframe = NULL;
        return NODE_STA_RUN;
    }
    struct stream_frame *frame = jlstream_get_frame(hdl->node->oport, read_size);
    if (!frame) {
        *pframe = NULL;
        return NODE_STA_RUN;
    }
    int rlen = net_cbuf_read_data(frame->data, read_size);
    frame->len = MAX(rlen, 0);
    if (rlen != read_size) {
        printf("cbuf_read mismatch: req=%d, act=%d\n", read_size, rlen);
    }
    *pframe = frame;
    return NODE_STA_RUN;
}

static void duer_audio_begin(void)
{
    struct ai_rx_player_param param = {0};
    param.coding_type = AUDIO_CODING_STREAM_MP3;
    param.sample_rate = 16000;
    param.channel_mode = AUDIO_CH_MIX;
    param.type = AI_SERVICE_VOICE;
    ai_rx_player_open(NULL, 0, &param);
    extern void audio_app_volume_set(u8 state, s16 volume, u8 fade);
    audio_app_volume_set(1, 100, 1);
}

static void duer_audio_start(void *arg)
{
    duer_audio_begin();
}

void duer_audio_play()
{
    duer_audio_start(NULL);
}

void duer_audio_stop(void *arg)
{
    ai_rx_player_close(0);
}

#endif
