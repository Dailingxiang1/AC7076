#include "app_config.h"
#include "jlui_app/res_config.h"
#include "ui/ui_api.h"
#include "smartbox_user_app.h"
#include "events_adapter.h"
#include "rtc.h"
#include "ui_bg_manage.h"
#include "system/timer.h"
#include "smartbox_info_manager.h"

#define LOG_TAG_CONST       UI
#define LOG_TAG             "[UI-ACTION]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"

#define STYLE_NAME          JL
#define TOPBAR_TAB_PATH     MODE_PATH "JL/JL.tab"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ui_topbar.data.bss")
#pragma data_seg(".ui_topbar.data")
#pragma const_seg(".ui_topbar.text.const")
#pragma code_seg(".ui_topbar.text")
#endif

#if (defined(CONFIG_UI_STYLE_JL_CSC_PUBLIC_MODLS_ENABLE))

typedef struct {
    uint32_t control_container_id; // 控制容器ID
    uint32_t control_id;           // 控件ID
    bool is_in_use;                // 是否正在使用
} control_id_using_t;

// 定义两个控制ID用于交替加载
#define CONTROL_ID_COUNT 2
static control_id_using_t control_id_using[CONTROL_ID_COUNT] = {0};

static const uint32_t available_control_ids[CONTROL_ID_COUNT] = {
    UNIFORM_TOPBAR_LAYOUT,
    UNIFORM_TOPBAR_LAYOUT_1
};

static uint8_t current_control_id_index = 0;

static int topbar_onchange(void *ctrl, enum element_change_event event, void *arg);
static int topbar_child_onchange(void *ctrl, enum element_change_event event, void *arg);



#define ADD_TOP(id) \
    REGISTER_UI_EVENT_HANDLER(id)\
        .ontouch = NULL, \
        .onkey = NULL, \
        .onchange = topbar_onchange \
    };

#define REGISTER_TOPBAR_CHILD(id) \
    REGISTER_UI_EVENT_HANDLER(id)\
        .onchange = topbar_child_onchange, \
        .onkey = NULL, \
        .ontouch = NULL \
    };

// 注册多个TOPBAR事件处理器
ADD_TOP(MUSIC_LAYOUT);
ADD_TOP(DENOISE_LAYOUT);
ADD_TOP(EQUALIZER_LAYOUT);
ADD_TOP(BRIGHTNESS_VALUE_LAYOUT);
ADD_TOP(TIKTOK_LAYOUT);
ADD_TOP(PHOTOGRAPH_LAYOUT);
ADD_TOP(VOLUME_LAYOUT);
ADD_TOP(LOCKSELECT_LAYOUT);
ADD_TOP(LANGUAGE_SELECT_LAYOUT);
ADD_TOP(FIND_EARPHON_LAY);
ADD_TOP(SET_TIME_MAIN);
ADD_TOP(ALARM_CLOCK_LAYOUT);


void topbar_elm_status_update(struct element *elm)
{
    if (!elm) {
        log_error("%s elm is NULL", __func__);
        return;
    }

    int type = ui_id2type(elm->id);

    switch (type) {
    case CTRL_TYPE_NUMBER: {
        struct unumber num = {0};
        struct ui_number *number = (struct ui_number *)elm;

        num.type = TYPE_NUM;
        num.number[0] = 100;

        if (strncmp(number->source, "cap_l", 6) == 0) {
            num.number[0] = sbox_battery_left_get();
        } else if (strncmp(number->source, "cap_r", 6) == 0) {
            num.number[0] = sbox_battery_right_get();
        } else if (strncmp(number->source, "cap_c", 6) == 0) {
            num.number[0] = sbox_battery_box_get();
            if (num.number[0] > 100) {
                num.number[0] = 100;
            }
        } else {
            break;
        }
        ui_number_update(number, &num);
        break;
    }

    case CTRL_TYPE_TIME: {
        struct sys_time current_time = {0};
        struct ui_time *time_ctrl = (struct ui_time *)elm;

        if (strcmp(time_ctrl->source, "rtc") == 0) {
            rtc_read_time(&current_time);
            log_debug("Time - Day: %d, Hour: %d, Min: %d, Sec: %d\n",
                      current_time.day, current_time.hour, current_time.min, current_time.sec);
            ui_time_update(time_ctrl, (struct utime *)&current_time);
        }

        break;
    }

    case CTRL_TYPE_PIC: {
        struct ui_pic *pic_ctrl = (struct ui_pic *)elm;
        int ble_sta_index = 0;
        if (sbox_ble_connect_flag_get() && sbox_bt_connect_flag_get()) {
            ble_sta_index = 2;
        } else if (sbox_ble_connect_flag_get()) {
            ble_sta_index = 1;
        }

        if (!strcmp(pic_ctrl->source, "ble_sta")) {
            ui_pic_set_image_index(pic_ctrl, ble_sta_index);
        } else if (!strcmp(pic_ctrl->source, "l_sta")) {
            ui_pic_set_image_index(pic_ctrl, sbox_left_charging_get());
        } else if (!strcmp(pic_ctrl->source, "r_sta")) {
            ui_pic_set_image_index(pic_ctrl, sbox_right_charging_get());
        } else if (!strcmp(pic_ctrl->source, "box_sta")) {
            ui_pic_set_image_index(pic_ctrl, sbox_box_charging_get());
        }

        break;
    }

    default:
        break;
    }
}

int window_bt_status_update_cb(const char *type, uint32_t argc)
{
    struct element *p;
    /*为了刷新两个页面同时存在时候，遍历刷新*/
    list_for_each_child_element(p, ui_core_get_root()) {
        /*刷新页面中需要根据蓝牙状态变化的图标*/
        switch (p->id) {
#if (defined TCFG_UI_EQ_ENABLE) && TCFG_UI_EQ_ENABLE
        case ID_WINDOW_EQUALIZER:
            extern void ui_eq_window_bt_status_update(void);
            ui_eq_window_bt_status_update();
            break;
#endif
#if (defined TCFG_UI_MUSIC_CTRL_ENABLE) && TCFG_UI_MUSIC_CTRL_ENABLE
        case ID_WINDOW_MUSIC_PLAYER:
            extern void ui_music_window_bt_status_update(void);
            ui_music_window_bt_status_update();
            break;
#endif
#if (defined TCFG_UI_VOLUME_ENABLE) && TCFG_UI_VOLUME_ENABLE
        case ID_WINDOW_VOLUME:
            extern void ui_volume_window_bt_status_update(void);
            ui_volume_window_bt_status_update();
            break;
#endif
#if (defined TCFG_UI_DENOISE_ENABLE) && TCFG_UI_DENOISE_ENABLE
        case ID_WINDOW_EARPHONE_DISNOISE:
            extern void ui_denoise_window_bt_status_update(void);
            ui_denoise_window_bt_status_update();
            break;
#endif
#if (defined TCFG_UI_TIKTOK_ENABLE) && TCFG_UI_TIKTOK_ENABLE
        case ID_WINDOW_TIKTOK:
            extern void ui_tiktok_window_bt_status_update(void);
            ui_tiktok_window_bt_status_update();
            break;
#endif
#if (defined TCFG_UI_FIND_EARPHONE_ENABLE) && TCFG_UI_FIND_EARPHONE_ENABLE
        case ID_WINDOW_FIND_EARPHONE:
            extern void ui_find_earphone_bt_status_update(void);
            ui_find_earphone_bt_status_update();
            break;
#endif
        default:
            break;
        }

        /*上边栏蓝牙图标状态刷新*/
        int ble_sta_index = 0;
        if (sbox_ble_connect_flag_get() && sbox_bt_connect_flag_get()) {
            ble_sta_index = 2;
        } else if (sbox_ble_connect_flag_get()) {
            ble_sta_index = 1;
        }
        struct ui_pic *bt_status_pic;
        bt_status_pic = (struct ui_pic *)ui_core_get_element_by_id(UNIFORM_TOPBAR_EARPHONE);
        if (bt_status_pic) {
            ui_pic_set_image_index(bt_status_pic, ble_sta_index);
        }
        bt_status_pic = (struct ui_pic *)ui_core_get_element_by_id(UNIFORM_TOPBAR_EARPHONE_1);
        if (bt_status_pic) {
            ui_pic_set_image_index(bt_status_pic, ble_sta_index);
        }

        ui_core_redraw(p);
    }

    return 0;
}

int topbar_battery_status_update_cb(const char *type, uint32_t argc)
{
    struct element *layout_elm, *p;
    for (int i = 0; i < CONTROL_ID_COUNT; i++) {
        if (control_id_using[i].is_in_use && ui_core_get_element_by_id(control_id_using[i].control_id)) {
            layout_elm = ui_core_get_element_by_id(control_id_using[i].control_id);

            list_for_each_child_element(p, layout_elm) {
                topbar_elm_status_update(p);
            }
            ui_core_redraw(layout_elm);
        }
    }
    return 0;
}


static uint32_t assign_control_id(uint32_t container_id)
{
    // 尝试分配未使用的control_id
    for (int i = 0; i < CONTROL_ID_COUNT; i++) {
        uint8_t index = (current_control_id_index + i) % CONTROL_ID_COUNT;
        if (!control_id_using[index].is_in_use) {
            control_id_using[index].control_container_id = container_id;
            control_id_using[index].control_id = available_control_ids[index];
            control_id_using[index].is_in_use = true;
            current_control_id_index = (index + 1) % CONTROL_ID_COUNT;
            return control_id_using[index].control_id;
        }
    }

    // 如果所有control_ids都在使用中，选择下一个控制ID以交替使用
    uint8_t index = current_control_id_index % CONTROL_ID_COUNT;
    control_id_using[index].control_container_id = container_id;
    control_id_using[index].control_id = available_control_ids[index];
    control_id_using[index].is_in_use = true; // 即使之前在使用中，也强制分配
    current_control_id_index = (index + 1) % CONTROL_ID_COUNT;
    return control_id_using[index].control_id;
}

static int release_control_id(uint32_t container_id)
{
    for (int i = 0; i < CONTROL_ID_COUNT; i++) {
        if (control_id_using[i].is_in_use && control_id_using[i].control_container_id == container_id) {
            control_id_using[i].is_in_use = false;
            control_id_using[i].control_container_id = 0;
            control_id_using[i].control_id = 0;
            return 0;
        }
    }
    log_error("Release failed: No control_id found for container_id: 0x%x\n", container_id);
    return -1;
}


static int topbar_onchange(void *ctrl, enum element_change_event event, void *arg)
{
    struct element *elm = (struct element *)ctrl;
    struct draw_context *dc = (struct draw_context *)arg;

    switch (event) {
    case ON_CHANGE_INIT: {
        uint32_t new_control_id = assign_control_id(elm->id);
        if (new_control_id == 0) {
            // 分配失败，退出
            log_error("Failed to assign control_id for container_id: 0x%x\n", elm->id);
            return false;
        }
        // struct element *p;
        // list_for_each_child_element(p, elm) {
        //     ui_set_default_handler(p, NULL, NULL, topbar_child_onchange);
        //     redraw_child_in_ui_task(p);
        // }
        if (!elm->dc) {
            ui_core_get_dc(elm);
        }



        // 创建控件
        create_control_by_id(TOPBAR_TAB_PATH, ID_WINDOW_TOPBAR, new_control_id, elm->id);

        break;
    }

    case ON_CHANGE_SHOW_PROBE: {
        // 检查是否已经有对应的control_id
        bool found = false;
        for (int i = 0; i < CONTROL_ID_COUNT; i++) {
            if (control_id_using[i].is_in_use && control_id_using[i].control_container_id == elm->id) {
                found = true;
                break;
            }
        }

        if (!found) {
            log_debug("ON_CHANGE_SHOW_PROBE for container_id: 0x%x\n", elm->id);
            // 分配新的control_id
            uint32_t new_control_id = assign_control_id(elm->id);
            if (new_control_id == 0) {
                // 分配失败，退出
                log_error("Failed to assign control_id for container_id: 0x%x\n", elm->id);
                return false;
            }
            // struct element *p;
            // list_for_each_child_element(p, elm) {
            //     ui_set_default_handler(p, NULL, NULL, topbar_child_onchange);
            //     redraw_child_in_ui_task(p);
            // }

            // 创建控件
            create_control_by_id(TOPBAR_TAB_PATH, ID_WINDOW_TOPBAR, new_control_id, elm->id);

        }

        break;
    }
#if (defined TCFG_UI_BG_ENABLE) && TCFG_UI_BG_ENABLE
    case ON_CHANGE_SHOW:
        csbg_show(elm, dc, CSBG_TYPE_WALLPAPER);
        break;
#endif
    case ON_CHANGE_RELEASE: {
        // csbg_show_deinit();
        if (release_control_id(elm->id) != 0) {
            log_error("Failed to release control_id for container_id: 0x%x\n", elm->id);
        }
        break;
    }

    default:
        break;
    }

    return false;
}

static int topbar_child_onchange(void *ctrl, enum element_change_event event, void *arg)
{
    struct element *elm = (struct element *)ctrl;

    switch (event) {
    case ON_CHANGE_INIT:
        topbar_elm_status_update(elm);
        break;
    default:
        break;
    }
    return false;
}


REGISTER_TOPBAR_CHILD(UNIFORM_TOPBAR_EARPHONE);
REGISTER_TOPBAR_CHILD(UNIFORM_TOPBAR_L_EAR_CAPACITY);
REGISTER_TOPBAR_CHILD(UNIFORM_TOPBAR_R_EAR_CAPACITY);
REGISTER_TOPBAR_CHILD(UNIFORM_TOPBAR_CHARGING_CASE_CAPACITY);
REGISTER_TOPBAR_CHILD(UNIFORM_TOPBAR_TIME);
REGISTER_TOPBAR_CHILD(UNIFORM_TOPBAR_L_EAR_PIC);
REGISTER_TOPBAR_CHILD(UNIFORM_TOPBAR_R_EAR_PIC);
REGISTER_TOPBAR_CHILD(UNIFORM_TOPBAR_CHARGING_CASE_PIC);
REGISTER_TOPBAR_CHILD(UNIFORM_TOPBAR_EARPHONE_1);
REGISTER_TOPBAR_CHILD(UNIFORM_TOPBAR_L_EAR_CAPACITY_1);
REGISTER_TOPBAR_CHILD(UNIFORM_TOPBAR_R_EAR_CAPACITY_1);
REGISTER_TOPBAR_CHILD(UNIFORM_TOPBAR_CHARGING_CASE_CAPACITY_1);
REGISTER_TOPBAR_CHILD(UNIFORM_TOPBAR_TIME_1);
REGISTER_TOPBAR_CHILD(UNIFORM_TOPBAR_L_EAR_PIC_1);
REGISTER_TOPBAR_CHILD(UNIFORM_TOPBAR_R_EAR_PIC_1);
REGISTER_TOPBAR_CHILD(UNIFORM_TOPBAR_CHARGING_CASE_PIC_1);

#endif
