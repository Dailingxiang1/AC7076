#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".intelligent_duer_task.data.bss")
#pragma data_seg(".intelligent_duer_task.data")
#pragma const_seg(".intelligent_duer_task.text.const")
#pragma code_seg(".intelligent_duer_task.text")
#endif
#include "duer_task.h"
#include "duer_http_req.h"
#include "os_api.h"

#if INTELLIGENT_DUER
#define LOG_TAG_CONST       NET_DUER
#define LOG_TAG             "[DUER_TASK]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"


#define INTELLIGENT_DUER_NAME		"intelligent_duer"

static DUER_DATA *duer = NULL;

static TokenData *token = NULL;

void ntp_sync_callback(int result, const struct tm *sync_time, const char *msg)
{
    if (result == 0) {
        char time_str[64];
        strftime_2(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", sync_time);
        log_info("NTP同步成功: %s | %s\n", time_str, msg);
        os_taskq_post(INTELLIGENT_DUER_NAME, 1, DUER_TOKEN_GET); //同步时间成功，下一步鉴权
    } else {
        log_info("NTP同步失败 (错误码: %d): %s\n", result, msg);
        os_taskq_post(INTELLIGENT_DUER_NAME, 1, DUER_ERROR);
    }
}

static void duer_data_init()
{
    if (duer == NULL) {
        duer = net_interface_malloc(sizeof(DUER_DATA));
    }
}

static void duer_data_destory()
{
    if (duer->access_token) {
        net_interface_free(duer->access_token);
    }
    if (duer != NULL) {
        net_interface_free(duer);
    }
}

static void duer_ask_access_token()
{
    token = duer_get_access_token();
    if (token) {
        size_t len = strlen(token->access_token);
        duer->access_token = net_interface_malloc(len + 1);
        if (duer->access_token == NULL) {
            os_taskq_post(INTELLIGENT_DUER_NAME, 1, DUER_ERROR);
        }
        snprintf(duer->access_token, len + 1, "%s", token->access_token);
        log_info(">>>>>>>>>>duer->access_token %s \n", duer->access_token);
        duer_free_token_data(token);
        os_taskq_post(INTELLIGENT_DUER_NAME, 1, DUER_WS_CONNECT);
    } else {
        os_taskq_post(INTELLIGENT_DUER_NAME, 1, DUER_ERROR);
    }
}

static void task_kill_callback()
{
    task_kill(INTELLIGENT_DUER_NAME);
}

static void intelligent_duer_task(void *priv)
{
    int msg[3] = {0};
    int res = 0;
    while (1) {
        res = os_taskq_pend(NULL, msg, ARRAY_SIZE(msg));
        if (res == OS_TASKQ) {
            switch (msg[1]) {
            case DUER_NTP_SYNC:
                ntp_get_time_to_sync_rtc_with_callback();
                break;
            case DUER_TOKEN_GET:
                duer_ask_access_token();
                break;
            case DUER_WS_CONNECT://ws通信、opus数据交互、url数据接收
                duer_websocket_client_thread_create(duer->access_token);
                break;
            case DUER_NETDOWN://URL索引下载，解码播放
                net_url_download();
                break;
            case DUER_ERROR:
            case DUER_CLOSING:
                goto err;		//error和close都会关闭
                break;
            }
        }
    }
err:
    duer_data_destory();
    msg[0] = (int)task_kill_callback;
    msg[1] = 1;
    os_taskq_post_type("app_core", Q_CALLBACK, 2, msg);
    os_time_dly(-1);
}

void intelligent_duer_task_create()
{
    duer_data_init();
    os_task_create(intelligent_duer_task,
                   NULL,
                   30,
                   512 * 2,
                   16,
                   INTELLIGENT_DUER_NAME);
}

void duer_sync_time_msg()
{
    os_taskq_post(INTELLIGENT_DUER_NAME, 1, DUER_NTP_SYNC);
}

void duer_netdownload_msg()
{
    os_taskq_post(INTELLIGENT_DUER_NAME, 1, DUER_NETDOWN);
}

void duer_netdownload_close_msg()
{
    os_taskq_post(INTELLIGENT_DUER_NAME, 1, DUER_CLOSING);
}
#endif
