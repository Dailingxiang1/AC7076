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

#define LOG_TAG_CONST       UI
#define LOG_TAG     		"[UI_BREATHTRAIN]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ui_action_breathtrain.data.bss")
#pragma data_seg(".ui_action_breathtrain.data")
#pragma const_seg(".ui_action_breathtrain.text.const")
#pragma code_seg(".ui_action_breathtrain.text")
#endif

#if (defined (CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE))
#if TCFG_UI_ENABLE_BREATH_TRAIN

#define STYLE_NAME  JL

REGISTER_UI_STYLE(STYLE_NAME)

typedef struct {
    u32 breath_time;		//呼吸时间
    u32 breath_rhythm;		//呼吸节奏
    u32 heart_rate;			//平均心率
    u32 cur_layout;
    u16 test_timer;
} BREATHTRAIN_UI_PARAM;

static BREATHTRAIN_UI_PARAM *breath_train_ui_handler = NULL;

#define __this		breath_train_ui_handler
#define PIC_PLAY_POSITIVE		0		//按照图片列表正序播放
#define PIC_PLAY_NEGATIVE		1		//按照图片列表倒序播放
#define PIC_PLAY_CROSS			2		//正反交叉播放
#define RESP_REASON_ONCE		0
#define RESP_REASON_STOP		1

typedef struct {
    u32 id;
    u32 cnt;		//播放的图片的数量
    int play_index;	//当前播放的图片
    bool cur_order; //当前播放顺序
    u16 timer;
} PIC_ANIM;

static PIC_ANIM *train_pic_anim = NULL;
static PIC_ANIM *train_start_anim = NULL;

static void pic_anim_init(PIC_ANIM **pic_anim)
{
    if (*pic_anim == NULL) {
        *pic_anim = malloc(sizeof(PIC_ANIM));
    }
    if (pic_anim) {
        memset(*pic_anim, 0, sizeof(PIC_ANIM));
    }
}

static void pic_anim_release(PIC_ANIM **pic_anim)
{
    if (*pic_anim) {
        free(*pic_anim);
        *pic_anim = NULL;
    }
}

static void breathtrain_parm_init(void)
{
    if (__this) {
        __this->breath_time = 1;
        __this->breath_rhythm = 1;
        __this->heart_rate = 0;
        __this->cur_layout = BREATHTRAIN_MAIN_LAYOUT;
        __this->test_timer = 0;
    }
}

static u32 ui_list_get_child_id(u32 list_id, u32 index)
{
    struct ui_grid *grid;
    struct element *elm;

    grid = (struct ui_grid *)ui_core_get_element_by_id(list_id);
    elm = (struct element *) &grid->item[index];

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

static u32 ui_optbox_get_opt(u32 optbox_id)
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


//一个page中含有多个布局需要跳转，使用一个数组来管理
static u32 breathtrain_layout[] = {
    BREATHTRAIN_MAIN_LAYOUT,
    BREATHTRAIN_TIME_LAYOUT,
    BREATHTRAIN_START_LAYOUT,
    BREATHTRAIN_FIN_LAYOUT,
    BREATHTRAIN_RHYTHM_LAYOUT
};

static void breathtrain_layout_show(u32 layout_id)
{
    u32 layout_num;

    layout_num = sizeof(breathtrain_layout) / sizeof(u32);

    for (int i = 0; i < layout_num; i++) {
        if (layout_id == breathtrain_layout[i]) {
            ui_hide(__this->cur_layout);
            ui_show(breathtrain_layout[i]);
            __this->cur_layout = layout_id;
            break;
        } else if (i == layout_num - 1) {
            return;
        }
    }
}

static int breathtrain_button_ontouch(void *ctr, struct element_touch_event *e)
{
    struct element *elm = (struct element *)ctr;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:
        switch (elm->id) {
        case BREATHTRAIN_TIME_BUTTON:
            breathtrain_layout_show(BREATHTRAIN_TIME_LAYOUT);
            break;
        case BREATHTRAIN_RHYTHM_BUTTON:
            breathtrain_layout_show(BREATHTRAIN_RHYTHM_LAYOUT);
            break;
        case BREATHTRAIN_START_BUTTON:
            breathtrain_layout_show(BREATHTRAIN_START_LAYOUT);
            break;
        }
        break;
    }
    return false;

}

REGISTER_UI_EVENT_HANDLER(BREATHTRAIN_TIME_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = breathtrain_button_ontouch,
};

REGISTER_UI_EVENT_HANDLER(BREATHTRAIN_RHYTHM_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = breathtrain_button_ontouch,
};

REGISTER_UI_EVENT_HANDLER(BREATHTRAIN_START_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = breathtrain_button_ontouch,
};


static int breathtrain_time_layout_ontouch(void *ctr, struct element_touch_event *e)
{
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:
        break;
    case ELM_EVENT_TOUCH_R_MOVE:
        breathtrain_layout_show(BREATHTRAIN_MAIN_LAYOUT);
        break;
    }
    return true;

}

REGISTER_UI_EVENT_HANDLER(BREATHTRAIN_TIME_LAYOUT)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = breathtrain_time_layout_ontouch,
};

static int breathtrain_rhythm_layout_ontouch(void *ctr, struct element_touch_event *e)
{
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:
        break;
    case ELM_EVENT_TOUCH_R_MOVE:
        breathtrain_layout_show(BREATHTRAIN_MAIN_LAYOUT);
        break;
    }
    return true;

}

REGISTER_UI_EVENT_HANDLER(BREATHTRAIN_RHYTHM_LAYOUT)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = breathtrain_rhythm_layout_ontouch,
};

static int breathtrain_start_layout_ontouch(void *ctr, struct element_touch_event *e)
{
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:
        break;
    case ELM_EVENT_TOUCH_R_MOVE:
        breathtrain_layout_show(BREATHTRAIN_MAIN_LAYOUT);
        break;
    }
    return true;

}

REGISTER_UI_EVENT_HANDLER(BREATHTRAIN_START_LAYOUT)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = breathtrain_start_layout_ontouch,
};

static void train_start_pic_play(void *p)
{
    PIC_ANIM *pic_anim = (PIC_ANIM *)p;
    if (pic_anim->timer == 0) {
        return;
    }

    pic_anim->play_index++;
    if (pic_anim->play_index >= pic_anim->cnt) {
        pic_anim->cur_order = 0;

        //倒计时图片播放结束后，跳转到开始训练
        if (pic_anim->timer) {
            sys_timer_del(pic_anim->timer);
            pic_anim->timer = 0;
        }
        ui_hide(BREATHTRAIN_START_PIC);
        ui_show(BREATHTRAIN_ANIMATION_LAYOUT);
        return;
    }
    ui_pic_show_image_by_id(pic_anim->id, pic_anim->play_index);
}

static void train_start_pic_stop(void *p)
{
    if (train_start_anim->timer) {
        sys_timer_del(train_start_anim->timer);
        train_start_anim->timer = 0;
    }
}


static int breathtrain_start_pic_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    /* log_info("%s", __func__); */
    struct element *elm = (struct element *)_ctrl;
    struct unumber numb;

    switch (event) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_SHOW:
        break;
    case ON_CHANGE_FIRST_SHOW:
        pic_anim_init(&train_start_anim);
        ui_auto_shut_down_disable();
        train_start_anim->id = BREATHTRAIN_START_PIC;
        train_start_anim->cnt = 3;
        train_start_anim->play_index = 0;
        train_start_anim->cur_order = 0;
        if (train_start_anim->timer == 0) {
            train_start_anim->timer = sys_timer_add((void *)train_start_anim, train_start_pic_play, 1000);
        }
        break;
    case ON_CHANGE_SHOW_POST:
        break;
    case ON_CHANGE_HIDE:
        break;
    case ON_CHANGE_RELEASE:
        printf("start pic RELEASE!!!");
        if (train_start_anim) {
            if (train_start_anim->timer) {
                sys_timer_del(train_start_anim->timer);
                train_start_anim->timer = 0;
            }
            pic_anim_release(&train_start_anim);
        }
        break;
    default:
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(BREATHTRAIN_START_PIC)
.onchange = breathtrain_start_pic_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static void train_pic_play(void *p)
{
    PIC_ANIM *pic_anim = (PIC_ANIM *)p;
    if (pic_anim->timer == 0) {
        return;
    }

    /* ui_pic_show_image_by_id(pic_anim->id, pic_anim->play_index); */
    struct element *pic_elm;
    pic_elm = ui_core_get_element_by_id(pic_anim->id);

    ui_core_set_element_ratio(pic_elm, (float)(pic_anim->play_index + 1) * 5 / 100, (float)(pic_anim->play_index + 1) * 5 / 100, true);
    ui_core_redraw(pic_elm);

    if (pic_anim->cur_order == 1) {
        pic_anim->play_index++;
        if (pic_anim->play_index >= pic_anim->cnt) {
            pic_anim->cur_order = 0;
            pic_anim->play_index = pic_anim->cnt - 1;
            ui_text_show_index_by_id(BREATHTRAIN_ANIMATION_TEXT, 0);
        }
    } else if (pic_anim->cur_order == 0) {
        pic_anim->play_index--;
        if (pic_anim->play_index < 0) {
            pic_anim->cur_order = 1;
            pic_anim->play_index = 0;
            ui_text_show_index_by_id(BREATHTRAIN_ANIMATION_TEXT, 1);
        }
    }
}

static void train_pic_stop(void *p)
{
    if (train_pic_anim->timer) {
        sys_timer_del(train_pic_anim->timer);
        train_pic_anim->timer = 0;
    }
    breathtrain_layout_show(BREATHTRAIN_FIN_LAYOUT);
}

static u16 train_pic_play_timer = 0;

static int breathtrain_animation_layout_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    /* log_info("%s", __func__); */
    struct element *elm = (struct element *)_ctrl;
    struct unumber numb;

    switch (event) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_SHOW:
        break;
    case ON_CHANGE_FIRST_SHOW:
        pic_anim_init(&train_pic_anim);
        train_pic_anim->id = BREATHTRAIN_ANIMATION_PIC;
        train_pic_anim->cnt = 20;
        train_pic_anim->play_index = 0;
        train_pic_anim->cur_order = 1;
        ui_text_show_index_by_id(BREATHTRAIN_ANIMATION_TEXT, 1);
        if (train_pic_anim->timer == 0) {
            train_pic_anim->timer = sys_timer_add((void *)train_pic_anim, train_pic_play, __this->breath_rhythm * 20);
        }
        train_pic_play_timer = sys_timeout_add(NULL, train_pic_stop, __this->breath_time * 60 * 1000);
        break;
    case ON_CHANGE_SHOW_POST:
        break;
    case ON_CHANGE_ANIMATION_END:
        break;
    case ON_CHANGE_RELEASE:
        printf("ANIM LAYOUT RELEASE!!!");
        ui_auto_shut_down_enable();
        if (train_pic_anim) {
            if (train_pic_anim->timer) {
                sys_timer_del(train_pic_anim->timer);
                train_pic_anim->timer = 0;
            }
            if (train_pic_play_timer) {
                sys_timeout_del(train_pic_play_timer);
                train_pic_play_timer = 0;
            }
            pic_anim_release(&train_pic_anim);
        }
        break;
    default:
        break;
    }
    return false;
}

static int breathtrain_animation_layout_ontouch(void *ctr, struct element_touch_event *e)
{
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_UP:
        break;
    case ELM_EVENT_TOUCH_R_MOVE:
        breathtrain_layout_show(BREATHTRAIN_MAIN_LAYOUT);
        break;
    }
    return true;

}
REGISTER_UI_EVENT_HANDLER(BREATHTRAIN_ANIMATION_LAYOUT)
.onchange = breathtrain_animation_layout_onchange,
 .onkey = NULL,
  .ontouch = breathtrain_animation_layout_ontouch,
};

static int breathtrain_ok_button_ontouch(void *ctr, struct element_touch_event *e)
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
        //更新选项框的值
        opt_index = ui_optbox_get_opt(BREATHTRAIN_TIME_VLIST);
        __this->breath_time = opt_index + 1;
        printf("BREATH TIME SET:%d\n", __this->breath_time);
        breathtrain_layout_show(BREATHTRAIN_MAIN_LAYOUT);
        break;
    case ELM_EVENT_TOUCH_U_MOVE:
        break;
    default:
        break;
    }
    return false;//接管消息
}


REGISTER_UI_EVENT_HANDLER(BREATHTRAIN_OK_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = breathtrain_ok_button_ontouch,
};

static int breathtrain_rhythmok_button_ontouch(void *ctr, struct element_touch_event *e)
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
        //更新选项框的值
        opt_index = ui_optbox_get_opt(BREATHTRAIN_RHYTHM_VLIST);
        __this->breath_rhythm = opt_index + 1;
        printf("BREATH RHYTHM SET:%d\n", __this->breath_rhythm);
        breathtrain_layout_show(BREATHTRAIN_MAIN_LAYOUT);
        break;
    case ELM_EVENT_TOUCH_U_MOVE:
        break;
    default:
        break;
    }
    return false;//接管消息
}


REGISTER_UI_EVENT_HANDLER(BREATHTRAIN_RHYTHMOK_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = breathtrain_rhythmok_button_ontouch,
};

static int breathtrain_finok_button_ontouch(void *ctr, struct element_touch_event *e)
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
        breathtrain_layout_show(BREATHTRAIN_MAIN_LAYOUT);
        break;
    case ELM_EVENT_TOUCH_U_MOVE:
        break;
    default:
        break;
    }
    return false;//接管消息
}


REGISTER_UI_EVENT_HANDLER(BREATHTRAIN_FINOK_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = breathtrain_finok_button_ontouch,
};

static int breathtrain_main_layout_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    /* log_info("%s", __func__); */
    struct element *elm = (struct element *)_ctrl;
    struct unumber numb;

    switch (event) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_SHOW:
        break;
    case ON_CHANGE_FIRST_SHOW:
        printf("MAIN layout first show!!!\n");
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

REGISTER_UI_EVENT_HANDLER(BREATHTRAIN_MAIN_LAYOUT)
.onchange = breathtrain_main_layout_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int breathtrain_fin_layout_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    /* log_info("%s", __func__); */
    struct element *elm = (struct element *)_ctrl;
    struct ui_text *text = NULL;
    struct ui_number *number = NULL;
    struct unumber numb;

    switch (event) {
    case ON_CHANGE_INIT:
        text = (struct ui_text *)ui_core_get_element_by_id(BREATHTRAIN_FINTIME_TEXT);
        number = (struct ui_number *)ui_core_get_element_by_id(BREATHTRAIN_FINHR_NUM);

        ui_text_set_index(text, __this->breath_time - 1);
        numb.type = TYPE_NUM;
        numb.numbs = 1;
        numb.number[0] = __this->heart_rate;

        ui_number_update(number, &numb);
        break;
    case ON_CHANGE_SHOW:
        printf("FIN layout show!!!\n");
        break;
    case ON_CHANGE_FIRST_SHOW:
        printf("fin layout first show!!!\n");
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

REGISTER_UI_EVENT_HANDLER(BREATHTRAIN_FIN_LAYOUT)
.onchange = breathtrain_fin_layout_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int window_breath_train_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    /* log_info("%s", __func__); */
    struct element *elm = (struct element *)_ctrl;
    struct unumber numb;
    struct draw_context *dc = (struct draw_context *)arg;
    struct rect rect;

    switch (event) {
    case ON_CHANGE_INIT:
        printf("ID_WINDOW_BREATH_TRAIN init!!!");
        if (!__this) {
            __this = malloc(sizeof(BREATHTRAIN_UI_PARAM));
        }
        breathtrain_parm_init();
        break;
    case ON_CHANGE_RELEASE:
        printf("ID_WINDOW_BREATH_TRAIN release!!!");
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

REGISTER_UI_EVENT_HANDLER(ID_WINDOW_BREATH_TRAIN)
.onchange = window_breath_train_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};




#endif /*if TCFG_UI_ENABLE_BREATH_TRAIN*/
#endif /*#if (defined(CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE))*/

