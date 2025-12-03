#include "app_config.h"
#include "ui/ui_api.h"
#include "jlui/ui.h"
#include "jlui_app/ui_style.h"
#include "app_task.h"
#include "system/timer.h"
#include "app_main.h"
#include "init.h"
#include "key_event_deal.h"
#include "device/device.h"
#include "app_power_manage.h"
#include "btstack/avctp_user.h"
#include "asm/charge.h"
#include "rtc.h"
#include "rcsp_manage.h"
#include "jlui_app/ui_sys_param.h"

#define LOG_TAG_CONST       UI
#define LOG_TAG     		"[UI_SHORTCUT_MENU]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ui_action_shortcut_menu.data.bss")
#pragma data_seg(".ui_action_shortcut_menu.data")
#pragma const_seg(".ui_action_shortcut_menu.text.const")
#pragma code_seg(".ui_action_shortcut_menu.text")
#endif

#if (defined (CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE))
#if TCFG_UI_ENABLE_SHORTCUT_MENU

#define STYLE_NAME  JL

/* REGISTER_UI_STYLE(STYLE_NAME) */


extern u8 is_bredr_close();
extern void bt_close_bredr();
extern void bt_init_bredr();
extern void bredr_conn_last_dev();
extern void set_is_raise_hand(int flag);
extern u8 get_is_raise_hand();
extern u8 get_is_low_power_mode();
extern void set_is_low_power_mode(int flag);
extern u8 get_all_day_undisturb_sel();
extern void set_all_day_undisturb_sel(u8 sel);
extern void enter_low_power_mode();
extern void exit_low_power_mode();
extern u8 rtc_calculate_week_val(struct sys_time *data_time);

static u8 is_paycode_enter = 0;      // 是否从快捷菜单的支付码进入支付宝
u8 get_is_paycode_enter()
{
    return is_paycode_enter;
}

void set_is_paycode_enter(u8 flag)
{
    is_paycode_enter = flag;
}


enum {
    BT_MUSIC,				//音频蓝牙 0
    ABOUT,					//关于
    LOW_POWER,				//省电
    QR_CODE,				//二维码
    BRIGHTNESS_SETTING,		//亮度调节
    UNDISTURB,				//勿扰
    ALARM,					//闹钟
    SETTING,				//设置
    RAISE_HAND,				//抬手唤醒
    FLASHLIGHT,				//手电筒

    ALIPAY,					//支付     10
    PAYMENT,				//付款码
    FIND_PHONE,				//查找手机
    VOICE_SETTING,			//音量
    EDIT,					//添加		15
    MAX_INDEX,				//MAX

    BT_MUSIC_ON = MAX_INDEX,	//持续状态显示的图标
    LOW_POWER_ON,
    UNDISTURB_ON,
    RAISE_HAND_ON,
};

struct shortcut_sw_map {
    u8 sw_index_off;
    u8 sw_index_on;
};

const static struct shortcut_sw_map shortcut_map_info[] = {
    {BT_MUSIC,		BT_MUSIC_ON},
    {LOW_POWER,		LOW_POWER_ON},
    {UNDISTURB,		UNDISTURB_ON},
    {RAISE_HAND,	RAISE_HAND_ON},
};

enum {
    SHORTCUT_NOT_SEL,
    SHORTCUT_SEL,
};
enum {
    SHORTCUT_SW_OFF,
    SHORTCUT_SW_ON,
};
struct shortcut_info {
    u32 sel0;
    /* u32 sel1; */
    u32 sw0;
    /* u32 sw1; */
};

static struct  shortcut_info shortcut_menu_info;
#define __this (& shortcut_menu_info)

static int shortcut_item_sel_set(int index, int sel)
{
    if (sel) {
        __this->sel0 |= BIT(index);
    } else {
        __this->sel0 &= ~BIT(index);
    }
    return 0;
}
static int shortcut_item_sel_get(int index)
{
    if (__this->sel0 & BIT(index)) {
        return SHORTCUT_SEL;
    } else {
        return SHORTCUT_NOT_SEL;
    }
}
static int shortcut_item_sel_num_get()
{
    int cnt = 0;
    for (int i = 0; i < MAX_INDEX; i++) {
        if (__this->sel0 & BIT(i)) {
            cnt++;
        }
    }
    return cnt;
}

static int shortcut_map_check(int index)
{
    int check = 0;
    for (int i = 0; i < ARRAY_SIZE(shortcut_map_info); i++) {
        if (shortcut_map_info[i].sw_index_off == index) {
            check = 1;
            break;
        }
    }
    return check;
}
static int shortcut_map_sw_index(int index, int sw)
{
    for (int i = 0; i < ARRAY_SIZE(shortcut_map_info); i++) {
        if (shortcut_map_info[i].sw_index_off == index) {
            if (sw) {
                return shortcut_map_info[i].sw_index_on;
            }
        }
    }
    return index;
}
static int shortcut_item_sw_set(int index, int sw)
{
    if (sw) {
        __this->sw0 |= BIT(index);
    } else {
        __this->sw0 &= ~BIT(index);
    }
    return 0;
}
static int shortcut_item_sw_get(int index)
{
    if (__this->sw0 & BIT(index)) {
        return SHORTCUT_SW_ON;
    } else {
        return SHORTCUT_SW_OFF;
    }
}
static void shortcut_info_dump()
{
    for (int i = 0; i < MAX_INDEX; i++) {
        printf("[shortcut]index:%d sel:%d sw:%d", i, shortcut_item_sel_get(i), shortcut_item_sw_get(i));
    }
}
static int shortcut_item_dynamic_index_to_sel_index(int dynamic_index)
{
    int sel_index = 0;								//选中项排序
    int sw_index = -1;								//选中项映射回原排序
    for (int i = 0; i < MAX_INDEX ; i++) {
        if (__this->sel0 & BIT(i)) {
            if (sel_index == dynamic_index) {
                sw_index = i;
                break;
            }
            sel_index++;
        }
    }
    if (sw_index == -1) {
        sw_index = EDIT;
    }
    return sw_index;
}

static int shortcut_sw_todo(int index, int sw)
{
    int ret = 0;//0 页面调整，1原页面刷新
    switch (index) {
    case BT_MUSIC:
#if TCFG_USER_BT_CLASSIC_ENABLE
        if (sw &&  is_bredr_close()) {
            bredr_conn_last_dev();
        } else if ((!sw) && (!is_bredr_close())) {
            bt_close_bredr();
        }
        ret = 1;
#endif
        break;
    case ABOUT:
        UI_HIDE_CURR_WINDOW();
        UI_SHOW_WINDOW(ID_WINDOW_ABOUT);
        break;
    case LOW_POWER:
        if (sw) {
            enter_low_power_mode();
            set_is_low_power_mode(1);
        } else {
            exit_low_power_mode();
            set_is_low_power_mode(0);
        }
        ret = 1;
        break;
    case QR_CODE:
        UI_HIDE_CURR_WINDOW();
        UI_SHOW_WINDOW(ID_WINDOW_APP_QRCODE);
        break;
    case BRIGHTNESS_SETTING:
        UI_HIDE_CURR_WINDOW();
        UI_SHOW_WINDOW(ID_WINDOW_SETTING_BRIGHTNESS);
        break;
    case UNDISTURB:
        if (sw) {
            UI_HIDE_CURR_WINDOW();
            UI_SHOW_WINDOW(ID_WINDOW_UNDISTURB);
        } else {
            set_all_day_undisturb_sel(0);
            ret = 1;
        }
        break;
    case ALARM:
        UI_HIDE_CURR_WINDOW();
        UI_SHOW_WINDOW(ID_WINDOW_ALARM);
        break;
    case SETTING:
        UI_HIDE_CURR_WINDOW();
        UI_SHOW_WINDOW(ID_WINDOW_SETTING);
        break;
    case RAISE_HAND:
        if (sw) {
            set_is_raise_hand(1);
        } else {
            set_is_raise_hand(0);
        }
        ret = 1;
        break;
    case FLASHLIGHT:
        UI_HIDE_CURR_WINDOW();
        UI_SHOW_WINDOW(ID_WINDOW_FLASHLIGHT);
        break;
    case ALIPAY:
        UI_HIDE_CURR_WINDOW();
        UI_SHOW_WINDOW(ID_WINDOW_ALIPAY);
        break;
    case PAYMENT:
        UI_HIDE_CURR_WINDOW();
        UI_SHOW_WINDOW(ID_WINDOW_ALIPAY);
        break;
    case FIND_PHONE:
        break;
    case VOICE_SETTING:
        UI_HIDE_CURR_WINDOW();
        UI_SHOW_WINDOW(ID_WINDOW_SETTING_VOICE);
        break;
    case EDIT	:
        ui_hide(SHORTCUT_MENU_LAYOUT);
        ui_show(SHORTCUT_EDIT_LAYOUT);
        break;
    }
    return ret;
}
static int shortcut_menu_init()
{
    //get sel from vm
    __this->sel0 = get_ui_sys_param(shortcut_info_sel);
    //update sw status
    //bt
#if TCFG_USER_BT_CLASSIC_ENABLE
    if (!is_bredr_close()) {
        shortcut_item_sw_set(BT_MUSIC, SHORTCUT_SW_ON);
    } else {
        shortcut_item_sw_set(BT_MUSIC, SHORTCUT_SW_OFF);
    }
#endif
    //undisturb
    if (get_all_day_undisturb_sel()) {
        shortcut_item_sw_set(UNDISTURB, SHORTCUT_SW_ON);
    } else {
        shortcut_item_sw_set(UNDISTURB, SHORTCUT_SW_OFF);
    }
    //hand
    if (get_is_raise_hand()) {
        shortcut_item_sw_set(RAISE_HAND, SHORTCUT_SW_ON);
    } else {
        shortcut_item_sw_set(RAISE_HAND, SHORTCUT_SW_OFF);
    }
    //low_power
    if (get_is_low_power_mode()) {
        shortcut_item_sw_set(LOW_POWER, SHORTCUT_SW_ON);
    } else {
        shortcut_item_sw_set(LOW_POWER, SHORTCUT_SW_OFF);
    }
    return true;

}
static int shortcut_menu_uninit()
{
    //set sel from vm
    set_ui_sys_param(shortcut_info_sel, __this->sel0);
    write_UIInfo_to_vm(0);
    return true;
}
static int shortcut_menu_form_child_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    if (event != ON_CHANGE_UPDATE_ITEM) {
        return false;
    }
    /* shortcut_info_dump(); */
    int index = (int)arg;
    int sel_total =  shortcut_item_sel_num_get();	//选择控件数量
    int show_total = sel_total + 1;				//加上”add“

    struct ui_pic *upic  = (struct ui_pic *)_ctrl;

    if (index >= show_total) {						//超过的隐藏
        upic->elm.css.invisible = 1;
        return false;
    } else {
        upic->elm.css.invisible = 0;
    }
    if (index == show_total - 1) {						//show"add"
        ui_pic_set_image_index(upic, EDIT);
        return false;
    }
    int sw_index =  shortcut_item_dynamic_index_to_sel_index(index);
    int sw = shortcut_item_sw_get(sw_index);		//获取开关状态
    sw =  shortcut_map_sw_index(sw_index, sw);		//map
    ui_pic_set_image_index(upic, sw);				//set pic
    return false;
}
static int shortcut_menu_form_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)_ctrl;

    switch (event) {
    case ON_CHANGE_INIT:
        /* shortcut_info_dump(); */
        shortcut_menu_init();
        int col = 2;
        int row = (shortcut_item_sel_num_get() + 1 + 1) / 2; //+1”add icon“  +1 行对齐
        ui_set_default_handler(&grid->elm, NULL, NULL, shortcut_menu_form_child_onchange);
        ui_grid_init_dynamic(grid, &row, &col);
        ui_grid_set_slide_direction(grid, SCROLL_DIRECTION_UD);
        struct scroll_area area = {0, 0, 10000, 10000};
        ui_grid_set_scroll_area(grid, &area);
        ui_grid_flick_ctrl_close(grid, 1);
        break;
    case ON_CHANGE_RELEASE:
        shortcut_menu_uninit();
        break;
    default:
        break;
    }

    return 0;
}
static int shortcut_menu_form_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        if (e->move_flag) {
            break;
        }
        int dynamic_item =  ui_grid_cur_item_dynamic(grid);
        if (grid->touch_index < 0) {
            break;
        }
        //最后一项不是"+"时，不响应触摸
        if (dynamic_item > shortcut_item_sel_num_get()) {
            break;
        }
        int sel_item  = shortcut_item_dynamic_index_to_sel_index(dynamic_item);
        shortcut_item_sw_set(sel_item, !shortcut_item_sw_get(sel_item));
        printf("%s dyn:%d sel:%d sw_now:%d", __func__, dynamic_item, sel_item, shortcut_item_sw_get(sel_item));
        //todo
        int ret = shortcut_sw_todo(sel_item, shortcut_item_sw_get(sel_item));
        if (ret) {
            ui_grid_update_by_id_dynamic(grid->elm.id, grid->touch_index, 1);
        }
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SHORTCUT_FORM)
.ontouch = shortcut_menu_form_ontouch,
 .onkey = NULL,
  .onchange  = shortcut_menu_form_onchange,
};
static int shortcut_menu_edit_child_update(struct ui_grid *grid)
{
    for (int i = 0; i < grid->avail_item_num; i++) {
        struct element *item = (struct element *)&grid->item[i];
        struct element *p;
        list_for_each_child_element(p, item) {
            if (ui_id2type(p->id) == CTRL_TYPE_PIC) {
                struct ui_pic *upic = (struct ui_pic *)p;
                if (!strcmp(upic->source, "sw")) {
                    if (__this->sel0 & BIT(i)) {
                        ui_pic_set_image_index(upic, SHORTCUT_SEL);
                    } else {
                        ui_pic_set_image_index(upic, SHORTCUT_NOT_SEL);
                    }
                }
            }
        }
    }
    return 0;
}
static int shortcut_menu_edit_vlist_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)_ctrl;
    switch (event) {
    case ON_CHANGE_INIT:

        /* shortcut_info_dump(); */
        ui_grid_set_slide_direction(grid, SCROLL_DIRECTION_UD);
        struct scroll_area area = {0, 0, 10000, 10000};
        ui_grid_set_scroll_area(grid, &area);
        ui_grid_flick_ctrl_close(grid, 1);
        shortcut_menu_edit_child_update(grid);
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        break;
    }
    return 0;
}
static int shortcut_menu_edit_vlist_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        if (e->move_flag) {
            return false;
        }
        int sel_item = ui_grid_touch_item(grid);
        if (sel_item < 0) {
            return false;
        }
        shortcut_item_sel_set(sel_item, ! shortcut_item_sel_get(sel_item));
        set_ui_sys_param(shortcut_info_sel, __this->sel0);
        shortcut_menu_edit_child_update(grid);
        ui_core_redraw(&grid->elm);
        break;
    case ELM_EVENT_TOUCH_R_MOVE:
        ui_hide(SHORTCUT_EDIT_LAYOUT);
        ui_show(SHORTCUT_MENU_LAYOUT);
        return true;
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SHORTCUT_EDIT_VLIST)
.onchange = shortcut_menu_edit_vlist_onchange,
 .onkey = NULL,
  .ontouch =  shortcut_menu_edit_vlist_ontouch,
};
void app_conn_status_update(void *p)
{
    struct element *elm = (struct element *)p;
    ui_core_redraw(elm);
}
static int shortcut_app_conn_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_pic *pic = (struct ui_pic *)_ctrl;

    switch (event) {
    case ON_CHANGE_INIT:
        if (!pic->timer) {
            pic->timer = sys_timer_add(&pic->elm, app_conn_status_update, 500);
        }
        break;
    case ON_CHANGE_SHOW:
#if TCFG_USER_BT_CLASSIC_ENABLE
        int bt_status = bt_get_connect_status();
        if ((bt_status == BT_STATUS_WAITINT_CONN) || (bt_status == BT_STATUS_INITING) || (bt_status == BT_STATUS_AUTO_CONNECTINT)) {
            ui_pic_set_image_index(pic, 0);
        } else {
            ui_pic_set_image_index(pic, 1);
        }
#endif
        break;
    case ON_CHANGE_RELEASE:
        if (pic->timer) {
            sys_timer_del(pic->timer);
            pic->timer = 0;
        }
        break;
    default:
        break;
    }
    return 0;
}
REGISTER_UI_EVENT_HANDLER(SHORTCUT_FORM_APP_PIC)
.onchange =  shortcut_app_conn_onchange,
 .onkey = NULL,
  .ontouch =  NULL,
};
static int shortcut_week_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_text *text = (struct ui_text *)_ctrl;

    switch (event) {
    case ON_CHANGE_SHOW_PROBE:
        struct sys_time time_t;
        rtc_read_time(&time_t);
        int week_index  = rtc_calculate_week_val(&time_t);
        ui_text_set_index(text, week_index); //0周日
        break;

    default:
        break;
    }
    return 0;
}

REGISTER_UI_EVENT_HANDLER(SHORTCUT_FORM_WEEK_TEXT)
.ontouch = NULL,
 .onkey = NULL,
  .onchange  =  shortcut_week_onchange,
};



#endif
#endif

