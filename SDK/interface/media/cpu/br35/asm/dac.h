#ifndef __CPU_DAC_H__
#define __CPU_DAC_H__


#include "generic/typedef.h"
#include "generic/atomic.h"
#include "os/os_api.h"
#include "audio_src.h"
#include "media/audio_cfifo.h"
#include "system/spinlock.h"
#include "audio_def.h"

#define AUDIO_DAC_CHANNEL_NUM	1

// TCFG_AUDIO_DAC_MODE
#define DAC_MODE_SINGLE                    (0)
#define DAC_MODE_DIFF                      (1)
#define DAC_MODE_VCMO                      (2)

/************************************
             dac性能模式
************************************/
// TCFG_DAC_PERFORMANCE_MODE
#define	DAC_MODE_HIGH_PERFORMANCE          (0)
#define	DAC_MODE_LOW_POWER		           (1)

/************************************
             dac 供电模式
************************************/
// TCFG_DAC_POWER_LEVEL
#define	AUDIO_VCM_CAP_LEVEL1	           (0)	// VCM-cap, VCM = 1.2v
#define	AUDIO_VCM_CAP_LEVEL2	           (1)  // VCM-cap, VCM = 1.3v
#define	AUDIO_VCM_CAP_LEVEL3	           (2)	// VCM-cap, VCM = 1.4v
#define	AUDIO_VCM_CAP_LEVEL4	           (3)	// VCM-cap, VCM = 1.5v
#define	AUDIO_VCM_CAP_LEVEL5	           (4)	// VCM-cap, VCM = 1.6v
#define	AUDIO_VCM_CAPLESS_LEVEL1           (5)	// VCM-capless, DACLDO = 2.4v
#define	AUDIO_VCM_CAPLESS_LEVEL2           (6)	// VCM-capless, DACLDO = 2.6v
#define	AUDIO_VCM_CAPLESS_LEVEL3           (7)	// VCM-capless, DACLDO = 2.8v
#define	AUDIO_VCM_CAPLESS_LEVEL4           (8)	// VCM-capless, DACLDO = 3.0v


#define AUDIO_DAC_SYNC_IDLE             0
#define AUDIO_DAC_SYNC_PROBE            1
#define AUDIO_DAC_SYNC_START            2
#define AUDIO_DAC_SYNC_NO_DATA          3
#define AUDIO_DAC_ALIGN_NO_DATA         4
#define AUDIO_DAC_SYNC_ALIGN_COMPLETE   5
#define AUDIO_DAC_SYNC_KEEP_RATE_DONE   6

#define AUDIO_SRC_SYNC_ENABLE   1
#define SYNC_LOCATION_FLOAT      1
#if SYNC_LOCATION_FLOAT
#define PCM_PHASE_BIT           0
#else
#define PCM_PHASE_BIT           8
#endif

#define DA_LEFT        0
#define DA_RIGHT       1

#define DA_SOUND_NORMAL                 0x0
#define DA_SOUND_RESET                  0x1
#define DA_SOUND_WAIT_RESUME            0x2

#define DACR32_DEFAULT		8192
#define DA_SYNC_INPUT_BITS              20
#define DA_SYNC_MAX_NUM                 (1 << DA_SYNC_INPUT_BITS)

//void audio_dac_power_state(u8 state)
//在应用层重定义 audio_dac_power_state 函数可以获取dac模拟开关的状态
struct audio_dac_hdl;
struct dac_platform_data {
    void (*analog_open_cb)(struct audio_dac_hdl *);
    void (*analog_close_cb)(struct audio_dac_hdl *);
    void (*analog_light_open_cb)(struct audio_dac_hdl *);
    void (*analog_light_close_cb)(struct audio_dac_hdl *);
    u16 dma_buf_time_ms;    // DAC dma buf 大小
    u8 output;              // DAC输出模式
    u8 output_mode;         // single/diff/vcmo
    u8 performance_mode;    // low_power/high_performance
    u8 light_close;         //使能轻量级关闭，最低功耗保持dac开启
    u32 max_sample_rate;    //支持的最大采样率
    u8 dcc_level;				// DAC去直流滤波器档位, 0~7:关闭    8~15：开启(档位越大，高通截止点越小)
    u8 bit_width;             // DAC输出位宽
    u8 power_level;
    u8 mode;
    u8 clock_mode;
};

struct analog_module {
    /*模拟相关的变量*/
    u8 inited;
};

struct digital_module {
    /*数字相关的变量*/
    u8 inited;
};

struct trim_init_param_t {
    u8 clock_mode;
    u8 power_level;
    struct audio_dac_trim *dac_trim;
    struct audio_dac_hdl *dac;              /* DAC设备*/
};

struct audio_dac_trim {
    s16 left;
    s16 right;
};

// *INDENT-OFF*
struct audio_dac_sync {
    u32 channel : 3;
    u32 start : 1;
    u32 fast_align : 1;
    u32 connect_sync : 1;
    u32 release_by_bt : 1;
    u32 resevered : 1;
    u32 input_num : DA_SYNC_INPUT_BITS;
    int fast_points;
    int keep_points;
    int phase_sub;
    int in_rate;
    int out_rate;
    int bt_clkn;
    int bt_clkn_phase;
#if AUDIO_SRC_SYNC_ENABLE
    struct audio_src_sync_handle *src_sync;
    void *buf;
    int buf_len;
    void *filt_buf;
    int filt_len;
#else
    struct audio_src_base_handle *src_base;
#endif
#if SYNC_LOCATION_FLOAT
    float pcm_position;
#else
    u32 pcm_position;
#endif
    void *priv;
    int (*handler)(void *priv, u8 state);
    void *correct_priv;
    void (*correct_cabllback)(void *priv, int diff);
};

struct audio_dac_fade {
    u8 enable;
    volatile u8 ref_L_gain;
    volatile u8 ref_R_gain;
    int timer;
};

struct audio_dac_sync_node {
    u8 triggered;
    u8 network;
    u32 timestamp;
    void *hdl;
    struct list_head entry;
    void *ch;
};

struct audio_dac_channel_attr {
    u8  write_mode;         /*DAC写入模式*/
    u16 delay_time;         /*DAC通道延时*/
    u16 protect_time;       /*DAC延时保护时间*/
};

struct audio_dac_channel {
    u8  state;              /*DAC状态*/
    u8  pause;
    u8  samp_sync_step;     /*数据流驱动的采样同步步骤*/
    struct audio_dac_channel_attr attr;     /*DAC通道属性*/
    struct audio_sample_sync *samp_sync;    /*样点同步句柄*/
    struct audio_dac_hdl *dac;              /* DAC设备*/
    struct audio_cfifo_channel fifo;        /*DAC cfifo通道管理*/

    //struct audio_dac_sync sync;
    // struct list_head sync_list;
};

struct audio_dac_hdl {
    struct analog_module analog;
    struct digital_module digital;
    const struct dac_platform_data *pd;
    OS_SEM sem;
    struct audio_dac_trim trim;
    void (*fade_handler)(u8 left_gain, u8 right_gain);
    void (*update_frame_handler)(u8 channel_num, void *data, u32 len);
    volatile u8 mute;
    volatile u8 state;
    volatile u8 light_state;
    volatile u8 agree_on;
    u8 gain;
    u8 vol_l;
    u8 vol_r;
    u8 channel;
    u16 max_d_volume;
    u16 d_volume[4];
    u32 sample_rate;
    u16 start_ms;
    u16 delay_ms;
    u16 start_points;
    u16 delay_points;
    u16 prepare_points;//未开始让DAC真正跑之前写入的PCM点数
    u16 irq_points;
    s16 protect_time;
    s16 protect_pns;
    s16 fadein_frames;
    s16 fade_vol;
    u8 protect_fadein;
    u8 vol_set_en;
    u8 volume_enhancement;
    u8 sound_state;
    unsigned long sound_resume_time;
    s16 *output_buf;
    u16 output_buf_len;

    u8 anc_dac_open;
    u8 dac_read_flag;	//AEC可读取DAC参考数据的标志

    u8 fifo_state;
    u16 unread_samples;             /*未读样点个数*/
    struct audio_cfifo fifo;        /*DAC cfifo结构管理*/
    struct audio_dac_channel main_ch;

	u8 dvol_mute;             //DAC数字音量是否mute
#if 0
    struct audio_dac_sync sync;
    struct list_head sync_list;
    u8 sync_step;
#endif

	u8 active;
    s16 weight;

    void *resample_ch;
    void *resample_buf;
    int resample_buf_len;
	void *feedback_priv;
    void (*underrun_feedback)(void *priv);
/*******************************************/
/**sniff退出时，dac模拟提前初始化，避免模拟初始化延时,影响起始同步********/
	u8 power_on;
    u8 need_close;
    OS_MUTEX mutex;
    OS_MUTEX mutex_power_off;
    OS_MUTEX dvol_mutex;
/*******************************************/
    spinlock_t lock;
	u32 dac_dvol;             //记录DAC 停止前数字音量寄存器DAC_VL0的值

/*******************************************/
	struct list_head sync_list;
};

/*
*********************************************************************
*              audio_dac_init
* Description: DAC 初始化
* Arguments  : dac      dac 句柄
*			   pd		dac 参数配置结构体
* Return	 : 0：成功  -1：失败
* Note(s)    :
*********************************************************************
*/
int audio_dac_init(struct audio_dac_hdl *dac, const struct dac_platform_data *pd);

void *audio_dac_get_pd_data(void);
/*
*********************************************************************
*              audio_dac_get_hdl
* Description: 获取 DAC 句柄
* Arguments  :
* Return	 : DAC 句柄
* Note(s)    :
*********************************************************************
*/
struct audio_dac_hdl *audio_dac_get_hdl(void);

int audio_dac_trim_value_check(struct audio_dac_trim *dac_trim);

/*
*********************************************************************
*              audio_dac_do_trim
* Description: DAC 直流偏置校准
* Arguments  : dac      	dac 句柄
*			   dac_trim		存放 trim 值结构体的地址
*              fast_trim 	快速 trim 使能
* Return	 : 0：成功  -1：失败
* Note(s)    :
*********************************************************************
*/
int audio_dac_do_trim(struct audio_dac_hdl *dac, struct audio_dac_trim *dac_trim, u8 fast_trim);

/*
*********************************************************************
*              audio_dac_set_trim_value
* Description: 将 DAC 直流偏置校准值写入 DAC 配置
* Arguments  : dac      	dac 句柄
*			   dac_trim		存放 trim 值结构体的地址
* Return	 : 0：成功  -1：失败
* Note(s)    :
*********************************************************************
*/
int audio_dac_set_trim_value(struct audio_dac_hdl *dac, struct audio_dac_trim *dac_trim);

/*
*********************************************************************
*              audio_dac_irq_handler
* Description: DAC 中断回调函数
* Arguments  : dac      	dac 句柄
* Return	 :
* Note(s)    :
*********************************************************************
*/
void audio_dac_irq_handler(struct audio_dac_hdl *dac);

/*
*********************************************************************
*              audio_dac_write
* Description: 将数据写入默认的 dac channel cfifo。等同于调用 audio_dac_channel_write 函数 private_data 传 NULL
* Arguments  : dac      	dac 句柄
*              buf  		数据的起始地址
*              len  		数据的字节长度
* Return	 : 成功写入的数据字节长度
* Note(s)    :
*********************************************************************
*/
int audio_dac_write(struct audio_dac_hdl *dac, void *buf, int len);

int audio_dac_get_write_ptr(struct audio_dac_hdl *dac, s16 **ptr);

int audio_dac_update_write_ptr(struct audio_dac_hdl *dac, int len);

/*
*********************************************************************
*              audio_dac_set_sample_rate
* Description: 设置 DAC 的输出采样率
* Arguments  : dac      		dac 句柄
*              sample_rate  	采样率
* Return	 : 0：成功  -1：失败
* Note(s)    :
*********************************************************************
*/
int audio_dac_set_sample_rate(struct audio_dac_hdl *dac, int sample_rate);

/*
*********************************************************************
*              audio_dac_get_sample_rate
* Description: 获取 DAC 的输出采样率
* Arguments  : dac      		dac 句柄
* Return	 : 采样率
* Note(s)    :
*********************************************************************
*/
int audio_dac_get_sample_rate(struct audio_dac_hdl *dac);

u32 audio_dac_select_sample_rate(u32 sample_rate);

int audio_dac_clk_switch(u8 clk);

int audio_dac_clk_get(void);

int audio_dac_set_channel(struct audio_dac_hdl *dac, u8 channel);

int audio_dac_get_channel(struct audio_dac_hdl *dac);

/*
*********************************************************************
*              audio_dac_set_digital_vol
* Description: 设置 DAC 的数字音量
* Arguments  : dac      	dac 句柄
*              vol  		需要设置的数字音量等级
* Return	 : 0：成功  -1：失败
* Note(s)    :
*********************************************************************
*/
int audio_dac_set_digital_vol(struct audio_dac_hdl *dac, u16 vol);

/*
*********************************************************************
*              audio_dac_set_analog_vol
* Description: 设置 DAC 的模拟音量
* Arguments  : dac      	dac 句柄
*              vol  		需要设置的模拟音量等级
* Return	 : 0：成功  -1：失败
* Note(s)    :
*********************************************************************
*/
int audio_dac_set_analog_vol(struct audio_dac_hdl *dac, u16 vol);

int audio_dac_ch_analog_gain_set(struct audio_dac_hdl *dac, u32 ch, u32 again);
/*
*********************************************************************
*              audio_dac_try_power_on
* Description: 根据设置好的参数打开 DAC 的模拟模块和数字模块。与 audio_dac_start 功能基本一样，但不设置 PNS
* Arguments  : dac      	dac 句柄
* Return	 : 0：成功  -1：失败
* Note(s)    :
*********************************************************************
*/
int audio_dac_try_power_on(struct audio_dac_hdl *dac);

/*
*********************************************************************
*              audio_dac_start
* Description: 根据设置好的参数打开 DAC 的模拟模块和数字模块。
* Arguments  : dac      	dac 句柄
* Return	 : 0：成功  -1：失败
* Note(s)    :
*********************************************************************
*/
int audio_dac_start(struct audio_dac_hdl *dac);

/*
*********************************************************************
*              audio_dac_stop
* Description: 关闭 DAC 数字部分。所有 DAC channel 都关闭后才能调用这个函数
* Arguments  : dac      	dac 句柄
* Return	 : 0：成功  -1：失败
* Note(s)    :
*********************************************************************
*/
int audio_dac_stop(struct audio_dac_hdl *dac);

/*
*********************************************************************
*              audio_dac_idle
* Description: 获取 DAC 空闲状态
* Arguments  : dac      	dac 句柄
* Return	 : 0：非空闲  1：空闲
* Note(s)    :
*********************************************************************
*/
int audio_dac_idle(struct audio_dac_hdl *dac);

void audio_dac_mute(struct audio_dac_hdl *hdl, u8 mute);

u8  audio_dac_digital_mute_state(struct audio_dac_hdl *hdl);

void audio_dac_digital_mute(struct audio_dac_hdl *dac, u8 mute);

int audio_dac_open(struct audio_dac_hdl *dac);

/*
*********************************************************************
*              audio_dac_close
* Description: 关闭 DAC 模拟部分。audio_dac_stop 之后才可以调用
* Arguments  : dac      	dac 句柄
* Return	 : 0：成功  -1：失败
* Note(s)    :
*********************************************************************
*/
int audio_dac_close(struct audio_dac_hdl *dac);

/*
*********************************************************************
*              audio_dac_set_volume
* Description: 设置音量等级记录变量，但是不会直接修改音量。只有当 DAC 关闭状态时，第一次调用 audio_dac_channel_start 打开 dac fifo 后，会根据 audio_dac_set_fade_handler 设置的回调函数来设置系统音量，回调函数的传参就是 audio_dac_set_volume 设置的音量值。
* Arguments  : dac      	dac 句柄
			   gain			记录的音量等级
* Return	 : 0：成功  -1：失败
* Note(s)    :
*********************************************************************
*/
int audio_dac_set_volume(struct audio_dac_hdl *dac, u8 gain);

int audio_dac_set_L_digital_vol(struct audio_dac_hdl *dac, u16 vol);

int audio_dac_set_R_digital_vol(struct audio_dac_hdl *dac, u16 vol);

int audio_dac_set_RL_digital_vol(struct audio_dac_hdl *dac, u16 vol);

int audio_dac_set_RR_digital_vol(struct audio_dac_hdl *dac, u16 vol);

void audio_dac_ch_high_resistance(struct audio_dac_hdl *dac, u8 ch, u8 en);

/*
*********************************************************************
*              audio_dac_ch_mute
* Description: 将某个通道静音，用于降低底噪，或者做串扰隔离的功能
* Arguments  : dac      	dac 句柄
*              ch			需要操作的通道，BIT(n)代表操作第n个通道，可以多个通道或上操作
*			   mute		    0:解除mute  1:mute
* Return	 : 0：成功  -1：失败
* Note(s)    :
*********************************************************************
*/
void audio_dac_ch_mute(struct audio_dac_hdl *dac, u8 ch, u8 mute);

/*
*********************************************************************
*              audio_dac_set_fade_handler
* Description: DAC 关闭状态时，第一次调用 audio_dac_channel_start 打开 dac fifo 后，会根据 audio_dac_set_fade_handler 设置的回调函数来设置系统音量
* Arguments  : dac      		dac 句柄
*              priv				暂无作用
*			   fade_handler 	回调函数的地址
* Return	 : 0：成功  -1：失败
* Note(s)    :
*********************************************************************
*/
void audio_dac_set_fade_handler(struct audio_dac_hdl *dac, void *priv, void (*fade_handler)(u8, u8));


int audio_dac_get_status(struct audio_dac_hdl *dac);

u8 audio_dac_is_working(struct audio_dac_hdl *dac);

int audio_dac_set_irq_time(struct audio_dac_hdl *dac, int time_ms);

int audio_dac_data_time(struct audio_dac_hdl *dac);

int audio_dac_irq_enable(struct audio_dac_hdl *dac, int time_ms, void *priv, void (*callback)(void *));

int audio_dac_set_protect_time(struct audio_dac_hdl *dac, int time, void *priv, void (*feedback)(void *));

int audio_dac_buffered_frames(struct audio_dac_hdl *dac);

void audio_dac_add_syncts_handle(struct audio_dac_hdl *dac, void *syncts);

void audio_dac_remove_syncts_handle(struct audio_dac_channel *ch, void *syncts);

void audio_dac_count_to_syncts(struct audio_dac_channel *ch, int frames);
void audio_dac_syncts_latch_trigger(struct audio_dac_channel *ch);
void audio_dac_add_syncts_with_timestamp(struct audio_dac_channel *ch, void *syncts, u32 timestamp);
void audio_dac_syncts_trigger_with_timestamp(struct audio_dac_channel *ch, u32 timestamp);
/*
 * 音频同步
 */
void *audio_dac_resample_channel(struct audio_dac_hdl *dac);

int audio_dac_sync_resample_enable(struct audio_dac_hdl *dac, void *resample);

int audio_dac_sync_resample_disable(struct audio_dac_hdl *dac, void *resample);

void audio_dac_set_input_correct_callback(struct audio_dac_hdl *dac,
        void *priv,
        void (*callback)(void *priv, int diff));

int audio_dac_set_sync_buff(struct audio_dac_hdl *dac, void *buf, int len);

int audio_dac_set_sync_filt_buff(struct audio_dac_hdl *dac, void *buf, int len);

int audio_dac_sync_open(struct audio_dac_hdl *dac);

int audio_dac_sync_set_channel(struct audio_dac_hdl *dac, u8 channel);

int audio_dac_sync_set_rate(struct audio_dac_hdl *dac, int in_rate, int out_rate);

int audio_dac_sync_auto_update_rate(struct audio_dac_hdl *dac, u8 on_off);

int audio_dac_sync_flush_data(struct audio_dac_hdl *dac);

int audio_dac_sync_fast_align(struct audio_dac_hdl *dac, int in_rate, int out_rate, int fast_output_points, float phase_diff);

#if SYNC_LOCATION_FLOAT
float audio_dac_sync_pcm_position(struct audio_dac_hdl *dac);
#else
u32 audio_dac_sync_pcm_position(struct audio_dac_hdl *dac);
#endif

int audio_dac_sync_keep_rate(struct audio_dac_hdl *dac, int points);

int audio_dac_sync_pcm_input_num(struct audio_dac_hdl *dac);

void audio_dac_sync_input_num_correct(struct audio_dac_hdl *dac, int num);

void audio_dac_set_sync_handler(struct audio_dac_hdl *dac, void *priv, int (*handler)(void *priv, u8 state));

int audio_dac_sync_start(struct audio_dac_hdl *dac);

int audio_dac_sync_stop(struct audio_dac_hdl *dac);

int audio_dac_sync_reset(struct audio_dac_hdl *dac);

int audio_dac_sync_data_lock(struct audio_dac_hdl *dac);

int audio_dac_sync_data_unlock(struct audio_dac_hdl *dac);

void audio_dac_sync_close(struct audio_dac_hdl *dac);




u32 local_audio_us_time_set(u16 time);

int local_audio_us_time(void);

int audio_dac_start_time_set(void *_dac, u32 us_timeout, u32 cur_time, u8 on_off);

u32 audio_dac_sync_pcm_total_number(void *_dac);

void audio_dac_sync_set_pcm_number(void *_dac, u32 output_points);

u32 audio_dac_pcm_total_number(void *_dac, int *pcm_r);

u8 audio_dac_sync_empty_state(void *_dac);

void audio_dac_sync_empty_reset(void *_dac, u8 state);

void audio_dac_set_empty_handler(void *_dac, void *empty_priv, void (*handler)(void *priv, u8 empty));

void audio_dac_set_dcc(u8 dcc);

u8 audio_dac_ana_gain_mapping(u8 level);

/*
*********************************************************************
*          audio_dac_volume_enhancement_mode_get
* Description: DAC 音量增强模式切换
* Arguments  : dac      dac 句柄
* Return	 : 1：音量增强模式  0：普通模式
* Note(s)    : None.
*********************************************************************
*/
u8 audio_dac_volume_enhancement_mode_get(struct audio_dac_hdl *dac);

void audio_dac_channel_start(void *private_data);
void audio_dac_channel_close(void *private_data);
int audio_dac_channel_write(void *private_data, struct audio_dac_hdl *dac, void *buf, int len);
int audio_dac_channel_set_attr(struct audio_dac_channel *ch, struct audio_dac_channel_attr *attr);
int audio_dac_new_channel(struct audio_dac_hdl *dac, struct audio_dac_channel *ch);

void audio_dac_add_update_frame_handler(struct audio_dac_hdl *dac, void (*update_frame_handler)(u8, void *, u32));
void audio_dac_del_update_frame_handler(struct audio_dac_hdl *dac);

/*AEC参考数据软回采接口*/
int audio_dac_read_reset(void);
int audio_dac_read(s16 points_offset, void *data, int len, u8 read_channel);

#endif

