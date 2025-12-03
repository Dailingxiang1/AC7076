#include "app_config.h"
#include "ui/ui_api.h"
#include "font/language_list.h"
#include "a2dp_player.h"
#include "avctp_user.h"
#include "events_adapter.h"
#include "smartbox_info_manager.h"
#include "app_mode_manager/app_mode_manager.h"
#include "app_task.h"
#include "ui_sd_music.h"
#include "bt_key_func.h"
#include "bt_event_func.h"
#include "app_music.h"
#if (defined TCFG_UI_UP_MOVE_DEMO_ENABLE && TCFG_UI_UP_MOVE_DEMO_ENABLE)
#include "ui_up_move.h"
#endif
#if TCFG_LRC_LYRICS_ENABLE
#include "file_player.h"
#include "jlui_app/lyrics_api.h"
#endif



#define LOG_TAG_CONST       UI
#define LOG_TAG     		"[UI_MUSIC]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ui_music.data.bss")
#pragma data_seg(".ui_music.data")
#pragma const_seg(".ui_music.text.const")
#pragma code_seg(".ui_music.text")
#endif

#define STYLE_NAME  JL
#define __this (music_handler)

#if (defined(CONFIG_UI_STYLE_JL_CSC_PUBLIC_MODLS_ENABLE))
#if (defined TCFG_UI_MUSIC_CTRL_ENABLE && TCFG_UI_MUSIC_CTRL_ENABLE)

static u8 music_mode = PHONE_MUSIC_MODE;
static u8 music_play_sel = EARPHONE_PLAY;
static u8 bt_emitter_connecting_flag = 0;
static u16 timer_id = 0;
music_info *music_handler = NULL;

extern int music_player_get_play_status(void);
extern u32 dev_manager_get_total(u8 valid);
extern bool lrc_show_api(int text_id, u16 dbtime_s, u8 btime_100ms);
extern u8 *get_cur_connect_emitter_mac_addr();
extern void custom_client_send_ctrl_edr_conn(u8 cmd);
extern void bt_init_bredr();
extern u8 is_bredr_close(void);
extern void emitter_bt_connect(u8 *mac);
extern void sbox_get_earphone_mac(u8 *addr);
extern void bt_close_bredr();
extern void bt_register_lyric_callback(bt_lrc_cb_t cb);
extern void custom_client_send_ctrl_edr_info(void);

static void music_status_check(void *p);
static int music_bt_lyrics_change(const char *type, u32 arg);
static void ui_song_lyric_update(void *p, u32 arg);
static void ui_music_name_update();
/* extern int music_file_get_cur_time(struct file_player *music_player); */

typedef enum {
    ICON_STATUS_DISCONNECT = 0,
    ICON_STATUS_CONNECT,
    ICON_STATUS_PRESSING,
} ICON_STATUS;

static const struct uimsg_handl ui_msg_handler[] = {
    {"up_music_title", (int (*)(const char *, u32)) ui_song_lyric_update },
    {"up_music_name", (int (*)(const char *, u32)) ui_music_name_update },
    {"up_music_status", (int (*)(const char *, u32)) music_status_check },
    { "music_bt_lyrics",        	    music_bt_lyrics_change }, /* 蓝牙歌词显示 */
    {NULL, NULL},
};

static int ui_music_get_language(void)
{
    u8 sel_language[5] = {Chinese_Simplified, English, German, French, Spanish};
    u8 language_id = 0;
    int index = 0;
    language_id = sbox_language_ui_get();
    for (int i = 0 ; i < ARRAY_SIZE(sel_language) ; i++) {
        if (language_id == sel_language[i]) {
            index = i;
            break;
        }
    }
    return index ;
}


/**
 * @func : 音乐播放暂停控制按钮显示逻辑
 */
static int music_status(char *type, u32 arg)
{
    log_info("<%s> type:%s, arg:%d ,task:%s\n", __func__, type, arg, os_current_task());
    int state = arg;
    if (!strcmp(type, "pp")) {
        struct element *elm = ui_core_get_element_by_id(MUSIC_PLAY_PIC);
        if (!elm) {
            log_error("elm is null");
            return -1;
        }
        if (arg == MUSIC_STATE_PLAY) {
            ui_pic_set_image_index((struct ui_pic *)elm, 2);
        } else if (arg == MUSIC_STATE_PAUSE) {
            ui_pic_set_image_index((struct ui_pic *)elm, 1);
        }
        ui_core_redraw(elm);
    }
    return 0;
}


void ui_music_update(char *type, int arg)
{
    if (strcmp(os_current_task(), "ui")) {
        // printf("qcallback!\n" );
        int argv[4];
        argv[0] = (int)music_status;
        argv[1] = 2;
        argv[2] = (int)type;
        argv[3] = arg;
        int ret = os_taskq_post_type("ui", Q_CALLBACK, ARRAY_SIZE(argv), argv);
        printf("qcallback ret:%d\n", ret);
        return ;
    }
    music_status(type, arg);
}
static void music_player_status_init()
{
    if (music_mode == PHONE_MUSIC_MODE) {
        u8 state = sbox_music_ui_state_get();
        if (sbox_ble_connect_flag_get()) {
            if (state == MUSIC_STATE_PLAY) {
                ui_pic_show_image_by_id(MUSIC_PLAY_PIC, 2);
            } else if (state == MUSIC_STATE_PAUSE) {
                ui_pic_show_image_by_id(MUSIC_PLAY_PIC, 1);
            }
        } else {
            ui_pic_show_image_by_id(MUSIC_PLAY_PIC, 0);
        }
    }
#if TCFG_APP_MUSIC_EN
    else {
        if (dev_manager_get_total(1)) {
            if (music_player_get_play_status()) {
                ui_pic_show_image_by_id(MUSIC_PLAY_PIC, 2);
            } else {
                ui_pic_show_image_by_id(MUSIC_PLAY_PIC, 1);
            }
        } else {
            ui_pic_show_image_by_id(MUSIC_PLAY_PIC, 0);
        }
    }
#endif
}
static int music_pp_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_pic *pic = (struct ui_pic *)_ctrl;
    u32 cur_window_id = UI_GET_WINDOW_ID();
    switch (event) {
    case ON_CHANGE_INIT:
        // ui_register_msg_handler(ID_WINDOW_MUSIC_PLAYER, ui_msg_handler);
        music_player_status_init();

        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        break;
    }

    return 0;
}



/**
 * @func : 音乐播放暂停控制按钮控制逻辑
 */

static int music_change_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;
    struct app_mode *cur_mode;
    log_info("func:%s ,e->event :%d , e->move_flag : %d \n ", __func__, e->event, e->move_flag);
    static u8 flag = 0;
    const char *sd_name = "SD MUSIC";
    const char *phone_name = "PHONE MUSIC";
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
    case ELM_EVENT_TOUCH_UP:
        if (e->move_flag) {
            return false;
        }
        ui_hide(MUSIC_CONTROL_LAYOUT);
        ui_show(MUSIC_MODE_LAYOUT);
        break;
    default :
        break;
    }
    return false;
}

static int music_pp_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;
    struct app_mode *cur_mode;
    /* log_info("func:%s ,e->event :%d , e->move_flag : %d \n ", __func__, e->event, e->move_flag); */
    static u8 flag = 0;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
    case ELM_EVENT_TOUCH_UP:
        if (e->move_flag) {
            return false;
        }
        if (__this->mode == PHONE_MUSIC_MODE && __this->play_sel == EARPHONE_PLAY) {
            if (!sbox_ble_connect_flag_get()) {
                r_printf("播放按钮，未连接");
                ui_pic_set_image_index(pic, 0);
                return false;
            }
            if (!e->move_flag) {
                int state = sbox_music_ui_state_get();
                if (state == MUSIC_STATE_PLAY) {
                    ui_pic_show_image_by_id(MUSIC_PLAY_PIC, 2);
                    sbox_music_ui_state_set(MUSIC_STATE_PAUSE);
                    custom_client_send_music_ctrl(MUSIC_STATE_PAUSE);
                } else if (state == MUSIC_STATE_PAUSE) {
                    ui_pic_show_image_by_id(MUSIC_PLAY_PIC, 1);
                    sbox_music_ui_state_set(MUSIC_STATE_PLAY);
                    custom_client_send_music_ctrl(MUSIC_STATE_PLAY);
                }
            }
        } else if ((__this->mode == PHONE_MUSIC_MODE) && (__this->play_sel == LOCAL_PLAY)) {
            u8 a2dp_state = bt_a2dp_get_status();
            log_debug("a2dp_state :%d \n", a2dp_state);
            if (a2dp_state == BT_MUSIC_STATUS_STARTING) {
                bt_cmd_prepare(USER_CTRL_AVCTP_PAUSE_MUSIC, 0, NULL);
            } else {
                bt_cmd_prepare(USER_CTRL_AVCTP_OPID_PLAY, 0, NULL);
            }
        }
#if TCFG_APP_MUSIC_EN
        else if (__this->mode == LOCAL_MODE) {
            cur_mode = app_get_current_mode();
            music_task_set_parm(MUSIC_TASK_START_BY_NORMAL, 0);   // 本地音乐界面暂停播放从断点开始播放
            int next_mode = cur_mode->name;
            if (cur_mode->name != APP_MODE_MUSIC) {
                next_mode = APP_MODE_MUSIC;
            } else {
#if TCFG_APP_BT_EN
                if (cur_mode->name != APP_MODE_BT) {
                    next_mode = APP_MODE_BT;
                }
#endif
            }
            if (next_mode != cur_mode->name) {
                /* app_send_message(APP_MSG_MUSIC_PP, 0); */
                app_send_message(APP_MSG_GOTO_MODE, next_mode);


            } else {
                app_send_message(APP_MSG_MUSIC_PP, 0);
                /* ui_pic_show_image_by_id(MUSIC_PLAY_PIC, 2); */
            }
            break;
        }
#endif

        break;
    default :
        break;
    }
    return false;
}



/**
 * @func : 音乐上一曲按钮显示逻辑
 */
static int music_prev_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_pic *pic = (struct ui_pic *)_ctrl;
    switch (event) {
    case ON_CHANGE_INIT:
        if (sbox_ble_connect_flag_get()) {
            ui_pic_set_image_index(pic, 1);
        } else {
            ui_pic_set_image_index(pic, 0);
        }
        break;
    default:
        break;
    }
    return 0;
}



/**
 * @func : 音乐上一曲按钮控制逻辑
 */
static int music_prev_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;
    log_info("func:%s ,e->event :%d , e->move_flag : %d \n ", __func__, e->event, e->move_flag);
    static u8 flag = 0;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
    case ELM_EVENT_TOUCH_UP:
        if (e->move_flag) {
            return false;
        }
#if TCFG_APP_MUSIC_EN
        if (__this->mode == LOCAL_MODE && __this->play_sel == LOCAL_PLAY) {
            if (app_get_current_mode_name() == APP_MODE_MUSIC) {
                app_send_message(APP_MSG_MUSIC_PREV, 0);
                break;
            }
        }
#endif
#if TCFG_APP_BT_EN
        if (__this->mode == PHONE_MUSIC_MODE && __this->play_sel == LOCAL_PLAY) {
            bt_key_music_prev();
            break;
        }
#endif
#if TCFG_USER_EMITTER_ENABLE
        if (__this->mode == PHONE_MUSIC_MODE && __this->play_sel == LOCAL_PLAY) {

        }
#endif
        {
            if (!sbox_ble_connect_flag_get()) {
                return false;
            }
            custom_client_send_music_ctrl(MUSIC_STATE_PRV);
            break;
        }

        break;
    default :
        break;
    }
    return false;
}





/**
 * @func : 音乐下一曲按钮显示逻辑
 */
static int music_next_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_pic *pic = (struct ui_pic *)_ctrl;
    switch (event) {
    case ON_CHANGE_INIT:
        if (sbox_ble_connect_flag_get()) {
            ui_pic_set_image_index(pic, 1);
        } else {
            ui_pic_set_image_index(pic, 0);
        }
        break;
    default:
        break;
    }
    return 0;
}



/**
 * @func : 音乐下一曲按钮控制逻辑
 */
static int music_next_ontouch(void *ctr, struct element_touch_event *e)
{

    struct ui_pic *pic = (struct ui_pic *)ctr;
    log_info("func:%s ,e->event :%d , e->move_flag : %d \n ", __func__, e->event, e->move_flag);
    static u8 flag = 0;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
    case ELM_EVENT_TOUCH_UP:
        if (e->move_flag) {
            return false;
        }
#if TCFG_APP_MUSIC_EN
        if (__this->mode == LOCAL_MODE && __this->play_sel == LOCAL_PLAY) {
            if (app_get_current_mode_name() == APP_MODE_MUSIC) {
                app_send_message(APP_MSG_MUSIC_NEXT, 0);
                break;
            }
        }
#endif
#if TCFG_APP_BT_EN
        if (__this->mode == PHONE_MUSIC_MODE && __this->play_sel == LOCAL_PLAY) {
            bt_key_music_next();
            break;
        }
#endif
#if TCFG_USER_EMITTER_ENABLE
        if (__this->mode == PHONE_MUSIC_MODE && __this->play_sel == LOCAL_PLAY) {

        }
#endif
        {
            if (!sbox_ble_connect_flag_get()) {
                return false;
            }
            custom_client_send_music_ctrl(MUSIC_STATE_NEXT);
            break;
        }
    default :
        break;
    }
    return false;
}
static void music_sd_memory_malloc()
{
    if (__this) {
        return;
    }
    __this = (music_info *)zalloc(sizeof(music_info));
    ASSERT(__this);
}


static void music_sd_memory_free()
{
    if (!__this) {
        return;
    }
    free(__this);
    __this = NULL;
}

u8 music_get_info_handle()
{
    return music_play_sel;
}
u8 music_mode_get_info()
{
    return music_mode;
}
void music_change_layout_css(u8 tmp, struct element *elm)
{
    if (LOCAL_LIST == __this->core) {
        ui_card_disable();
    } else {
        ui_card_enable();
    }
    if (__this->core == tmp) {
        elm->css.invisible = 0;
    } else {
        elm->css.invisible = 1;
    }
}

void music_ui_layout_switch(u8 ui_temp)
{
    struct element *ctr = ui_core_get_element_by_id(MUSIC_LAYER);
    music_player_status_init();
    if (ctr == NULL) {
        return;
    }
    __this->core = ui_temp;
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
        /* music_ui_layout_switch(LOCAL_LIST); */
        return true;
        break;
    }
    return false;
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
#if TCFG_LRC_LYRICS_ENABLE&&TCFG_APP_MUSIC_EN
static void ui_sd_music_show_lyrics()
{
    if (music_is_play()) {
        lrc_show_api(MUSIC_TITLE_TEXT, music_file_get_cur_time(get_music_file_player()), 0);
    }
}

#endif

/**
 * @func : 音乐标题显示逻辑
 */
static void ui_song_lyric_update(void *p, u32 arg)
{

    if (__this->mode == PHONE_MUSIC_MODE && __this->play_sel == EARPHONE_PLAY) {
        struct element *title_elm = ui_core_get_element_by_id(MUSIC_TITLE_TEXT);
        char *text_show = (char *)arg;
        if (title_elm) {
            ui_text_set_textu_by_id(MUSIC_TITLE_TEXT, text_show, strlen(text_show), FONT_DEFAULT | FONT_SHOW_MULTI_LINE | FONT_SHOW_SCROLL);
        }
    }
}
static void ui_music_name_update()
{

#if TCFG_APP_MUSIC_EN
    if (__this->mode == LOCAL_MODE) {
        struct element *title_elm = ui_core_get_element_by_id(MUSIC_TITLE_TEXT);
        extern void ui_music_get_file_name();
        ui_music_get_file_name();

        /* snprintf(__this->name, sizeof(__this->name), "%s", music_file_name); */
        /* char *text_show = (char *)arg; */
        if (title_elm) {
            ui_text_set_textu_by_id(MUSIC_TITLE_TEXT, __this->name, strlen(__this->name), FONT_DEFAULT | FONT_SHOW_MULTI_LINE);
        }

    }
#endif
}

static int music_bt_lyrics_change(const char *type, u32 arg)
{
    u32 cur_window_id = UI_GET_WINDOW_ID();
    if (cur_window_id == ID_WINDOW_MUSIC_PLAYER && (__this->mode == PHONE_MUSIC_MODE) && (__this->play_sel == LOCAL_PLAY)) {
        ui_text_set_textu_by_id(MUSIC_TITLE_TEXT, (const char *)__this->bt_lyrics, strlen(__this->bt_lyrics), FONT_DEFAULT | FONT_SHOW_SCROLL);
    }
    return 0;
}

static int music_text_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_text *text = (struct ui_text *)_ctrl;
    char name[2][32] = {
        "SD Music",
        "Phone Music",
    };
    switch (event) {
    case ON_CHANGE_INIT:
        switch (text->elm.id) {
        case MUSIC_TITLE_TEXT:
            text_set_font_scroll_interval(150);
            log_info("func:%s ,strlen(lyrics_artist_name_array): %d \n ", __func__, strlen(lyrics_artist_name_array));
#if TCFG_APP_MUSIC_EN
            if (app_get_current_mode_name()  == APP_MODE_MUSIC) {
                ui_music_name_update();
                return false;
            }
#endif
            if (lyrics_artist_name_array[0] == '\n') {
                int index ;

#if TCFG_APP_MUSIC_EN
                if (music_mode == LOCAL_MODE) {
                    index = 0;
                } else
#endif
                {
                    index = 1;
                }
                log_debug("func:%s ,index : %d \n ", __func__, index);
                if (index < 2) {
                    memcpy(lyrics_artist_name_array, &name[index], strlen((const char *)&name[index]));
                }
            }
            /*文字控件 字库类型 避免空buf进去*/
            ui_text_set_text_attrs(text, lyrics_artist_name_array, strlen(lyrics_artist_name_array), FONT_ENCODE_UTF8, FONT_ENDIAN_SMALL, FONT_DEFAULT);
            break;
        default:
            break;
        }
        break;
    case ON_CHANGE_RELEASE:
        memset(lyrics_artist_name_array, '\n', sizeof(lyrics_artist_name_array));
        break;
    default:
        break;
    }
    return 0;
}


static void music_status_check(void *p)
{
    struct app_mode *cur_mode;
    u32 cur_window_id = UI_GET_WINDOW_ID();
    extern u8 bt_a2dp_get_status(void);
    cur_mode = app_get_current_mode();
    ASSERT(cur_mode);
#if TCFG_APP_BT_EN
    if ((cur_mode->name == APP_MODE_BT) && (__this->mode == PHONE_MUSIC_MODE) && (__this->play_sel == EARPHONE_PLAY)) {
        if (!sbox_ble_connect_flag_get()) {
            ui_pic_show_image_by_id(MUSIC_PLAY_PIC, 0);
            return ;
        }
        int state = sbox_music_ui_state_get();
        if (state == MUSIC_STATE_PLAY) {
            ui_pic_show_image_by_id(MUSIC_PLAY_PIC, 2);
        } else if (state == MUSIC_STATE_PAUSE) {
            ui_pic_show_image_by_id(MUSIC_PLAY_PIC, 1);
        }
        return ;
    }
    if ((cur_mode->name == APP_MODE_BT) && (__this->mode == PHONE_MUSIC_MODE) && (__this->play_sel == LOCAL_PLAY)) {
        u8 a2dp_state = bt_a2dp_get_status();
        log_debug("a2dp_state :%d \n", a2dp_state);
        if (a2dp_state == BT_MUSIC_STATUS_STARTING) {
            ui_pic_show_image_by_id(MUSIC_PLAY_PIC, 2);
        } else if (a2dp_state == BT_MUSIC_STATUS_SUSPENDING) {
            ui_pic_show_image_by_id(MUSIC_PLAY_PIC, 1);
        } else {
            ui_pic_show_image_by_id(MUSIC_PLAY_PIC, 0);
        }
        return ;

    }
#endif

#if TCFG_APP_MUSIC_EN
    if ((cur_mode->name == APP_MODE_MUSIC) || (__this->mode == LOCAL_MODE)) {
        if (dev_manager_get_total(1)) {
            if (music_player_get_play_status()) {
                ui_pic_show_image_by_id(MUSIC_PLAY_PIC, 2);
            } else {
                ui_pic_show_image_by_id(MUSIC_PLAY_PIC, 1);
            }
        } else {
            ui_pic_show_image_by_id(MUSIC_PLAY_PIC, 0);
        }
        return ;
    }
#endif

}
static int music_list_ontouch(void *ctr, struct element_touch_event *e)
{
#if TCFG_APP_MUSIC_EN
    struct ui_pic *pic = (struct ui_pic *)ctr;
    struct app_mode *cur_mode;
    log_info("func:%s ,e->event :%d , e->move_flag : %d \n ", __func__, e->event, e->move_flag);
    static u8 flag = 0;
    const char *sd_name = "SD MUSIC";
    const char *phone_name = "PHONE MUSIC";
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
    case ELM_EVENT_TOUCH_UP:
        if (e->move_flag) {
            return false;
        }

        if (!dev_manager_get_total(1)) {// 获取有效可播放设备数量
            return -1;
        }
        music_ui_layout_switch(LOCAL_LIST);
        break;
    default :
        break;
    }
#endif
    return false;
}

static int music_list_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct element *elm = (struct element *)_ctrl;
    switch (event) {
    case ON_CHANGE_INIT:

#if TCFG_APP_MUSIC_EN
        if (music_mode == LOCAL_MODE) {
            elm->css.invisible = 0;
            /* ui_show(MUSIC_LIST_BUTTON); */
        } else
#endif
        {
            elm->css.invisible = 1;
            /* ui_hide(MUSIC_LIST_BUTTON); */
        }
        ui_core_redraw(elm);
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        break;
    }
    return 0;
}

static int sd_music_text_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct element *elm = (struct element *)_ctrl;
    switch (event) {
    case ON_CHANGE_INIT:

#if !TCFG_APP_MUSIC_EN
        elm->css.invisible = 1;
        /* ui_show(MUSIC_LIST_BUTTON); */
#endif
        /* ui_hide(MUSIC_LIST_BUTTON); */
        ui_core_redraw(elm);
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        break;
    }
    return 0;
}

static int music_mode_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct element *elm = (struct element *)_ctrl;
    switch (event) {
    case ON_CHANGE_INIT:
        ui_card_disable();
        break;
    case ON_CHANGE_RELEASE:
        ui_card_enable();
        break;
    default:
        break;
    }
    return 0;
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

static int music_page_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    switch (event) {
    case ON_CHANGE_INIT:
        music_sd_memory_malloc();
        __this->core = MUSIC_PLAYER;
#if TCFG_APP_MUSIC_EN
        if (music_mode == LOCAL_MODE) {
            __this->mode = LOCAL_MODE;
        } else
#endif
        {
            __this->mode = PHONE_MUSIC_MODE;
        }
        if (music_play_sel == EARPHONE_PLAY) {
            __this->play_sel = EARPHONE_PLAY;
        } else {
            __this->play_sel = LOCAL_PLAY;
        }
        struct draw_context dc = {0};
        struct rect rect;
        /* ui_core_get_draw_context(&dc, &window->elm, &rect); */
#if TCFG_APP_BT_EN
        bt_register_lyric_callback(ui_bt_lyric_cb);
        bt_cmd_prepare(USER_CTRL_AVCTP_OPID_GET_MUSIC_INFO, 0, NULL);
#endif
        ui_register_msg_handler(ID_WINDOW_MUSIC_PLAYER, ui_msg_handler);
        timer_id = sys_timer_add(NULL, music_status_check, 500);

        break;
    case ON_CHANGE_RELEASE:
        sys_timer_del(timer_id);
        music_sd_memory_free();
        break;
    default:
        break;
    }
    return 0;
}

/**
 * @func : 音乐标题显示逻辑
 */
static int CS_MUSIC_TEXT_ontouch(void *ctr, struct element_touch_event *e)
{
    log_info("func:%s ,e->event :%d , e->move_flag : %d \n ", __func__, e->event, e->move_flag);
    switch (e->event) {
    case  ELM_EVENT_TOUCH_UP:
        if (!e->move_flag) {
            ui_hide_curr_main();
            UI_SHOW_WINDOW(ID_WINDOW_MUSIC_LRC);
        }
        break;
    }
    return false;
}

void ui_music_window_bt_status_update(void)
{
    struct ui_pic *pic = NULL;
    if (music_mode == PHONE_MUSIC_MODE && music_play_sel == EARPHONE_PLAY) {
        pic = (struct ui_pic *)ui_core_get_element_by_id(MUSIC_PRE_PIC);
        if (pic) {
            ui_pic_set_image_index(pic, sbox_ble_connect_flag_get() ? ICON_STATUS_CONNECT : ICON_STATUS_DISCONNECT);
        }

        pic = (struct ui_pic *)ui_core_get_element_by_id(MUSIC_PLAY_PIC);
        if (pic) {
            ui_pic_set_image_index(pic, sbox_ble_connect_flag_get() ? ICON_STATUS_CONNECT : ICON_STATUS_DISCONNECT);
        }

        pic = (struct ui_pic *)ui_core_get_element_by_id(MUSIC_NEXT_PIC);
        if (pic) {
            ui_pic_set_image_index(pic, sbox_ble_connect_flag_get() ? ICON_STATUS_CONNECT : ICON_STATUS_DISCONNECT);
        }
    }
}

static int music_mode_list_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    int col, row;
    u8 grid_music_mode = (music_mode << 1) | music_play_sel;
    switch (e) {
    case ON_CHANGE_INIT:
        /* app_audio_set_volume(APP_AUDIO_STATE_MUSIC, 40, 0); */
        ui_grid_set_slide_direction(grid, SCROLL_DIRECTION_UD);
        /* set_ui_sys_param(ConnNewPhone, 0); */
        struct scroll_area area = {0, 0, 10000, 10000};
        ui_grid_set_scroll_area(grid, &area);
        ui_grid_flick_ctrl_close(grid, 1);
        switch (grid_music_mode) {
        case GRID_PHONE_MUSIC_EAR_PLAY:
            grid->hi_index = PHONE_MUSIC_EAR_PLAY;
            break;
        case GRID_PHONE_MUSIC_LOCAL_PLAY:
            grid->hi_index = PHONE_MUSIC_LOCAL_PLAY;
            break;
        case GRID_LOCAL_MUSIC_LOCAL_PLAY:
            grid->hi_index = LOCAL_MUSIC_LOCAL_PLAY;
            break;
        case GRID_LOCAL_MUSIC_EAR_PLAY:
            grid->hi_index = LOCAL_MUSIC_EAR_PLAY;
            break;

        }

        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        break;
    default:
        return false;
    }
    return false;
}
/* static   u8 mac_buf[6] = {0xB0, 0xBD, 0x17, 0x75, 0xBE, 0x3F}; */
/* static   u8 mac_buf[6] = {0x3F,0xBE,0X75,0X17,0XBD,0XB0}; */
void bt_emitter_disconnect_deal(void *priv)
{
    /* u16 *timer_id=(u16 *)priv; */
    extern u16 emitter_disconnect_timer;
    emitter_disconnect_timer = 0;
    if (music_mode != LOCAL_MODE || music_play_sel != EARPHONE_PLAY) {
        return ;
    }
    if (__this != NULL) {
        __this->mode = PHONE_MUSIC_MODE;
        __this->play_sel = EARPHONE_PLAY;
    }
    if (is_bredr_close() == 0) {
        /* bt_cmd_prepare(USER_CTRL_AVCTP_PAUSE_MUSIC, 0, NULL); */
        bt_close_bredr();
    }
    music_mode = PHONE_MUSIC_MODE;
    music_play_sel = EARPHONE_PLAY;
    /* extern int clock_free(char *name); */
    /* clock_free("bt_scan"); */

    app_send_message(APP_MSG_GOTO_MODE, APP_MODE_BT);

}
u8 bt_emitter_need_reconnect()
{

    if (music_mode != LOCAL_MODE && music_play_sel != LOCAL_PLAY) {
        return false;
    } else {
        return true;
    }
}
#if TCFG_APP_MUSIC_EN
void entry_mode_err_deal()
{

    __this->mode = PHONE_MUSIC_MODE;
    __this->play_sel = EARPHONE_PLAY;
    if (is_bredr_close() == 0) {
        /* bt_cmd_prepare(USER_CTRL_AVCTP_PAUSE_MUSIC, 0, NULL); */
        bt_close_bredr();
    }
    music_mode = __this->mode;
    music_play_sel = __this->play_sel;
    if (get_cur_connect_emitter_mac_addr()) {
        custom_client_send_ctrl_edr_conn(1);
        custom_client_send_ctrl_edr_conn(2);
    }
    app_send_message(APP_MSG_GOTO_MODE, APP_MODE_BT);

}
void cut_in_music_mode()
{
    app_send_message(APP_MSG_GOTO_MODE, APP_MODE_MUSIC);
}

void check_emitter_connect_status()
{
    static u8 cnt = 0;
    u8 mac_buf[6];
    sbox_get_earphone_mac(mac_buf);
    if (get_cur_connect_emitter_mac_addr() != NULL) {
        cnt = 0;
        /* app_send_message(APP_MSG_GOTO_MODE, APP_MODE_MUSIC); */
        custom_client_send_ctrl_edr_info();
        bt_emitter_connecting_flag = 0;
        __this->mode = LOCAL_MODE;
        __this->play_sel = EARPHONE_PLAY;
        music_play_sel = __this->play_sel;
        music_mode = __this->mode;
        /* app_send_message(APP_MSG_GOTO_MODE, APP_MODE_MUSIC); */
        sys_timeout_add(NULL, cut_in_music_mode, 1500);
    } else if (++cnt >= 100) {
        entry_mode_err_deal();

        log_info("%s connect err!!!!", __FUNCTION__);
        bt_emitter_connecting_flag = 0;
        cnt = 0;
    } else {
        struct file_player *file_player = NULL;     // 若连接蓝牙发射，先暂停本地音乐播放
        file_player = get_music_file_player();
        if (music_file_get_player_status(file_player) == FILE_PLAYER_START) {
            app_send_message(APP_MSG_MUSIC_PP, 0);
        }
        emitter_bt_connect(mac_buf);
        sys_timeout_add(NULL, (void (*)(void *priv))check_emitter_connect_status, 100);
    }
}


#endif
static int music_mode_list_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    int sel_item;
    static u8 move_flag = 0;
    u8 *emitter_mac;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        move_flag = 0;
        return true;
    case ELM_EVENT_TOUCH_UP:
        if (move_flag) {
            move_flag = 0;
            return true;
        }
        sel_item = grid->touch_index ;
        struct file_player *file_player = NULL;     // 若连接蓝牙发射，先暂停本地音乐播放
#if TCFG_APP_MUSIC_EN
        file_player = get_music_file_player();
        if (music_file_get_player_status(file_player) == FILE_PLAYER_START) {
            app_send_message(APP_MSG_MUSIC_PP, 0);
        }
#endif
        switch (sel_item) {
#if TCFG_APP_MUSIC_EN
        case LOCAL_MUSIC_EAR_PLAY:
            if (!sbox_ble_connect_flag_get()) {
                log_info("no connect earphone ble");
                break;
            }
            if (bt_emitter_connecting_flag == 1) {
                log_info("emitter connecting !!!");
                break;
            }

            if (is_bredr_close() == 0) {
                /* bt_cmd_prepare(USER_CTRL_AVCTP_PAUSE_MUSIC, 0, NULL); */
                bt_close_bredr();
            }
            /* app_send_message(APP_MSG_GOTO_MODE, APP_MODE_MUSIC); */
            emitter_mac = get_cur_connect_emitter_mac_addr();
            if (!emitter_mac && sbox_bt_connect_flag_get()) {
                custom_client_send_ctrl_edr_conn(1);
            }

            /* app_send_message(APP_MSG_GOTO_MODE, APP_MODE_BT); */
            bt_emitter_connecting_flag = 1;
            sys_timeout_add(NULL, (void (*)(void *priv))check_emitter_connect_status, 100);
            break;
        case LOCAL_MUSIC_LOCAL_PLAY:
            __this->mode = LOCAL_MODE;
            __this->play_sel = LOCAL_PLAY;
            if (music_play_sel == __this->play_sel && music_mode == __this->mode) {
                file_player = get_music_file_player();
                if (music_file_get_player_status(file_player) == FILE_PLAYER_PAUSE && bt_emitter_connecting_flag == 0) {
                    app_send_message(APP_MSG_MUSIC_PP, 0);
                }
                log_info("sel same mode !!!");

                break;
            }
            if (is_bredr_close() == 0) {
                /* bt_cmd_prepare(USER_CTRL_AVCTP_PAUSE_MUSIC, 0, NULL); */
                bt_close_bredr();
            }
            music_mode = __this->mode;
            music_play_sel = __this->play_sel;
            if (get_cur_connect_emitter_mac_addr()) {
                custom_client_send_ctrl_edr_conn(1);
                custom_client_send_ctrl_edr_conn(2);
            }
            /* app_send_message(APP_MSG_GOTO_MODE, APP_MODE_MUSIC); */
            sys_timeout_add(NULL, cut_in_music_mode, 600);
            break;
#endif
        case PHONE_MUSIC_EAR_PLAY:
            __this->mode = PHONE_MUSIC_MODE;
            __this->play_sel = EARPHONE_PLAY;
            if (music_play_sel == __this->play_sel && music_mode == __this->mode) {
                log_info("sel same mode !!!");
                break;
            }
            if (is_bredr_close() == 0) {
                /* bt_cmd_prepare(USER_CTRL_AVCTP_PAUSE_MUSIC, 0, NULL); */
                bt_close_bredr();
            }
            music_mode = __this->mode;
            music_play_sel = __this->play_sel;
            if (get_cur_connect_emitter_mac_addr()) {
                custom_client_send_ctrl_edr_conn(1);
                custom_client_send_ctrl_edr_conn(2);
            }
            app_send_message(APP_MSG_GOTO_MODE, APP_MODE_BT);
            break;

        case PHONE_MUSIC_LOCAL_PLAY:
            __this->mode = PHONE_MUSIC_MODE;
            __this->play_sel = LOCAL_PLAY;
            if (music_play_sel == __this->play_sel && music_mode == __this->mode && is_bredr_close() == 0) {
                log_info("sel same mode !!!");
                break;
            }
            music_mode = __this->mode;
            music_play_sel = __this->play_sel;
            if (get_cur_connect_emitter_mac_addr()) {
                custom_client_send_ctrl_edr_conn(1);
                /* custom_client_send_ctrl_edr_conn(2); */
            }
            bt_init_bredr();
            app_send_message(APP_MSG_GOTO_MODE, APP_MODE_BT);
            break;
        default:
            return true;

            break;
        }
        ui_hide(MUSIC_MODE_LAYOUT);
        ui_show(MUSIC_CONTROL_LAYOUT);
        /* sys_timeout_add(NULL, music_player_status_init, 700); */
        /* UI_MSG_POST("UI_MUSIC_STATUS"); */
        /* if() */
        break;
    case ELM_EVENT_TOUCH_MOVE:
        move_flag = 1;
        return true;

        break;
    case ELM_EVENT_TOUCH_R_MOVE:
        ui_hide(MUSIC_MODE_LAYOUT);
        ui_show(MUSIC_CONTROL_LAYOUT);
        return true;
        break;
    default :
        break;
    }
    return true;
}
static int ui_sd_music_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:
        struct element *local_elm = ui_core_get_element_by_id(MUSIC_LOCAL_LAYOUT);
        music_change_layout_css(LOCAL_LIST, local_elm);
#if TCFG_LRC_LYRICS_ENABLE&&TCFG_APP_MUSIC_EN
        if (__this && !__this->timer) {
            __this->timer = sys_timer_add(NULL, ui_sd_music_show_lyrics, 500);
        }
#endif
        break;
    case ON_CHANGE_RELEASE:
#if TCFG_LRC_LYRICS_ENABLE
        if (__this && __this->timer) {
            sys_timer_del(__this->timer);
            __this->timer = 0;
        }
#endif
        break;
    default:
        return false;
    }
    return false;
}


REGISTER_UI_EVENT_HANDLER(MUSIC_PLAY_PIC)
.onchange = music_pp_onchange,
 .onkey = NULL,
  .ontouch = music_pp_ontouch,
};
REGISTER_UI_EVENT_HANDLER(MUSIC_PRE_PIC)
.onchange = music_prev_onchange,
 .onkey = NULL,
  .ontouch = music_prev_ontouch,
};

REGISTER_UI_EVENT_HANDLER(MUSIC_NEXT_PIC)
.onchange = music_next_onchange,
 .onkey = NULL,
  .ontouch = music_next_ontouch,
};

REGISTER_UI_EVENT_HANDLER(MUSIC_TITLE_TEXT)
.onchange = music_text_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(ID_WINDOW_MUSIC_PLAYER)
.onchange = music_page_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(MUSIC_CONTROL_LAYOUT)
.onchange = ui_sd_music_onchange,
 .onkey = NULL,
  .ontouch = ui_sd_music_enter_ontouch,
};

REGISTER_UI_EVENT_HANDLER(MUSIC_CHANGE_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = music_change_ontouch,
};

REGISTER_UI_EVENT_HANDLER(MUSIC_MODE_LAYOUT)
.onchange = music_mode_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(MUSIC_MODE_LIST)
.onchange = music_mode_list_onchange,
 .onkey = NULL,
  .ontouch = music_mode_list_ontouch,
};
REGISTER_UI_EVENT_HANDLER(MUSIC_LIST_BUTTON)
.onchange = music_list_onchange,
 .onkey = NULL,
  .ontouch = music_list_ontouch,
};
REGISTER_UI_EVENT_HANDLER(SD_MUSIC_LOCAL_PLAY)
.onchange = sd_music_text_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(SD_MUSIC_EAR_PLAY)
.onchange = sd_music_text_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

#if (defined TCFG_UI_UP_MOVE_DEMO_ENABLE && TCFG_UI_UP_MOVE_DEMO_ENABLE)
//  上划操作统一接口
static int ui_up_move_deal(void *p)
{
    log_info("func:%s \n", __func__);
    UI_HIDE_CURR_WINDOW();
    UI_SHOW_WINDOW(ID_WINDOW_MUSIC_LRC);
    return 0;
}

REGISTER_UP_MOVE_PAGE(music) = {
    .id = ID_WINDOW_MUSIC_PLAYER,
    .pram = NULL,
    .callback = ui_up_move_deal,
};
#endif
#endif
#endif
