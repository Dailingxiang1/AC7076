#include "app_config.h"
#include "jlui_app/ui_style.h"
#include "jlui/ui.h"
#include "ui/ui_api.h"
#include "app_task.h"
#include "system/timer.h"
#include "device/device.h"
#include "key_event_deal.h"
#include "res/resfile.h"
#include "jlui_app/res_config.h"
#include "jlui_app/ui_resource.h"
#include "system/includes.h"
#include "audio_config.h"
#include "asm/mcpwm.h"
#include "jlui_app/ui_sys_param.h"
#include "jlui_app/watch_syscfg_manage.h"
#include "font/language_list.h"
#include "bt_common.h"
#include "btstack/btstack_task.h"
#include "btstack/avctp_user.h"
#include "custom_cfg.h"

#define LOG_TAG_CONST       UI
#define LOG_TAG     		"[UI_PASSWORD]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ui_action_password.data.bss")
#pragma data_seg(".ui_action_password.data")
#pragma const_seg(".ui_action_password.text.const")
#pragma code_seg(".ui_action_password.text")
#endif

#if (defined (CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE))
#if TCFG_UI_ENABLE_POWERON_PASSWORD

#define STYLE_NAME JL
/* struct poweron_password_t { */
/*     u8 index; */
/*     char final_password[5]; // 保存的密码 */
/*     char password[5];     // 用于键盘输入 */
/* }; */
static struct password_t *password = NULL;

static int is_poweron_password;

u8 get_poweron_password()
{
    return is_poweron_password;
}

void set_poweron_password(u8 flag)
{
    is_poweron_password = flag;
}

static void ui_show_password(int index)
{
    printf("index %d\n", index);
    struct ui_pic *pic = NULL;
    struct ui_text *text = NULL;
    if (index == 0) {
        pic = ui_pic_for_id(POWERON_PASSWORD_1_PIC);
        if (pic) {
            ui_pic_set_image_index(pic, 0);
        }
        pic = ui_pic_for_id(POWERON_PASSWORD_2_PIC);
        if (pic) {
            ui_pic_set_image_index(pic, 0);
        }
        pic = ui_pic_for_id(POWERON_PASSWORD_3_PIC);
        if (pic) {
            ui_pic_set_image_index(pic, 0);
        }
        pic = ui_pic_for_id(POWERON_PASSWORD_4_PIC);
        if (pic) {
            ui_pic_set_image_index(pic, 0);
        }
    }
    if (index == 1) {
        text = ui_text_for_id(POWERON_PASSWORD_RES_TEXT);
        if (text) {
            text->elm.css.invisible = 1;
        }
        //ui_hide(POWERON_PASSWORD_RES_TEXT);
        pic = ui_pic_for_id(POWERON_PASSWORD_1_PIC);
        if (pic) {
            pic->elm.css.invisible = 0;
            ui_pic_set_image_index(pic, 1);
        }
        pic = ui_pic_for_id(POWERON_PASSWORD_2_PIC);
        if (pic) {
            pic->elm.css.invisible = 0;
            ui_pic_set_image_index(pic, 0);
        }
        pic = ui_pic_for_id(POWERON_PASSWORD_3_PIC);
        if (pic) {
            pic->elm.css.invisible = 0;
            ui_pic_set_image_index(pic, 0);
        }
        pic = ui_pic_for_id(POWERON_PASSWORD_4_PIC);
        if (pic) {
            pic->elm.css.invisible = 0;
            ui_pic_set_image_index(pic, 0);
        }
    } else if (index == 2) {
        pic = ui_pic_for_id(POWERON_PASSWORD_1_PIC);
        if (pic) {
            ui_pic_set_image_index(pic, 1);
        }
        pic = ui_pic_for_id(POWERON_PASSWORD_2_PIC);
        if (pic) {
            ui_pic_set_image_index(pic, 1);
        }
        pic = ui_pic_for_id(POWERON_PASSWORD_3_PIC);
        if (pic) {
            ui_pic_set_image_index(pic, 0);
        }
        pic = ui_pic_for_id(POWERON_PASSWORD_4_PIC);
        if (pic) {
            ui_pic_set_image_index(pic, 0);
        }
    } else if (index == 3) {
        pic = ui_pic_for_id(POWERON_PASSWORD_1_PIC);
        if (pic) {
            ui_pic_set_image_index(pic, 1);
        }
        pic = ui_pic_for_id(POWERON_PASSWORD_2_PIC);
        if (pic) {
            ui_pic_set_image_index(pic, 1);
        }
        pic = ui_pic_for_id(POWERON_PASSWORD_3_PIC);
        if (pic) {
            ui_pic_set_image_index(pic, 1);
        }
        pic = ui_pic_for_id(POWERON_PASSWORD_4_PIC);
        if (pic) {
            ui_pic_set_image_index(pic, 0);
        }
    } else if (index == 4) {
        pic = ui_pic_for_id(POWERON_PASSWORD_1_PIC);
        if (pic) {
            ui_pic_set_image_index(pic, 1);
        }
        pic = ui_pic_for_id(POWERON_PASSWORD_2_PIC);
        if (pic) {
            ui_pic_set_image_index(pic, 1);
        }
        pic = ui_pic_for_id(POWERON_PASSWORD_3_PIC);
        if (pic) {
            ui_pic_set_image_index(pic, 1);
        }
        pic = ui_pic_for_id(POWERON_PASSWORD_4_PIC);
        if (pic) {
            ui_pic_set_image_index(pic, 1);
        }
    }
}

static void password_add_number(char ch)
{
    if (password->index >= 4) {
        return;
    }
    password->password[password->index++] = ch;
    password->password[password->index] = '\0';
    ui_show_password(password->index);
    printf("add pass:%s\n", password->password);
}

static void password_remove_number()
{
    if (password->index <= 0) {
        return;
    }
    password->password[--password->index] = '\0';
    ui_show_password(password->index);
    printf("del pass:%s\n", password->password);
}

static void ui_hide_password(int index)
{
    struct ui_pic *pic = NULL;
    struct ui_text *text = NULL;
    pic = ui_pic_for_id(POWERON_PASSWORD_1_PIC);
    if (pic) {
        pic->elm.css.invisible = 1;
    }
    pic = ui_pic_for_id(POWERON_PASSWORD_2_PIC);
    if (pic) {
        pic->elm.css.invisible = 1;
    }
    pic = ui_pic_for_id(POWERON_PASSWORD_3_PIC);
    if (pic) {
        pic->elm.css.invisible = 1;
    }
    pic = ui_pic_for_id(POWERON_PASSWORD_4_PIC);
    if (pic) {
        pic->elm.css.invisible = 1;
    }
    text = ui_text_for_id(POWERON_PASSWORD_RES_TEXT);
    text->elm.css.invisible = 0;
    ui_text_set_index(text, index);
}

static void password_sure_process()
{
    if (strcmp(password->password, password->final_password) != 0) {   // 密码错误
        ui_hide_password(1);
        memset(password->password, 0, sizeof(password->password));
        password->index = 0;
    } else {
        set_need_password(0);
        UI_HIDE_CURR_WINDOW();
        UI_SHOW_WINDOW(ID_WINDOW_DIAL);
        UI_WINDOW_BACK_DEL(ID_WINDOW_POWERON_PASSWORD);
    }
}


static int password_page_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct window *window = (struct window *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        puts("\n***password_onchange***\n");
        set_poweron_password(1);
        if (!password) {
            password = zalloc(sizeof(struct password_t));
        }
        break;
    case ON_CHANGE_RELEASE:
        set_poweron_password(0);
        if (password) {
            free(password);
            password = NULL;
        }
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ID_WINDOW_POWERON_PASSWORD)
.onchange = password_page_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int password_layout_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct layout *layout = (struct layout *)_ctrl;

    switch (event) {
    case ON_CHANGE_INIT:
        syscfg_read(USER_PASSWORD, &(password->final_password), POWERON_PASSWORD_LEN);
        printf("password: %s\n", password->final_password);
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        break;
    }
    return 0;
}

REGISTER_UI_EVENT_HANDLER(POWERON_PASSWORD_LAYOUT)//通用-垂直列表
.onchange = password_layout_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int password_enter_button_ontouch(void *ctrl, struct element_touch_event *e)
{
    struct element *elm = (struct element *)ctrl;

    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_UP:
        switch (elm->id) {
        case POWERON_PASSWORD_DEL_BUTTON:
            printf("[5]DIAL_DEL UP\n");
            password_remove_number();
            break;
        case POWERON_PASSWORD_0:
            printf("[5]DIAL_0 UP\n");
            password_add_number('0');
            break;
        case POWERON_PASSWORD_1:
            printf("[5]DIAL_1 UP\n");
            password_add_number('1');
            break;
        case POWERON_PASSWORD_2:
            printf("[5]DIAL_2 UP\n");
            password_add_number('2');
            break;
        case POWERON_PASSWORD_3:
            printf("[5]DIAL_3 UP\n");
            password_add_number('3');
            break;
        case POWERON_PASSWORD_4:
            printf("[5]DIAL_4 UP\n");
            password_add_number('4');
            break;
        case POWERON_PASSWORD_5:
            printf("[5]DIAL_5 UP\n");
            password_add_number('5');
            break;
        case POWERON_PASSWORD_6:
            printf("[5]DIAL_6 UP\n");
            password_add_number('6');
            break;
        case POWERON_PASSWORD_7:
            printf("[5]DIAL_7 UP\n");
            password_add_number('7');
            break;
        case POWERON_PASSWORD_8:
            printf("[5]DIAL_8 UP\n");
            password_add_number('8');
            break;
        case POWERON_PASSWORD_9:
            printf("[5]DIAL_9 UP\n");
            password_add_number('9');
            break;
        case POWERON_PASSWORD_SURE:
            printf("[5]DIAL_SURE UP\n");
            password_sure_process();
            break;
        default:
            break;
        }
        ui_core_redraw(elm->parent);

        break;
    default:
        break;
    }
    return true;
}

REGISTER_UI_EVENT_HANDLER(POWERON_PASSWORD_DEL_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = password_enter_button_ontouch,
};
REGISTER_UI_EVENT_HANDLER(POWERON_PASSWORD_0)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = password_enter_button_ontouch,
};
REGISTER_UI_EVENT_HANDLER(POWERON_PASSWORD_1)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = password_enter_button_ontouch,
};
REGISTER_UI_EVENT_HANDLER(POWERON_PASSWORD_2)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = password_enter_button_ontouch,
};
REGISTER_UI_EVENT_HANDLER(POWERON_PASSWORD_3)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = password_enter_button_ontouch,
};
REGISTER_UI_EVENT_HANDLER(POWERON_PASSWORD_4)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = password_enter_button_ontouch,
};
REGISTER_UI_EVENT_HANDLER(POWERON_PASSWORD_5)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = password_enter_button_ontouch,
};
REGISTER_UI_EVENT_HANDLER(POWERON_PASSWORD_6)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = password_enter_button_ontouch,
};
REGISTER_UI_EVENT_HANDLER(POWERON_PASSWORD_7)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = password_enter_button_ontouch,
};
REGISTER_UI_EVENT_HANDLER(POWERON_PASSWORD_8)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = password_enter_button_ontouch,
};
REGISTER_UI_EVENT_HANDLER(POWERON_PASSWORD_9)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = password_enter_button_ontouch,
};
REGISTER_UI_EVENT_HANDLER(POWERON_PASSWORD_SURE)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = password_enter_button_ontouch,
};

#endif
#endif
