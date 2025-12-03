#include "app_config.h"
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
#include "tone_player.h"
#include "audio_config.h"



#define LOG_TAG_CONST       UI
#define LOG_TAG     		"[UI_SLEEPAID]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ui_action_sleepaid.data.bss")
#pragma data_seg(".ui_action_sleepaid.data")
#pragma const_seg(".ui_action_sleepaid.text.const")
#pragma code_seg(".ui_action_sleepaid.text")
#endif

#if (defined (CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE))
#if TCFG_UI_ENABLE_SLEEPAID

#define STYLE_NAME  JL

REGISTER_UI_STYLE(STYLE_NAME)

typedef struct {
    u8 file_name[10];     //long file name
} FILE_INFO;

typedef struct {
    u32 sleepaid_con;	//助眠播放器的状态；0：暂停；1：播放
    u32 sleepaid_time;	//助眠播放器定时时长(单位：min)
    u32 cur_layout;
    u32 cur_music; 		//当前音乐
    u32 cur_volume;		//当前音量
    u32 cur_music_time;	//当前播放到音乐的哪个时间点
    u32 list_count;		//音乐菜单中的音乐数量
    FS_DIR_INFO *dir_buf;	//播放目录下的文件信息
    FILE_INFO *file_buf;
    FILE *file;
    u16 timer_id;
} SLEEPAID_UI_PARAM;

static SLEEPAID_UI_PARAM *sleepaid_ui_handler = NULL;

#define __this		sleepaid_ui_handler

#define SLEEPAID_FILE_ROOT			"mnt/sdfile/res/"
#define SLEEPAID_FILE_PATH 			"sleepaid"					//存放助眠曲的文件夹
#define SLEEPAID_FILE_FORMAT 		"WTG"						//助眠曲的格式

#define SLEEPAID_KEY_STEP           5                           // 旋钮调节步幅
#define MIN_UNIT                    60*1000                     //分钟单位ms


int music_is_play(void);

static void sleepaid_music_pause(void);
static void sleepaid_play_con_set(u8 con);
static void sleepaid_music_play(void);

static const u32 optbox_content[] = {15, 30, 60};

static u32 ui_list_get_child_id(u32 list_id, u32 index)
{
    struct ui_grid *grid;
    struct element *elm;

    grid = (struct ui_grid *)ui_core_get_element_by_id(list_id);
    elm = (struct element *)(&grid->item[index]);

    return elm->id;
}

static u32 ui_list_get_child_num(u32 list_id)
{
    struct ui_grid *grid;
    struct element *elm;
    u32 row_num;

    grid = (struct ui_grid *)ui_core_get_element_by_id(list_id);
    row_num = grid->row_num;

    return row_num;
}

static u32 ui_if_hignlight(u32 id)
{
    struct element *elm;

    elm = ui_core_get_element_by_id(id);
    return elm->highlight;
}

static u32 sleepaid_get_cur_volume_percent(void)
{
    u32 percent;
    s8 volume = app_audio_get_volume(APP_AUDIO_STATE_WTONE);
    s8 max_volume = app_audio_volume_max_query(SysVol_TONE);

    percent = volume * 100 / max_volume;
    return percent;
}

static void sleepaid_set_volume(u32 percent)
{
    s8 max_volume = app_audio_volume_max_query(SysVol_TONE);
    s8 volume = percent * max_volume / 100;

    /* printf("<%s>----max_volume:%d, volume:%d\n", __func__, max_volume, volume); */
    app_audio_set_volume(APP_AUDIO_STATE_WTONE, volume, 1);
    __this->cur_volume = percent;
}

static void sleepaid_timeout_cb(void *p)
{
    log_info("%s %d", __func__, __this->sleepaid_con);
    if (__this->sleepaid_con == 1) {
        ui_auto_shut_down_enable();
        sleepaid_music_pause();
        sleepaid_play_con_set(0);
    }
    __this->timer_id = 0;
}

static void sleepaid_time_start(void)
{
    log_info("%s %d", __func__, __this->sleepaid_con);
    if (__this->sleepaid_con == 0) {
        ui_auto_shut_down_disable();
        if (music_is_play() == true) {
            app_send_message(APP_MSG_MUSIC_PP, 0);
        }
        sleepaid_music_play();
        sleepaid_play_con_set(1);
    }
    __this->timer_id = sys_timeout_add(NULL, sleepaid_timeout_cb, __this->sleepaid_time * MIN_UNIT);
}

static void sleepaid_time_stop(void)
{
    log_info("%s", __func__);
    if (__this->timer_id) {
        sys_timeout_del(__this->timer_id);
        __this->timer_id = 0;
    }
}

static void sleepaid_parm_init(void)
{
    if (__this) {
        printf("size of parm:%lu\n", sizeof(SLEEPAID_UI_PARAM));
        memset(__this, 0, sizeof(SLEEPAID_UI_PARAM));
    }

    __this->cur_layout = SLEEPAID_MAIN_LAYOUT;
    __this->cur_volume = sleepaid_get_cur_volume_percent();
}

//一个page中含有多个布局需要跳转，使用一个数组来管理
static u32 sleepaid_layout[] = {
    SLEEPAID_MAIN_LAYOUT,
    SLEEPAID_TIME_LAYOUT,
    SLEEPAID_MUSICLIST_LAYOUT
};

static void sleepaid_layout_show(u32 layout_id)
{
    u32 layout_num;

    layout_num = sizeof(sleepaid_layout) / sizeof(u32);

    for (int i = 0; i < layout_num; i++) {
        if (layout_id == sleepaid_layout[i]) {
            ui_hide(__this->cur_layout);
            ui_show(sleepaid_layout[i]);
            __this->cur_layout = layout_id;
            break;
        } else if (i == layout_num - 1) {
            return;
        }
    }


}

static void sleepaid_play_con_set(u8 con)
{
    if (con == 0) {
        __this->sleepaid_con = 0;
        ui_show(SLEEPAID_START_BUTTON);
        ui_hide(SLEEPAID_PAUSE_BUTTON);
    } else if (con == 1) {
        __this->sleepaid_con = 1;
        ui_show(SLEEPAID_PAUSE_BUTTON);
        ui_hide(SLEEPAID_START_BUTTON);
    }
}

static int grid_child_cb(void *_ctrl, int id, int type, int index)
{
    FILE_INFO *info;
    if (index < __this->list_count) {
        info = &__this->file_buf[index];
    }
    /* FS_DIR_INFO *info = file_list_read_by_index(index); */
    switch (type) {
    case CTRL_TYPE_PROGRESS:
        break;
    case CTRL_TYPE_MULTIPROGRESS:
        break;
    case CTRL_TYPE_TEXT:
        struct ui_text *text = (struct ui_text *)_ctrl;

        if (!info) {
            return 0;
        }
        /* put_buf(info->file_name, strlen((char *)info->file_name)); */
        printf("id:0x%x, file_name:%s, len:%lu\n", text->elm.id, info->file_name, strlen((char *)info->file_name));
        if (!strcmp(text->source, "name")) {
            ui_text_set_utf8_str(text, UI_TEXT_ENCODE_TEXT, (char *)info->file_name, strlen((char *)info->file_name), FONT_DEFAULT);
            /* ui_text_set_text_by_id(text->elm.id, (char *)info->file_name, strlen((char *)info->file_name), FONT_DEFAULT); */
        }

        break;
    case CTRL_TYPE_NUMBER:
        break;
    case CTRL_TYPE_PIC:
        struct ui_pic *pic = (struct ui_pic *)_ctrl;

        if (!info) {
            return 0;
        }

        if (__this->cur_music == index) {
            ui_show(pic->elm.id);
        } else {
            ui_hide(pic->elm.id);
        }
        break;
    case CTRL_TYPE_TIME:
        break;
    }
    return 0;
}


static void file_list_init(char *path, char *type)
{
    /* printf("<%s>----------path:%s\n", __func__, path); */
    struct vfscan *fs = NULL;
    char fscan_parm[50];
    u8 file_name[20];
    int row = 0;
    int col = 0;
    FILE *file;
    FILE_INFO *info;

    sprintf(fscan_parm, "-t%s -sn -ar -d", type);
    fs = fscan(path, fscan_parm, 9);
    if (fs) {
        printf("file numb:%d\n", fs->file_number);
        __this->list_count = fs->file_number;
        __this->file_buf = zalloc(sizeof(FILE_INFO) *  __this->list_count);
    }

    for (int i = 0; i < __this->list_count; i++) {
        file = fselect(fs, FSEL_BY_NUMBER, i + 1);
        if (!file) {
            break;		//文件已经扫描结束
        }
        info = &__this->file_buf[i];
        fget_name(file, info->file_name, 10);
        fclose(file);
    }

    if (fs) {
        fscan_release(fs);
    }
    return;
}

u32 ui_optbox_get_opt(u32 optbox_id)
{
    u32 num;
    u32 id;

    num = ui_list_get_child_num(optbox_id);

    for (int i = 0; i < num; i++) {
        id = ui_list_get_child_id(optbox_id, i);
        if (ui_if_hignlight(id)) {
            return i;
        }
    }
    return num;
}

static int sleepaid_time_vlist_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    /* log_info("%s", __func__); */
    struct element *elm = (struct element *)_ctrl;
    u32 opt_index;

    switch (event) {
    case ON_CHANGE_INIT:
        ui_grid_energy_auto_center((struct ui_grid *)elm, AUTO_CENTER_MODE2);
        break;
    case ON_CHANGE_SHOW:
        break;
    case ON_CHANGE_FIRST_SHOW:
        break;
    case ON_CHANGE_SHOW_POST:
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        break;
    }
    return false;
}

static int sleepaid_time_vlist_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    int sel_item;
    u32 opt_index;

    switch (e->event) {
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_UP:
        break;
    case ELM_EVENT_TOUCH_U_MOVE:
        break;
    default:
        break;
    }
    return false;//接管消息
}


REGISTER_UI_EVENT_HANDLER(SLEEPAID_TIME_VLIST)
.onchange = sleepaid_time_vlist_onchange,
 .onkey = NULL,
  .ontouch = sleepaid_time_vlist_ontouch,
};

static int sleepaid_ok_button_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    int sel_item;
    u32 opt_index;

    switch (e->event) {
    case ELM_EVENT_TOUCH_MOVE:
        putchar('b');
        break;
    case ELM_EVENT_TOUCH_DOWN:
        putchar('a');
        break;
    case ELM_EVENT_TOUCH_UP:
        putchar('c');
        //更新选项框的值
        opt_index = ui_optbox_get_opt(SLEEPAID_TIME_VLIST);
        __this->sleepaid_time = optbox_content[opt_index];
        printf("sleepaid time:%d\n\n",  __this->sleepaid_time);
        sleepaid_time_start();
        sleepaid_layout_show(SLEEPAID_MAIN_LAYOUT);
        break;
    case ELM_EVENT_TOUCH_U_MOVE:
        break;
    default:
        break;
    }
    return false;//接管消息
}


REGISTER_UI_EVENT_HANDLER(SLEEPAID_OK_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = sleepaid_ok_button_ontouch,
};

static int sleepaid_curvolume_slider_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    /* log_info("%s", __func__); */
    struct element *elm = (struct element *)_ctrl;
    u32 opt_index;
    struct ui_slider *slider;

    switch (event) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_SHOW:
        break;
    case ON_CHANGE_FIRST_SHOW:
        slider = (struct ui_slider *)elm;

        ui_slider_set_persent(slider, __this->cur_volume);
        break;
    case ON_CHANGE_SHOW_POST:
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        break;
    }
    return false;
}

static int sleepaid_curvolume_slider_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_slider *slider = (struct ui_slider *)ctr;
    int sel_item;
    u32 opt_index;
    struct rect rect;

    switch (e->event) {
    case ELM_EVENT_TOUCH_R_MOVE:
        printf("curvolume touch rmove");
        ui_core_get_element_abs_rect((struct element *)ctr, &rect);
        int ret = in_rect(&rect, &e->origin);
        if (ret == 0) {
            return false;
        } else if (ret == 1) {
            return true;
        }
        break;
    case ELM_EVENT_TOUCH_MOVE:
        slider_touch_slider_move(slider, e);
        u32 percent = slider_get_percent(slider);
        printf("set percent:%d\n", percent);
        sleepaid_set_volume(percent);

        break;
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_UP:
        printf("curvolume touch up!!!");
        break;
    case ELM_EVENT_TOUCH_U_MOVE:
        break;
    default:
        break;
    }
    return true;//接管消息
}

static int sleepaid_curvolume_slider_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_slider *slider = ui_slider_for_id(SLEEPAID_CURVOLUME_SLIDER);
    int percent = 0;
    switch (e->value) {
    case KEY_UI_PLUS:
        percent = slider_get_percent(slider);
        percent += SLEEPAID_KEY_STEP;
        if (percent >= 100) {
            percent = 100;
        }
        sleepaid_set_volume(percent);
        ui_slider_set_persent_by_id(SLEEPAID_CURVOLUME_SLIDER, __this->cur_volume);
        return true;
    case KEY_UI_MINUS:
        percent = slider_get_percent(slider);
        percent -= SLEEPAID_KEY_STEP;
        if (percent <= 0) {
            percent = 0;
        }
        sleepaid_set_volume(percent);
        ui_slider_set_persent_by_id(SLEEPAID_CURVOLUME_SLIDER, __this->cur_volume);
        return true;
    default:
        break;
    }

    return false;
}

REGISTER_UI_EVENT_HANDLER(SLEEPAID_CURVOLUME_SLIDER)
.onchange = sleepaid_curvolume_slider_onchange,
 .onkey = sleepaid_curvolume_slider_onkey,
  .ontouch = sleepaid_curvolume_slider_ontouch,
};

static int musiclist_child_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct element *k ;
    struct element *elm = (struct element *)_ctrl;
    if (event == ON_CHANGE_UPDATE_ITEM) {
        int index = (u32)arg;
        printf("index:%d\n", index);
        grid_child_cb(elm, elm->id, ui_id2type(elm->id), index);

    }
    return 0;
}

static int sleepaid_musiclist_vlist_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    /* log_info("%s", __func__); */
    struct element *elm = (struct element *)_ctrl;
    u32 opt_index;
    struct ui_slider *slider;
    struct ui_grid *grid = (struct ui_grid *)ui_core_get_element_by_id(SLEEPAID_MUSICLIST_VLIST);
    struct ui_grid_dynamic *dynamic;

    switch (event) {
    case ON_CHANGE_INIT:
        printf("MUSIC LIST init!!!");
        ui_set_default_handler(elm, NULL, NULL, musiclist_child_onchange);
        ui_grid_set_slide_direction(grid, SCROLL_DIRECTION_UD);
        struct scroll_area area = {0, 0, 10000, 10000};
        ui_grid_set_scroll_area(grid, &area);
        ui_grid_flick_ctrl_close(grid, 1);
        //根据文件数量创建动态列表
        int row = __this->list_count;
        int col = 1;
        ui_grid_init_dynamic(grid, &row, &col);
        break;
    case ON_CHANGE_FIRST_SHOW:
        printf("MUSIC LIST FIRST SHOW!!!");
        break;
    case ON_CHANGE_SHOW:
        printf("MUSIC LIST SHOW!!!");
        break;
    case ON_CHANGE_SHOW_POST:
        break;
    case ON_CHANGE_RELEASE:
        printf("MUSIC LIST RELEASE!!!");
        break;
    default:
        break;
    }
    return false;
}
static int sleepaid_musiclist_vlist_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    int sel_item;
    struct ui_grid_dynamic *dynamic;


    switch (e->event) {
    case ELM_EVENT_TOUCH_R_MOVE:
        break;
    case ELM_EVENT_TOUCH_L_MOVE:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_UP:
        if (!e->move_flag) {
            printf("MUSIC LIST TOUCH UP");
            dynamic = grid->dynamic;
            printf("%d, %d, %d, %d\n", dynamic->min_row_index, dynamic->max_row_index, dynamic->min_show_row_index, dynamic->max_show_row_index);
            sel_item = ui_grid_cur_item_dynamic(grid);
            u32 last_music = __this->cur_music;
            __this->cur_music = sel_item;
            printf("sel_item；%d\n", sel_item);

            /* ui_grid_set_hindex_dynamic(grid, __this->cur_music, 0, 0); */

#if 1
            if (last_music >= dynamic->min_row_index && last_music <= dynamic->max_row_index) {
                ui_grid_update_by_id_dynamic(grid->elm.id, last_music - dynamic->min_row_index, true);
            }
            ui_grid_update_by_id_dynamic(grid->elm.id, __this->cur_music - dynamic->min_row_index, true);
#endif


        }
        break;
    default:
        break;
    }
    return false;
    /* return true;//接管消息 */
}

REGISTER_UI_EVENT_HANDLER(SLEEPAID_MUSICLIST_VLIST)
.onchange = sleepaid_musiclist_vlist_onchange,
 .onkey = NULL,
  .ontouch = sleepaid_musiclist_vlist_ontouch,

};


static int sleepaid_play_end_callback(void *priv, enum stream_event event)
{
    if (event == STREAM_EVENT_STOP && __this) {
        /* printf("The callback is runned after play stop!!!\n"); */
        if (__this->sleepaid_con) {
            //播放下一首
            if (__this->cur_music >= __this->list_count - 1) {
                __this->cur_music = 0;
            } else {
                __this->cur_music++;
            }
            sleepaid_music_play();
        }
    }
    return 0;
}

static void sleepaid_music_play(void)
{
    int ret;
    char file_name[25];
    FILE_INFO *info;

    //获取当前需要播放的音乐
    info = &__this->file_buf[__this->cur_music];
    sprintf(file_name, SLEEPAID_FILE_PATH"/%s", info->file_name);
    /* printf("<%s>-------file_name:%s\n", __func__, file_name); */

    ret = play_tone_file_callback(file_name, NULL, sleepaid_play_end_callback);
    if (ret) {
        log_error("power on tone play err!!!");
    }
}

static void sleepaid_music_pause(void)
{
    tone_player_stop();
}

static int sleepaid_button_ontouch(void *ctr, struct element_touch_event *e)
{
    struct element *elm = (struct element *)ctr;

    switch (e->event) {
    case ELM_EVENT_TOUCH_R_MOVE:
        printf("sleep aid button rmove");
        break;
    case ELM_EVENT_TOUCH_L_MOVE:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_UP:
        switch (elm->id) {
        case SLEEPAID_TIME_BUTTON:
            printf("time button touch!!!");
            sleepaid_layout_show(SLEEPAID_TIME_LAYOUT);
            break;
        case SLEEPAID_START_BUTTON:
            printf("start button touch!!!");
            ui_auto_shut_down_disable();
            if (music_is_play() == true) {
                app_send_message(APP_MSG_MUSIC_PP, 0);
            }
            if (__this->sleepaid_con == 0) {
                sleepaid_play_con_set(1);
            } else {
                sleepaid_play_con_set(0);
            }
            sleepaid_music_play();
            break;
        case SLEEPAID_PAUSE_BUTTON:
            printf("pause button touch!!!");
            ui_auto_shut_down_enable();
            if (__this->sleepaid_con == 0) {
                sleepaid_play_con_set(1);
            } else {
                sleepaid_play_con_set(0);
            }
            sleepaid_music_pause();
            sleepaid_time_stop();
            break;
        case SLEEPAID_LIST_BUTTON:
            printf("list button touch!!!");
            sleepaid_layout_show(SLEEPAID_MUSICLIST_LAYOUT);
            break;
        }
        break;
    default:
        break;
    }
    return false;
    /* return true;//接管消息 */
}

REGISTER_UI_EVENT_HANDLER(SLEEPAID_TIME_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = sleepaid_button_ontouch,

};

REGISTER_UI_EVENT_HANDLER(SLEEPAID_START_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = sleepaid_button_ontouch,

};

REGISTER_UI_EVENT_HANDLER(SLEEPAID_PAUSE_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = sleepaid_button_ontouch,

};

REGISTER_UI_EVENT_HANDLER(SLEEPAID_LIST_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = sleepaid_button_ontouch,

};

static int sleepaid_layout_ontouch(void *ctr, struct element_touch_event *e)
{
    struct element *elm = (struct element *)ctr;

    switch (e->event) {
    case ELM_EVENT_TOUCH_R_MOVE:
        switch (elm->id) {
        case SLEEPAID_TIME_LAYOUT:
            printf("time layout touch!!!");
            sleepaid_layout_show(SLEEPAID_MAIN_LAYOUT);
            break;
        case SLEEPAID_MUSICLIST_LAYOUT:
            printf("musiclist layout touch!!!");
            sleepaid_layout_show(SLEEPAID_MAIN_LAYOUT);
            break;
        }
        break;
    case ELM_EVENT_TOUCH_L_MOVE:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_UP:
        break;
    default:
        break;
    }
    /* return false; */
    return true;//接管消息
}

REGISTER_UI_EVENT_HANDLER(SLEEPAID_TIME_LAYOUT)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = sleepaid_layout_ontouch,

};

REGISTER_UI_EVENT_HANDLER(SLEEPAID_MUSICLIST_LAYOUT)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = sleepaid_layout_ontouch,

};

static int sleepaid_main_layout_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    /* log_info("%s", __func__); */
    struct element *elm = (struct element *)_ctrl;
    u32 opt_index;
    struct ui_slider *slider;

    switch (event) {
    case ON_CHANGE_INIT:
        if (__this->sleepaid_con == 0) {
            sleepaid_play_con_set(0);
        } else {
            sleepaid_play_con_set(1);
        }
        break;
    case ON_CHANGE_SHOW:
        break;
    case ON_CHANGE_FIRST_SHOW:
        break;
    case ON_CHANGE_SHOW_POST:
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(SLEEPAID_MAIN_LAYOUT)
.onchange = sleepaid_main_layout_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int window_sleepaid_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    /* log_info("%s", __func__); */
    struct element *elm = (struct element *)_ctrl;
    struct unumber numb;
    struct draw_context *dc = (struct draw_context *)arg;
    struct rect rect;
    FILE_INFO *info;

    switch (event) {
    case ON_CHANGE_INIT:
        printf("ID_WINDOW_SLEEPAID init!!!");
        if (!__this) {
            __this = malloc(sizeof(SLEEPAID_UI_PARAM));
        }
        sleepaid_parm_init();

        //先获取助眠曲路径下的文件存放到__this->file_buf
        file_list_init(SLEEPAID_FILE_ROOT SLEEPAID_FILE_PATH, SLEEPAID_FILE_FORMAT);
#if 0
        printf("---------------------------------------------------\n");
        printf("file num:%d\n", __this->list_count);
        for (int i = 0; i < __this->list_count; i++) {
            info = &__this->file_buf[i];
            printf("file%d:%s\n", i, info->file_name);
        }
#endif
        break;
    case ON_CHANGE_RELEASE:
        printf("ID_WINDOW_SLEEPAID release!!!");
        ui_auto_shut_down_enable();
        if (__this->sleepaid_con == 1) {
            sleepaid_music_pause();
        }
        sleepaid_time_stop();
        //退出界面时，先释放存放文件信息的buf
        if (__this->file_buf) {
            free(__this->file_buf);
            __this->file_buf = NULL;
        }
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

REGISTER_UI_EVENT_HANDLER(ID_WINDOW_SLEEPAID)
.onchange = window_sleepaid_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};





#endif /* #if TCFG_UI_ENABLE_HEAT */
#endif

