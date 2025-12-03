#ifndef _DUER_SOCKET_H_
#define _DUER_SOCKET_H_



#include "websocket_api.h"
#include "duer_common.h"



extern void duer_websocket_client_thread_create(void *priv);

extern int task_kill(const char *name);


#endif
