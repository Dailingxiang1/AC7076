#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".usb.data.bss")
#pragma data_seg(".usb.data")
#pragma code_seg(".usb.text")
#pragma const_seg(".usb.text.const")
#pragma str_literal_override(".usb.text.const")
#endif

/* #include "video/fb.h" */
#include "video.h"
#include "videobuf.h"
#include "device/device.h"
#include "system/init.h"
#include "system/task.h"
#include "system/malloc.h"
#include "event.h"
#include "ascii.h"
/* #include "asm/imc_driver.h" */
/* #include "asm/isp_dev.h" */

#include "debug.h"
#define LOG_TAG_CONST       USB
#define LOG_TAG             "[USB]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE



#define list_for_each_video_device(dev) \
	for (dev = video_dev_begin; dev < video_dev_end; dev++)


u32 video_buf_free_space(struct video_device *video)
{
    return videobuf_stream_free_space(&video->video_q);
}

void *video_buf_malloc(struct video_device *video, u32 size)
{
    struct videobuf_buffer *b;

    b = videobuf_stream_alloc(&video->video_q, size);
    if (!b) {
        return NULL;
    }
    b->len = size;

//	printf("video_buf: %x\n", b, b->data);

    return b->data;
}

void *video_buf_realloc(struct video_device *video, void *buf, int size)
{
    ASSERT(buf);
    struct videobuf_buffer *b = container_of(buf, struct videobuf_buffer, data);
    void *lbuf;
    ASSERT(b);


    b->len = size;
    lbuf = videobuf_stream_realloc(&video->video_q, b, size);
    if (lbuf == NULL) {
        return NULL;
    }

    return b->data;
}


void video_buf_free(struct video_device *video, void *buf)
{
    struct videobuf_buffer *b = container_of(buf, struct videobuf_buffer, data);

    if (buf == NULL) {
        return;
    }
    /* ASSERT(buf != NULL, "video_buf_free\n"); */

    videobuf_stream_free(&video->video_q, b);
}

int video_buf_query(struct video_device *video, struct videobuf_state *sta)
{
    return videobuf_query(&video->video_q, sta);
}

void *video_buf_ptr(void *buf)
{
    return buf;
}

u32 video_buf_size(void *buf)
{
    struct videobuf_buffer *b = container_of(buf, struct videobuf_buffer, data);

    return b->len;
}

int video_frame_cnt(struct video_device *video)
{
    return video->frame_cnt;
}


static void video_insert_end_frame(struct video_device *video)
{
    struct device_event e = {0};
    struct videobuf_buffer *b ;//= container_of(buf, struct videobuf_buffer, data);

    printf("=========finish: f=%d/%d t=%lu/%d i=%d\n", video->frame_cnt, video->stop_frame_cnt,
           jiffies, video->stop_time, video->insert_frame_cnt);
    b = videobuf_stream_alloc(&video->video_q, 8);
    if (b) {
        if (video->pixelformat & VIDEO_PIX_FMT_H264) {
            u32 flag = 0x18;
            b->len = 8;
            memcpy(b->data + 4, &flag, 4);

        } else if (video->pixelformat & VIDEO_PIX_FMT_JPEG) {
            u8 *buf = (u8 *)b->data;
            buf[0] = 0x56;
            buf[1] = 0x18;
            buf[2] = 0x57;
            buf[3] = 0x19;
            b->len = 8;
        }
        videobuf_stream_finish(&video->video_q, b);
#if 0
        if (video->time_base) {
            e.arg = "video_rec_time";
            e.value = 0;
            e.event = DEVICE_EVENT_CHANGE;
            device_event_notify(DEVICE_EVENT_FROM_VIDEO, &e);

        }
#endif
        video->frame_cnt = 0;
        video->insert_frame_cnt = 0;
        video->stop_frame_cnt = video->stop_frame_interval * video->fps.target_fps;
        /* video->stop_time = jiffies + msecs_to_jiffies(video->stop_frame_interval * 1000); */
        video->start_time = video->stop_time;
        video->stop_time = video->stop_time + msecs_to_jiffies(video->stop_frame_interval * 1000);

        if (video->dev_ops->start_I_frame) {
            video->dev_ops->start_I_frame(video);
        }
    }
    /* spin_lock(&video->ff.lock); */
    /* video->ff.o_frame_cnt++; */
    /* spin_unlock(&video->ff.lock); */
}

void video_buf_stream_finish(struct video_device *video, void *buf)
{
    struct videobuf_buffer *b = container_of(buf, struct videobuf_buffer, data);

    if (video->pixelformat & VIDEO_PIX_FMT_JPEG) {
        if (!video->frame_cnt) {
            video_buf_free(video, buf);
        } else {
            videobuf_stream_finish(&video->video_q, b);
        }
    } else {

        videobuf_stream_finish(&video->video_q, b);
    }


    video->frame_cnt++;

    /* if (video->time_base && (video->frame_cnt % video->fps.target_fps) == 0) { */
    /* e.arg = "video_rec_time"; */
    /* e.type = SYS_DEVICE_EVENT; */
    /* e.u.dev.value = video->frame_cnt / video->fps.target_fps; */
    /* e.u.dev.event = DEVICE_EVENT_CHANGE; */
    /* sys_event_notify(&e); */
    /* } */

    if (video->stop_skip_frame != 2) {
        spin_lock(&video->ff.lock);
        video->ff.o_frame_cnt++;
        spin_unlock(&video->ff.lock);
    }
}


void video_check_stream_finish(struct video_device *video)
{
    if (video->stop_time == 0) {
        return;
    }
    if (video->stop_skip_frame == 0) {
        if (video->pixelformat & VIDEO_PIX_FMT_H264) {
            if (time_after(jiffies, video->stop_time - 200)) {
                spin_lock(&video->ff.lock);
                video->stop_skip_frame = 1;
                spin_unlock(&video->ff.lock);

                /* if (video->frame_cnt < video->stop_frame_cnt && video->stop_frame_cnt - video->frame_cnt == 20) { */
                log_debug("force insert I frame %d/%d\n", video->frame_cnt, video->stop_frame_cnt);
                if (video->dev_ops->start_I_frame) {
                    video->dev_ops->start_I_frame(video);
                }
            }
        } else {
            if (time_after(jiffies, video->stop_time - 200)) {
                spin_lock(&video->ff.lock);
                video->stop_skip_frame = 1;
                spin_unlock(&video->ff.lock);
            }
        }
    }
#if 0
    //计算当前时间点应有的帧数
    int jiffies_interval = jiffies - video->start_time;
    int correct_frame_cnt = (jiffies_interval * 10) / (1000 / video->fps.target_fps);
    //当前时间的应当有的帧数与实际帧数相差太大,补帧
    if (correct_frame_cnt - (int)video->frame_cnt > 3) {
        int insert_frame_interval =  jiffies_to_msecs(jiffies - video->last_insert_time);
        int frame_interval = 1000 / video->fps.target_fps;
#if 0   //todo,打开这个会导致第三路录像慢放(多插帧)
        //如果帧差别过大，不考虑卡顿问题，一直补帧
        if (correct_frame_cnt - (int)video->frame_cnt >= 10) {
            video->insert_frame_cnt++;
            video->last_insert_time = jiffies;
            video_force_skip_frame(video);
        }
        //控制补帧间隔，避免连续补帧造成画面卡顿
        else
#endif
            if (insert_frame_interval > frame_interval * 3) {
                video->insert_frame_cnt++;
                video->last_insert_time = jiffies;
                video_force_skip_frame(video);
            }
    }
#endif

    if (video->time_base && (video->frame_cnt % (video->fps.target_fps * 10)) == 0) {
        //每10秒同步下UI计时
#if 0
        struct device_event e = {0};
        e.arg = "video_rec_time";
        e.value = video->frame_cnt / video->fps.target_fps;
        e.event = DEVICE_EVENT_CHANGE;
        device_event_notify(DEVICE_EVENT_FROM_VIDEO, &e);
#endif

    }

    if (time_after(jiffies, video->stop_time) || video->frame_cnt >= video->stop_frame_cnt) {
        if (video->stop_skip_frame == 1) {
            if (video->frame_cnt <= video->stop_frame_cnt) {
                spin_lock(&video->ff.lock);
                video->stop_skip_frame = 2;
                spin_unlock(&video->ff.lock);

                u32 need_skip_frame = video->stop_frame_cnt - video->frame_cnt + 1;
                while (need_skip_frame--) {
                    log_debug("force skip frame %d/%d\n", video->frame_cnt, video->stop_frame_cnt);
                    video_force_skip_frame(video);
                    /* spin_lock(&video->ff.lock); */
                    /* video->ff.o_frame_cnt++; */
                    /* spin_unlock(&video->ff.lock); */
                    /* need_skip_frame = video->stop_frame_cnt - video->frame_cnt; */
                }
                video_insert_end_frame(video);

                spin_lock(&video->ff.lock);
                video->stop_skip_frame = 0;
                spin_unlock(&video->ff.lock);
            } else {
                video_insert_end_frame(video);

                spin_lock(&video->ff.lock);
                video->stop_skip_frame = 0;
                spin_unlock(&video->ff.lock);
            }
            video_dev_fill_frames_reset(video);
        }
    }

}

int video_check_h264_manu_finish(struct video_device *video)
{
    if (video->stop_time == 0) {
        return 0;
    }

    /* log_d("frame %d/%d %d %d\n", jiffies, video->stop_time,video->frame_cnt,video->stop_frame_cnt); */
    if (time_after(jiffies, video->stop_time - 200)) {
        if (video->pixelformat & VIDEO_PIX_FMT_H264) {
            if (video->frame_cnt <= video->stop_frame_cnt && video->stop_frame_cnt - video->frame_cnt < 1) {

                video_insert_end_frame(video);
                return 1;
            }

        }
    }

    return 0;
}

static void __calculate_fill_interval(struct video_fill_frames *ff)
{
    int interval;

    if (ff->lost_frames == 0 || ff->fps_remain <= 1) {
        ff->fill_dly = 0;
        ff->interval = 0;
    } else {
        interval = (ff->fps_remain - 1) / ff->lost_frames;
        if (interval == 0) {
            ff->fill_dly = 0;
        } else if (ff->fill_dly > 0) {
            ff->fill_dly -= ff->interval - interval;
        } else {
            ff->fill_dly = interval;
        }
        ff->interval = interval;
    }
}


static void video_rec_check_fps(void *_video)
{
    struct video_device *video = (struct video_device *)_video;
    struct video_fill_frames *ff = &video->ff;
    struct video_dev_fps *fps = &video->fps;

    spin_lock(&ff->lock);

    if (video->dev_ops->adjust_fps) {
        video->dev_ops->adjust_fps(video, (void *)&fps->real_fps);
        if (fps->real_fps == 0) {
            fps->real_fps = fps->camera_fps;
        }
    }
    ff->reset_timer = 1;
    ff->fps_remain  = video->fps.real_fps;

    /* printf("time cnt : %d, target fps : %d, o frame count : %d, real fps : %d\n", ff->time_cnt, fps->target_fps, ff->o_frame_cnt, fps->real_fps); */
    if (fps->tlp_time) {
        ff->lost_frames = ++ff->time_cnt * 1000 / fps->tlp_time - ff->o_frame_cnt - 1000 / fps->tlp_time;
    } else {
        ff->lost_frames = ++ff->time_cnt * fps->target_fps - ff->o_frame_cnt - fps->real_fps;
    }
    /* if (ff->time_cnt >= 60) { */
    /* ff->time_cnt = 1; */
    /* ff->o_frame_cnt = 0; */
    /* } */

    __calculate_fill_interval(ff);

    spin_unlock(&ff->lock);

    /*printf("%ds: %d, %d\n", ff->time_cnt, ff->lost_frames, ff->interval);*/
}


static void video_rec_one_frame_timer(void *_video)
{
    struct video_device *video = (struct video_device *)_video;
    struct video_fill_frames *ff = &video->ff;

    spin_lock(&ff->lock);

    ff->lost_frames++;
    if (ff->fps_remain > 0) {
        ff->fps_remain--;
    }

    __calculate_fill_interval(ff);

    spin_unlock(&ff->lock);

    printf("lost: %d, %d\n", ff->lost_frames, ff->fill_dly);
}

/*
static void video_sec_timer(struct video_device *video)
{
    struct video_fill_frames *ff = &video->ff;

    if (ff->reset_timer) {
        sys_hi_timer_modify(ff->timer, 1000);
        ff->reset_timer = 0;
    }
}
*/

int video_dev_is_need_fill_frame(struct video_device *video)
{
    struct video_fill_frames *ff = &video->ff;
    int tmp;

    spin_lock(&ff->lock);

    tmp = ff->fill_dly - 1;
    if ((ff->lost_frames > 0) && (tmp <= 0)) {
        tmp = ff->lost_frames - 1;
        spin_unlock(&ff->lock);
        return tmp + 1;
    }

    spin_unlock(&ff->lock);

    return 0;
}
int video_dev_need_fill_frame(struct video_device *video)
{
#if 0
    struct video_fill_frames *ff = &video->ff;

#if 0
    if (ff->reset_timer) {
        ff->reset_timer = 0;
        sys_hi_timer_modify(ff->timer, 1000 - 16);
    }
#else
    if (ff->reset_timer && !ff->timer) {
        ff->timer = sys_hi_timer_add(video, video_rec_check_fps, 1000);
        ff->reset_timer = 0;
    }
#endif
    if (ff->o_frame_cnt <= 5) {
        return 0;
    }

    spin_lock(&ff->lock);

    if (ff->lost_frames > 0 && --ff->fill_dly <= 0) {
        ff->lost_frames--;
        __calculate_fill_interval(ff);

        if (ff->lost_frames > 100 * (ff->msg_post + 1)) {
            ff->msg_post ++;
            printf("ff->lost_frames:%d", ff->lost_frames);
            /* extern void video_write_err(u8 id, int drop_fcnt); */
            /* video_write_err(video->major, ff->lost_frames + 1); */
        }

        spin_unlock(&ff->lock);

        /* printf("\ndrop %d:%d %d\n", video->major, ff->lost_frames, ff->msg_post); */
        /* printf("fill: %d, %d, %d\n", ff->fps_remain, ff->lost_frames, ff->interval); */

        return ff->lost_frames + 1;
    } else {
        ff->msg_post = 0;
    }

    spin_unlock(&ff->lock);
#endif

    return 0;
}

void video_dev_reset_frame_interval_timer(struct video_device *video)
{
#if 0
    struct video_fill_frames *ff = &video->ff;
    u32 fps;

    spin_lock(&ff->lock);
    fps = video->fps.real_fps;
    spin_unlock(&ff->lock);

    if (ff->timer_one_frame) {
        /* sys_hi_timer_modify(ff->timer_one_frame, (1000 / video->fps.real_fps) * 3 / 2); */
        sys_hi_timer_modify(ff->timer_one_frame, (1000 / fps) * 3 / 2);
    }
#endif
}

void video_dev_real_frame_dec(struct video_device *video)
{
    struct video_fill_frames *ff = &video->ff;

    spin_lock(&ff->lock);

    if (ff->fps_remain > 0) {
        ff->fps_remain--;
    }

    spin_unlock(&ff->lock);
}

static void video_dev_fill_frames_init(struct video_device *video)
{
    struct video_fill_frames *ff = &video->ff;

    spin_lock_init(&ff->lock);

    video_rec_check_fps(video);

    /*ff->timer = sys_hi_timer_add(video, video_rec_check_fps, 2000);*/

    /*if (video->fps.real_fps == video->fps.camera_fps) {
        ff->timer_one_frame = sys_hi_timer_add(video, video_rec_one_frame_timer, 1000);
    }*/
}

void video_dev_fill_frames_reset(struct video_device *video)
{
    struct video_fill_frames *ff = &video->ff;
    spin_lock(&ff->lock);
    ff->time_cnt = 0;
    ff->o_frame_cnt = 0;
    ff->lost_frames = 0;
    spin_unlock(&ff->lock);
}

static void video_dev_fill_frames_uninit(struct video_device *video)
{
#if 0
    struct video_fill_frames *ff = &video->ff;

    if (ff->timer) {
        sys_hi_timer_del(ff->timer);
        ff->timer = 0;
    }

    if (ff->timer_one_frame) {
        sys_hi_timer_del(ff->timer_one_frame);
    }
#endif
}


static int video_dev_reqbufs(void *_video, struct video_reqbufs *b)
{
    struct video_device *video = (struct video_device *)_video;

    return videobuf_reqbufs(&video->video_q, b);
}


static int video_dev_qbuf(void *_video, struct video_buffer *b)
{
    struct video_device *video = (struct video_device *)_video;

    return videobuf_qbuf(&video->video_q, b);
}

static int video_dev_dqbuf(void *_video, struct video_buffer *b)
{
    struct video_device *video = (struct video_device *)_video;
    return videobuf_dqbuf(&video->video_q, b);
}



static void video_dev_dbuf_release(void *_video)
{
    struct video_device *video = (struct video_device *)_video;
    videobuf_streamoff(&video->video_q, 0);
    videobuf_queue_release(&video->video_q);
    video->mal_dbuf = 0;
}
static int video_dev_reqdbuf(void *_video, struct video_reqbufs *b)
{
    struct video_device *video = (struct video_device *)_video;
    int err = 0;
    u8 channel = 0;
    err = videobuf_reqbufs(&video->video_q, b);
    if (!err) {
        err = videobuf_streamon(&video->video_q, (u8 *)&channel);
        video->mal_dbuf = 1;
    }
    return err;
}
static int video_dev_idbuf(void *_video, struct video_buffer *b)
{
    struct video_device *video = (struct video_device *)_video;

    return videobuf_qbuf(&video->video_q, b);
}
static int video_dev_rdbuf(void *_video, struct video_buffer *b)
{
    struct video_device *video = (struct video_device *)_video;

    videobuf_dqbuf(&video->video_q, b);
    return 0;
}


static int video_ioctrl(void *_video, u32 cmd, void *arg)
{
    struct video_device *video = (struct video_device *)_video;

    if (video->dev_ops && video->dev_ops->ioctl) {
        return video->dev_ops->ioctl(video, cmd, (u32)arg);
    }

    return 0;
}


/*int video_subdev_request(struct video_endpoint *ep, int req, void *arg)
{
//	int err;
    struct video_endpoint *p;
    struct video_device *video = (struct video_device *)ep->parent;

    list_for_each_endpoint(p, video) {
        if (p == ep) {
            continue;
        }
        if (p->dev->ops->response) {
            p->dev->ops->response(p, req, arg);
        }
    }

    if (req == VIDREQ_IMAGE_CAPTURE_COMPLETED) {
        //video->icap->baddr = (u8 *)video_buf_ptr(arg);
        video->icap->size = (u32)arg;//video_buf_size(arg);
        [>puts("buf & size : \n");
        put_u32hex((u32)video->icap->baddr);
        put_u32hex(video->icap->size);<]
        os_sem_post(&video->sem);
    }

    return 0;
}*/


static int video_overlay(struct video_device *_video, unsigned int on)
{
    int err = -EINVAL;
    struct video_device *video = (struct video_device *)_video;

    if (video->dev_ops && video->dev_ops->overlay) {
        err = video->dev_ops->overlay(video, on);
    }

    return err;
}


static int video_streamon(struct video_device *_video, int channel)
{
    int err;
    struct video_device *video = (struct video_device *)_video;

    videobuf_streamon(&video->video_q, (u8 *)channel);

    if (video->streamon++) {
        return 0;
    }

    if (video->dev_ops && video->dev_ops->streamon) {
        err = video->dev_ops->streamon(video);
        if (err) {
            videobuf_streamoff(&video->video_q, channel);
            return err;
        }
    }
    if ((video->major == 0 && video->mijor == 0) || (video->major == 4 && video->mijor == 0)) {
        video->time_base = 1;
    }
    video->start_time = jiffies;
    video->stop_time = jiffies + msecs_to_jiffies(video->stop_frame_interval * 1000);

    printf("video:%x video->stop_time:%d", (u32)_video, video->stop_time);
    video_dev_fill_frames_init(video);

    return 0;
}


static int video_streamoff(void *_video, int index)
{
    int err;
    struct video_device *video = (struct video_device *)_video;

    if (--video->streamon == 0) {
        if (video->dev_ops && video->dev_ops->streamoff) {
            video->dev_ops->streamoff(video);
        }

        video_dev_fill_frames_uninit(video);
    }

    err = videobuf_streamoff(&video->video_q, index);
    if (err) {
        return err;
    }

    return 0;
}


static int video_remove_image_buf(struct video_device *video, void *arg)
{

    struct video_image_capture *icap = (struct video_image_capture *)arg;
    struct videobuf_buffer *b = NULL;

    b = container_of(video->icap->baddr, struct videobuf_buffer, data);
    ASSERT(video->icap->baddr != NULL, "video_remove_image_buf\n");

    videobuf_stream_free(&video->video_q, b);

    return 0;
}

static int video_image_capture(void *_video, struct video_image_capture *icap)
{
    int err = 0;
    struct video_device *video = (struct video_device *)_video;

    if (video->dev_ops && video->dev_ops->image_capture) {
        err = video->dev_ops->image_capture(video, icap);
    }

    return err;
}




static int video_init(const struct dev_node *node, void *_data)
{
    struct video_device_ops *ops;
    const struct video_platform_data *data = (const struct video_platform_data *)_data;

    list_for_each_video_device(ops) {
        if (!ASCII_StrCmp(ops->name, node->name, -1)) {
            if (ops->init) {
                ops->init(node->name, data);
            }
        }
    }

    return 0;
}

static bool video_online(const struct dev_node *node)
{
    struct video_device_ops *ops;

    list_for_each_video_device(ops) {
        if (!ASCII_StrCmp(ops->name, node->name, -1)) {
            if (ops->online) {
                return ops->online((void *)node->name);
            }
        }
    }

    return false;
}


static int video_open(const char *name, struct device **device, void *arg)
{
    int major = 0;
    int mijor = 0;
    struct video_device *video, *n;
    struct video_device_ops *ops;

    log_debug("video_device_open: %s\n", name);
    sscanf(name, "video%d.%d", &major, &mijor);

    video = (struct video_device *)zalloc(sizeof(*video));
    if (!video) {
        log_error("no mem\n");
        return -ENOMEM;
    }

    video->major = major;
    video->mijor = mijor;

    list_for_each_video_device(ops) {
        printf("ops->name:%s  name:%s\n", ops->name, name);
        if (!ASCII_StrCmp(ops->name, name, -1)) {
            int err = ops->open(video);
            if (err) {
                free(video);
                return err;
            }
            video->dev_ops = ops;
            break;
        }
    }

    *device = &video->device;
    (*device)->private_data = video;
    os_sem_create(&video->sem, 0);
    videobuf_queue_init(&video->video_q, 32, name);

    return 0;
}

static int video_set_fmt(struct video_device *video, struct video_format *f)
{
    if (video->dev_ops && video->dev_ops->set_fmt) {
        if (f->type == VIDEO_BUF_TYPE_VIDEO_CAPTURE) {
            video->pixelformat = f->pixelformat;
        }
        return video->dev_ops->set_fmt(video, f);
    }

    return -EINVAL;
}

static int video_write(struct device *device, void *data, u32 len, u32 addr)
{
    struct video_device *video = (struct video_device *)device->private_data;

    if (video->dev_ops && video->dev_ops->write) {
        if (len == video->dev_ops->write(video, data, len)) {
            return len;
        }
    }

    return 0;
}


static int video_close(struct device *device)
{
    struct video_device *video = (struct video_device *)device->private_data;

    log_debug("video dev close: video%d.%d\n", video->major, video->mijor);

    if (video->dev_ops && video->dev_ops->close) {
        video->dev_ops->close(video);
    }

    if (video->mal_dbuf) {
        video_dev_dbuf_release(video);
    } else {
        videobuf_queue_release(&video->video_q);
    }

    free(video);

    return 0;
}

static int video_ioctl(struct device *device, u32 cmd, u32 arg)
{
    int ret = 0;
    struct video_device *video = (struct video_device *)device->private_data;

    switch (cmd) {
    case VIDIOC_SET_FMT:
        ret = video_set_fmt(video, (struct video_format *)arg);
        log_debug("video_dev_set_fmt: %s\n", !ret ? "suss" : "err");
        break;
    case VIDIOC_OVERLAY:
        ret = video_overlay(video, arg);
        break;
    case VIDIOC_PLAY:
        ret = video_overlay(video, arg);
        break;
    case VIDIOC_STREAM_ON:
        ret = video_streamon(video, arg);
        log_debug("video_dev_streamon: %s\n", !ret ? "suss" : "err");
        break;
    case VIDIOC_STREAM_OFF:
        ret = video_streamoff(video, arg);
        break;
    case VIDIOC_IMAGE_CAPTURE:
        ret = video_image_capture(video, (struct video_image_capture *)arg);
        break;
    case VIDIOC_DEL_IMAGE:
        ret = video_remove_image_buf(video, (void *)arg);
        break;
    case VIDIOC_REQBUFS:
        ret = video_dev_reqbufs(video, (struct video_reqbufs *)arg);
        break;
    case VIDIOC_DQBUF:
        return video_dev_dqbuf(video, (struct video_buffer *)arg);
        break;
    case VIDIOC_QBUF:
        return video_dev_qbuf(video, (struct video_buffer *)arg);
        break;
    case VIDIOC_REQDBUF:
        ret = video_dev_reqdbuf(video, (struct video_reqbufs *)arg);
        break;
    case VIDIOC_QUERYBUF:
        video_buf_query(video, (struct videobuf_state *)arg);
        break;
    case VIDIOC_GET_FPS:
        ret = video_ioctrl(video, cmd, (void *)arg);
        if (ret == 0) {
            struct video_dev_fps *fps = (struct video_dev_fps *)arg;
        }
        break;
    case VIDIOC_SET_FPS:
        ret = video_ioctrl(video, cmd, (void *)arg);
        if (ret == 0) {
            struct video_dev_fps *fps = (struct video_dev_fps *)arg;
            if (video->streamon == 0) {
                memcpy(&video->fps, fps, sizeof(struct video_dev_fps));
            } else {
                struct video_fill_frames *ff = &video->ff;
                spin_lock(&ff->lock);
                video->fps.real_fps = fps->real_fps;
                spin_unlock(&ff->lock);
            }
        }
        log_debug("video_dev_set_fps: %s\n", !ret ? "suss" : "err");
        break;
    case VIDIOC_GET_FRAME_CNT:
        *(u32 *)arg = video->frame_cnt;
        break;
    case VIDIOC_SET_STOP_FRAME_INTERVAL:
        video->stop_frame_interval = arg;
        video->stop_frame_cnt = arg * video->fps.target_fps;
        printf("-------stop_frame_interval: %d %d %d\n", arg, video->stop_frame_interval, video->stop_frame_cnt);
        break;
    case VIDIOC_TRY_STOP_FRAME_INTERVAL:
        video->stop_frame_cnt = arg;
        break;
    case VIDIOC_START_IFREME:
        if (video->dev_ops->start_I_frame) {
            video->dev_ops->start_I_frame(video);
        }
        break;
    default:
        ret = video_ioctrl(video, cmd, (void *)arg);
        break;

    }

    if (ret) {
        printf("video_ioct: err = %d\n", ret);
    }

    return ret;
}


const struct device_operations video_dev_ops = {
    .init = video_init,
    .open = video_open,
    .write = video_write,
    .close = video_close,
    .ioctl = video_ioctl,
    .online = video_online,
};


