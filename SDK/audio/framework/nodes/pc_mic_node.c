#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".pc_mic_node.data.bss")
#pragma data_seg(".pc_mic_node.data")
#pragma const_seg(".pc_mic_node.text.const")
#pragma code_seg(".pc_mic_node.text")
#endif
#include "jlstream.h"
#include "media/audio_base.h"
#include "sync/audio_syncts.h"
#include "circular_buf.h"
#include "audio_splicing.h"
#include "app_config.h"
#include "gpio.h"
#include "audio_cvp.h"
#include "media/audio_general.h"
#include "pc_mic_recoder.h"
#include "uac_stream.h"
#include "effects/convert_data.h"

#if TCFG_USB_SLAVE_AUDIO_MIC_ENABLE

#define PC_MIC_BUF_SIZE   			(1024 * 4)
#define PC_MIC_SINGLE2DUAL_BUF_LEN	(1024 * 2)
#define PC_MIC_3BYTE_24BIT_BUF_LEN	(1024 * 3) //单转双buf的3/2倍

static u8 pc_mic_node_get_data_flag = 0;

struct pc_mic_node_hdl {
    u8 start;
    u8 cache_flag;
    u8 iport_channel_mode;	//保存输入节点的声道数
    u8 iport_bit_width;     //保存输入节点的位宽
    u8 tmp_buf[PC_MIC_SINGLE2DUAL_BUF_LEN];		//单转双用到的数组
    u8 pcm_3byte_24bit_buf[PC_MIC_3BYTE_24BIT_BUF_LEN]; //16bit转3byte_24bit的buf
    u32 input_rate;
    u32 output_rate;
    struct stream_node *node;
    struct stream_frame *frame;
    enum stream_scene scene;
    cbuffer_t pc_mic_cache_cbuffer;
    u8 *cache_buf;
};

struct pc_mic_fmt_t {
    u8 init;
    u8 channel;
    u8 bit;
    u32 sample_rate;
};
struct pc_mic_fmt_t pc_mic_fmt = {
    .init = 0,
    .channel = MIC_CHANNEL,
    .bit = MIC_AUDIO_RES,
    .sample_rate = MIC_AUDIO_RATE,
};

static DEFINE_SPINLOCK(pc_mic_lock);

//pc mic 驱动拿数接口
static int pc_mic_tx_handler(void *priv, void *buf, int len)
{
    struct pc_mic_node_hdl *hdl = (struct pc_mic_node_hdl *)priv;
    int rlen = 0;
    /* spin_lock(&pc_mic_lock); */
    if (hdl) {
        if (hdl->start == 1 && hdl->cache_flag == 1 && hdl->cache_buf) {
            rlen = cbuf_read(&hdl->pc_mic_cache_cbuffer, buf, len);
        }
    }
    /* spin_unlock(&pc_mic_lock); */
    return rlen;
}

// 数据流节点回调，做数据缓存
__STREAM_CACHE_CODE
static void pc_mic_handle_frame(struct stream_iport *iport, struct stream_note *note)
{
    struct pc_mic_node_hdl *hdl = (struct pc_mic_node_hdl *)iport->private_data;
    /* struct stream_frame *frame = NULL; */
    /* struct stream_node *node = iport->node; */

    if (pc_mic_node_get_data_flag == 0) {
        pc_mic_node_get_data_flag = 1;
    }
    if (hdl->start == 0) {
        return;
    }

    while (1) {
        if (hdl->frame == NULL) {
            hdl->frame = jlstream_pull_frame(iport, NULL);
            if (!hdl->frame) {
                return;
            }
            hdl->frame->offset = 0;
        }
        s16 *data = (s16 *)(hdl->frame->data + hdl->frame->offset);
        u32 remain = hdl->frame->len - hdl->frame->offset;
        int wlen = 0;

        spin_lock(&pc_mic_lock);

        //声道处理
#if (MIC_CHANNEL == 1)	//mic的声道是单声道
        if (AUDIO_CH_NUM(hdl->iport_channel_mode) == 2) {
            //mic声道是单声道，输入声道是双声道，需要双转单
            if (hdl->iport_bit_width) {
                pcm_dual_to_single_32bit(data, data, remain);
            } else {
                pcm_dual_to_single(data, data, remain);
            }
            remain /= 2;
        }
#elif (MIC_CHANNEL == 2)	//mic的声道是双声道
        if (AUDIO_CH_NUM(hdl->iport_channel_mode) == 1) {
            //mic声道是双声道，输入声道是单声道，需要单转双
            s16 *src = data;
            s16 *dst = (s16 *)(hdl->tmp_buf);
            remain = (remain > PC_MIC_SINGLE2DUAL_BUF_LEN) ? PC_MIC_SINGLE2DUAL_BUF_LEN : remain;
            if (hdl->iport_bit_width) {
                pcm_single_to_dual_32bit(dst, src, remain);
            } else {
                pcm_single_to_dual(dst, src, remain);
            }
            remain = remain * 2;
            data = (s16 *)(hdl->tmp_buf);
        }
#endif

        //位宽处理
        if (pc_mic_fmt.bit == 16) {
            if (hdl->iport_bit_width) {
                //输入位宽是4byte_24bit
                audio_convert_data_32bit_to_16bit_round((s32 *)data, (s16 *)data, remain / 4);
                remain /= 2;
            }
        } else if (pc_mic_fmt.bit == 24) {
            if (hdl->iport_bit_width) {
                //输入位宽是4byte_24bit
                audio_convert_data_4byte24bit_to_3byte24bit((s32 *)data, (s32 *)data, remain / 4);
                remain = remain * 3 / 4;
            } else {
                //输入位宽是16bit
                remain = (remain > PC_MIC_3BYTE_24BIT_BUF_LEN) ? PC_MIC_3BYTE_24BIT_BUF_LEN : remain;
                audio_convert_data_16bit_to_3byte24bit((s16 *)data, (s32 *)hdl->pcm_3byte_24bit_buf, remain / 2);
                remain = remain * 3 / 2;
                data = (s16 *)hdl->pcm_3byte_24bit_buf;
            }
        }

        wlen = cbuf_write(&hdl->pc_mic_cache_cbuffer, data, remain);
        if (wlen != remain) {
            putchar('w');
        }

        if (hdl->cache_flag == 0) {
            if (cbuf_get_data_len(&hdl->pc_mic_cache_cbuffer) >= PC_MIC_BUF_SIZE / 2) {
                hdl->cache_flag = 1;
            }
        }
        jlstream_free_frame(hdl->frame);
        hdl->frame = NULL;
        spin_unlock(&pc_mic_lock);
    }
}


static void pc_mic_ioc_start(struct pc_mic_node_hdl *hdl)
{
    spin_lock(&pc_mic_lock);
    if (hdl->start == 0) {
        y_printf("## Enter Func:%s, Line:%d\n", __func__, __LINE__);
        if (!hdl->cache_buf) {
            hdl->cache_buf = malloc(PC_MIC_BUF_SIZE);
            if (!hdl->cache_buf) {
                r_printf("Error, Func:%s, Line:%d\n", __func__, __LINE__);
                spin_unlock(&pc_mic_lock);
                return;
            }
            cbuf_init(&hdl->pc_mic_cache_cbuffer, hdl->cache_buf, PC_MIC_BUF_SIZE);
            set_uac_mic_tx_handler((void *)hdl, pc_mic_tx_handler);
        }
        hdl->cache_flag = 0;
        hdl->start = 1;
    }
    spin_unlock(&pc_mic_lock);
}

static void pc_mic_ioc_stop(struct pc_mic_node_hdl *hdl)
{
    if (hdl->start == 1) {
        hdl->cache_flag = 0;
        hdl->start = 0;
    }
    pc_mic_node_get_data_flag = 0;
}


static void pc_mic_adapter_open_iport(struct stream_iport *iport)
{
    iport->handle_frame = pc_mic_handle_frame;
    iport->private_data = iport->node->private_data;
}

static int pc_mic_ioc_negotiate(struct stream_iport *iport)
{
    y_printf("## Enter Func : %s\n", __func__);
    int ret = NEGO_STA_ACCPTED;
    struct stream_fmt *in_fmt = &iport->prev->fmt;
    struct pc_mic_node_hdl *hdl = (struct pc_mic_node_hdl *)iport->private_data;
    u32 coding_type = in_fmt->coding_type;
    u8 channel_mode = in_fmt->channel_mode;
    int output_rate = in_fmt->sample_rate;
    u8 bit_width = in_fmt->bit_wide;

    if (hdl->scene != STREAM_SCENE_PC_MIC) {
        r_printf("## Error , Func:%s, Line:%d\n", __func__, __LINE__);
        ret = NEGO_STA_ABORT;
    }
    if (coding_type != AUDIO_CODING_PCM) {
        r_printf("## Error , Func:%s, Line:%d\n", __func__, __LINE__);
        ret = NEGO_STA_ABORT;
    }
    if (output_rate != pc_mic_get_fmt_sample_rate()) {
        in_fmt->sample_rate = pc_mic_get_fmt_sample_rate();
        r_printf("## Error , Func:%s, Line:%d\n", __func__, __LINE__);
        /* ret = NEGO_STA_ABORT; */
        ret = NEGO_STA_CONTINUE;
    }

    hdl->iport_channel_mode = channel_mode;
    hdl->output_rate = output_rate;
    hdl->iport_bit_width = bit_width;
    g_printf("## Func:%s, negotiate_state : %d, output_rate:%d, channel_mode:%d\n", __func__, ret, output_rate, channel_mode);
    return ret;
}


static int pc_mic_adapter_ioctl(struct stream_iport *iport, int cmd, int arg)
{
    int ret = 0;
    struct pc_mic_node_hdl *hdl = (struct pc_mic_node_hdl *)iport->private_data;

    switch (cmd) {
    case NODE_IOC_OPEN_IPORT:
        pc_mic_adapter_open_iport(iport);
        break;
    case NODE_IOC_NEGOTIATE:
        *(int *)arg |= pc_mic_ioc_negotiate(iport);
        break;
    case NODE_IOC_SET_SCENE:
        hdl->scene = arg;
        break;
    case NODE_IOC_GET_DELAY:
        break;
    case NODE_IOC_START:
        pc_mic_ioc_start(hdl);
        break;
    case NODE_IOC_SUSPEND:
    case NODE_IOC_STOP:
        pc_mic_ioc_stop(hdl);
        break;
    case NODE_IOC_SYNCTS:
        break;
    default:
        break;
    }
    return ret;
}



static int pc_mic_adapter_bind(struct stream_node *node, u16 uuid)
{
    spin_lock(&pc_mic_lock);
    struct pc_mic_node_hdl *hdl = zalloc(sizeof(*hdl));
    ASSERT(hdl, "%s hdl = NULL!\n", __func__);
    hdl->node = node;
    node->private_data = hdl;
    spin_unlock(&pc_mic_lock);
    return 0;
}

static void pc_mic_adapter_release(struct stream_node *node)
{
    struct pc_mic_node_hdl *hdl = (struct pc_mic_node_hdl *)node->private_data;
    if (hdl) {
        spin_lock(&pc_mic_lock);
        pc_mic_ioc_stop(hdl);
        if (hdl->cache_buf) {
            free(hdl->cache_buf);
            hdl->cache_buf = NULL;
        }
        free(hdl);
        hdl = NULL;
        spin_unlock(&pc_mic_lock);
    }
}

// 返回1代表数据流有跑起来
u8 pc_mic_get_node_state(void)
{
    return pc_mic_node_get_data_flag;
}

// 设置pc mic 的数据格式，传入0不设置
void pc_mic_set_fmt(u8 channel, u8 bit, u32 sample_rate)
{
    y_printf("----------- Call %s, bit:%d, sr:%d\n", __func__, bit, sample_rate);
    pc_mic_fmt.init = 1;
    if (channel != 0) {
        pc_mic_fmt.channel = channel;
    }
    if (bit != 0) {
        pc_mic_fmt.bit = bit;
    }
    if (sample_rate != 0) {
        pc_mic_fmt.sample_rate = sample_rate;
    }
}

u32 pc_mic_get_fmt_sample_rate(void)
{
    y_printf("Mic Sr : %d\n", pc_mic_fmt.sample_rate);
    return pc_mic_fmt.sample_rate;
}


REGISTER_STREAM_NODE_ADAPTER(pc_mic_node_adapter) = {
    .name       = "pc_mic",
    .uuid       = NODE_UUID_PC_MIC,
    .bind       = pc_mic_adapter_bind,
    .ioctl      = pc_mic_adapter_ioctl,
    .release    = pc_mic_adapter_release,
};

#endif




