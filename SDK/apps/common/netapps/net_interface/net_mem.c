#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".net_interface.data.bss")
#pragma data_seg(".net_interface.data")
#pragma const_seg(".net_interface.text.const")
#pragma code_seg(".net_interface.text")
#endif
#include "net_mem.h"

#if NET_INTERFACE_EN

#define NET_INTERFACE_MEM_USE_PSRAM		1

#if NET_INTERFACE_MEM_USE_PSRAM
#define NET_MALLOC(size) 			malloc_psram(size)
#define NET_REALLOC(ptr,size) 		realloc_psram(ptr,size)
#define NET_FREE(ptr) 				free_psram(ptr)
#else
#define NET_MALLOC(size) 			malloc(size)
#define NET_REALLOC(ptr,size) 		realloc(ptr,size)
#define NET_FREE(ptr) 				free(ptr)
#endif

void *net_interface_malloc(size_t size)
{
    return NET_MALLOC(size);
}

void net_interface_free(void *pv)
{
    if (pv != NULL) {
        NET_FREE(pv);
    }
}

void *net_interface_calloc(unsigned long count, unsigned long size)
{
    size_t total = count * size;
    void *p = NET_MALLOC(total);
    if (p) {
        memset(p, 0, total);
    }
    return p;
}

void *net_interface_realloc(void *ptr, size_t size)
{
    return NET_REALLOC(ptr, size);
}

_WEAK_
void *calloc(unsigned long count, unsigned long size)
{
    void *p;

    p = malloc(count * size);
    if (p) {
        memset(p, 0, count * size);
    }
    return p;
}

#endif//NET_INTERFACE_EN
