#ifndef _DUER_AUTH_ALGORITHM_H_
#define _DUER_AUTH_ALGORITHM_H_


#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "sys_time.h"
#include "duer_common.h"
#include "app_config.h"

extern unsigned int random32(int type);

// 生成对话请求ID函数
extern void duer_generate_dialog_request_id(char *request_id);


// 生成6个字符的随机字符串
extern void duer_generate_random_string(char *output, int length);

#endif
