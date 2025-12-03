#ifndef _DUER_COMMON_H_
#define _DUER_COMMON_H_



#include "system/includes.h"
#include "duer_auth_algorithm.h"
#include "duer_http_req.h"
#include "duer_socket.h"
#include "duer_json_parse.h"
#include "duer_json_request.h"
#include "duer_task.h"
#include "board_config.h"
#include "cJSON.h"
#include "net_includes.h"


#if TCFG_INTELLIGENT_DUER && TCFG_NETAPPLICATION_ENABLE
#define INTELLIGENT_DUER TCFG_INTELLIGENT_DUER
#else
#define INTELLIGENT_DUER 0
#endif



#define DUER_CLIENT_AK				"xxxxxx"
#define DUER_CLIENT_SK				"xxxxxx"
#define DUER_APP_PID				"xxxxxx"
#define DUER_APP_FORMAT				"opus"
#define DUER_APP_SAMPLE				16000



#endif
