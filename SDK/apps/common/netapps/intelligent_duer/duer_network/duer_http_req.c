#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".intelligent_duer_network.data.bss")
#pragma data_seg(".intelligent_duer_network.data")
#pragma const_seg(".intelligent_duer_network.text.const")
#pragma code_seg(".intelligent_duer_network.text")
#endif
#include "duer_common.h"

#if INTELLIGENT_DUER

#define LOG_TAG_CONST       NET_DUER
#define LOG_TAG             "[DUER_HTTP_REQ]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"



static TokenData *duer_get_access_token_child()
{
    char url_buffer[256];
    snprintf(url_buffer, sizeof(url_buffer),
             "https://openapi.baidu.com/oauth/2.0/token?"
             "grant_type=client_credentials&"
             "client_id=%s&"
             "client_secret=%s",
             DUER_CLIENT_AK, DUER_CLIENT_SK);
    char *response = NULL;
    net_http_get_request(url_buffer, &response);
    if (!response) {
        log_error("API request failed\n");
        return NULL;
    }
    TokenData *token = duer_parse_token_json(response);
    net_interface_free(response);  // 无论成功与否都要释放响应
    if (!token) {
        log_error("JSON parsing failed\n");
    }
    return token;
}

TokenData *duer_get_access_token()
{
    TokenData *token = duer_get_access_token_child();
    if (!token) {
        log_error("Failed to get access token\n");
        return NULL;
    }
    log_debug("refresh_token: %s\n", token->refresh_token);
    log_debug("expires_in: %d\n", token->expires_in);
    log_debug("session_key: %s\n", token->session_key);
    log_debug("access_token: %s\n", token->access_token);
    log_debug("scope: %s\n", token->scope);
    log_debug("session_secret: %s\n", token->session_secret);
    return token;
}



#endif
