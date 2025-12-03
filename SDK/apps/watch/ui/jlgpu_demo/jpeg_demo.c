
#include "typedef.h"
#include "rect.h"
#include "res/resfile.h"

#include "dbi.h"
#include "jlgpu_math.h"		// matrix
#include "jlgpu_driver.h"	// gpu driver
#include "ui_resource.h"	// JL UI resource
#include "ui_expand/ui_expand.h"	// macro defined

#include "gpu_port.h"		// module head file
#include "gpu_draw.h"		// custom draw head file

#include "ui_core.h"
#include "ui_measure.h"

#include "ui/lcd/lcd_drive.h"
#include "res/mem_var.h"
#include "football.h"

#include "gpu_demo.h"
#include "jljpeg_decode.h"

#include <math.h>
#if defined JPEG_DEMO_ENABLE && JPEG_DEMO_ENABLE
//===============================================================================================//
//			从文件解码jpeg demo
//			双链表解码jpeg参考 ui_platform.c \ gpu_task.c 相关流程
//===============================================================================================//


#define JPEG_FILE_PATH		"storage/res_nor_mode/C/demo/jpg_test.jpg"


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
        //不支持局部刷新
        return;
    }
    jdec_opj *opj = (jdec_opj *)priv;				//jpeg解码句柄
    opj->matrix = matrix;							//获取变换矩阵
    //计算缩放系数
    float ratio_w = 1.0;
    float ratio_h = 1.0;
    if (opj->matrix) {
        gpu_matrix_t *m = matrix;
        ratio_w = m->m[0][0];//
        ratio_h = m->m[1][1];//
        /* printf("ratio_w: %f, ratio_h: %f\n", ratio_w, ratio_h); */
    }

    /* 计算变换后图片的输出区域 */
    struct rect jpeg_draw_src;
    memcpy(&jpeg_draw_src, src_r, sizeof(struct rect));
    //放大时为中心放大
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
    //计算缩放后的区域*以推屏区域、jpeg缩放区域、控件显示区域的交集作为最终输出，放大时受控件尺寸限制会变成中心放大
    struct rect cover_r;
    get_rect_cover(&cover_r_t, &jpeg_draw_src, &cover_r);
    /* DUMP_RECT(__func__, __LINE__, "cover_r", &cover_r);					//重叠区域 */
    //计算jpeg解码的区域
    int draw_top = cover_r.top - jpeg_draw_src.top;							//绘制的top
    int draw_btm = cover_r.height + draw_top;								//绘制的bottom
    int draw_h = cover_r.height;											//绘制高度
    /* int dec_top = draw_top * ratio_h;										//需要解码的top */
    /* int dec_btm = ceilf((float)draw_btm * ratio_h);							//需要解码的高度，这里需要向上取 */
    /* int dec_h	 = dec_btm - dec_top;										//解码高度 */
    int max_dec_height = (opj->msy << 3) / ratio_h;							//一个mcu分块缩放后可以有效绘制的行数
    int sub_top = 0;
    int sub_height = (draw_h > max_dec_height) ? max_dec_height : draw_h;
    //按mcu快缩放后的有效高度进行分行处理
    while (sub_top + draw_top < draw_btm) {
        int dec_sub_top	= (draw_top + sub_top) * ratio_h;
        int dec_sub_btm = ceilf((float)(draw_top + sub_top + sub_height) * ratio_h);
        int dec_sub_h  = dec_sub_btm - dec_sub_top;
        /* printf("%s draw(%d %d) dec(%d %d)%d maxdec%d sub(%d %d) dec_sub(%d %d)", */
        /* __func__, draw_top, draw_btm, dec_top, dec_btm, dec_h, max_dec_height, sub_top, sub_height, dec_sub_top, dec_sub_btm); */
        struct rect jpeg_dec_r;												/*jpeg解码区域*/
        /* jpeg_dec_r.left = (cover_r.left - src_r->left) * ratio_w; */
        jpeg_dec_r.left = (cover_r.left - jpeg_draw_src.left) * ratio_w;
        jpeg_dec_r.top = dec_sub_top;
        jpeg_dec_r.width = cover_r.width * ratio_w;
        jpeg_dec_r.height = dec_sub_h;
        struct rect block_r;												/*缩放后显示的区域*/
        block_r.top = cover_r.top + sub_top;
        block_r.height = sub_height;
        block_r.left = dst_r->left;
        block_r.width = dst_r->width;
        /* DUMP_RECT(__func__, __LINE__, "block_r", &block_r); */
        /* DUMP_RECT(__func__, __LINE__, "jpeg_dec_r", &jpeg_dec_r); */
        u8 *disp_buf = dst_buf + (block_r.top - dst_r->top) * 2 * lcd_get_screen_width();
        /*启动解码*/
        jljpeg_decode_start(opj, &jpeg_draw_src, &jpeg_dec_r, &cover_r, &block_r, disp_buf);
        sub_top += sub_height;
        sub_height = (sub_top + sub_height + draw_top < draw_btm) ? max_dec_height : draw_btm - draw_top - sub_top;
    }
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

static u32 jlui_jpeg_infunc(struct _jdec_opj *opj, u8 *buf, u32 len)
{
    u32 rets;
    __asm__ volatile("%0 = rets":"=r"(rets));
    printf("%s jpeg_info:%x %x buf:%x rets:0x%x\n", __func__, (int)opj, (int)opj->device, (int)buf, rets);
    if (buf) {
        FILE *fp = (FILE *)opj->device;
        fseek(fp, opj->curr_offset, SEEK_SET);
        return fread(buf, len, 1, fp);
    } else {
        return len;
    }
}
/* ------------------------------------------------------------------------------------*/
/**
 * @brief jlui_free_jpeg_res_cb 解码资源释放回调,由gpu task调用
 *
 * @param p
 */
/* ------------------------------------------------------------------------------------*/
static void __jlui_free_jpeg_res_cb(void *p)
{
    printf("%s 0x%x", __func__, (u32)p);
    jdec_opj *opj = (jdec_opj *)p;
    if (opj) {
        /*释放图片资源*/
        FILE *fp = (FILE *)opj->device;
        fclose(fp);
        /*释放底层*/
        jljpeg_decode_release(opj, 0);
        /*释放句柄*/
        free(opj);
        /*如果全局句柄与当前一致，释放，如果不一致说明已经切换为新的句柄，就不需再清*/
        if (jljpeg_get_decode_hd() == opj) {
            jljpeg_get_decode_hd_clr();
        }
    }
}
/* ------------------------------------------------------------------------------------*/
/**
 * @brief jlui_draw_jpeg_cb_exit 任务销毁时调用，释放jpeg资源
 *
 * @param priv	jpeg句柄
 *
 * @return
 */
/* ------------------------------------------------------------------------------------*/
static int jlui_draw_jpeg_cb_exit(void *draw_info, void *priv)
{
    pJLGPUTaskDraw_t __draw_info = (pJLGPUTaskDraw_t)draw_info;
#if 0//双链表
    /*使用双任务链推屏时，在息屏前或更新资源句柄前释放jpeg句柄，避免双链表冲突*/
    __draw_info->priv = NULL;//由应用层释放
    __draw_info->priv_len = 0;
#else//单链表可在exit释放资源
    __jlui_free_jpeg_res_cb(__draw_info->priv);
    __draw_info->priv = NULL;//由应用层释放
    __draw_info->priv_len = 0;
#endif
    return 0;
}
/* ------------------------------------------------------------------------------------*/
/**
 * @brief jlui_draw_jpeg_cb_exit 任务销毁时调用，释放jpeg资源
 *
 * @param priv	jpeg句柄
 *
 * @return
 */
/* ------------------------------------------------------------------------------------*/
static int jlui_draw_jpeg_res_free(jdec_opj *opj)
{
    /* jdec_opj *opj = (jdec_opj *)jljpeg_get_decode_hd(); */
    if (opj) {
#if 0//双链表
        /*同步到gpu线程释放，避免gpu在使用jpeg资源*/
        jlgpu_scheduler_async_free_jpeg_res(opj);
#else//单链表
        //同一线程使用可以直接释放
        __jlui_free_jpeg_res_cb(opj);
#endif
    }
    return 0;
}
/* ------------------------------------------------------------------------------------*/
/**
 * @brief jlui_draw_jpeg_cb_enter 初始化jpeg资源
 *
 * @param priv		jpeg句柄
 *
 * @return
 */
/* ------------------------------------------------------------------------------------*/
static int jlui_draw_jpeg_cb_enter(void *priv)
{
    jdec_opj *opj = (jdec_opj *)priv;
    /* printf("%s jpeg_info:%x %x", __func__, (int)opj, (int)opj->device); */
    void *dev = opj->device;
    /*初始化句柄*/
    int ret = jljpeg_decode_init(opj);
    if (!ret) {
        /*解析资源*/
        ret = jljpeg_img_info_get(opj, jlui_jpeg_infunc, dev);
        ASSERT(!ret, "jpeg file notsupport err:%x", ret);
    }
    return ret;
}



void jpeg_demo_from_file(pJLGPUTaskHead_t head, int x, int y)
{
    jlgpu_task_basic_param_init(GPU_TASK_DRAW, 0x1, 0);
    task_param.format = GPU_FORMAT_RGB565;

    jlgpu_get_win_rect(&task_param.area);

    task_param.cb_func = jlui_draw_jpeg_cb;
    task_param.cb_exit = jlui_draw_jpeg_cb_exit;
    //判断jpeg句柄是否存在
    jdec_opj *jpg_hd = jljpeg_get_decode_hd();

    if (jpg_hd) {
        struct flash_file_info *jpg_dev = (struct flash_file_info *)jpg_hd->device;
        /* printf("%s hd:0x%x rec:0x%x new:0x%x",__func__,(u32)hd,jpg_dev->offset,task_param.info.offset); */
        //比较是否为同一张图片
        if (task_param.info.offset == jpg_dev->offset) {
            task_param.priv = jpg_hd;
            task_param.priv_len = sizeof(jdec_opj);
            task_param.draw_en = 1;
        } else {
            jlui_draw_jpeg_res_free(jpg_hd);
            goto __new_jpg_hd;
        }
    } else {
__new_jpg_hd:
        task_param.priv = malloc(sizeof(jdec_opj));
        task_param.priv_len = sizeof(jdec_opj);

        task_param.draw_en = 1;
        jpg_hd = task_param.priv;
        FILE *fp = fopen(JPEG_FILE_PATH, "r");
        jpg_hd->device = fp;
        printf("%s dev:%x", __func__, (u32)jpg_hd->device);
        if (jlui_draw_jpeg_cb_enter(jpg_hd)) {
            jlui_draw_jpeg_res_free(jpg_hd);
            return ;
        }
    }
    task_param.draw.left = x;
    task_param.draw.top = y;
    task_param.draw.width = jpg_hd->width;
    task_param.draw.height = jpg_hd->height;

    jlgpu_update_task_by_id(head, task_param.task_id, task_param.element_id, &task_param);

}

void jpeg_demo_test(pJLGPUTaskHead_t head)
{
    //参数化jpeg模块
    jpeg_decode_module_init(NULL, NULL, jdec_to_jlgpu_texture_blend);
    //创建jpeg任务
    jpeg_demo_from_file(head, 20, 20);

}



#endif











