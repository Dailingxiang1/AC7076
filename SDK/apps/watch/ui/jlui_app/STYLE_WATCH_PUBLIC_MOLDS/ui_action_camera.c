

#include "app_config.h"
/* #include "app_task.h" */
#include "system/timer.h"
#include "device/device.h"
#include  "dev_manager.h"
#include "key_event_deal.h"

#include "res/resfile.h"
#include "jlui/ui.h"
#include "jlui_app/ui_style.h"
#include "jlui_app/ui_api.h"
#include "jlui_app/res_config.h"
#include "jlui_app/ui_resource.h"
#include "jlui_app/ui_sys_param.h"

#include "video/video_rec.h"
#include "video/avi/avilib.h"
#include "video/avi_video.h"
#include "jpeg_stream.h"

#define LOG_TAG_CONST       UI
#define LOG_TAG     		"[UI_DRAW_DEMO]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ui_action_draw_demo.data.bss")
#pragma data_seg(".ui_action_draw_demo.data")
#pragma const_seg(".ui_action_draw_demo.text.const")
#pragma code_seg(".ui_action_draw_demo.text")
#endif

#ifdef CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE
#if TCFG_UI_CAMERA_ENABLE

#define STYLE_NAME  JL
REGISTER_UI_STYLE(STYLE_NAME)





#define DEV_ROOT	"storage/sd0/C/"
#define PHOTO_DIR 	"PHOTO/"
#define VIDEO_DIR	"VIDEO/"
#define PHOTO_EXT 	"JPG"
#define VIDEO_EXT 	"AVI"
#define PHOTO_NAME	"CAM_****"						//****由文件系统自增
#define VIDEO_NAME	"CAM_****"						//****由文件系统自增
#define PHOTO_PATH	DEV_ROOT PHOTO_DIR PHOTO_NAME "." PHOTO_EXT 			//照片路径
#define VIDEO_PATH  DEV_ROOT VIDEO_DIR VIDEO_NAME "." VIDEO_EXT 			//视频路径

#define VIEW_ITEM_NUM	6
struct brower_set_info {
int flist_index;  							//文件列表首项所指的索引
int cur_total;								//文件数
FILE *file;									//文件句柄
struct vfscan *fs;							//文件系统句柄
FS_DIR_INFO *dir_buf;						//文件(夹)信息
int show_temp;								//显示项
#if (TCFG_LFN_EN)
u8  lfn_buf[512];							//长文件名
#endif//TCFG_LFN_EN

};
#define PHOTO_CONTINUE_SAVE					3
struct camera_ctrl {
u32 layout_curr;								//当前布局id
u32 video_rec_time;								//录像开始时间戳，用于计时
u8 view_video;									//1 查看录像 0 查看照片
u8 video_doing;									//录像中状态
u16 view_remap[VIEW_ITEM_NUM];
u8 view_vaild[VIEW_ITEM_NUM];
volatile s8 photo_save_cnt;						//
struct brower_set_info brower_info;				//文件浏览
char *sel_path;
} __camera_ctrl;
#define __this (&__camera_ctrl)						//相机句柄



static int cam_camera_video_sw();
//****************************************************************************************//
//								功能接口
//****************************************************************************************//
static int cam_ctrl_init()
{
    __this->layout_curr = CAM_MAIN_LAYOUT;
    __this->video_rec_time = 0;
    __this->video_doing = 0;
    /* __this->view_video = 1; */
    return 0;
}
static int cam_ctrl_deinit()
{
    return 0;
}
/* ------------------------------------------------------------------------------------*/
/**
 * @brief  camera_dec_reflush_sync相机画面刷新 (外部调用,线程同步)
 */
/* ------------------------------------------------------------------------------------*/
static void camera_dec_flush(void)
{
    struct element *elm;
    elm = ui_core_get_element_by_id(CAM_CAMERA_LAYOUT);
    if (elm != NULL) {
        /* printf(">>>>>>elm != NULL"); */
        ui_redraw(CAM_CAMERA_LAYOUT);
    }
}
static void camera_rec_err()
{
    cam_camera_video_sw();
    ui_show(CAM_WARNING_LAYOUT);
}
void camera_dec_reflush_sync(int status)
{
    void jlui_malloc_ram_info_dump();
    /* jlui_malloc_ram_info_dump(); */
    log_debug("%s status:%d !!\n", __func__, status);

    int msg[3] = {0};

    if (!status) {
        //正常刷新
        msg[0] = (int)camera_dec_flush;
    } else {
        //异常显示
        msg[0] = (int) camera_rec_err;
    }
    msg[1] = 1;
    msg[2] = 0;
    int ret = os_taskq_post_type("ui", Q_CALLBACK, 3, msg);


}
/* ------------------------------------------------------------------------------------*/
/**
 * @brief cam_layout_sw 切换当前布局
 *
 * @param layout 要切换的布局id
 *
 * @return
 */
/* ------------------------------------------------------------------------------------*/
static int cam_layout_sw(int layout)
{
    log_debug("%s hide:0x%x show:0x%x", __func__, __this->layout_curr, layout);
    ui_hide(__this->layout_curr);
    ui_show(layout);
    __this->layout_curr = layout;
    return 0;
}

/* ------------------------------------------------------------------------------------*/
/**
 * @brief cam_ctrl_view_video  切换为录像视频列表
 *
 * @param enable 选择录像视频列表
 *
 * @return
 */
/* ------------------------------------------------------------------------------------*/
static int cam_ctrl_view_video(int enable)
{
    if (__this->view_video != enable) {
        __this->view_video = enable;
        //reload layout
        ui_hide(CAM_PHOTO_LAYOUT);
        /* ui_core_hide(ui_core_get_element_by_id(CAM_PHOTO_LAYOUT)); */
        ui_show(CAM_PHOTO_LAYOUT);
    }
    return 0;
}
static void cam_camera_photo_savc_cb(char *path)
{
    if (jljpeg_stream_src_data_get()) {
        return;
    }
    jljpeg_stream_src_data_save_to_file(path);
    if (__this && (__this->photo_save_cnt > 0)) {
        __this->photo_save_cnt--;
    }
}
/* ------------------------------------------------------------------------------------*/
/**
 * @brief cam_camera_photo_save 拍照(录像时也可以调用)
 *
 * @return
 */
/* ------------------------------------------------------------------------------------*/
static int cam_camera_photo_save()
{
    if (__this && (__this->photo_save_cnt < PHOTO_CONTINUE_SAVE)) {
        __this->photo_save_cnt++;
        char *filename = PHOTO_PATH;//	"storage/sd0/C/CAM_****.jpg";
        int msg[3] = {0};
        msg[0] = (int)cam_camera_photo_savc_cb;
        msg[1] = 1;
        msg[2] = (u32)filename;
        int ret = os_taskq_post_type(jlcamera_video_get_task_name(), Q_CALLBACK, 3, msg);
        return ret;
    }
    return 0;
}
/* ------------------------------------------------------------------------------------*/
/**
 * @brief cam_camera_video_sw 开关录像功能
 *
 * @return
 */
/* ------------------------------------------------------------------------------------*/
static int cam_camera_video_sw()
{
    int ret = 0;
    //控制录像开始/暂停
    if (__this->video_doing) {
        ret = jlcamera_video_rec_stop();
    } else {
        char *filename = VIDEO_PATH;//	"storage/sd0/C/CAM_****.jpg";
        ret = jlcamera_video_rec_start(filename, -1);
        __this->video_rec_time = (u32)jiffies_msec();
    }
    //更新录像状态
    if (!ret) {
        __this->video_doing = !__this->video_doing;
    }
    //录像计时布局
    struct element *elm = ui_core_get_element_by_id(CAM_CAMERA_TIME_LAYOUT);
    if (elm) {
        if (__this->video_doing) {
            elm->css.top = 0;
        } else {
            elm->css.top = -elm->css.height;
        }
    }
    return ret;
}
static int cam_file_handler_close()
{
    if (__this->brower_info.fs) {
        fscan_release(__this->brower_info.fs);
        __this->brower_info.fs = NULL;
    }
    if (__this->brower_info.file) {
        fclose(__this->brower_info.file);
        __this->brower_info.file = NULL;
    }

    if (__this->brower_info.dir_buf) {
        free(__this->brower_info.dir_buf);
        __this->brower_info.dir_buf = NULL;
    }
    return 0;
}

static int cam_file_handler_open(int show_temp, char *dir, char *ext_name)
{
    cam_file_handler_close();
    if (!dev_manager_get_total(1)) {// 获取有效可播放设备数量
        return -1;
    }
    struct __dev *dev = dev_manager_find_active(1);//在有效设备中获取活跃设备
    if (!dev) {
        return -1;
    }

    log_debug("dev_root:%s dir:%s ext:%s \n", dev_manager_get_root_path(dev), dir, ext_name);
    //活跃分区 与目标一致
    if (!strstr(dev_manager_get_root_path(dev), DEV_ROOT)) {
        return -1;
    }
    char path[64] = {0};
    strcat(path, DEV_ROOT);
    strcat(path, dir);
    __this->brower_info.show_temp = show_temp + !show_temp;
    __this->brower_info.dir_buf = zalloc(sizeof(FS_DIR_INFO) *  __this->brower_info.show_temp);
    log_debug("%s %d path:%s", __func__, __LINE__, path);
    fset_ext_type(path, "ALL");	//设置后缀类型
    __this->brower_info.cur_total = fopen_dir_info(path, &__this->brower_info.file, 0); //打开目录
    if (!__this->brower_info.file) {
        goto __err;
    }
    //打开根目录
    __this->brower_info.cur_total = fenter_dir_info(__this->brower_info.file, __this->brower_info.dir_buf);//进入目录

    __this->brower_info.flist_index = 1;//记录索引

    log_debug("%s %d total:%d ", __func__, __LINE__, __this->brower_info.cur_total);
    if (!__this->brower_info.cur_total) {
        goto __err;
    }
    //打开一级目录，区分视频or图片
    /* for (int index = __this->brower_info.cur_total - 1; index >= 0; index--) { */
    for (int index = 1; index <= __this->brower_info.cur_total; index++) {
        fget_dir_info(__this->brower_info.file, index, 1, &__this->brower_info.dir_buf[0]);
        FS_DIR_INFO *dir_info  = &__this->brower_info.dir_buf[0];
        log_debug("%s idx:%d dir:%d fn:%d %s", __func__, index, dir_info->dir_type, dir_info->fn_type, dir_info->lfn_buf.lfn);
        /* put_buf((u8 *)dir_info->lfn_buf.lfn, 20); */
        if (!strncmp(dir_info->lfn_buf.lfn, dir, strlen(dir) - 1)) {
            log_debug("find it!!!!!\n");
            __this->brower_info.cur_total = fenter_dir_info(__this->brower_info.file, dir_info);
            log_debug("%s dir:%s total:%d ", __func__, dir, __this->brower_info.cur_total);
            if (!__this->brower_info.cur_total) {
                goto __err;
            }
            return 0;
            /* break; */
        }
    }
__err:
    cam_file_handler_close();
    return -1;
}
static FS_DIR_INFO *cam_file_list_read_by_index(u32 index)
{
    index = index - 1;//index 是 无符号 ,0 -1 会变成-1

    if (!__this->brower_info.file) {
        return NULL;
    }
    if (index >= __this->brower_info.cur_total) {
        return NULL;
    }
    index ++;
    index = index - __this->brower_info.flist_index;
    int i = index % __this->brower_info.show_temp;
    log_debug("%s idx:%x i:%x ", __func__, index, i);
    return &__this->brower_info.dir_buf[i];
}

static int cam_handler_prepare_cb(void *ctrl, int count, int start)
{
    FILE *f = NULL;
    struct vfs_attr attr;

    if (!__this || !__this->brower_info.file) {
        return 0;
    }

    if (!start  && (1 == __this->brower_info.flist_index)) {
        //针对start 是 0情况判断是否需要更新buf，减小重刷
        return 0;
    }

    start = !start + start;//针对0的情况

    if (start > __this->brower_info.cur_total) {
        return 0;
    }

    int index = start - __this->brower_info.flist_index;
    int i = index % __this->brower_info.show_temp;
    log_debug("%s start:%x i:%x ", __func__, start, i);
    fget_dir_info(__this->brower_info.file, start, 1, &__this->brower_info.dir_buf[i]);

    return 0;
}

//****************************************************************************************//
//							    消息处理
//****************************************************************************************//
static int ui_video_show_exit_handler(const char *type, u32 arg)
{
    log_info("[%s]", __func__);

    switch (__this->layout_curr) {
    case CAM_SHOW_LAYOUT:
        cam_layout_sw(CAM_PHOTO_LAYOUT);
        break;
    default:
        break;
    }
    return true;
}

static const struct uimsg_handl ui_msg_handler[] = {
    { "video_show_exit",          ui_video_show_exit_handler         },
    { NULL, NULL},      /* 必须以此结尾！ */
};

//****************************************************************************************//
//								提醒
//****************************************************************************************//
static int cam_warning_layout_ontouch(void *_ctrl, struct element_touch_event *e)
{
    struct element *elm = (struct element *)_ctrl;
    switch (e->event) {
    case ELM_EVENT_TOUCH_R_MOVE:
        ui_hide(elm->id);
        return true;
    default:
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(CAM_WARNING_LAYOUT)
.onchange =  NULL,
 .onkey = NULL,
  .ontouch =  cam_warning_layout_ontouch,
};
//****************************************************************************************//
//								主页面
//****************************************************************************************//
static int cam_layer_onchange(void *ctrl, enum element_change_event event, void *arg)
{
    switch (event) {
    case ON_CHANGE_INIT:

        ui_auto_shut_down_disable();
        //图层初始化变量
        /* f_format("sd0","fat",0); */
        cam_ctrl_init();
        ui_register_msg_handler(ID_WINDOW_CAMERA, ui_msg_handler);
        break;
    case ON_CHANGE_RELEASE:
        cam_ctrl_deinit();

        ui_auto_shut_down_enable();
        break;
    default:
        break;
    }
    return false;
}
static int cam_layer_ontouch(void *_ctrl, struct element_touch_event *e)
{
    struct element *elm = (struct element *)_ctrl;
    switch (e->event) {
    case ELM_EVENT_TOUCH_R_MOVE:
        if (!__this) {
            return false;
        }
        switch (__this->layout_curr) {
        case CAM_SHOW_LAYOUT:
            cam_layout_sw(CAM_PHOTO_LAYOUT);
            break;
        case CAM_PHOTO_LAYOUT:
        case CAM_CAMERA_LAYOUT:
            cam_layout_sw(CAM_MAIN_LAYOUT);
            break;
        case CAM_MAIN_LAYOUT:
            return false;
            break;
        default:
            break;
        }
        return true;
    default:
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(CAM_LAYER)
.onchange =  cam_layer_onchange,
 .onkey = NULL,
  .ontouch =  cam_layer_ontouch,
};
/* ------------------------------------------------------------------------------------*/
/**
 * @brief cam_main_button_ontouch 主页面按键
 *
 * @param _ctrl
 * @param e
 *
 * @return
 */
/* ------------------------------------------------------------------------------------*/
static int cam_main_button_ontouch(void *_ctrl, struct element_touch_event *e)
{
    struct element *elm = (struct element *)_ctrl;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
    case ELM_EVENT_TOUCH_UP:
        if (e->move_flag) {
            break;
        }
        //相机
        if (elm->id == CAM_CAMERA_MAIN_ENTER) {
            cam_layout_sw(CAM_CAMERA_LAYOUT);
        }
        //相册
        else if (elm->id == CAM_PHOTO_MAIN_ENTER) {
            cam_layout_sw(CAM_PHOTO_LAYOUT);
        }
        return true;
    default:
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(CAM_CAMERA_MAIN_ENTER)
.onchange =  NULL,
 .onkey = NULL,
  .ontouch = cam_main_button_ontouch,
};
REGISTER_UI_EVENT_HANDLER(CAM_PHOTO_MAIN_ENTER)
.onchange =  NULL,
 .onkey = NULL,
  .ontouch = cam_main_button_ontouch,
};

//****************************************************************************************//
//									相机
//****************************************************************************************//

/* ------------------------------------------------------------------------------------*/
/**
 * @brief cam_camera_layout_onchange 相机功能
 *
 * @param ctrl
 * @param event
 * @param arg
 *
 * @return
 */
/* ------------------------------------------------------------------------------------*/
static int cam_camera_layout_onchange(void *ctrl, enum element_change_event event, void *arg)
{
    struct element *elm = (struct element *)ctrl;
    struct draw_context *dc = NULL;
    switch (event) {
    case ON_CHANGE_INIT:
        //初始化jpeg数据流解码显示
        jljpeg_stream_init();
        //初始化相机
        jlcamera_video_rec_init();
        //注册刷新回调
        jlcamera_video_rec_refresh_cb(camera_dec_reflush_sync);
        break;
    case ON_CHANGE_SHOW_POST:
        if (!jljpeg_stream_src_data_len_get()) {
            //码流数据为空时不显示
            break;
        }
        dc = (struct draw_context *)arg;
        ui_custom_draw_clear(dc);
        //实时显示画面
        jpeg_image_ram(dc, 40, 0, 240, 320,
                       jljpeg_stream_src_data_get(),
                       jljpeg_stream_src_data_len_get());
        break;
    case ON_CHANGE_RELEASE:
        if (__this->video_doing) {
            jlcamera_video_rec_stop();
            __this->video_doing = 0;
        }
        //关闭摄像头
        jlcamera_video_rec_deinit();
        //关闭jpeg数据流解码
        jljpeg_stream_deinit();
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(CAM_CAMERA_LAYOUT)
.onchange =  cam_camera_layout_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};


/* ------------------------------------------------------------------------------------*/
/**
 * @brief cam_camera_button_ontouch 相机页面按键
 *
 * @param _ctrl
 * @param e
 *
 * @return
 */
/* ------------------------------------------------------------------------------------*/
static int cam_camera_button_ontouch(void *_ctrl, struct element_touch_event *e)
{
    struct element *elm = (struct element *)_ctrl;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
    case ELM_EVENT_TOUCH_UP:
        if (e->move_flag) {
            break;
        }
        //拍照
        if (elm->id == CAM_CAMERA_CTRL) {
            cam_camera_photo_save();
        }
        //录像
        else if (elm->id == CAM_VIDEO_CTRL) {
            cam_camera_video_sw();
        }
        //进入相册
        else if (elm->id == CAM_PHOTO_CTRL) {
            cam_layout_sw(CAM_PHOTO_LAYOUT);
        }
        return true;
    default:
        break;
    }
    return false;
}
static int cam_video_ctrl_onchange(void *ctrl, enum element_change_event event, void *arg)
{
    struct element *elm = (struct element *)ctrl;
    struct draw_context *dc = (struct draw_context *)arg;
    if (event == ON_CHANGE_SHOW) {
        dc->custom_color = BIT(UI_CUSTOM_COLOR_BIT_IMAGE);
        //录像时，录像按键显示为红色
        if (__this->video_doing) {
            dc->custom_argb8888 = 0xffff0000;//红
        } else {
            dc->custom_argb8888 = 0xffffffff;//白
        }
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(CAM_CAMERA_CTRL)
.onchange =  NULL,
 .onkey = NULL,
  .ontouch =  cam_camera_button_ontouch,
};
REGISTER_UI_EVENT_HANDLER(CAM_VIDEO_CTRL)
.onchange =  cam_video_ctrl_onchange,
 .onkey = NULL,
  .ontouch =  cam_camera_button_ontouch,
};
REGISTER_UI_EVENT_HANDLER(CAM_PHOTO_CTRL)
.onchange = NULL,
 .onkey = NULL,
  .ontouch =  cam_camera_button_ontouch,
};


/* ------------------------------------------------------------------------------------*/
/**
 * @brief cam_camera_time_onchange 录像时间
 *
 * @param ctrl
 * @param event
 * @param arg
 *
 * @return
 */
/* ------------------------------------------------------------------------------------*/
static int cam_camera_time_onchange(void *ctrl, enum element_change_event event, void *arg)
{
    struct element *elm = (struct element *)ctrl;
    if (event == ON_CHANGE_SHOW_PROBE) {
        //使用系统tick做差，减少计时误差
        u32 time = jiffies_msec2offset(__this->video_rec_time, jiffies_msec()) / 1000;
        struct utime t = {0};
        t.sec = time % 60;
        t.min = (time / 60) % 60;
        t.hour = time / 3600;
        ui_time_update((struct ui_time *)elm, &t);
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(CAM_CAMERA_TIME)
.onchange =   cam_camera_time_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
static int cam_camera_time_layout_onchange(void *ctrl, enum element_change_event event, void *arg)
{
    struct element *elm = (struct element *)ctrl;
    if (event == ON_CHANGE_SHOW) {
        if (__this->video_doing) {
            elm->css.top = 0;
        } else {
            elm->css.top = -elm->css.height;
        }
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(CAM_CAMERA_TIME_LAYOUT)
.onchange =   cam_camera_time_layout_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
//****************************************************************************************//
//									相册
//****************************************************************************************//
void *animig_open(char *name, int window_id, int arg);
/* ------------------------------------------------------------------------------------*/
/**
 * @brief cam_photo_list_child_onchange 子控件
 *
 * @param ctrl
 * @param event
 * @param arg
 *
 * @return
 */
/* ------------------------------------------------------------------------------------*/
static int cam_photo_list_child_onchange(void *ctrl, enum element_change_event event, void *arg)
{
    struct element *elm = (struct element *)ctrl;
    if (event == ON_CHANGE_UPDATE_ITEM) {
        int index = (int)arg;
        log_debug("%s update: elmid:0x%x index%d", __func__, elm->id, index);
        struct ui_pic *pic = (struct ui_pic *)elm;
        if (!strncmp(pic->source, "view", 4)) {
            int idx = pic->source[4] - '0';
            ASSERT((idx >= 0) && (idx < VIEW_ITEM_NUM));
            __this->view_remap[idx] = index;
        }
    }
    if (event == ON_CHANGE_SHOW_POST) {
        if (ui_id2type(elm->id) == CTRL_TYPE_TEXT) {
            struct draw_context *dc = (struct draw_context *)arg;
            int index = 0;
            struct ui_text *text = (struct ui_text *)elm;
            if (!strncmp(text->source, "view", 4)) {
                int idx = text->source[4] - '0';
                ASSERT((idx >= 0) && (idx < VIEW_ITEM_NUM));
                index = __this->view_remap[idx];
            }
            cam_handler_prepare_cb(elm, 1, index + 1);
            FS_DIR_INFO *info = cam_file_list_read_by_index(index + 1);

            log_debug("%s show: elmid:0x%x index%d info:0x%x", __func__, elm->id, index, (u32)info);
            if (!info) {
                jlgpu_scheduler_wait_sync();
                jlgpu_task_clean_up_by_id(dc->gpu_task_head, dc->elm->id, 0x0);
                return true;
            }
        } else if (ui_id2type(elm->id) == CTRL_TYPE_PIC) {
            struct draw_context *dc = (struct draw_context *)arg;
            int index = 0;
            int idx  = 0;
            struct ui_pic *pic = (struct ui_pic *)elm;
            if (!strncmp(pic->source, "view", 4)) {
                idx = pic->source[4] - '0';
                ASSERT((idx >= 0) && (idx < VIEW_ITEM_NUM));
                index = __this->view_remap[idx];
            }
            log_debug("%s show: elmid:0x%x index%d", __func__, elm->id, index);
            cam_handler_prepare_cb(elm, 1, index + 1);
            FS_DIR_INFO *info = cam_file_list_read_by_index(index + 1);
            if (!info) {
                ui_custom_draw_clear(dc);
                return false;
            }
            log_debug("%s index:%d fn:%d", __func__, index, info->fn_type);
            if (!info->fn_type) {
                log_debug("file_name:%s ", info->lfn_buf.lfn);
                /* } else { */
                /* put_buf((u8 *)info->lfn_buf.lfn, 20); */
            }
            char path[64] = {0};
            strcat(path, DEV_ROOT);
            if (__this->view_video) {
                strcat(path, VIDEO_DIR);
            } else {
                strcat(path, PHOTO_DIR);
            }
            strncat(path, info->lfn_buf.lfn, 8);
            strcat(path, ".");
            if (__this->view_video) {
                strcat(path, VIDEO_EXT);
            } else {
                strcat(path, PHOTO_EXT);
            }
            log_debug("%s path:%s", __func__, path);
            struct rect r;
            ui_core_get_element_abs_rect(elm, &r);
            /* jpeg_image_file(dc, r.left, r.top, 240, 320, (u8 *)path, strlen(path)); */
            //avi时显示预览图
            int jpg_ret = jpeg_image_file_psram(dc, r.left, r.top, 240, 320, path, strlen(path), 1, 0.625f);
            if (jpg_ret) {
                ui_custom_draw_clear(dc);
                __this->view_vaild[idx] = 0;
            } else {
                __this->view_vaild[idx] = 1;
            }
            log_debug("%s idx:%d index:%d vaild:%d", __func__, idx, index, __this->view_vaild[idx]);
            log_debug("end");
            /* jpeg_image_file(dc,40,0,240,320,(u8*)path,strlen(path)); */
            /* ui_pic_set_image_index((struct ui_pic*)elm,index); */
        }
    }
    return false;
}

/* ------------------------------------------------------------------------------------*/
/**
 * @brief cam_photo_list_onchange 相册列表
 *
 * @param ctrl
 * @param event
 * @param arg
 *
 * @return
 */
/* ------------------------------------------------------------------------------------*/
static int cam_photo_list_onchange(void *ctrl, enum element_change_event event, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)ctrl;
    switch (event) {
    case ON_CHANGE_INIT:
        if (__this->view_video) {
            cam_file_handler_open(grid->avail_item_num, VIDEO_DIR, VIDEO_EXT);
        } else {
            cam_file_handler_open(grid->avail_item_num, PHOTO_DIR, PHOTO_EXT);
        }
        int col = 2;
        int row = (__this->brower_info.cur_total + 1) / col;

        ui_set_default_handler(&grid->elm, NULL, NULL, cam_photo_list_child_onchange);
        ui_grid_set_slide_direction(grid, SCROLL_DIRECTION_UD);
        ui_grid_init_dynamic(grid, &row, &col);
        break;
    case ON_CHANGE_RELEASE:
        ui_set_default_handler(&(grid->elm), NULL, NULL, NULL);
        cam_file_handler_close();
        break;
    default:
        break;
    }
    return false;
}
static int cam_photo_list_ontouch(void *_ctrl, struct element_touch_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)_ctrl;
    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        if (e->move_flag) {
            break;
        }
        //查看大图
        //get touch_index
        if (ui_grid_touch_item(grid) == -1) {
            break;
        }
        int dyn_idx = ui_grid_cur_item_dynamic(grid);
        for (int i = 0; i < VIEW_ITEM_NUM; i++) {
            log_debug("%s i:%d index:%d vaild:%d dyn:%d", __func__, i, __this->view_remap[i], __this->view_vaild[i], dyn_idx);
            if (__this->view_remap[i] == dyn_idx) {
                if (!__this->view_vaild[i]) {
                    return true;
                } else {
                    break;
                }
            }
        }
        cam_handler_prepare_cb(&grid->elm, 1, dyn_idx + 1);
        FS_DIR_INFO *info = cam_file_list_read_by_index(dyn_idx + 1);
        log_debug("%s idx:%d fn:%d", __func__, dyn_idx, info->fn_type);
        if (!info->fn_type) {
            log_debug("file_name:%s ", info->lfn_buf.lfn);
            /* } else { */
            /* put_buf((u8 *)info->lfn_buf.lfn, 20); */
        }
        __this->sel_path = zalloc(64);
        char *path = __this->sel_path;
        strcat(path, DEV_ROOT);
        if (__this->view_video) {
            strcat(path, VIDEO_DIR);
        } else {
            strcat(path, PHOTO_DIR);
        }
        strncat(path, info->lfn_buf.lfn, 8);
        strcat(path, ".");
        if (__this->view_video) {
            strcat(path, VIDEO_EXT);
        } else {
            strcat(path, PHOTO_EXT);
        }
        log_debug("%s path:%s", __func__, path);

        //show_layout
        cam_layout_sw(CAM_SHOW_LAYOUT);
        return true;
    default:
        break;
    }
    return false;
}


REGISTER_UI_EVENT_HANDLER(CAM_PHOTO_LIST)
.onchange = cam_photo_list_onchange,
 .onkey = NULL,
  .ontouch =  cam_photo_list_ontouch,
};
/* ------------------------------------------------------------------------------------*/
/**
 * @brief cam_view_button_ontouch 切换图片/视频相册
 *
 * @param _ctrl
 * @param e
 *
 * @return
 */
/* ------------------------------------------------------------------------------------*/
static int cam_view_button_ontouch(void *_ctrl, struct element_touch_event *e)
{
    struct element *elm = (struct element *)_ctrl;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
    case ELM_EVENT_TOUCH_UP:
        if (e->move_flag) {
            break;
        }
        if (elm->id == CAM_VIEW_PHOTO) {
            cam_ctrl_view_video(0);
        } else if (elm->id == CAM_VIEW_VIDEO) {
            cam_ctrl_view_video(1);
        }
        return true;
    default:
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(CAM_VIEW_PHOTO)
.onchange = NULL,
 .onkey = NULL,
  .ontouch =  cam_view_button_ontouch,
};
REGISTER_UI_EVENT_HANDLER(CAM_VIEW_VIDEO)
.onchange = NULL,
 .onkey = NULL,
  .ontouch =  cam_view_button_ontouch,
};

static void avi_flush_timer(void *p)
{
    avi_set_avi_playtimer_id(0);
    log_debug("timer");
    ui_redraw((int)p);
    /* wdt_clear(); */
    jlgpu_scheduler_wait_sync();
}
void __jpeg_draw_cb_gpu(int id, u8 *dst_buf, struct rect *dst_r, struct rect *src_r, u8 bytes_per_pixel, void *priv, void *matrix);
static void camera_video_dec_flush(void)
{
    struct element *elm;
    elm = ui_core_get_element_by_id(CAM_SHOW_LAYOUT);
    if (elm != NULL) {
        /* printf(">>>>>>elm != NULL"); */
        ui_redraw(CAM_SHOW_LAYOUT);
    }
}

void video_dec_reflush_sync(int status)
{
    void jlui_malloc_ram_info_dump();
    /* jlui_malloc_ram_info_dump(); */
    log_debug("%s status:%d !!\n", __func__, status);

    int msg[3] = {0};

    if (!status) {
        //正常刷新
        msg[0] = (int) camera_video_dec_flush;
    } else {
        //异常显示
    }
    msg[1] = 1;
    msg[2] = 0;
    int ret = os_taskq_post_type("ui", Q_CALLBACK, 3, msg);
}

#if 0//使用video_dec解码测试.

extern int video_dec_init();
extern int video_dec_deinit();
extern int video_dec_set_path(s8(*path)[64], u8 path_number);
extern int video_dec_start(u8 index, u8 mode);
extern int video_dec_refresh_cb_register(void (*cb)(int status));
extern int video_dec_stop();
static int cam_show_layout_onchange(void *ctrl, enum element_change_event event, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)ctrl;
    struct element *elm = (struct element *)ctrl;
    struct draw_context *dc = (struct draw_context *)arg;
    switch (event) {
    case ON_CHANGE_INIT:
        if (!__this->sel_path) {
            break;
        }
        if (__this->view_video) {
            video_dec_init();
            video_dec_refresh_cb_register(video_dec_reflush_sync);
            video_dec_set_path((s8(*)[64])__this->sel_path, 1);
            video_dec_start(0, 0);
        }
        break;
    case ON_CHANGE_SHOW_POST:
        if (!__this->sel_path) {
            break;
        }
        ui_custom_draw_clear(dc);
        if (__this->view_video) {
            //实时显示画面
            if (!jljpeg_stream_src_data_len_get()) {
                //码流数据为空时不显示
                break;
            }
            jpeg_image_ram(dc, 40, 0, 240, 320,
                           jljpeg_stream_src_data_get(),
                           jljpeg_stream_src_data_len_get());
        }
        break;
    case ON_CHANGE_RELEASE:
        video_dec_stop();
        video_dec_deinit();
        break;
    default:
        break;
    }
    return false;
}
#endif

static int cam_show_layout_onchange1(void *ctrl, enum element_change_event event, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)ctrl;
    struct element *elm = (struct element *)ctrl;
    struct draw_context *dc = (struct draw_context *)arg;
    switch (event) {
    case ON_CHANGE_INIT:
        if (!__this->sel_path) {
            break;
        }
        if (__this->view_video) {
            set_avi_play_mode(0);
            animig_open(__this->sel_path, 0, 3);
        }
        break;
    case ON_CHANGE_SHOW_POST:
        if (!__this->sel_path) {
            break;
        }
        if (__this->view_video) {
            if (!get_aviplay_handle()) {
                break;
            }
            u16 timer_id = 0;
            if (!avi_get_avi_playtimer_id()) {
                timer_id = sys_timeout_add((void *)elm->id, (avi_flush_timer), 10); // 强制满帧刷新
                avi_set_avi_playtimer_id(timer_id);
            }
            u32 avip = (u32)get_avi_player_st_handle();
            log_debug("player->st %x %d %d ", avip, avi_get_width(get_avi_player_st_handle()), avi_get_height(get_avi_player_st_handle()));
            ui_draw(dc,
                    NULL,
                    0,
                    0,
                    avi_get_width(get_avi_player_st_handle()),
                    avi_get_height(get_avi_player_st_handle()),
                    __jpeg_draw_cb_gpu,
                    &avip,
                    sizeof(avip),
                    1);
        } else {
            struct rect r;
            ui_core_get_element_abs_rect(elm, &r);
            jpeg_image_file_psram(dc, 40, 30, 240, 320, __this->sel_path, strlen(__this->sel_path), 0, 1.0f);
        }
        break;
    case ON_CHANGE_RELEASE:
        avi_play_shutdown();
        if (__this->sel_path) {
            free(__this->sel_path);
            __this->sel_path = NULL;
        }
        break;
    default:
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(CAM_SHOW_LAYOUT)
/* .onchange = cam_show_layout_onchange, */
.onchange = cam_show_layout_onchange1,
 .onkey = NULL,
  .ontouch =  NULL,
};
#endif// TCFG_UI_DRAW_DEMO
#endif// CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE
