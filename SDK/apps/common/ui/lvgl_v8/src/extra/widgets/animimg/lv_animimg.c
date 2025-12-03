#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".lv_ui_core.data.bss")
#pragma data_seg(".lv_ui_core.data")
#pragma const_seg(".lv_ui_core.text.const")
#pragma code_seg(".lv_ui_core.text")
#endif
/**
 * @file lv_animimg.c
 *
 */

/*********************
 *      INCLUDES
 *********************/

#include "lvgl.h"
#include "lv_animimg.h"
#if LV_USE_ANIMIMG != 0

/*Testing of dependencies*/
#if LV_USE_IMG == 0
#error "lv_animimg: lv_img is required. Enable it in lv_conf.h (LV_USE_IMG  1) "
#endif

#include "../../../misc/lv_assert.h"
#include "../../../draw/lv_img_decoder.h"
#include "../../../misc/lv_fs.h"
#include "../../../misc/lv_txt.h"
#include "../../../misc/lv_math.h"
#include "../../../misc/lv_log.h"
#include "../../../misc/lv_anim.h"

/*********************
 *      DEFINES
 *********************/
#define LV_OBJX_NAME "lv_animimg"

#define MY_CLASS &lv_animimg_class

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void index_change(lv_obj_t *obj, int32_t index);
static void lv_animimg_constructor(const lv_obj_class_t *class_p, lv_obj_t *obj);
static void lv_animing_destructor(const lv_obj_class_t *class_p, lv_obj_t *obj);

/**********************
 *  STATIC VARIABLES
 **********************/
const lv_obj_class_t lv_animimg_class = {
    .constructor_cb = lv_animimg_constructor,
    .destructor_cb = lv_animing_destructor,
    .instance_size = sizeof(lv_animimg_t),
    .base_class = &lv_img_class
};

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lv_obj_t *lv_animimg_create(lv_obj_t *parent)
{
    LV_LOG_INFO("begin");
    lv_obj_t *obj = lv_obj_class_create_obj(&lv_animimg_class, parent);
    lv_obj_class_init_obj(obj);
    return obj;
}

void lv_animimg_set_src(lv_obj_t *obj, const void *dsc[], uint8_t num)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_animimg_t *animimg = (lv_animimg_t *)obj;
    if (animimg->dsc_type == LV_IMG_SRC_FILE) {
        lv_img_set_src(obj, NULL);
        lv_img_cache_invalidate_src(&animimg->imgdsc);
        lv_mem_free((void *)animimg->imgdsc.data);
        lv_mem_free(animimg->video_file_path);
        lv_memset_00(&animimg->imgdsc, sizeof(lv_img_dsc_t));
    }
    animimg->dsc_type = LV_IMG_SRC_VARIABLE;
    animimg->dsc = dsc;
    animimg->pic_count = num;
    lv_anim_set_values(&animimg->anim, 0, num);
}
lv_res_t lv_animimg_set_video_file_src(lv_obj_t *obj, const char *video_file_path)
{
    lv_animimg_t *animimg = (lv_animimg_t *)obj;
    if (animimg->dsc_type == LV_IMG_SRC_FILE) {
        lv_img_set_src(obj, NULL);
        lv_img_cache_invalidate_src(&animimg->imgdsc);
        lv_mem_free(animimg->video_file_path);
        lv_mem_free((void *)animimg->imgdsc.data);
        lv_memset_00(&animimg->imgdsc, sizeof(lv_img_dsc_t));
    }
    uint32_t pic_count = 0;
    if (lv_video_get_info(video_file_path, &pic_count, &animimg->video_max_data_decompress)) {
        animimg->pic_count = (int8_t)pic_count;
        animimg->dsc_type = LV_IMG_SRC_FILE;
        animimg->video_file_path = lv_mem_alloc(strlen(video_file_path) + 1);
        lv_memset_00(animimg->video_file_path, strlen(video_file_path) + 1);
        lv_memcpy(animimg->video_file_path, video_file_path, strlen(video_file_path));
        lv_anim_set_values(&animimg->anim, 0, animimg->pic_count);
        return LV_RES_OK;
    } else {
        animimg->dsc_type = LV_IMG_SRC_UNKNOWN;
        LV_LOG_WARN("lv_animimg: failed to get video info: %s", video_file_path);
        return LV_RES_INV;
    }

}

void lv_animimg_start(lv_obj_t *obj)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_animimg_t *animimg = (lv_animimg_t *)obj;
    lv_anim_start(&animimg->anim);
}

/*=====================
 * Setter functions
 *====================*/

void lv_animimg_set_duration(lv_obj_t *obj, uint32_t duration)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_animimg_t *animimg = (lv_animimg_t *)obj;
    lv_anim_set_time(&animimg->anim, duration);
    lv_anim_set_playback_delay(&animimg->anim, duration);
}

void lv_animimg_set_repeat_count(lv_obj_t *obj, uint16_t count)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_animimg_t *animimg = (lv_animimg_t *)obj;
    lv_anim_set_repeat_count(&animimg->anim, count);
}

/*=====================
 * Getter functions
 *====================*/

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void lv_animimg_constructor(const lv_obj_class_t *class_p, lv_obj_t *obj)
{
    LV_TRACE_OBJ_CREATE("begin");

    LV_UNUSED(class_p);
    lv_animimg_t *animimg = (lv_animimg_t *)obj;

    animimg->dsc = NULL;
    animimg->dsc_type = LV_IMG_SRC_UNKNOWN;
    animimg->pic_count = -1;
    animimg->video_file_path = NULL;
    animimg->video_max_data_decompress = 0;
    memset(&animimg->imgdsc, 0, sizeof(lv_img_dsc_t));
    //initial animation
    lv_anim_init(&animimg->anim);
    lv_anim_set_var(&animimg->anim, obj);
    lv_anim_set_time(&animimg->anim, 30);
    lv_anim_set_exec_cb(&animimg->anim, (lv_anim_exec_xcb_t)index_change);
    lv_anim_set_values(&animimg->anim, 0, 1);
    lv_anim_set_repeat_count(&animimg->anim, LV_ANIM_REPEAT_INFINITE);
}

static void lv_animing_destructor(const lv_obj_class_t *class_p, lv_obj_t *obj)
{
    LV_TRACE_OBJ_CREATE("begin");
    LV_UNUSED(class_p);
    lv_animimg_t *animimg = (lv_animimg_t *)obj;
    if (animimg->dsc_type == LV_IMG_SRC_FILE) {
        lv_img_cache_invalidate_src(&animimg->imgdsc);
        lv_mem_free((void *)animimg->imgdsc.data);
        lv_mem_free(animimg->video_file_path);
    }
}
static void index_change(lv_obj_t *obj, int32_t index)
{
    lv_coord_t idx;
    lv_animimg_t *animimg = (lv_animimg_t *)obj;

    idx = index % animimg->pic_count;

    if (animimg->dsc_type == LV_IMG_SRC_FILE) {
        idx = animimg->imgdsc.data_size == 0 ? 0 : idx;


#if LV_USE_LZ4_INTERNAL
        if (lv_video_get_image(animimg->video_file_path, &animimg->imgdsc, animimg->pic_count, idx,
                               animimg->video_max_data_decompress) == LV_RES_OK) {
            lv_img_set_src(obj, &animimg->imgdsc);
        } else {
            LV_LOG_WARN("lv_animimg: failed to get image from video");
        }
#else

        LV_LOG_ERROR("lv_animimg: failed to get image from video");
        LV_LOG_ERROR("lv_animimg: please open LV_USE_LZ4_INTERNAL in lv_conf.h");
#endif


    } else {
        lv_img_set_src(obj, animimg->dsc[idx]);
    }
}

#endif
