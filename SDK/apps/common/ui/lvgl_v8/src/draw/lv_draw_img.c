#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".lv_ui_core.data.bss")
#pragma data_seg(".lv_ui_core.data")
#pragma const_seg(".lv_ui_core.text.const")
#pragma code_seg(".lv_ui_core.text")
#endif
/**
 * @file lv_draw_img.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_draw_img.h"
#include "lv_img_cache.h"
#include "../hal/lv_hal_disp.h"
#include "../misc/lv_log.h"
#include "../core/lv_refr.h"
#include "../misc/lv_mem.h"
#include "../misc/lv_math.h"
#include "lvgl.h"
#if LV_USE_GPU2D_JL
#include "../draw/jlgpu/lv_draw_jlgpu.h"
#endif

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static lv_res_t /* LV_ATTRIBUTE_FAST_MEM */ decode_and_draw(lv_draw_ctx_t *draw_ctx,
        const lv_draw_img_dsc_t *draw_dsc,
        const lv_area_t *coords, const void *src);

static void show_error(lv_draw_ctx_t *draw_ctx, const lv_area_t *coords, const char *msg);
static void draw_cleanup(_lv_img_cache_entry_t *cache);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_draw_img_dsc_init(lv_draw_img_dsc_t *dsc)
{
    lv_memset_00(dsc, sizeof(lv_draw_img_dsc_t));
    dsc->recolor = lv_color_black();
    dsc->opa = LV_OPA_COVER;
    dsc->zoom = LV_IMG_ZOOM_NONE;
    dsc->antialias = LV_COLOR_DEPTH > 8 ? 1 : 0;
}

/**
 * Draw an image
 * @param coords the coordinates of the image
 * @param mask the image will be drawn only in this area
 * @param src pointer to a lv_color_t array which contains the pixels of the image
 * @param dsc pointer to an initialized `lv_draw_img_dsc_t` variable
 */
void lv_draw_img(lv_draw_ctx_t *draw_ctx, const lv_draw_img_dsc_t *dsc, const lv_area_t *coords, const void *src)
{
    if (src == NULL) {
        LV_LOG_WARN("Image draw: src is NULL");
        show_error(draw_ctx, coords, "No\ndata");
        return;
    }

    if (dsc->opa <= LV_OPA_MIN) {
        return;
    }

    lv_res_t res = LV_RES_INV;

    if (draw_ctx->draw_img) {
        res = draw_ctx->draw_img(draw_ctx, dsc, coords, src);
    }

    if (res != LV_RES_OK) {
        res = decode_and_draw(draw_ctx, dsc, coords, src);
    }

    if (res != LV_RES_OK) {
        LV_LOG_WARN("Image draw error");
        show_error(draw_ctx, coords, "No\ndata");
    }
}

/**
 * Get the pixel size of a color format in bits
 * @param cf a color format (`LV_IMG_CF_...`)
 * @return the pixel size in bits
 */
uint8_t lv_img_cf_get_px_size(lv_img_cf_t cf)
{
    uint8_t px_size = 0;

    switch (cf) {
    case LV_IMG_CF_UNKNOWN:
    case LV_IMG_CF_RAW:
        px_size = 0;
        break;
    case LV_IMG_CF_TRUE_COLOR:
    case LV_IMG_CF_TRUE_COLOR_CHROMA_KEYED:
        px_size = LV_COLOR_SIZE;
        break;
    case LV_IMG_CF_TRUE_COLOR_ALPHA:
        px_size = LV_IMG_PX_SIZE_ALPHA_BYTE << 3;
        break;
    case LV_IMG_CF_INDEXED_1BIT:
    case LV_IMG_CF_ALPHA_1BIT:
        px_size = 1;
        break;
    case LV_IMG_CF_INDEXED_2BIT:
    case LV_IMG_CF_ALPHA_2BIT:
        px_size = 2;
        break;
    case LV_IMG_CF_INDEXED_4BIT:
    case LV_IMG_CF_ALPHA_4BIT:
        px_size = 4;
        break;
    case LV_IMG_CF_INDEXED_8BIT:
    case LV_IMG_CF_ALPHA_8BIT:
        px_size = 8;
        break;
    default:
        px_size = 0;
        break;
    }

    return px_size;
}

/**
 * Check if a color format is chroma keyed or not
 * @param cf a color format (`LV_IMG_CF_...`)
 * @return true: chroma keyed; false: not chroma keyed
 */
bool lv_img_cf_is_chroma_keyed(lv_img_cf_t cf)
{
    bool is_chroma_keyed = false;

    switch (cf) {
    case LV_IMG_CF_TRUE_COLOR_CHROMA_KEYED:
    case LV_IMG_CF_RAW_CHROMA_KEYED:
        is_chroma_keyed = true;
        break;

    default:
        is_chroma_keyed = false;
        break;
    }

    return is_chroma_keyed;
}
bool lv_img_cf_has_alpha_bit(const void *src, lv_img_cf_t cf)
{
#if LV_USE_GPU2D_JL
    return (cf >= LV_IMG_CF_ALPHA_1BIT && cf <= LV_IMG_CF_ALPHA_8BIT);
#else
    return false;
#endif
}
bool lv_img_cf_has_indexed(const void *src, lv_img_cf_t cf)
{
    return (cf >= LV_IMG_CF_INDEXED_1BIT && cf <= LV_IMG_CF_INDEXED_8BIT);
}
/**
 * Check if a color format has alpha channel or not
 * @param cf a color format (`LV_IMG_CF_...`)
 * @return true: has alpha channel; false: doesn't have alpha channel
 */
bool lv_img_cf_has_alpha(lv_img_cf_t cf)
{
    bool has_alpha = false;

    switch (cf) {
    case LV_IMG_CF_TRUE_COLOR_ALPHA:
    case LV_IMG_CF_RAW_ALPHA:
    case LV_IMG_CF_INDEXED_1BIT:
    case LV_IMG_CF_INDEXED_2BIT:
    case LV_IMG_CF_INDEXED_4BIT:
    case LV_IMG_CF_INDEXED_8BIT:
    case LV_IMG_CF_ALPHA_1BIT:
    case LV_IMG_CF_ALPHA_2BIT:
    case LV_IMG_CF_ALPHA_4BIT:
    case LV_IMG_CF_ALPHA_8BIT:
        has_alpha = true;
        break;
    default:
        has_alpha = false;
        break;
    }

    return has_alpha;
}

/**
 * Get the type of an image source
 * @param src pointer to an image source:
 *  - pointer to an 'lv_img_t' variable (image stored internally and compiled into the code)
 *  - a path to a file (e.g. "S:/folder/image.bin")
 *  - or a symbol (e.g. LV_SYMBOL_CLOSE)
 * @return type of the image source LV_IMG_SRC_VARIABLE/FILE/SYMBOL/UNKNOWN
 */
lv_img_src_t lv_img_src_get_type(const void *src)
{
    lv_img_src_t img_src_type = LV_IMG_SRC_UNKNOWN;

    if (src == NULL) {
        return img_src_type;
    }

    if (lv_get_jl_src_variable(src)) {
        img_src_type = LV_IMG_SRC_BIN;
        return img_src_type;
    }

    const uint8_t *u8_p = src;

    /*The first or fourth byte depending on platform endianess shows the type of the image source*/
#if LV_BIG_ENDIAN_SYSTEM
    if (u8_p[3] >= 0x20 && u8_p[3] <= 0x7F) {
#else
    if (u8_p[0] >= 0x20 && u8_p[0] <= 0x7F) {
#endif
        img_src_type = lv_get_img_src_type(src);
    }
#if LV_BIG_ENDIAN_SYSTEM
    else if (u8_p[3] >= 0x80) {
#else
    else if (u8_p[0] >= 0x80) {
#endif
        img_src_type = LV_IMG_SRC_SYMBOL; /*Symbols begins after 0x7F*/
    } else {
        img_src_type = LV_IMG_SRC_VARIABLE; /*`lv_img_dsc_t` is draw to the first byte < 0x20*/
    }

    if (LV_IMG_SRC_UNKNOWN == img_src_type) {
        LV_LOG_WARN("lv_img_src_get_type: unknown image type");
    }

    return img_src_type;
}

void lv_draw_img_decoded(lv_draw_ctx_t *draw_ctx, const lv_draw_img_dsc_t *dsc,
                         const lv_area_t *coords, const uint8_t *map_p, lv_img_cf_t color_format)
{
    if (draw_ctx->draw_img_decoded == NULL) {
        return;
    }

    draw_ctx->draw_img_decoded(draw_ctx, dsc, coords, map_p, color_format);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static lv_res_t LV_ATTRIBUTE_FAST_MEM decode_and_draw(lv_draw_ctx_t *draw_ctx,
        const lv_draw_img_dsc_t *draw_dsc,
        const lv_area_t *coords, const void *src)
{
    if (draw_dsc->opa <= LV_OPA_MIN) {
        return LV_RES_OK;
    }

    _lv_img_cache_entry_t *cdsc = _lv_img_cache_open(src, draw_dsc->recolor, draw_dsc->frame_id);

    if (cdsc == NULL) {
        return LV_RES_INV;
    }


#if LV_USE_GPU_LIST_DRAW
    if (cdsc->dec_dsc.header.cf >= LV_IMG_CF_INDEXED_1BIT && cdsc->dec_dsc.header.cf <= LV_IMG_CF_ALPHA_8BIT) {
        draw_ctx->suport_list_format = 0;
    } else {
        draw_ctx->suport_list_format = 1;
    }
#endif

    lv_img_cf_t cf;
    if (lv_img_cf_has_indexed(src, cdsc->dec_dsc.header.cf)) {
        cf = cdsc->dec_dsc.header.cf;
    } else if (lv_img_cf_has_alpha_bit(src, cdsc->dec_dsc.header.cf)) {
        cf = cdsc->dec_dsc.header.cf;
    } else {
        if (lv_img_cf_is_chroma_keyed(cdsc->dec_dsc.header.cf)) {
            cf = LV_IMG_CF_TRUE_COLOR_CHROMA_KEYED;
        } else if (LV_IMG_CF_ALPHA_8BIT == cdsc->dec_dsc.header.cf) {
            cf = LV_IMG_CF_ALPHA_8BIT;
        } else if (LV_IMG_CF_RGB565A8 == cdsc->dec_dsc.header.cf) {
            cf = LV_IMG_CF_RGB565A8;
        } else if (lv_img_cf_has_alpha(cdsc->dec_dsc.header.cf)) {
            cf = LV_IMG_CF_TRUE_COLOR_ALPHA;
        } else {
            cf = LV_IMG_CF_TRUE_COLOR;
        }
    }
#if (LV_USE_GPU2D_JL == 0)
    if (cf == LV_IMG_CF_ALPHA_8BIT) {
        if (draw_dsc->angle || draw_dsc->zoom != LV_IMG_ZOOM_NONE) {
            /* resume normal method */
            cf = LV_IMG_CF_TRUE_COLOR_ALPHA;
            cdsc->dec_dsc.img_data = NULL;
        }
    }
#endif
    draw_ctx->compress_type = lv_get_compress_type(src);
    draw_ctx->res_block = lv_get_res_block(src);

    if (cdsc->dec_dsc.error_msg != NULL) {
        LV_LOG_WARN("Image draw error");

        show_error(draw_ctx, coords, cdsc->dec_dsc.error_msg);
    }
    /*The decoder could open the image and gave the entire uncompressed image.
     *Just draw it!*/
    else if (cdsc->dec_dsc.img_data) {
        lv_area_t map_area_rot;
        lv_area_copy(&map_area_rot, coords);
        if (draw_dsc->angle || draw_dsc->zoom != LV_IMG_ZOOM_NONE) {
            int32_t w = lv_area_get_width(coords);
            int32_t h = lv_area_get_height(coords);

            _lv_img_buf_get_transformed_area(&map_area_rot, w, h, draw_dsc->angle, draw_dsc->zoom, &draw_dsc->pivot);

            map_area_rot.x1 += coords->x1;
            map_area_rot.y1 += coords->y1;
            map_area_rot.x2 += coords->x1;
            map_area_rot.y2 += coords->y1;
#if LV_USE_GPU2D_JL
            /* lv_draw_jlgpu_set_trans_area(&map_area_rot); */
#endif
        }

        lv_area_t clip_com; /*Common area of mask and coords*/
        bool union_ok;
        union_ok = _lv_area_intersect(&clip_com, draw_ctx->clip_area, &map_area_rot);
        /*Out of mask. There is nothing to draw so the image is drawn successfully.*/
        if (union_ok == false) {
            draw_cleanup(cdsc);
            return LV_RES_OK;
        }

        const lv_area_t *clip_area_ori = draw_ctx->clip_area;
        draw_ctx->clip_area = &clip_com;
        lv_draw_img_decoded(draw_ctx, draw_dsc, coords, cdsc->dec_dsc.img_data, cf);
        draw_ctx->clip_area = clip_area_ori;
    }
    /*The whole uncompressed image is not available. Try to read it line-by-line*/
    else {
        lv_area_t mask_com; /*Common area of mask and coords*/
        bool union_ok;
        int32_t row;
        int32_t column;

        const lv_area_t *clip_area_ori = draw_ctx->clip_area;
        lv_res_t read_res;

#if LV_USE_JLJPEG   // 如果是 .jpg 并且使用了 JL HW JPEG 解码器则执行分块流程，而不执行原本的分行流程
        void *jljpeg_data = cdsc->dec_dsc.decoder->user_data;

        if ((jljpeg_data != NULL) && \
            (lv_jljpeg_check_magic(jljpeg_data) == true)) {
#if (LV_USE_JLJPEG_LINE_DRAW_ENABLE == 1)
            //确认目前图像的解码器时 JL HW JPEG
            uint16_t mx, my, width, height;
            lv_jljpeg_get_img_mcu_info(jljpeg_data, &mx, &my, &width, &height);
            //LV_LOG_WARN("Image mcu mx = %d; my = %d; width = %d; height = %d.", mx, my, width, height);

            lv_area_t mcu_inter_area; //mcu 块在图像内部的相对区域
            mcu_inter_area.x1 = 0;
            mcu_inter_area.y1 = 0;
            mcu_inter_area.x2 = width - 1;
            mcu_inter_area.y2 = my - 1;
            lv_area_t mcu_img_area; //mcu 块在屏幕上的绝对区域
            mcu_img_area.x1 = mcu_inter_area.x1 + coords->x1;
            mcu_img_area.y1 = mcu_inter_area.y1 + coords->y1;
            mcu_img_area.x2 = mcu_inter_area.x2 + coords->x1;
            mcu_img_area.y2 = mcu_inter_area.y2 + coords->y1;

#if LV_USE_GPU_LIST_DRAW
#if  LV_COLOR_DEPTH == 32
#error JL HW JPEG : LV_COLOR_DEPTH is not supported yet
#elif  LV_COLOR_DEPTH == 16 //RGB565
            // 不再申请这个公共的临时buf
            //uint8_t   *buf = lv_mem_buf_get(mx * my * 2);  /* Tempormx * my * 2arily apply for space to store block image data */
            uint8_t *buf;
#elif  LV_COLOR_DEPTH == 8
#error JL HW JPEG : LV_COLOR_DEPTH is not supported yet
#else
#error JL HW JPEG : LV_COLOR_DEPTH is not supported yet
#endif
#else
#if  LV_COLOR_DEPTH == 32
#error JL HW JPEG : LV_COLOR_DEPTH is not supported yet
#elif  LV_COLOR_DEPTH == 16 //RGB565
            uint8_t   *buf = lv_mem_buf_get(width * my * 2);  /* Temporarily apply for space to store block image data */
#elif  LV_COLOR_DEPTH == 8
#error JL HW JPEG : LV_COLOR_DEPTH is not supported yet
#else
#error JL HW JPEG : LV_COLOR_DEPTH is not supported yet
#endif
#endif
            //extern int64_t get_system_us(void);
            //u64 all_start_time = get_system_us();
            //u64 jpeg_add_time = 0;
            for (row = 0; row < height; row += my) {
                //更新变换中点
                lv_point_t pivot_new;
                pivot_new.x = draw_dsc->pivot.x - mcu_inter_area.x1;
                pivot_new.y = draw_dsc->pivot.y - mcu_inter_area.y1;

                //区域变换
                lv_area_t map_area_rot;
                lv_area_copy(&map_area_rot, &mcu_img_area);
                if (draw_dsc->angle || draw_dsc->zoom != LV_IMG_ZOOM_NONE) {
                    int32_t w = lv_area_get_width(&mcu_img_area);
                    int32_t h = lv_area_get_height(&mcu_img_area);

                    _lv_img_buf_get_transformed_area(&map_area_rot, w, h, draw_dsc->angle, draw_dsc->zoom, &pivot_new);

                    // map_area_rot.x1 += mcu_img_area.x1 - 1;
                    // map_area_rot.y1 += mcu_img_area.y1 - 1;
                    // map_area_rot.x2 += mcu_img_area.x1 + 1;
                    // map_area_rot.y2 += mcu_img_area.y1 + 1;
                    map_area_rot.x1 += mcu_img_area.x1;
                    map_area_rot.y1 += mcu_img_area.y1;
                    map_area_rot.x2 += mcu_img_area.x1;
                    map_area_rot.y2 += mcu_img_area.y1;
                }

                //开始确定绘图动作

                //当没有旋转变换时，这个动作可以减少部分不必要的解码时间
                // TODO ：可优化方向，想办法在旋转变换时也能减少无用的解码动作
                if ((!draw_dsc->angle) && (map_area_rot.y1 > clip_area_ori->y2)) {
                    goto img_draw_end;  //该 mcu 块坐标区域以及后续区域都不会和绘制区域有相交的可能，直接跳过本次轮的图像绘制
                }

#if LV_USE_GPU_LIST_DRAW
                lv_area_t mask_mcu;
                union_ok = _lv_area_intersect(&mask_mcu, clip_area_ori, &map_area_rot);
                if (union_ok == false) {
                    read_res = lv_img_decoder_read_line(&cdsc->dec_dsc, mcu_inter_area.x1, mcu_inter_area.y1, 0, NULL);
                    if (read_res != LV_RES_OK) {
                        lv_img_decoder_close(&cdsc->dec_dsc);
                        LV_LOG_WARN("Image draw can't read the jpeg mcu block!!");
                        draw_cleanup(cdsc);
                        draw_ctx->clip_area = clip_area_ori;
                        return LV_RES_INV;
                    }
                    goto mcu_draw_end;  //该 mcu 块坐标区域和绘制区域无相交，但是图像绘制还未结束，只是跳过该 mcu 块的绘制
                } else {
                    buf = lv_gpu_attach_buf_add_node(draw_ctx, NULL, width * my * 2);
                    //u64 tmp_time = get_system_us();
                    read_res = lv_img_decoder_read_line(&cdsc->dec_dsc, mcu_inter_area.x1, mcu_inter_area.y1, 0, buf);
                    if (read_res != LV_RES_OK) {
                        LV_LOG_WARN("Image draw can't read the jpeg mcu block!!");
                        LV_ASSERT_NULL(0);  // 严重异常, 断言
                    }
                    //jpeg_add_time += (get_system_us() - tmp_time);
                }
#else
                lv_area_t mask_mcu;
                union_ok = _lv_area_intersect(&mask_mcu, clip_area_ori, &map_area_rot);
                if (union_ok == false) {
                    read_res = lv_img_decoder_read_line(&cdsc->dec_dsc, mcu_inter_area.x1, mcu_inter_area.y1, 0, NULL);
                    if (read_res != LV_RES_OK) {
                        lv_img_decoder_close(&cdsc->dec_dsc);
                        LV_LOG_WARN("Image draw can't read the jpeg mcu block!!");
                        draw_cleanup(cdsc);
                        draw_ctx->clip_area = clip_area_ori;
                        return LV_RES_INV;
                    }
                    goto mcu_draw_end;  //该 mcu 块坐标区域和绘制区域无相交，但是图像绘制还未结束，只是跳过该 mcu 块的绘制
                } else {
                    read_res = lv_img_decoder_read_line(&cdsc->dec_dsc, mcu_inter_area.x1, mcu_inter_area.y1, 0, buf);
                    if (read_res != LV_RES_OK) {
                        lv_img_decoder_close(&cdsc->dec_dsc);
                        LV_LOG_WARN("Image draw can't read the jpeg mcu block!!");
                        lv_mem_buf_release(buf);
                        draw_cleanup(cdsc);
                        draw_ctx->clip_area = clip_area_ori;
                        return LV_RES_INV;
                    }
                }
#endif

                //printf("mcu_img_area: (x1, y1) = (%d, %d); (x2, y2) = (%d, %d).", mcu_img_area.x1, mcu_img_area.y1, mcu_img_area.x2,mcu_img_area.y2);
                draw_ctx->clip_area = &mask_mcu;
                lv_draw_img_dsc_t draw_dsc_new;
                memcpy(&draw_dsc_new, draw_dsc, sizeof(lv_draw_img_dsc_t));
                // draw_dsc_new.scale_x = draw_dsc_new.scale_y = 256;
                draw_dsc_new.pivot = pivot_new;
                lv_draw_img_decoded(draw_ctx, &draw_dsc_new, &mcu_img_area, buf, cf);
mcu_draw_end:

                //绘制mcu块后的纵向坐标偏移
                mcu_inter_area.x1 = 0;
                mcu_inter_area.x2 = width - 1;
                mcu_img_area.x1 = mcu_inter_area.x1 + coords->x1;
                mcu_img_area.x2 = mcu_inter_area.x2 + coords->x1;

                mcu_inter_area.y1 += my;
                mcu_inter_area.y2 += my;
                mcu_img_area.y1 = mcu_img_area.y2 + 1;
                mcu_img_area.y2 += my;
            }
img_draw_end:
            //u64 all_stop_time = get_system_us();

            //printf("all diff = %llu; jpeg_add_time = %llu.", all_stop_time - all_start_time, jpeg_add_time);

            draw_ctx->clip_area = clip_area_ori;
#if LV_USE_GPU_LIST_DRAW
            // 不需要释放临时的 buf
            // lv_mem_buf_release(buf);
#else
            lv_mem_buf_release(buf);
#endif
            goto exit;
#else   // !LV_USE_JLJPEG_LINE_DRAW_ENABLE
            //确认目前图像的解码器时 JL HW JPEG
            uint16_t mx, my, width, height;
            lv_jljpeg_get_img_mcu_info(jljpeg_data, &mx, &my, &width, &height);
            //LV_LOG_WARN("Image mcu mx = %d; my = %d; width = %d; height = %d.", mx, my, width, height);

            lv_area_t mcu_inter_area; //mcu 块在图像内部的相对区域
            mcu_inter_area.x1 = 0;
            mcu_inter_area.y1 = 0;
            mcu_inter_area.x2 = mx - 1;
            mcu_inter_area.y2 = my - 1;
            lv_area_t mcu_img_area; //mcu 块在屏幕上的绝对区域
            mcu_img_area.x1 = mcu_inter_area.x1 + coords->x1;
            mcu_img_area.y1 = mcu_inter_area.y1 + coords->y1;
            mcu_img_area.x2 = mcu_inter_area.x2 + coords->x1;
            mcu_img_area.y2 = mcu_inter_area.y2 + coords->y1;

#if LV_USE_GPU_LIST_DRAW
#if  LV_COLOR_DEPTH == 32
#error JL HW JPEG : LV_COLOR_DEPTH is not supported yet
#elif  LV_COLOR_DEPTH == 16 //RGB565
            // 不再申请这个公共的临时buf
            //uint8_t   *buf = lv_mem_buf_get(mx * my * 2);  /* Tempormx * my * 2arily apply for space to store block image data */
            uint8_t *buf;
#elif  LV_COLOR_DEPTH == 8
#error JL HW JPEG : LV_COLOR_DEPTH is not supported yet
#else
#error JL HW JPEG : LV_COLOR_DEPTH is not supported yet
#endif
#else
#if  LV_COLOR_DEPTH == 32
#error JL HW JPEG : LV_COLOR_DEPTH is not supported yet
#elif  LV_COLOR_DEPTH == 16 //RGB565
            uint8_t   *buf = lv_mem_buf_get(mx * my * 2);  /* Temporarily apply for space to store block image data */
#elif  LV_COLOR_DEPTH == 8
#error JL HW JPEG : LV_COLOR_DEPTH is not supported yet
#else
#error JL HW JPEG : LV_COLOR_DEPTH is not supported yet
#endif
#endif

            for (row = 0; row < height; row += my) {
                for (column = 0; column < width; column += mx) {
                    //更新变换中点
                    lv_point_t pivot_new;
                    pivot_new.x = draw_dsc->pivot.x - mcu_inter_area.x1;
                    pivot_new.y = draw_dsc->pivot.y - mcu_inter_area.y1;

                    //区域变换
                    lv_area_t map_area_rot;
                    lv_area_copy(&map_area_rot, &mcu_img_area);
                    if (draw_dsc->angle || draw_dsc->zoom != LV_IMG_ZOOM_NONE) {
                        int32_t w = lv_area_get_width(&mcu_img_area);
                        int32_t h = lv_area_get_height(&mcu_img_area);

                        _lv_img_buf_get_transformed_area(&map_area_rot, w, h, draw_dsc->angle, draw_dsc->zoom, &pivot_new);

                        // map_area_rot.x1 += mcu_img_area.x1 - 1;
                        // map_area_rot.y1 += mcu_img_area.y1 - 1;
                        // map_area_rot.x2 += mcu_img_area.x1 + 1;
                        // map_area_rot.y2 += mcu_img_area.y1 + 1;
                        map_area_rot.x1 += mcu_img_area.x1;
                        map_area_rot.y1 += mcu_img_area.y1;
                        map_area_rot.x2 += mcu_img_area.x1;
                        map_area_rot.y2 += mcu_img_area.y1;
                    }

                    //开始确定绘图动作

                    //当没有旋转变换时，这个动作可以减少部分不必要的解码时间
                    // TODO ：可优化方向，想办法在旋转变换时也能减少无用的解码动作
                    if ((!draw_dsc->angle) && (map_area_rot.y1 > clip_area_ori->y2)) {
                        goto img_draw_end;  //该 mcu 块坐标区域以及后续区域都不会和绘制区域有相交的可能，直接跳过本次轮的图像绘制
                    }

#if LV_USE_GPU_LIST_DRAW
                    lv_area_t mask_mcu;
                    union_ok = _lv_area_intersect(&mask_mcu, clip_area_ori, &map_area_rot);
                    if (union_ok == false) {
                        read_res = lv_img_decoder_read_line(&cdsc->dec_dsc, mcu_inter_area.x1, mcu_inter_area.y1, 0, NULL);
                        if (read_res != LV_RES_OK) {
                            lv_img_decoder_close(&cdsc->dec_dsc);
                            LV_LOG_WARN("Image draw can't read the jpeg mcu block!!");
                            draw_cleanup(cdsc);
                            draw_ctx->clip_area = clip_area_ori;
                            return LV_RES_INV;
                        }
                        goto mcu_draw_end;  //该 mcu 块坐标区域和绘制区域无相交，但是图像绘制还未结束，只是跳过该 mcu 块的绘制
                    } else {
                        buf = lv_gpu_attach_buf_add_node(draw_ctx, NULL, mx * my * 2);
                        read_res = lv_img_decoder_read_line(&cdsc->dec_dsc, mcu_inter_area.x1, mcu_inter_area.y1, 0, buf);
                        if (read_res != LV_RES_OK) {
                            LV_LOG_WARN("Image draw can't read the jpeg mcu block!!");
                            LV_ASSERT_NULL(0);  // 严重异常, 断言
                        }
                    }
#else
                    read_res = lv_img_decoder_read_line(&cdsc->dec_dsc, mcu_inter_area.x1, mcu_inter_area.y1, 0, buf);
                    if (read_res != LV_RES_OK) {
                        lv_img_decoder_close(&cdsc->dec_dsc);
                        LV_LOG_WARN("Image draw can't read the jpeg mcu block!!");
                        lv_mem_buf_release(buf);
                        draw_cleanup(cdsc);
                        draw_ctx->clip_area = clip_area_ori;
                        return LV_RES_INV;
                    }

                    lv_area_t mask_mcu;
                    union_ok = _lv_area_intersect(&mask_mcu, clip_area_ori, &map_area_rot);
                    if (union_ok == false) {
                        goto mcu_draw_end;  //该 mcu 块坐标区域和绘制区域无相交，但是图像绘制还未结束，只是跳过该 mcu 块的绘制
                    }
#endif

                    draw_ctx->clip_area = &mask_mcu;
                    lv_draw_img_dsc_t draw_dsc_new;
                    memcpy(&draw_dsc_new, draw_dsc, sizeof(lv_draw_img_dsc_t));
                    // draw_dsc_new.scale_x = draw_dsc_new.scale_y = 256;
                    draw_dsc_new.pivot = pivot_new;
                    lv_draw_img_decoded(draw_ctx, &draw_dsc_new, &mcu_img_area, buf, cf);

mcu_draw_end:
                    //绘制mcu块后的横向坐标偏移
                    mcu_img_area.x1 = mcu_img_area.x2 + 1;
                    mcu_img_area.x2 += mx;
                    mcu_inter_area.x1 += mx;
                    mcu_inter_area.x2 += mx;
                }
                //绘制mcu块后的纵向坐标偏移
                mcu_inter_area.x1 = 0;
                mcu_inter_area.x2 = mx - 1;
                mcu_img_area.x1 = mcu_inter_area.x1 + coords->x1;
                mcu_img_area.x2 = mcu_inter_area.x2 + coords->x1;

                mcu_inter_area.y1 += my;
                mcu_inter_area.y2 += my;
                mcu_img_area.y1 = mcu_img_area.y2 + 1;
                mcu_img_area.y2 += my;
            }
img_draw_end:
            draw_ctx->clip_area = clip_area_ori;
#if LV_USE_GPU_LIST_DRAW
            // 不需要释放临时的 buf
            // lv_mem_buf_release(buf);
#else
            lv_mem_buf_release(buf);
#endif
            goto exit;
#endif
        }
#endif

        union_ok = _lv_area_intersect(&mask_com, draw_ctx->clip_area, coords);
        /*Out of mask. There is nothing to draw so the image is drawn successfully.*/
        if (union_ok == false) {
            draw_cleanup(cdsc);
            return LV_RES_OK;
        }

        int32_t width = lv_area_get_width(&mask_com);

        uint8_t   *buf = lv_mem_buf_get(lv_area_get_width(&mask_com) *
                                        LV_IMG_PX_SIZE_ALPHA_BYTE);  /*+1 because of the possible alpha byte*/

        lv_area_t line;
        lv_area_copy(&line, &mask_com);
        lv_area_set_height(&line, 1);
        int32_t x = mask_com.x1 - coords->x1;
        int32_t y = mask_com.y1 - coords->y1;
        for (row = mask_com.y1; row <= mask_com.y2; row++) {
            lv_area_t mask_line;
            union_ok = _lv_area_intersect(&mask_line, clip_area_ori, &line);
            if (union_ok == false) {
                continue;
            }

            read_res = lv_img_decoder_read_line(&cdsc->dec_dsc, x, y, width, buf);
            if (read_res != LV_RES_OK) {
                lv_img_decoder_close(&cdsc->dec_dsc);
                LV_LOG_WARN("Image draw can't read the line");
                lv_mem_buf_release(buf);
                draw_cleanup(cdsc);
                draw_ctx->clip_area = clip_area_ori;
                return LV_RES_INV;
            }

            draw_ctx->clip_area = &mask_line;
            lv_draw_img_decoded(draw_ctx, draw_dsc, &line, buf, cf);
            line.y1++;
            line.y2++;
            y++;
        }
        draw_ctx->clip_area = clip_area_ori;
        lv_mem_buf_release(buf);
    }

exit:
    draw_cleanup(cdsc);
    return LV_RES_OK;
}

static void show_error(lv_draw_ctx_t *draw_ctx, const lv_area_t *coords, const char *msg)
{
    lv_draw_rect_dsc_t rect_dsc;
    lv_draw_rect_dsc_init(&rect_dsc);
    rect_dsc.bg_color = lv_color_white();
    lv_draw_rect(draw_ctx, &rect_dsc, coords);

    lv_draw_label_dsc_t label_dsc;
    lv_draw_label_dsc_init(&label_dsc);
    lv_draw_label(draw_ctx, &label_dsc, coords, msg, NULL);
}

static void draw_cleanup(_lv_img_cache_entry_t *cache)
{
    /*Automatically close images with no caching*/
#if LV_IMG_CACHE_DEF_SIZE == 0
    lv_img_decoder_close(&cache->dec_dsc);
#else
    LV_UNUSED(cache);
#endif
}
