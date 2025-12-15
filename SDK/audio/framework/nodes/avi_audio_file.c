#include "system/includes.h"
#include "source_node.h"
#include "app_config.h"
#include "reference_time.h"
#include "avi_audio_player.h"
/* #include "rcsp_translator.h" */

#if TCFG_VIDEO_DEC_NODE_ENABLE

extern u8 avi_audio_start;
extern u8 get_avi_audio_status();

struct avi_audio_file_handle {
    void *file;
    void *bt_addr;
    u8 start;
    u8 source;
    u8 reference;
    struct stream_node *node;
    u32 play_latency; // us
    struct avi_audio_player_param param;
    const struct stream_file_ops *file_ops;
};

extern u32 bt_audio_conn_clock_time(void *addr);
extern cbuffer_t avi_pcm_cbuf;

static int avi_fseek(void *_hdl, u32 fpos)
{
    struct avi_audio_file_handle *hdl = (struct avi_audio_file_handle *)_hdl;

    if (!hdl->file) {
        return -1;
    }
    return hdl->file_ops->seek(hdl->file, fpos, SEEK_SET);
}

static int avi_fread(void *_hdl, u8 *data, int size)
{
    u32 rets_addr;
    __asm__ volatile("%0 = rets ;" : "=r"(rets_addr));
    struct avi_audio_file_handle *hdl = (struct avi_audio_file_handle *)_hdl;

    if (!hdl->file) {
        return 0;
    }
    return hdl->file_ops->read(hdl->file, data, size);
}
static int avi_ioc_get_fmt(struct avi_audio_file_handle *hdl, struct stream_fmt *fmt)
{
    return hdl->file_ops->get_fmt(hdl->file, fmt);
}

static void avi_ioc_set_file(struct avi_audio_file_handle *hdl, struct stream_file_info *info)
{
    hdl->file = info->file;
    hdl->file_ops = info->ops;
}


static enum stream_node_state avi_audio_get_frame(void *file, struct stream_frame **pframe)
{
    struct avi_audio_file_handle *hdl = (struct avi_audio_file_handle *)file;
    struct stream_frame *frame;
    u16 fram_len = 512; // 帧长度

    static int offset = 0;

    frame = jlstream_get_frame(hdl->node->oport, fram_len);
    if (frame == NULL) {
        *pframe = NULL;
        return NODE_STA_RUN;
    }
    // int rlen = fread(frame->data, fram_len, 1, hdl->file);
#if TCFG_VIDEO_DIAL_ENABLE // avi 音频pcm数据读取

    if (get_avi_audio_status()) {
        u32 data_len = cbuf_get_data_size(&avi_pcm_cbuf);
        // printf("[cbuf_get_data_size] data_len:%d\n", data_len);
        if (data_len >= fram_len) {
            cbuf_read(&avi_pcm_cbuf, frame->data, fram_len);
            // printf("[cbuf_read] fram_len:%d\n", fram_len);
            frame->len = fram_len;
            // printf("[avi_audio_get_frame] frame->len:%d\n", frame->len);
        } else {
            // puts("silent audio data\n");
            memset(frame->data, 0, fram_len); // 数据不足时填零静音
            frame->len = 0;//fram_len;
        }
    }

#else
    //如果没有开视屏表盘 用到该流程需要自行添加数据源。
    frame->len = 0;
#endif
    *pframe = frame;
    return NODE_STA_RUN;
}

static void *avi_audio_file_init(void *priv, struct stream_node *node)
{
    struct avi_audio_file_handle *hdl = (struct avi_audio_file_handle *)zalloc(sizeof(struct avi_audio_file_handle));

    hdl->node = node;
    node->type |= NODE_TYPE_FLOW_CTRL; // NODE_TYPE_IRQ;
    return hdl;
}

static int avi_audio_file_set_bt_addr(struct avi_audio_file_handle *hdl, void *bt_addr)
{
    hdl->bt_addr = (void *)bt_addr;

    return 0;
}
static void avi_audio_tick_handler(void *priv, u8 source)
{
    struct avi_audio_file_handle *hdl = (struct avi_audio_file_handle *)priv;

    if (hdl->start && (hdl->node->state & NODE_STA_SOURCE_NO_DATA)) {
        jlstream_wakeup_thread(NULL, hdl->node, NULL);
    }
}

static int avi_audio_file_get_fmt(struct avi_audio_file_handle *hdl, struct stream_fmt *fmt)
{
    fmt->coding_type = hdl->param.coding_type;
    fmt->sample_rate = hdl->param.sample_rate;
    fmt->channel_mode = hdl->param.channel_mode;
    fmt->frame_dms = hdl->param.frame_dms;
    fmt->bit_rate = hdl->param.bit_rate;
    printf("%s  0x%x, %d, %d, %d, %d\n", __FUNCTION__, fmt->coding_type, fmt->sample_rate, fmt->channel_mode, fmt->frame_dms, fmt->bit_rate);
    return 0;
}

static int avi_audio_file_start(struct avi_audio_file_handle *hdl)
{
    if (hdl->start) {
        return 0;
    }

    hdl->start = 1;
    return 0;
}

static int avi_audio_file_stop(struct avi_audio_file_handle *hdl)
{
    if (!hdl->start) {
        return 0;
    }
    hdl->start = 0;
    if (hdl->reference) {
        audio_reference_clock_exit(hdl->reference);
    }
    return 0;
}

static int avi_audio_file_ioctl(void *file, int cmd, int arg)
{
    int err = 0;
    struct avi_audio_file_handle *hdl = (struct avi_audio_file_handle *)file;

    switch (cmd) {
    case NODE_IOC_SET_BTADDR:
        avi_audio_file_set_bt_addr(hdl, (void *)arg);
        break;
    case NODE_IOC_SET_FMT:
        memcpy(&hdl->param, (struct avi_audio_player_param *)arg, sizeof(struct avi_audio_player_param));
        break;
    case NODE_IOC_GET_FMT:
        avi_audio_file_get_fmt(hdl, (struct stream_fmt *)arg);
        break;
    case NODE_IOC_SET_FILE:
        avi_ioc_set_file(hdl, (struct stream_file_info *)arg);
        break;
    case NODE_IOC_START:
        avi_audio_file_start(hdl);
        break;
    case NODE_IOC_SET_PARAM:
        hdl->source = (u8)arg;
        break;
    case NODE_IOC_SUSPEND:
    case NODE_IOC_STOP:
        avi_audio_file_stop(hdl);
        break;
    }

    return 0;
}

static void avi_audio_file_release(void *file)
{
    struct avi_audio_file_handle *hdl = (struct avi_audio_file_handle *)file;

    free(hdl);
}

REGISTER_SOURCE_NODE_PLUG(avi_audio_file_plug) = {
    .uuid = NODE_UUID_VIDEO_DEC,
    .init = avi_audio_file_init,
    .get_frame = avi_audio_get_frame,
    .ioctl = avi_audio_file_ioctl,
    .release = avi_audio_file_release,
};

#endif /*TCFG_avi_audio_NODE_ENABLE*/
