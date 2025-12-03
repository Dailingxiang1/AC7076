#include "app_config.h"
#include "media/includes.h"
#include "audio_config.h"
#include "JL_rcsp_protocol.h"
#include "ui/ui_api.h"
#include "jlui/ui.h"
#include "jlui_app/ui_style.h"
#include "app_task.h"
#include "system/timer.h"
#include "app_main.h"
#include "init.h"
#include "key_event_deal.h"
#include "ai_interaction/ai_audio.h"
#include "btstack/avctp_user.h"
#include "jlui/ui_number.h"
/* #include "smartbox_rcsp_manage.h" */
#include "jlui/ui_text.h"
#include "JL_rcsp_protocol.h"
#if TCFG_AI_INTERACTION_ENABLE

#define STYLE_NAME JL
void memset_txt(void);
extern int ai_interaction_rec_start(void);
extern int ai_interaction_rec_stop(void);
extern u8 ui_auto_shut_down_disable(void);
extern void ui_auto_shut_down_enable(void);
extern u8 get_rcsp_connect_status(void);
extern void font_get_text_width(u8 encode, struct font_info *info, u8 *str, u16 strlen);
extern struct font_info *text_font_init(u8 init);
extern volatile u8 *tts_txt;
extern volatile u8 *ai_txt1;
extern volatile u8 *ai_txt2;
extern volatile u8 *ai_txttempa;
extern volatile u8 *ai_txttempb;

static bool ai_timeout = false; //倒计时超时标志位0---不超时   1---超时
volatile static u8 flag_button = 0; //开始录音的按钮控件取反标志位 0----开始录音  1----停止录音
static u8 flag = 0;
static u8 i_flag = 20; //20秒的标志位
static u8 cro_ui = 0; //0--未连接；1---录音;2--思考；3--文本显示
static u16 ai_recoreai_gettxt1_timer = 0;
static u16 ai_countdown_timer = 0;
static u16 ai_thinking_timer = 0;
static u16 ai_gettxt1_timer = 0;
static u16 ai_gettxt2_timer = 0;
static u16 ai_show_empty_timer = 0;
static int last_pos_y = 0;
static int y_move_total = 0;
static int string_height = 0;
static char *empty_str = NULL;

struct AI_EXIST {
    u8 mode_choice;
    u8 ltv_len;
    u8 ltv_type;
    u8 version;
    u8 status;
} __attribute__((packed));
static void send_0X09(void)//协议09指令
{
    printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>090909>>>>>>>>>>>>\n");
    struct AI_EXIST ai_exist;
    ai_exist.mode_choice = 0XFF;
    ai_exist.ltv_len = 0X03;
    ai_exist.ltv_type = 0X1A;
    ai_exist.version = 0X00;
    ai_exist.status = 0X02;
    JL_CMD_send(JL_OPCODE_SYS_INFO_AUTO_UPDATE, (u8 *)&ai_exist, sizeof(struct AI_EXIST), JL_NOT_NEED_RESPOND, 0, NULL);
}
static void send_0X09_ENTER(void)//协议09指令进入云服务
{
    printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>090909>>>>>>>>>>>>\n");
    struct AI_EXIST ai_exist;
    ai_exist.mode_choice = 0XFF;
    ai_exist.ltv_len = 0X03;
    ai_exist.ltv_type = 0X1A;
    ai_exist.version = 0X00;
    ai_exist.status = 0X01;
    JL_CMD_send(JL_OPCODE_SYS_INFO_AUTO_UPDATE, (u8 *)&ai_exist, sizeof(struct AI_EXIST), JL_NOT_NEED_RESPOND, 0, NULL);
}
static void page_switch(u8 ui_temp)
{
    struct element *ctr = ui_core_get_element_by_id(AI_IFLYTEK);
    //	ASSERT(ctr);
    if (ctr == NULL) {
        return;
    }
    if (get_rcsp_connect_status()) {
        cro_ui = ui_temp;
    } else {
        cro_ui = 0;
    }
    struct element *elm;
    list_for_each_child_element(elm, (struct element *)ctr) {
        if (elm->id == AI_STATUS) {
            if (!cro_ui) {
                elm->css.invisible = 0;
            } else {
                elm->css.invisible = 1;
            }
        }
        if (elm->id == AI_TALK1) {
            if (cro_ui == 1) {
                elm->css.invisible = 0;
            } else {
                elm->css.invisible = 1;
            }

        }
        if (elm->id == AI_THINK) {
            if (cro_ui == 2) {
                elm->css.invisible = 0;
            } else {
                elm->css.invisible = 1;
            }

        }
        if (elm->id == AI_SHOW) {
            if (cro_ui == 3) {
                elm->css.invisible = 0;
            } else {
                elm->css.invisible = 1;
            }
        }
        ui_core_redraw(ctr);
    }
}
static int page_link()
{
    page_switch(0);
    return 0;
}
static int page_think()
{
    if (ai_countdown_timer) {
        sys_timer_del(ai_countdown_timer);
        ai_countdown_timer = 0;
    }
    if (ai_recoreai_gettxt1_timer) {
        sys_timer_del(ai_recoreai_gettxt1_timer);
        ai_recoreai_gettxt1_timer = 0;
    }
    page_switch(2);

    return 0;
}
static int page_showtxt()
{
    if (ai_thinking_timer) {
        sys_timer_del(ai_thinking_timer);
        ai_thinking_timer = 0;
    }
    page_switch(3);
    return 0;
}
static const struct uimsg_handl ui_msg_cmd_handler[] = {
    {"link", page_link},
    {"think", page_think},
    {"SEND_TXT", page_showtxt},
    {NULL, NULL},
};


static int show_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:
        send_0X09_ENTER();
        break;
    case ON_CHANGE_FIRST_SHOW:
        flag_button = 0;
        if (get_rcsp_connect_status()) {
            cro_ui = 1;
        } else {
            cro_ui = 0;
        }
        struct element *p;
        list_for_each_child_element(p, (struct element *)ctr) {
            if (p->id == AI_STATUS) {
                if (!cro_ui) {
                    p->css.invisible = 0;
                } else {
                    p->css.invisible = 1;
                }
            }
            if (p->id == AI_TALK1) {
                if (cro_ui == 1) {
                    p->css.invisible = 0;
                } else {
                    p->css.invisible = 1;
                }

            }
            if (p->id == AI_THINK) {
                if (ai_recoreai_gettxt1_timer) {
                    sys_timer_del(ai_recoreai_gettxt1_timer);
                    ai_recoreai_gettxt1_timer = 0;
                }
                if (ai_countdown_timer) {
                    sys_timer_del(ai_countdown_timer);
                    ai_countdown_timer = 0;
                }
                if (cro_ui == 2) {
                    p->css.invisible = 0;
                } else {
                    p->css.invisible = 1;
                }

            }
            if (p->id == AI_SHOW) {
                if (ai_thinking_timer) {
                    sys_timer_del(ai_thinking_timer);
                    ai_thinking_timer = 0;
                }
                if (cro_ui == 3) {
                    p->css.invisible = 0;
                } else {
                    p->css.invisible = 1;
                }
            }
        }
        ui_register_msg_handler(ID_WINDOW_APP_IFLYTEK, ui_msg_cmd_handler);//注册消息交互的回调
        break;
    case ON_CHANGE_RELEASE:
        ai_interaction_rec_stop();
        if (ai_thinking_timer) {
            sys_timer_del(ai_thinking_timer);
            ai_thinking_timer = 0;
        }


        ui_register_msg_handler(ID_WINDOW_APP_IFLYTEK, NULL);//注册消息交互的回调
        send_0X09();
        break;
    default:
        break;
    }
    return 0;
}
REGISTER_UI_EVENT_HANDLER(AI_IFLYTEK)
.onchange = show_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};





static void AI_time1_handler(void *priv)//20秒倒计时，不息屏
{
    ui_auto_shut_down_disable();
    i_flag--;
    if (i_flag == 0) { //20倒计时结束后，超时标志位置1
        i_flag = 20;
        ai_timeout = true;
    } else if (flag_button == 0) { //按钮控件按下，开始录音倒计时
        i_flag = 20;
    }
    if (ai_timeout) { //超时结束，退出到列表，结束录音
        ai_interaction_rec_stop();
        UI_MSG_POST("think");
        ai_timeout = false;
    }
    struct unumber num;
    num.type = TYPE_NUM;
    num.numbs = 1;
    num.number[0] = i_flag;
    ui_number_update_by_id(AI_TIME, &num);
    wdt_clear();
}
static void AI_PIC_handler(void *priv)//录音UI的动画
{
    printf(">>>>>>>>>>>>>>>>PIC>>>>>>\n");
    static int i = 0;
    i++;
    ui_pic_show_image_by_id(AI_PIC, i);
    if (i == 10) {
        i = 1;
    }
    wdt_clear();
    return;
}
//
static int talk_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct window *window = (struct window *)ctr;
    struct element *elm = (struct element *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_SHOW:
        break;
    case ON_CHANGE_RELEASE:
        ui_auto_shut_down_enable();
        if (ai_countdown_timer) {
            sys_timer_del(ai_countdown_timer);
            ai_countdown_timer = 0;
        }
        if (ai_recoreai_gettxt1_timer) {
            sys_timer_del(ai_recoreai_gettxt1_timer);
            ai_recoreai_gettxt1_timer = 0;
        }
        break;
    default:
        break;
    }
    return 0;
}
static int talk_ontouch(void *ctr, struct element_touch_event *e)
{
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        if (!flag_button) { //按下按钮开始录音、动画、倒计时
            memset_txt();
            i_flag = 20;
            printf("开始录音\n");
            flag_button = 1;
            ai_interaction_rec_start();
            ui_text_show_index_by_id(AI_TEXT, 1);
            if (!ai_recoreai_gettxt1_timer) {
                ai_recoreai_gettxt1_timer = sys_timer_add(NULL, AI_PIC_handler, 200);
            }
            if (!ai_countdown_timer) {
                ai_countdown_timer = sys_timer_add(NULL, AI_time1_handler, 1000);
            }
        } else if (flag_button) { //1---停止录音
            ai_interaction_rec_stop();
            printf("停止录音\n");
            ui_text_show_index_by_id(AI_TEXT, 0);
            UI_MSG_POST("think");
            if (ai_countdown_timer) {
                sys_timer_del(ai_countdown_timer);
                ai_countdown_timer = 0;
                struct unumber num;       // 更新倒计时时间
                num.type = TYPE_NUM;
                num.numbs = 1;
                num.number[0] = 20;
                ui_number_update_by_id(AI_TIME, &num);
            }
            if (ai_recoreai_gettxt1_timer) {
                sys_timer_del(ai_recoreai_gettxt1_timer);
                ai_recoreai_gettxt1_timer = 0;
            }
            flag_button = 0;
        }
        break;
    default:
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(AI_TALK1)
.onchange = talk_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(AI_CONFIG)
.onchange = talk_onchange,
 .onkey = NULL,
  .ontouch = talk_ontouch,
};










static int think_ontouch(void *ctr, struct element_touch_event *e)//在语音识别界面手势滑动，退出语音识别到列表，停止录音
{
    switch (e->event) {
    default:
        break;
    }
    return true;
}
static void AI_thinking_handler(void *priv)
{
    printf("think\n");
    static int i = 0;
    i++;
    struct ui_pic *pic = ui_pic_for_id(AI_THINKING);
    if (pic) {
        i++;
        ui_core_set_element_rotate(pic, 115, 115, 155, 155, 36 * i, true);
        ui_core_redraw(pic);
    }
    flag++;
    if (flag == 25) { //等待5秒没有获得信号，切到列表
        flag = 0;
        page_showtxt();
    }
    wdt_clear();
    return;
}
static int think_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct element *elm = (struct element *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_SHOW_POST:
        if (cro_ui == 2) {
            if (!ai_thinking_timer) {
                ai_thinking_timer = sys_timer_add(NULL, AI_thinking_handler, 200);
            }
        }
        break;
    case ON_CHANGE_RELEASE:
        if (ai_thinking_timer) {
            sys_timer_del(ai_thinking_timer);
            ai_thinking_timer = 0;
        }
        break;
    default:
        break;
    }
    return 0;
}
REGISTER_UI_EVENT_HANDLER(AI_THINKSTR)
.onchange = think_onchange,
 .onkey = NULL,
  .ontouch = think_ontouch,
};


static void ai_show_empty_handler(void *priv)
{
    if (tts_txt) {
        if (strlen((char *)tts_txt) == 0) {
            if (empty_str) {
                char empty_str_copy[50] = "您好像并没有说话";
                strcpy(empty_str, empty_str_copy);
            }
            ui_text_set_textu_by_id(AI_TXT2, (char *)empty_str, strlen((char *)empty_str), FONT_DEFAULT | FONT_SHOW_MULTI_LINE);
            return;
        }
    } else {
        if (empty_str) {
            char empty_str_copy[50] = "您好像并没有说话";
            strcpy(empty_str, empty_str_copy);
        }
        ui_text_set_textu_by_id(AI_TXT2, (char *)empty_str, strlen((char *)empty_str), FONT_DEFAULT | FONT_SHOW_MULTI_LINE);
    }
    if (ai_show_empty_timer) {
        sys_timeout_del(ai_show_empty_timer);
        ai_show_empty_timer = 0;
    }
}

static void reflash_gettxt_handler(void *priv)
{
    if (tts_txt) {
        if (!strlen((char *)tts_txt)) {
            return;
        }
        ui_text_set_textu_by_id(AI_TXT2, (char *)tts_txt, strlen((char *)tts_txt), FONT_DEFAULT | FONT_SHOW_MULTI_LINE | FONT_VERTICAL_SCROLL);
        struct font_info *info = NULL;
        info = text_font_init(true);
        int height = info->text_height;
        info->text_height = 10000;
        font_get_text_width(FONT_ENCODE_UTF8, info, (u8 *)tts_txt, strlen((char *)tts_txt));
        if (!string_height) {
            string_height = info->string_height;
        }
        info->text_height = height;
        if (ai_show_empty_timer) {
            sys_timeout_del(ai_show_empty_timer);
            ai_show_empty_timer = 0;
        }
    } else {
        tts_txt = NULL;
    }
}
static void reflash_gettxtII_handler(void *priv)
{
    if (ai_txt1) {
        ui_text_set_textu_by_id(AI_TXT1, (char *)ai_txt1, strlen((char *)ai_txt1), FONT_DEFAULT | FONT_SHOW_SCROLL);
    } else {
        ai_txt1 = NULL;
        ui_text_set_textu_by_id(AI_TXT1, (char *)ai_txt1, 0, FONT_DEFAULT | FONT_SHOW_SCROLL);
    }
}
void memset_txt(void)
{
    if (tts_txt) {
        memset((char *)tts_txt, '\0', strlen((char *)tts_txt) + 1);
    }
    if (ai_txt1) {
        memset((char *)ai_txt1, '\0', strlen((char *)ai_txt1) + 1);
    }
    if (ai_txt2) {
        memset((char *)ai_txt2, '\0', strlen((char *)ai_txt2) + 1);
    }
}
static int showtxt_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:
        if (!empty_str) {
            empty_str = zalloc(sizeof(char) * 50);
        }
        break;
    case ON_CHANGE_SHOW_POST:
        if (cro_ui == 3) {
            if (!ai_gettxt1_timer) {
                ai_gettxt1_timer = sys_timer_add(NULL, reflash_gettxt_handler, 1000);
            }
            if (!ai_gettxt2_timer) {
                ai_gettxt2_timer = sys_timer_add(NULL, reflash_gettxtII_handler, 1000);
            }
            if (!ai_show_empty_timer) {
                ai_show_empty_timer = sys_timeout_add(NULL, ai_show_empty_handler, 2000);

            }
        }
        break;
    case ON_CHANGE_RELEASE:
        if (ai_gettxt1_timer) {
            sys_timer_del(ai_gettxt1_timer);
            ai_gettxt1_timer = 0;
        }
        if (ai_gettxt2_timer) {
            sys_timer_del(ai_gettxt2_timer);
            ai_gettxt2_timer = 0;
        }
        if (ai_show_empty_timer) {
            sys_timeout_del(ai_show_empty_timer);
            ai_show_empty_timer = 0;
        }
        if (empty_str) {
            free(empty_str);
            empty_str = NULL;
        }
        break;
    default:
        break;
    }
    return 0;
}
static int showtxt_ontouch(void *ctr, struct element_touch_event *e)
{
    switch (e->event) {
    case ELM_EVENT_TOUCH_R_MOVE:
        tts_txt = ai_txttempa;
        page_switch(1);
        string_height = 0;
        last_pos_y = 0;
        y_move_total = 0;
        if (ai_gettxt1_timer) {
            sys_timer_del(ai_gettxt1_timer);
            ai_gettxt1_timer = 0;
        }
        if (ai_gettxt2_timer) {
            sys_timer_del(ai_gettxt2_timer);
            ai_gettxt2_timer = 0;
        }
        if (ai_show_empty_timer) {
            sys_timeout_del(ai_show_empty_timer);
            ai_show_empty_timer = 0;
        }
        break;
    case ELM_EVENT_TOUCH_L_MOVE:
        tts_txt = ai_txttempb;
        break;
    default:
        break;
    }
    return true;
}

REGISTER_UI_EVENT_HANDLER(AI_SHOW)
.onchange = showtxt_onchange,
 .onkey = NULL,
  .ontouch = showtxt_ontouch,
};

static int ai_showtxt_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_text *text = ui_text_for_id(AI_TXT2);
    switch (e->event) {
    case ELM_EVENT_TOUCH_MOVE:
        int y_move = e->pos.y - last_pos_y;
        int height = text->elm.css.height;
        last_pos_y = e->pos.y;
        y_move_total += y_move;
        if (string_height < height) {
            break;
        }
        if (y_move_total >= 0) {
            y_move_total = 0;
        }
        if (-y_move_total >= string_height - height) {
            y_move_total = -(string_height - height);
        }
        text->attrs.top_extra_fill = y_move_total;
        ui_core_redraw(text);
        break;
    case ELM_EVENT_TOUCH_DOWN:
        last_pos_y = e->pos.y;

        return true;
    default:
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(AI_TXT2)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = ai_showtxt_ontouch,
};
#endif
