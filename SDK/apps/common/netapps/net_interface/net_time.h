#ifndef _NET_TIME_H_
#define _NET_TIME_H_




#include "system/includes.h"
#include "sys_time.h"
#include <time.h>
#include "net_includes.h"
#include "timestamp.h"
#include "rtc.h"

extern void net_get_sys_time(struct sys_time *time);

#endif
