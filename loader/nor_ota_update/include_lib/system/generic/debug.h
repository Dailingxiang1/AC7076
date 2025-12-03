#ifndef _DEBUG_H_
#define _DEBUG_H_

#include "printf.h"
#include "asm/cpu.h"
#include "generic/typedef.h"

#define RedBold             "\033[31;1m" // 红色加粗
#define RedBoldBlink        "\033[31;1;5m" // 红色加粗、闪烁
#define GreenBold             "\033[32;1m" // 红色加粗
#define GreenBoldBlink        "\033[32;1;5m" // 红色加粗、闪烁
#define YellowBold             "\033[33;1m" // 红色加粗
#define YellowBoldBlink        "\033[33;1;5m" // 红色加粗、闪烁
#define BlueBold            "\033[34;1m" // 蓝色加粗
#define BlueBoldBlink       "\033[34;1;5m" // 蓝色加粗、闪烁
#define PurpleBold          "\033[35;1m"   // 紫色加粗
#define PurpleBoldBlink     "\033[35;1;5m" // 紫色加粗、闪烁
#define DGreenBold             "\033[36;1m" // 红色加粗
#define DGreenBoldBlink        "\033[36;1;5m" // 红色加粗、闪烁
#define WhiteBold             "\033[37;1m" // 红色加粗
#define WhiteBoldBlink        "\033[37;1;5m" // 红色加粗、闪烁
#define Reset               "\033[0;25m"   // 颜色复位

#define LOG_VERBOSE     v
#define LOG_INFO        i
#define LOG_DEBUG       d
#define LOG_WARN        w
#define LOG_ERROR       e
#define LOG_CHAR        c

#define _STR(x)     #x
#define STR(x)     "["_STR(x)"]"


#define _LOG_TAG_CONST_DECLARE(level, name)         extern const char log_tag_const_##level##_##name
#define LOG_TAG_CONST_DECLARE(level, name)          _LOG_TAG_CONST_DECLARE(level, name)

#define ___LOG_IS_ENABLE(level, name)   (log_tag_const_##level##_##name)
#define __LOG_IS_ENABLE(level, name)    ___LOG_IS_ENABLE(level, name)
#define _LOG_IS_ENABLE(level)           __LOG_IS_ENABLE(level, LOG_TAG_CONST)

#ifdef LOG_TAG_CONST
LOG_TAG_CONST_DECLARE(LOG_VERBOSE,  LOG_TAG_CONST);
LOG_TAG_CONST_DECLARE(LOG_INFO,     LOG_TAG_CONST);
LOG_TAG_CONST_DECLARE(LOG_DEBUG,    LOG_TAG_CONST);
LOG_TAG_CONST_DECLARE(LOG_WARN,     LOG_TAG_CONST);
LOG_TAG_CONST_DECLARE(LOG_ERROR,    LOG_TAG_CONST);
LOG_TAG_CONST_DECLARE(LOG_CHAR,     LOG_TAG_CONST);

#define _LOG_TAG            LOG_TAG
#define LOG_IS_ENABLE(level)            _LOG_IS_ENABLE(level)

#else
#define _LOG_TAG            "[NULL]"
#define LOG_IS_ENABLE(x)    0
#endif

#ifdef __DEBUG

#define log_print printf

#define log_info(format, ...)       \
    if (LOG_IS_ENABLE(LOG_INFO)) \
        log_print("[Info] " _LOG_TAG format "\r\n", ## __VA_ARGS__)

#define log_info_hexdump(x, y)     \
    if (LOG_IS_ENABLE(LOG_INFO)) \
        printf_buf(x, y)


#define log_debug(format, ...)       \
    if (LOG_IS_ENABLE(LOG_DEBUG)) \
        log_print("[Debug] " _LOG_TAG format "\r\n", ## __VA_ARGS__)

#define log_debug_hexdump(x, y)     \
    if (LOG_IS_ENABLE(LOG_DEBUG)) \
        printf_buf(x, y)

#define log_error(format, ...)       \
    if (LOG_IS_ENABLE(LOG_ERROR)) \
        log_print("<Error> " _LOG_TAG format "\r\n", ## __VA_ARGS__)

#define log_error_hexdump(x, y)     \
    if (LOG_IS_ENABLE(LOG_ERROR)) \
        printf_buf(x, y)

#define log_char(x)       \
    if (LOG_IS_ENABLE(LOG_CHAR)) \
		putchar(x)

#else

#define log_info(format, ...)
#define log_info_hexdump(x, y)
#define log_debug(format, ...)
#define log_debug_hexdump(x, y)
#define log_error(format, ...)
#define log_error_hexdump(x, y)
#define log_char(x)

#endif

int printf_lite(const char *format, ...);
void log_putbyte(char c);
void put_buf_lite(void *_buf, u32 len);


#endif//__DEBUG_H_

