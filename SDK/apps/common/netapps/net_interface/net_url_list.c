#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".net_interface.data.bss")
#pragma data_seg(".net_interface.data")
#pragma const_seg(".net_interface.text.const")
#pragma code_seg(".net_interface.text")
#endif

/**
 * @file net_url_list.c
 * @brief URL列表管理接口实现
 *
 * 该模块提供动态URL列表的存储、迭代和内存管理功能。列表使用动态数组实现,
 * 支持自动扩容、安全迭代和资源清理。适用于需要管理多个URL字符串的场景,
 * 支持最小单元测试。
*/

#include "net_url_list.h"

#if NET_INTERFACE_EN

#define LOG_TAG_CONST       NET_INTERFACE
#define LOG_TAG             "[NET_URL_LIST]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"

static url_list_t g_url_list; // 全局URL列表实例

void net_url_list_init(void)
{
    g_url_list.capacity = 5; // 初始容量设为5
    g_url_list.count = 0;
    g_url_list.iterator_index = 0;
    // 为URL指针数组申请初始空间
    g_url_list.urls = (char **)net_interface_malloc(sizeof(char *) * g_url_list.capacity);
    if (g_url_list.urls == NULL) {
        g_url_list.capacity = 0;
        log_error("错误: 初始化内存分配失败\n");
    }
}

// 销毁URL列表，释放所有内存
void net_url_list_destroy(void)
{
    // 首先释放每个URL字符串本身的内存
    for (int i = 0; i < g_url_list.count; i++) {
        net_interface_free(g_url_list.urls[i]);
    }
    // 然后释放存放URL指针的数组
    net_interface_free(g_url_list.urls);
    // 重置所有状态
    g_url_list.urls = NULL;
    g_url_list.count = 0;
    g_url_list.capacity = 0;
    g_url_list.iterator_index = 0;
}

// 设置（添加）一个URL到列表中
void net_url_set(char *url)
{
    if (url == NULL) {
        log_warn("警告: 尝试添加空URL\n");
        return;
    }

    // 检查容量是否足够，不足则扩容
    if (g_url_list.count >= g_url_list.capacity) {
        int new_capacity = g_url_list.capacity * 2; // 容量翻倍
        char **new_urls = (char **)net_interface_realloc(g_url_list.urls, sizeof(char *) * new_capacity);

        if (new_urls == NULL) {
            log_error("错误: 内存扩容失败，无法添加URL: %s\n", url);
            return;
        }
        g_url_list.urls = new_urls;
        g_url_list.capacity = new_capacity;
    }

    // 为新的URL字符串分配内存并复制内容
    g_url_list.urls[g_url_list.count] = (char *)net_interface_malloc(strlen(url) + 1); // +1 用于字符串结束符'\0'
    if (g_url_list.urls[g_url_list.count] == NULL) {
        log_error("错误: 无法为URL分配内存: %s\n", url);
        return;
    }
    strcpy(g_url_list.urls[g_url_list.count], url);
    g_url_list.count++;
}

// 获取当前URL列表中的URL数量
int net_url_get_count(void)
{
    return g_url_list.count;
}

// 根据索引获取URL，索引从0开始
char *net_get_url_by_index(int index)
{
    if (index < 0 || index >= g_url_list.count) {
        log_error("错误: 索引%d越界(总数:%d)\n", index, g_url_list.count);
        return NULL;
    }
    return g_url_list.urls[index];
}

// 重置迭代器到列表开头
void net_url_reset_iterator(void)
{
    g_url_list.iterator_index = 0;
}

// 获取迭代器当前指向的URL，并将迭代器移动到下一个位置
char *net_url_get_next(void)
{
    if (g_url_list.iterator_index >= g_url_list.count) {
        return NULL; // 已经遍历完所有URL
    }
    return g_url_list.urls[g_url_list.iterator_index++];
}

// 获取第一个URL并重置迭代器
char *net_url_get_first(void)
{
    net_url_reset_iterator();
    return net_url_get_next();
}

// 判断当前URL是否是最后一个（相对于迭代器位置）
int net_url_is_last(void)
{
    // 如果列表为空或迭代器已在最后一个或之后，返回1
    return (g_url_list.count == 0) || (g_url_list.iterator_index >= g_url_list.count - 1);
}

// 打印所有URL
void net_print_urls(void)
{
    log_info("URL列表(共%d个):\n", g_url_list.count);
    for (int i = 0; i < g_url_list.count; i++) {
        log_info("%d: %s\n", i, g_url_list.urls[i]);
    }
}
#if 0
void test_url_management()
{
    log_info("URL List Manager Test\n");
    log_info("=====================\n");

    // 1. 初始化URL链表
    net_url_list_init();
    log_info("1. List initialized\n");

    // 2. 添加URL
    net_url_set("https://www.example.com");
    net_url_set("https://www.github.com");
    net_url_set("https://www.openai.com");
    net_url_set("https://www.kernel.org");
    net_url_set("https://www.python.org");
    log_info("2. URLs added\n");

    // 3. 打印所有URL
    net_print_urls();

    // 4. 按索引获取URL
    log_info("\n4. Access by index:");
    for (int i = 0; i < net_url_get_count(); i++) {
        char *url = net_get_url_by_index(i);
        log_info("\n  [%d] %s", i, url);
    }
    log_info("\n");

    // 5. 迭代器测试
    log_info("\n5. Iterator test:");
    int count = 0;
    for (char *url = net_url_get_first();
         url != NULL;
         url = net_url_get_next()) {
        log_info("\n  Iteration %d: %s", ++count, url);
    }

    // 6. 重置迭代器
    net_url_reset_iterator();

    // 7. 边缘测试：越界访问
    log_info("\n\n7. Boundary tests:");
    log_info("\n  Index -1: %s",
             net_get_url_by_index(-1) ? "Found" : "NULL");
    log_info("\n  Index %d: %s",
             net_url_get_count(),
             net_get_url_by_index(net_url_get_count()) ? "Found" : "NULL");

    // 8. 销毁链表
    net_url_list_destroy();
    log_info("\n\n8. List destroyed\n");

    // 9. 销毁后访问
    log_info("\n9. Post-destruction access:");
    log_info("\n  Count: %d", net_url_get_count());
    net_print_urls();

    // 10. 重新初始化并添加新URL
    net_url_list_init();
    net_url_set("https://www.new-start.com");
    log_info("\n10. Reinitialized list:\n");
    net_print_urls();
    net_url_list_destroy();
    return;
}
#endif
#endif//NET_INTERFACE_EN
