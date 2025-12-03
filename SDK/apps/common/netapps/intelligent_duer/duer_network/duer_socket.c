#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".intelligent_duer_network.data.bss")
#pragma data_seg(".intelligent_duer_network.data")
#pragma const_seg(".intelligent_duer_network.text.const")
#pragma code_seg(".intelligent_duer_network.text")
#endif

#include "duer_socket.h"
#if INTELLIGENT_DUER

#define LOG_TAG_CONST       NET_DUER
#define LOG_TAG             "[DUER_SOCKET]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"


#define OBJ_URL 	"ws://duer-kids.baidu.com/sandbox/sota/realtime_asr?sn=%s"
#define CUID_LEN 6
#define MAX_URL_LEN 256
#define MAX_TOKEN_LEN 256

static void task_kill_callback(char *buf);
static u8 force_kill = 0; //结束标志位

static void websockets_callback(u8 *buf, u32 len, u8 type)
{
    log_info("wbs recv msg : %s\n", buf);
    InsideRCResponse *response = duer_parse_inside_rc_json((char *)buf);
    if (response &&
        response->data &&
        response->data->is_end == 1) {
        force_kill = 1;
        duer_free_inside_rc_response(response);
        duer_netdownload_msg();
    }
}
/*******************************************************************************
*   Websocket Client api
*******************************************************************************/
static void websockets_client_reg(struct websocket_struct *websockets_info, char mode)
{
    memset(websockets_info, 0, sizeof(struct websocket_struct));
    websockets_info->_init           = websockets_client_socket_init;
    websockets_info->_exit           = websockets_client_socket_exit;
    websockets_info->_handshack      = webcockets_client_socket_handshack;
    websockets_info->_send           = websockets_client_socket_send;
    websockets_info->_recv_thread    = websockets_client_socket_recv_thread;
    websockets_info->_heart_thread   = websockets_client_socket_heart_thread;
    websockets_info->_recv_cb        = websockets_callback;
    websockets_info->_recv           = NULL;
    websockets_info->websocket_mode  = mode;
}

static int websockets_client_init(struct websocket_struct *websockets_info, u8 *url, const char *origin_str, const char *user_agent_str)
{
    websockets_info->ip_or_url = url;
    websockets_info->origin_str = origin_str;
    websockets_info->user_agent_str = user_agent_str;
    websockets_info->recv_time_out = 1000;

    //应用层和库的版本检测，结构体不一样则返回出错
    int err = websockets_struct_check(sizeof(struct websocket_struct));
    if (err == FALSE) {
        return err;
    }
    return websockets_info->_init(websockets_info);
}

static int websockets_client_handshack(struct websocket_struct *websockets_info)
{
    log_info("myurl %s \n", websockets_info->ip_or_url);
    return websockets_info->_handshack(websockets_info);
}

static int websockets_client_send(struct websocket_struct *websockets_info, u8 *buf, int len, char type)
{
    //SSL加密时一次发送数据不能超过16K，用户需要自己分包
    return websockets_info->_send(websockets_info, buf, len, type);
}

static void websockets_client_exit(struct websocket_struct *websockets_info)
{
    websockets_info->_exit(websockets_info);
}


/*******************************************************************************
*   Websocket Client.c
*   Just one example for test
*******************************************************************************/
static void websockets_client_main_thread(void *priv)
{

    int err = 0;
    char mode = WEBSOCKET_MODE;
    char access_token[MAX_TOKEN_LEN] = {0};
    char cuid_str[CUID_LEN + 1] = {0};
    char url[MAX_URL_LEN] = {0};
    duer_generate_random_string(cuid_str, sizeof(cuid_str) - 1);
    log_debug("Generated CUID: %s", cuid_str);

    if (snprintf(url, sizeof(url), OBJ_URL, cuid_str) >= sizeof(url)) {
        log_error("URL buffer overflow");
        return;
    }

    if (snprintf(access_token, sizeof(access_token), "%s", (char *)priv) >= sizeof(access_token)) {
        log_error("Token buffer overflow");
        return;
    }
    log_debug("WebSocket URL: %s, Access Token: %s", url, access_token);

    const char *origin_str = "http://coolaf.com";
    /* 0 . malloc buffer */
    struct websocket_struct *websockets_info = net_interface_malloc(sizeof(struct websocket_struct));
    if (!websockets_info) {
        return;
    }
    /* 1 . register */
    websockets_client_reg(websockets_info, mode);

    /* 2 . init */
    err = websockets_client_init(websockets_info, (u8 *)url, origin_str, NULL);
    if (FALSE == err) {
        log_error("  . ! Cilent websocket init error !!!\r\n");
        goto exit_ws;
    }

    /* 3 . hanshack */
    err = websockets_client_handshack(websockets_info);
    if (FALSE == err) {
        log_error("  . ! Handshake error !!!\r\n");
        goto exit_ws;
    }
    log_debug(" . Handshake success \r\n");

    /* 4 . CreateThread */
    err = os_task_create(websockets_info->_heart_thread,
                         websockets_info,
                         19,
                         512,
                         0,
                         "websocket_client_heart");
    if (err == 0) {
        websockets_info->ping_thread_id = 1;
    } else {

        websockets_info->ping_thread_id = 0;
    }
    err = os_task_create(websockets_info->_recv_thread,
                         websockets_info,
                         18,
                         512,
                         0,
                         "websocket_client_recv");
    if (err == 0) {
        websockets_info->recv_thread_id = 1;
    } else {

        websockets_info->recv_thread_id = 0;
    }
    os_time_dly(100);

    //server_log_id
    char dialog_id[60];
    duer_generate_dialog_request_id(dialog_id);// 生成对话请求ID
    log_debug("Generated Dialog Request ID: %s\n", dialog_id);

    //start_frame
    char *start_str = duer_start_frame(DUER_CLIENT_AK, DUER_CLIENT_SK, DUER_CLIENT_AK, DUER_APP_PID,
                                       cuid_str, DUER_APP_FORMAT, 1, DUER_APP_SAMPLE,
                                       cuid_str, access_token, 1,
                                       1, dialog_id, 1,
                                       "1");
    log_debug(">>>start_str %s \n", start_str);
    err = websockets_client_send(websockets_info, (u8 *)start_str, strlen(start_str), WCT_TXTDATA);
    if (FALSE == err) {
        log_debug("  . ! send err !!!\r\n");
        goto exit_ws;
    }
    os_time_dly(40);
    /* char *finish = build_finish_frame(); */
    net_rec_start();
    u8 send_buf[40];
    u32 buf_size = sizeof(send_buf);
    while (1) {
        int available = net_rec_data_len();
        if (available == 0) {
            os_time_dly(1);
            continue;
        }
        int bytes_read = net_rec_read_data(send_buf, buf_size);
        if (bytes_read == 0) {
            os_time_dly(1);
            continue;
        }
        printf("Read %d bytes from cbuf\n", bytes_read);
        err = websockets_client_send(websockets_info, send_buf, bytes_read, WCT_BINDATA);
        if (false == err) {
            log_error("  . ! send err !!!\r\n");
            goto exit_ws;
        }
        if (force_kill) {
            goto exit_ws;
        }
        os_time_dly(10);
    }
exit_ws:
    /* 6 . exit */
    net_record_stop_with_clean();
    if (websockets_info->ping_thread_id) {
        websockets_info->ping_kill_flag = 1;
    }
    if (websockets_info->recv_thread_id) {
        websockets_info->recv_kill_flag = 1;
    }
    while (websockets_info->recv_kill_flag || websockets_info->ping_kill_flag) {
        os_time_dly(10);
    }
    websockets_client_exit(websockets_info);
    net_interface_free(websockets_info);
    net_interface_free(start_str);
    /* my_free(finish); */
    int msg[5];
    msg[0] = (int)task_kill_callback;
    msg[1] = 1;
    msg[2] = (int)os_current_task();
    err = os_taskq_post_type("app_core", Q_CALLBACK, 3, msg);
    os_time_dly(-1);
}


static void task_kill_callback(char *buf)
{
    log_info("[msg]>>>>>>>>>>>*buf=%s", buf);
    task_kill(buf);
}

void duer_websocket_client_thread_create(void *priv)
{
    net_url_list_init();
    force_kill = 0;
    os_task_create(websockets_client_main_thread,
                   priv,
                   15,
                   512 * 5,
                   0,
                   "websockets_client_main");
}

#endif
