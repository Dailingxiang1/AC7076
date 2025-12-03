#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".usb.data.bss")
#pragma data_seg(".usb.data")
#pragma code_seg(".usb.text")
#pragma const_seg(".usb.text.const")
#pragma str_literal_override(".usb.text.const")
#endif

#include "system/includes.h"
/* #include "server/usb_server.h" */
#include "app_config.h"
/* #include "action.h" */
/* #include "storage_device.h" */
/* #include "user_isp_cfg.h" */
/* #include "server/video_server.h" */
#include "uvc_device.h"
#include "usb_config.h"
#include "host_uvc.h"
#include "video_ioctl.h"
#include "video.h"
#include "videobuf.h"
#include "sdk_config.h"

#include "debug.h"
/* #define LOG_TAG_CONST       USB */
#define LOG_TAG             "[USB]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE

#define TIME_UVC_DEBUG		1//uvc时间debug
#define USER_UVC_DEBUG     	0//用于debug uvc一帧数据是否有问题的debug。主要是加上crc和写SD卡做校验
#define LOG_SEM_ENABLE   	0// 信号量debug
#if LOG_SEM_ENABLE
#define sem_putchar(x)                	putchar(x)
#define sem_printf                    	printf
#define sem_put_buf(x,len)				put_buf(x,len)
#else
#define sem_putchar(...)
#define sem_printf(...)
#define sem_put_buf(...)
#endif // LOG_SEM_ENABLE


#define JLUVC_JPEG_SIZE_SET_ENABLE  0	//是否限制jpeg大小
#define UVC_JPEG_BUF_SIZE  (30 * 1024) 	//限制jpeg 大小为30KB,(need enable)

extern void jpeg_dec_flush(void);

u8 *jluvc_jpeg_buf = NULL;
int jluvc_jpeg_buf_len = 0;

static u8 jluvc_start = 0;
static OS_SEM jluvc_sem;

struct uvc_cam_jpeg_t {
    volatile u8 streamon;
    u16 cur_width;
    u16 cur_height;
    void (*ui_refresh_cb)(void);
};
struct uvc_cam_jpeg_t _uvc_cam_jpeg = {0};

#define __this   (&_uvc_cam_jpeg)

static int find_reso(u16 ref_width, u16 ref_height, struct uvc_capability *uvc_cap)
{
    u8 find_width = 0;
    u8 find_height = 0;
    int w_index = 0;
    int h_index = 0;
    int i;

    while (h_index < uvc_cap->reso_num) {
        for (i = w_index; i < uvc_cap->reso_num; i++) {
            if (ref_width == uvc_cap->reso[i].width) {
                find_width = 1;
                w_index = i;
            }
        }

        for (i = h_index; i < uvc_cap->reso_num; i++) {
            if (ref_height == uvc_cap->reso[i].height) {
                find_height = 1;
                h_index = i;
                break;
            }
        }

        if (find_width && find_height) {
            if (uvc_cap->reso[h_index].width == ref_width) {
                return h_index;
            } else if (uvc_cap->reso[w_index].height == ref_height) {
                return w_index;
            } else {
                w_index++;
                h_index++;
                continue;
            }
        } else if (find_height) {
            return h_index;
        } else if (find_width) {
            return w_index;
        } else {
            return 0;
        }
    }

    return 0;
}

/**
 * @brief 设置JLUVC运行状态
 *
 * 设置JLUVC的运行状态，0表示未运行，1表示运行中
 *
 * @param status 要设置的运行状态
 */
void jlset_uvc_status(u8 status)
{
    log_debug("set_uvc_status: %d\n", status);
    jluvc_start = status;
}

/**
 * @brief 获取JLUVC运行状态
 *
 * 返回JLUVC当前的运行状态，0表示未运行，1表示运行中
 *
 * @return u8 JLUVC当前的运行状态
 */
u8 jlget_uvc_status(void)
{
    return jluvc_start;
}

/**
 * @brief 创建JLUVC信号量
 *
 * 初始化并创建JLUVC使用的信号量资源
 */
void jluvc_sem_create(void)
{
    sem_printf("-----%s>>\n", __func__);
    os_sem_create(&jluvc_sem, 0);
}

/**
 * @brief 等待JLUVC信号量
 *
 * 阻塞当前线程，直到JLUVC信号量的计数大于0。
 * 当信号量计数大于0时，线程被唤醒并继续执行。
 */
void jluvc_sem_pend(void)
{
    sem_printf("-----%s>\n", __func__);
    os_sem_pend(&jluvc_sem, 0);
}

/**
 * @brief 释放JLUVC信号量
 *
 * 通过增加信号量计数来释放信号量，允许等待该信号量的线程继续执行
 */
void jluvc_sem_post(void)
{
    sem_printf("-----%s>>\n", __func__);
    os_sem_post(&jluvc_sem);
}

/**
 * @brief 删除JLUVC信号量
 *
 * 释放并删除JLUVC使用的信号量资源
 */
void jluvc_sem_del(void)
{
    sem_printf("-----%s>>\n", __func__);
    os_sem_del(&jluvc_sem, 0);
}

/**
 * @brief 初始化JLUVC缓冲区
 *
 * 分配指定大小的内存块用于JLUVC操作
 *
 * @param size 要分配的内存大小(字节)
 * @return 成功时返回指向分配内存的指针，失败时返回NULL
 */
void *jluvc_buf_init(u16 size)
{
    log_debug("-----%s>>\n", __func__);
    void *ptr = NULL;
    if (ptr == NULL) {
        ptr = malloc(size);
    }
    return ptr;
}

/**
 * @brief 释放JLUVC缓冲区
 *
 * 释放之前分配的JPEG缓冲区内存，并将缓冲区指针置为NULL
 */
void jluvc_buf_deinit(void)
{
    log_debug("-----%s>>\n", __func__);
    if (jluvc_jpeg_buf) {
        free(jluvc_jpeg_buf);
        jluvc_jpeg_buf = NULL;
    }
}

/**
 * @brief 刷新JLUVC界面显示
 *
 * 该函数用于向UI任务发送刷新消息，触发UVC JPEG测试界面的刷新操作。
 * 通过向"ui"任务发送Q_CALLBACK类型消息来实现界面更新。
 *
 * @note 该函数通过任务间通信机制实现界面刷新，不直接操作UI组件
 */
static void jluvc_ui_reflush(void)
{
    if (__this && __this->ui_refresh_cb) {
        __this->ui_refresh_cb();
    }
}
int jluvc_set_refresh_cb(void (*cb)(void))
{
    if (!__this) {
        log_error("%s not find\n", __func__);
        return -1;
    }
    __this->ui_refresh_cb = cb;
    return 0;
}

/**
 * @brief UVC JPEG接收任务处理函数
 * 该函数负责初始化UVC设备，接收JPEG格式的视频流数据，并进行处理。
 * 主要功能包括：设备初始化、视频流控制、JPEG数据接收和处理等。
 * @param arg 任务参数指针，未使用
 * @return 无返回值
 * @note 该任务为循环执行的任务，在系统退出时会进行资源释放
 */
static void uvc_jpeg_recv_task(void *arg)
{
    void *fd = NULL;
    struct uvc_reqbufs breq = {0};
    struct video_buffer b = {0};
    b.timeout = 50;
    b.noblock = 0;
    struct uvc_capability uvc_cap;
    int err = -1;
    int msg[32];
    u32 i = 0;
    if (dev_online("uvc")) {
        const struct video_platform_data *video_data = NULL;
        struct uvc_platform_data *uvc_info = NULL;
        struct video_var_param_info info;
        struct video_format video_f;
        char video_name[6] = {0};
        sprintf(video_name, "video%d", 0);
        extern void *device_platform_data_get(const char *name);
        video_data = device_platform_data_get(video_name);
        if (video_data && video_data->data) {
            for (int i = 0; i < video_data->num; i++) {
                if (video_data->data[i].tag == VIDEO_TAG_UVC) {
                    uvc_info = video_data->data[i].data;
                    break;
                }
            }
        }
        video_f.uvc_id  = 0;
        info.f = &video_f;
        fd = dev_open("uvc", (void *)&info); //指定uvc设备0
        if (fd) {
            dev_ioctl(fd, UVCIOC_QUERYCAP, (unsigned int)&uvc_cap);

            for (int i = 0; i < uvc_cap.reso_num; i++) {
                log_debug("uvc support [%d] %d x %d", i, uvc_cap.reso[i].width, uvc_cap.reso[i].height);
            }

            u8 fmt = uvc_cap.fmt;
            int reso_index = find_reso(uvc_info->width, uvc_info->height, &uvc_cap);
            u16 width = uvc_cap.reso[reso_index].width;
            u16 height = uvc_cap.reso[reso_index].height;
            log_debug("fmt :%d reso : %d, ref size : %d x %d, size : %d x %d.\n", fmt, reso_index, uvc_info->width, uvc_info->height, width, height);

            dev_ioctl(fd, UVCIOC_SET_CAP_SIZE, (unsigned int)(reso_index + 1));

            breq.buf = NULL;
            /* breq.size = UVC_JPEG_BUF_SIZE; */
            breq.size = uvc_info->mem_size;;
            /* breq.size = 300*1024;; */

            err = dev_ioctl(fd, UVCIOC_REQBUFS, (unsigned int)&breq);
            if (!err) {
                dev_ioctl(fd, UVCIOC_STREAM_ON, 0);
            } else {
                err = -1;
                puts("uvc0 video open err1--------------------\n");
            }
        }
    }
    jluvc_sem_create();//create sem
    u8 usr_wdt_cnt = 0;//定时喂狗
    while (1) {
        os_taskq_accept(ARRAY_SIZE(msg), msg);
        /* os_taskq_pend(NULL, msg, ARRAY_SIZE(msg)); */
        if (msg[0] == Q_MSG) {
            if (msg[1] == 0xaa) {
                if (os_task_del_req(OS_TASK_SELF) == OS_TASK_DEL_REQ) {
                    //准备退出线程
                    if (fd) {
                        dev_ioctl(fd, UVCIOC_STREAM_OFF, 0);
                        dev_close(fd);
                    }
                    os_task_del_res(OS_TASK_SELF);
                }
            }
        }
        if (usr_wdt_cnt++ >= 20) {
            usr_wdt_cnt = 0;
            wdt_clear();
        }
        //从uvc 缓存中请求一帧图像
        if (fd) {
            err = dev_ioctl(fd, UVCIOC_DQBUF, (unsigned int)&b);
        }
        if (err || b.len <= 0) { //请求失败
            os_time_dly(1);
            continue;
        }
        //请求成功
        //TODO
        putchar('R');
        log_debug("[msg]>>>>>>>>>>>b.len=%d", b.len);
#if TIME_UVC_DEBUG
        static u32 last_time = 0;
        static u32 curr_time = 0;
        curr_time = jiffies_msec();
        printf("[UVC]T = %d ms\n", curr_time - last_time);
#endif
        //copy current stream to ui
#if JLUVC_JPEG_SIZE_SET_ENABLE
        if (b.len > UVC_JPEG_BUF_SIZE) {
            continue;
        }
        /* if (!jluvc_jpeg_buf && (b.len <= UVC_JPEG_BUF_SIZE)) { //具体要不要限制jpeg的buff大小在这里限制 */
        /* jluvc_jpeg_buf = jluvc_buf_init(b.len); */
        /* } */
#else
        /* if (!jluvc_jpeg_buf) { */
        /* jluvc_jpeg_buf = jluvc_buf_init(b.len); */
        /* } */
#endif //endif JLUVC_JPEG_SIZE_SET_ENABLE
        /* if (jluvc_jpeg_buf [> && (b.len < UVC_JPEG_BUF_SIZE)<]) { //UI刷新数据 */
        if (1 /* && (b.len < UVC_JPEG_BUF_SIZE)*/) { //UI刷新数据
            jlset_uvc_status(1);
#if USER_UVC_DEBUG
            extern u16 CRC16(const void *ptr, u32 len);
            void *pp = (void *)b.baddr;
            int len = b.len;
            FILE *fp;
            s8 path[64] = {0};
            u16 wr_crc = CRC16((u8 *)pp, len);
            u16 rd_crc = 0;
            sprintf(path, "storage/sd0/C/a%d.jpg", i);
            fp = fopen(path, "wb+");
            if (fp) {
                printf("@@@path=%s\n", path);
                fwrite(pp, len, 1, fp);
                i++;
                fclose(fp);
            }
            /* free(pp); */
            void *rp = fopen(path, "rb");
            if (rp) {
                printf("@@@path=%s\n", path);
                void *rd_buf = malloc(len);
                fread(rd_buf, len, 1, rp);
                rd_crc = CRC16((u8 *)rd_buf, len);
                fclose(rp);
                free(rd_buf);
                rd_buf = NULL;
            }
            printf("wr_crc=%d, rd_crc=%d\n", wr_crc, rd_crc);
#endif//endif

            /* memset(jluvc_jpeg_buf, 0, b.len); */
            /* memcpy(jluvc_jpeg_buf, (u8 *)b.baddr, b.len); */
            /* jluvc_jpeg_buf_len = b.len; */
#if TCFG_UI_ENABLE
            int jljpeg_stream_src_data_copy(u8 * buf, int size);
            if (!jljpeg_stream_src_data_copy((u8 *)b.baddr, b.len)) {
                jluvc_ui_reflush();
            }
#endif
            /* jluvc_sem_post(); */
        }

#if TIME_UVC_DEBUG
        last_time = curr_time;
#endif
        // put_buf((u8 *)b.baddr+ b.len -512 ,512);
        /* if (f) { */
        /* fwrite(f, b.baddr, b.len); */
        /* } */
#if UVC_JPG_DATA_WRITE2SD
        /// fwrite 一帧数据到SD卡中。
        void *pp = b.baddr;
        int len = b.len;
        FILE *fp;
        u8 path[64] = {0};
        sprintf(path, "storage/sd0/C/avid_%d.jpg", i);
        fp = fopen(path, "wb+");
        if (fp) {
            printf("@@@path=%s\n", path);
            fwrite(pp, len, 1, fp);
            i++;
            fclose(fp);
        }
        free(pp);
#endif //endif UVC_JPG_DATA_WRITE2SD

        dev_ioctl(fd, UVCIOC_QBUF, (unsigned int)&b);
    }

}

int video_uvc_jpeg_stream_open(u16 width, u16 height)
{
    log_debug("video_uvc_jpeg_stream_open\n");
    if (__this->streamon) {
        return -1;
    }
    memset(__this, 0x00, sizeof(struct uvc_cam_jpeg_t));
    __this->cur_width = width;
    __this->cur_height = height;
    __this->streamon = 1;
    return os_task_create(uvc_jpeg_recv_task, NULL, 20, 1024, 32, "uvc_jpeg_recv");
}

int video_uvc_jpeg_stream_close()
{
    while (OS_TASK_NOT_EXIST != os_task_del_req((const char *)"uvc_jpeg_recv")) {
        os_taskq_post_msg((const char *)"uvc_jpeg_recv", 1,  0xaa);
        os_time_dly(1);
    }
    jluvc_sem_del();
    jluvc_buf_deinit();
    __this->streamon = 0;

    return 0;
}

