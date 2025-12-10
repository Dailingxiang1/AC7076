#include "avilib.h"
#include "media/includes.h"
#include "audio_config.h"
#include "avi_audio_player.h"
//===========================================================================
//			使用avilib解码参考
//===========================================================================
#if TCFG_PSRAM_DEV_ENABLE
#define malloc(size)       malloc_psram(size)
#define free(ptr)          free_psram(ptr)
#else
#define malloc(size)       malloc(size)
#define free(ptr)          free(ptr)
#endif


extern cbuffer_t avi_pcm_cbuf;
extern int jljpeg_stream_src_data_copy(u8 *buf, int size);
#define VIDEO_DEC_TASK_NAME  "VIDEO_DEC"
#define PATH_LEN 64
#define PCM_CBUF_SIZE (12*1024)


struct video_player {
    s8(*path)[64];
    u8 path_number;
    u8 path_index;
    u8 play_mode;
    u8 play_status;
    int frame_index;
    int frame_rate;
    int video_chunks;
    int audio_chunks;
    u32 start_msec;
    u32 pause_msec;
    u32 curr_msec;
    u32 audio_offset;
    u16 tick_id;
    void *dec_hd;
    void (*ui_refresh_cb)(int status);
    u8 *audio_buf;
    OS_SEM task_kill_sem;
} *__player;

#define __this (__player)

enum {
    VIDEO_DEC_MSG_INIT,
    VIDEO_DEC_MSG_EXIT,
    VIDEO_DEC_MSG_START,
    VIDEO_DEC_MSG_STOP,
    VIDEO_DEC_MSG_UPDATE_FILE,
    VIDEO_DEC_MSG_TICK,
    VIDEO_DEC_MSG_SUSPEND,
    VIDEO_DEC_MSG_RESUME,
};
enum {
    VIDEO_DEC_PLAY_MODE_SINGLE,//单次播放
    VIDEO_DEC_PLAY_MODE_SINGLE_LOOP,//单次循环
    VIDEO_DEC_PLAY_MODE_LIST,//列表播放，不循环
    VIDEO_DEC_PLAY_MODE_LIST_LOOP,//列表播放，循环
};
#if 0//使用video_dec解码时打开
u8 get_avi_audio_status()
{
    if (__this == NULL) {
        return 0;
    }
    return 1;
}
#endif
static int video_msg_post(int msg, void  *priv, int priv_len)
{
    int err = os_taskq_post_type(VIDEO_DEC_TASK_NAME, msg, priv_len, priv);
    if (err) {
        printf("%s err::%d\n", __func__, err);
    }
    return  err;
}

static int video_dec_player_update_dec_hd()
{
    if (__this->dec_hd) {
        AVI_close(__this->dec_hd);
        __this->dec_hd = NULL;
    }
    printf("%s path:%s\n", __func__, (char *)__this->path[__this->path_index]);
    __this->dec_hd = AVI_open_input_file((char *)__this->path[__this->path_index], 1);
    __this->frame_rate =  AVI_frame_rate(__this->dec_hd);
    __this->audio_chunks = AVI_audio_chunks(__this->dec_hd);
    __this->video_chunks = AVI_video_frames(__this->dec_hd);
    __this->frame_index = 0;
    return 0;
}
static void video_dec_tick_cb(void *p)
{
    video_msg_post(VIDEO_DEC_MSG_TICK, NULL, 0);
}
int video_dec_tick_update()
{
    video_msg_post(VIDEO_DEC_MSG_TICK, NULL, 0);
    return 0;
}
static int video_dec_player_tick()
{
    if (!__this->frame_index) {
        __this->start_msec = jiffies_msec();
    }
    __this->curr_msec = jiffies_msec2offset(__this->start_msec, jiffies_msec());
    int real_frame_index = __this->curr_msec * __this->frame_rate / 1000;
    /* printf("%s rf:%d total:%d",__func__,real_frame_index,__this->video_chunks); */
    if (real_frame_index >= __this->video_chunks) {
        //stop
        if (__this->tick_id) {
            usr_timer_del(__this->tick_id);
            __this->tick_id = 0;
        }
        return -1;
    }
    AVI_set_video_position(__this->dec_hd, real_frame_index);
    int keyframe;
    u32 offset = 0;
    int frame_len = AVI_frame_size(__this->dec_hd, real_frame_index);
    u8 *file_buf = malloc(20 * 1024);
    if (frame_len <= 20 * 1024) {
        int file_len = AVI_read_frame(__this->dec_hd, (char *)file_buf, &keyframe);
        //copy_to_ui
        if (!jljpeg_stream_src_data_copy(file_buf, file_len)) {
            if (__this->ui_refresh_cb) {
                __this->ui_refresh_cb(0);
            }
        }
    }
    int exit =  0;
    if (__this->video_chunks) {
        real_frame_index = real_frame_index * __this->audio_chunks / __this->video_chunks;
    }
    /* printf("%s %d->%d(%d)",__func__,__this->frame_index,real_frame_index,__this->audio_chunks); */
    for (int i = __this->frame_index ; i < real_frame_index; i++) {
        int audio_chunk_size =  AVI_audio_size(__this->dec_hd, i);
        AVI_set_audio_position(__this->dec_hd, __this->audio_offset);
        int file_len = AVI_read_audio_chunk(__this->dec_hd, (char *)file_buf);
        //copy_to_pcm_player
        int len = file_len;
        u8 *ptr = file_buf;
        while (len) {
            int w = cbuf_write(&avi_pcm_cbuf, ptr, len);
            /* printf("[w] len:%d wl:%d",len,w); */
            if (w == 0) {
                /* os_time_dly(1); */
                exit = 1;
                break;
            }
            ptr += w;
            len -= w;
        }
        if (exit) {
            __this->frame_index  = i;
            break;
        }
        /* printf("[cw] i:%d chunks:%d filelen:%d",i,audio_chunk_size,file_len); */
        //todo
        __this->audio_offset += audio_chunk_size;
    }
    if (!exit) {
        __this->frame_index = real_frame_index;
    }

    if (!__this->frame_index) {
        __this->frame_index ++;
    }
    free(file_buf);

    return 0;
}
static int video_dec_player_start()
{
    int msec = (int)1000 / __this->frame_rate; // /2?
    if (__this->tick_id) {
        usr_timer_del(__this->tick_id);
    }

    __this->tick_id = usr_timer_add(NULL, video_dec_tick_cb, msec, 1);


    app_audio_state_switch(APP_AUDIO_STATE_MUSIC, app_audio_volume_max_query(AppVol_MUSIC), NULL);   // 音量状态设置
    audio_dac_set_volume(&dac_hdl, app_audio_volume_max_query(AppVol_MUSIC) / 100);                  // dac 音量设置

    struct avi_audio_player_param param = {0};
    param.coding_type = AUDIO_CODING_PCM;
    param.channel_mode = (AVI_audio_channels(__this->dec_hd) == 1) ? AUDIO_CH_L : AUDIO_CH_MIX;
    param.sample_rate = AVI_audio_rate(__this->dec_hd);
    param.bit_rate = AVI_audio_channels(__this->dec_hd) * AVI_audio_rate(__this->dec_hd) * AVI_audio_bits(__this->dec_hd);
    param.type = AVI_SERVICE_VOICE;
    printf("simple_rate:%d channel:%d bitrate:%d", param.sample_rate, param.channel_mode, param.bit_rate);
    // 打开AI voice播放
    int ret = avi_audio_player_open(NULL, 0, &param);

    return 0;
}
static int video_dec_player_stop()
{

    avi_audio_player_close(0);
    if (__this->tick_id) {
        usr_timer_del(__this->tick_id);
        __this->tick_id = 0;
    }
    if (__this->audio_buf) {
        free(__this->audio_buf);
        __this->audio_buf = NULL;
    }
    if (__this->dec_hd) {
        AVI_close(__this->dec_hd);
        __this->dec_hd = NULL;
    }
    return 0;
}
static int video_dec_player_suspend()
{
    /* if(__this->tick_id){ */
    /* usr_timer_del(__this->tick_id); */
    /* } */

    return 0;
}
static int video_dec_player_resume()
{
    /* int msec = (int)1000/__this->frame_rate;// /2? */
    /* if(__this->tick_id){ */
    /* usr_timer_del(__this->tick_id); */
    /* } */
    /* __this->tick_id = usr_timer_add(NULL, video_dec_tick_cb,msec,1); */
    return 0;
}

static void video_dec_task(void *p)
{
    int msg[32];
    int ret;
    while (1) {
        ret = os_taskq_pend(NULL, msg, ARRAY_SIZE(msg));
        printf("%s msg:%d \n", __func__, msg[0]);
        if (!__this) {
            break;
        }
        switch (msg[0]) {
        case VIDEO_DEC_MSG_TICK:
            video_dec_player_tick();
            break;
        case VIDEO_DEC_MSG_UPDATE_FILE:
            video_dec_player_update_dec_hd();
            break;
        case VIDEO_DEC_MSG_START:
            video_dec_player_start();
            break;
        case VIDEO_DEC_MSG_STOP:
            video_dec_player_stop();
            break;
        case VIDEO_DEC_MSG_SUSPEND:
            video_dec_player_suspend();
            break;
        case VIDEO_DEC_MSG_RESUME:
            video_dec_player_resume();
            break;
        case VIDEO_DEC_MSG_EXIT:
            os_sem_post(&__this->task_kill_sem);
            os_time_dly(-1);
            break;
        default:
            break;
        }
    }
}

int video_dec_init()
{
    if (!__this) {
        __this = malloc(sizeof(struct video_player));
        memset(__this, 0, sizeof(struct video_player));
    }
    if (!__this->audio_buf) {
        __this->audio_buf = malloc(PCM_CBUF_SIZE);
    }
    cbuf_init(&avi_pcm_cbuf, __this->audio_buf, PCM_CBUF_SIZE);
    os_sem_create(&__this->task_kill_sem, 0);
    //create_task
    int ret = os_task_create(video_dec_task, NULL, 8, 1024, 1024, VIDEO_DEC_TASK_NAME);
    if (ret) {
        printf("%s err\n", __func__);
        free(__this);
        __this = NULL;
    }
    return ret;
}
int video_dec_deinit()
{
    //kill
    if (__this->tick_id) {
        usr_timer_del(__this->tick_id);
        __this->tick_id = 0;
    }
    int ret = video_msg_post(VIDEO_DEC_MSG_EXIT, NULL, 0);

    os_sem_pend(&__this->task_kill_sem, 0);
    os_sem_del(&__this->task_kill_sem, OS_DEL_ALWAYS);

    task_kill(VIDEO_DEC_TASK_NAME);
    //free

    if (__this->audio_buf) {
        free(__this->audio_buf);
        __this->audio_buf = NULL;
    }
    if (__this->dec_hd) {
        AVI_close(__this->dec_hd);
        __this->dec_hd = NULL;
    }
    if (__this) {
        free(__this);
        __this = NULL;
    }
    return 0;
}
int video_dec_set_path(s8(*path)[64], u8 path_number)
{
    __this->path = path;
    __this->path_number = path_number;
    return 0;
}
int video_dec_start(u8 index, u8 mode)
{
    if ((index < __this->path_number) && (index >= 0)) {
        __this->frame_index = index;
    } else {
        __this->frame_index = 0;
    }
    int ret = 0;
    ret += video_msg_post(VIDEO_DEC_MSG_UPDATE_FILE, NULL, 0);

    ret += video_msg_post(VIDEO_DEC_MSG_START, NULL, 0);
    return 0;
}
int video_dec_stop()
{
    int ret = video_msg_post(VIDEO_DEC_MSG_STOP, NULL, 0);

    return ret;
}
int video_dec_refresh_cb_register(void (*cb)(int status))
{
    if (!__this) {
        printf("%s not find\n", __func__);
        return -1;
    }
    __this->ui_refresh_cb = cb;
    return 0;
}

#if 0
//读取AVI文件视频帧和音频帧行写卡在PC上验证
int video_dec_demo(void)
{
    char *filename = "storage/sd0/C/VID_0001.AVI";
    avi_t *in_fd = AVI_open_input_file(filename, 1);
    if (in_fd == NULL) {
        printf("AVI_open_input_file err \n");
        return -1;
    }

    u8 *file_buf = malloc(20 * 1024);
    if (!file_buf) {
        printf("file_buf malloc err\n");
        return -1;
    }

    int total_frame = AVI_video_frames(in_fd);
    printf("AVI total video frame:%d \n", total_frame);
    int audio_chunks =  AVI_audio_chunks(in_fd);
    printf("AVI total audio frame:%d \n", audio_chunks);

    char save_file_name[64];
    int i = 0;
    for (i = 0; i < total_frame; i++) {
        AVI_set_video_position(in_fd, i);

        int keyframe;
        int file_len = AVI_read_frame(in_fd, (char *)file_buf, &keyframe);
        if (file_len > 0) {
            printf("write video frame:%d \n", i);
            snprintf(save_file_name, sizeof(save_file_name), "storage/sd0/C/avi_test/jpg_%d.jpg", i);
            FILE *fd = fopen(save_file_name, "w+");
            fwrite(file_buf, 1, file_len, fd);
            fclose(fd);
        }
    }

    snprintf(save_file_name, sizeof(save_file_name), "storage/sd0/C/avi_test/test.pcm");
    FILE *fd = fopen(save_file_name, "w+");

    u32 offset = 0;
    for (i = 0; i < audio_chunks; i++) {
        int audio_chunk_size =  AVI_audio_size(in_fd, i);
        AVI_set_audio_position(in_fd, offset);


        int file_len = AVI_read_audio_chunk(in_fd, (char *)file_buf);
        if (file_len > 0) {
            printf("write audio frame:%d  size:%d chunk_size:%d \n", i, file_len, audio_chunk_size);
            fwrite(file_buf, 1, file_len, fd);
        }

        offset += audio_chunk_size;
    }
    fclose(fd);


    AVI_close(in_fd);
    free(file_buf);

    /* os_time_dly(-1); */
    return 0;
}

#endif
