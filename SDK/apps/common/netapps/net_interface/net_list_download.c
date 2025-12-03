#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".net_interface.data.bss")
#pragma data_seg(".net_interface.data")
#pragma const_seg(".net_interface.text.const")
#pragma code_seg(".net_interface.text")
#endif

#include "net_list_download.h"

#if NET_INTERFACE_EN

#define LOG_TAG_CONST       NET_INTERFACE
#define LOG_TAG             "[NET_INTERFACE_LIST_DOWNLOAD]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"


static net_buf_t net;
/*=========================cbuf========================*/
#define NET_RX_BUF_SIZE				1024*10			//网络缓存cbuf大小，可适当调整

static bool net_cbuf_init(void)
{
    if (!net.buf) {
        net.buf = net_interface_malloc(NET_RX_BUF_SIZE);
        if (!net.buf) {
            return false;
        }
    }
    cbuf_init(&net.cbuf, net.buf, NET_RX_BUF_SIZE);
    return true;
}

static void net_cbuf_write(void *buf, int len)
{
    int requested_len = len;
    static u32 w_len;
    int wlen = cbuf_write(&net.cbuf, buf, requested_len);
    w_len += wlen;
    if (wlen != requested_len) {
    }
}

static void net_cbuf_exit(void)
{
    cbuf_clear(&(net.cbuf));
    net_interface_free(net.buf);
    net.buf = NULL;
}

int net_cbuf_data_len()
{
    return cbuf_get_data_len(&(net.cbuf));
}

int net_cbuf_read_data(void *buf, u32 len)
{
    return cbuf_read(&(net.cbuf), buf, len);
}



/*=======================================================================*/
typedef struct {
    void *handle;                           // 传入指针以获取句柄
    int ret;                                // 返回值
    int download_status;                    // 网络下载状态[net_download.h]
    int http_err_status;                    // http链路状态
    struct net_download_parm parm;          // 下载参数配置
    int file_len;                           // 获取下载文件大小
    int bytes_read;                         // 记每次下载的数据量
    int total_read;                         // 记录当前下载数据总量
    char buf[1024];                         // 缓冲数据
} my_net_download_variable;                 // 在download_demo里面所用

static int g_current_url_index = 0;			//记录当前下载的URL索引
static int g_total_urls = 0;				//记录总共需要下载的URL数量


static void download_demo(const char *url, const char *save_path, int url_index)
{
    if (!url || !save_path) {
        log_error("错误: 参数为空\n");
        return;
    }

    log_info("开始下载第%d个URL: %s\n", url_index + 1, url);

    my_net_download_variable mynet;

    mynet.parm.url = url;
    mynet.parm.cbuf_size = 10 * 1024; 		// 10KB 环形缓冲区
    mynet.parm.timeout_millsec = 1000; 		// 10秒连接超时
    mynet.parm.save_file = 1; 				// 保存到文件
    mynet.parm.file_dir = save_path; 		// 保存目录
    mynet.parm.dir_len = strlen(save_path);
    mynet.parm.seek_threshold = 1024; 		// 1024KB跳转阈值

    mynet.ret = net_download_open(&mynet.handle, &mynet.parm);
    if (mynet.ret != 0) {
        log_error("net_download_open failed: %d\n", mynet.ret);
        return;
    }

    mynet.file_len = net_download_get_file_len(mynet.handle);
    if (mynet.file_len > 0) {
        log_error("File length: %d bytes\n", mynet.file_len);
    }

    while (1) {
        if (net_download_exit_flag(mynet.handle)) {
            log_error("Download exit flag set\n");
            goto close;
        }

        net_download_get_status(mynet.handle, &mynet.download_status, &mynet.http_err_status);
        if (mynet.download_status < 0) {
            log_info("Download failed: status=%d, http_err=%d\n",
                     mynet.download_status, mynet.http_err_status);
            goto close;
        } else if (mynet.download_status == NET_DOWNLOAD_COMPLETE) {
            log_info("Download completed successfully\n");
            goto close;
        }

        if (net_download_check_ready(mynet.handle) >= 0) {
            mynet.bytes_read = net_download_read(mynet.handle, mynet.buf, sizeof(mynet.buf));//这里实际会缓存够buf_size
            if (mynet.bytes_read > 0) {
                mynet.total_read += mynet.bytes_read;
                log_info("Download progress: %d bytes\r", mynet.total_read);
                net_cbuf_write(mynet.buf, mynet.bytes_read);
            } else if (mynet.bytes_read < 0) {
                log_info("Read error: %d\n", mynet.bytes_read);
                goto close;
            }
        } else {
            os_time_dly(10);
        }
    }

close:
    mynet.ret = net_download_close(mynet.handle);
    if (mynet.ret != 0) {
        log_error("net_download_close failed: %d\n", mynet.ret);
    } else {
        log_info("Download closed normally\n");
    }
}

static void stop_all_downloads();
static void net_download(void *priv)
{
    int msg[3] = {0};
    net_url_reset_iterator();

    g_total_urls = net_url_get_count();
    log_info("总共有 %d 个URL需要下载\n", g_total_urls);

    if (g_total_urls == 0) {
        log_error("错误: URL列表为空\n");
        return;
    }

    char save_path[128];

    for (g_current_url_index = 0; g_current_url_index < g_total_urls; g_current_url_index++) {
        char *url = net_get_url_by_index(g_current_url_index);
        if (url) {
            log_info("开始下载第%d个URL(共%d个): %s\n",
                     g_current_url_index + 1, g_total_urls, url);
            snprintf(save_path, sizeof(save_path), "storage/sd0/C/913_%d.mp3", g_current_url_index);
            download_demo(url, save_path, g_current_url_index);

            log_info("完成下载第%d个URL\n", g_current_url_index + 1);
        } else {
            log_error("错误: 获取第%d个URL失败\n", g_current_url_index + 1);
        }
        os_time_dly(50);
    }

    log_info("所有URL下载完成!\n");
    msg[0] = (int)stop_all_downloads;
    msg[1] = 1;
    os_taskq_post_type("app_core", Q_CALLBACK, 2, msg);
    os_time_dly(-1);
}

void net_url_download(void)
{
    net_cbuf_init();
    os_task_create(net_download, NULL, 29, 512 * 5, 0, "dl_task");
}

// 获取当前下载进度信息
void get_download_progress(int *current_index, int *total_urls)
{
    if (current_index) {
        *current_index = g_current_url_index;
    }
    if (total_urls) {
        *total_urls = g_total_urls;
    }
    log_info("current_download_url_index: %d total_urls %d ", current_index, total_urls);
}

// 停止所有下载
static void stop_all_downloads()
{
    net_cbuf_exit();
    task_kill("dl_task");
    g_current_url_index = 0;
    g_total_urls = 0;
    log_info("所有下载已停止\n");
}

#endif//NET_INTERFACE_EN
