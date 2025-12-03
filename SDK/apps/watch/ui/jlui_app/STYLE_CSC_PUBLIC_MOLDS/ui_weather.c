#include "app_config.h"
#include "ui/ui_api.h"
#include "data_storage.h"


#define LOG_TAG_CONST       UI
#define LOG_TAG     		"[UI-WEATHER]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_DUMP_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"

#if (defined(CONFIG_UI_STYLE_JL_CSC_PUBLIC_MODLS_ENABLE))
#if ((defined TCFG_UI_WEATHER_ENABLE) && (TCFG_UI_WEATHER_ENABLE) && (defined WEATHER_DETAILS) && (defined WEATHER_DETAILS))

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ui_action_weather.data.bss")
#pragma data_seg(".ui_action_weather.data")
#pragma const_seg(".ui_action_weather.text.const")
#pragma code_seg(".ui_action_weather.text")
#endif


// /**********************
//  * DEFINES
//  *********************/
// #define abs(x)  ((x)>0?(x):-(x))
// #define __this  (p_weather_info)

// /**********************
//  * STATIC PROTOTYPES
//  *********************/

#define STYLE_NAME  JL

static u16 store_buf[3][10];
static int weather_chile_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct element *elm = (struct element *)ctr;
    int type = ui_id2type(elm->id);
    log_info("%s %d", __FUNCTION__, type);
    u8 index_buf[10] ALIGNED(4);
    int value = 0;
    int index = 0;
    switch (type) {
    case CTRL_TYPE_TEXT:
        //文本方式实现
        struct ui_text *text = (struct ui_text *)elm;
        if (!strcmp(text->source, "temp")) {
            // 温度
            value = 25;
            index_buf[index++] = value / 10;
            index_buf[index++] = value % 10;
            index_buf[index++] = 10;// 符号
        } else if (!strcmp(text->source, "tempmin")) {
            value = 14;
            index_buf[index++] = value / 10;
            index_buf[index++] = value % 10;
            index_buf[index++] = 10;// 符号
        } else if (!strcmp(text->source, "tempmax")) {
            value = 25;
            index_buf[index++] = value / 10;
            index_buf[index++] = value % 10;
            index_buf[index++] = 10;// 符号
        } else if (!strcmp(text->source, "wind")) {
            value = 2;
            index_buf[index++] = value % 10;
            index_buf[index++] = 10;// 符号

        } else if (!strcmp(text->source, "hum")) {
            value = 18;
            index_buf[index++] = value / 10;
            index_buf[index++] = value % 10;
            index_buf[index++] = 10;// 符号

        }
        log_info("func:%s, text->source:%s, value:%d, index:%d\n", __func__, text->source, value, index);
        if (index > 0) {
            ui_text_set_combine_index(text, store_buf[0], index_buf, index);
        }

        break;
    case CTRL_TYPE_NUMBER:
        //数字方式实现
        struct ui_number *numb = (struct ui_number *)elm;
        struct unumber num;
        if (!strcmp(numb->source, "temp")) {
            // 温度
            value = 25;
        } else if (!strcmp(numb->source, "tempmin")) {
            value = 14;
        } else if (!strcmp(numb->source, "tempmax")) {
            value = 25;
        } else if (!strcmp(numb->source, "wind")) {
            value = 2;
        } else if (!strcmp(numb->source, "hum")) {
            value = 18;
        }
        log_info("func:%s, numb->source:%s, value:%d\n", __func__, numb->source, value);
        num.type = TYPE_NUM;
        num.numbs = 1;
        num.number[0] = value;
        ui_number_update(numb, &num);
        break;
    default:
        break;
    }

    return 0;
}

static int ui_weather_layer_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct layout *layout = (struct layout *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_set_default_handler(&layout->elm, NULL, NULL, weather_chile_onchange);
        break;
    case ON_CHANGE_RELEASE:

        break;
    default:
        break;
    }
    return false;
}


REGISTER_UI_EVENT_HANDLER(WEATHER_DETAILS)
.onchange = ui_weather_layer_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

#endif
#endif//CONFIG_UI_STYLE_JL_SCREEN_BOX_PUBLIC_MODLS_ENABLE
