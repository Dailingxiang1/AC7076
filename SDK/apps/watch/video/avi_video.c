#include "app_config.h"
#include "cpu.h"
#include "fs/fs.h"
#include "app_config.h"
#include "board_config.h"
#include "generic/circular_buf.h"
#include "app_config.h"
#include "asm/audio_adc.h"
#include "generic/circular_buf.h"
#include "media/audio_decoder.h"
#include "media/mixer.h"
#include "generic/log.h"
#include "app_config.h"
#include "system/timer.h"
#include "device/device.h"
#include "key_event_deal.h"
#include "fs.h"
#include "res/resfile.h"
#include "jlui/ui.h"
#include "jlui_app/ui_style.h"
#include "jlui_app/ui_api.h"
#include "jlui_app/res_config.h"
#include "jlui_app/ui_resource.h"
#include "jlui_app/ui_sys_param.h"

#include "timer.h"            // 定时器
#include "ui/lcd/lcd_drive.h" // lcd 驱动

#include "res_config.h" // res 文件管理
#include <math.h>
#include "ascii.h"
#include "res/font_ascii.h"
#include "res/mem_var.h"
#include "res/zz.h"
#include "res/jpeg_decoder.h"
#include "dev_manager.h"
#include "fs/fs.h"
#include "watch_syscfg_manage.h"

#include "ui_core.h"
#include "control.h"

#include "dbi.h"          // 推屏模块
#include "jlgpu_math.h"   // matrix
#include "jlgpu_driver.h" // gpu 驱动

#include "gpu_port.h" // GPU 接口，依赖于 jlgpu_driver.h
#include "gpu_task.h"
#include "ui_resource.h" // ui资源管理

#include "jlui_effect/jlui_effect.h"  //特效相关
#include "ui_expand/buffer_manager.h" // buffer管理
#include "ui_expand/ui_expand.h"      // 部分宏定义

#include "font/font_all.h"
#include "font/font_textout.h"

#include "ui_draw/ui_circle.h"
#include "ui_animation.h"
#include "font/language_list.h"

#include "ui_measure.h"
#include "jljpeg_decode.h"
#include "gpu_draw.h"
#include "ui_page_switch.h"
#include "ui_draw/ui_basic.h"
#include "ui_text.h"
#include "jlui_app/ui_style.h"
#include "res/resfile.h"

#include "media/includes.h"
#include "audio_config.h"
#include "gpio.h"
#include "app_task.h"
#include "bt.h"

#define LOG_TAG_CONST AVI_VIDEO
#define LOG_TAG "[AVI_VIDEO]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"

#include "avi_video.h"
#include "avi_audio_player.h"
#if TCFG_VIDEO_DIAL_ENABLE
#define AVI_TASK_NAME "avi_task"
#define NEW_PARSE_RIFF_FILE 1 // 该宏打开使用新的文件解析流程，与旧流程主要区别在于兼容packres工具打包后的avi资源解析
#define DEFAULT_DWSUGGESTEDBUFFER_SIZE 20*1024
#define PCM_CBUF_SIZE (20*1024) /* 根据负载可调 320B*3帧约等于1KB */
struct avi_fs_info {
    FILE *fp;
    u32 offset;
    u32 len;
    int width;
    int height;
};

cbuffer_t avi_pcm_cbuf;
static u8 avi_play_mode  = AVI_PLAY_LOOP;
static MV_DRAW *avi_player = NULL;
static PLAY_VIDEO_PATH_MANAGE *video_path_mange = NULL;
#define __this			(avi_player)
/* static void *gpu_task_head; */
static void *avi_taskp;
OS_SEM avi_msg_sem;
static const char *avi_files[] = {
    "storage/UI_FAT/C/Btest.avi",
    // 可以继续添加更多文件
};
static u16 task_switch_flag;
u16 app_video_task_switch_flag_get()
{
    return  task_switch_flag;
}
u16 app_video_task_switch_flag_set(u16 flag)
{
    task_switch_flag = flag;
    return task_switch_flag;
}
/* static u8 __this->param.is_audio_mute = 1; */

#define AVI_FILE_COUNT (sizeof(avi_files) / sizeof(avi_files[0]))

static int parse_list_chunk(FILE *file, long list_end, ChunkCallback callback,
                            void *user_param, long file_size, const char **parent_hierarchy,
                            int parent_level);
typedef void (*DisplayMJpegFunc)(void *player, FILE *file, int data, int chunk_size, int width, int height);
typedef void (*PlayPCMFunc)(void *player, FILE *file, int data, int chunk_size);


/* --------------- */
/*  extern define  */
/* --------------- */
extern struct ui_load_info ui_load_info_table[];
extern UI_RESFILE *res_file;
extern UI_RESFILE *str_file;
extern void *jlui_malloc(int size, u32 ram_type, u32 module_type);
extern void *jlui_zalloc(int size);
extern void jlui_free(void *buf, u32 ram_type, u32 module_type);
extern void jlui_malloc_ram_info_clr(u8 clr);
extern void ui_module_ram_dump();
extern u32 psram_get_used_block(void);
extern void avi_resume(void *avip);
extern int avi_exec(void *avip, DisplayMJpegFunc jpgcb, PlayPCMFunc pcmcb);
extern void avi_close(void *avip);
extern void *avi_open(const char *path, int en_buf, int redrawid);
extern u8 *avi_get_mjpegbuf(void *avip);
extern int avi_get_mjpegbuflen(void *avip);
extern int jpeg_module_opj_init(void *priv);
extern char *watch_avi_get_related_path(u8 cur_watch);
extern int watch_get_style();
extern uint32_t lv_tick_get(void);
extern void audio_app_volume_set(u8 state, s16 volume, u8 fade);
extern const u8 norflash_ext_sfc_en; // 给gpu判断互斥
extern void norflash_espi_mutex_enter();
extern void norflash_espi_mutex_exit();
extern u32 ps_ram_size[];
extern int avi_get_width(void *avip);
extern int avi_get_height(void *avip);
extern int avi_fluh(void *priv);
extern void malloc_list_update_rets(void *buf, int rets);


void avi_stop(void *avip);
void avi_pause(void *avip);
void animig_close(void *ap);
/* void *animig_open(char *name, AVI_PARAM param, int arg); */







static void *avi_zalloc(size_t size)
{
    u32 rets;
    __asm__ volatile("%0 = rets":"=r"(rets));


    void *p = NULL;
#if TCFG_PSRAM_DEV_ENABLE
    p = malloc_psram(size);
    memset(p, 0, size);
#else
    p = zalloc(size);
#endif
    malloc_list_update_rets(p, rets);

    return p;
}
static void *avi_malloc(size_t size)
{
    u32 rets;
    __asm__ volatile("%0 = rets":"=r"(rets));


    void *p = NULL;
#if TCFG_PSRAM_DEV_ENABLE
    p  = malloc_psram(size);
#else
    p  = malloc(size);
#endif
    malloc_list_update_rets(p, rets);
    return p;
}
static void avi_free(void *pv)
{
#if TCFG_PSRAM_DEV_ENABLE
    free_psram(pv);
#else
    free(pv);
#endif
}

// 添加AI voice播放头文件引用
//#include "ai_rx_player.h"
// PCM cbuf资源（对齐ai链路命名）
static void pcm_cbuf_init(void)
{
    if (!__this->avi_pcm_buf) {
        __this->avi_pcm_buf = avi_malloc(PCM_CBUF_SIZE);
    }
    ASSERT(__this->avi_pcm_buf, "pcm cbuf avi_malloc fail\n");
    cbuf_init(&avi_pcm_cbuf, __this->avi_pcm_buf, PCM_CBUF_SIZE);
    __this->avi_audio_start = 1;
    log_info("\n[pcm_cbuf_init]mem_status:\n");
    mem_stats();
}

static void pcm_cbuf_deinit(void)
{
    __this->avi_audio_start = 0;
    if (__this->avi_pcm_buf) {
        cbuf_clear(&avi_pcm_cbuf);
        avi_free(__this->avi_pcm_buf);
        __this->avi_pcm_buf = NULL;
    }
    log_info("\n[pcm_cbuf_deinit]mem_status:\n");
    mem_stats();
}
__attribute__((weak))
u8 get_avi_audio_status()
{
    if (__this == NULL) {
        return 0;
    }
    return __this->avi_audio_start;
}
static void avi_switch_timeunlock(void *p)
{
    if (__this == NULL) {
        return ;
    }
    __this->avi_is_switching = 0;
    wdt_clear();
}
static void avi_switch_timelock(void)
{
    __this->avi_is_switching = 1;
    u16 lock_timer = 0;
    lock_timer = sys_timeout_add(NULL, avi_switch_timeunlock, 200);
}
// 设置文件路径
void set_avi_index(int index)
{
    __this->avi_index = index;
}
const char *set_avi_play_file(void)
{
    if (__this == NULL || __this->avi_index > AVI_FILE_COUNT - 1 || __this->avi_index < 0) {
        log_error("\n\navi index overflow or __this is null,show default avi\n\n");
        return avi_files[0];
    }
    return avi_files[__this->avi_index];
}
void avi_set_avi_playtimer_id(u16 timer_id)
{
    if (__this == NULL) {
        log_error("%s __this is NULL", __func__);
        return ;
    }
    __this->avi_playtimer = timer_id;
}
u16 avi_get_avi_playtimer_id()
{
    if (__this == NULL) {
        log_error("%s __this is NULL", __func__);
        return 0;
    }

    return __this->avi_playtimer;
}
// 新的avi停止接口，确保avi完全停止以及释放资源
void avi_play_shutdown(void)
{
    u32 rets;
    __asm__ volatile("%0 = rets":"=r"(rets));
    log_info("\n\n avi_play_shutdown rets=%x\n\n", rets);
#if TCFG_APP_VIDEO_EN
    if (app_get_current_mode_name() == APP_MODE_VIDEO) {
        app_mode_stack_clr(APP_MODE_VIDEO);
        app_task_switch_back();
    }
#endif

    if (__this == NULL) {
        return ;
    }
    if (__this->avi_playtimer) {
        sys_timeout_del(__this->avi_playtimer);
        __this->avi_playtimer = 0;
    }
    jlgpu_scheduler_wait_sync();
    if (avi_player) {
        animig_close(avi_player);
        avi_player = NULL;
    }
    log_info("\n\n[avi_play_shutdown]mem_status:\n");
    mem_stats();
}
void video_manage_init()
{
    video_path_mange = avi_zalloc(sizeof(PLAY_VIDEO_PATH_MANAGE));
}
void set_avi_play_mode(u8 mode)
{
    avi_play_mode = mode;
    avi_play_shutdown();
}
// 播放下一个文件
void avi_play_next(void)
{
    AVI_PARAM param;
    if (__this != NULL) {

        memcpy(&param, &(__this->param), sizeof(AVI_PARAM));//拷贝当前配置
    }
    u8 is_dial = __this->param.is_dial;
    u8 *video_path = NULL;
    if (is_dial == 0) {
        video_path = (u8 *)param.fname;

    } else {
        video_path = (u8 *)watch_avi_get_related_path(watch_get_style());
    }
    if (video_path == NULL) {
        log_error("%s path is NULL");
        return ;
    }
    if (!__this->avi_is_switching) {
        avi_switch_timelock();
        wdt_clear();
        avi_play_shutdown();
        avi_player = animig_open((char *)video_path, param, 3); // 缓存3帧可以达到流畅播放30FPS
        /* avi_player = (MV_DRAW *) animig_open((char *)set_avi_play_file(), 0, 3); // 缓存3帧可以达到流畅播放30FPS */
        __this->avi_playtimer = 0;
        // UI_HIDE_CURR_WINDOW();
        // UI_SHOW_WINDOW(AVI_WINDOW);
        // UI_SHOW_WINDOW(STYLE_DIAL_ID(WATCH));
    }
}
void avi_play_loop(void)
{
    AVI_PARAM param;
    if (__this != NULL) {

        memcpy(&param, &(__this->param), sizeof(AVI_PARAM));//拷贝当前配置
    }
    u8 is_dial = __this->param.is_dial;
    u8 *video_path = NULL;

    if (is_dial == 0) {
        video_path = (u8 *)param.fname;

    } else {
        video_path = (u8 *)watch_avi_get_related_path(watch_get_style());
    }
    if (video_path == NULL) {
        log_error("%s path is NULL");
        return ;
    }
    if (!__this->avi_is_switching) {
        avi_switch_timelock();
        wdt_clear();
        avi_play_shutdown();
        avi_player = animig_open((char *)video_path, param, 3); // 缓存3帧可以达到流畅播放30FPS
        /* avi_player = (MV_DRAW *) animig_open((char *)set_avi_play_file(), 0, 3); // 缓存3帧可以达到流畅播放30FPS */
        __this->avi_playtimer = 0;
        // UI_HIDE_CURR_WINDOW();
        // UI_SHOW_WINDOW(AVI_WINDOW);
        // UI_SHOW_WINDOW(STYLE_DIAL_ID(WATCH));
    }
}
// 播放上一个文件
void avi_play_prev(void)
{
    AVI_PARAM param;
    if (__this != NULL) {

        memcpy(&param, &(__this->param), sizeof(AVI_PARAM));//拷贝当前配置
    }
    u8 is_dial = __this->param.is_dial;
    u8 *video_path = NULL;
    if (is_dial == 0) {

        video_path = (u8 *)param.fname;
    } else {
        video_path = (u8 *)watch_avi_get_related_path(watch_get_style());
    }
    if (video_path == NULL) {
        log_error("%s path is NULL");
        return ;
    }
    if (!__this->avi_is_switching) {
        avi_switch_timelock();
        wdt_clear();
        avi_play_shutdown();
        avi_player = animig_open((char *)video_path, param, 3); // 缓存3帧可以达到流畅播放30FPS
        /* avi_player = (MV_DRAW *) animig_open((char *)set_avi_play_file(), 0, 3); // 缓存3帧可以达到流畅播放30FPS */
        __this->avi_playtimer = 0;
        // UI_HIDE_CURR_WINDOW();
        // UI_SHOW_WINDOW(AVI_WINDOW);
        // UI_SHOW_WINDOW(STYLE_DIAL_ID(WATCH));
    }

}



static size_t avi_fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{

    int len = fread(ptr, size, nmemb, stream);
    // log_info("pos %d len %d retlen %d", ftell(stream), size*nmemb, len);
    len = len / size;
    return len;
}
#define rewind(fp) fseek(fp, 0, SEEK_SET)


char  parse_riff_file(FILE *file, ChunkCallback callback, void *user_param, int curr_pos)
{
    char *head = "";

    /* 获取文件总大小 */
    // fseek(file, 0, SEEK_END);

    log_info("\n\n\nparse_riff_file: initial file position: %u\n", ftell(file));
    fseek(file, 0, SEEK_SET);
    __this->file_offset = ftell(file);
    log_info("\n\n\nparse_riff_file: after seek to 0, position: %u\n", ftell(file));

    long file_size = flen(file); // ftell(file);
    // flen
    fseek(file, 0, SEEK_SET);

    log_info("flen %ld", file_size);

    fseek(file, curr_pos, SEEK_SET);

    char chunk_id[5] = {0};
    u32 chunk_size;
    char format[5] = {0};

    /* 读取RIFF头 */
    if (avi_fread(chunk_id, 1, 4, file) != 4) {
        log_error("Failed to read chunk ID");
        return -AVI_READ_CHUNK_ID_FAIL;
    }
    chunk_id[4] = '\0';

    if (avi_fread(&chunk_size, 4, 1, file) != 1) {
        log_error("Failed to read chunk size");
        return -AVI_READ_CHUNK_SIZE_FAIL;
    }
    log_info("\n\n\n\n\nRIFF chunk_size: %d\n\n\n\n\n", chunk_size);
    /* 校验块大小合法性 */
    long current_pos = ftell(file) - __this->file_offset;
    /* printf("%s 111 chunk:%s current_pos: %d, chunk_size: %d, file_size: %d", __func__, chunk_id, (int)current_pos, chunk_size, (int)file_size); */
    log_info("\n\n\n\n[%s]current_pos: %d, chunk_size: %d, file_size: %d\n\n\n\n", __func__, (int)current_pos, chunk_size, (int)file_size);
    if (current_pos + chunk_size > file_size) {
        log_info("校验块大小合法性 %d %d %d", (int)current_pos, chunk_size, (int)file_size);
        log_error("RIFF chunk size exceeds file size");
        return -AVI_FILE_LEN_ERR;
    }

    /* 处理子块 */
    long riff_data_end = ftell(file) + chunk_size - __this->file_offset;
    log_info("riff_data_end %d %d", (int)riff_data_end, chunk_size);

    fseek(file, curr_pos, SEEK_SET);
    while ((ftell(file) - __this->file_offset) < riff_data_end) {
        wdt_clear();
        char sub_chunk_id[5] = {0};
        u32 sub_chunk_size;

        /* 读取子块头 */
        if (avi_fread(sub_chunk_id, 1, 4, file) != 4) {
            log_error("Failed to read sub chunk ID");
            return -AVI_READ_SUB_CHUNK_ID_FAIL;
        }
        sub_chunk_id[4] = '\0';

        if (avi_fread(&sub_chunk_size, 4, 1, file) != 1) {
            log_error("Failed to read sub chunk size");
            return -AVI_READ_SUB_CHUNK_ID_FAIL;
        }
        /* 校验子块大小 */
        current_pos = ftell(file) - __this->file_offset;
        /* printf("%s 222 chunk:%s current_pos: %d, chunk_size: %d, file_size: %d", __func__, sub_chunk_id, (int)current_pos, sub_chunk_size, (int)file_size); */
        if (current_pos + sub_chunk_size > file_size) {
            log_info("校验子块大小 %d %d %d", (int)current_pos, sub_chunk_size, (int)file_size);
            log_error("Sub chunk exceeds file size");
            return -AVI_SUB_CHUNK_LEN_ERR;
        }

        off_t sub_start = ftell(file) - 8 - __this->file_offset; // 计算子块起始位置

        /* 处理LIST块 */
        if (strcmp(sub_chunk_id, "LIST") == 0 || strcmp(sub_chunk_id, "RIFF") == 0) {
            off_t list_start = sub_start; // LIST块起始位置
            char list_type[5] = {0};
            if (avi_fread(list_type, 1, 4, file) != 4) {
                log_error("Failed to read LIST type");
                return -AVI_LIST_TYPE_ERR;
            }
            list_type[4] = '\0';

            int jump = 0;
            /* 构建LIST块层级信息 */
            const char *list_hierarchy[] = {head, format, sub_chunk_id, list_type};
            /* 调用回调时传递list_type */
            if (callback)
                callback(file, list_start, sub_chunk_id, sub_chunk_size,
                         list_type, list_hierarchy, 4, &jump, user_param);

            if (jump) {
                // fseek(file, sub_chunk_size - 4, SEEK_CUR);
                fseek(file, current_pos + sub_chunk_size, SEEK_SET);
            } else {
                long list_end = ftell(file) - __this->file_offset + sub_chunk_size - 4;
                int ret = parse_list_chunk(file, list_end, callback, user_param, file_size,
                                           list_hierarchy, 4);
                return ret;
                fseek(file, current_pos + sub_chunk_size, SEEK_SET); // add
            }
        } else {

            /* 构建子块层级信息 */
            const char *sub_hierarchy[] = {head, format, sub_chunk_id};
            /* 处理子块时调用回调 */

            if (callback)
                callback(file, sub_start, sub_chunk_id, sub_chunk_size,
                         NULL, sub_hierarchy, 3, NULL, user_param);

            fseek(file, current_pos + sub_chunk_size, SEEK_SET);
        }

        /* 处理对齐填充 */
        if (sub_chunk_size % 2 != 0) {
            fseek(file, 1, SEEK_CUR);
        }
    }
    return 0;
}
/* LIST块解析实现 */
static int parse_list_chunk(FILE *file, long list_end, ChunkCallback callback,
                            void *user_param, long file_size, const char **parent_hierarchy,
                            int parent_level)
{
    while (ftell(file) - __this->file_offset < list_end) {
        wdt_clear();
        off_t chunk_start = ftell(file) - __this->file_offset; // 记录块起始位置
        char sub_chunk_id[5] = {0};
        u32 sub_chunk_size;

        /* 读取子块头 */
        if (avi_fread(sub_chunk_id, 1, 4, file) != 4) {
            log_error("Failed to read sub chunk ID");
            return -AVI_READ_SUB_CHUNK_ID_FAIL;
        }
        sub_chunk_id[4] = '\0';

        if (avi_fread(&sub_chunk_size, 4, 1, file) != 1) {
            log_error("Failed to read sub chunk size");
            return -AVI_READ_SUB_CHUNK_SIZE_FAIL;
        }
        /* 校验子块大小 */
        long current_pos = ftell(file) - __this->file_offset;

        /* printf("%s 333 chunk:%s current_pos: %d, chunk_size: %d, file_size: %d", __func__, sub_chunk_id, (int)current_pos, sub_chunk_size, (int)file_size); */
        if (current_pos + sub_chunk_size > file_size) {
            log_error("Sub chunk exceeds file size");
            return -AVI_SUB_CHUNK_LEN_ERR;
        }

        /* 构建层级信息 */
        const char **current_hierarchy = avi_malloc((parent_level + 1) * sizeof(const char *));
        memcpy(current_hierarchy, parent_hierarchy, parent_level * sizeof(const char *));
        current_hierarchy[parent_level] = sub_chunk_id;

        /* 处理嵌套LIST块 */
        if (strcmp(sub_chunk_id, "LIST") == 0) {

            char list_type[5] = {0};
            if (avi_fread(list_type, 1, 4, file) != 4) {
                log_error("Failed to read nested LIST type");
                avi_free(current_hierarchy);
                return -AVI_LIST_TYPE_ERR;
            }
            list_type[4] = '\0';

            int jump = 0;
            off_t nested_start = chunk_start;
            // log_info("nested_start %d", nested_start);
            /* 构建嵌套层级信息 */
            const char **nested_hierarchy = avi_malloc((parent_level + 2) * sizeof(const char *));
            memcpy(nested_hierarchy, current_hierarchy, (parent_level + 1) * sizeof(const char *));
            nested_hierarchy[parent_level + 1] = list_type;

            callback(file, nested_start, sub_chunk_id, sub_chunk_size,
                     list_type, nested_hierarchy, parent_level + 2, &jump, user_param);

            if (jump) {
                // fseek(file, sub_chunk_size - 4, SEEK_CUR);
                fseek(file, current_pos + sub_chunk_size, SEEK_SET);
            } else {
                long nested_end = ftell(file) - __this->file_offset + sub_chunk_size - 4;
                parse_list_chunk(file, nested_end, callback, user_param, file_size,
                                 nested_hierarchy, parent_level + 2);

                fseek(file, current_pos + sub_chunk_size, SEEK_SET);
            }

            avi_free(nested_hierarchy);
        } else {

            /* 调用回调 */
            // log_info("chunk_start %d", chunk_start);
            if (callback)
                callback(file, chunk_start, sub_chunk_id, sub_chunk_size,
                         NULL, current_hierarchy, parent_level + 1, NULL, user_param);

            // fseek(file, sub_chunk_size, SEEK_CUR);
            fseek(file, current_pos + sub_chunk_size, SEEK_SET);
        }

        /* 处理对齐填充 */
        if (sub_chunk_size % 2 != 0) {
            fseek(file, 1, SEEK_CUR);
        }

        avi_free(current_hierarchy);
    }
    return 0;
}

// 打印流头信息（增强版）
void print_stream_header(const StreamHeader *h)
{
    const char *stream_type = "Unknown";
    if (strncmp(h->fccType, "vids", 4) == 0) {
        stream_type = "Video Stream";
    } else if (strncmp(h->fccType, "auds", 4) == 0) {
        stream_type = "Audio Stream";
    }

    // 常见FourCC编解码器识别
    const char *codec_name = "Unknown";
    char fourcc[5] = {0};
    memcpy(fourcc, h->fccHandler, 4);

    if (strcmp(fourcc, "MJPG") == 0) {
        codec_name = "Motion-JPEG";
    } else if (strcmp(fourcc, "MP43") == 0) {
        codec_name = "MPEG-4 V3";
    } else if (strcmp(fourcc, "DIVX") == 0) {
        codec_name = "DivX";
    } else if (strcmp(fourcc, "H264") == 0) {
        codec_name = "H.264";
    } else if (strcmp(fourcc, "AAC") == 0) {
        codec_name = "AAC Audio";
    } else if (strcmp(fourcc, "mp3") == 0) {
        codec_name = "MPEG Layer-3";
    }
    wdt_clear();
    log_info("[strh] Stream Header @ 0x%08X\n", (uint32_t)h);
    log_info("  Stream Type:    %s (%s)\n", stream_type, fourcc);
    log_info("  Codec:          %s\n", codec_name);
    log_info("  Time Base:      %u/%u (%.2f fps)\n",
             h->dwScale, h->dwRate,
             (h->dwScale > 0) ? (float)h->dwRate / h->dwScale : 0);
    log_info("  Stream Length:  %u samples\n", h->dwLength);
    log_info("  Sample Size:    %u bytes\n", h->dwSampleSize);
    log_info("  Buffer Size:    %u bytes\n", h->dwSuggestedBufferSize);
    log_info("  Quality:        %u/10000\n", h->dwQuality);
    log_info("  Flags:          %s%s%s\n",
             (h->dwFlags & 0x0001) ? "AVISF_DISABLED " : "",
             (h->dwFlags & 0x0002) ? "AVISF_VIDEO_PALCHANGES " : "",
             (h->dwFlags & 0x0004) ? "AVISF_KNOWN_FLAGS " : "");
    log_info("--------------------------------\n");
    wdt_clear();
}

// 打印视频格式（增强版）
void print_video_format(const VideoFormat *f)
{
    const char *compression = "Unknown";
    char fourcc[5] = {0};
    memcpy(fourcc, &f->biCompression, 4);

    if (strcmp(fourcc, "MJPG") == 0) {
        compression = "Motion-JPEG";
    } else if (strcmp(fourcc, "DIB ") == 0) {
        compression = "Uncompressed RGB";
    } else if (strcmp(fourcc, "MP4V") == 0) {
        compression = "MPEG-4 Video";
    } else if (strcmp(fourcc, "H264") == 0) {
        compression = "H.264/AVC";
    }

    log_info("[strf] Video Format @ 0x%08X\n", (uint32_t)f);
    log_info("  Dimensions:     %ux%u pixels\n", f->biWidth, f->biHeight);
    log_info("  Compression:    %s (%s)\n", fourcc, compression);
    log_info("  Color Planes:   %u\n", f->biPlanes);
    log_info("  Bit Depth:      %u bits/pixel\n", f->biBitCount);
    log_info("  Frame Size:     %u bytes\n", f->biSizeImage);
    log_info("  Aspect Ratio:   %.2f:1\n",
             (f->biWidth > 0 && f->biHeight > 0) ? (float)f->biWidth / (float)f->biHeight : 0);
    log_info("  Data Rate:      %.2f MB/s\n",
             (f->biSizeImage > 0) ? (float)f->biSizeImage * 30.0f / (1024 * 1024) : 0); // 假设30fps
    log_info("--------------------------------\n");
}

// 打印音频格式（增强版）
void print_audio_format(const AudioFormat *f)
{
    const char *format = "Unknown";
    switch (f->wFormatTag) {
    case 0x0001:
        format = "PCM";
        break;
    case 0x0055:
        format = "MP3";
        break;
    case 0x0050:
        format = "MPEG2";
        break;
    case 0x000F:
        format = "IMA ADPCM";
        break;
    case 0x0161:
        format = "WMAv2";
        break;
    case 0x0160:
        format = "WMAv1";
        break;
    case 0x0162:
        format = "WMA Pro";
        break;
    }

    log_info("[strf] Audio Format @ 0x%08X\n", (uint32_t)f);
    log_info("  Format:         %s (0x%04X)\n", format, f->wFormatTag);
    log_info("  Channels:       %u (%s)\n",
             f->nChannels,
             f->nChannels == 1 ? "Mono" : f->nChannels == 2 ? "Stereo"
             : "Multi-channel");
    log_info("  Sample Rate:    %u Hz\n", f->nSamplesPerSec);
    log_info("  Bit Depth:      %u bits\n", f->wBitsPerSample);
    log_info("  Bitrate:        %u kbps\n",
             (f->nAvgBytesPerSec * 8) / 1000);
    log_info("  Block Align:    %u bytes\n", f->nBlockAlign);
    log_info("  Audio Duration: %.2f sec (estimated)\n",
             (f->nSamplesPerSec > 0) ? (float)f->nSamplesPerSec / f->nSamplesPerSec : 0); // 需要实际帧数计算
    log_info("--------------------------------\n");
}

static void parse_callback(FILE *file, off_t offset, const char *chunk_id,
                           uint32_t size, const char *list_name,
                           const char **hierarchy, int level, int *jump,
                           void *user_param)
{
    AVIPlayer *p = (AVIPlayer *)user_param;

    // 解析AVI主头
    if (!strcmp(chunk_id, "avih") /* && level == 4 &&
         !strcmp(hierarchy[3], "hdrl")*/
       ) {
        fseek(file, offset + 8, SEEK_SET);
        AVIH_Header avih;
        avi_fread(&avih, sizeof(avih), 1, file);
        p->total_frames = avih.dwTotalFrames;
        p->frame_rate = 1000000 / avih.dwMicroSecPerFrame;
        p->total_duration = (uint64_t)avih.dwTotalFrames * (uint64_t)avih.dwMicroSecPerFrame;
        p->width = (int)avih.dwWidth;
        p->height = (int)avih.dwHeight;

        log_info("%d %d", p->width, p->height);
        log_info("%u %u", avih.dwWidth, avih.dwHeight);
        wdt_clear();
        log_info("info %u %d %ld", p->total_frames, p->frame_rate, (long)p->total_duration);
        log_info("AVI 头信息:\n");
        log_info("每帧时长(微秒): %u\n", avih.dwMicroSecPerFrame);
        log_info("最大字节/秒: %u\n", avih.dwMaxBytesPerSec);
        log_info("数据填充粒度: %u\n", avih.dwPaddingGranularity);
        log_info("全局标志: %08X\n", avih.dwFlags); // 十六进制显示更直观
        log_info("总帧数: %u\n", avih.dwTotalFrames);
        log_info("初始帧数: %u\n", avih.dwInitialFrames);
        log_info("流数量: %u\n", avih.dwStreams);
        log_info("建议缓冲区大小: %u\n", avih.dwSuggestedBufferSize);
        log_info("视频宽度: %u 像素\n", avih.dwWidth);
        log_info("视频高度: %u 像素\n", avih.dwHeight);
        log_info("保留字段: [%u][%u][%u][%u]\n", // 显示保留字段内容
                 avih.dwReserved[0],
                 avih.dwReserved[1],
                 avih.dwReserved[2],
                 avih.dwReserved[3]);
        wdt_clear();
        p->avih_header = avih;
    }

    // 流头
    else if (!strcmp(chunk_id, "strh")) {
        StreamHeader stream_header = {0};
        fseek(file, offset + 8, SEEK_SET);
        avi_fread(&stream_header, 1, sizeof(stream_header), file);
        print_stream_header(&stream_header);

        if (!strncmp(stream_header.fccType, "vids", 4)) {
            p->video_stream_header = stream_header;
            p->stream_type = STREAM_VIDEO;
        } else if (!strncmp(stream_header.fccType, "auds", 4)) {
            p->audio_stream_header = stream_header;
            p->stream_type = STREAM_AUDIO;
        } else {
            p->stream_type = STREAM_NULL;
        }
    }

    //  流格式
    else if (!strcmp(chunk_id, "strf")) {
        fseek(file, offset + 8, SEEK_SET);
        if (p->stream_type == STREAM_VIDEO) {
            VideoFormat vf = {0};
            avi_fread(&vf, 1, sizeof(vf), file);
            print_video_format(&vf);
            p->video_format = vf;
        } else if (p->stream_type == STREAM_AUDIO) {
            AudioFormat af = {0};
            avi_fread(&af, 1, sizeof(af), file);
            print_audio_format(&af);
            p->audio_format = af;
        }
    }

    // 记录movi列表位置
    else if (!strcmp(chunk_id, "LIST") && list_name &&
             !strcmp(list_name, "movi")) {
        p->movi_data_start = offset + 8; // LIST头(12字节)后开始
        p->movi_data_len = size;
        log_info("movi %d", (int)p->movi_data_start);

        *jump = 1; // 跳过movi节省遍历时间
    }

    //  计算总帧数

    // 记录idx1块信息
    else if (!strcmp(chunk_id, "idx1")) {
        p->idx1_offset = offset + 8; // 跳过chunk_id和size
        p->idx1_size = size;
        log_info("idx1 %d %d", (int)p->idx1_offset, (int)p->idx1_size);
    }
}

void *avi_open(const char *path, int en_buf, int redrawid)
{

    // myavi_fread_cleanup();

    log_info("%s %d %s", __func__, __LINE__, path);
    FILE *fp = fopen(path, "r");
    AVIPlayer *player = avi_zalloc(sizeof(AVIPlayer));
    wdt_clear();
    log_info("%s %d fp %p", __func__, __LINE__, fp);
    mem_stats();
    if (!fp) {
        log_error("%s open file fail ");
        goto __err;
    }
    log_info("%s %d", __func__, __LINE__);
    mem_stats();
    player->file = fp;
    player->redrawid = redrawid;
    log_info("%s %d", __func__, __LINE__);
    mem_stats();
    // 解析文件结构
    log_info("\n\n\nBefore parse_riff_file: file position: %u\n", ftell(fp));

    char parse_ret = parse_riff_file(fp, parse_callback, player, 0);
    if (parse_ret != 0) {
        log_error("%s check file fail =%d", __func__, parse_ret);
        goto __err;
    }
    log_info("%s %d", __func__, __LINE__);
    mem_stats();
    rewind(fp);

    if (player->avih_header.dwTotalFrames == 0) {

        log_error("%s frames info no get ");
        goto __err;
    }

    void *usr_mmu = strstr(path, "virfat_flash");

    if (usr_mmu) {
        player->view_file_info = avi_zalloc(sizeof(struct flash_file_info));
        ASSERT(player->view_file_info);
        ui_res_flash_info_get(player->view_file_info, (char *)path, "res", true);

        player->cache.cnt = en_buf;
        player->cache.block_len = 0;

        player->cache.m = avi_zalloc(player->cache.cnt * sizeof(mjpegdata));

        player->cache.m[0].time = lv_tick_get();
        player->cache.act_idx = player->cache.pre_idx = 0;
    } else if (en_buf) {
        if (player->video_stream_header.dwSuggestedBufferSize == 0) {
            player->video_stream_header.dwSuggestedBufferSize =	DEFAULT_DWSUGGESTEDBUFFER_SIZE;
        }
        int len = (player->video_stream_header.dwSuggestedBufferSize + 3) & ~3;
        log_info("%d", len);
        player->cache.cnt = en_buf;
        player->cache.block_len = len;

        player->cache.data = avi_zalloc(player->cache.cnt * player->cache.block_len);
        ASSERT(player->cache.data);
        player->cache.m = avi_zalloc(player->cache.cnt * sizeof(mjpegdata));

        for (int i = 0; i < player->cache.cnt; i++) {
            player->cache.m[i].data = player->cache.data + i * player->cache.block_len;
        }

        extern uint32_t lv_tick_get(void);
        player->cache.m[0].time = lv_tick_get();
        player->cache.act_idx = player->cache.pre_idx = 0;
    } else {
        ASSERT(0, "非norflash资源需要使能缓存");
    }

    log_info("%s %d", __func__, __LINE__);
    mem_stats();

    // os_time_dly(100);

    AVIPlayer *p = player;
    // // p->audio_format.wFormatTag  = 0;
    // if(p->audio_format.wFormatTag == 0x0001) {
    //     log_info("%s %d", __func__, __LINE__ );
    //     mem_stats();
    //     app_audio_state_switch(APP_AUDIO_STATE_MUSIC, app_audio_volume_max_query(AppVol_MUSIC), NULL);	// 音量状态设置
    //     audio_dac_set_volume(&dac_hdl, app_audio_volume_max_query(AppVol_MUSIC)/100);					// dac 音量设置
    //     audio_dac_set_sample_rate(&dac_hdl, p->audio_format.nSamplesPerSec * p->audio_format.nChannels);							// 采样率设置
    //     log_info(">>> rate %d",p->audio_format.nSamplesPerSec * p->audio_format.nChannels);
    //     audio_dac_start(&dac_hdl);											// dac 启动
    //     audio_dac_channel_start(NULL);
    //     JL_APA->APA_VL0 = 0x080F080F;//0x080F080F;//0x40004000;
    //     audio_dac_set_volume(&dac_hdl, 1);
    // }

    os_mutex_create(&p->mutex);

    return player;
__err:
    printf("\n [ERROR] %s -[yuyu] %d\n", __FUNCTION__, __LINE__);
    void avi_close(void *avip);
    avi_close(player);

    return NULL;
}

void avi_close(void *avip)
{

    // myavi_fread_cleanup();
    AVIPlayer *p = avip;
    if (p) {
        avi_stop(p);
        // avi_fs.offset = 0;
        if (p->timerid) {
            sys_timer_del(p->timerid);
            p->timerid = 0;
        }

        if (p->cache.m) {
            for (int i = 0; i < p->cache.cnt; i++) {
                if (p->cache.m[i].opj) {
                    jljpeg_decode_release(p->cache.m[i].opj, 1);
                    jpeg_free(p->cache.m[i].opj->device);
                    jpeg_free(p->cache.m[i].opj);
                    p->cache.m[i].opj = NULL;
                }
            }
        }

        if (p->cache.m) {
            avi_free(p->cache.m);
            p->cache.m = NULL;
        }
        if (p->cache.data) {
            free_psram(p->cache.data);
            p->cache.data = NULL;
        }

        // if(p->mjpegbuf){
        //     avi_free(p->mjpegbuf);
        //     p->mjpegbuf = NULL;
        // }
        fclose(p->file);

        if (p->view_file_info) {
            ui_res_flash_info_free(p->view_file_info, "res");
            avi_free(p->view_file_info);
            p->view_file_info = NULL;
        }
        if (p) {
            avi_free(p);
            p = NULL;
        }
    }
}

static off_t find_frame_offset(AVIPlayer *p, int frame_num)
{
    // 每个索引条目16字节: ckid(4)+flags(4)+offset(4)+size(4)
    off_t entry_pos = p->idx1_offset + frame_num * 16;

    fseek(p->file, entry_pos, SEEK_SET);

    struct {
        char ckid[4];
        uint32_t flags;
        uint32_t offset;
        uint32_t size;
    } entry;

    avi_fread(&entry, sizeof(entry), 1, p->file);
    return p->movi_data_start + entry.offset;
}

//  定位时间
void avi_seek(void *avip, float sec)
{
    AVIPlayer *p = (AVIPlayer *)avip;
    p->lv_pos = 0;
    int frame_num = sec * p->frame_rate;
    if (frame_num >= p->total_frames) {
        frame_num = p->total_frames - 1;
    }
    p->current_pos = find_frame_offset(p, frame_num);
}

//  定位帧数
void avi_seek_frame(void *avip, int frame_num)
{
    AVIPlayer *p = (AVIPlayer *)avip;
    p->lv_pos = 0;
    if (frame_num >= p->total_frames) {
        frame_num = p->total_frames - 1;
    }
    p->current_pos = find_frame_offset(p, frame_num);
}

u8 get_avi_pause_status()
{
    if (__this == NULL || __this->pause == 1) {
        return true;
    }
    return false;
}
void avi_pause()
{
    /* AVIPlayer *p = (AVIPlayer *)avip; */

    AVIPlayer *p = (AVIPlayer *)__this->st;
    if (__this == NULL || __this->pause == 1 || p == NULL) {
        return;
    }
    p->ui_work = 2;
    avi_player->pause = 1;

    os_taskq_del_type("ui", Q_CALLBACK);

    if (p->audio_format.wFormatTag == 0x0001 && 0) {
        audio_dac_stop(&dac_hdl);
        audio_dac_close(&dac_hdl);
    }

    p->is_playing = 0;
}

static void avi_timer(void *priv);

void avi_pcm_open(void *avip)
{
    AVIPlayer *p = (AVIPlayer *)avip;
    // 初始化循环缓冲区和互斥锁
    if (p->audio_format.wFormatTag == 0) {
        return ;
    }
    pcm_cbuf_init();

    // 创建AI voice播放参数
    struct avi_audio_player_param param = {0};
    param.coding_type = AUDIO_CODING_PCM;
    param.channel_mode = (p->audio_format.nChannels == 1) ? AUDIO_CH_L : AUDIO_CH_MIX;
    param.sample_rate = p->audio_format.nSamplesPerSec;
    param.bit_rate = p->audio_format.nSamplesPerSec * p->audio_format.nChannels * p->audio_format.wBitsPerSample;
    param.type = AVI_SERVICE_VOICE;
    // 帧长度 该参数底层库未使用 故注释
    // param.frame_dms = 8; // p->total_duration / p->total_frames;

    log_info("[AVI_PCM] 音频参数: 采样率=%u, 声道=%d, 位深=%d, 帧长度=%dms\n",
             p->audio_format.nSamplesPerSec,
             p->audio_format.nChannels,
             p->audio_format.wBitsPerSample,
             param.frame_dms);

    // 打开AI voice播放
    int ret = avi_audio_player_open(NULL, 0, &param);
    if (ret != 0) {
        log_info("[AVI_PCM] AI voice播放打开失败: %d\n", ret);
    } else {
        log_info("[AVI_PCM] AI voice播放打开成功\n");
    }
    /* audio_app_volume_set(APP_AUDIO_STATE_MUSIC, 2, 1); */

}

void avi_pcm_close(void *avip)
{
    AVIPlayer *p = (AVIPlayer *)avip;
    if (p->audio_format.wFormatTag == 0) {
        return ;
    }
    log_info("%s %d ", __func__, __LINE__);

    pcm_cbuf_deinit();
    avi_audio_player_close(0);
}

void avi_resume()
{

    /* AVIPlayer *p = (AVIPlayer *)avip; */
    AVIPlayer *p = (AVIPlayer *)__this->st;
    printf("\n [ERROR] %s -[yuyu] %d\n", __FUNCTION__, __LINE__);
    /* log_info("dwMicroSecPerFrame %u", p->avih_header.dwMicroSecPerFrame); */

    if (avi_player == NULL || avi_player->pause == 0) {
        return;
    }
    int argv[3] = {0};
    int retry = 3;
    argv[0] = (int)avi_timer;
    argv[1] = 1;
    argv[2] = (int)p;
    /* int ret = os_taskq_post_type("ui", Q_CALLBACK, 3, argv); */

    avi_player->pause = 0;
    /* avi_pcm_open(p); */


    if (p->audio_format.wFormatTag == 0x0001 && 0) {
        log_info("%s %d", __func__, __LINE__);
        mem_stats();

        app_audio_state_switch(APP_AUDIO_STATE_MUSIC, app_audio_volume_max_query(AppVol_MUSIC), NULL);   // 音量状态设置
        audio_dac_set_volume(&dac_hdl, app_audio_volume_max_query(AppVol_MUSIC) / 100);                  // dac 音量设置
        audio_dac_set_sample_rate(&dac_hdl, p->audio_format.nSamplesPerSec * p->audio_format.nChannels); // 采样率设置
        log_info(">>> rate %u", p->audio_format.nSamplesPerSec * p->audio_format.nChannels);
        audio_dac_start(&dac_hdl); // dac 启动
        audio_dac_channel_start(NULL);
        JL_APA->APA_VL0 = 0x40004000; // 0x080F080F;//0x40004000;
        audio_dac_set_volume(&dac_hdl, 1);
    }

    p->is_playing = 1;
    p->ui_work = 0;
}

void avi_stop(void *avip)
{
    AVIPlayer *p = (AVIPlayer *)avip;

    avi_pause(avip);

    p->is_playing = 0;
    p->current_pos = 0;
}

typedef void (*DisplayMJpegFunc)(void *player, FILE *file, int data, int chunk_size, int width, int height);
typedef void (*PlayPCMFunc)(void *player, FILE *file, int data, int chunk_size);

void avi_resume(void *avip);
int avi_exec(void *avip, DisplayMJpegFunc jpgcb, PlayPCMFunc pcmcb)
{
    AVIPlayer *p = (AVIPlayer *)avip;
    int argv[2] = {0};
    int ret;
    /* log_info("%s %d is_playing %d,", __func__, __LINE__, p->is_playing); */

    if (!p) {
        return -1;
    }

    if (p->current_pos == 0) {
        avi_seek(avip, 0.0);
    }

    u8 is_vid = 0;
    while (is_vid == 0) {
        // 到达movi数据末尾时停止播放
        if (p->current_pos >= p->movi_data_start + p->movi_data_len) {
            switch (avi_play_mode) {
            case AVI_PLAY_LOOP:
                // 默认模式，单视频循环
                /* p->current_pos = 0; */
                /* avi_seek(avip, 0.0); */
                argv[0] = (int)avi_play_loop;
                argv[1] = 0;
                ret = os_taskq_post_type("ui", Q_CALLBACK, 2, argv);

                log_info("\n\n[AVI_PLAY_LOOP]%s %d is_playing %d\n\n", __func__, __LINE__, p->is_playing);
                return -1;
                break;
            case AVI_PLAY_ONCE:
                // 单视频播放一次（如有需要）
                /* argv[0] = (int)avi_play_next; */
                /* argv[1] = 0; */
                /* ret = os_taskq_post_type("ui", Q_CALLBACK, 2, argv); */
                log_info("\n\n[AVI_PLAY_SINGLE]%s %d is_playing %d\n\n", __func__, __LINE__, p->is_playing);
                return -1;
                break;
            case AVI_PLAY_SEQUENCE:
                // 按顺序播放视频
                argv[0] = (int)avi_play_next;
                argv[1] = 0;
                ret = os_taskq_post_type("ui", Q_CALLBACK, 2, argv);
                log_info("\n\n[AVI_PLAY_SEQUENCE]%s %d is_playing %d\n\n", __func__, __LINE__, p->is_playing);
                return -1;
                break;
            default:
                break;
            }
        }

        fseek(p->file, p->current_pos, SEEK_SET);

        char ckid[5] = {0};
        uint32_t chunk_size;

        // 读取当前chunk头信息
        if (avi_fread(ckid, 4, 1, p->file) != 1 ||
            avi_fread(&chunk_size, 4, 1, p->file) != 1) {
            log_error("Failed to read chunk header");
            p->current_pos = 0;
            return -1;
        }

        // 处理视频帧
        if (strncmp(ckid, "00dc", 4) == 0) {
            // log_info("00dc");
            // jpgcb(p, p->file, p->lv_pos, p->lv_size, p->width, p->height);
            jpgcb(p, p->file, p->current_pos + 8, chunk_size, p->width, p->height);
            p->lv_pos = p->current_pos + 8;
            p->lv_size = chunk_size;
            is_vid = 1;
        }
        // 处理音频帧
        else if (strncmp(ckid, "01wb", 4) == 0) {
            // log_info("01wb");
            pcmcb(p, p->file, p->current_pos + 8, chunk_size);
        }
        // 跳过未知chunk类型
        else {
            log_error("Skipping unknown chunk: %s\n", ckid);
            put_buf((u8 *)ckid, 5);
            p->current_pos = 0;
        }

        if (avi_player->pause == 1) {
            break;
        }
        // 更新位置指针（chunk数据大小 + 头8字节）
        p->current_pos += chunk_size + 8;

        // 处理填充字节（奇数长度对齐）
        if (chunk_size % 2 != 0) {
            p->current_pos++;
        }

        if (is_vid == 1) {
            break;
        }
    }

    return 0;
}

int avi_get_width(void *avip)
{
    AVIPlayer *p = (AVIPlayer *)avip;
    return p->width;
}
int avi_get_height(void *avip)
{
    AVIPlayer *p = (AVIPlayer *)avip;
    return p->height;
}
void *avi_get_fp(void *avip)
{
    AVIPlayer *p = (AVIPlayer *)avip;
    return p->file;
}
int avi_get_now_len(void *avip)
{
    AVIPlayer *p = (AVIPlayer *)avip;
    return p->now_len;
}
int avi_get_now_offset(void *avip)
{
    AVIPlayer *p = (AVIPlayer *)avip;
    return p->now_offset;
}
// u8* avi_get_mjpegbuf(void* avip){AVIPlayer* p = (AVIPlayer*)avip;return p->mjpegbuf;}
int avi_get_mjpegbuflen(void *avip)
{
    AVIPlayer *p = (AVIPlayer *)avip;
    return p->video_stream_header.dwSuggestedBufferSize;
}
int avi_get_redrawid(void *avip)
{
    AVIPlayer *p = (AVIPlayer *)avip;
    return p->redrawid;
}
float avi_get_total_sec(void *avip)
{
    AVIPlayer *p = (AVIPlayer *)avip;
    return (float)p->total_duration / 1000000.0;
}
void *avi_get_view_file_info(void *priv)
{
    AVIPlayer *p = (AVIPlayer *)priv;
    return (p->view_file_info);
}

/* GPU task id 产生 */
#define GPU_TASK_ID(page, id, index) ((page << 26) | (0 << 24) | (id << 8) | index)
/* ------------------------------------------------------------------------------------*/
/**
 * @brief JLGPU_TASK_INHERIT_PARENT 继承父控件的GPU参数
 *
 * 注意：这里可能会修改task_param的draw参数，应放到draw参数赋值之后
 */
/* ------------------------------------------------------------------------------------*/
#define JLGPU_TASK_INHERIT_PARENT(tran)                                                                           \
    {                                                                                                             \
        if (dc->elm && dc->elm->parent)                                                                           \
        {                                                                                                         \
            log_debug("%s(), get parent info!:%x\n", __func__, dc->elm->id);                                      \
            struct element *elm = dc->elm->parent;                                                                \
            struct element *par = elm->parent;                                                                    \
            if ((par) && (ui_id2type(par->id) == CTRL_TYPE_GRID))                                                 \
            {                                                                                                     \
                elm = par;                                                                                        \
            }                                                                                                     \
            u8 is_overstepping_parent;                                                                            \
            pJLGPUTaskUnit_t task_parent = jlgpu_get_task_by_id(dc->gpu_task_head, JLGPU_ID_NONE, elm->id);       \
            if (task_parent)                                                                                      \
            {                                                                                                     \
                is_overstepping_parent = false;                                                                   \
                struct element *elm = ui_core_get_element_by_id(task_parent->info.element_id);                    \
                if (elm)                                                                                          \
                {                                                                                                 \
                    ui_core_get_element_rect_relative_screen(elm, &task_param.area);                              \
                    jlgpu_shift_rect(&task_param.area);                                                           \
                }                                                                                                 \
                else                                                                                              \
                {                                                                                                 \
                    jlgpu_get_task_rect(task_parent, &task_param.area);                                           \
                }                                                                                                 \
            }                                                                                                     \
            else                                                                                                  \
            {                                                                                                     \
                is_overstepping_parent = true;                                                                    \
                /* ui_core_get_element_abs_rect(elm, &task_param.area); */                                        \
                ui_core_get_element_rect_relative_screen(elm, &task_param.area);                                  \
            }                                                                                                     \
            /*log_info("%s(), parent area[%d, %d, %d, %d]\n", __func__,                                           \
                    task_param.area.left, task_param.area.top, task_param.area.width, task_param.area.height); */ \
            INHERIT_PARENT_AFFINE(tran);                                                                          \
            if (task_param.matrix && is_overstepping_parent)                                                      \
            {                                                                                                     \
                gpu_boundbox_t parent_area;                                                                       \
                parent_area.minx = task_param.area.left;                                                          \
                parent_area.maxx = task_param.area.width + parent_area.minx;                                      \
                parent_area.miny = task_param.area.top;                                                           \
                parent_area.maxy = task_param.area.height + parent_area.miny;                                     \
                gpu_matrix_get_boundbox(task_param.matrix, &parent_area, &parent_area);                           \
                task_param.area.left = parent_area.minx;                                                          \
                task_param.area.width = parent_area.maxx - parent_area.minx;                                      \
                task_param.area.top = parent_area.miny;                                                           \
                task_param.area.height = parent_area.maxy - parent_area.miny;                                     \
            }                                                                                                     \
            else if (is_overstepping_parent)                                                                      \
            {                                                                                                     \
                jlgpu_shift_rect(&task_param.area);                                                               \
                /* struct lcd_info *lcd_info = &(((struct ui_priv *)__this)->info); */                            \
                /* task_param.area.left += ((GPU_BOUND_BOX_MAX - lcd_info->width) / 2); */                        \
                /* task_param.area.top += ((GPU_BOUND_BOX_MAX - lcd_info->height) / 2); */                        \
            }                                                                                                     \
        }                                                                                                         \
    }

/* ------------------------------------------------------------------------------------*/
/**
 * @brief JLGPU_TASK_SCALE_CONFIG 配置GPU任务的缩放参数
 */
/* ------------------------------------------------------------------------------------*/
#define JLGPU_TASK_SCALE_CONFIG(tran)                                             \
    {                                                                             \
        task_param.scale_en = dc->elm->ratio_en;                                  \
        if (task_param.scale_en && dc->elm->css.part)                             \
        {                                                                         \
            (tran)->ratio_w = dc->elm->css.part->ratio.ratio_w;                   \
            (tran)->ratio_h = dc->elm->css.part->ratio.ratio_h;                   \
            log_debug("%s(), scale_en: %d, ratio_w: %f, ratio_h: %f\n", __func__, \
                      task_param.scale_en, (tran)->ratio_w, (tran)->ratio_h);     \
        }                                                                         \
    }

/* ------------------------------------------------------------------------------------*/
/**
 * @brief INHERIT_PARENT_AFFINE 继承父控件的变换矩阵
 */
/* ------------------------------------------------------------------------------------*/
#define INHERIT_PARENT_AFFINE(tran)                                                                         \
    {                                                                                                       \
        for (; elm; elm = elm->parent)                                                                      \
        {                                                                                                   \
            pJLGPUTaskUnit_t task_parent = jlgpu_get_task_by_id(dc->gpu_task_head, JLGPU_ID_NONE, elm->id); \
            if (task_parent && task_parent->info.matrix)                                                    \
            {                                                                                               \
                log_info("%s(), get parent affine! parent elm_id: 0x%x\n", __func__, elm->id);              \
                task_param.matrix = task_parent->info.matrix;                                               \
                struct rect parent_rect = {0};                                                              \
                ui_core_get_element_rect_relative_screen(elm, &parent_rect);                                \
                /* ui_core_get_element_rect_relative_parent(elm, &parent_rect); */                          \
                /* DUMP_RECT(__func__, __LINE__, "parent", &parent_rect); */                                \
                task_param.draw.left -= (/* parent_rect.left < 0 ? 0 : */ parent_rect.left);                \
                task_param.draw.top -= (/* parent_rect.top < 0 ? 0 : */ parent_rect.top);                   \
                if (task_param.draw.height <= 0 || task_param.draw.width <= 0)                              \
                {                                                                                           \
                    task_param.invisible = true;                                                            \
                }                                                                                           \
                if (task_param.rotate_en)                                                                   \
                {                                                                                           \
                    (tran)->rotate_dx -= parent_rect.left;                                                  \
                    (tran)->rotate_dy -= parent_rect.top;                                                   \
                }                                                                                           \
            }                                                                                               \
        }                                                                                                   \
    }
/* ------------------------------------------------------------------------------------*/
/**
 * @brief JLGPU_TASK_ROTATE_CONFIG 配置GPU任务的旋转参数
 */
/* ------------------------------------------------------------------------------------*/
#define JLGPU_TASK_ROTATE_CONFIG(tran)                                                         \
    {                                                                                          \
        task_param.rotate_en = dc->elm->rotate_en;                                             \
        if (task_param.rotate_en && dc->elm->css.part)                                         \
        {                                                                                      \
            (tran)->rotate_cx = dc->elm->css.part->rotate.cent_x;                              \
            (tran)->rotate_cy = dc->elm->css.part->rotate.cent_y;                              \
            (tran)->rotate_dx = dc->elm->css.part->rotate.dx;                                  \
            (tran)->rotate_dy = dc->elm->css.part->rotate.dy;                                  \
            (tran)->rotate_angle = dc->elm->css.part->rotate.angle;                            \
            log_debug("%s(), rotate_en: %d, cx:%d, cy:%d, dx:%d, dy:%d, angle:%f\n", __func__, \
                      task_param.rotate_en, (tran)->rotate_cx, (tran)->rotate_cy,              \
                      (tran)->rotate_dx, (tran)->rotate_dy, (tran)->rotate_angle);             \
        }                                                                                      \
    }
/* ------------------------------------------------------------------------------------*/
/**
 * @brief jlui_image_invert_deal 图片翻转处理
 *
 * @param dc		绘图句柄
 * @param taskp		gpu任务
 *
 * @return
 */
/* ------------------------------------------------------------------------------------*/
static int jlui_image_invert_deal(struct draw_context *dc, pJLGPUTaskUnit_t taskp)
{
    gpu_matrix_t matrix;
    struct rect win_rect;
    jlgpu_get_win_rect(&win_rect);
    jlgpu_get_matrix(taskp, &matrix);                           // 获取任务矩阵
    gpu_matrix_translate(&matrix, win_rect.left, win_rect.top); // 回到gpu原点

    gpu_matrix_flip(&matrix, dc->draw_img.invert_x_en, dc->draw_img.invert_y_en); // 翻转

    int invert_x_offset = 0;
    int invert_y_offset = 0;
    if (dc->draw_img.invert_x_en == 1) {
        invert_x_offset += 2 * taskp->info.image_width;
        invert_x_offset += dc->draw_img.invert_x_dist;
    } else if (dc->draw_img.invert_x_en == 2) {
        invert_x_offset -= dc->draw_img.invert_x_dist;
    }
    if (dc->draw_img.invert_y_en == 1) {
        invert_y_offset += 2 * taskp->info.image_height;
        invert_y_offset += dc->draw_img.invert_y_dist;
    } else if (dc->draw_img.invert_y_en == 2) {
        invert_y_offset -= dc->draw_img.invert_y_dist;
    }
    gpu_matrix_translate(&matrix, invert_x_offset, invert_y_offset); // 翻转后偏移

    int trans_x = -win_rect.left;
    int trans_y = -win_rect.top;
    if (dc->draw_img.invert_x_en) {
        trans_x *= -1;
    }
    if (dc->draw_img.invert_y_en) {
        trans_y *= -1;
    }
    gpu_matrix_translate(&matrix, trans_x, trans_y); // 回到gpu中心

    jlgpu_set_matrix(taskp, &matrix); // 配置任务矩阵

    gpu_boundbox_t bbox;
    bbox.minx = 0;
    bbox.miny = 0;
    bbox.maxx = taskp->info.image_width;
    bbox.maxy = taskp->info.image_height;
    gpu_matrix_get_boundbox(&matrix, &bbox, &bbox);  // 计算翻转后的有效区域
    jlgpu_driver_set_bbox(&taskp->task_addr, &bbox); // 配置任务有效区域
    return 0;
}
/* ------------------------------------------------------------------------------------*/
/**
 * @brief jlui_draw_jpeg_cb jpeg绘图回调
 *
 * @param id				0
 * @param dst_buf			屏显buffer
 * @param dst_r				屏显buffer rect
 * @param src_r				图片/控件尺寸
 * @param bytes_per_pixel	格式
 * @param priv				jpeg句柄
 * @param matrix			变换矩阵
 */
/* ------------------------------------------------------------------------------------*/
static void jlui_draw_jpeg_cb(int id, u8 *dst_buf, struct rect *dst_r, struct rect *src_r, u8 bytes_per_pixel, void *priv, void *matrix)
{

    if ((dst_r->width != lcd_get_screen_width()) && (dst_r->width + dst_r->left != lcd_get_screen_width())) {
        // 不支持局部刷新
        return;
    }
    jdec_opj *opj = (jdec_opj *)priv; // jpeg解码句柄
    opj->matrix = matrix;             // 获取变换矩阵
    // 计算缩放系数
    float ratio_w = 1.0;
    float ratio_h = 1.0;
    if (opj->matrix) {
        gpu_matrix_t *m = matrix;
        ratio_w = m->m[0][0]; //
        ratio_h = m->m[1][1]; //
        /* log_info("ratio_w: %f, ratio_h: %f\n", ratio_w, ratio_h); */
    }

    /* 计算变换后图片的输出区域 */
    struct rect jpeg_draw_src;
    memcpy(&jpeg_draw_src, src_r, sizeof(struct rect));
    // 放大时为中心放大
    int center_y = src_r->top + src_r->height / 2;
    jpeg_draw_src.height = opj->height / ratio_h;
    jpeg_draw_src.top = center_y - jpeg_draw_src.height / 2;
    int center_x = src_r->left + src_r->width / 2;
    jpeg_draw_src.width = opj->width / ratio_w;
    jpeg_draw_src.left = center_x - jpeg_draw_src.width / 2;
    /* DUMP_RECT(__func__, __LINE__, "dst_r", dst_r);						//推屏buf */
    /* DUMP_RECT(__func__, __LINE__, "src_r", src_r);						//jpeg显示区域 */
    /* DUMP_RECT(__func__, __LINE__, "jpeg_draw_src", &jpeg_draw_src);		//jpeg缩放后区域 */

    /* 计算图片输出和推屏Buf的重叠区域 */
    struct rect cover_r_t;
    if (!get_rect_cover(dst_r, src_r, &cover_r_t)) {
        return;
    }
    // 计算缩放后的区域*以推屏区域、jpeg缩放区域、控件显示区域的交集作为最终输出，放大时受控件尺寸限制会变成中心放大
    struct rect cover_r;
    get_rect_cover(&cover_r_t, &jpeg_draw_src, &cover_r);
    /* DUMP_RECT(__func__, __LINE__, "cover_r", &cover_r);					//重叠区域 */
    // 计算jpeg解码的区域
    int draw_top = cover_r.top - jpeg_draw_src.top; // 绘制的top
    int draw_btm = cover_r.height + draw_top;       // 绘制的bottom
    int draw_h = cover_r.height;                    // 绘制高度
    /* int dec_top = draw_top * ratio_h;										//需要解码的top */
    /* int dec_btm = ceilf((float)draw_btm * ratio_h);							//需要解码的高度，这里需要向上取 */
    /* int dec_h	 = dec_btm - dec_top;										//解码高度 */
    int max_dec_height = (opj->msy << 3) / ratio_h; // 一个mcu分块缩放后可以有效绘制的行数
    int sub_top = 0;
    int sub_height = (draw_h > max_dec_height) ? max_dec_height : draw_h;

    int a = jiffies_usec();

    // 按mcu快缩放后的有效高度进行分行处理
    while (sub_top + draw_top < draw_btm) {
        int dec_sub_top = (draw_top + sub_top) * ratio_h;
        int dec_sub_btm = ceilf((float)(draw_top + sub_top + sub_height) * ratio_h);
        int dec_sub_h = dec_sub_btm - dec_sub_top;
        /* log_info("%s draw(%d %d) dec(%d %d)%d maxdec%d sub(%d %d) dec_sub(%d %d)", */
        /* __func__, draw_top, draw_btm, dec_top, dec_btm, dec_h, max_dec_height, sub_top, sub_height, dec_sub_top, dec_sub_btm); */
        struct rect jpeg_dec_r; /*jpeg解码区域*/
        /* jpeg_dec_r.left = (cover_r.left - src_r->left) * ratio_w; */
        jpeg_dec_r.left = (cover_r.left - jpeg_draw_src.left) * ratio_w;
        jpeg_dec_r.top = dec_sub_top;
        jpeg_dec_r.width = cover_r.width * ratio_w;
        jpeg_dec_r.height = dec_sub_h;
        struct rect block_r; /*缩放后显示的区域*/
        block_r.top = cover_r.top + sub_top;
        block_r.height = sub_height;
        block_r.left = dst_r->left;
        block_r.width = dst_r->width;
        /* DUMP_RECT(__func__, __LINE__, "block_r", &block_r); */
        /* DUMP_RECT(__func__, __LINE__, "jpeg_dec_r", &jpeg_dec_r); */
        u8 *disp_buf = dst_buf + (block_r.top - dst_r->top) * 2 * lcd_get_screen_width();
        /*启动解码*/

        // log_info("--");
        jljpeg_decode_start(opj, &jpeg_draw_src, &jpeg_dec_r, &cover_r, &block_r, disp_buf);

        sub_top += sub_height;
        sub_height = (sub_top + sub_height + draw_top < draw_btm) ? max_dec_height : draw_btm - draw_top - sub_top;
    }

    int b = jiffies_usec();
    // log_info(">>>%d", b-a);
}

/* ------------------------------------------------------------------------------------*/
/**
 * @brief jlui_jpeg_infunc jpeg位流输入
 *
 * @param opj	句柄
 * @param buf	位流buf
 * @param len	预获取长度
 *
 * @return 		实际数据长度
 */
/* ------------------------------------------------------------------------------------*/

static volatile const u32 psram_size = (u32)ps_ram_size;

#define IS_SFC1_ADDR(addr) (!psram_size && ((addr) >= 0x08000000) && ((addr) < 0x0c000000))

static u32 jlui_jpeg_infunc(struct _jdec_opj *opj, u8 *buf, u32 len)
{
    u32 rets;
    __asm__ volatile("%0 = rets" : "=r"(rets));
    /* log_info("%s jpeg_info:%x %x buf:%x rets:0x%x\n", __func__, (int)opj, (int)opj->device, (int)buf, rets); */
    struct flash_file_info *image_info = (struct flash_file_info *)opj->device;
    int start_addr, end_addr, curr_addr, read_len;
    if (image_info->tab) {
        // 使用mmu_table
        start_addr = image_info->tab[0] + image_info->offset;
        end_addr = image_info->tab[image_info->tab_size / 4 - 1] + image_info->last_tab_data_len;
        curr_addr = start_addr + opj->curr_offset;
        read_len = (curr_addr + len > end_addr) ? end_addr - curr_addr : len;
    } else {
        start_addr = image_info->offset;
        end_addr = image_info->offset + image_info->last_tab_data_len;
        curr_addr = start_addr + opj->curr_offset;
        read_len = (curr_addr + len > end_addr) ? end_addr - curr_addr : len;
    }

    /* log_info("%s start_addr:0x%x end_addr:0x%x file_len:%d curr_addr:0x%x len:%d read_len:%d", */
    /* __func__, start_addr, end_addr, end_addr - start_addr, curr_addr, len, read_len); */

    if (buf) {
        if (norflash_ext_sfc_en && IS_SFC1_ADDR(curr_addr)) {
            norflash_espi_mutex_enter();
        }
        memcpy(buf, (const u8 *)curr_addr, read_len);
        if (norflash_ext_sfc_en && IS_SFC1_ADDR(curr_addr)) {
            norflash_espi_mutex_exit();
        }
    }

    return read_len;
}

static u32 jlui_jpeg_infunc_by_fs(struct _jdec_opj *opj, u8 *buf, u32 len)
{
    u32 rets;
    __asm__ volatile("%0 = rets" : "=r"(rets));

    struct avi_fs_info *image_info = (struct avi_fs_info *)opj->device;

    if (image_info->fp) {
        static int all_time = 0;
        static int seek_time = 0;
        if (opj->curr_offset == 0) {
            // log_info("ss");
            all_time = 0;
            seek_time = 0;
        } else {
            // log_info("cb");
        }

        if (buf) {
            int a = jiffies_usec();
            FILE *fp = image_info->fp;
            fseek(fp, opj->curr_offset + image_info->offset, SEEK_SET);
            if (opj->curr_offset + len > image_info->len) {
                len = image_info->len - opj->curr_offset;
            }
            int c = jiffies_usec();

            int ret = avi_fread(buf, 1, len, fp);
            int b = jiffies_usec();
            all_time += b - a;
            seek_time += c - a;

            // log_info("%d", opj->curr_offset);
            if (len == 0) {
                // log_info(">%d %d %d %d", all_time, seek_time, opj->curr_offset, len);
            } else {
                // log_info("=%d %d", b-a, opj->curr_offset + image_info->offset);
            }
            return ret;
        } else {
            return len;
        }
    } else {
        // RAM
        if (opj->curr_offset == 0) {
            // log_info("ss");
        } else {
            // log_info("cb");
        }
        if (buf) {
            if (opj->curr_offset + len > image_info->len) {
                len = image_info->len - opj->curr_offset;
            }
            memcpy(buf, (u8 *)image_info->offset + opj->curr_offset, len);
            return len;
        } else {
            return len;
        }
    }
}


static void _write_pcm(void *player, u8 *ptr, int len);
static int  _write_jlaudio(void *player, u8 *ptr, int len)
{
    AVIPlayer *pp = (AVIPlayer *)player;
    // log_info("[_write_aivoice] len:%d\n", len);
    int cnt = 0;
    if (__this->param.is_audio_mute != 1) {
        while (len) {
            u32 w = cbuf_write(&avi_pcm_cbuf, ptr, len); // 直接按len写，返回实际写入
            /* log_info("[cbuf_write] w:%d len=%d\n", w, len); */
            if (w == 0) {
                os_time_dly(1);
                continue;
            }
            ptr += w;
            len -= w;
        }
    }
    pp->jump_mjpeg = jiffies_usec();
    return len;
}
static void play_pcm(void *player, FILE *file, int data, int chunk_size)
{
    AVIPlayer *ppp = (AVIPlayer *)player;
    if (ppp->audio_format.wFormatTag == 0) {
        return;
    }

    // int a = jiffies_usec();
    uint8_t *pp = avi_malloc(chunk_size + 4);
    uint8_t  *p = pp;
    p = (u8 *)(((int)p + 3) & ~3);
    avi_fread((void *)p, chunk_size, 1, file);
    // int b = jiffies_usec();
    int rlen = _write_jlaudio(player, p, chunk_size);
    avi_free(pp);

    // log_info("pcm %d s %d",b-a, chunk_size);
}

static void display_mjpeg(void *player, FILE *file, int data, int chunk_size, int width, int height)
{
    AVIPlayer *p = (AVIPlayer *)player;

    // 找空闲的buf

    int idle = -1;
    os_mutex_pend(&p->mutex, 0);
    int i = p->cache.pre_idx;
    i++;
    if (i >= p->cache.cnt) {
        i = 0;
    }
    if (i != p->cache.act_idx) {
        idle = i;
    }
    os_mutex_post(&p->mutex);

    if (idle != -1) {
        p->cache.m[idle].pos = data;
        p->cache.m[idle].len = chunk_size;
        // log_info("w %d %x", idle, 0);
        if (p->cache.m[idle].data) {

            // static void *temp;
            // if(temp==0){
            //     temp = malloc_psram(60*1024);
            // }

            int rlen = avi_fread(p->cache.m[idle].data, 1, chunk_size, file);
            // rlen = avi_fread(temp, 1, chunk_size, file);

            // int ret = memcmp(temp, p->cache.m[idle].data, chunk_size);
            // ASSERT(ret);
        }

        /* if (chunk_size == chunk_size) */
        if (1) {
            // psram_flush_cache(p->cache.m[idle].data, chunk_size);

            os_mutex_pend(&p->mutex, 0);
            p->cache.pre_idx = idle;
            os_mutex_post(&p->mutex);
            p->buf_ok = 1;
        } else {
            log_info("read mjpeg error %d", 0);
        }
    }

    // os_mutex_post(&p->mutex);

    return;
}

//  等待上一帧GPU合成完成和忙标志，即等待文件系统空闲
//  if(音频帧)
//      遍历所有写cbuf

//  if(视频帧)
//      直接ui 发送draw gpu任务，使能忙标志

int avi_fluh(void *priv)
{
    return avi_exec(priv, display_mjpeg, play_pcm);
}

static void avi_timer(void *priv)
{
    // log_info("%s %d", __func__, __LINE__);

    // log_info("priv %x", priv);

    AVIPlayer *p = (AVIPlayer *)priv;

    // if(p->ui_work){
    //     return;
    // }

    int a = jiffies_usec();
    //  等待上一个画面绘制完成，这个过程不能操作所有avi_fread
    // p->ui_work = 0;
    // jlgpu_scheduler_wait_sync();
    int b = jiffies_usec();
    if (p->ui_work == 2 || -1 == avi_exec(priv, display_mjpeg, play_pcm)) {
        p->ui_work = 0;
        log_info("exti avitimer");
        p->is_playing = 0;
        return;
    }
    // jlgpu_scheduler_wait_sync();
    int c = jiffies_usec();
    // log_info("avi_timer %d %d", b-a, c-b);

    int argv[3] = {0};
    int retry = 3;
    argv[0] = (int)avi_timer;
    argv[1] = 1;
    argv[2] = (int)p;

__try_again:
    int ret = os_taskq_post_type("ui", Q_CALLBACK, 3, argv);
    if (ret) {
        if (retry) {
            os_time_dly(1);
            retry--;
            goto __try_again;
        }
        log_info("%s post ret:%d retry:%d\n", __func__, ret, retry);
    }
}

// void avi_draw(void *dc){
//     if(avi_fs.offset){

//         jlui_draw_image(dc, 0, 0, 0, avi_fs);
//     }
// }

static void _write_pcm(void *player, u8 *ptr, int len)
{
    // log_info(">>> _write_pcm len=%d\n", len);

    // int len = len;

    AVIPlayer *pp = (AVIPlayer *)player;

    while (len) {
        // 往 dac 写数据

        // log_info("ptr %d len %d\n", ptr, len);

        // u8 * p = avi_malloc(len);
        // static u8 p[4096];
        // memcpy(p, ptr, len);

        int wlen = audio_dac_write(&dac_hdl, ptr, len);
        // avi_free(p);

        if (wlen != len) {
            // dac缓存满了，延时 10ms 后再继续写
            os_time_dly(1);
        }

        // if (wlen != len) {	// dac缓存满了，延时 10ms 后再继续写d

        //     int rate = pp->audio_format.nSamplesPerSec * pp->audio_format.nChannels / 100;
        //     // os_time_dly(10);
        //     // static int last_jiff = 0;
        //     if(len - wlen >= rate){
        //         // log_info(">>> dac buffer full %d %d\n", wlen , len);
        //         // log_info("D >>> %d", (jiffies_usec() - last_jiff)/1000);
        //         // if(jiffies_usec() - last_jiff > 20*1000) {
        //         //     pp->jump_mjpeg = 1;
        //         // } else {
        //         //     pp->jump_mjpeg = 0;
        //         // }
        //         // last_jiff = jiffies_usec();

        //         // if((len - wlen)/rate/2){
        //         //     log_info("delay %d", (len - wlen)/rate);
        //             os_time_dly(1);
        //         // }

        //     }
        // }
        ptr += wlen; // / 2 / ((dac_hdl.pd->bit_width) ? 2 : 1);
        len -= wlen;
    }
    pp->jump_mjpeg = jiffies_usec();
}

// AVI线程

static void md_flush(MV_DRAW *md)
{
    // 刷帧
    // log_info("11 %d", jiffies_msec());

    AVIPlayer *p = (AVIPlayer *)md->st;
    /* printf("[msg]%s-%d>>>>>>>>>>>md->pause=%d,p->avih_header.dwMicroSecPerFrame=%d p->audio_format.wFormatTag=%d", __FUNCTION__, __LINE__, md->pause, p->avih_header.dwMicroSecPerFrame, p->audio_format.wFormatTag); */

    if (md->pause == 1) {
        md->flushTimer = sys_timeout_add(md, (void(*)(void *))md_flush, p->avih_header.dwMicroSecPerFrame / 2000);
        return ;
    }
    avi_fluh(md->st);


    if (p->audio_format.wFormatTag == 0x0001 && __this->param.is_audio_mute != 1) {
        md->flushTimer = sys_timeout_add(md, (void(*)(void *))md_flush, p->avih_header.dwMicroSecPerFrame / 2000);
    } else {
        md->flushTimer = sys_timeout_add(md, (void(*)(void *))md_flush, p->avih_header.dwMicroSecPerFrame / 1000);
    }
}

void play_task(MV_DRAW *md)
{

    // void * test = avi_malloc(30000);
    // put_buf(test, 16);
    int msg[32];
    int ret;
    while (1) {
        // 接收
        memset(msg, 0, sizeof(msg));
        ret = os_taskq_pend("taskq", msg, ARRAY_SIZE(msg));

        log_info("ret %d %d %d %d", ret, msg[0], msg[1], msg[2]);

        if (ret != OS_TASKQ) {
            continue;
        }

        //  play
        if (msg[1] == AVI_PLAY_STATUS) {
            log_info("avi play");
            md->flushTimer = sys_timeout_add(md, (void(*)(void *))md_flush, 1);
        }

        // stop
        if (msg[1] == AVI_STOP_STATUS) {
            log_info("avi stop");
            if (__this->param.is_audio_mute != 1) {

                avi_pcm_close(md->st);
            }
            if (md->flushTimer) {
                sys_timeout_del(md->flushTimer);
                md->flushTimer = 0;
            }
            os_sem_post(&md->mv_sem);
            os_time_dly(-1);
        }
    }
}
void *get_aviplay_handle()
{
    return __this;
}
void *get_avi_player_st_handle()
{
    return __this->st;
}

#if TCFG_APP_VIDEO_EN
void app_video_mode_switch(void *priv)
{
    int ret = FALSE;
    u16 flag = (u16)priv;
    log_info("%s", __func__);
    if (flag != task_switch_flag) {
        log_info("%s %d %d--%d", __func__, __LINE__, flag, task_switch_flag);
        return;
    }

    ++task_switch_flag;
    if (app_get_current_mode_name() == APP_MODE_VIDEO) {
        return;
    }

#if TCFG_APP_BT_EN && TCFG_BT_BACKGROUND_ENABLE
    if (bt_check_already_initializes()) {
#else
    if (1) {
#endif
        ret = app_task_switch_to(APP_MODE_VIDEO, NULL_VALUE);
    }

    if (ret == FALSE) {
        sys_timeout_add((void *)(long)task_switch_flag, app_video_mode_switch, 500);
    }
}
#endif

void *animig_open(char *name, AVI_PARAM param, int arg)
{

    u8 type = 0;

#if TCFG_APP_VIDEO_EN
#if TCFG_APP_BT_EN && TCFG_BT_BACKGROUND_ENABLE
    if (app_get_current_mode_name() != APP_MODE_VIDEO && bt_check_already_initializes()) {
#else
    if (app_get_current_mode_name() != APP_MODE_VIDEO) {
#endif
        int ret = app_task_switch_to(APP_MODE_VIDEO, NULL_VALUE);
        if (ret == FALSE) {
            sys_timeout_add((void *)(long)task_switch_flag, app_video_mode_switch, 500);
        }
    }
#if TCFG_APP_BT_EN && TCFG_BT_BACKGROUND_ENABLE
    else {
        sys_timeout_add((void *)(long)task_switch_flag, app_video_mode_switch, 500);
    }
#endif
#endif

    FILE *f = fopen(name, "r");
    if (f == NULL) {
        log_info("open file fail %s", name);
        return NULL;
    }
    char chunk_id[5] = {0};
    /* 读取RIFF头 */
    if (avi_fread(chunk_id, 1, 4, f) != 4) {
        log_error("Failed to read chunk ID");
        fclose(f);
        return NULL;
    }
    if (strcmp("RIFF", chunk_id)) {
        log_info("no avi %s", name);
        fclose(f);
        return NULL;
    }

    fclose(f);
    type = 2;
    if (__this != NULL) {
        log_error("__this not free");
        avi_free(__this);
    }
    MV_DRAW *md = avi_zalloc(sizeof(MV_DRAW));
    __this = md;
    ASSERT(__this);
    memcpy(&(__this->param), &param, sizeof(AVI_PARAM));
    os_sem_create(&md->mv_sem, 0);
    md->type = type;
    md->repeat = -1;

    if (md->type == 2) {
        md->st = avi_open(name, arg, 0);
        AVIPlayer *p = (AVIPlayer *)(md->st);
        if (md->st == NULL) {
            goto __err;
        }
        if (p->width > LCD_WIDTH || p->height > LCD_HEIGHT) {
            log_error("vidoe width %d or height%d over lcd width %d or height %d", p->width, p->height, LCD_WIDTH, LCD_HEIGHT);
            goto __err;
        }
        if (__this->param.is_audio_mute != 1) {
            avi_pcm_open(md->st);
        }
        wdt_clear();
        avi_fluh(md->st);
    }
    /* strcpy(__this->param.fname,name,) */
    /* char *taskname = strrchr(name, '/'); */
    /* taskname = taskname ? taskname + 1 : name; */
    /* taskname = "avi_task"; */
    u8 fname_len = (sizeof(__this->param.fname) / sizeof(__this->param.fname[0]));
    strncpy(__this->param.fname, name, fname_len);
    printf("[msg]%s-%d>>>>>>>>>>>__this->param.fname=%s", __FUNCTION__, __LINE__, __this->param.fname);
    __this->param.fname[fname_len - 1] = '\0';
    log_info("create %s", AVI_TASK_NAME);
    task_create((void(*)(void *))play_task, md, AVI_TASK_NAME);

    os_taskq_post(AVI_TASK_NAME, 1, AVI_PLAY_STATUS);
    return md;
__err:
    void avi_close(void *avip);
    avi_close(md->st);
    __this = NULL;
    if (md) {
        avi_free(md);
    }
    return NULL;
}

void animig_close(void *ap)
{
    MV_DRAW *md = ap;

    // while (md->act != -1) {
    //     md->act = 0;
    //     os_time_dly(1);
    // }\\

    os_taskq_post(AVI_TASK_NAME, 1, AVI_STOP_STATUS);
    os_sem_pend(&md->mv_sem, 0);
    log_info("kill %s", AVI_TASK_NAME);
    jlgpu_scheduler_wait_sync();
    task_kill("avi_task");

    avi_close(md->st);

    // jlgpu_scheduler_wait_sync();
    // os_sem_del(&md->mv_sem, 0);

    // jlgpu_scheduler_wait_sync();
    if (md) {
        avi_free(md);
        md = NULL;
    }
}

void avi_jpeg_init(void *priv)
{

    log_debug("\n%s,line = %d\n", __func__, __LINE__);
    AVIPlayer *p = priv;

    ASSERT(p);

    os_mutex_pend(&p->mutex, 0);

    // 找出最接近act的已成功cache的jpg
    if (1) {
        log_debug("\n%s,line = %d\n", __func__, __LINE__);
        void *new = jpeg_malloc(sizeof(jdec_opj)); // 提前申请避免撞指针

        wdt_clear();
        /* log_info("\n%s,line = %d\n", __func__, __LINE__); */
        /* log_info("\np->cache = %x\n", (u32)p->cache); */
        /* log_info("\np->cache->idx = %x\n", (u32)p->cache.act_idx); */
        /* log_info("\np->cache.m = %x\n", p->cache.m); */
        /* log_info("\np->cache.m[p->cache.act_idx] = %x\n", p->cache.m[p->cache.act_idx]); */
        /* log_info("\np->cache.m[p->cache.act_idx].opj = %x\n", p->cache.m[p->cache.act_idx].opj); */
        /* log_info("\n%s,line = %d\n", __func__, __LINE__); */

        if (p->cache.m[p->cache.act_idx].opj) {
            log_debug("\n%s,line = %d\n", __func__, __LINE__);
            jljpeg_decode_release(p->cache.m[p->cache.act_idx].opj, 1);
            jpeg_free(p->cache.m[p->cache.act_idx].opj->device);
            jpeg_free(p->cache.m[p->cache.act_idx].opj);

            if (jljpeg_get_decode_hd() == p->cache.m[p->cache.act_idx].opj) {
                jljpeg_get_decode_hd_clr();
            }
            p->cache.m[p->cache.act_idx].opj = NULL;
        }
        log_debug("\n%s,line = %d\n", __func__, __LINE__);
        if (p->cache.act_idx != p->cache.pre_idx) {
            p->cache.act_idx++;
            if (p->cache.act_idx >= p->cache.cnt) {
                p->cache.act_idx = 0;
            }
            // log_info("->%d %x", p->cache.act_idx, p->cache.m[p->cache.act_idx].data );
        } else {
            /* log_info("="); */
        }

        if (p == NULL) {
            log_debug("P == NULL!!!\n");
            return;
        }
        log_debug("p = %p \n", p);
        log_debug("p->cache = %x\n", (u32) & (p->cache));
        log_debug("\n\n[avi_jpeg_init] idx =%d\n\n", p->cache.act_idx);

        p->cache.m[p->cache.act_idx].opj = new; // jpeg_malloc(sizeof(jdec_opj));
        ASSERT(p->cache.m[p->cache.act_idx].opj);
        memset(p->cache.m[p->cache.act_idx].opj, 0, sizeof(jdec_opj));
        p->cache.m[p->cache.act_idx].opj->device = jpeg_malloc(sizeof(struct flash_file_info));
        memset(p->cache.m[p->cache.act_idx].opj->device, 0, sizeof(struct flash_file_info));
        ASSERT(p->cache.m[p->cache.act_idx].opj->device);
        struct flash_file_info *f = p->cache.m[p->cache.act_idx].opj->device;
        if (avi_get_view_file_info(p)) {
            // TAB表
            struct flash_file_info *info = avi_get_view_file_info(p);
            struct image_file file = {0};
            file.offset = p->cache.m[p->cache.act_idx].pos;
            file.len = p->cache.m[p->cache.act_idx].len;
            ui_res_get_image_flash_info(f, info, &file);
        } else {
            f->offset = (u32)p->cache.m[p->cache.act_idx].data;
            f->last_tab_data_len = p->cache.m[p->cache.act_idx].len;
        }

        int ret;
        if (p->buf_ok) {
            ret = jpeg_module_opj_init((void *)p->cache.m[p->cache.act_idx].opj);
        } else {
            ret = -1;
        }

        // log_info("=");
        if (ret) {
            log_info("%s %d", __func__, __LINE__);
            if (p->cache.m[p->cache.act_idx].opj) {
                jljpeg_decode_release(p->cache.m[p->cache.act_idx].opj, 1);
                if (p->cache.m[p->cache.act_idx].opj->device) {
                    jpeg_free(p->cache.m[p->cache.act_idx].opj->device);
                }
                jpeg_free(p->cache.m[p->cache.act_idx].opj);
                p->cache.m[p->cache.act_idx].opj = NULL;
            }
        }
    }

    __this->avi_now_idx = p->cache.act_idx;

    os_mutex_post(&p->mutex);
}

static void jpeg_draw_cb(int id, u8 *dst_buf, struct rect *dst_r, struct rect *src_r, u8 bytes_per_pixel, void *priv, void *matrix)
{
    jdec_opj *opj = (jdec_opj *)priv; // jpeg解码句柄
    if (jljpeg_get_decode_hd() && (opj != jljpeg_get_decode_hd())) {
        jljpeg_decode_reset_curr();
    }
    opj->matrix = matrix; // 获取变换矩阵
    // 计算缩放系数
    float ratio_w = 1.0;
    float ratio_h = 1.0;
    if (opj->matrix) {
        gpu_matrix_t *m = matrix;
        ratio_w = m->m[0][0]; //
        ratio_h = m->m[1][1]; //
        /* log_debug("ratio_w: %f, ratio_h: %f\n", ratio_w, ratio_h); */
    }

    /* 计算变换后图片的输出区域 */
    struct rect jpeg_draw_src;
    memcpy(&jpeg_draw_src, src_r, sizeof(struct rect));
    // 放大时为中心放大
    int center_y = src_r->top + src_r->height / 2;
    jpeg_draw_src.height = opj->height / ratio_h;
    jpeg_draw_src.top = center_y - jpeg_draw_src.height / 2;
    int center_x = src_r->left + src_r->width / 2;
    jpeg_draw_src.width = opj->width / ratio_w;
    jpeg_draw_src.left = center_x - jpeg_draw_src.width / 2;
    /* DUMP_RECT(__func__, __LINE__, "dst_r", dst_r);						//推屏buf */
    /* DUMP_RECT(__func__, __LINE__, "src_r", src_r);						//jpeg显示区域 */
    /* DUMP_RECT(__func__, __LINE__, "jpeg_draw_src", &jpeg_draw_src);		//jpeg缩放后区域 */

    /* 计算图片输出和推屏Buf的重叠区域 */
    struct rect cover_r_t;
    if (!get_rect_cover(dst_r, src_r, &cover_r_t)) {
        return;
    }
    // 计算缩放后的区域*以推屏区域、jpeg缩放区域、控件显示区域的交集作为最终输出，放大时受控件尺寸限制会变成中心放大
    struct rect cover_r;
    get_rect_cover(&cover_r_t, &jpeg_draw_src, &cover_r);
    /* DUMP_RECT(__func__, __LINE__, "cover_r", &cover_r);					//重叠区域 */
    // 计算jpeg解码的区域
    int draw_top = cover_r.top - jpeg_draw_src.top; // 绘制的top
    int draw_btm = cover_r.height + draw_top;       // 绘制的bottom
    int draw_h = cover_r.height;                    // 绘制高度
    /* int dec_top = draw_top * ratio_h;										//需要解码的top */
    /* int dec_btm = ceilf((float)draw_btm * ratio_h);							//需要解码的高度，这里需要向上取 */
    /* int dec_h	 = dec_btm - dec_top;										//解码高度 */
    int max_dec_height = (opj->msy << 3) / ratio_h; // 一个mcu分块缩放后可以有效绘制的行数
    int sub_top = 0;
    int sub_height = (draw_h > max_dec_height) ? max_dec_height : draw_h;
    // 按mcu快缩放后的有效高度进行分行处理
    while (sub_top + draw_top < draw_btm) {
        int dec_sub_top = (draw_top + sub_top) * ratio_h;
        int dec_sub_btm = ceilf((float)(draw_top + sub_top + sub_height) * ratio_h);
        int dec_sub_h = dec_sub_btm - dec_sub_top;
        /* log_debug("%s draw(%d %d) dec(%d %d)%d maxdec%d sub(%d %d) dec_sub(%d %d)", */
        /* __func__, draw_top, draw_btm, dec_top, dec_btm, dec_h, max_dec_height, sub_top, sub_height, dec_sub_top, dec_sub_btm); */
        struct rect jpeg_dec_r; /*jpeg解码区域*/
        /* jpeg_dec_r.left = (cover_r.left - src_r->left) * ratio_w; */
        jpeg_dec_r.left = (cover_r.left - jpeg_draw_src.left) * ratio_w;
        jpeg_dec_r.top = dec_sub_top;
        jpeg_dec_r.width = cover_r.width * ratio_w;
        jpeg_dec_r.height = dec_sub_h;
        struct rect block_r; /*缩放后显示的区域*/
        block_r.top = cover_r.top + sub_top;
        block_r.height = sub_height;
        block_r.left = dst_r->left;
        block_r.width = dst_r->width;
        /* DUMP_RECT(__func__, __LINE__, "block_r", &block_r); */
        /* DUMP_RECT(__func__, __LINE__, "jpeg_dec_r", &jpeg_dec_r); */
        u8 *disp_buf = dst_buf + (block_r.top - dst_r->top) * 2 * dst_r->width;
        /*启动解码*/
        jljpeg_decode_start(opj, &jpeg_draw_src, &jpeg_dec_r, &cover_r, &block_r, disp_buf);
        sub_top += sub_height;
        sub_height = (sub_top + sub_height + draw_top < draw_btm) ? max_dec_height : draw_btm - draw_top - sub_top;
    }
}
void __jpeg_draw_cb_gpu(int id, u8 *dst_buf, struct rect *dst_r, struct rect *src_r, u8 bytes_per_pixel, void *priv, void *matrix)
{
    // return;
    // log_info("priv %d", priv);
    // avi_jpeg_init(priv);

    AVIPlayer *avip = (AVIPlayer *)(*((u32 *)priv));
    // log_info("to avip %x", avip);

    if (!avip) {
        return;
    }

    // os_mutex_pend(&avip->mutex, 0);

    mjpegdata *m = &avip->cache.m[avip->cache.act_idx];
    if (m && !m->opj) {
        log_info("opj == null");
        // mem_stats();
        // os_mutex_post(&avip->mutex);
        return;
    }

    // jljpeg_decode_reset_curr();
    // jljpeg_get_decode_hd_clr();

    // log_info("%x opj %x", priv, m->opj);

    ASSERT(__this->avi_now_idx == avip->cache.act_idx);

    jpeg_draw_cb(id, dst_buf, dst_r, src_r, bytes_per_pixel, m->opj, matrix);

    // os_mutex_post(&avip->mutex);
}
void avi_gpu_scheduler_kick_start(void *priv)
{
    if (avi_player == NULL) {
        return;
    }
    void *gpu_task_head = priv;
    pJLGPUTaskUnit_t pp = jlgpu_task_find_first_task(gpu_task_head);
    while (pp) {
        extern void __jpeg_draw_cb_gpu(int id, u8 * dst_buf, struct rect * dst_r, struct rect * src_r, u8 bytes_per_pixel, void *priv, void *matrix);
        if (pp->info.draw_info) {
            if (pp->info.draw_info->cb_func == __jpeg_draw_cb_gpu) {
                extern void avi_jpeg_init(void *priv);
                u32 avip = *((u32 *)pp->info.draw_info->priv);
                avi_jpeg_init((void *)avip);
            }
        }
        // log_info("444");
        pp = jlgpu_find_next_task(pp);
    }
}
#endif
