#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".net_interface.data.bss")
#pragma data_seg(".net_interface.data")
#pragma const_seg(".net_interface.text.const")
#pragma code_seg(".net_interface.text")
#endif
#include "net_http.h"

#if NET_INTERFACE_EN
#define LOG_TAG_CONST       NET_INTERFACE
#define LOG_TAG             "[NET_INTERFACE_HTTP]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"


int net_http_get_request(char *url, char **response)
{
    int error = 0;
    http_body_obj http_body_buf;
    log_info("wt get url: %s\n", url);
    httpcli_ctx *ctx = (httpcli_ctx *)net_interface_calloc(1, sizeof(httpcli_ctx));
    if (!ctx) {
        log_error("calloc failed\n");
        return -1;
    }
    memset(&http_body_buf, 0x0, sizeof(http_body_obj));
    http_body_buf.recv_len = 0;
    http_body_buf.buf_len = 2 * 1024;
    http_body_buf.buf_count = 1;
    http_body_buf.p = (char *)net_interface_malloc(http_body_buf.buf_len * http_body_buf.buf_count);
    if (!http_body_buf.p) {
        net_interface_free(ctx);
        ctx = NULL;
        return -1;
    }
    ctx->url = url;
    ctx->timeout_millsec = 5000;
    ctx->priv = &http_body_buf;
    ctx->connection = "close";
    ctx->data_format = "application/json";
    error = httpcli_get(ctx);
    if (error != HERROR_OK) {
        log_error("get failed\n");
    } else {
        if (http_body_buf.recv_len > 0) {
            log_info("\nreceive %d bytes from(%s)\n", http_body_buf.recv_len, url);
            log_info("%s\n", http_body_buf.p);
            if (response != NULL) {
                *response = net_interface_malloc(http_body_buf.recv_len + 1);
                if (*response == NULL) {
                    log_error("Memory allocation failed for response\n");
                    error = -1;
                } else {
                    memcpy(*response, http_body_buf.p, http_body_buf.recv_len);
                    (*response)[http_body_buf.recv_len] = '\0';
                    log_info("Response copied to external pointer, length: %d\n", http_body_buf.recv_len);
                }
            }
        }
    }
    httpcli_close(ctx);
    if (http_body_buf.p) {
        net_interface_free(http_body_buf.p);
    }
    if (ctx) {
        net_interface_free(ctx);
    }
    return error;
}

int net_http_post_request(char *url, char **response)
{
    int ret = 0;
    http_body_obj http_body_buf;
    httpcli_ctx *ctx = (httpcli_ctx *)net_interface_calloc(1, sizeof(httpcli_ctx));
    if (!ctx) {
        log_error("calloc failed\n");
        return -1;
    }
    memset(&http_body_buf, 0x0, sizeof(http_body_obj));
    http_body_buf.recv_len = 0;
    http_body_buf.buf_len = 2 * 1024;
    http_body_buf.buf_count = 1;
    http_body_buf.p = (char *)net_interface_malloc(http_body_buf.buf_len * http_body_buf.buf_count);
    if (!http_body_buf.p) {
        net_interface_free(ctx);
        return -1;
    }
    ctx->url = url;
    ctx->timeout_millsec = 5000;
    ctx->priv = &http_body_buf;
    ctx->connection = "close";
    ctx->data_format = "application/json";
    ret = httpcli_post(ctx);
    if (ret != HERROR_OK) {
        log_error("HTTP POST request failed\n");
    } else {
        if (http_body_buf.recv_len > 0) {
            log_info("\nReceived %d Bytes from (%s)\n", http_body_buf.recv_len, url);
            if (response != NULL) {
                *response = net_interface_malloc(http_body_buf.recv_len + 1);
                if (*response == NULL) {
                    log_error("Memory allocation failed for response\n");
                    ret = -1;
                } else {
                    memcpy(*response, http_body_buf.p, http_body_buf.recv_len);
                    (*response)[http_body_buf.recv_len] = '\0';
                    log_info("Response copied to external pointer, length: %d\n", http_body_buf.recv_len);
                }
            }
        }
    }
    httpcli_close(ctx);
    if (http_body_buf.p) {
        net_interface_free(http_body_buf.p);
    }
    if (ctx) {
        net_interface_free(ctx);
    }
    return ret;
}
#endif //NET_INTERFACE_EN
