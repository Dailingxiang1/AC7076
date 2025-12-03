#ifndef _MY_PLATFORM_RECORD_H_
#define _MY_PLATFORM_RECORD_H_


#include "net_includes.h"
#include "fs.h"
#include "os/os_api.h"
#include "audio_def.h"

#define NET_REC_TYPE           AUDIO_CODING_OPUS

extern int ai_mic_is_busy(void);
extern int ai_mic_rec_close(void);
extern int mic_rec_pram_init(/* const char **name,  */u32 enc_type, u8 opus_type, u16(*speech_send)(u8 *buf, u16 len), u16 frame_num, u16 cbuf_size);
extern int ai_mic_rec_start(void);

#define NET_REC_CBUF_SIZE	10*1024

typedef struct {
    char *buf;
    cbuffer_t cbuf;
} net_rec_t;

typedef void (*StopCompletedCallback)(void);
/**
 * @brief   执行AI_TX录音
 *
 * @return  int 执行结果
 * @retval  1   成功
 * @retval  0  失败
 *
*/
extern int net_rec_start(void);

/**
 * @brief   获取录音缓存数据长度
 *
 * @return  int 缓存长度
 *
*/
extern int net_rec_data_len();

/**
 * @brief   获取录音缓存数据长度
 *
 * @retval  非0 长度
 * @retval  0  失败
 *
*/
extern int net_rec_read_data(void *buf, u32 len);

/**
 * @brief   清理缓存空间
*/
extern void net_rec_cbuf_exit(void);


/**
 * @brief   停止录音同时清理缓存空间
*/
extern void net_record_stop_with_clean();


/**
 * @brief   停止录音同时不清理缓存空间,需外部再去清理缓存空间
*/
extern void net_record_stop_without_clean();

#endif
