#include "app_config.h"
#include "ui/ui_api.h"

#define LOG_TAG_CONST       UI
#define LOG_TAG     		"[UI_TOUCH_CTRL]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ui_time_set.data.bss")
#pragma data_seg(".ui_time_set.data")
#pragma const_seg(".ui_time_set.text.const")
#pragma code_seg(".ui_time_set.text")
#endif

#ifdef CONFIG_UI_STYLE_JL_CSC_PUBLIC_MODLS_ENABLE
#if (defined TCFG_UI_TIME_SET_ENABLE && TCFG_UI_TIME_SET_ENABLE)

#define STYLE_NAME  JL

extern void rtc_read_time(struct sys_time *time);
extern void rtc_write_time(const struct sys_time *time);

static void get_sys_time(struct sys_time *time)//获取时间
{
    rtc_read_time(time);
}
static int Y_M_D_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_number *number = (struct ui_number *)_ctrl;
    struct sys_time get_time = {0};
    struct unumber num = {0};
    static u8 str[20] = {0};
    static u8 str1[20] = {0};
    switch (event) {
    case ON_CHANGE_INIT:
        switch (number->text.elm.id) {
        case Y_M_D:
            get_sys_time(&get_time);
            sprintf((char *)str, "%04d:%02d:%02d", get_time.year, get_time.month, get_time.day);
            num.type = TYPE_STRING;
            num.num_str = str;
            ui_number_update(number, &num);
            break;
        default:
            break;
        }
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        break;
    }
    return false;
}


REGISTER_UI_EVENT_HANDLER(Y_M_D)
.onchange = Y_M_D_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};


static int H_M_S_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_time *time = (struct ui_time *)_ctrl;
    switch (event) {
    case ON_CHANGE_INIT:
        struct utime time_num = {0};
        struct sys_time time_t;
        rtc_read_time(&time_t);
        time_num.hour = time_t.hour;
        time_num.min = time_t.min;
        time_num.sec = time_t.sec;
        time_num.year = time_t.year;
        time_num.month = time_t.month;
        time_num.day = time_t.day;
        ui_time_update(time, &time_num);
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        break;
    }
    return false;
}


REGISTER_UI_EVENT_HANDLER(H_M_S)
.onchange = H_M_S_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};



static int page_layout_1403_timer = 0;


static void ui_layout_1403_update(void *p)
{
    // return ;

    if (strcmp(os_current_task(), "ui")) {
        int argv[3];
        argv[0] = (int)ui_layout_1403_update;
        argv[1] = 0;
        os_taskq_post_type("ui", Q_CALLBACK, ARRAY_SIZE(argv), argv);
        return;
    }

}


static int page_layout_1403_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    switch (event) {
    case ON_CHANGE_INIT:
        if (!page_layout_1403_timer) {
            page_layout_1403_timer = sys_timer_add(NULL, ui_layout_1403_update, 500);
        }
        break;
    case ON_CHANGE_RELEASE:
        if (page_layout_1403_timer) {
            sys_timer_del(page_layout_1403_timer);
            page_layout_1403_timer = 0;
        }
        break;
    default:
        break;
    }
    return false;
}


REGISTER_UI_EVENT_HANDLER(SET_TIME_MAIN)
.onchange = page_layout_1403_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};




static int btn1_ontouch(void *ctr, struct element_touch_event *e)
{
    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        ui_hide(SET_TIME_MAIN);
        ui_show(TIMER_1);
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_DOWN:
        return true;
    case ELM_EVENT_TOUCH_U_MOVE:
        break;
    case ELM_EVENT_TOUCH_D_MOVE:
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(CALENDAR_DATA)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = btn1_ontouch,
};



static int btn2_ontouch(void *ctr, struct element_touch_event *e)
{
    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        ui_hide(SET_TIME_MAIN);
        ui_show(TIMER_2);
        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_DOWN:
        return true;
    case ELM_EVENT_TOUCH_U_MOVE:
        break;
    case ELM_EVENT_TOUCH_D_MOVE:
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(CLOCK_TIME)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = btn2_ontouch,
};




static int set_timer_year_vlist_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)_ctrl;
    int row, col;
    int base_index_once;
    int time_hour = 0;
    int base = 10000;
    int first_move_step = 0;
    struct rect r;
    struct sys_time get_time = {0};
    switch (event) {
    case ON_CHANGE_INIT:
        switch (grid->elm.id) {
        case SET_TIMER_YEAR_VLIST:
            get_sys_time(&get_time);

            base_index_once = base * 9999;
            row = base_index_once;
            col = 1;
            ui_grid_init_dynamic(grid, &row, &col);
            log_info("dynamic_grid %d X %d\n", row, col);

            base = (base / 2) * 9999;

            ui_grid_set_hindex_dynamic(grid, time_hour + base, true, 1);

            base_index_once = ((time_hour >= 1) ? (time_hour - 1) : 0) + base;
            ui_core_get_element_abs_rect(&grid->item[0].elm, &r);
            first_move_step = (time_hour == 0) ? r.height + 5 : 0;

            ui_grid_set_base_dynamic(grid, base_index_once, -((32 + 4) * (get_time.year - 1)));
            break;
        case SET_TIMER_MOUTH_VLIST:
            get_sys_time(&get_time);
            base_index_once = base * 12;
            row = base_index_once;
            col = 1;
            ui_grid_init_dynamic(grid, &row, &col);
            log_info("dynamic_grid %d X %d\n", row, col);

            base = (base / 2) * 12;

            ui_grid_set_hindex_dynamic(grid, time_hour + base, true, 1);

            base_index_once = ((time_hour >= 1) ? (time_hour - 1) : 0) + base;
            ui_core_get_element_abs_rect(&grid->item[0].elm, &r);
            first_move_step = (time_hour == 0) ? r.height + 5 : 0;

            ui_grid_set_base_dynamic(grid, base_index_once, -((32 + 4) * (get_time.month - 1)));
            break;
        case SET_TIMER_DAY_VLIST:
            get_sys_time(&get_time);
            base_index_once = base * 31;
            row = base_index_once;
            col = 1;
            ui_grid_init_dynamic(grid, &row, &col);
            log_info("dynamic_grid %d X %d\n", row, col);

            base = (base / 2) * 31;

            ui_grid_set_hindex_dynamic(grid, time_hour + base, true, 1);

            base_index_once = ((time_hour >= 1) ? (time_hour - 1) : 0) + base;
            ui_core_get_element_abs_rect(&grid->item[0].elm, &r);
            first_move_step = (time_hour == 0) ? r.height + 5 : 0;

            ui_grid_set_base_dynamic(grid, base_index_once, -((32 + 4) * (get_time.day - 1)));
            break;
        case SET_TIMER_HOUR_VLIST:
            get_sys_time(&get_time);

            base_index_once = base * 24;
            row = base_index_once;
            col = 1;
            ui_grid_init_dynamic(grid, &row, &col);
            log_info("dynamic_grid %d X %d\n", row, col);

            base = (base / 2) * 24;

            ui_grid_set_hindex_dynamic(grid, time_hour + base, true, 1);

            base_index_once = ((time_hour >= 1) ? (time_hour - 1) : 0) + base;
            ui_core_get_element_abs_rect(&grid->item[0].elm, &r);
            first_move_step = (time_hour == 0) ? r.height + 5 : 0;

            ui_grid_set_base_dynamic(grid, base_index_once, -((32 + 4) * (get_time.hour - 1)));
            break;
        case SET_TIMER_MIN_VLIST:
            get_sys_time(&get_time);
            base_index_once = base * 60;
            row = base_index_once;
            col = 1;
            ui_grid_init_dynamic(grid, &row, &col);
            log_info("dynamic_grid %d X %d\n", row, col);

            base = (base / 2) * 60;

            ui_grid_set_hindex_dynamic(grid, time_hour + base, true, 1);

            base_index_once = ((time_hour >= 1) ? (time_hour - 1) : 0) + base;
            ui_core_get_element_abs_rect(&grid->item[0].elm, &r);
            first_move_step = (time_hour == 0) ? r.height + 5 : 0;

            ui_grid_set_base_dynamic(grid, base_index_once, -((32 + 4) * (get_time.min - 1)));
            break;
        case SET_TIMER_SEC_VLIST:
            get_sys_time(&get_time);
            base_index_once = base * 60;
            row = base_index_once;
            col = 1;
            ui_grid_init_dynamic(grid, &row, &col);
            log_info("dynamic_grid %d X %d\n", row, col);

            base = (base / 2) * 60;

            ui_grid_set_hindex_dynamic(grid, time_hour + base, true, 1);

            base_index_once = ((time_hour >= 1) ? (time_hour - 1) : 0) + base;
            ui_core_get_element_abs_rect(&grid->item[0].elm, &r);
            first_move_step = (time_hour == 0) ? r.height + 5 : 0;

            ui_grid_set_base_dynamic(grid, base_index_once, -((32 + 4) * (get_time.sec - 1)));
            break;
        default:
            break;
        }
        ui_grid_set_slide_direction(grid, SCROLL_DIRECTION_UD);
        ui_grid_energy_auto_center(grid, AUTO_CENTER_MODE2);
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        break;
    }
    return 0;
}


REGISTER_UI_EVENT_HANDLER(SET_TIMER_YEAR_VLIST)//勿扰模式-开始时间-动态垂直列表
.onchange = set_timer_year_vlist_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

REGISTER_UI_EVENT_HANDLER(SET_TIMER_MOUTH_VLIST)//勿扰模式-开始时间-动态垂直列表
.onchange = set_timer_year_vlist_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};


REGISTER_UI_EVENT_HANDLER(SET_TIMER_DAY_VLIST)//勿扰模式-开始时间-动态垂直列表
.onchange = set_timer_year_vlist_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

REGISTER_UI_EVENT_HANDLER(SET_TIMER_HOUR_VLIST)//勿扰模式-开始时间-动态垂直列表
.onchange = set_timer_year_vlist_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};


REGISTER_UI_EVENT_HANDLER(SET_TIMER_MIN_VLIST)//勿扰模式-开始时间-动态垂直列表
.onchange = set_timer_year_vlist_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

REGISTER_UI_EVENT_HANDLER(SET_TIMER_SEC_VLIST)//勿扰模式-开始时间-动态垂直列表
.onchange = set_timer_year_vlist_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};


static int timer_year_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_number *number = (struct ui_number *)_ctrl;
    struct sys_time get_time = {0};
    struct unumber num = {0};
    static u8 *time_use_buf = NULL;
    if (time_use_buf == NULL) {
        time_use_buf = zalloc(10 * 24);
    }
    u8 *str[24]; // 创建指针数组
    for (u8 i = 0; i < 24; i++) {
        str[i] = time_use_buf + i * 10;
    }
    int index;
    switch (event) {
    case ON_CHANGE_UPDATE_ITEM:
        switch (number->text.elm.id) {
        case YEAR_NUM_FIR:
            index = (u32)arg;
            index = index % 9999;//这里必须求余数方式获取索引
            printf("tid1 %d\n", index);
            sprintf((char *)str[0], "%04d", index);
            num.type = TYPE_STRING;
            num.num_str = str[0];
            ui_number_update(number, &num);
            break;
        case YEAR_NUM_SEC:
            index = (u32)arg;
            index = index % 9999;//这里必须求余数方式获取索引
            printf("tid2 %d\n", index);
            sprintf((char *)str[1], "%04d", index);
            num.type = TYPE_STRING;
            num.num_str = str[1];
            ui_number_update(number, &num);
            break;
        case YEAR_NUM_THI:
            index = (u32)arg;
            index = index % 9999;//这里必须求余数方式获取索引
            printf("tid3 %d\n", index);
            sprintf((char *)str[2], "%04d", index);
            num.type = TYPE_STRING;
            num.num_str = str[2];
            ui_number_update(number, &num);
            break;
        case YEAR_NUM_FOU:
            index = (u32)arg;
            index = index % 9999;//这里必须求余数方式获取索引
            printf("tid4 %d\n", index);
            sprintf((char *)str[3], "%04d", index);
            num.type = TYPE_STRING;
            num.num_str = str[3];
            ui_number_update(number, &num);
            break;
        case MON_NUM_FIR:
            index = (u32)arg;
            index = index % 12;//这里必须求余数方式获取索引
            if (index == 0) {
                index = 12;
            }
            printf("tid1 %d\n", index);
            sprintf((char *)str[4], "%02d", index);
            num.type = TYPE_STRING;
            num.num_str = str[4];
            ui_number_update(number, &num);
            break;
        case MON_NUM_SEC:
            index = (u32)arg;
            index = index % 12;//这里必须求余数方式获取索引
            if (index == 0) {
                index = 12;
            }
            printf("tid2 %d\n", index);
            sprintf((char *)str[5], "%02d", index);
            num.type = TYPE_STRING;
            num.num_str = str[5];
            ui_number_update(number, &num);
            break;
        case MON_NUM_THI:
            index = (u32)arg;
            index = index % 12;//这里必须求余数方式获取索引
            if (index == 0) {
                index = 12;
            }
            printf("tid3 %d\n", index);
            sprintf((char *)str[6], "%02d", index);
            num.type = TYPE_STRING;
            num.num_str = str[6];
            ui_number_update(number, &num);
            break;
        case MON_NUM_FOU:
            index = (u32)arg;
            index = index % 12;//这里必须求余数方式获取索引
            if (index == 0) {
                index = 12;
            }
            printf("tid4 %d\n", index);
            sprintf((char *)str[7], "%02d", index);
            num.type = TYPE_STRING;
            num.num_str = str[7];
            ui_number_update(number, &num);
            break;

        case DAY_NUM_FIR:
            index = (u32)arg;
            index = index % 31;//这里必须求余数方式获取索引
            if (index == 0) {
                index = 31;
            }
            printf("tid1 %d\n", index);
            sprintf((char *)str[8], "%02d", index);
            num.type = TYPE_STRING;
            num.num_str = str[8];
            ui_number_update(number, &num);
            break;
        case DAY_NUM_SEC:
            index = (u32)arg;
            index = index % 31;//这里必须求余数方式获取索引
            if (index == 0) {
                index = 31;
            }
            printf("tid2 %d\n", index);
            sprintf((char *)str[9], "%02d", index);
            num.type = TYPE_STRING;
            num.num_str = str[9];
            ui_number_update(number, &num);
            break;
        case DAY_NUM_THI:
            index = (u32)arg;
            index = index % 31;//这里必须求余数方式获取索引
            if (index == 0) {
                index = 31;
            }
            printf("tid3 %d\n", index);
            sprintf((char *)str[10], "%02d", index);
            num.type = TYPE_STRING;
            num.num_str = str[10];
            ui_number_update(number, &num);
            break;
        case DAY_NUM_FOU:
            index = (u32)arg;
            index = index % 31;//这里必须求余数方式获取索引
            if (index == 0) {
                index = 31;
            }
            printf("tid4 %d\n", index);
            sprintf((char *)str[11], "%02d", index);
            num.type = TYPE_STRING;
            num.num_str = str[11];
            ui_number_update(number, &num);
            break;
        case HOUR_NUM_FIR:
            index = (u32)arg;
            index = index % 24;//这里必须求余数方式获取索引
            printf("tid1 %d\n", index);
            sprintf((char *)str[12], "%02d", index);
            /* printf("[msg]%s-%d>>>>>>>>>>>str12=%s",__FUNCTION__, __LINE__,str12 ); */
            num.type = TYPE_STRING;
            num.num_str = str[12];
            ui_number_update(number, &num);
            break;
        case HOUR_NUM_SEC:
            index = (u32)arg;
            index = index % 24;//这里必须求余数方式获取索引
            printf("tid2 %d\n", index);
            sprintf((char *)str[13], "%02d", index);
            num.type = TYPE_STRING;
            num.num_str = str[13];
            ui_number_update(number, &num);
            break;
        case HOUR_NUM_THI:
            index = (u32)arg;
            index = index % 24;//这里必须求余数方式获取索引
            printf("tid3 %d\n", index);
            sprintf((char *)str[14], "%02d", index);
            num.type = TYPE_STRING;
            num.num_str = str[14];
            ui_number_update(number, &num);
            break;
        case HOUR_NUM_FOU:
            index = (u32)arg;
            index = index % 24;//这里必须求余数方式获取索引
            printf("tid4 %d\n", index);
            sprintf((char *)str[15], "%02d", index);
            num.type = TYPE_STRING;
            num.num_str = str[15];
            ui_number_update(number, &num);
            break;
        case MIN_NUM_FIR:
            index = (u32)arg;
            index = index % 60;//这里必须求余数方式获取索引
            printf("tid1 %d\n", index);
            sprintf((char *)str[16], "%02d", index);
            num.type = TYPE_STRING;
            num.num_str = str[16];
            ui_number_update(number, &num);
            break;
        case MIN_NUM_SEC:
            index = (u32)arg;
            index = index % 60;//这里必须求余数方式获取索引
            printf("tid2 %d\n", index);
            sprintf((char *)str[17], "%02d", index);
            num.type = TYPE_STRING;
            num.num_str = str[17];
            ui_number_update(number, &num);
            break;
        case MIN_NUM_THI:
            index = (u32)arg;
            index = index % 60;//这里必须求余数方式获取索引
            printf("tid3 %d\n", index);
            sprintf((char *)str[18], "%02d", index);
            num.type = TYPE_STRING;
            num.num_str = str[18];
            ui_number_update(number, &num);
            break;
        case MIN_NUM_FOU:
            index = (u32)arg;
            index = index % 60;//这里必须求余数方式获取索引
            printf("tid4 %d\n", index);
            sprintf((char *)str[19], "%02d", index);
            num.type = TYPE_STRING;
            num.num_str = str[19];
            ui_number_update(number, &num);
            break;
        case SEC_NUM_FIR:
            index = (u32)arg;
            index = index % 60;//这里必须求余数方式获取索引
            printf("tid1 %d\n", index);
            sprintf((char *)str[20], "%02d", index);
            /* printf("[msg]%s-%d>>>>>>>>>>>str20=%s",__FUNCTION__, __LINE__,str20 ); */
            num.type = TYPE_STRING;
            num.num_str = str[20];
            ui_number_update(number, &num);
            break;
        case SEC_NUM_SEC:
            index = (u32)arg;
            index = index % 60;//这里必须求余数方式获取索引
            printf("tid2 %d\n", index);
            sprintf((char *)str[21], "%02d", index);
            num.type = TYPE_STRING;
            num.num_str = str[21];
            ui_number_update(number, &num);
            break;
        case SEC_NUM_THI:
            index = (u32)arg;
            index = index % 60;//这里必须求余数方式获取索引
            printf("tid3 %d\n", index);
            sprintf((char *)str[22], "%02d", index);
            num.type = TYPE_STRING;
            num.num_str = str[22];
            ui_number_update(number, &num);
            break;
        case SEC_NUM_FOU:
            index = (u32)arg;
            index = index % 60;//这里必须求余数方式获取索引
            printf("tid4 %d\n", index);
            sprintf((char *)str[23], "%02d", index);
            num.type = TYPE_STRING;
            num.num_str = str[23];
            ui_number_update(number, &num);
            break;
        default:
            break;
        }
        break;
    case ON_CHANGE_RELEASE:
        if (time_use_buf != NULL) {
            free(time_use_buf);
            time_use_buf = NULL;
        }
        break;
    default:
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(YEAR_NUM_FIR)
.onchange = timer_year_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

REGISTER_UI_EVENT_HANDLER(YEAR_NUM_SEC)
.onchange = timer_year_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

REGISTER_UI_EVENT_HANDLER(YEAR_NUM_THI)
.onchange = timer_year_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

REGISTER_UI_EVENT_HANDLER(YEAR_NUM_FOU)
.onchange = timer_year_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

REGISTER_UI_EVENT_HANDLER(MON_NUM_FIR)
.onchange = timer_year_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

REGISTER_UI_EVENT_HANDLER(MON_NUM_SEC)
.onchange = timer_year_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

REGISTER_UI_EVENT_HANDLER(MON_NUM_THI)
.onchange = timer_year_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

REGISTER_UI_EVENT_HANDLER(MON_NUM_FOU)
.onchange = timer_year_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

REGISTER_UI_EVENT_HANDLER(HOUR_NUM_FIR)
.onchange = timer_year_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

REGISTER_UI_EVENT_HANDLER(HOUR_NUM_SEC)
.onchange = timer_year_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

REGISTER_UI_EVENT_HANDLER(HOUR_NUM_THI)
.onchange = timer_year_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

REGISTER_UI_EVENT_HANDLER(HOUR_NUM_FOU)
.onchange = timer_year_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

REGISTER_UI_EVENT_HANDLER(DAY_NUM_FIR)
.onchange = timer_year_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

REGISTER_UI_EVENT_HANDLER(DAY_NUM_SEC)
.onchange = timer_year_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

REGISTER_UI_EVENT_HANDLER(DAY_NUM_THI)
.onchange = timer_year_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

REGISTER_UI_EVENT_HANDLER(DAY_NUM_FOU)
.onchange = timer_year_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

REGISTER_UI_EVENT_HANDLER(MIN_NUM_FIR)
.onchange = timer_year_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

REGISTER_UI_EVENT_HANDLER(MIN_NUM_SEC)
.onchange = timer_year_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

REGISTER_UI_EVENT_HANDLER(MIN_NUM_THI)
.onchange = timer_year_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

REGISTER_UI_EVENT_HANDLER(MIN_NUM_FOU)
.onchange = timer_year_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

REGISTER_UI_EVENT_HANDLER(SEC_NUM_FIR)
.onchange = timer_year_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

REGISTER_UI_EVENT_HANDLER(SEC_NUM_SEC)
.onchange = timer_year_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

REGISTER_UI_EVENT_HANDLER(SEC_NUM_THI)
.onchange = timer_year_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

REGISTER_UI_EVENT_HANDLER(SEC_NUM_FOU)
.onchange = timer_year_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};


static int btn29_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;
    struct sys_time get_time = {0};
    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        switch (pic->elm.id) {
        case TIMER_29:
            ui_hide(TIMER_1);
            ui_show(SET_TIME_MAIN);
            break;
        case TIMER_31:
            ui_hide(TIMER_2);
            ui_show(SET_TIME_MAIN);
            break;
        case TIMER_30:
            get_sys_time(&get_time);
            get_time.year = ui_grid_get_hindex_dynamic(ui_grid_for_id(SET_TIMER_YEAR_VLIST)) % 9999;
            get_time.month = ui_grid_get_hindex_dynamic(ui_grid_for_id(SET_TIMER_MOUTH_VLIST)) % 12;
            get_time.day = ui_grid_get_hindex_dynamic(ui_grid_for_id(SET_TIMER_DAY_VLIST)) % 31;
            rtc_write_time(&get_time);
            ui_hide(TIMER_1);
            ui_show(SET_TIME_MAIN);
            break;
        case TIMER_32:
            get_sys_time(&get_time);
            get_time.hour = ui_grid_get_hindex_dynamic(ui_grid_for_id(SET_TIMER_HOUR_VLIST)) % 24;
            get_time.min = ui_grid_get_hindex_dynamic(ui_grid_for_id(SET_TIMER_MIN_VLIST)) % 60;
            get_time.sec = ui_grid_get_hindex_dynamic(ui_grid_for_id(SET_TIMER_SEC_VLIST)) % 60;
            rtc_write_time(&get_time);
            ui_hide(TIMER_2);
            ui_show(SET_TIME_MAIN);
            break;
        default:
            break;
        }

        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        break;
    case ELM_EVENT_TOUCH_DOWN:
        return true;
    case ELM_EVENT_TOUCH_U_MOVE:
        break;
    case ELM_EVENT_TOUCH_D_MOVE:
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TIMER_29)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = btn29_ontouch,
};

REGISTER_UI_EVENT_HANDLER(TIMER_30)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = btn29_ontouch,
};

REGISTER_UI_EVENT_HANDLER(TIMER_31)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = btn29_ontouch,
};

REGISTER_UI_EVENT_HANDLER(TIMER_32)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = btn29_ontouch,
};



#endif  //SCREEN_UI_TIME_SET_ENABLE
#endif //CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE
