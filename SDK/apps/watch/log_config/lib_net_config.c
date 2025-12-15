#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".lib_net_config.data.bss")
#pragma data_seg(".lib_net_config.data")
#pragma const_seg(".lib_net_config.text.const")
#pragma code_seg(".lib_net_config.text")
#endif


#include "app_config.h"

//================================================//
//          lwip btnet 支持
//================================================//
#if USER_SUPPORT_PROFILE_PAN
const int CONFIG_LWIP_NET_ENABLE = 1;

/*
 * @brief 以下接口为各网络模块动态内存申请
 		  以帮助划分内存消耗和相关内存策略
 		  在各对应库内均有相关弱函数实现
 * @note
 		  set 1-----use psram mem
 		  set 0-----use ram mem
*/

#define LWIP_MEM_USE_PSRAM				1			//lwip内存分配
#define HTTP_MEM_USE_PSRAM				1			//http内存分配
#define HTTPS_MEM_USE_PSRAM				1			//https内存分配
#define NET_AUDIO_MEM_USE_PSRAM			1			//net_audio内存分配
#define MBEDTLS_MEM_USE_PSRAM           1           //mbedtls库内存分配
#define WEBSOCKET_API_USE_PSRAM         1           //websocket使用psram

extern void *realloc_psram(void *pv, size_t size);

#if LWIP_MEM_USE_PSRAM
#define LWIP_ALLOC(size) 			malloc_psram(size)
#define LWIP_FREE(ptr) 				free_psram(ptr)
#else
#define LWIP_ALLOC(size) 			malloc(size)
#define LWIP_FREE(ptr) 				free(ptr)
#endif

void *lwip_malloc(size_t size)
{
    return LWIP_ALLOC(size);
}

void *lwip_calloc(size_t count, size_t size)
{
    size_t total_size = count * size;
    void *p = LWIP_ALLOC(total_size);
    if (p) {
        memset(p, 0, total_size);
    }
    return p;
}

void lwip_free(void *pv)
{
    if (pv != NULL) {
        LWIP_FREE(pv);
    }
}




#if HTTP_MEM_USE_PSRAM
#define HTTP_MALLOC(size) 			malloc_psram(size)
#define HTTP_REALLOC(ptr,size) 		realloc_psram(ptr,size)
#define HTTP_FREE(ptr) 				free_psram(ptr)
#else
#define HTTP_MALLOC(size) 			malloc(size)
#define HTTP_REALLOC(ptr,size) 		realloc(ptr,size)
#define HTTP_FREE(ptr) 				free(ptr)
#endif

void *http_malloc(size_t size)
{
    return HTTP_MALLOC(size);
}

void *http_realloc(void *ptr, size_t size)
{
    return HTTP_REALLOC(ptr, size);
}

void http_free(void *pv)
{
    if (pv != NULL) {
        HTTP_FREE(pv);
    }
}






#if HTTPS_MEM_USE_PSRAM
#define HTTPS_ALLOC(size) 				malloc_psram(size)
#define HTTPS_FREE(ptr) 				free_psram(ptr)
#else
#define HTTPS_ALLOC(size) 				malloc(size)
#define HTTPS_FREE(ptr) 				free(ptr)
#endif


void *https_malloc(size_t size)
{
    return HTTPS_ALLOC(size);
}

void *https_calloc(unsigned long count, unsigned long size)
{
    size_t total_size = count * size;
    void *p = HTTPS_ALLOC(total_size);
    if (p) {
        memset(p, 0, count * size);
    }
    return p;
}


void https_free(void *pv)
{
    if (pv != NULL) {
        HTTPS_FREE(pv);
    }
}





#if NET_AUDIO_MEM_USE_PSRAM
#define NET_AUDIO_ALLOC(size) 				malloc_psram(size)
#define NET_AUDIO_FREE(ptr) 				free_psram(ptr)
#else
#define NET_AUDIO_ALLOC(size) 				malloc(size)
#define NET_AUDIO_FREE(ptr) 				free(ptr)
#endif


void *net_audio_malloc(size_t size)
{
    void *p = NET_AUDIO_ALLOC(size);
    return p;
}

void net_audio_free(void *pv)
{
    if (pv != NULL) {
        NET_AUDIO_FREE(pv);
    }
}

void *net_audio_calloc(unsigned long count, unsigned long size)
{
    size_t total = count * size;
    void *p = NET_AUDIO_ALLOC(total);
    if (p) {
        memset(p, 0, total);
    }
    return p;
}

#if MBEDTLS_MEM_USE_PSRAM
#define MBEDTLS_MALLOC(size) 		    malloc_psram(size)
#define MBEDTLS_FREE(ptr) 				free_psram(ptr)
#else
#define MBEDTLS_MALLOC(size) 		    malloc(size)
#define MBEDTLS_FREE(ptr) 				free(ptr)
#endif


void jl_mbedtls_free(void *pv)
{
    if (pv != NULL) {
        MBEDTLS_FREE(pv);
    }
}

void *jl_mbedtls_calloc(unsigned long count, unsigned long size)
{
    size_t total = count * size;
    void *p = MBEDTLS_MALLOC(total);
    if (p) {
        memset(p, 0, total);
    }
    return p;
}

#if WEBSOCKET_API_USE_PSRAM
#define WEBSOCKET_API_MALLOC(size) 		    malloc_psram(size)
#define WEBSOCKET_API_REALLOC(ptr, size) 	realloc_psram(ptr, size)
#define WEBSOCKET_API_FREE(ptr) 			free_psram(ptr)
#else
#define WEBSOCKET_API_MALLOC(size) 		    malloc(size)
#define WEBSOCKET_API_REALLOC(ptr, size) 	realloc(ptr, size)
#define WEBSOCKET_API_FREE(ptr) 			free(ptr)
#endif


void websocket_api_free(void *pv)
{
    if (pv != NULL) {
        WEBSOCKET_API_FREE(pv);
    }
}

void *websocket_api_malloc(size_t size)
{
    void *p = WEBSOCKET_API_MALLOC(size);
    if (p) {
        memset(p, 0, size);
    }
    return p;
}

void jl_websocket_api_free(void *pv)
{
    if (pv != NULL) {
        WEBSOCKET_API_FREE(pv);
        pv = NULL;
    }
}

void *jl_websocket_api_malloc(size_t size)
{
    void *p = WEBSOCKET_API_MALLOC(size);
    return p;
}

void *jl_websocket_api_zalloc(size_t size)
{
    void *ptr = WEBSOCKET_API_MALLOC(size);
    if (ptr) {
        memset(ptr, 0, size);
    }
    return ptr;
}

void *jl_websocket_api_realloc(void *ptr, size_t size)
{
    return WEBSOCKET_API_REALLOC(ptr, size);
}
#else
const int CONFIG_LWIP_NET_ENABLE = 0;
#endif


