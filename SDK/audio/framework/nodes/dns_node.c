#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".dns_node.data.bss")
#pragma data_seg(".dns_node.data")
#pragma const_seg(".dns_node.text.const")
#pragma code_seg(".dns_node.text")
#endif

#include "jlstream.h"
#include "audio_config.h"
#include "media/audio_base.h"
#include "audio_ns.h"
#include "btstack/avctp_user.h"
#include "effects/effects_adj.h"
#include "frame_length_adaptive.h"
#include "app_config.h"
#include "overlay_code.h"

#if 1
#define ns_log	printf
#else
#define ns_log(...)
#endif/*log_en*/

#if TCFG_DNS_NODE_ENABLE

extern void aec_code_movable_load(void);
extern void aec_code_movable_unload(void);

enum {
    AUDIO_NS_TYPE_ESCO_DL = 1,	//下行降噪类型
    AUDIO_NS_TYPE_GENERAL,		//通用类型
};

struct ns_cfg_t {
    u8 ns_type;//降噪类型选择，通用降噪/通话下行降噪
    u8 call_active_trigger;//接通电话触发标志, 只有通话下行降噪会使用
    float aggressfactor;   //降噪强度(越大越强:1~2)
    float minsuppress;     //降噪最小压制(越小越强:0~1)
    float noiselevel;      //初始噪声水平(评估初始噪声，加快收敛)
} __attribute__((packed));

struct dns_node_hdl {
    u16 sr;
    void *dns;
    struct stream_frame *out_frame;
    struct stream_node *node;		//节点句柄
    struct ns_cfg_t cfg;
    struct frame_length_adaptive_hdl *olen_adaptive;//输出长度适配
};

extern int db2mag(int db, int dbQ, int magDQ);//10^db/20
int dns_param_cfg_read(struct stream_node *node)
{
    struct ns_cfg_t config;
    struct dns_node_hdl *hdl = (struct dns_node_hdl *)node->private_data;
    int ret = 0;
    if (!hdl) {
        return -1 ;
    }
    /*
     *获取配置文件内的参数,及名字
     * */
    ret = jlstream_read_node_data_new(NODE_UUID_DNS_NOISE_SUPPRESSOR, node->subid, (void *)&config, NULL);
    if (ret != sizeof(config)) {
        printf("%s, read node data err %d = %d\n", __FUNCTION__, ret, (int)sizeof(config));
        return -1 ;
    }

    hdl->cfg = config;

    ns_log("type %d\n", hdl->cfg.ns_type);
    ns_log("call_active_trigger %d\n", hdl->cfg.call_active_trigger);
    ns_log("aggressfactor  %d/1000\n", (int)(hdl->cfg.aggressfactor * 1000.f));
    ns_log("minsuppress    %d/1000\n", (int)(hdl->cfg.minsuppress * 1000.f));
    ns_log("noiselevel     %d/1000\n", (int)(hdl->cfg.noiselevel * 1000.f));

    return ret;
}

/*节点输出回调处理，可处理数据或post信号量*/
static void dns_handle_frame(struct stream_iport *iport, struct stream_note *note)
{
    struct dns_node_hdl *hdl = (struct dns_node_hdl *)iport->private_data;
    struct stream_node *node = iport->node;
    struct stream_frame *in_frame;
    int wlen;
    u8 trigger = 1;
    int out_frame_len = 0;

    out_frame_len = DNS_FRAME_SIZE;

    while (1) {
        in_frame = jlstream_pull_frame(iport, note);		//从iport读取数据
        if (!in_frame) {
            break;
        }

        if (hdl->cfg.ns_type == AUDIO_NS_TYPE_ESCO_DL) {
            if (hdl->cfg.call_active_trigger) {
                if (bt_get_call_status() != BT_CALL_ACTIVE) {
                    trigger = 0;
                }
            }
        }
        if (trigger) {
            /*接通的时候再开始做降噪*/
            out_frame_len = in_frame->len > out_frame_len ? in_frame->len : out_frame_len;
            if (!hdl->out_frame) {
                hdl->out_frame = jlstream_get_frame(node->oport, out_frame_len);
                if (!hdl->out_frame) {
                    jlstream_return_frame(iport, in_frame);
                    return;
                }
            }
            wlen = audio_dns_run(hdl->dns, (s16 *)in_frame->data,
                                 (s16 *)hdl->out_frame->data, in_frame->len);

            if (!hdl->olen_adaptive) {
                hdl->olen_adaptive = frame_length_adaptive_open(in_frame->len, out_frame_len);
            }
            int len = frame_length_adaptive_run(hdl->olen_adaptive, (s16 *)hdl->out_frame->data, (s16 *)hdl->out_frame->data, wlen);

            if (len) {
                hdl->out_frame->len = len;
                jlstream_push_frame(node->oport, hdl->out_frame);	//将数据推到oport
                hdl->out_frame = NULL;
            }
            jlstream_free_frame(in_frame);	//释放iport资源
        } else {
            jlstream_push_frame(node->oport, in_frame);	//将数据推到oport
        }
    }
}

/*节点预处理-在ioctl之前*/
static int dns_adapter_bind(struct stream_node *node, u16 uuid)
{
    struct dns_node_hdl *hdl = malloc(sizeof(*hdl));

    memset(hdl, 0, sizeof(*hdl));

    hdl->node = node;
    node->private_data = hdl;	//保存私有信息
    dns_param_cfg_read(node);

    return 0;
}

/*打开改节点输入接口*/
static void dns_ioc_open_iport(struct stream_iport *iport)
{
    iport->handle_frame = dns_handle_frame;				//注册输出回调
    iport->private_data = iport->node->private_data;	//保存节点私有句柄
}

/*节点参数协商*/
static int dns_ioc_negotiate(struct stream_iport *iport)
{
    return 0;
}

/*节点start函数*/
static void dns_ioc_start(struct dns_node_hdl *hdl)
{
    struct stream_fmt *fmt = &hdl->node->oport->fmt;
    printf("dns node start");

    dns_param_t param = {
        .DNS_OverDrive = hdl->cfg.aggressfactor,
        .DNS_GainFloor = hdl->cfg.minsuppress,
        .DNS_NoiseLevel = hdl->cfg.noiselevel,
        .DNS_highGain = 2.5f,
        .DNS_rbRate = 0.3f,
        .sample_rate = fmt->sample_rate,
    };
    overlay_load_code(OVERLAY_AEC);
    aec_code_movable_load();
    /*打开算法*/
    hdl->dns = audio_dns_open(&param);
}

/*节点stop函数*/
static void dns_ioc_stop(struct dns_node_hdl *hdl)
{
    if (hdl->dns) {
        audio_dns_close(hdl->dns);
        hdl->dns = NULL;
        aec_code_movable_unload();
    }
    if (hdl->olen_adaptive) {
        frame_length_adaptive_close(hdl->olen_adaptive);
        hdl->olen_adaptive = NULL;
    }
    if (hdl->out_frame) {
        jlstream_free_frame(hdl->out_frame);
        hdl->out_frame = NULL;
    }
}

/*节点ioctl函数*/
static int dns_adapter_ioctl(struct stream_iport *iport, int cmd, int arg)
{
    int ret = 0;
    struct dns_node_hdl *hdl = (struct dns_node_hdl *)iport->private_data;

    switch (cmd) {
    case NODE_IOC_OPEN_IPORT:
        dns_ioc_open_iport(iport);
        break;
    case NODE_IOC_NEGOTIATE:
        *(int *)arg |= dns_ioc_negotiate(iport);
        break;
    case NODE_IOC_START:
        dns_ioc_start(hdl);
        break;
    case NODE_IOC_SUSPEND:
    case NODE_IOC_STOP:
        dns_ioc_stop(hdl);
        break;
    }

    return ret;
}

/*节点用完释放函数*/
static void dns_adapter_release(struct stream_node *node)
{
    struct dns_node_hdl *hdl = (struct dns_node_hdl *)node->private_data;
    if (!hdl) {
        return;
    }
    free(hdl);
}

/*节点adapter 注意需要在sdk_used_list声明，否则会被优化*/
REGISTER_STREAM_NODE_ADAPTER(dns_node_adapter) = {
    .name       = "dns",
    .uuid       = NODE_UUID_DNS_NOISE_SUPPRESSOR,
    .bind       = dns_adapter_bind,
    .ioctl      = dns_adapter_ioctl,
    .release    = dns_adapter_release,
};

//注册工具在线调试
REGISTER_ONLINE_ADJUST_TARGET(dns_noise_suppressor) = {
    .uuid = NODE_UUID_DNS_NOISE_SUPPRESSOR,
};

#endif/* TCFG_DNS_NODE_ENABLE*/

