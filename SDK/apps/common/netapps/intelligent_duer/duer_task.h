#ifndef _DUER_TASK
#define _DUER_TASK

#include  "duer_common.h"
#include  "time.h"

// 状态枚举
typedef enum {
    DUER_NTP_SYNC,
    DUER_TOKEN_GET,
    DUER_WS_CONNECT,
    DUER_NETDOWN,
    DUER_ERROR,
    DUER_CLOSING
} SystemState;

// 错误码枚举
typedef enum {
    ERROR_NONE = 0,
    ERROR_NTP_FAILED,
    ERROR_HTTP_FAILED,
    ERROR_TOKEN_INVALID,
    ERROR_WS_CONNECT_FAILED,
    ERROR_SERVER_REJECT,
    ERROR_RECORD_FAILED,
    ERROR_UPLOAD_FAILED,
    ERROR_TIMEOUT
} ErrorCode;

typedef struct {
    char *access_token;
    u8 run;
} DUER_DATA;

extern void ntp_get_time_to_sync_rtc_with_callback();

extern int task_kill(const char *name);

extern void intelligent_duer_task_create();

extern void duer_sync_time_msg();

extern void duer_netdownload_msg();

extern size_t strftime_2(char *ptr, size_t maxsize, const char *format, const struct tm *timeptr);

#endif
