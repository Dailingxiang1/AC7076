#ifndef _NET_URL_LIST_H_
#define _NET_URL_LIST_H_

#include "net_includes.h"

#include "list.h"
#include <stdlib.h>
#include <string.h>

// 定义URL列表的管理结构体
typedef struct {
    char **urls;          // 指向URL字符串指针数组的指针
    int capacity;         // 指针数组当前的容量
    int count;            // 当前存储的URL数量
    int iterator_index;   // 用于外部迭代的当前位置
} url_list_t;



/**
 * @brief 检查当前迭代位置是否指向最后一个URL
 *
 * 判断迭代器是否已到达最后一个URL（或越界）。
 *
 * @param None
 * @return int 1表示是最后一个或越界，0表示否
 *
 * @note 列表为空时返回1
 * @example if (net_url_is_last()) break;
 */
extern int net_url_is_last(void);
/**
 * @brief 初始化URL列表结构
 *
 * 初始化全局URL列表，设置初始容量为5，并分配初始内存。
 * 如果内存分配失败，容量设为0并记录错误。
 *
 * @param None
 * @return None
 *
 * @note 初始容量选择5是为了平衡内存使用和扩容频率。
 * @example net_url_list_init();
*/
extern void net_url_list_init(void);
/**
 * @brief 销毁URL列表并释放所有内存
 *
 * 安全释放所有URL字符串内存和指针数组内存，重置列表状态。
 * 防止内存泄漏的关键函数，必须在功能结束前调用。
 *
 * @param None
 * @return None
 *
 * @note 释放顺序：先释放每个URL字符串，再释放指针数组。
 * @example net_url_list_destroy();
 */
extern void net_url_list_destroy(void);
/**
 * @brief 添加URL到列表
 *
 * 将URL字符串复制到列表末尾。如果容量不足，自动扩容至原容量2倍。
 * 支持空URL检查，扩容失败或内存分配失败时记录错误。
 *
 * @param url 要添加的URL字符串（需以空字符结尾）
 * @return None
 *
 * @example net_url_set("xxxxxx");
 */

extern void net_url_set(char *url);
/**
 * @brief 打印所有URL到日志
 *
 * 调试用途：将列表中所有URL按索引打印到日志。
 *
 * @param None
 * @return None
 *
 * @example net_print_urls();
 */

extern void net_print_urls(void);
/**
 * @brief 通过索引获取URL
 *
 * 根据索引（从0开始）返回对应的URL字符串。索引越界时返回NULL并记录错误。
 *
 * @param index URL的索引（0 ≤ index < count）
 * @return char* 成功返回URL字符串指针，失败返回NULL
 *
 * @note 返回的指针为列表内部数据，不应手动释放。
 * @example char *url = net_get_url_by_index(0);
 */

extern char *net_get_url_by_index(int index);
/**
 * @brief 获取第一个URL并重置迭代器
 *
 * 便捷函数：重置迭代器并返回第一个URL。列表为空时返回NULL。
 *
 * @param None
 * @return char* 第一个URL或NULL
 *
 * @example char *first = net_url_get_first();
 */

extern char *net_url_get_first(void);
/**
 * @brief 获取下一个URL（迭代器方式）
 *
 * 返回迭代器当前指向的URL，并将迭代器移动到下一个位置。
 * 遍历完成后返回NULL。
 *
 * @param None
 * @return char* URL字符串或NULL（遍历结束时）
 *
 * @note 与net_url_reset_iterator配合使用，实现安全迭代。
 * @example
 * net_url_reset_iterator();
 * while ((char *url = net_url_get_next()) != NULL) { ... }
 */

extern char *net_url_get_next(void);
/**
 * @brief 重置迭代器到列表开头
 *
 * 将内部迭代器位置重置为0，用于重新开始遍历。
 *
 * @param None
 * @return None
 *
 * @see net_url_get_next
 * @example net_url_reset_iterator();
 */

extern void net_url_reset_iterator(void);
/**
 * @brief 获取当前URL数量
 *
 * 返回列表中当前存储的URL数量，用于循环或状态检查。
 *
 * @param None
 * @return int URL数量（总为非负整数）
 *
 * @example int count = net_url_get_count();
 */
extern int net_url_get_count(void);

#endif

