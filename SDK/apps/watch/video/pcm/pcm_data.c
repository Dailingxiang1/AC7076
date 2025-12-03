#include "system/includes.h"
#include "app_config.h"
#include "mic_data.h"
#include "lbuf.h"

// 如果定义了 USE_PSRAM，就把 malloc/free 映射到 malloc_psram/free_psram
#if TCFG_PSRAM_DEV_ENABLE
#define malloc(size)       malloc_psram(size)
#define free(ptr)          free_psram(ptr)
#else
#define malloc(size)       malloc(size)
#define free(ptr)          free(ptr)
#endif


#define offsetof(type, memb) \
	((unsigned long)(&((type *)0)->memb))

enum {
    Q_PCM_TASK_KILL   = (Q_USER + 100),
};
#define USED_JL_STREAM_ENABLE	0
#define MIC_BUFFER_SIZE     4 * 1024
#define MIC_READ_LEN        512

struct lbuf_data_head {
    int len;
    u8 data[0];
};

struct pcm_data_hdl {
    void *mic;
    int frame_size;
    int sample_rate;
    u8 *lbuf_ptr;
    struct lbuff_head *lbuf_handle;

    OS_SEM task_kill_sem;
    char task_name[32];
};

static void pcm_data_task(void *priv)
{
    printf("pcm data task run !\n");
    struct pcm_data_hdl *hdl = (struct pcm_data_hdl *)priv;

    struct lbuf_data_head *lbuf_data = NULL;


    int read_szie = 0;
    int msg[8];

    while (1) {
        if (os_taskq_accept(ARRAY_SIZE(msg), msg) == OS_TASKQ) {
            if (msg[0] == Q_PCM_TASK_KILL) {
                printf("pcm data task exit !\n");
                os_sem_post((OS_SEM *)msg[1]);
                os_time_dly(-1);
            }
        }

        if (read_szie == 0 && !lbuf_data) {
            lbuf_data = lbuf_alloc(hdl->lbuf_handle, hdl->frame_size);
            if (!lbuf_data) {
                printf("pcm data lbuf_alloc err \n");
                os_time_dly(1);
                continue;
            }
        } else if (read_szie == hdl->frame_size) {
            lbuf_data->len = hdl->frame_size;
            lbuf_push(lbuf_data, BIT(0));
            lbuf_data = NULL;
            read_szie = 0;
            continue;
        }

        int remain_size = hdl->frame_size - read_szie;
        int rlen =  remain_size < MIC_READ_LEN ?  remain_size : MIC_READ_LEN;

        u8 *outbuf = lbuf_data->data + read_szie;
#if USED_JL_STREAM_ENABLE
        int mic_data_cbuf_read(void *buf, int len);
        int len =  mic_data_cbuf_read(outbuf, rlen);
#else
        int len = mic_data_read(hdl->mic, outbuf, rlen);
#endif
        if (!len) {
            uint32_t delay_ms = ((float)MIC_READ_LEN / (hdl->sample_rate * 2)) * 1000; //默认16位深
            uint32_t tick = delay_ms / 10;
            os_time_dly(tick + 1);
            /* putchar('\n'); */
            /* putchar('C'); */
        } else {
            read_szie += len;
            /* printf("read_szie:%d len:%d \n", read_szie, len); */
        }
    }
}


void *pcm_data_init(int sample_rate, int frame_size, int buf_size)
{
    struct pcm_data_hdl *hdl = malloc(sizeof(struct pcm_data_hdl));
    if (!hdl) {
        printf("pcm data hdl malloc fail \n");
        goto err;
    }
    memset(hdl, 0x00, sizeof(struct pcm_data_hdl));

    hdl->sample_rate = sample_rate;
    hdl->frame_size = frame_size;

    hdl->lbuf_ptr = malloc(buf_size);
    if (!hdl->lbuf_ptr) {
        printf("pcm data lbuf_ptr malloc fail \n");
        goto err;
    }

    hdl->lbuf_handle = lbuf_init(hdl->lbuf_ptr, buf_size, 4, sizeof(struct lbuf_data_head));
#if USED_JL_STREAM_ENABLE
    hdl->mic = NULL;
    //开音频流
    int avi_video_recoder_open();
    if (avi_video_recoder_open()) {
        printf("%s avi_video_recoder err\n", __func__);
        goto err;
    }
#else
    hdl->mic = mic_data_open(VOICE_MCU_MIC, MIC_BUFFER_SIZE, sample_rate);
    if (!hdl->mic) {
        printf("mic data open err \n");
        goto err;
    }
#endif
    snprintf(hdl->task_name, sizeof(hdl->task_name), "pcm_data_task");

    os_sem_create(&hdl->task_kill_sem, 0);

    if (os_task_create(pcm_data_task, hdl, 10, 1024, 1024, hdl->task_name)) {
        printf("pcm data task create fail \n");
        goto err;
    }

    return (void *)hdl;
err:
    if (hdl) {
        if (hdl->mic) {
            mic_data_close(hdl->mic);
        }
#if USED_JL_STREAM_ENABLE
        void avi_video_recoder_close();
        avi_video_recoder_close();
#endif
        if (hdl->lbuf_ptr) {
            free(hdl->lbuf_ptr);
        }
        free(hdl);
    }
    return NULL;
}

void pcm_data_exit(void *data_hdl)
{
    if (!data_hdl) {
        return;
    }
    struct pcm_data_hdl *hdl = (struct pcm_data_hdl *)data_hdl;

    int msg = (int)&hdl->task_kill_sem;
    os_taskq_post_type(hdl->task_name, Q_PCM_TASK_KILL, 1, &msg);
    os_sem_pend(&hdl->task_kill_sem, 0);
    os_sem_del(&hdl->task_kill_sem, OS_DEL_ALWAYS);
    task_kill(hdl->task_name);

    if (hdl->mic) {
        mic_data_close(hdl->mic);
    }
#if USED_JL_STREAM_ENABLE
    void avi_video_recoder_close();
    avi_video_recoder_close();
#endif
    if (hdl->lbuf_ptr) {
        free(hdl->lbuf_ptr);
    }

    free(hdl);
}

int pcm_data_read(void *data_hdl, u8 **pp_data, int *p_len)
{
    struct lbuf_data_head *node;

    if (!data_hdl || !pp_data || !p_len) {
        printf("pcm data read ptr err \n");
        return -1;
    }

    struct pcm_data_hdl *hdl = (struct pcm_data_hdl *)data_hdl;

    node = lbuf_pop(hdl->lbuf_handle, BIT(0));
    if (!node) {
        return -2;
    }

    *pp_data = node->data;
    *p_len   = node->len;
    return 0;
}

int pcm_data_read_done(void *data_hdl, u8 *data_buf)
{
    struct lbuf_data_head *node;

    if (!data_hdl || !data_buf) {
        printf("pcm data read done ptr err \n");
        return -1;
    }

    struct pcm_data_hdl *hdl = (struct pcm_data_hdl *)data_hdl;

    /* offsetof(struct lbuf_data_head, data) 就是 data 字段相对于结构体开头的字节偏移 */
    node = (struct lbuf_data_head *)((u8 *)data_buf - offsetof(struct lbuf_data_head, data));

    lbuf_free(node);
    return 0;
}


// demo
// os_task_create(pcm_data_demo,NULL,10,2048,1024,"pcm_data_demo");
void pcm_data_demo(void *priv)
{
    printf("pcm data dem run !\n");

    int frame_size = 4096;
    int sample_rate = 8000;
    void *hdl = pcm_data_init(sample_rate, frame_size, frame_size * 10);
    u8 *pcm;
    int len;

    void *fp = (void *)fopen("storage/sd0/C/output.pcm", "w+");
    if (!fp) {
        printf("pcm data demo fopen err \n");
        goto exit;
    }

    int frame_cnt = 4 * 100;

    while (1) {
        if (!frame_cnt) {
            printf("pcm data demo read done \n");
            break;
        }

        if (pcm_data_read(hdl, &pcm, &len) == 0) {
            if (fwrite(pcm, 1, len, fp) != len) {
                printf("pcm data demo fwrite err \n");
            }
            pcm_data_read_done(hdl, pcm);

            frame_cnt--;
        } else {
            /* putchar('D'); */
            uint32_t delay_ms = ((float)frame_size / (sample_rate * 2)) * 1000;
            uint32_t tick = delay_ms / 10;
            os_time_dly(tick + 1);
        }
    }

    fclose(fp);
exit:
    if (hdl) {
        pcm_data_exit(hdl);
    }

    os_time_dly(-1);
}




