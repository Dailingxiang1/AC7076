#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".net_interface.data.bss")
#pragma data_seg(".net_interface.data")
#pragma const_seg(".net_interface.text.const")
#pragma code_seg(".net_interface.text")
#endif
#include "net_record.h"

#if NET_INTERFACE_EN

#define LOG_TAG_CONST       NET_INTERFACE
#define LOG_TAG             "[NET_INTERFACE_RECORD]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"


#define NET_AUDIO_SAVE_TEST    1
#if NET_AUDIO_SAVE_TEST
static FILE *rec_file = NULL;
#define __file rec_file
#define FILE_SAVE_PATH			"storage/sd0/C/record.bin"
#endif

static net_rec_t rc;
#define __buf	rc.buf
#define __cbuf	rc.cbuf

static void net_rec_cbuf_init()
{
    if (!__buf) {
        __buf = net_interface_malloc(NET_REC_CBUF_SIZE);
    }
    cbuf_init(&__cbuf, __buf, NET_REC_CBUF_SIZE);
}

void net_rec_cbuf_exit(void)
{
    cbuf_clear(&__cbuf);
    net_interface_free(__buf);
    __buf = NULL;
}

static int my_fwrite(FILE *file, void *buf, u32 size)
{
    int ret = fwrite(buf, size, 1, file);
    return ret;
}

static u16 net_rec_write_data(u8 *voice_buf, u16 voice_len)
{
#if NET_AUDIO_SAVE_TEST
    if (__file) {
        int wlen = my_fwrite(__file, voice_buf, voice_len);
        if (wlen != voice_len) {
            log_error("save file err: %d, %d\n", wlen, voice_len);
        }
    }
#endif
    int wlen = cbuf_write(&__cbuf, voice_buf, voice_len);
    if (wlen != voice_len) {
        log_error("pcm out err: %d, %d\n", wlen, voice_len);
    }
    return 0;
}

static int net_rec_stop(StopCompletedCallback callback)
{
    if (!ai_mic_is_busy()) {
        log_info("ai_mic_is_null \n\n");
        return true;
    }
    ai_mic_rec_close();
#if NET_AUDIO_SAVE_TEST
    if (__file) {
        fclose(__file);
        __file = NULL;
    }
#endif
    if (callback) {
        callback();
    }
    return true;
}


int net_rec_start(void)
{
    printf(">>>zwz info: %s %d %s\n", __FUNCTION__, __LINE__, __FILE__);
    net_rec_cbuf_init();
    if (ai_mic_is_busy()) {
        log_error("my_mic_is_busy \n\n");
        return false;
    }
#if NET_AUDIO_SAVE_TEST
    if (__file) {
        fclose(__file);
        __file = NULL;
    }
    __file = fopen(FILE_SAVE_PATH, "w+");
    if (!__file) {
        log_error("fopen err \n\n");
    }
#endif
    mic_rec_pram_init(NET_REC_TYPE, 0, net_rec_write_data, 1, 1024);
    ai_mic_rec_start();
    return true;
}

int net_rec_data_len()
{
    return cbuf_get_data_len(&__cbuf);
}

int net_rec_read_data(void *buf, u32 len)
{
    return cbuf_read(&__cbuf, buf, len);
}


void net_record_stop_with_clean()
{
    net_rec_stop(net_rec_cbuf_exit);
}


void net_record_stop_without_clean()
{
    net_rec_stop(NULL);
}

void net_rec_test(void *priv)
{
    net_record_stop_with_clean();
}

void net_record_test()
{
    printf(">>>zwz info: %s %d %s\n", __FUNCTION__, __LINE__, __FILE__);
    net_rec_start();
    sys_timeout_add(NULL, net_rec_test, 5000);

}
#endif//NET_INTERFACE_EN
