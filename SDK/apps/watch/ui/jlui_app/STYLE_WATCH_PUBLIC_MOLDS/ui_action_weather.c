#include "app_config.h"
#include "ui_api.h"
#include "ui.h"
#include "ui_style.h"
#include "app_task.h"
#include "system/timer.h"
#include "app_main.h"
#include "init.h"
#include "key_event_deal.h"
#include "data_storage.h"
#include "res/font_ascii.h"


#define LOG_TAG_CONST       UI
#define LOG_TAG     		"[UI-WEATHER]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"


#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ui_action_weather.data.bss")
#pragma data_seg(".ui_action_weather.data")
#pragma const_seg(".ui_action_weather.text.const")
#pragma code_seg(".ui_action_weather.text")
#endif


/**********************
 * DEFINES
 *********************/
#define abs(x)  ((x)>0?(x):-(x))
#define __this  (p_weather_info)

/**********************
 * STATIC PROTOTYPES
 *********************/
static void update_ui_watcher_pic(struct ui_pic *pic);
static void update_ui_watcher_text(struct ui_text *text);
static void update_ui_watcher_temp_number(struct ui_number *number);
static void update_ui_watcher_address_text(struct ui_text *text);
static void update_ui_watcher_update_time(struct ui_time *time);
static void update_ui_watcher_humidity_number(struct ui_number *number);
static void update_ui_watcher_wind_dir_text(struct ui_text *text);
static void update_ui_watcher_wind_power_number(struct ui_number *number);
static int update_weather_info_handler(const char *type, u32 arg);
static void weather_info_init(u8 *data, u32 len);
static void weather_info_deinit(void);

/**********************
 * STATIC VARIABLES
 *********************/
static struct __WEATHER_INFO *p_weather_info;
static char *address_text_buf; /*内容: 省份+市级城市 */




/************************************************
 *  weather_info 接受app传过来的数据
 ***********************************************/

void data_func_attr_weather_set(void *priv, u8 attr, u8 *data, u16 len, u16 ble_con_handle, u8 *spp_remote_addr)
{
#if TCFG_DATA_STORAGE_ENABLE
    u32 create_id, wlen;
    wlen = small_file_write(F_TYPE_WEATHER, &create_id, 0, data, len, len);
    if (wlen != len) {
        log_info("<%s> small file write err!!!", __func__);
        return;
    }
#endif
    UI_MSG_POST("weather_info");
}


#if (defined (CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE))
#if TCFG_UI_ENABLE_WEATHER

#define STYLE_NAME  JL


/************************************************
 *  天气 相关初始化
 ***********************************************/

static const struct uimsg_handl ui_weather_handler[] = {
    { "weather_info",       update_weather_info_handler     },
    { NULL, NULL},      /* 必须以此结尾！ */
};

static int ui_weather_window_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:
        __this = (struct __WEATHER_INFO *)zalloc(sizeof(struct __WEATHER_INFO));
        if (!__this) {
            log_error("[%s] line:%d zalloc fail", __func__, __LINE__);
            break;
        }
        ui_register_msg_handler(ID_WINDOW_WEATHER, ui_weather_handler);

        int file_count = ui_small_file_weather_get_count();
        /*目前默认只有一个天气小文件*/
        if (file_count != 1) {
            break;
        }
        int file_size;
        u8 *weather_file_storage_buf;
        file_size = ui_small_file_weather_get_size_by_index(0);
        weather_file_storage_buf = zalloc(file_size);
        if (!weather_file_storage_buf) {
            break;
        }

        ui_small_file_weather_read_by_index(weather_file_storage_buf, file_size, 0);

        weather_info_init(weather_file_storage_buf, file_size);
        free(weather_file_storage_buf);

        break;
    case ON_CHANGE_RELEASE:
        ui_register_msg_handler(ID_WINDOW_WEATHER, NULL);
        weather_info_deinit();
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

REGISTER_UI_EVENT_HANDLER(ID_WINDOW_WEATHER)
.onchange = ui_weather_window_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int ui_weather_app_not_conneted_layout_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct layout *layout = (struct layout *)_ctrl;

    switch (event) {
    case ON_CHANGE_INIT_PROBE:
        if (__this->update_time) {
            layout->elm.css.invisible = 1;
        } else {
            layout->elm.css.invisible = 0;
        }
        break;
    default:
        break;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(WEATHER_APP_NOT_CONNETED_LAYOUT)
.onchange = ui_weather_app_not_conneted_layout_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int ui_weather_layout_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct layout *layout = (struct layout *)_ctrl;
    switch (event) {
    case ON_CHANGE_INIT_PROBE:
        if (__this->update_time) {
            layout->elm.css.invisible =  0;
        } else {
            layout->elm.css.invisible = 1;
        }
        break;
    default:
        break;
    }

    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(WEATHER_LAYOUT)
.onchange = ui_weather_layout_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int ui_weather_show_grid_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)_ctrl;
    switch (event) {
    case ON_CHANGE_INIT:
        ui_grid_energy_auto_center(grid, AUTO_CENTER_MODE2);
        break;
    default:
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(WEATHER_SHOW_GRID)
.onchange = ui_weather_show_grid_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};



/************************************************
 *  天气 相关数据显示控件
 ***********************************************/

static int ui_weather_pic_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_pic *pic = (struct ui_pic *)_ctrl;
    switch (event) {
    case ON_CHANGE_INIT:
        update_ui_watcher_pic(pic);
        break;
    default:
        break;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(WEATHER_PIC)
.onchange = ui_weather_pic_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int ui_weather_text_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_text *text = (struct ui_text *)_ctrl;
    switch (event) {
    case ON_CHANGE_INIT:
        update_ui_watcher_text(text);
        break;
    default:
        break;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(WEATHER_TEXT)
.onchange = ui_weather_text_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int ui_weather_temp_number_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_number *number = (struct ui_number *)_ctrl;
    switch (event) {
    case ON_CHANGE_INIT:
        update_ui_watcher_temp_number(number);
        break;
    default:
        break;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(WEATHER_TEMP_NUMBER)
.onchange = ui_weather_temp_number_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int ui_weather_address_text_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_text *text = (struct ui_text *)_ctrl;
    switch (event) {
    case ON_CHANGE_INIT:
        update_ui_watcher_address_text(text);
        break;
    default:
        break;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(WEATHER_ADDRESS_TEXT)
.onchange = ui_weather_address_text_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int ui_weather_update_time_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_time *time = (struct ui_time *)_ctrl;
    switch (event) {
    case ON_CHANGE_INIT:
        update_ui_watcher_update_time(time);
        break;
    default:
        break;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(WEATHER_UPDATE_TIME)
.onchange = ui_weather_update_time_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int ui_weather_humidity_number_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_number *number = (struct ui_number *)_ctrl;
    switch (event) {
    case ON_CHANGE_INIT:
        update_ui_watcher_humidity_number(number);
        break;
    default:
        break;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(WEATHER_HUMIDITY_NUMBER)
.onchange = ui_weather_humidity_number_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int ui_weather_wind_dir_text_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_text *text = (struct ui_text *)_ctrl;
    switch (event) {
    case ON_CHANGE_INIT:
        update_ui_watcher_wind_dir_text(text);
        break;
    default:
        break;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(WEATHER_WIND_DIR_TEXT)
.onchange = ui_weather_wind_dir_text_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int ui_weather_wind_power_number_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_number *number = (struct ui_number *)_ctrl;
    switch (event) {
    case ON_CHANGE_INIT:
        update_ui_watcher_wind_power_number(number);
        break;
    default:
        break;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(WEATHER_WIND_POWER_NUMBER)
.onchange = ui_weather_wind_power_number_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};



/************************************************
 *  天气 相关数据显示刷新函数
 ***********************************************/

static void update_ui_watcher_pic(struct ui_pic *pic)
{
    if (!pic) {
        return;
    }
    log_debug("<%s> __this->weather:%d", __func__, __this->weather);
    ui_pic_set_image_index(pic, __this->weather);
}

static void update_ui_watcher_text(struct ui_text *text)
{
    if (!text) {
        return;
    }
    log_debug("<%s> __this->weather:%d", __func__, __this->weather);
    ui_text_set_index(text, __this->weather);
}

static void update_ui_watcher_temp_number(struct ui_number *number)
{
    if (!number) {
        return;
    }

    /*比如显示"23°"*/
    static char temp_number_str[4] = {0};

    /*'' 空格图片 来显示 '°'图片 */
    snprintf(temp_number_str, ARRAY_SIZE(temp_number_str), "%d ", __this->temperature);

    log_debug("<%s> __this->temperature:%d str:%s", __func__, __this->temperature, temp_number_str);

    struct unumber unum;
    unum.type = TYPE_STRING;
    unum.num_str = (u8 *)temp_number_str;
    ui_number_update(number, &unum);
}

static void update_ui_watcher_address_text(struct ui_text *text)
{
    if (!text) {
        return;
    }

    if (!address_text_buf) {
        return;
    }

    log_debug("<%s> address_text_buf:%s", __func__, address_text_buf);

    ui_text_set_text_attrs(text, (const char *)address_text_buf, strlen((const char *)address_text_buf), FONT_ENCODE_UTF8, FONT_ENDIAN_SMALL, FONT_DEFAULT | FONT_SHOW_SCROLL);
}

static void update_ui_watcher_update_time(struct ui_time *time)
{
    if (!time) {
        return;
    }

    u32 timestamp;
    struct utime time_r;

    /*比如显示"2024-05-30"*/
    timestamp = __this->update_time;
    time_r.year = ((timestamp >> 26) & 0x3f) + 2010;
    time_r.month = (timestamp >> 22) & 0xf;
    time_r.day = (timestamp >> 17) & 0x1f;

    log_debug("<%s> update_time %d-%d-%d", __func__, time_r.year, time_r.month, time_r.day);
    ui_time_update(time, &time_r);
}

static void update_ui_watcher_humidity_number(struct ui_number *number)
{
    if (!number) {
        return;
    }

    /*比如显示"45%"*/
    static char humidity_number_str[4] = {0};

    /*'' 空格图片 来显示 '%'图片 */
    snprintf(humidity_number_str, ARRAY_SIZE(humidity_number_str), "%d ", __this->humidity);

    log_debug("<%s> __this->humidity:%d str:%s", __func__, __this->humidity, humidity_number_str);

    struct unumber unum;
    unum.type = TYPE_STRING;
    unum.num_str = (u8 *)humidity_number_str;
    ui_number_update(number, &unum);
}

static void update_ui_watcher_wind_dir_text(struct ui_text *text)
{
    if (!text) {
        return;
    }
    log_debug("<%s> __this->wind_direction:%d", __func__, __this->wind_direction);
    ui_text_set_index(text, __this->wind_direction);
}

static void update_ui_watcher_wind_power_number(struct ui_number *number)
{
    if (!number) {
        return;
    }

    log_debug("<%s> __this->wind_power:%d", __func__, __this->wind_power);
    struct unumber unum;
    unum.type = TYPE_NUM;
    unum.numbs = 1;
    unum.number[0] = __this->wind_power;
    ui_number_update(number, &unum);
}

static int update_weather_info_handler(const char *type, u32 arg)
{
    struct element_css *css1, *css2;
    log_info("<%s>", __func__);
    css1 = ui_core_get_element_css(ui_core_get_element_by_id(WEATHER_LAYOUT));
    css2 = ui_core_get_element_css(ui_core_get_element_by_id(WEATHER_APP_NOT_CONNETED_LAYOUT));

    if (__this->update_time) {
        css1->invisible = false;
        css2->invisible = true;
    } else {
        css1->invisible = true;
        css2->invisible = false;
        goto __end;
    }

    update_ui_watcher_pic((struct ui_pic *)ui_core_get_element_by_id(WEATHER_PIC));
    update_ui_watcher_text((struct ui_text *)ui_core_get_element_by_id(WEATHER_TEXT));
    update_ui_watcher_temp_number((struct ui_number *)ui_core_get_element_by_id(WEATHER_TEMP_NUMBER));
    update_ui_watcher_address_text((struct ui_text *)ui_core_get_element_by_id(WEATHER_ADDRESS_TEXT));
    update_ui_watcher_update_time((struct ui_time *)ui_core_get_element_by_id(WEATHER_UPDATE_TIME));
    update_ui_watcher_humidity_number((struct ui_number *)ui_core_get_element_by_id(WEATHER_HUMIDITY_NUMBER));
    update_ui_watcher_wind_dir_text((struct ui_text *)ui_core_get_element_by_id(WEATHER_WIND_DIR_TEXT));
    update_ui_watcher_wind_power_number((struct ui_number *)ui_core_get_element_by_id(WEATHER_WIND_POWER_NUMBER));

__end:
    ui_core_redraw(ui_core_get_element_by_id(ID_WINDOW_WEATHER));
    return 0;
}

static void weather_info_init(u8 *data, u32 len)
{
    u8 offset = 0;
    if (!__this) {
        log_error("<%s> __this is null", __func__, __LINE__);
        return;
    }

    log_info("->%s\n", __FUNCTION__);
    log_info("=============");
    put_buf(data, len);
    log_info("=============");

    __this->province_name_len = data[offset];
    offset++;

    __this->province = zalloc(__this->province_name_len + 1);
    if (!__this->province) {
        log_error("<%s> line:%d, zalloc fail", __func__, __LINE__);
        return;
    }
    memcpy(__this->province, data + offset, __this->province_name_len);
    offset += __this->province_name_len;
    log_debug("__this->province:%s", __this->province);

    __this->city_name_len = data[offset];
    offset++;

    __this->city = zalloc(__this->city_name_len + 1);
    if (!__this->city) {
        log_error("<%s> line:%d, zalloc fail", __func__, __LINE__);
        return;
    }
    memcpy(__this->city, data + offset, __this->city_name_len);
    offset += __this->city_name_len;
    log_debug("__this->city:%s", __this->city);

    __this->weather = data[offset];
    offset++;
    log_debug("__this->weather:%d", __this->weather);

    __this->temperature = data[offset];
    offset++;
    log_debug("__this->temperature:%d", __this->temperature);

    __this->humidity = data[offset];
    offset++;
    log_debug("__this->humidity:%d", __this->humidity);

    __this->wind_direction = data[offset];
    offset++;
    log_debug("__this->wind_direction:%d", __this->wind_direction);

    __this->wind_power = data[offset];
    offset++;
    log_debug("__this->wind_power:%d", __this->wind_power);

    __this->update_time = (*(data + offset) << 24) | (*(data + offset + 1) << 16) | (*(data + offset + 2) << 8) | (*(data + offset + 3) << 0);
    log_debug("__this->update_time:%x", __this->update_time);

    address_text_buf = zalloc(__this->province_name_len + __this->city_name_len + 1);
    if (!address_text_buf) {
        log_error("<%s> line:%d, zalloc fail", __func__, __LINE__);
        return;
    }
    memcpy(address_text_buf, __this->province, __this->province_name_len);
    memcpy(address_text_buf + __this->province_name_len, __this->city, __this->city_name_len);
}

static void weather_info_deinit(void)
{
    if (!__this) {
        log_error("<%s> __this is null", __func__, __LINE__);
        return;
    }

    if (__this->province) {
        free(__this->province);
        __this->province = NULL;
    }

    if (__this->city) {
        free(__this->city);
        __this->city = NULL;
    }

    if (address_text_buf) {
        free(address_text_buf);
        address_text_buf = NULL;
    }
}


#endif /* if TCFG_UI_ENABLE_WEATHER */
#endif /* #if (defined (CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE)) */

