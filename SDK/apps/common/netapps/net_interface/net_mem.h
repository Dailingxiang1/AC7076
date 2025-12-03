#ifndef _NET_MEM_H
#define _NET_MEM_H

#include "system/includes.h"
#include "net_includes.h"

extern void *net_interface_malloc(size_t size);
extern void net_interface_free(void *pv);
extern void *net_interface_calloc(unsigned long count, unsigned long size);
extern void *net_interface_realloc(void *ptr, size_t size);

#endif // _NET_MEM_H
