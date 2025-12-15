#include "app_config.h"
#include "video_rec.h"
#include "bf30a2_cfg.h"
#include "camera_manager.h"
#include "avilib.h"
#include "pcm_data.h"
#include "mic_data.h"
#include "clock_manager/clock_manager.h"

#define LOG_TAG_CONST      	VIDEO_REC
#define LOG_TAG     		"[VIDEO_REC]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_CHAR_ENABLE		//log_char enable */
#define LOG_CLI_ENABLE
#include "debug.h"
#if TCFG_CAMERA_MANAGER_ENABLE
/*
	是否使用独立pcm线程管理mic数据,
	会消耗更多的ram(11*4k)，直接获取mic数据仅需要(3*4k+512)
	默认不开
*/
#define MIC_DATA_FORM_PCM_TASK_ENABLE		0

#define MIC_FREMA_SIZE						(4*1024)
#define MIC_BUFFER_SIZE						(MIC_FREMA_SIZE*2+512)
#define MIC_SIMPLE_RATE						(8000)

enum {
    Q_AVI_TASK_KILL   = (Q_USER + 100),
    Q_AVI_TASK_REC_START,
    Q_AVI_TASK_REC_STOP,
};

struct fill_frame {
    u32 start_msecs;//开始录像时间
    u32 msecs;//上一次补帧时间
    u32 secs;//录像时间
    u32 fnum;//需要补帧的数
    u32 cnt_fnum;//总帧数
    u16 fps;//目标帧率
    u16 one_sec_fps;//1秒实际帧率
    u8  fill_frame_en;
    u8  droping_frame_en;
};


struct video_rec_handle {
    void *pcm_hdl;
    void *mic;
    u8 *mic_frame;
    char avi_task_name[32];
    u8 write_error;				//写文件异常
    u8 busy;
    int aframe_size;
    int video_rate;
    int sample_rate;
    void (*ui_refresh_cb)(int status);
    int bytes_per_sample;
    struct fill_frame fill;
};
static struct video_rec_handle *video_rec;
#define __this (video_rec)

#if TCFG_PSRAM_DEV_ENABLE
#define malloc(size)       malloc_psram(size)
#define free(ptr)          free_psram(ptr)
#else
#define malloc(size)       malloc(size)
#define free(ptr)          free(ptr)
#endif



extern uint32_t timer_get_ms(void);

//摄像头流程 bf30a2_camera_init->start->read->read_done->read...->stop->exit.
//麦克风流程 pcm_data_init->read->read_done->read...->exit.

extern int jljpeg_stream_src_data_copy(u8 *buf, int size);

extern void camera_dec_flush(void);

static void jlcamera_ui_reflush(void)
{
    if (__this && __this->ui_refresh_cb) {
        __this->ui_refresh_cb(__this->write_error);
    }
}

/* ------------------------------------------------------------------------------------*/
/**
 * @brief jlcamera_video_rec_refresh_cb 注册刷新回调
 *
 * @param cb ui 刷新回调
 *
 * @return
 */
/* ------------------------------------------------------------------------------------*/
int jlcamera_video_rec_refresh_cb(void (*cb)(int status))
{
    if (!__this) {
        log_error("%s not find\n", __func__);
        return -1;
    }
    __this->ui_refresh_cb = cb;
    return 0;
}

/* ------------------------------------------------------------------------------------*/
/**
 * @brief jlcamera_video_rec_start 启动录像
 *
 * @param filename	录制文件路径
 * @param cyc_time	循环录制(-1不循环)
 *
 * @return
 */
/* ------------------------------------------------------------------------------------*/
int jlcamera_video_rec_start(char *filename, int cyc_time)
{
    log_debug("%s file_name:%s cyc_time:%d\n", __func__, filename, cyc_time);
    if (!__this) {
        log_error("%s not init\n", __func__);
        return -1;
    }
    int msg[3] = {0};
    msg[0] = (u32)filename;
    msg[1] = cyc_time;
    return os_taskq_post_type(__this->avi_task_name, Q_AVI_TASK_REC_START, 2, msg);
}

/* ------------------------------------------------------------------------------------*/
/**
 * @brief jlcamera_video_rec_stop 暂停录制
 *
 * @return
 */
/* ------------------------------------------------------------------------------------*/
int jlcamera_video_rec_stop()
{
    if (!__this) {
        log_error("%s not init\n", __func__);
        return -1;
    }
    log_debug("%s \n", __func__);
    return os_taskq_post_type(__this->avi_task_name, Q_AVI_TASK_REC_STOP, 0, NULL);
}

/* ------------------------------------------------------------------------------------*/
/**
 * @brief 补帧模块初始化
 *
 * @return
 */
/* ------------------------------------------------------------------------------------*/
static void jljpeg_frame_fill_init()
{
    if (!__this) {
        log_error("%s not init\n", __func__);
        return;
    }
    memset(&__this->fill, 0, sizeof(struct fill_frame));
    __this->fill.fps = __this->video_rate;
    __this->fill.start_msecs = jiffies_msec();;
    __this->fill.msecs = jiffies_msec();
    __this->fill.droping_frame_en = 1;
    __this->fill.fill_frame_en = 1;
}

/* ------------------------------------------------------------------------------------*/
/**
 * @brief 补帧模块判断(需要运行在写帧后面)
 *
 * @return
 */
/* ------------------------------------------------------------------------------------*/
static void jljpeg_frame_fill_judge(avi_t *out_fd, u32 frame_cnt)
{
    if (!__this) {
        log_error("%s not init\n", __func__);
        return;
    }

    u32 cur_fcnt;
    u32 need_fcnt;

    __this->fill.cnt_fnum += frame_cnt;

    __this->fill.one_sec_fps += frame_cnt;


    if (jiffies_msec2offset(__this->fill.msecs, jiffies_msec()) >= 1000) {
        __this->fill.secs = (jiffies_msec2offset(__this->fill.start_msecs, jiffies_msec())) / 1000;
        if (!__this->fill.secs) {
            __this->fill.secs = 1;
        }
        need_fcnt = __this->fill.secs * __this->fill.fps;
        cur_fcnt = __this->fill.cnt_fnum + __this->fill.fnum;
        if (cur_fcnt < need_fcnt) {
            __this->fill.fnum += need_fcnt - cur_fcnt;
        }
        __this->fill.msecs = jiffies_msec();
        /* printf("avi dup video fps :%d  act fps:%d ,ext fps: %d %d\n", __this->fill.cnt_fnum / __this->fill.secs, __this->fill.one_sec_fps, __this->fill.fps,__this->fill.fnum); */
        __this->fill.one_sec_fps = 0;
    }

    if (__this->fill.fnum && __this->fill.fill_frame_en) {
        int  ret = AVI_dup_frame(out_fd);
        if (!ret) {
            __this->fill.cnt_fnum++;
            __this->fill.fnum--;

        }
    }
}

/* ------------------------------------------------------------------------------------*/
/**
 * @brief 动态丢帧补帧模块判断(需要运行在写帧前面)
 *
 * @return
 */
/* ------------------------------------------------------------------------------------*/
static int jljpeg_frame_dropping_judge()
{
    if (!__this || !__this->fill.droping_frame_en) {
        return 0;
    }
    u32 msecs = jiffies_msec2offset(__this->fill.msecs, jiffies_msec());
    /* printf("%d %d %d\n",(__this->fill.one_sec_fps+1) , (msecs*__this->fill.fps/1000),msecs); */
    if ((__this->fill.one_sec_fps) > (msecs * __this->fill.fps / 1000) + 1) {
        /* printf("fps > %d ,need droping\n",__this->fill.fps); */
        if (__this->fill.fnum) {
            __this->fill.fnum--;
            return 0;
        }
        return 1;
    }
    return 0;
}



static void video_show_task(void *priv)
{
    log_debug("%s enter>>>>>>>>>>>>>>>>>>>>\n ", __func__);
    int ret = 0;
    int msg[8];

    int frame_cnt = 0;
    int frame_width = 0;
    int frame_height = 0;

    int cyc_time = -1;  //循环录影时间，单位秒,-1不循环

    u8 *pcm_data, *jpeg_data;
    int pcm_data_len, jpeg_data_len;

    char *filename = NULL;
    avi_t *out_fd = NULL;

    camera_manager_get_dev_width_height(&frame_width, &frame_height);
    log_info("%s frame[%d x %d] \n", __func__, frame_width, frame_height);

    while (1) {
        u8 read_data = 0;
        if (os_taskq_accept(ARRAY_SIZE(msg), msg) == OS_TASKQ) {
            if (msg[0] == Q_AVI_TASK_KILL) {
                log_debug("avi task exit !\n");
                if (out_fd) {
                    ret = AVI_close(out_fd);
                    if (ret) {
                        log_error("camera devide avi close err :%d \n", ret);
                        ASSERT(0);
                    } else {
                        log_debug("avi write success \n");
                        out_fd = NULL;
                    }
                }
#if MIC_DATA_FORM_PCM_TASK_ENABLE
                if (__this->pcm_hdl) {
                    pcm_data_exit(__this->pcm_hdl);
                    __this->pcm_hdl = NULL;
                }
#else
                if (__this->mic) {
                    mic_data_close(__this->mic);
                    __this->mic = NULL;
                }
                if (__this->mic_frame) {
                    free(__this->mic_frame);
                    __this->mic_frame = NULL;
                }
#endif
                log_debug("%s exit >>>>>>>>>>>>>>>>>>>>\n ", __func__);
                //下面禁止加打印{
                os_sem_post((OS_SEM *)msg[1]);
                os_time_dly(-1);
                //}
                break;
            } else if (msg[0] == Q_AVI_TASK_REC_START) {
                filename = (char *)msg[1];
                cyc_time = msg[2];
                __this->write_error = 0;
                log_debug("avi task rec start <%s> cyc_time:%d !\n", filename, cyc_time);
#if MIC_DATA_FORM_PCM_TASK_ENABLE
                __this->pcm_hdl = pcm_data_init(__this->sample_rate, __this->aframe_size, __this->aframe_size * 10);
                if (!__this->pcm_hdl) {
                    //TODO
                    log_error("avi task rec start pcm err!!!\n");
                    /* break; */
                    __this->write_error = -1;
                    continue;
                }
#else
                __this->mic = mic_data_open(VOICE_MCU_MIC, MIC_BUFFER_SIZE, MIC_SIMPLE_RATE);
                if (!__this->mic) {
                    //TODO
                    log_error("avi task rec start pcm err!!!\n");
                    /* break; */
                    __this->write_error = -1;
                    continue;
                }
                __this->mic_frame = malloc(MIC_FREMA_SIZE);
                if (!__this->mic_frame) {
                    log_error("avi task rec start pcm err!!!\n");
                    /* break; */
                    __this->write_error = -1;
                    continue;
                }
#endif
                out_fd = AVI_open_output_file(filename);
                if (out_fd == NULL) {
                    log_error("open file erro\n");
                    __this->write_error = -1;
                    continue;
                    /* break; */
                }
                AVI_set_video_suggestbuffersize(out_fd, 20 * 1024); //建议缓存区20k，解码用
                AVI_set_video(out_fd, frame_width, frame_height, __this->video_rate, "MJPG");
                AVI_set_audio(out_fd, 1, __this->sample_rate, 16, WAVE_FORMAT_PCM, 0);//默认单声道s16格式
                jljpeg_frame_fill_init();
                frame_cnt = 0;
                /* break; */
            } else if (msg[0] == Q_AVI_TASK_REC_STOP) {
                log_debug("avi task rec stop!\n");
                if (out_fd) {
                    ret = AVI_close(out_fd);
                    if (ret) {
                        log_error("camera devide avi close err :%d \n", ret);
                        __this->write_error = 1;
                        /* ASSERT(0); */
                    } else {
                        log_debug("avi write success \n");
                    }
                    out_fd = NULL;
                }
#if MIC_DATA_FORM_PCM_TASK_ENABLE
                if (__this->pcm_hdl) {
                    pcm_data_exit(__this->pcm_hdl);
                    __this->pcm_hdl = NULL;
                }
#else
                if (__this->mic) {
                    mic_data_close(__this->mic);
                    __this->mic = NULL;
                }
                if (__this->mic_frame) {
                    free(__this->mic_frame);
                    __this->mic_frame = NULL;
                }
#endif
                __this->write_error = 0;
                /* break; */
            }
        }

        log_char('\n');

        log_char('C');
        //录像输出
        if (out_fd) {
#if MIC_DATA_FORM_PCM_TASK_ENABLE
            if (pcm_data_read(__this->pcm_hdl, &pcm_data, &pcm_data_len) == 0) {
                read_data = 1;
                __this->write_error = AVI_write_audio(out_fd, (char *)pcm_data, pcm_data_len);
                pcm_data_read_done(__this->pcm_hdl, pcm_data);
            }
#else
            if (__this->mic && __this->mic_frame) {
                if (mic_data_read(__this->mic, __this->mic_frame, MIC_FREMA_SIZE)) {
                    read_data = 1;
                    __this->write_error = AVI_write_audio(out_fd, (char *)__this->mic_frame, MIC_FREMA_SIZE);
                }
            }
#endif
        }
        if (camera_manager_data_read(&jpeg_data, &jpeg_data_len) == 0) {
            log_char('M');
            read_data = 1;
            frame_cnt ++;
#if ((defined TCFG_UI_ENABLE) && TCFG_UI_ENABLE)
            //ui帧输出
            if (!jljpeg_stream_src_data_copy(jpeg_data, jpeg_data_len)) {
                log_char('R');
                jlcamera_ui_reflush();
            }
#endif
            if (out_fd) {		//录像输出


                if (!jljpeg_frame_dropping_judge()) {
                    __this->write_error = AVI_write_frame(out_fd, (char *)jpeg_data, jpeg_data_len, 1);
                    jljpeg_frame_fill_judge(out_fd, 1); //补帧计算
                } else {
                    jljpeg_frame_fill_judge(out_fd, 0); //补帧计算
                }
            }
            camera_manager_read_done(jpeg_data);
#if 0
            static int cam_last_msec = 0;
            static int cam_frame_sum = 0;
            static int cam_frame_cnt = 0;
            if (cam_last_msec) {
                u32 cur_msec  = jiffies_msec();
                u32 offset  = jiffies_msec2offset(cam_last_msec, cur_msec);
                cam_frame_sum += offset;
                cam_frame_cnt ++;
                if (cam_frame_cnt && cam_frame_sum) {
                    u32 avg  = cam_frame_sum / cam_frame_cnt;
                    u32 fps = 1000 / avg;
                    printf("CAM avg:%d fps:%d \n", avg, fps);
                }
            }
            cam_last_msec = jiffies_msec();
#endif
        }

        if (out_fd && (cyc_time != -1)) {
            if (frame_cnt == __this->video_rate * cyc_time) {
                ret = AVI_close(out_fd);
                if (ret) {
                    log_error("camera avi close err :%d \n", ret);
                } else {
                    log_debug("avi write success \n");
                }

                out_fd = AVI_open_output_file(filename);
                if (out_fd == NULL) {
                    log_error("open file erro\n");
                    continue;
                }
                AVI_set_video_suggestbuffersize(out_fd, 20 * 1024); //建议缓存区20k，解码用
                AVI_set_video(out_fd, frame_width, frame_height, __this->video_rate, "MJPG");
                AVI_set_audio(out_fd, 1, __this->sample_rate, 16, WAVE_FORMAT_PCM, 0);//默认单声道s16格式
                frame_cnt = 0;
            }
        }

        //没有读到数据
        if (!read_data) {
            //根据音频包计算延时
            u32 delay_ms = ((float)__this->aframe_size / (__this->sample_rate * 2)) * 1000;
            u32 tick = delay_ms / 10;
            os_time_dly(tick + 1);
        }
    }
    log_debug("%s exit >>>>>>>>>>>>>>>>>>>>\n ", __func__);
}

s8 *jlcamera_video_get_task_name()
{
    if (__this) {
        return __this->avi_task_name;
    }
    return NULL;
}
/* ------------------------------------------------------------------------------------*/
/**
 * @brief jlcamera_video_rec_init 初始化摄像头
 *
 * @return
 */
/* ------------------------------------------------------------------------------------*/
int jlcamera_video_rec_init(void)
{
    int ret = 0;
    ASSERT(!__this);
    __this = malloc(sizeof(struct video_rec_handle));
    if (!__this) {
        goto __err;
    }
    memset(__this, 0, sizeof(struct video_rec_handle));
    __this->busy = 1;
    clock_lock("camera_video", clk_get_max_frequency());
    //初始化驱动
    ret = camera_manager_init();
    if (ret) {
        goto __err;
    }
    //启动摄像头
    ret = camera_manager_start();
    if (ret) {
        goto __err;
    }
    int fps;
    camera_manager_get_dev_fps(&fps);
    //视频帧率
    __this->video_rate = fps / 2;
    //音频采样率
    __this->sample_rate = MIC_SIMPLE_RATE;
    __this->aframe_size = MIC_FREMA_SIZE;

    snprintf(__this->avi_task_name, sizeof(__this->avi_task_name), "video_show_task");
    if (os_task_create(video_show_task, __this, 8, 1024, 1024, __this->avi_task_name)) {
        log_error("avi task create fail \n");
        goto __err;
    }

    return 0;
__err:
    log_error("%s err\n", __func__);
    camera_manager_stop();
    camera_manager_deinit();
    if (__this) {
        free(__this);
        __this = NULL;
    }
    return -1;
}
/* ------------------------------------------------------------------------------------*/
/**
 * @brief jlcamera_video_rec_deinit
 *
 * @return
 */
/* ------------------------------------------------------------------------------------*/
int jlcamera_video_rec_deinit(void)
{
    /* printf("%s %d", __func__, __LINE__); */
    /* void dma_memcpy_wait_idle(void); */
    /* dma_memcpy_wait_idle(); */
    camera_manager_stop();
    if (__this) {
        OS_SEM task_kill_sem;
        os_sem_create(&task_kill_sem, 0);
        int msg = (int)&task_kill_sem;
        os_taskq_del_type(__this->avi_task_name, Q_AVI_TASK_REC_START);
        os_taskq_del_type(__this->avi_task_name, Q_AVI_TASK_REC_STOP);
        os_taskq_post_type(__this->avi_task_name, Q_AVI_TASK_KILL, 1, &msg);
        os_sem_pend(&task_kill_sem, 0);
        os_sem_del(&task_kill_sem, OS_DEL_ALWAYS);
        task_kill(__this->avi_task_name);
    }

    camera_manager_deinit();
    clock_unlock("camera_video");
    __this->busy = 0;
    free(__this);
    __this = NULL;
    return 0;
}

static u8 jl_camera_video_idle_query(void)
{
    if (__this) {
        return __this->busy;
    }
    return 1;
}
REGISTER_LP_TARGET(camera_video_lp_target) = {
    .name = "camera_video",
    .is_idle = jl_camera_video_idle_query,
};


#endif


#if 0
//原代码
static void avi_task(void *priv)
{
    int ret = 0;
    int msg[8];
    int frame_cnt = 0;
    int cyc_time = 30;  //循环录影时间，单位秒
    u8 *pcm_data, *jpeg_data;
    int pcm_data_len, jpeg_data_len;

    char *filename = "storage/sd0/C/VID_****.AVI";
    avi_t *out_fd = AVI_open_output_file(filename);
    if (out_fd == NULL) {
        printf("open file erro\n");
        os_time_dly(-1);
    }

    AVI_set_video(out_fd, BF30A2_INPUT_W, BF30A2_INPUT_H, __this->video_rate, "MJPG");
    AVI_set_audio(out_fd, 1, __this->sample_rate, 16, WAVE_FORMAT_PCM, 0);//默认单声道s16格式

    while (1) {
        u8 read_data = 0;

        if (os_taskq_accept(ARRAY_SIZE(msg), msg) == OS_TASKQ) {
            if (msg[0] == Q_AVI_TASK_KILL) {
                if (out_fd) {
                    AVI_close(out_fd);
                }
                printf("avi task exit !\n");
                os_sem_post((OS_SEM *)msg[1]);
                os_time_dly(-1);
                break;
            }
        }

        if (pcm_data_read(__this->pcm_hdl, &pcm_data, &pcm_data_len) == 0) {
            read_data = 1;
            AVI_write_audio(out_fd, (char *)pcm_data, pcm_data_len);
            pcm_data_read_done(__this->pcm_hdl, pcm_data);
        }
        if (bf30a2_camera_data_read(&jpeg_data, &jpeg_data_len) == 0) {
            read_data = 1;
            frame_cnt ++;

            AVI_write_frame(out_fd, (char *)jpeg_data, jpeg_data_len, 1);
            bf30a2_camera_read_done(jpeg_data);
        }

        if (frame_cnt == __this->video_rate * cyc_time) {
            ret = AVI_close(out_fd);
            if (ret) {
                printf("bf30a2 avi close err :%d \n", ret);

            } else {
                printf("avi write success \n");
            }

            out_fd = AVI_open_output_file(filename);
            if (out_fd == NULL) {
                printf("open file erro\n");
                break;
            }
            AVI_set_video(out_fd, BF30A2_INPUT_W, BF30A2_INPUT_H, __this->video_rate, "MJPG");
            AVI_set_audio(out_fd, 1, __this->sample_rate, 16, WAVE_FORMAT_PCM, 0);//默认单声道s16格式
            frame_cnt = 0;
        }

        //没有读到数据
        if (!read_data) {
            //根据音频包计算延时
            u32 delay_ms = ((float)__this->aframe_size / (__this->sample_rate * 2)) * 1000;
            u32 tick = delay_ms / 10;
            os_time_dly(tick + 1);
        }

    }
}

//基于音频时间进行补帧(音频包太大有误差)
static void avi_task_dup1(void *priv)
{
    int ret = 0;
    int msg[8];
    int frame_cnt = 0;
    int cyc_time = 30;  //循环录影时间，单位秒
    u8 *pcm_data, *jpeg_data;
    int pcm_data_len, jpeg_data_len;

    char *filename = "storage/sd0/C/VID_****.AVI";
    avi_t *out_fd = AVI_open_output_file(filename);
    if (out_fd == NULL) {
        printf("open file erro\n");
        os_time_dly(-1);
    }

    AVI_set_video(out_fd, BF30A2_INPUT_W, BF30A2_INPUT_H, __this->video_rate, "MJPG");
    AVI_set_audio(out_fd, 1, __this->sample_rate, 16, WAVE_FORMAT_PCM, 0);//默认单声道s16格式

    // --- 用于补帧的统计变量 ---
    u32 total_audio_bytes = 0;                  // 累加收到并写入的音频字节数
    const int bytes_per_sample = __this->bytes_per_sample;
    const int video_rate = __this->video_rate;
    const int sample_rate = __this->sample_rate;
    int dup_frame_cnt  = 0;     //调试使用

    while (1) {
        u8 read_data = 0;

        if (os_taskq_accept(ARRAY_SIZE(msg), msg) == OS_TASKQ) {
            if (msg[0] == Q_AVI_TASK_KILL) {
                if (out_fd) {
                    AVI_close(out_fd);
                }
                printf("avi task exit !\n");
                os_sem_post((OS_SEM *)msg[1]);
                os_time_dly(-1);
                break;
            }
        }

        if (pcm_data_read(__this->pcm_hdl, &pcm_data, &pcm_data_len) == 0) {
            read_data = 1;
            AVI_write_audio(out_fd, (char *)pcm_data, pcm_data_len);

            // 累加已写入的音频字节（用于推算应该产生的视频帧数）
            total_audio_bytes += pcm_data_len;

            pcm_data_read_done(__this->pcm_hdl, pcm_data);
        }
        if (bf30a2_camera_data_read(&jpeg_data, &jpeg_data_len) == 0) {
            read_data = 1;
            frame_cnt ++;

            AVI_write_frame(out_fd, (char *)jpeg_data, jpeg_data_len, 1);
            bf30a2_camera_read_done(jpeg_data);

            // 基于音频进度推算并补帧
            if (total_audio_bytes >= sample_rate * bytes_per_sample) {
                u32 total_audio_samples = total_audio_bytes / bytes_per_sample; // 整数
                u32 expected_frames = (total_audio_samples * video_rate) / sample_rate;

                if (expected_frames > frame_cnt) {
                    ret = AVI_dup_frame(out_fd);
                    if (!ret) {
                        dup_frame_cnt++;
                        frame_cnt++;
                        printf("avi dup video frame :%d  cur frame:%d \n", dup_frame_cnt, frame_cnt);
                    }
                }
            }
        }


        if (frame_cnt >= __this->video_rate * cyc_time) {
            ret = AVI_close(out_fd);
            if (ret) {
                printf("bf30a2 avi close err :%d \n", ret);

            } else {
                printf("avi write success \n");
            }

            out_fd = AVI_open_output_file(filename);
            if (out_fd == NULL) {
                printf("open file erro\n");
                break;
            }
            AVI_set_video(out_fd, BF30A2_INPUT_W, BF30A2_INPUT_H, __this->video_rate, "MJPG");
            AVI_set_audio(out_fd, 1, __this->sample_rate, 16, WAVE_FORMAT_PCM, 0);//默认单声道s16格式
            frame_cnt = 0;
            dup_frame_cnt = 0;
            total_audio_bytes = 0;
        }

        //没有读到数据
        if (!read_data) {
            //根据音频包计算延时
            u32 delay_ms = ((float)__this->aframe_size / (__this->sample_rate * 2)) * 1000;
            u32 tick = delay_ms / 10;
            os_time_dly(tick + 1);
        }

    }
}

//基于系统时间进行补帧
static void avi_task_dup2(void *priv)
{
    int ret = 0;
    int msg[8];
    int frame_cnt = 0;
    int cyc_time = 30;  //循环录影时间，单位秒
    u8 *pcm_data, *jpeg_data;
    int pcm_data_len, jpeg_data_len;

    char *filename = "storage/sd0/C/VID_****.AVI";
    avi_t *out_fd = AVI_open_output_file(filename);
    if (out_fd == NULL) {
        printf("open file erro\n");
        os_time_dly(-1);
    }

    AVI_set_video(out_fd, BF30A2_INPUT_W, BF30A2_INPUT_H, __this->video_rate, "MJPG");
    AVI_set_audio(out_fd, 1, __this->sample_rate, 16, WAVE_FORMAT_PCM, 0);//默认单声道s16格式

    // --- 用于补帧的统计变量 ---
    const int video_rate = __this->video_rate;
    u32 start_ms = timer_get_ms();   // 每次 open file 时重置
    int dup_frame_cnt  = 0;          // 调试使用

    while (1) {
        u8 read_data = 0;

        if (os_taskq_accept(ARRAY_SIZE(msg), msg) == OS_TASKQ) {
            if (msg[0] == Q_AVI_TASK_KILL) {
                if (out_fd) {
                    AVI_close(out_fd);
                }
                printf("avi task exit !\n");
                os_sem_post((OS_SEM *)msg[1]);
                os_time_dly(-1);
                break;
            }
        }

        if (pcm_data_read(__this->pcm_hdl, &pcm_data, &pcm_data_len) == 0) {
            read_data = 1;
            AVI_write_audio(out_fd, (char *)pcm_data, pcm_data_len);
            pcm_data_read_done(__this->pcm_hdl, pcm_data);
        }
        if (bf30a2_camera_data_read(&jpeg_data, &jpeg_data_len) == 0) {
            read_data = 1;
            frame_cnt ++;
            AVI_write_frame(out_fd, (char *)jpeg_data, jpeg_data_len, 1);
            bf30a2_camera_read_done(jpeg_data);

            // --------- 基于系统时钟检查是否需要补帧 ---------
            u32 now = timer_get_ms();
            u32 elapsed_ms = now - start_ms;
            u32 expected_frames = (elapsed_ms * video_rate) / 1000;
            if (expected_frames > frame_cnt) {
                ret = AVI_dup_frame(out_fd);
                if (!ret) {
                    dup_frame_cnt++;
                    frame_cnt++;
                    printf("avi dup video frame :%d  cur frame:%d \n", dup_frame_cnt, frame_cnt);
                }
            }

        }

        if (frame_cnt >= __this->video_rate * cyc_time) {
            ret = AVI_close(out_fd);
            if (ret) {
                printf("bf30a2 avi close err :%d \n", ret);

            } else {
                printf("avi write success \n");
            }

            out_fd = AVI_open_output_file(filename);
            if (out_fd == NULL) {
                printf("open file erro\n");
                break;
            }
            AVI_set_video(out_fd, BF30A2_INPUT_W, BF30A2_INPUT_H, __this->video_rate, "MJPG");
            AVI_set_audio(out_fd, 1, __this->sample_rate, 16, WAVE_FORMAT_PCM, 0);//默认单声道s16格式
            frame_cnt = 0;
            dup_frame_cnt = 0;
            start_ms = timer_get_ms();
        }

        //没有读到数据
        if (!read_data) {
            //根据音频包计算延时
            u32 delay_ms = ((float)__this->aframe_size / (__this->sample_rate * 2)) * 1000;
            u32 tick = delay_ms / 10;
            os_time_dly(tick + 1);
        }

    }
}




int video_rec_start_demo(void)
{
    int ret = 0;

    ret = bf30a2_camera_init();
    if (ret) {
        //TODO
        return -1;
    }
    ret = bf30a2_camera_start();
    if (ret) {
        //TODO
        return -1;
    }
    __this->video_rate = BF30A2_INPUT_FPS / 2; //视频帧率

    __this->sample_rate = 8000;             //音频采样率
    __this->bytes_per_sample = 2;            // 单声道 s16 -> 每采样 2 字节
    __this->aframe_size = 4096;
    __this->pcm_hdl = pcm_data_init(__this->sample_rate, __this->aframe_size, __this->aframe_size * 10);
    if (!__this->pcm_hdl) {
        //TODO
        return -1;
    }

    snprintf(__this->avi_task_name, sizeof(__this->avi_task_name), "avi_task");
    if (os_task_create(avi_task, __this, 8, 1024, 1024, __this->avi_task_name)) {
        printf("avi task create fail \n");
        //TODO
        return -1;
    }

    /*     __this->video_rate = 20; //可配置一个摄像头达不到的帧率，模拟需要补帧的情况 */
    /* //根据系统时间进行补帧 */
    /* if (os_task_create(avi_task_dup2, __this, 8, 1024, 1024, __this->avi_task_name)) { */
    /* printf("avi task create fail \n"); */
    /* //TODO */
    /* return -1; */
    /* } */

    return 0;
}

int video_rec_stop_demo(void)
{
    OS_SEM task_kill_sem;
    os_sem_create(&task_kill_sem, 0);
    int msg = (int)&task_kill_sem;
    os_taskq_post_type(__this->avi_task_name, Q_AVI_TASK_KILL, 1, &msg);
    os_sem_pend(&task_kill_sem, 0);
    os_sem_del(&task_kill_sem, OS_DEL_ALWAYS);
    task_kill(__this->avi_task_name);

    pcm_data_exit(__this->pcm_hdl);

    bf30a2_camera_stop();

    bf30a2_camera_exit();

    return 0;
}
#endif




