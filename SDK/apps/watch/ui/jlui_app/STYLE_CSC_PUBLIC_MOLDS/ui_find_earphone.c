#include "app_config.h"
#include "ui/ui_api.h"
#include "events_adapter.h"
#include "smartbox_info_manager.h"
#include "smartbox_user_app.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ui_cs_find_earphone.data.bss")
#pragma data_seg(".ui_cs_find_earphone.data")
#pragma const_seg(".ui_cs_find_earphone.text.const")
#pragma code_seg(".ui_cs_find_earphone.text")
#endif

#define LOG_TAG_CONST       UI
#define LOG_TAG     		"[UI_FIND_EARPHONE]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"

#if (defined(CONFIG_UI_STYLE_JL_CSC_PUBLIC_MODLS_ENABLE))
#if(defined TCFG_UI_FIND_EARPHONE_ENABLE && TCFG_UI_FIND_EARPHONE_ENABLE)
#define STYLE_NAME  JL

enum {
    FIND_EARPHON_CTRL = 0,
    EARPHONE_FINDING,
};

enum {
    L_EAR_FIND,
    R_EAR_FIND,
    NO_EAR_FIND,
};

enum {
    L_EAR_FIND_START,
    R_EAR_FIND_START,
    L_EAR_FIND_CLOSE,
    R_EAR_FIND_CLOSE,
};


typedef struct layout_maneger {
    int index ;
    int id;
    int (*callback)(void *arg);
} layout_maneger_t;


const static layout_maneger_t ui_find_earphone_layout_manager[] = {
    {FIND_EARPHON_CTRL,    FIND_EAR_CTRL,     NULL},
    {EARPHONE_FINDING,    FINDING,     NULL},
};

static u8 touch_earphone_state; /*记录当前点击查找哪个耳机*/


/*因为现有的协议无法控制查找耳机进行单独开关,所以做了兼容*/
static void find_earphone_by_ble(int command)
{
    switch (command) {
    case L_EAR_FIND_START:
        sbox_left_finding_set(TRUE);

        /*CMD_FIND_MY_L当前耳机处理，在左耳查找时候，会把右耳查找关掉*/
        if (sbox_right_finding_get()) {
            custom_client_send_find_earphone(CMD_FIND_MY_LR);
        } else {
            custom_client_send_find_earphone(CMD_FIND_MY_L);
        }
        break;
    case R_EAR_FIND_START:
        sbox_right_finding_set(TRUE);

        /*CMD_FIND_MY_R当前耳机处理，在右耳查找时候，会把左耳查找关掉*/
        if (sbox_left_finding_get()) {
            custom_client_send_find_earphone(CMD_FIND_MY_LR);
        } else {
            custom_client_send_find_earphone(CMD_FIND_MY_R);
        }
        break;
    case L_EAR_FIND_CLOSE:
        custom_client_send_find_earphone(CMD_FIND_CLOSE);
        sbox_left_finding_set(FALSE);

        if (sbox_right_finding_get()) {
            custom_client_send_find_earphone(CMD_FIND_MY_R);
        }
        break;
    case R_EAR_FIND_CLOSE:
        custom_client_send_find_earphone(CMD_FIND_CLOSE);
        sbox_right_finding_set(FALSE);

        if (sbox_left_finding_get()) {
            custom_client_send_find_earphone(CMD_FIND_MY_L);
        }
        break;
    default:
        break;
    }
}


static void ui_find_earphone_switch_layout(int layout_index, int redraw)
{
    struct element *elm = NULL;
    for (int i = 0 ; i < sizeof(ui_find_earphone_layout_manager) / sizeof(ui_find_earphone_layout_manager[0]) ; i++) {
        elm = ui_core_get_element_by_id(ui_find_earphone_layout_manager[i].id);
        if (ui_find_earphone_layout_manager[i].index == layout_index) {
            if (elm) {
                elm->css.invisible = 0;
            }
        } else {
            if (elm) {
                elm->css.invisible = 1;
            }
        }
    }
    if (redraw) {
        if (elm && elm->parent) {
            ui_core_redraw(elm->parent);
        } else if (elm) {
            ui_core_redraw(elm);
        }
    }
}


static int ui_find_earphone_child_onchang(void *ctr, enum element_change_event e, void *arg)
{
    struct element *elm = (struct element *)ctr;
    int type = ui_id2type(elm->id);
    switch (type) {
    case CTRL_TYPE_PIC:
        if (e == ON_CHANGE_INIT || e == ON_CHANGE_SHOW_PROBE) {
            struct ui_pic *pic = (struct ui_pic *)ctr;
            if (!strcmp(pic->source, "l_find")) {
                if (sbox_left_finding_get()) {
                    if (sbox_ble_connect_flag_get()) {
                        ui_pic_set_image_index(pic, 2);
                    } else {
                        ui_pic_set_image_index(pic, 0);
                    }
                } else {
                    if (sbox_ble_connect_flag_get()) {
                        ui_pic_set_image_index(pic, 1);
                    } else {
                        ui_pic_set_image_index(pic, 0);
                    }
                }
            } else if (!strcmp(pic->source, "r_find")) {
                if (sbox_right_finding_get()) {
                    if (sbox_ble_connect_flag_get()) {
                        ui_pic_set_image_index(pic, 2);
                    } else {
                        ui_pic_set_image_index(pic, 0);
                    }
                } else {
                    if (sbox_ble_connect_flag_get()) {
                        ui_pic_set_image_index(pic, 1);
                    } else {
                        ui_pic_set_image_index(pic, 0);
                    }
                }
            }
        }
        break;
    default:
        break;
    }

    return 0;
}


// 子控件刷新逻辑操作接口
static int pic_ontouch(void *_ctrl, struct element_touch_event *e)
{
    struct ui_pic *pic = (struct ui_pic *)_ctrl;

    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
    case ELM_EVENT_TOUCH_UP:
        if (e->move_flag) {
            break;
        }

        if (sbox_ble_connect_flag_get() == 0) {
            return 0;
        }

        if (!strcmp(pic->source, "l_find")) {
            if (sbox_left_finding_get()) {
                find_earphone_by_ble(L_EAR_FIND_CLOSE);
                ui_pic_show_image_by_id(pic->elm.id, 1);
            } else {
                touch_earphone_state = L_EAR_FIND;
                ui_find_earphone_switch_layout(EARPHONE_FINDING, 1);
            }
        } else if (!strcmp(pic->source, "r_find")) {
            if (sbox_right_finding_get()) {
                find_earphone_by_ble(R_EAR_FIND_CLOSE);
                ui_pic_show_image_by_id(pic->elm.id, 1);
            } else {
                touch_earphone_state = R_EAR_FIND;
                ui_find_earphone_switch_layout(EARPHONE_FINDING, 1);
            }
        } else if (!strcmp(pic->source, "ex_find")) {
            touch_earphone_state = NO_EAR_FIND;
            ui_find_earphone_switch_layout(FIND_EARPHON_CTRL, 1);
        } else if (!strcmp(pic->source, "st_find")) {

            if (touch_earphone_state == L_EAR_FIND) {
                find_earphone_by_ble(L_EAR_FIND_START);
            } else if (touch_earphone_state == R_EAR_FIND) {
                find_earphone_by_ble(R_EAR_FIND_START);
            } else {
                log_error("%s touch_earphone_state %d is error!", __func__, touch_earphone_state);
            }

            touch_earphone_state = NO_EAR_FIND;
            ui_find_earphone_switch_layout(FIND_EARPHON_CTRL, 1);
        }

        return true;
    }
    return false;
}

static int ui_find_earphone_child_ontouch(void *_ctrl, struct element_touch_event *e)
{
    struct element *elm = (struct element *)_ctrl;
    int ret = false;
    int type = ui_id2type(elm->id);
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        switch (type) {
        case CTRL_TYPE_PIC:
            ret = pic_ontouch(elm, e);
        }
        return ret;
    case ELM_EVENT_TOUCH_UP:
        switch (type) {
        case CTRL_TYPE_PIC:
            ret = pic_ontouch(elm, e);
        }
        return ret;
    default:
        break;
    }

    return ret;
}

static int ui_find_earphone_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct layout *layout = (struct layout *)_ctrl;

    switch (event) {
    case ON_CHANGE_INIT_PROBE:
        ui_set_default_handler(&layout->elm, ui_find_earphone_child_ontouch, NULL, ui_find_earphone_child_onchang);
        break;
    case ON_CHANGE_RELEASE:
        touch_earphone_state = NO_EAR_FIND;
        break;
    default:
        break;
    }

    return 0;
}

REGISTER_UI_EVENT_HANDLER(FIND_EARPHON_LAYOUT)
.onchange = ui_find_earphone_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};


void ui_find_earphone_bt_status_update(void)
{
    u8 ble_flag = sbox_ble_connect_flag_get();

    log_info("%s ble_flag:%d", __func__, ble_flag);
    ui_find_earphone_switch_layout(FIND_EARPHON_CTRL, 0);
    struct ui_pic *pic = (struct ui_pic *)ui_core_get_element_by_id(FIND_6);
    if (pic) {
        ui_pic_set_image_index(pic, ble_flag);
    }
    pic = (struct ui_pic *)ui_core_get_element_by_id(FIND_7);
    if (pic) {
        ui_pic_set_image_index(pic, ble_flag);
    }
}


#endif
#endif
