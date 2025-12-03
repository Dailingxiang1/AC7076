#ifndef __UI_ACTION_SMARTWIN_H__
#define __UI_ACTION_SMARTWIN_H__

#include "asm/includes.h"
#include "system/includes.h"
#include "ui.h"
#include "ui_api.h"
#include "jlui_app/ui_style.h"
#include "jlui_app/ui_sys_param.h"

#if (defined (CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE))
#if TCFG_UI_ENABLE_SMARTWIN

/* 定义电量的事件响应 */
typedef enum {
    SMARTWIN_CHARGE_STATUS_CHARGING,       /* 正在充电 */
    SMARTWIN_CHARGE_STATUS_FULL,           /* 电量已满 */
    SMARTWIN_CHARGE_STATUS_DISCONNECT,     /* 充电器未连接 */
    SMARTWIN_CHARGE_STATUS_MAX
} SwChargeStatus;

/* 定义音乐播放的事件响应 */
typedef enum {
    SMARTWIN_MUSIC_STATUS_PLAY,     /* 播放 */
    SMARTWIN_MUSIC_STATUS_PAUSE,    /* 暂停 */
    SMARTWIN_MUSIC_STATUS_STOP,     /* 停止 */
    SMARTWIN_MUSIC_STATUS_MAX
} SwMusicStatus;

/* 定义消息的事件响应 */
typedef enum {
    SMARTWIN_MESSAGE_STATUS_ADD,        /* 播放 */
    SMARTWIN_MESSAGE_STATUS_UPDATE,     /* 暂停 */
    SMARTWIN_MESSAGE_STATUS_CLEAR,      /* 停止 */
    SMARTWIN_MESSAGE_STATUS_MAX
} SwMessageStatus;

/* 灵动岛的类型 */
typedef enum {
    SMARTWIN_TYPE_CHARGE,       /* 电量 */
    SMARTWIN_TYPE_MUSIC,        /* 音乐 */
    SMARTWIN_TYPE_NOTICE,       /* 通知 */
    SMARTWIN_TYPE_NULL,         /* 暂无状态（这个状态下，灵动岛不显示） */
    SMARTWIN_TYPE_MAX
} SmartwinType;

/* 充电状态信息 */
typedef struct {
    SwChargeStatus charge_status;            // 充电状态
    u8 battery_level;                        // 当前电量百分比
} SmartWinChargeInfo;

/* 音乐状态信息 */
typedef struct {
    SwMusicStatus music_status;             // 音乐状态
    char track_title[32];                   // 曲目名称
} SmartWinMusicInfo;

/* 消息状态信息 */
typedef struct {
    SwMessageStatus message_status;       // 当前未读消息数量
    u8 latest_type;                       // 最新消息类型（如短信、微信等）
    char summary[64];                     // 消息摘要
} SmartWinMessageInfo;

/* 灵动岛状态信息联合体 */
typedef union {
    SmartWinChargeInfo charge_info;
    SmartWinMusicInfo  music_info;
    SmartWinMessageInfo message_info;
} SmartWinTypeInfo;

SmartwinType smartwin_get_type(void);
void smartwin_set_type(SmartwinType new_type);

/* 音乐相关接口 */
u8 smartwin_get_music_state(void);
u8 smartwin_music_status_check(void);

/* 充电相关接口 */

/* 消息相关接口 */

#endif /*#if TCFG_UI_SD_MUSIC_ENABLE*/
#endif /*#if (defined (CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE))*/
#endif
