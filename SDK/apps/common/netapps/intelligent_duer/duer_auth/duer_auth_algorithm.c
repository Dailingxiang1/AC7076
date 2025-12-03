#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".intelligent_duer_auth.data.bss")
#pragma data_seg(".intelligent_duer_auth.data")
#pragma const_seg(".intelligent_duer_auth.text.const")
#pragma code_seg(".intelligent_duer_auth.text")
#endif

#include "duer_auth_algorithm.h"
#if INTELLIGENT_DUER

#define LOG_TAG_CONST       NET_DUER
#define LOG_TAG             "[DUER_AUTH]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"



#define CST_OFFSET_SECONDS        (28800)        // 北京时间时差（秒）
#define UUID_BYTE_LENGTH          (16)           // UUID字节数
#define STRIPPED_UUID_LEN         (32)           // 无横杠UUID长度
#define RANDOM_PART_LEN           (8)            // 随机部分长度
#define CHARSET_SIZE              (62)            // 字符集大小
#define UUID_BUFFER_SIZE          (33)           // UUID缓冲区大小
#define REQUEST_ID_BUFFER_SIZE    (60)           // 请求ID缓冲区大小
#define RANDOM_STRING_LENGTH      (6)            // 随机字符串长度

// 生成随机字节函数
static void duer_get_random_bytes(unsigned char *buf, int nbytes)
{
    while (nbytes--) {
        *buf = random32(0);
        ++buf;
    }
}

// 生成UUID字符串函数
static void generate_uuid_string_without_hyphens(char *uuid_buffer)
{
    unsigned char rand_bytes[UUID_BYTE_LENGTH];
    duer_get_random_bytes(rand_bytes, UUID_BYTE_LENGTH);

    snprintf(uuid_buffer, UUID_BUFFER_SIZE,
             "%02x%02x%02x%02x%02x%02x%02x%02x"
             "%02x%02x%02x%02x%02x%02x%02x%02x",
             rand_bytes[0],  rand_bytes[1],  rand_bytes[2],  rand_bytes[3],
             rand_bytes[4],  rand_bytes[5],  rand_bytes[6],  rand_bytes[7],
             rand_bytes[8],  rand_bytes[9],  rand_bytes[10], rand_bytes[11],
             rand_bytes[12], rand_bytes[13], rand_bytes[14], rand_bytes[15]);
}

// 生成对话请求ID函数
void duer_generate_dialog_request_id(char *request_id)
{
    struct sys_time curtime;
    net_get_sys_time(&curtime);

    // 时间参数日志
    log_info("Current Time Parameters:");
    log_info("Year:  %d", curtime.year);
    log_info("Month: %d", curtime.month);
    log_info("Day:   %d", curtime.day);
    log_info("Hour:  %d", curtime.hour);
    log_info("Minute:%d", curtime.min);
    log_info("Second:%d", curtime.sec);

    // 时间戳计算
    long long utc_seconds = timestamp_mytime_2_utc_sec(&curtime) - CST_OFFSET_SECONDS;
    long long milliseconds = utc_seconds * 1000;  // 转换为毫秒
    log_info(">>>info: %s %d %s utc_s %lld utc_ms %lld \n", __FUNCTION__, __LINE__, __FILE__, utc_seconds, milliseconds);
    char stripped_uuid[STRIPPED_UUID_LEN + 1] = {0};
    generate_uuid_string_without_hyphens(stripped_uuid);

    char random_part[RANDOM_PART_LEN + 1] = {0};
    strncpy(random_part, stripped_uuid, RANDOM_PART_LEN);

    snprintf(request_id, REQUEST_ID_BUFFER_SIZE, "%lld_%s", milliseconds, random_part);
}

// 生成随机字符串函数
void duer_generate_random_string(char *output, int length)
{
    const char charset[] = "0123456789"
                           "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                           "abcdefghijklmnopqrstuvwxyz";
    const int charset_size = sizeof(charset) - 1;  // 字符集实际大小

    unsigned char rand_bytes[length];
    duer_get_random_bytes(rand_bytes, length);

    for (int i = 0; i < length; i++) {
        output[i] = charset[rand_bytes[i] % charset_size];
    }
    output[length] = '\0';  // 确保字符串终止符
}
#endif
