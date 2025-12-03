
#include "app_config.h"
#include "jlui_app/ui_style.h"
#include "jlui/ui.h"
#include "ui/ui_api.h"
#include "jlui_app/ui_sys_param.h"
#include "jlui_app/watch_syscfg_manage.h"
#include "syscfg_id.h"
#include "system/timer.h"
#include "app_main.h"
#include "ui_core.h"
#include "ble_fmy_fmna.h"

#define LOG_TAG_CONST       UI
#define LOG_TAG     		"[UI_FINDMY]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"


#if (BT_AI_SEL_PROTOCOL & FMNA_EN)

#define STYLE_NAME  JL
REGISTER_UI_STYLE(STYLE_NAME)

typedef struct {
    u8 find_my_opend_flag: 4;//findmy广播标志位
    u8 find_my_bind_flag: 4;//findmy是否完成配对标志位
    u8 find_my_binding_flag: 4;//findmy是否处于绑定过程
    u8 find_my_connecting_flag: 2;//findmy是否处于连接过程
    u8 find_my_first_enter_flag: 2;//findmy第一次进入页面
    u16 findmy_return_set_id;//记录定时ID
    u16 check_cnt;//定时器计数值
    u16 findmy_id;//记录定时器ID
} findmy_state;

typedef struct {
    u8  head_tag;
    u8  reset_config;//flag
    u8  is_open;
} fmy_vm_t;

static findmy_state *findmy_flag = NULL;

u8 is_findmy_open();
u8 is_findmy_bind();
void findmy_return_setting();
int findmy_pair_success(const char *type, u32 arg);
static int findmy_unpair_success(const char *type, u32 arg);
static int set_findmy_connecting_flag(const char *type, u32 flag);

extern bool fmy_vm_deal(fmy_vm_t *info, u8 rw_flag);

static const struct uimsg_handl ui_msg_handler[] = {
    {"PAIR_SUCCESS", findmy_pair_success },
    {"UNPAIR_SUCCESS", findmy_unpair_success },
    {"SET_CONNECTING_FLAG", set_findmy_connecting_flag},
    {NULL, NULL},
};

int findmy_pair_success(const char *type, u32 arg)
{
    struct element *elm = NULL;
    u8 binding_baseform_invisible = 1;

    elm = ui_core_get_element_by_id(FINDMY_BINDING_BASEFORM);
    if (elm != NULL) {
        binding_baseform_invisible = elm->css.invisible;
    }
    if (binding_baseform_invisible == 0) {
        ui_hide(FINDMY_BINDING_BASEFORM);
        ui_show(FINDMY_BIND_SUCCEED);
    }
    findmy_flag->check_cnt = 0;
    findmy_flag->findmy_return_set_id = sys_timeout_add(NULL, findmy_return_setting, 1000);
    return 0;
}

static int findmy_unpair_success(const char *type, u32 arg)
{
    if (is_findmy_open()) {
        ui_text_show_index_by_id(FINDMY_PAIRING_BUTTON, 0);//把UI字体更新为开始配对
    }
    return 0;
}

static void text_name_change(void *p)
{
    ui_text_show_index_by_id(FINDMY_PAIRING_BUTTON, is_findmy_bind());
}

static int set_findmy_connecting_flag(const char *type, u32 flag)
{
    findmy_flag->find_my_connecting_flag = flag;
    return 0;
}

void findmy_return_setting()
{
    findmy_flag->findmy_return_set_id = 0;
    if (is_findmy_bind()) {
        ui_hide(FINDMY_BIND_SUCCEED);
        ui_show(FINDMY_SETTING);
    } else {
        ui_hide(FINDMY_BIND_FAIL);
        ui_show(FINDMY_SETTING);
    }
    ui_text_show_index_by_id(FINDMY_PAIRING_BUTTON, is_findmy_bind());//刷新一下开始配对的字体，防止刷新不成功。
}

static int findmy_switch_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_pic *pic = (struct ui_pic *)_ctrl;

    switch (event) {
    case ON_CHANGE_INIT:
        if (!is_findmy_open()) {
            ui_pic_show_image(pic, 0);
        } else {
            sys_timeout_add(NULL, text_name_change, 1);
            ui_pic_show_image(pic, 1);
        }
        ui_register_msg_handler(ID_WINDOW_FINDMY, ui_msg_handler);
        break;
    default:
        break;
    }

    return 0;
}

static int findmy_base_form_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_pic *pic = (struct ui_pic *)_ctrl;

    switch (event) {
    case ON_CHANGE_INIT:
        findmy_flag = zalloc(sizeof(findmy_state));
        break;
    case ON_CHANGE_RELEASE:
        if (findmy_flag != NULL && findmy_flag->findmy_return_set_id != 0) {
            sys_timeout_del(findmy_flag->findmy_return_set_id);
        }
        if (findmy_flag != NULL && findmy_flag->findmy_id != 0) {
            sys_timer_del(findmy_flag->findmy_id);
        }
        free(findmy_flag);
        findmy_flag = NULL;
        break;
    default:
        break;
    }

    return 0;
}

static int findmy_bind_fail_baseform_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    switch (event) {
    case ON_CHANGE_RELEASE:
        if (findmy_flag != NULL && findmy_flag->findmy_return_set_id != 0) {
            sys_timeout_del(findmy_flag->findmy_return_set_id);
            findmy_flag->findmy_return_set_id = 0;
        }
        break;
    default:
        break;


    }

    return 0;
}

static int findmy_cancle_no_ontouch(void *ctr, struct element_touch_event *e)
{
    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        ui_show(FINDMY_SETTING);
        ui_hide(FINDMY_CANCEL);
        break;
    }

    return false;
}

static int findmy_cancle_yes_ontouch(void *ctr, struct element_touch_event *e)
{
    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        fmy_open_close_pairing_mode(0);
        fmy_factory_reset();
        ui_show(FINDMY_SETTING);
        ui_hide(FINDMY_CANCEL);
        break;
    }

    return false;
}

static int findmy_cancel_ontouch(void *ctr, struct element_touch_event *e)
{
    switch (e->event) {
    case ELM_EVENT_TOUCH_R_MOVE:
        ui_hide(FINDMY_CANCEL);
        ui_show(FINDMY_SETTING);
        break;
    }

    return true;
}


static int findmy_bind_succ_baseform_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    switch (event) {
    case ON_CHANGE_RELEASE:
        if (findmy_flag != NULL && findmy_flag->findmy_return_set_id != 0) {
            sys_timeout_del(findmy_flag->findmy_return_set_id);
            findmy_flag->findmy_return_set_id = 0;
        }
        break;
    default:
        break;
    }

    return 0;
}

u8 is_findmy_open()
{
    /* static u8 first_flag = 0; */

    if (findmy_flag->find_my_first_enter_flag == 0) {
        u8 vm_flag[3];
        int ret = syscfg_read(CFG_FMY_INFO, vm_flag, 3);
        log_info("[msg]%s-%d>>>>>>>>>>>ret=%d", __FUNCTION__, __LINE__, ret);
        if (ret != 3) {
            findmy_flag->find_my_opend_flag = 0;
        } else {

            findmy_flag->find_my_opend_flag = vm_flag[2];
        }
        findmy_flag->find_my_first_enter_flag = 1;
    }
    return findmy_flag->find_my_opend_flag;
}

static void toogle_findmy_open_flag()
{
    fmy_vm_t vm_flag;
    int ret;
    ret =	fmy_vm_deal(&vm_flag, 0);
    log_info("[msg]%s-%d>>>>>>>>>>>ret=%d,CFG_FMY_INFO=%d", __FUNCTION__, __LINE__, ret, CFG_FMY_INFO);
    put_buf((void *)&vm_flag, 3);
    findmy_flag->find_my_opend_flag = vm_flag.is_open;
    findmy_flag->find_my_opend_flag = findmy_flag->find_my_opend_flag ^ BIT(0);
}

u8 is_findmy_bind()
{
    return fmy_get_pair_state();
}

static int findmy_binding_ontouch(void *ctr, struct element_touch_event *e)
{
    return true;//在绑定过程界面不允许右划 左划退出
}

void findmy_binding_time(void)
{
    struct element *elm = NULL;
    u8 binding_baseform_invisible = 1;
    struct ui_pic *pic = NULL;

    findmy_flag->check_cnt++;//时间计数值，30s超时判断配对失败
    pic = ui_pic_for_id(FINDMY_BINDING);
    if (pic) {
        ui_core_set_element_rotate(pic, 81, 81, 156, 138, 36 * findmy_flag->check_cnt, true);
        ui_core_redraw(pic);
    }
    /* ui_pic_show_image_by_id(FINDMY_BINDING, findmy_flag->check_cnt % 10); */
    if (((findmy_flag->check_cnt >= 150) && (findmy_flag->find_my_connecting_flag == 0)) ||
        ((findmy_flag->check_cnt >= 200) && (findmy_flag->find_my_connecting_flag == 1))) { //30s超时配对失败 或者如果检测到正在连接增加多5s来保证配对过程UI不会错乱
        findmy_flag->check_cnt = 0;
        fmy_open_close_pairing_mode(0);
        elm = ui_core_get_element_by_id(FINDMY_BINDING_BASEFORM);
        if (elm != NULL) {
            binding_baseform_invisible = elm->css.invisible;
        }
        if (binding_baseform_invisible == 0) {
            ui_hide(FINDMY_BINDING_BASEFORM);
            ui_show(FINDMY_BIND_FAIL);
        }
        findmy_flag->findmy_return_set_id = sys_timeout_add(NULL, findmy_return_setting, 1000); //记录ID 退出删除。
        return;
    }
}

static int findmy_binding_baseform_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    switch (event) {
    case ON_CHANGE_INIT:
        findmy_flag->check_cnt = 0;
        findmy_flag->findmy_id = sys_timer_add(NULL, (void(*)(void *))findmy_binding_time, 200);
        set_findmy_connecting_flag(NULL, 0);
        ui_auto_shut_down_disable();
        break;
    case ON_CHANGE_RELEASE:
        fmy_open_close_pairing_mode(0);
        if (findmy_flag != NULL && findmy_flag->findmy_id != 0) {
            sys_timer_del(findmy_flag->findmy_id);
            findmy_flag->findmy_id = 0;
        }
        ui_auto_shut_down_enable();
        break;
    default:
        break;

    }

    return 0;
}

static int pairing_button_ontouch(void *ctr, struct element_touch_event *e)
{
    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        if (is_findmy_bind()) {
            ui_show(FINDMY_CANCEL);
            ui_hide(FINDMY_SETTING);
        } else {
            fmy_open_close_pairing_mode(1);
            ui_show(FINDMY_BINDING_BASEFORM);
            ui_hide(FINDMY_SETTING);
        }
        break;
    }

    return false;
}

static int set_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)_ctrl;

    switch (event) {
    case ON_CHANGE_INIT:
        ui_grid_set_slide_direction(grid, SCROLL_DIRECTION_UD);
        /* set_ui_sys_param(ConnNewPhone, 0); */
        struct scroll_area area = {0, 0, 10000, 10000};
        ui_grid_set_scroll_area(grid, &area);
        ui_grid_flick_ctrl_close(grid, 1);
        /* setpage_item_set(grid, setpage_item_memory); */
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        break;
    }
    return 0;
}

static int findmy_set_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    static u8 touch_action = 0;
    int sel_item;
    u8 buf_flag[3];
    static u8 cnt = 0;

    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        if (touch_action != 1) {
            break;
        }
        sel_item = ui_grid_cur_item(grid);
        if (sel_item <= 0) {
            break;
        }
        switch (sel_item) {
        case 1:
            toogle_findmy_open_flag();
            if (is_findmy_open()) {
                fmy_enable(1);
                ui_text_show_index_by_id(FINDMY_PAIRING_BUTTON, is_findmy_bind());//这里需要预留接口去判断
                ui_pic_show_image_by_id(SETTING_FINDMY_OPEN_BUTTON, 1);
            } else {
                fmy_enable(0);
                ui_hide(FINDMY_PAIRING_BUTTON);

                ui_pic_show_image_by_id(SETTING_FINDMY_OPEN_BUTTON, 0);
            }
            break;
        }
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        touch_action = 2;
        break;
    case ELM_EVENT_TOUCH_DOWN:
        touch_action = 1;
        break;
    case ELM_EVENT_TOUCH_U_MOVE:
        break;
    case ELM_EVENT_TOUCH_D_MOVE:
        break;
    }

    return false;
}

REGISTER_UI_EVENT_HANDLER(FINDMY_VLIST)//设置-垂直列表
.onchange = set_onchange,
 .onkey = NULL,
  .ontouch = findmy_set_ontouch,
};
REGISTER_UI_EVENT_HANDLER(FINDMY_PAIRING_BUTTON)//配对开关控制
.onchange = NULL,
 .onkey = NULL,
  .ontouch = pairing_button_ontouch,
};
REGISTER_UI_EVENT_HANDLER(FINDMY_BINDING_BASEFORM)//findmy绑定界面
.onchange = findmy_binding_baseform_onchange,
 .onkey = NULL,
  .ontouch = findmy_binding_ontouch,
};
REGISTER_UI_EVENT_HANDLER(FINDMY_BIND_FAIL)//取消配对界面
.onchange = findmy_bind_fail_baseform_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(FINDMY_BIND_SUCCEED)//取消配对界面
.onchange = findmy_bind_succ_baseform_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(FINDMY_CANCLE_NO)//取消配对-否
.onchange = NULL,
 .onkey = NULL,
  .ontouch = findmy_cancle_no_ontouch,
};
REGISTER_UI_EVENT_HANDLER(FINDMY_CANCLE_YES)//取消配对-是
.onchange = NULL,
 .onkey = NULL,
  .ontouch = findmy_cancle_yes_ontouch,
};
REGISTER_UI_EVENT_HANDLER(FINDMY_CANCEL)//取消配对界面
.onchange = NULL,
 .onkey = NULL,
  .ontouch = findmy_cancel_ontouch,
};
REGISTER_UI_EVENT_HANDLER(SETTING_FINDMY_OPEN_BUTTON)//设置-findmy开关按钮
.onchange = findmy_switch_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(ID_WINDOW_FINDMY)//页面PAGE
.onchange = findmy_base_form_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

#endif
