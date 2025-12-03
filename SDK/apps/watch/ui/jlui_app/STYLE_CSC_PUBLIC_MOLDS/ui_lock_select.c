#include "app_config.h"
#include "ui/ui_api.h"
#include "res/resfile.h"
#include "rcsp_extra_flash_opt.h"

#define LOG_TAG_CONST       UI
#define LOG_TAG             "[UI_CS_LOCK_SELECT]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ui_cs_lock_select.data.bss")
#pragma data_seg(".ui_cs_lock_select.data")
#pragma const_seg(".ui_cs_lock_select.text.const")
#pragma code_seg(".ui_cs_lock_select.text")
#endif

#if (defined(CONFIG_UI_STYLE_JL_CSC_PUBLIC_MODLS_ENABLE))
#define STYLE_NAME  JL
REGISTER_UI_STYLE(STYLE_NAME);
extern int watch_set_style(int style);
extern int watch_get_style();
extern char *watch_get_cur_path();
extern int watch_get_items_num();
extern int window_init(int id);
extern char *watch_get_full_path();


struct lock_pic_param {
    UI_RESFILE *view_file;
    struct flash_file_info view_file_info;
};

struct lock_sel_info {
    u8 update_flag;		//更新标志
    struct lock_pic_param pic_param;
};


static struct lock_sel_info *info;


#define __this (info)


static int lock_pic_deinit(struct lock_pic_param *param);


static void set_ui_mode(s8 mode)
{
    char *bgp_cur = watch_bgp_get_related(watch_get_style());
    int index = watch_bgp_get_index(bgp_cur);
    log_info("%s style:%d bgp_nums:%d index:%d mode:%d\n", __func__, watch_get_style(), watch_bgp_get_nums(), index, mode);
    if (mode == 1) {
        if (index + 1 < watch_bgp_get_nums()) {
            index += 1;
            char  *bgp = watch_bgp_get_item_without_path(index);
            log_info("%s new_idx:%d bgp:%s\n", __func__, index, bgp);
            watch_bgp_set_related(bgp,	watch_get_style(),	0);
            __this->update_flag = 1;
        }
    } else if (mode == -1) {
        if (index - 1 >= 0) {
            index -= 1;
            char  *bgp = watch_bgp_get_item_without_path(index);

            log_info("%s new_idx:%d bgp:%s\n", __func__, index, bgp);
            watch_bgp_set_related(bgp,	watch_get_style(),	0);
            __this->update_flag = 1;
        }
    }
}

static int lock_pic_init(struct lock_pic_param *param)
{
    lock_pic_deinit(param);
    u32 flag;
    char *bg_path;
    bg_path = watch_bgp_get_related_path(watch_get_style());
    log_debug("cur watch style %d, bgp_path:%s\n\n\n\n\n", watch_get_style(), bg_path);
    if (bg_path) {
        param->view_file = res_fopen(bg_path, "r");
        if (!param->view_file) {
            log_warn("bgp_not_find:%s\n", bg_path);
            return -1;
        }
        if (UI_DATA_STORE_IN_NORFLASH) {
            ui_res_flash_info_get(&param->view_file_info, bg_path, "res", true);
        }

        res_fread(param->view_file, &flag, sizeof(flag));
        log_debug("flag : 0x%x\n", flag);
    }
    return 0;
}

static int lock_pic_load(struct lock_pic_param *param, struct element *elm, struct draw_context *dc)
{
    if (param->view_file) {
        elm->css.background_image = 1;
        dc->preview.file = param->view_file;
        dc->preview.file_info = &param->view_file_info;
        dc->preview.id = 1;
        dc->preview.page = 0;
    }
    return 0;

}

static int lock_pic_reload(struct lock_pic_param *param)
{
    if (!__this->update_flag) {
        return -1;
    }
    __this->update_flag = 0;

    lock_pic_deinit(param);
    u32 flag;
    char *bg_path;
    bg_path = watch_bgp_get_related_path(watch_get_style());
    log_debug("cur watch style %d, bgp_path:%s\n\n\n\n\n", watch_get_style(), bg_path);
    if (bg_path) {
        param->view_file = res_fopen(bg_path, "r");
        if (!param->view_file) {
            log_warn("bgp_not_find:%s\n", bg_path);
            return -1;
        }
        if (UI_DATA_STORE_IN_NORFLASH) {
            ui_res_flash_info_get(&param->view_file_info, bg_path, "res", true);
        }

        res_fread(param->view_file, &flag, sizeof(flag));
        log_debug("flag : 0x%x\n", flag);
    }
    return 0;
}

static int lock_pic_deinit(struct lock_pic_param *param)
{
    if (param->view_file) {
        ui_res_flash_info_free(&param->view_file_info, "res");
        res_fclose(param->view_file);
        param->view_file = NULL;
    }
    return 0;
}



static int button_ontouch(void *_ctrl, struct element_touch_event *e)
{
    struct ui_pic *pic = (struct ui_pic *)_ctrl;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
    case ELM_EVENT_TOUCH_UP:
        if (!e->move_flag) {
            switch (pic->elm.id) {
            case LOCKSELECT_NEXT:
                set_ui_mode(1);
                break;
            case LOCKSELECT_LAST:
                set_ui_mode(-1);
                break;
            };
            lock_pic_reload(&__this->pic_param);
            ui_core_redraw(pic->elm.parent);
            rcsp_extra_flash_opt_dial_backgroud_nodify();
        }
        break;
    default :
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(LOCKSELECT_NEXT)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = button_ontouch,
};
REGISTER_UI_EVENT_HANDLER(LOCKSELECT_LAST)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = button_ontouch,
};


static int SMALLWIN_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct layout *layout = (struct layout *)_ctrl;
    switch (event) {
    case ON_CHANGE_INIT:
        if (!__this) {
            __this = (struct lock_sel_info *)zalloc(sizeof(struct lock_sel_info));
            ASSERT(__this);
        }
        lock_pic_init(&__this->pic_param);
        ui_core_set_element_ratio(&layout->elm, 0.458, 0.385, 1);
        break;
    case ON_CHANGE_SHOW:
        struct draw_context *dc = (struct draw_context *)arg;
        lock_pic_load(&__this->pic_param, &layout->elm, dc);
        break;
    case ON_CHANGE_RELEASE:
        lock_pic_deinit(&__this->pic_param);
        if (__this) {
            free(__this);
            __this = NULL;
        }
        break;
    default:
        break;
    }
    return false;
}


REGISTER_UI_EVENT_HANDLER(LOCKSMALL)
.onchange = SMALLWIN_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

#endif

