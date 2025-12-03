
#ifndef __AVI_VIDEO_
#define __AVI_VIDEO_


#include "app_config.h"
#include "cpu.h"
#include "fs/fs.h"
#include "app_config.h"
#include "board_config.h"
#include "generic/circular_buf.h"
#include "app_config.h"
#include "asm/audio_adc.h"
#include "generic/circular_buf.h"
#include "media/audio_decoder.h"
#include "media/mixer.h"
#include "generic/log.h"
#include "app_config.h"
#include "system/timer.h"
#include "device/device.h"
#include "key_event_deal.h"

#include "res/resfile.h"
#include "jlui/ui.h"
#include "jlui_app/ui_style.h"
#include "jlui_app/ui_api.h"
#include "jlui_app/res_config.h"
#include "jlui_app/ui_resource.h"
#include "jlui_app/ui_sys_param.h"

#include "timer.h"	// 定时器
#include "ui/lcd/lcd_drive.h"	// lcd 驱动

#include "res_config.h"	// res 文件管理
#include <math.h>
#include "ascii.h"
#include "res/font_ascii.h"
#include "res/mem_var.h"
#include "res/zz.h"
#include "res/jpeg_decoder.h"
#include "dev_manager.h"
#include "fs/fs.h"
#include "watch_syscfg_manage.h"

#include "ui_core.h"
#include "control.h"

#include "dbi.h"	// 推屏模块
#include "jlgpu_math.h"		// matrix
#include "jlgpu_driver.h"	// gpu 驱动

#include "gpu_port.h"	// GPU 接口，依赖于 jlgpu_driver.h
#include "gpu_task.h"
#include "ui_resource.h"	// ui资源管理

#include "jlui_effect/jlui_effect.h"		//特效相关
#include "ui_expand/buffer_manager.h"		// buffer管理
#include "ui_expand/ui_expand.h"		// 部分宏定义

#include "font/font_all.h"
#include "font/font_textout.h"

#include "ui_draw/ui_circle.h"
#include "ui_animation.h"
#include "font/language_list.h"

#include "ui_measure.h"
#include "jljpeg_decode.h"
#include "gpu_draw.h"
#include "ui_page_switch.h"
#include "ui_draw/ui_basic.h"
#include "ui_text.h"
#include "jlui_app/ui_style.h"
#include "res/resfile.h"

#include "media/includes.h"
#include "audio_config.h"
#include "gpio.h"




typedef void (*ChunkCallback)(FILE *file,
                              off_t chunk_offset,
                              const char *chunk_id,
                              uint32_t chunk_size,
                              const char *list_name,
                              const char **hierarchy,
                              int level,
                              int *jump,
                              void *user_param);


#pragma pack(push, 1)

typedef struct {
    uint32_t dwMicroSecPerFrame;  // 每帧时长（微秒）
    uint32_t dwMaxBytesPerSec;    // 最大字节/秒（数据传输率）
    uint32_t dwPaddingGranularity;// 数据填充粒度（单位字节）
    uint32_t dwFlags;             // 全局标志（如索引存在性）
    uint32_t dwTotalFrames;       // 总帧数
    uint32_t dwInitialFrames;     // 初始帧数（用于交错格式）
    uint32_t dwStreams;           // 流数量（如视频、音频）
    uint32_t dwSuggestedBufferSize; // 建议缓冲区大小
    uint32_t dwWidth;             // 视频宽度（像素）
    uint32_t dwHeight;            // 视频高度（像素）
    uint32_t dwReserved[4];       // 保留字段（必须为0）
} AVIH_Header;



// 流头结构（strh）
typedef struct {
    char    fccType[4];       // 流类型 'vids'或'auds'
    char    fccHandler[4];    // 编码类型
    uint32_t dwFlags;         // 标志位
    uint16_t wPriority;       // 优先级
    uint16_t wLanguage;       // 语言
    uint32_t dwInitialFrames; // 初始帧数
    uint32_t dwScale;         // 时间刻度
    uint32_t dwRate;          // 每秒采样数
    uint32_t dwStart;         // 开始时间
    uint32_t dwLength;        // 流长度
    uint32_t dwSuggestedBufferSize;
    uint32_t dwQuality;
    uint32_t dwSampleSize;    // 样本大小
} StreamHeader;

// 视频格式结构（strf）
typedef struct {
    uint32_t biSize;          // 结构体大小
    uint32_t biWidth;         // 视频宽度
    uint32_t biHeight;        // 视频高度
    uint16_t biPlanes;        // 颜色平面数
    uint16_t biBitCount;      // 每像素位数
    uint32_t biCompression;   // 压缩类型
    uint32_t biSizeImage;     // 图像数据大小
} VideoFormat;

// 音频格式结构（strf）
typedef struct {
    uint16_t wFormatTag;      // 格式类型
    uint16_t nChannels;       // 声道数
    uint32_t nSamplesPerSec;  // 采样率
    uint32_t nAvgBytesPerSec; // 平均字节率
    uint16_t nBlockAlign;     // 块对齐
    uint16_t wBitsPerSample;  // 位深
} AudioFormat;

typedef struct {
    u32 time; // jpeg 最新更新时间
    jdec_opj *opj; // jpeg 句柄
    int pos;  // 相对文件位置
    int len;  // 单个JPEG长度
    u8 *data;
} mjpegdata;

typedef struct {
    int block_len; // 最大的jpeg 长度
    int cnt;    // 缓存数量
    int act_idx;    // 当前正在忙的缓存
    int pre_idx;
    mjpegdata *m;
    u8 *data;  //所有jpeg+信息数据
} cachebuf;



#pragma pack(pop) // 恢复内存对齐


typedef enum {
    STREAM_NULL,
    STREAM_VIDEO,
    STREAM_AUDIO
} StreamType;

typedef struct {
    // 文件信息
    FILE *file;
    int movi_data_start;  // movi列表区起始位置(movi字符前)
    size_t movi_data_len;   //(包括movi字符)
    int idx1_offset;      // idx1块起始偏移
    size_t idx1_size;       // idx1块数据大小

    AVIH_Header avih_header;

    //  流头状态机
    StreamType stream_type;
    StreamHeader video_stream_header;
    VideoFormat video_format;
    StreamHeader audio_stream_header;
    AudioFormat audio_format;

    // 视频信息
    int width;
    int height;
    int frame_rate;
    uint32_t total_frames;
    uint64_t total_duration;

    // 音频信息
    int sample_rate;
    int channels;
    int bits_per_sample;
    u8 auds_vol;
    u8 auds_drop;
    int auds_idx;
    u8 auds_exit;
    u16 auds_inr;
    int auds_rate;
    int auds_total;
    char *auds_frame;
    cbuffer_t auds_cbuf;

    // 播放状态
    int current_pos;
    int is_playing;
    int lv_pos;
    int lv_size;

    cachebuf cache;

    int timerid;
    int redrawid;
    u8 ui_work;

    unsigned long jump_mjpeg;
    // MyReadHandle frhandle; // fread缓冲

    int now_offset; //  在文件的偏移
    int now_len;    //  块长度

    struct flash_file_info *view_file_info;

    OS_MUTEX mutex;

    struct audio_dac_hdl ad;

    uint8_t buf_ok;
} AVIPlayer;
void *avi_open(const char *path, int en_buf, int redrawid);
void avi_close(void *avip);
int avi_fluh(void *priv);
typedef struct {
    void *fp;
    char fname[64];
    OS_SEM mv_sem;
    void *st;   // 解码器
    int drawid;
    int act; // -1: 销毁 0：关闭解码器 1：活跃
    u8 pause;
    u8 repeat;
    u8 type;
    u8 avi_audio_start;
    u8 avi_play_mode;
    u8 avi_is_switching;
    u8 avi_index;
    u16 avi_playtimer;
    int start;
    int time;
    int frameCount;
    int flushTimer;
    int avi_now_idx;
    u8 *avi_pcm_buf;
    long file_offset;
} MV_DRAW;







void __jpeg_draw_cb_gpu(int id, u8 *dst_buf, struct rect *dst_r, struct rect *src_r, u8 bytes_per_pixel, void *priv, void *matrix);

void avi_pcm_open(void *avip);
void avi_pcm_close(void *avip) ;
// 0918新增内容

const char *set_avi_play_file(void);
void avi_play_next(void);
void avi_play_prev(void);
void avi_play_shutdown(void);
void set_avi_play_mode(u8 mode);

int avi_get_width(void *avip);
int avi_get_height(void *avip);
void *avi_get_fp(void *avip);
int avi_get_now_len(void *avip);
int avi_get_now_offset(void *avip);
int avi_get_mjpegbuflen(void *avip);
int avi_get_redrawid(void *avip);
float avi_get_total_sec(void *avip);
void *avi_get_view_file_info(void *priv);

void *animig_open(char *name, int window_id, int arg);

void avi_set_avi_playtimer_id(u16 timer_id);
u16 avi_get_avi_playtimer_id();
void *get_aviplay_handle();
void *get_avi_player_st_handle();


#define AVI_WINDOW     PAGE_77
#define AVI_LAYOUT   AVI_DEMO_LAYOUT
#endif



