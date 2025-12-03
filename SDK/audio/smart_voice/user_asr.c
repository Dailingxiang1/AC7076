#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".user_asr.data.bss")
#pragma data_seg(".user_asr.data")
#pragma const_seg(".user_asr.text.const")
#pragma code_seg(".user_asr.text")
#endif
/*****************************************************************
>file name : user_asr.c
>create time : Thu 23 Dec 2021 10:00:58 AM CST
>用户自定义语音识别算法平台接入
*****************************************************************/
#include "app_config.h"
#include "system/includes.h"
#include "user_asr.h"
#include "smart_voice.h"
#include "voice_mic_data.h"


#if ((defined TCFG_AUDIO_ASR_DEVELOP) && (TCFG_AUDIO_ASR_DEVELOP == ASR_CFG_USER_DEFINED))

#define ASR_FRAME_SAMPLES   160 /*语音识别帧长（采样点）*/

/*debug调试*/
#define SMART_VOICE_TEST_PRINT_PCM        0
#define SMART_VOICE_TEST_WRITE_FILE       0

#define SMART_VOICE_SAMPLE_RATE           16000
#define SMART_VOICE_REC_MIC_SECS          2 /*记录声音时长*/
#define SMART_VOICE_REC_DATA_LEN          (SMART_VOICE_SAMPLE_RATE * SMART_VOICE_REC_MIC_SECS * 2)


#if SMART_VOICE_TEST_WRITE_FILE
#define VOICE_DATA_BUFFER_SIZE     16 * 1024
#elif SMART_VOICE_TEST_PRINT_PCM
#define VOICE_DATA_BUFFER_SIZE     SMART_VOICE_REC_DATA_LEN
#else
#define VOICE_DATA_BUFFER_SIZE     2 * 1024
#endif


struct user_platform_asr_context {
    void *mic;
    void *core;
    u8 data_enable;
#if SMART_VOICE_TEST_WRITE_FILE
    void *file;
#endif
};

extern const int config_user_asr_enable;
static struct user_platform_asr_context *__this = NULL;


static void user_voice_mic_data_debug_stop_handler(struct user_platform_asr_context *sv);
static inline void user_voice_data_write_file(struct user_platform_asr_context *asr, void *data, int len);
static void user_voice_mic_data_debug_start(struct user_platform_asr_context *asr);


/*
 * 算法引擎打开函数
 */
static void *user_asr_core_open(int sample_rate)
{
    return NULL;
}

/*
 * 算法引擎关闭函数
 */
static void user_asr_core_close(void *core)
{

}

/*
 * 算法引擎数据处理
 */
static int user_asr_core_data_handler(void *core, void *data, int len)
{
    return 0;
}

/*
*********************************************************************
*       user asr data handler
* Description: 用户算法平台平台语音识别数据处理
* Arguments  : asr - 语音识别数据管理结构
* Return	 : 0 - 处理成功， 非0 - 处理失败.
* Note(s)    : 该函数通过读取mic数据送入算法引擎完成语音帧.
*********************************************************************
*/
static int user_asr_data_handler(struct user_platform_asr_context *asr)
{
    if (!asr->mic) {
        return -EINVAL;
    }

#if SMART_VOICE_TEST_PRINT_PCM
    putchar('*');
    return 1;
#endif

    s16 data[ASR_FRAME_SAMPLES];

    int len = voice_mic_data_read(asr->mic, data, sizeof(data));
    if (len < sizeof(data)) {
        return -EINVAL;
    }

#if SMART_VOICE_TEST_WRITE_FILE
    user_voice_data_write_file(asr, data, sizeof(data));
#else
    if (asr->core) {
        user_asr_core_data_handler(asr->core, data, sizeof(data));
    }
#endif

    return 0;
}

/*
*********************************************************************
*       user_asr core handler
* Description: 用户自定义语音识别处理
* Arguments  : priv - 语音识别私有数据
*              taskq_type - TASK消息类型
*              msg  - 消息存储指针(对应自身模块post的消息)
* Return	 : None.
* Note(s)    : 音频平台资源控制以及ASR主要识别算法引擎.
*********************************************************************
*/
int user_asr_core_handler(void *priv, int taskq_type, int *msg)
{
    struct user_platform_asr_context *asr = (struct user_platform_asr_context *)priv;
    int err = ASR_CORE_STANDBY;

    if (taskq_type != OS_TASKQ) {
        return err;
    }

    switch (msg[0]) {
    case SMART_VOICE_MSG_MIC_OPEN: /*语音识别打开 - MIC打开，算法引擎打开*/
        /* msg[1] - MIC数据源，msg[2] - 音频采样率，msg[3] - mic的数据总缓冲长度*/
        asr->mic = voice_mic_data_open(msg[1], msg[2], msg[3]);
        if (!asr->mic) {
            printf("asr mic open failed.\n");
        }
        asr->core = user_asr_core_open(msg[2]);
        asr->data_enable = 1;
        err = ASR_CORE_WAKEUP;
        break;
    case SMART_VOICE_MSG_SWITCH_SOURCE:
        /*这里进行MIC的数据源切换 （主系统MIC或VAD MIC）*/

        break;
    case SMART_VOICE_MSG_WAKE:
        err = ASR_CORE_WAKEUP;
        putchar('W');
        user_voice_mic_data_debug_start(asr);
        asr->data_enable = 1;
        break;
    case SMART_VOICE_MSG_STANDBY:
        asr->data_enable = 0;
        user_voice_mic_data_debug_stop_handler(asr);
        if (asr->mic) {
            voice_mic_data_clear(asr->mic);
        }
        break;
    case SMART_VOICE_MSG_MIC_CLOSE: /*语音识别关闭 - MIC关闭，算法引擎关闭*/
        asr->data_enable = 0;
        /* msg[2] - 信号量*/
        voice_mic_data_close(asr->mic);
        asr->mic = NULL;
        user_asr_core_close(asr->core);
        asr->core = NULL;
        asr->data_enable = 0;
        os_sem_post((OS_SEM *)msg[1]);
        break;
    case SMART_VOICE_MSG_DMA: /*MIC通路数据DMA消息*/
        asr->data_enable = 1;
        err = ASR_CORE_WAKEUP;
        break;
    default:
        break;
    }

    if (asr->data_enable) {
        err = user_asr_data_handler(asr);
        err = err ? ASR_CORE_STANDBY : ASR_CORE_WAKEUP;
    }
    return err;
}

int user_platform_asr_open(void)
{
    if (!config_user_asr_enable) {
        return 0;
    }
    int err = 0;
    struct user_platform_asr_context *asr = (struct user_platform_asr_context *)zalloc(sizeof(struct user_platform_asr_context));

    if (!asr) {
        return -ENOMEM;
    }

    err = smart_voice_core_create(asr);
    if (err != OS_NO_ERR) {
        goto __err;
    }

#if SMART_VOICE_TEST_WRITE_FILE
    extern void force_set_sd_online(char *sdx);
    force_set_sd_online("sd0");
    void *mnt = mount("sd0", "storage/sd0", "fat", 3, NULL);
    if (!mnt) {
        printf("sd0 mount fat failed.\n");
        goto __err;
    }
#endif

    /*
     * 推送MIC的资源打开到语音识别主任务
     * V1.1.0版本支持了低功耗VAD MIC的使用，需要可以改为VAD MIC
     *
     */
    smart_voice_core_post_msg(4, SMART_VOICE_MSG_MIC_OPEN, VOICE_MCU_MIC, VOICE_DATA_BUFFER_SIZE, SMART_VOICE_SAMPLE_RATE);

    __this = asr;
    return 0;
__err:
    if (asr) {
        free(asr);
    }
    return err;
}

void user_platform_asr_close(void)
{
    if (!config_user_asr_enable) {
        return;
    }
    if (__this) {
        OS_SEM *sem = (OS_SEM *)malloc(sizeof(OS_SEM));
        os_sem_create(sem, 0);
        smart_voice_core_post_msg(2, SMART_VOICE_MSG_MIC_CLOSE, (int)sem);
        os_sem_pend(sem, 0);
        free(sem);
        smart_voice_core_free();
        free(__this);
        __this = NULL;
    }
}


void user_voice_mic_data_debug_stop(void)
{
    smart_voice_core_post_msg(1, SMART_VOICE_MSG_STANDBY);
}


static void user_voice_mic_data_debug_stop_handler(struct user_platform_asr_context *asr)
{
#if SMART_VOICE_TEST_PRINT_PCM
    if (asr->mic) {
        voice_mic_data_dump(asr->mic);
    }
#endif
#if SMART_VOICE_TEST_WRITE_FILE
    if (asr->file) {
        fclose(asr->file);
        asr->file = NULL;
    }
#endif
}

static inline void user_voice_data_write_file(struct user_platform_asr_context *asr, void *data, int len)
{
#if SMART_VOICE_TEST_WRITE_FILE
    if (asr->file) {
        int ret = fwrite(data, len, 1, asr->file);
        if (ret <= 0) {
            printf("%s write error!!!", __func__);
        }
    }
#endif
}

static void user_voice_mic_data_debug_start(struct user_platform_asr_context *asr)
{
#if SMART_VOICE_TEST_WRITE_FILE
    asr->file = fopen("storage/sd0/C/AudioVAD/vad***.raw", "w+");
    if (!asr->file) {
        printf("Open file failed, can not test.\n");
    }
#endif
}


static u8 user_asr_idle_query(void)
{
    if (__this) {
        return !__this->data_enable;
    } else {
        return 1;
    }
}
REGISTER_LP_TARGET(user_asr_lp_target) = {
    .name = "smart_voice",
    .is_idle = user_asr_idle_query,
};

int audio_smart_voice_check_status(void)
{
    return (__this ? true : false);
}

#endif
