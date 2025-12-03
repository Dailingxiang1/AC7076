#ifndef  _NET_HTTP_H_
#define  _NET_HTTP_H_

#include "http/http_cli.h"
#include  "system/includes.h"
#include "net_includes.h"

/**
 * @brief   执行HTTP GET请求
 *
 * @param[in]   url     请求的URL地址
 * @param[out]  response 响应数据指针的地址，需要外部释放内存
 *
 * @return  int 执行结果
 * @retval  0   成功
 * @retval  -1  失败
 *
 * @note    调用者需要负责释放response指向的内存
 * @warning URL参数不能为NULL
*/
extern int net_http_get_request(char *url, char **response);

/**
 * @brief   执行HTTP POST请求
 *
 * @param[in]   url     请求的URL地址
 * @param[out]  response 响应数据指针的地址，需要外部释放内存
 *
 * @return  int 执行结果
 * @retval  0   成功
 * @retval  -1  失败
 *
 * @note    调用者需要负责释放response指向的内存
 * @warning URL参数不能为NULL
*/
extern int net_http_post_request(char *url, char **response);

#endif

