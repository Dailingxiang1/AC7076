/* #include "app_task.h" */
#include "system/timer.h"
#include "device/device.h"
#include "key_event_deal.h"

#include "res/resfile.h"
#include "jlui/ui.h"
#include "jlui_app/ui_style.h"
#include "jlui_app/ui_api.h"
#include "jlui_app/res_config.h"
#include "jlui_app/ui_resource.h"
#include "jlui_app/ui_sys_param.h"
#include "ui_draw/ui_type.h"

#include "jlgpu_math.h"		// matrix
#include "jlgpu_driver.h"	// gpu 驱动

#include "gpu_port.h"	// GPU 接口，依赖于 jlgpu_driver.h
#include "gpu_task.h"

#include "jljpeg_decode.h"
#include "jpeg_stream.h"
#include "video/avi/avilib.h"

#define LOG_TAG_CONST       JPEG
#define LOG_TAG     		"[JPEG_STREAM]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"

#define JPEG_STREAM_ALLOC  			malloc_psram
#define JPEG_STREAM_FREE			free_psram

#define JPEG_STREAM_SPIN_LOCK_EN	1

struct jpeg_stream_info {
    // jpeg原图
    u8 *src_buf;
    int src_len;
    volatile u8 data_lock;
    u16 idx;
#if JPEG_STREAM_SPIN_LOCK_EN
    spinlock_t lock;
#else
    OS_MUTEX lock;
#endif
} __jpeg_stream_info;
#define __this (&__jpeg_stream_info)

#if JPEG_STREAM_SPIN_LOCK_EN
#define JPEG_STREAM_LOCK_INIT spin_lock_init(&__this->lock);
#define JPEG_STREAM_LOCK	spin_lock(&__this->lock);
#define JPEG_STREAM_UNLOCK	spin_unlock(&__this->lock);
#else
#define JPEG_STREAM_LOCK_INIT  os_mutex_create(&__this->lock);
#define JPEG_STREAM_LOCK		os_mutex_pend(&__this->lock);
#define JPEG_STREAM_UNLOCK		os_mutex_post(&__this->lock);
#endif

#define GPU_TASK_ID(page, id, index)	((page << 26) | (0 << 24) | (id << 8) | index)


extern u32 usr_mmu_hash(u32 hash, u8 *data, int len);
extern void *jpeg_module_opj_create_file(void *path, int path_len, u32 check, u32 index, u32 task_id, u32 elm_id);
extern void *jpeg_module_opj_create(void *dev, int dev_len, u32 check, u32 index, u32 task_id, u32 elm_id);
extern int jpeg_module_opj_clr_all();
extern void *jpeg_hd_create_priv(u32 task_id, u32 elm_id);
extern int jpeg_hd_get_priv_len();
extern void jpeg_draw_cb_gpu(int id, u8 *dst_buf, struct rect *dst_r, struct rect *src_r, u8 bytes_per_pixel, void *priv, void *matrix);
extern void jlui_free(void *buf, u32 ram_type, u32 module_type);
extern int jpeg_module_opj_init(void *priv);
extern int jljpeg_decode_release(jdec_opj *opj, int global_hd_clr);

/* ------------------------------------------------------------------------------------*/
/**
 * @brief jljpeg_stream_init jpeg数据流初始化
 *
 * @return
 */
/* ------------------------------------------------------------------------------------*/
int jljpeg_stream_init()
{
    JPEG_STREAM_LOCK_INIT;
    __this->data_lock = 0;
    return 0;
}

/* ------------------------------------------------------------------------------------*/
/**
 * @brief jljpeg_stream_deinit

 *
 * @return
 */
/* ------------------------------------------------------------------------------------*/
int jljpeg_stream_deinit()
{
    JPEG_STREAM_LOCK;
    __this->data_lock = 1;
    __this->src_len = 0;
    JPEG_STREAM_FREE(__this->src_buf);
    __this->src_buf = NULL;
    JPEG_STREAM_UNLOCK;
    return 0;
}

/* ------------------------------------------------------------------------------------*/
/**
 * @brief jljpeg_stream_src_data_lock 数据锁保护
 */
/* ------------------------------------------------------------------------------------*/
void jljpeg_stream_src_data_lock()
{
    u32 rets;
    __asm__ volatile("%0 = rets":"=r"(rets));
    log_debug("<%s> rets:%x\n", __func__, rets);
    JPEG_STREAM_LOCK;
    __this->data_lock = 1;
    JPEG_STREAM_UNLOCK;
}

/* ------------------------------------------------------------------------------------*/
/**
 * @brief jljpeg_stream_src_data_unlock 数据解锁保护
 */
/* ------------------------------------------------------------------------------------*/
void jljpeg_stream_src_data_unlock()
{
    u32 rets;
    __asm__ volatile("%0 = rets":"=r"(rets));
    log_debug("<%s> rets:%x\n", __func__, rets);
    JPEG_STREAM_LOCK;
    __this->data_lock = 0;
    JPEG_STREAM_UNLOCK;
}

/* ------------------------------------------------------------------------------------*/
/**
 * @brief jljpeg_stream_src_data_lock_check 查询锁状态
 *
 * @return
 */
/* ------------------------------------------------------------------------------------*/
int jljpeg_stream_src_data_lock_check()
{
    return __this->data_lock;
}

/* ------------------------------------------------------------------------------------*/
/**
 * @brief jljpeg_stream_src_data_copy 输入数据并拷贝到psram缓存
 *
 * @param buf
 * @param size
 *
 * @return
 */
/* ------------------------------------------------------------------------------------*/
int jljpeg_stream_src_data_copy(u8 *buf, int size)
{
    u32 rets;
    __asm__ volatile("%0 = rets":"=r"(rets));
    log_debug("<%s> rets:%x size:%d\n", __func__, rets, size);
    if (jljpeg_stream_src_data_lock_check()) {
        return -1;
    }
    JPEG_STREAM_LOCK;
    if (__this->src_buf) {
        JPEG_STREAM_FREE(__this->src_buf);
    }
    log_debug("%s line:%d buf:%x", __func__, __LINE__, (u32)__this->src_buf);
    __this->src_len = size;
    __this->src_buf = JPEG_STREAM_ALLOC(size);
    memcpy(__this->src_buf, buf, size);
    __this->idx++;
    log_debug("%s line:%d buf:%x", __func__, __LINE__, (u32)__this->src_buf);
    JPEG_STREAM_UNLOCK;
    return 0;
}
/* ------------------------------------------------------------------------------------*/
/**
 * @brief jljpeg_stream_src_data_len_get 获取缓存数据长度
 *
 * @return
 */
/* ------------------------------------------------------------------------------------*/
int jljpeg_stream_src_data_len_get()
{
    u32 rets;
    __asm__ volatile("%0 = rets":"=r"(rets));
    log_debug("<%s> rets:%x len:%d\n", __func__, rets, __this->src_len);
    return __this->src_len;
}
/* ------------------------------------------------------------------------------------*/
/**
 * @brief jljpeg_stream_src_data_get 获取缓存数据地址
 *
 * @return
 */
/* ------------------------------------------------------------------------------------*/
u8 *jljpeg_stream_src_data_get()
{
    u32 rets;
    __asm__ volatile("%0 = rets":"=r"(rets));
    log_debug("<%s> rets:%x buf:0x%x\n", __func__, rets, (u32)__this->src_buf);
    return __this->src_buf;
}

void camera_manager_resume(void);
void camera_manager_suspend(void);

/* ------------------------------------------------------------------------------------*/
/**
 * @brief jljpeg_stream_src_data_save_to_file 将缓存数据写入到文件
 *
 * @param filename 路径
 *
 * @return
 */
/* ------------------------------------------------------------------------------------*/
int jljpeg_stream_src_data_save_to_file(s8 *filename)
{
    int has_photo = 0;
    //100ms 足够编完一帧
    for (int i = 0; i < 10; i++) {
        if (jljpeg_stream_src_data_len_get()) {
            log_debug("%s %d", __func__, __LINE__);
            has_photo  = 1;
            break;
        }
        os_time_dly(1);
    }

    if (!has_photo) {
        return -1;
    }
    /* camera_manager_suspend(); */
    /* wdt_clear(); */
    FILE *fp = fopen(filename, "w+");
    log_debug("%s file_name:%s save ", __func__, filename);
    if (fp) {
        fwrite(jljpeg_stream_src_data_get(), 1, jljpeg_stream_src_data_len_get(), fp);
        u8 name[17] = {0};
        fget_name(fp, name, 16);
        log_info("%s file_name:%s real_name:%s save succ!!!", __func__, filename, name);
        fclose(fp);
    }
    /* camera_manager_resume(); */
    return 0;

}

static void jpeg_task_cb_exit(void *draw_info, void *priv)
{
    /* jpeg_data_lock_unlock(); */
}

/* ------------------------------------------------------------------------------------*/
/**
 * @brief jpeg_image_ram 从ram中加载jpeg显示
 *
 * @param dc
 * @param left
 * @param top
 * @param width
 * @param height
 * @param addr	jpeg地址
 * @param len
 */
/* ------------------------------------------------------------------------------------*/
void jpeg_image_ram(struct draw_context *dc, int left, int top, int width, int height, u8 *addr, int len)
{
    jljpeg_stream_src_data_lock();
    int elm_id = dc->elm->id;
    u8 elm_index = 1;
    u16 idx = __this->idx;
    elm_index = dc->elm_index++;

    int task_id = GPU_TASK_ID(dc->page, idx, elm_index);
    jlgpu_task_basic_param_init(GPU_TASK_IMAGE, task_id, elm_id);
    task_param.priority = dc->elm->prior;
    task_param.image.width = width;
    task_param.image.height = height;
    struct rect image_rect;
    image_rect.left = left;
    image_rect.top = top;
    image_rect.width = task_param.image.width;
    image_rect.height = task_param.image.height;
    // if (!get_rect_align(&dc->rect, &image_rect, dc->hori_align, dc->vert_align)) {
    // 	return ;
    // }

    jlgpu_get_win_rect(&task_param.area);

    task_param.info.offset = (u32)addr;
    task_param.info.last_tab_data_len = len;
    /* printf("%s %d %d", __func__, task_param.info.offset, task_param.info.last_tab_data_len); */
    get_rect_cover(&dc->rect, &image_rect, &task_param.draw);
    /* memcpy(&task_param.draw, &image_rect, sizeof(struct rect)); */
    task_param.task_type = GPU_TASK_DRAW;
    task_param.cb_func = jpeg_draw_cb_gpu;
    /* task_param.cb_exit = jpeg_task_cb_exit; */
    u32 res_hash = 5383;
    res_hash = usr_mmu_hash(res_hash, (u8 *)&task_param.image, sizeof(struct image_file));
    res_hash = usr_mmu_hash(res_hash, (u8 *)&task_param.task_id, 4);
    res_hash = usr_mmu_hash(res_hash, (u8 *)&task_param.element_id, 4);
    void *opj =  jpeg_module_opj_create(&task_param.info, sizeof(struct flash_file_info), res_hash, dc->index, task_param.task_id, task_param.element_id);
    if (opj) {
        pJLGPUTaskUnit_t taskp = jlgpu_get_task_by_id(dc->gpu_task_head, task_param.task_id, task_param.element_id);
        if (taskp && taskp->info.draw_info) {
            jlui_free(taskp->info.draw_info->priv, UI_RAM_SRAM, UI_MODULE_FRAME);
        }
        task_param.priv = jpeg_hd_create_priv(task_param.task_id, task_param.element_id);
        task_param.priv_len = jpeg_hd_get_priv_len();
        task_param.draw_en = 1;
    }
    task_param.clut_format = task_param.image.clut_format;
    task_param.has_clut = task_param.image.has_clut;
    task_param.texture.data = (u8 *)task_param.info.offset;
    task_param.texture.mmu_tab_base = (u8 *)task_param.info.tab;

    jlgpu_update_task_by_id(dc->gpu_task_head, task_param.task_id, task_param.element_id, &task_param);

}
/* ------------------------------------------------------------------------------------*/
/**
 * @brief jpeg_image_file 从文件加载图片，不缓存
 *
 * @param dc
 * @param left
 * @param top
 * @param width
 * @param height
 * @param path		路径
 * @param path_len
 */
/* ------------------------------------------------------------------------------------*/
void jpeg_image_file(struct draw_context *dc, int left, int top, int width, int height, u8 *path, int path_len)
{
    int elm_id = dc->elm->id;
    u8 elm_index = 1;
    u16 idx = __this->idx;
    elm_index = dc->elm_index++;

    int task_id = GPU_TASK_ID(dc->page, idx, elm_index);
    jlgpu_task_basic_param_init(GPU_TASK_IMAGE, task_id, elm_id);
    task_param.priority = dc->elm->prior;
    task_param.image.width = width;
    task_param.image.height = height;
    struct rect image_rect;
    image_rect.left = left;
    image_rect.top = top;
    image_rect.width = task_param.image.width;
    image_rect.height = task_param.image.height;
    // if (!get_rect_align(&dc->rect, &image_rect, dc->hori_align, dc->vert_align)) {
    // 	return ;
    // }

    jlgpu_get_win_rect(&task_param.area);

    /* task_param.info.offset = (u32)addr; */
    /* task_param.info.last_tab_data_len = len; */
    /* printf("%s %d %d", __func__, task_param.info.offset, task_param.info.last_tab_data_len); */
    get_rect_cover(&dc->rect, &image_rect, &task_param.draw);
    /* memcpy(&task_param.draw, &image_rect, sizeof(struct rect)); */
    task_param.task_type = GPU_TASK_DRAW;
    task_param.cb_func = jpeg_draw_cb_gpu;
    /* task_param.cb_exit = jpeg_task_cb_exit; */
    u32 res_hash = 5383;
    res_hash = usr_mmu_hash(res_hash, (u8 *)path, path_len);
    res_hash = usr_mmu_hash(res_hash, (u8 *)&task_param.task_id, 4);
    res_hash = usr_mmu_hash(res_hash, (u8 *)&task_param.element_id, 4);
    void *opj =  jpeg_module_opj_create_file((void *)path, path_len, res_hash, dc->index, task_param.task_id, task_param.element_id);
    if (opj) {
        pJLGPUTaskUnit_t taskp = jlgpu_get_task_by_id(dc->gpu_task_head, task_param.task_id, task_param.element_id);
        if (taskp && taskp->info.draw_info) {
            jlui_free(taskp->info.draw_info->priv, UI_RAM_SRAM, UI_MODULE_FRAME);
        }
        task_param.priv = jpeg_hd_create_priv(task_param.task_id, task_param.element_id);
        task_param.priv_len = jpeg_hd_get_priv_len();
        task_param.draw_en = 1;
    }
    task_param.clut_format = task_param.image.clut_format;
    task_param.has_clut = task_param.image.has_clut;
    task_param.texture.data = (u8 *)task_param.info.offset;
    task_param.texture.mmu_tab_base = (u8 *)task_param.info.tab;

    jlgpu_update_task_by_id(dc->gpu_task_head, task_param.task_id, task_param.element_id, &task_param);

}
extern void cache_gpu_input_jpeg_data_cb(void *priv, void *dst, void *src, int len);
void *cache_gpu_input_jpeg_data_file(void *head, pJLGPUTaskParam_t task_param, u32 check, char *path)
{
    /* 检查是否已经有缓存，如果需要缓存jpg没在cache里，释放原来的，缓存新的 */
    void *cache_addr = gpu_input_stream_cache_check(check);
    //jpeg原始数据，nand使用
    u8 *jpeg_src_data = NULL;
    int jpeg_src_data_len = 0;
    log_debug("@@@@@ cache_addr: 0x%x\n crc:0x%x path:%s", (u32)cache_addr, check, path);
    if (!cache_addr) {//解码jpeg到psram
        log_debug("%s path%s", __func__, path);

        if (strstr(path, "avi") || strstr(path, "AVI")) {
            log_debug("AVI");
            wdt_clear();
#if 0//限制文件大小
            FILE *avi_fp = fopen(path, "r");
            int avi_len  = flen(avi_fp);
            fclose(avi_fp);
            if (avi_len > 80 * 1024 * 1024) {
                return NULL;
            }
#endif

#if 1//只解第一帧数据，内存开销小。code by ds
            int get_first_video_frame_info(const char *filename, int *offset, int *size);
            int frame_pos = 0, frame_len = 0;
            get_first_video_frame_info(path, &frame_pos, &frame_len);
            log_debug("%s %d pos:%d len:%d", __func__, __LINE__, frame_pos, frame_len);
            if (frame_len <= 0) {
                return NULL;
            }
            jpeg_src_data_len  = frame_len;
            if (jpeg_src_data_len <= 0) {
                log_warn("%s %d", __func__, __LINE__);
                return NULL;
            }
            jpeg_src_data  = jpeg_malloc(jpeg_src_data_len);
            FILE *fp = fopen(path, "r");
            if (fp) {
                fseek(fp, frame_pos, SEEK_SET);
                fread(jpeg_src_data, jpeg_src_data_len, 1, fp);
                fclose(fp);
                fp = NULL;
            }
#else//按avilib流程打开文件，内存开销都大
            avi_t *in_fd = AVI_open_input_file(path, 1);
            if (in_fd) {
                int frame_idx = 0;
#if 1//先拿到pos和len，再申请内存
                int AVI_get_video_frame_info(avi_t * AVI, int frame, int *pos, int *len);
                int pos, len = 0;

                AVI_get_video_frame_info(in_fd, frame_idx, &pos, &len);
                AVI_close(in_fd);
                log_debug("%s %d pos:%d len:%d", __func__, __LINE__, pos, len);
                jpeg_src_data_len  = len;
                if (jpeg_src_data_len <= 0) {
                    log_warn("%s %d", __func__, __LINE__);
                    return NULL;
                }
                jpeg_src_data  = jpeg_malloc(jpeg_src_data_len);
                FILE *fp = fopen(path, "r");
                if (fp) {
                    fseek(fp, pos, SEEK_SET);
                    fread(jpeg_src_data, jpeg_src_data_len, 1, fp);
                    fclose(fp);
                    fp = NULL;
                }
#else//先申请内存，再释放文件
                jpeg_src_data_len = AVI_frame_size(in_fd, frame_idx);
                if (jpeg_src_data_len <= 0) {
                    /* AVI_close(in_fd); */
                    log_warn("%s %d", __func__, __LINE__);
                    return NULL;
                }
                jpeg_src_data  = jpeg_malloc(jpeg_src_data_len);
                AVI_set_video_position(in_fd, frame_idx);
                int key_frame;
                AVI_read_frame(in_fd, (char *)jpeg_src_data, &key_frame);
                AVI_close(in_fd);
#endif
            } else {
                return NULL;
            }

#endif
        } else {
            log_debug("JPEG");
            FILE *fp = fopen(path, "r");
            if (fp) {
                //文件头解析的时候比较零散，这里直接将文件拷贝到psram，解完后free
                jpeg_src_data_len = flen(fp);
                jpeg_src_data = jpeg_malloc(jpeg_src_data_len);
                fseek(fp, 0, SEEK_SET);
                fread(jpeg_src_data, jpeg_src_data_len, 1, fp);
                fclose(fp);
            }
        }
        log_debug("%s dat:%x len%d\n", __func__, (u32)jpeg_src_data, jpeg_src_data_len);
        if (jpeg_src_data == NULL) {
            log_error("path:%s not find\n", path);
            return 0;
        }
        jdec_opj *jpg_hd = jpeg_malloc(sizeof(jdec_opj)); //在任务销毁时释放
        jpg_hd->device = jpeg_malloc(sizeof(struct flash_file_info));//设备句柄，这里可以是文件句柄等，搭配input_steam使用

        struct flash_file_info *p_info = (struct flash_file_info *)jpg_hd->device;
        p_info->offset = (u32)jpeg_src_data;
        p_info->tab = NULL;
        p_info->tab_size = 0;
        p_info->last_tab_data_len = jpeg_src_data_len;
        if (jpeg_module_opj_init(jpg_hd)) {//初始化jpeg头
            //异常释放资源
            jpeg_free(jpg_hd->device);
            jljpeg_decode_release(jpg_hd, 0);
            jpeg_free(jpg_hd);
            if (jpeg_src_data) {
                jpeg_free(jpeg_src_data);
            }
            return NULL;
        }
        //cache
        /* u32 jpeg_stride = 2 * (jpg_hd->width + 7) / 8 * 8; */
        /* u32 jpeg_height_align = (jpg_hd->height + 7) / 8 * 8; */
        u32 jpeg_width_align = (jpg_hd->width + 15) / 16  * 16;
        u32 jpeg_stride = 2 * jpeg_width_align;
        u32 jpeg_height_align = (jpg_hd->height + 15) / 16 * 16;
        u32 jpeg_img_len = jpeg_stride  * jpeg_height_align ;
        cache_addr = gpu_input_stream_cache_add_with_callback(jpg_hd, jpg_hd, jpeg_img_len,
                     check, NULL, 0,	cache_gpu_input_jpeg_data_cb);
        //从cache同步到物理介质，后续用nocache直接访问
        /* psram_flush_cache(cache_addr, jpeg_img_len); */
        jpeg_free(jpg_hd->device);
        jljpeg_decode_release(jpg_hd, 1);
        jpeg_free(jpg_hd);
        if (jpeg_src_data) {
            jpeg_free(jpeg_src_data);
        }

    }
    //使用nocache地址访问，提高读效率
    /* task_param->texture.data = psram_cache_2_nocache(cache_addr); */
    task_param->texture.data = (cache_addr);
    //更新任务配置
    task_param->texture.mmu_tab_base = NULL;
    task_param->format = GPU_FORMAT_RGB565;
    task_param->texture.not_compress = 1;
    task_param->texture.adr_mode = 0;
    int width = task_param->image.width;
    int height = task_param->image.height;

    task_param->image.width = (width + 15) / 16 * 16;
    task_param->image.height = (height + 15) / 16 * 16;

    if (cache_addr) {
        gpu_input_stream_cache_vaild_value_set_by_index((u32)cache_addr, 3);
    }
    return task_param->texture.data;
}
/* ------------------------------------------------------------------------------------*/
/**
 * @brief jpeg_image_file_psram 从文件加载jpeg解码到psram
 *
 * @param dc
 * @param left
 * @param top
 * @param width
 * @param height
 * @param path		路径
 * @param path_len
 * @param scale_en	缩放
 * @param scale_f
 *
 * @return
 */
/* ------------------------------------------------------------------------------------*/
int jpeg_image_file_psram(struct draw_context *dc, int left, int top, int width, int height, char *path, int path_len, int scale_en, float scale_f)
{
    int elm_id = dc->elm->id;
    u8 elm_index = 1;
    u16 idx = __this->idx;
    elm_index = dc->elm_index++;

    int task_id = GPU_TASK_ID(dc->page, idx, elm_index);
    jlgpu_task_basic_param_init(GPU_TASK_IMAGE, task_id, elm_id);
    task_param.priority = dc->elm->prior;
    task_param.image.width = width;
    task_param.image.height = height;
    struct rect image_rect;
    image_rect.left = left;
    image_rect.top = top;
    image_rect.width = task_param.image.width;
    image_rect.height = task_param.image.height;

    jlgpu_get_win_rect(&task_param.area);

    get_rect_cover(&dc->rect, &image_rect, &task_param.draw);

    task_param.task_type =  GPU_TASK_IMAGE;
    u32 res_hash = 5383;
    res_hash = usr_mmu_hash(res_hash, (u8 *)path, path_len);
    /* res_hash = usr_mmu_hash(res_hash, (u8 *)&task_param.task_id, 4); */
    /* res_hash = usr_mmu_hash(res_hash, (u8 *)&task_param.element_id, 4); */

    task_param.clut_format = task_param.image.clut_format;
    task_param.has_clut = task_param.image.has_clut;
    task_param.texture.data = (u8 *)task_param.info.offset;
    task_param.texture.mmu_tab_base = (u8 *)task_param.info.tab;

    task_param.texture.data = cache_gpu_input_jpeg_data_file(dc->gpu_task_head, &task_param, res_hash, path);
    if (!task_param.texture.data) {
        log_warn("%s %d", __func__, __LINE__);
        return -1;
    }
    //加个缩放
    task_param.scale_en = scale_en;
    task_param.texture.tran.ratio_w = scale_f;
    task_param.texture.tran.ratio_h = scale_f;
    if (task_param.texture.data) {
        jlgpu_update_task_by_id(dc->gpu_task_head, task_param.task_id, task_param.element_id, &task_param);
    }
    return 0;
}
