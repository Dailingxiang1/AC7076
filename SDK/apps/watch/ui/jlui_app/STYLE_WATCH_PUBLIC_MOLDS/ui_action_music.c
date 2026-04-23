/* Copyright(C)
 * not free
 * All right reserved
 *
 * @file ui_action_music.c
 * @brief 音乐播放控制 目前只有bt播放
 * @author
 * @version
 * @date 2024-05-21
 */

#include "app_config.h"
#include "system/timer.h"
#include "key_event_deal.h"
#include "app_mode_manager/app_mode_manager.h"
#include "app_main.h"
#include "a2dp_player.h"
#include "file_player.h"
#include "bt_key_func.h"
#include "audio_config.h"
#include "vol_sync.h"
#include "bt_event_func.h"
#include "ui.h"
#include "ui_api.h"
#include "jlui_app/ui_style.h"
#include "jlui_app/ui_sys_param.h"
#include "ui_action_sd_music.h"
#if TCFG_LRC_LYRICS_ENABLE
#include "jlui_app/lyrics_api.h"
#endif
#define LOG_TAG_CONST       UI
#define LOG_TAG     		"[UI-MUSIC]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"


#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ui_action_music.data.bss")
#pragma data_seg(".ui_action_music.data")
#pragma const_seg(".ui_action_music.text.const")
#pragma code_seg(".ui_action_music.text")
#endif

#if (defined(CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE))

/************************************************
 *     添加页面消息处理
 ***********************************************/
#if TCFG_UI_ENABLE_SPORTING
extern int sport_pause_handler(const char *type, u32 arg);
extern int sport_continue_handler(const char *type, u32 arg);
#endif

#if TCFG_UI_ENABLE_MUSIC
static int music_status(const char *type, u32 arg);
static int music_bt_lyrics_change(const char *type, u32 arg);
#endif

#if TCFG_APP_MUSIC_EN
static void music_sd_memory_free();
static void music_sd_memory_malloc();
#endif

const struct uimsg_handl ui_msg_handler[] = {
//    { "music_start",                    music_start     }, /* 音乐播放 */ 目前只有bt播放，默认不会发生出声设备切换
#if TCFG_UI_ENABLE_MUSIC
    { "music_status",                   music_status     }, /* 音乐状态 */
    { "music_bt_lyrics",        	    music_bt_lyrics_change }, /* 蓝牙歌词显示 */
#endif
#if TCFG_UI_ENABLE_SPORTING         // 运动音乐
    { "sport_pause",                    sport_pause_handler     },
    { "sport_continue",                 sport_continue_handler     },
#endif
    { NULL, NULL},      /* 必须以此结尾！ */
};

#if TCFG_UI_ENABLE_MUSIC

#define STYLE_NAME  JL

/**********************
 * DEFINES
 *********************/
#define MUSIC_BT_LYRICS_LEN             256
#define __this (ui_handler)

/**********************
 * TYPEDEFS
 *********************/
typedef struct _MUSIC_UI_VAR {
    u8 start: 1;
    u8 menu: 1;
    int timer;
    int total_time;
    int file_total;
    int cur_num;
    char bt_lyrics[MUSIC_BT_LYRICS_LEN];
    int last_music_percent;
} MUSIC_UI_VAR;

/**********************
 * STATIC PROTOTYPES
 *********************/
static int music_start(const char *type, u32 arg);
static int music_status(const char *type, u32 arg);
static int music_bt_lyrics_change(const char *type, u32 arg);
static int music_ui_volume_to_percent(s16 volume);
static void music_ui_set_volume(int precent);
static void music_status_check(void *p);
static void ui_bt_lyric_cb(u8 type, u32 time, u8 *info, u16 len);

/**********************
 * STATIC VARIABLES
 *********************/
static MUSIC_UI_VAR *ui_handler = NULL;

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
int music_is_play(void);
extern void ui_music_volume_up(void);
extern void ui_music_volume_down(void);

/************************************************
 *     1.相关初始化 2.添加该页面消息处理
 ***********************************************/
static int ui_music_window_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:
        __this = zalloc(sizeof(MUSIC_UI_VAR));
        if (!__this) {
            log_error("<%s> zalloc fail", __func__);
            break;
        }
        __this->timer = sys_timer_add(NULL, music_status_check, 500);
#if TCFG_APP_MUSIC_EN
        music_sd_memory_malloc();
#endif
        ui_register_msg_handler(ID_WINDOW_MUSIC_PLAYER, ui_msg_handler);
#if TCFG_APP_BT_EN
        bt_register_lyric_callback(ui_bt_lyric_cb);
        bt_cmd_prepare(USER_CTRL_AVCTP_OPID_GET_MUSIC_INFO, 0, NULL);
#endif
        break;
    case ON_CHANGE_RELEASE:
#if TCFG_APP_BT_EN
        bt_register_lyric_callback(NULL);
#endif
#if TCFG_APP_MUSIC_EN
        music_sd_memory_free();
#endif
        if (__this) {
            if (__this->timer) {
                sys_timer_del(__this->timer);
                __this->timer = 0;
            }
            free(__this);
            __this = NULL;
        }
#if TCFG_APP_MUSIC_EN
        if (app_get_current_mode_name() == APP_MODE_MUSIC) {
            if (music_player_get_play_status() == 0) {
                app_send_message(APP_MSG_GOTO_MODE, APP_MODE_BT);
            }
        }
#endif
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ID_WINDOW_MUSIC_PLAYER)
.onchange = ui_music_window_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};



/************************************************
 *              相关UI操作功能
 ***********************************************/

/*音乐 暂停开始操作*/
static int ui_music_pause_start_pic_ontouch(void *ctr, struct element_touch_event *e)
{
    struct app_mode *cur_mode;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
    case ELM_EVENT_TOUCH_UP:
        if (e->move_flag != 0) {
            break;
        }

        /*TODO 获取当前出声设备，判断是否处于对应mode，不在切到对应mode*/
        /*当前默认版本只有BT music*/
        cur_mode = app_get_current_mode();
        int next_mode = cur_mode->name;
#if TCFG_APP_MUSIC_EN
        music_task_set_parm(MUSIC_TASK_START_BY_NORMAL, 0);   // 本地音乐界面暂停播放从断点开始播放
        if (cur_mode->name != APP_MODE_MUSIC) {
#if TCFG_APP_BT_EN
            if (bt_get_connect_status() ==  BT_STATUS_WAITINT_CONN)
#endif
            {
                // 蓝牙未连接
                next_mode = APP_MODE_MUSIC;
            }
        } else
#endif
        {
#if TCFG_APP_BT_EN
            if (cur_mode->name != APP_MODE_BT) {
                next_mode = APP_MODE_BT;
            }
#endif
        }
        if (next_mode != cur_mode->name) {
            app_send_message(APP_MSG_GOTO_MODE, next_mode);
#if TCFG_USER_EMITTER_ENABLE
            extern u8 *get_cur_connect_emitter_mac_addr(void);
            extern u8 bt_emitter_pp(u8 pp);
            void *bt_addr = get_cur_connect_emitter_mac_addr();
            if (bt_addr) {     // 若蓝牙发射正在开启
                if (next_mode == APP_MODE_BT) {
                    bt_emitter_pp(0);
                    ui_pic_show_image_by_id(MUSIC_PAUSE_START_PIC, 0);
                } else if (next_mode == APP_MODE_MUSIC) {
                    bt_emitter_pp(1);
                    ui_pic_show_image_by_id(MUSIC_PAUSE_START_PIC, 1);
                }
            }
#endif
        } else {
            app_send_message(APP_MSG_MUSIC_PP, 0);
        }
        break;
    }
    return false;
}

/*音乐 暂停开始状态 初始显示*/
static int ui_music_pause_start_pic_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        if (music_is_play() == true) {
            ui_pic_set_image_index(pic, 1);
        } else {
            ui_pic_set_image_index(pic, 0);
        }
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(MUSIC_PAUSE_START_PIC)
.onchange = ui_music_pause_start_pic_onchange,
 .ontouch = NULL,
};

REGISTER_UI_EVENT_HANDLER(MUSIC_PAUSE_START_BUTTON)
.onchange = NULL,
 .ontouch = ui_music_pause_start_pic_ontouch,
};

/*音乐 上一曲*/
static int ui_music_prev_pic_ontouch(void *ctr, struct element_touch_event *e)
{
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
    case ELM_EVENT_TOUCH_UP:
        if (e->move_flag != 0) {
            break;
        }
#if TCFG_APP_MUSIC_EN
        if (app_get_current_mode_name() == APP_MODE_MUSIC) {
            app_send_message(APP_MSG_MUSIC_PREV, 0);
            break;
        }
#endif
#if TCFG_APP_BT_EN
        bt_key_music_prev();
#endif
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(MUSIC_PREV_PIC)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = ui_music_prev_pic_ontouch,
};

/*音乐 下一曲*/
static int ui_music_next_pic_ontouch(void *ctr, struct element_touch_event *e)
{
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
    case ELM_EVENT_TOUCH_UP:
        if (e->move_flag != 0) {
            break;
        }
#if TCFG_APP_MUSIC_EN
        if (app_get_current_mode_name() == APP_MODE_MUSIC) {
            app_send_message(APP_MSG_MUSIC_NEXT, 0);
            break;
        }
#endif
#if TCFG_APP_BT_EN
        bt_key_music_next();
#endif
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(MUSIC_NEXT_PIC)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = ui_music_next_pic_ontouch,
};

/*音乐 音量进度条控制*/
static int ui_music_volume_slider_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_slider *slider = (struct ui_slider *)_ctrl;
    int slider_percent;

    switch (event) {
    case ON_CHANGE_INIT:
        slider_percent = music_ui_volume_to_percent(app_audio_get_volume(APP_AUDIO_STATE_MUSIC));
        log_info("<%s> slider_percent:%d", __func__, slider_percent);
        ui_slider_set_persent(slider, slider_percent);
        break;
    default:
        break;
    }
    return 0;
}

static int ui_music_volume_slider_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_slider *slider = (struct ui_slider *)ctr;
    int slider_percent;
    static u8 R_MOVE_flag = 0;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        R_MOVE_flag = 1;
        ui_card_disable();
        return true;
    case ELM_EVENT_TOUCH_UP:
        R_MOVE_flag = 0;
        slider_percent = slider_get_percent(slider);
        log_info("<%s> slider_percent:%d", __func__, slider_percent);
        music_ui_set_volume(slider_percent);
        ui_card_enable();
        break;
    case ELM_EVENT_TOUCH_MOVE:
        slider_touch_slider_move(slider, e);
        return true;
        break;
    case ELM_EVENT_TOUCH_R_MOVE:
        if (R_MOVE_flag == 0) {
            break;
        } else {
            R_MOVE_flag = 0;
            return true;
        }
    default :
        break;
    }
    return false;
}

static int ui_music_volume_slider_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_slider *slider = (struct ui_slider *)ctr;
    int slider_percent;

    switch (e->value) {
    case KEY_UI_PLUS:
        ui_music_volume_up();
        slider_percent = music_ui_volume_to_percent(app_audio_get_volume(APP_AUDIO_STATE_MUSIC));
        ui_slider_set_persent_by_id(slider->elm.id, slider_percent);
        log_debug("<%s> slider_percent:%d", __func__, slider_percent);
        return true;
    case KEY_UI_MINUS:
        ui_music_volume_down();
        slider_percent = music_ui_volume_to_percent(app_audio_get_volume(APP_AUDIO_STATE_MUSIC));
        ui_slider_set_persent_by_id(slider->elm.id, slider_percent);
        log_debug("<%s> slider_percent:%d", __func__, slider_percent);
        return true;
    default:
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(MUSIC_VOLUME_SLIDER)
.onchange = ui_music_volume_slider_onchange,
 .onkey = ui_music_volume_slider_onkey,
  .ontouch = ui_music_volume_slider_ontouch,
};


/********************************************************************************************/

#if TCFG_APP_MUSIC_EN
music_sd *music_handler = NULL;
static void music_sd_memory_malloc()
{
    if (music_handler) {
        return;
    }
    music_handler = (music_sd *)zalloc(sizeof(music_sd));
}
static void music_sd_memory_free()
{
#if TCFG_LRC_LYRICS_ENABLE
    if (music_handler && music_handler->timer) {
        sys_timer_del(music_handler->timer);
        music_handler->timer = 0;
    }
#endif
    if (!music_handler) {
        return;
    }
    free(music_handler);
    music_handler = NULL;
}

void music_change_layout_css(u8 tmp, struct element *elm)
{
    if (music_handler->core == tmp) {
        elm->css.invisible = 0;
        ui_show(elm->id);
    } else {
        elm->css.invisible = 1;
        ui_hide(elm->id);
    }
}
void music_ui_layout_switch(u8 ui_temp)
{
    struct element *ctr = ui_core_get_element_by_id(MUSIC_LAYER);
    if (ctr == NULL) {
        return;
    }
    music_handler->core = ui_temp;
    struct element *elm;
    list_for_each_child_element(elm, (struct element *)ctr) {
        switch (elm->id) {
        case MUSIC_CONTROL_LAYOUT:
            music_change_layout_css(MUSIC_PLAYER, elm);
            break;
        case MUSIC_LOCAL_LAYOUT:
            music_change_layout_css(LOCAL_LIST, elm);
            break;
        }
        ui_core_redraw(ctr);
    }
}
static int ui_sd_music_enter_ontouch(void *ctr, struct element_touch_event *e)
{
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
        break;
    case ELM_EVENT_TOUCH_U_MOVE:
        music_ui_layout_switch(LOCAL_LIST);
        return true;
        break;
    }
    return false;
}
#if TCFG_LRC_LYRICS_ENABLE
static void ui_sd_music_show_lyrics()
{
    if (music_is_play()) {
        lrc_show_api(MUSIC_LYRICS_TEXT, music_file_get_cur_time(get_music_file_player()), 0);
    }
}
#endif
static int ui_sd_music_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:

#if TCFG_LRC_LYRICS_ENABLE
        if (music_handler && !music_handler->timer) {
            music_handler->timer = sys_timer_add(NULL, ui_sd_music_show_lyrics, 500);
        }
#endif
        break;
    case ON_CHANGE_FIRST_SHOW:
        music_handler->core = 0;
        struct element *elm;
        list_for_each_child_element(elm, (struct element *)ctr) {
            switch (elm->id) {
            case MUSIC_CONTROL_LAYOUT:
                music_change_layout_css(MUSIC_PLAYER, elm);
                break;
            case MUSIC_LOCAL_LAYOUT:
                music_change_layout_css(LOCAL_LIST, elm);
                break;
            }
        }
        break;
    case ON_CHANGE_RELEASE:
#if TCFG_LRC_LYRICS_ENABLE
        if (music_handler && music_handler->timer) {
            sys_timer_del(music_handler->timer);
            music_handler->timer = 0;
        }
#endif
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(MUSIC_CONTROL_LAYOUT)
.onchange = ui_sd_music_onchange,
 .onkey = NULL,
  .ontouch = ui_sd_music_enter_ontouch,
};
#else
static int ui_music_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:
        ui_hide(MUSIC_LOCAL_LAYOUT);
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(MUSIC_CONTROL_LAYOUT)
.onchange = ui_music_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

#endif
/********************************************************************************************/

/************************************************
 *       接受btstack消息进行处理
 ***********************************************/

static int ui_a2dp_bt_status_event_handler(int *event)
{
    struct bt_event *bt = (struct bt_event *)event;

    switch (bt->event) {
    case BT_STATUS_AVRCP_VOL_CHANGE:
        if (ui_get_current_window_id() == ID_WINDOW_MUSIC_PLAYER) {
            u32 phone_volume = bt->value;
            log_info("<%s> phone_volume:%d", __func__, phone_volume);
            if (phone_volume == 0xFF) {
                break;
            }
            UI_MSG_POST("music_status:volume=%4", phone_volume);
        }
        break;
    }
    return 0;
}

APP_MSG_HANDLER(ui_a2dp_stack_msg_handler) = {
    .owner      = 0xff,
    .from       = MSG_FROM_BT_STACK,
    .handler    = ui_a2dp_bt_status_event_handler,
};



/************************************************
 *
 ***********************************************/

static int music_status(const char *type, u32 arg)
{
    log_info("<%s> type:%s, arg:%d", __func__, type, arg);
    if (!strcmp(type, "volume")) {
        s16 phone_volume = (s16)arg;
        s16 volume = phone_volume * app_audio_volume_max_query(AppVol_BT_MUSIC) / 127;
        ui_slider_set_persent_by_id(MUSIC_VOLUME_SLIDER,  music_ui_volume_to_percent(volume));
    }
    return 0;
}

static int music_bt_lyrics_change(const char *type, u32 arg)
{
    u32 cur_window_id = UI_GET_WINDOW_ID();
    if (cur_window_id == ID_WINDOW_MUSIC_PLAYER) {
        ui_text_set_textu_by_id(MUSIC_LYRICS_TEXT, (const char *)__this->bt_lyrics, strlen(__this->bt_lyrics), FONT_DEFAULT | FONT_SHOW_SCROLL);
    } else if (cur_window_id == ID_WINDOW_SPORTING) {
        ui_text_set_textu_by_id(SPORTING_MUSIC_LYRICS_TEXT, (const char *)__this->bt_lyrics, strlen(__this->bt_lyrics), FONT_DEFAULT | FONT_SHOW_SCROLL);
    }

    return 0;
}

u16 music_get_bt_lyrics_change()
{
    if (__this != NULL) {

        return (strlen(__this->bt_lyrics));
    }
    return 0;
}

int music_is_play(void)
{
    if (app_get_current_mode_name() == APP_MODE_BT) {
        if (a2dp_player_runing()) {
            return true;
        }
    }
#if TCFG_APP_MUSIC_EN
    if (app_get_current_mode_name() == APP_MODE_MUSIC) {
        if (music_player_get_play_status()) {
            return true;
        }
    }
#endif

    return false;
}

static int music_ui_volume_to_percent(s16 volume)
{
    log_info("<%s> volume:%d volume_max:%d", __func__, volume, app_audio_volume_max_query(AppVol_BT_MUSIC));
    int percent;
    percent = volume * 100 / app_audio_volume_max_query(AppVol_BT_MUSIC);
    __this->last_music_percent = percent;
    return percent;
}

static void music_ui_set_volume(int precent)
{
    s16 volume;
    s16 volume_max =  app_audio_volume_max_query(AppVol_BT_MUSIC);
    volume = volume_max * precent / 100;
    if (volume > volume_max) {
        volume = volume_max;
    }

    log_info("<%s> volume:%d, volume_max:%d", __func__, volume, volume_max);
    if (__this->last_music_percent > precent) {
        opid_play_vol_sync_fun(&volume, 0);
    } else if (__this->last_music_percent < precent) {
        opid_play_vol_sync_fun(&volume, 1);
    }
    __this->last_music_percent = precent;

    app_audio_set_volume(APP_AUDIO_STATE_MUSIC, volume, 1);
#if TCFG_APP_BT_EN
    bt_cmd_prepare(USER_CTRL_AVCTP_OPID_SEND_VOL, 0, NULL);
#endif
#if TCFG_USER_EMITTER_ENABLE
    bt_emitter_cmd_prepare(USER_CTRL_AVCTP_OPID_SEND_VOL, 0, NULL);
#endif
    syscfg_write(CFG_MUSIC_VOL, &volume, 2);
}

static void music_status_check(void *p)
{
    struct app_mode *cur_mode;
    static u8 start_pic_mode_last = 0;
    u8 start_pic_mode = 0;
    if (!__this) {
        return;
    }

    u32 cur_window_id = UI_GET_WINDOW_ID();
    cur_mode = app_get_current_mode();
#if TCFG_APP_BT_EN
    if (cur_mode->name == APP_MODE_BT) {
        u8 a2dp_state = bt_a2dp_get_status();
        log_debug("a2dp_state :%d \n", a2dp_state);
        if (a2dp_state == BT_MUSIC_STATUS_STARTING) {
            if (cur_window_id == ID_WINDOW_MUSIC_PLAYER) {
                start_pic_mode = 1;
                /* ui_pic_show_image_by_id(MUSIC_PAUSE_START_PIC, 1); */
            } else if (cur_window_id == ID_WINDOW_SPORTING) {
                start_pic_mode = 1;
                /* ui_pic_show_image_by_id(SPORTING_MUSIC_PAUSE_START_PIC, 1); */
            }
        } else {
            if (cur_window_id == ID_WINDOW_MUSIC_PLAYER) {
                start_pic_mode = 0;
                /* ui_pic_show_image_by_id(MUSIC_PAUSE_START_PIC, 0); */
            } else if (cur_window_id == ID_WINDOW_SPORTING) {
                start_pic_mode = 0;
                /* ui_pic_show_image_by_id(SPORTING_MUSIC_PAUSE_START_PIC, 0); */
            }
        }
        if (start_pic_mode_last != start_pic_mode) {
            start_pic_mode_last = start_pic_mode;
            ui_pic_show_image_by_id(MUSIC_PAUSE_START_PIC, start_pic_mode);
        }
        return;
    }
#endif
#if TCFG_APP_MUSIC_EN
#if TCFG_USER_EMITTER_ENABLE
    extern u8 *get_cur_connect_emitter_mac_addr(void);
    void *bt_addr = get_cur_connect_emitter_mac_addr();
    if (bt_addr) {     // 若蓝牙发射正在开启
        if (app_var.a2dp_source_open_flag && cur_mode->name == APP_MODE_MUSIC) {
            start_pic_mode = 1;
            /* ui_pic_show_image_by_id(MUSIC_PAUSE_START_PIC, 1); */
        } else if (!app_var.a2dp_source_open_flag) {
            start_pic_mode = 0;
            /* ui_pic_show_image_by_id(MUSIC_PAUSE_START_PIC, 0); */
        }
        if (start_pic_mode_last != start_pic_mode) {
            start_pic_mode_last = start_pic_mode;
            ui_pic_show_image_by_id(MUSIC_PAUSE_START_PIC, start_pic_mode);
        }
        return;
    }
#endif
    if (cur_mode->name == APP_MODE_MUSIC) {
        if (music_player_get_play_status()) {
            if (cur_window_id == ID_WINDOW_MUSIC_PLAYER) {
                start_pic_mode = 1;
                /* ui_pic_show_image_by_id(MUSIC_PAUSE_START_PIC, 1); */
            } else if (cur_window_id == ID_WINDOW_SPORTING) {
                start_pic_mode = 1;
                /* ui_pic_show_image_by_id(SPORTING_MUSIC_PAUSE_START_PIC, 1); */
            }
        } else {
            if (cur_window_id == ID_WINDOW_MUSIC_PLAYER) {
                start_pic_mode = 0;
                /* ui_pic_show_image_by_id(MUSIC_PAUSE_START_PIC, 0); */
            } else if (cur_window_id == ID_WINDOW_SPORTING) {
                start_pic_mode = 0;
                /* ui_pic_show_image_by_id(SPORTING_MUSIC_PAUSE_START_PIC, 0); */
            }
        }
    }
#endif
    if (start_pic_mode_last != start_pic_mode) {
        start_pic_mode_last = start_pic_mode;
        ui_pic_show_image_by_id(MUSIC_PAUSE_START_PIC, start_pic_mode);
    }
}

static void ui_bt_lyric_cb(u8 type, u32 time, u8 *info, u16 len)
{
    struct app_mode *cur_mode;
    if (!__this) {
        return;
    }

    switch (type) {
    case 1:
        cur_mode = app_get_current_mode();
        if (cur_mode->name != APP_MODE_BT) {
            return ;
        }
        memset(__this->bt_lyrics, 0, MUSIC_BT_LYRICS_LEN);
        if (len >= MUSIC_BT_LYRICS_LEN) {
            len = MUSIC_BT_LYRICS_LEN - 1;
        }
        memcpy(__this->bt_lyrics, info, len);
        UI_MSG_POST("music_bt_lyrics");
        break;
    default:
        break;
    }
}



/************************************************
 * 运动的音乐控制
 ***********************************************/

#if TCFG_UI_ENABLE_SPORTING
static int ui_sporting_layout_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT_PROBE:
        __this = zalloc(sizeof(MUSIC_UI_VAR));
        if (!__this) {
            log_error("<%s> zalloc fail", __func__);
            break;
        }
        break;
    case ON_CHANGE_INIT:
        __this->timer = sys_timer_add(NULL, music_status_check, 500);
#if TCFG_APP_BT_EN
        bt_register_lyric_callback(ui_bt_lyric_cb);
        bt_cmd_prepare(USER_CTRL_AVCTP_OPID_GET_MUSIC_INFO, 0, NULL);
#endif
        break;
    case ON_CHANGE_RELEASE:
#if TCFG_APP_BT_EN
        bt_register_lyric_callback(NULL);
#endif
        if (__this) {
            if (__this->timer) {
                sys_timer_del(__this->timer);
                __this->timer = 0;
            }
            free(__this);
            __this = NULL;
        }
        break;
    default:
        return false;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(SPORTING_MUSIC_CONTROL_LAYOUT)
.onchange = ui_sporting_layout_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

REGISTER_UI_EVENT_HANDLER(SPORTING_MUSIC_PAUSE_START_PIC)
.onchange = ui_music_pause_start_pic_onchange,
 .ontouch = ui_music_pause_start_pic_ontouch,
};

REGISTER_UI_EVENT_HANDLER(SPORTING_MUSIC_PREV_PIC)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = ui_music_prev_pic_ontouch,
};

REGISTER_UI_EVENT_HANDLER(SPORTING_MUSIC_NEXT_PIC)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = ui_music_next_pic_ontouch,
};

REGISTER_UI_EVENT_HANDLER(SPORTING_MUSIC_VOLUME_SLIDER)
.onchange = ui_music_volume_slider_onchange,
 .onkey = ui_music_volume_slider_onkey,
  .ontouch = ui_music_volume_slider_ontouch,
};
#endif


#endif /* if TCFG_UI_ENABLE_MUSIC */
#endif /* #if (defined(CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE)) */


