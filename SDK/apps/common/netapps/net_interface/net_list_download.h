#ifndef _NET_LIST_DOWNLOAD_H_
#define _NET_LIST_DOWNLOAD_H_

#include "system/includes.h"
#include "circular_buf.h"
#include "net_download.h"
#include "http/http_cli.h"
#include <stdlib.h>
#include "net_includes.h"
#include "os/os_api.h"

typedef struct {
    char *buf;
    cbuffer_t cbuf;
} net_buf_t;
/**
 * @brief   获取网络cbuf缓存数据长度
 *
*/
extern int net_cbuf_data_len();
/**
 * @brief   获取网络cbuf缓存数据
 *
 * @param[in]   buf     外部传入buf地址
 * @param[in]   len     外部传入buf长度
 * @param[out]  response 响应数据指针的地址，需要外部释放内存
 *
 * @return  int 执行结果
 * @retval  0   成功
 * @retval  -1  失败
 *
 * @note    调用者需要负责释放buf指向的内存
*/
extern int net_cbuf_read_data(void *buf, u32 len);
/**
 * @brief   执行url链式下载,需配合net_url_list执行,线程内部管理生存周期
*/
extern void net_url_download(void);
/**
 * @brief   url链式下载进度
 * @param[in]   current_index     外部传入获取当前url索引的变量地址
 * @param[in]   total_urls     	  外部传入获取总共url数量的变量地址

 * @param[out]  current_index 	  当前下载的url索引
 * @param[out]  total_urls        总共的url数量
*/
extern void get_download_progress(int *current_index, int *total_urls);

#endif
