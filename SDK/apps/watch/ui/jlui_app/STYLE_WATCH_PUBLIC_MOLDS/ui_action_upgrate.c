#include "app_config.h"
#include "ui/ui_api.h"
#include "jlui/ui.h"
#include "jlui_app/ui_style.h"
#include "app_task.h"
#include "system/timer.h"
#include "device/device.h"
#include "key_event_deal.h"
#include "ui_draw/ui_type.h"
#include "jlui_app/style_upgrade_new.h"
#define LOG_TAG_CONST       UI
#define LOG_TAG     		"[UI-UPGRATE]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"


#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ui_action_upgrate.data.bss")
#pragma data_seg(".ui_action_upgrate.data")
#pragma const_seg(".ui_action_upgrate.text.const")
#pragma code_seg(".ui_action_upgrate.text")
#endif

#ifdef CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE
#if TCFG_UI_ENABLE_UPGRATE

#define STYLE_NAME  JL
REGISTER_UI_STYLE(STYLE_NAME)

extern void set_lcd_keep_open_flag(u8 flag);

//禁止用字库等升级过程中不跟随代码一起擦除的内容，如字库文本,或使用字库的文本

static int upgrade_handler(const char *type, u32 arg)
{
    static char process_record = -1;
    log_info("msg test %s %s %d \n", __FUNCTION__, type, arg);
    struct unumber num;
    ui_progress_set_persent_by_id(STYLE_UPGRADE_ID(UPGRADE_PROGRESS_BCAKUP), 100);
    if (type && !strcmp(type, "process")) {
        if (process_record != arg) {
            process_record = arg;
            ui_progress_set_persent_by_id(STYLE_UPGRADE_ID(UPGRADE_PROGRESS_PERCENT), arg);
            num.type = TYPE_NUM;
            num.numbs = 1;
            num.number[0] = arg;
            ui_number_update_by_id(STYLE_UPGRADE_ID(UPGRADE_PERCENT_VALUE), &num);
        }
    }

    if (type && !strcmp(type, "backup")) {
        ui_progress_set_persent_by_id(STYLE_UPGRADE_ID(UPGRADE_PROGRESS_BCAKUP), 100);
    }

    if (type && !strcmp(type, "file_num")) {
        struct unumber num;
        num.type = TYPE_NUM;
        num.numbs = 2;
        num.number[0] = arg;
        num.number[1] = 3;
        ui_show(STYLE_UPGRADE_ID(UPGRADE_STAGE_VALUE));
        ui_number_update_by_id(STYLE_UPGRADE_ID(UPGRADE_STAGE_VALUE), &num);
    }

    return 0;
}


static const struct uimsg_handl ui_msg_handler[] = {
    { "upgrade",        upgrade_handler     }, //
    { NULL, NULL},      /* 必须以此结尾！ */
};


static int upgrade_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct window *window = (struct window *)ctr;
    log_info("window call = %s %d id =%x \n", __FUNCTION__, __LINE__, window->elm.id);

    switch (e) {
    case ON_CHANGE_INIT:
        if (!ui_auto_shut_down_disable()) {
            set_lcd_keep_open_flag(1);
        }
        ui_register_msg_handler(window->elm.id, ui_msg_handler);
        break;

    case ON_CHANGE_RELEASE:
        set_lcd_keep_open_flag(0);
        ui_auto_shut_down_enable();
        break;

    default:
        return false;
    }
    return false;
}


static int upgrade_ontouch(void *ctr, struct element_touch_event *e)
{
    switch (e->event) {
    case ELM_EVENT_TOUCH_R_MOVE:
    case ELM_EVENT_TOUCH_L_MOVE:
        return true;
    default:
        break;
    }
    return false;
}


REGISTER_UI_EVENT_HANDLER(ID_WINDOW_UPGRADE)
.onchange = upgrade_onchange,
 .onkey    = NULL,
  .ontouch  = upgrade_ontouch,
};


static u8 loading_count = 0;
static void upgrade_constantly_display_cb(void *p)
{
    struct element *elm = ui_core_get_element_by_id(STYLE_UPGRADE_ID(UPGRADE_LAYOUT));
    if (!elm) {
        return;
    }
    loading_count %= 10;
    loading_count++;
    ui_core_redraw(elm);
}

static int upgrade_constantly_display_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct layout *layout = (struct layout *)ctr;
    static u16 constantly_display_timer;
    switch (e) {
    case ON_CHANGE_INIT:
        constantly_display_timer = sys_timer_add(NULL, upgrade_constantly_display_cb, 1000);
        break;
    case ON_CHANGE_SHOW_POST:

        ui_draw_bar(arg, 110, 120, 100, 20, 0xffff, loading_count * 10, 0);
        break;
    case ON_CHANGE_RELEASE:
        if (constantly_display_timer) {
            sys_timer_del(constantly_display_timer);
            constantly_display_timer = 0;
        }
        break;
    default:
        return false;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(STYLE_UPGRADE_ID(UPGRADE_LOADING))
.onchange = upgrade_constantly_display_onchange,
 .onkey    = NULL,
  .ontouch  = NULL,
};

#endif/* #if TCFG_UI_ENABLE_UPGRATE */
#endif/*#ifdef CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE*/



