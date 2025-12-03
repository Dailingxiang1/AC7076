#pragma once
#include "asm/includes.h"
#include "system/includes.h"
#include "ui.h"
#include "ui_api.h"
#include "jlui_app/ui_style.h"
#include "jlui_app/ui_sys_param.h"

#define MUSIC_BT_LYRICS_LEN             256

#if (defined(CONFIG_UI_STYLE_JL_CSC_PUBLIC_MODLS_ENABLE))
typedef struct music_sd_data {
    u8 core: 3;
    u8 mode: 3;
    u8 play_sel: 2;
    u16 timer;
    char name[128];
    char bt_lyrics[MUSIC_BT_LYRICS_LEN];
} music_info;

enum MUSIC_LAYOUT_CTR {
    LOCAL_LIST,
    MUSIC_PLAYER,
};
enum MUSIC_MODE_CTR {
    PHONE_MUSIC_MODE,
    LOCAL_MODE,
};
enum MUSIC_PLAY_CTR {
    EARPHONE_PLAY,
    LOCAL_PLAY,
};
enum MUSIC_MODE_SEL {
    PHONE_MUSIC_EAR_PLAY = 0,
    PHONE_MUSIC_LOCAL_PLAY,
    LOCAL_MUSIC_LOCAL_PLAY,
    LOCAL_MUSIC_EAR_PLAY,
};
enum GRID_MUSIC_MODE_SEL {
    GRID_PHONE_MUSIC_EAR_PLAY = 0,
    GRID_PHONE_MUSIC_LOCAL_PLAY,
    GRID_LOCAL_MUSIC_EAR_PLAY,
    GRID_LOCAL_MUSIC_LOCAL_PLAY,
};



#if TCFG_UI_SD_MUSIC_ENABLE
struct grid_set_info {
    int flist_index;  //文件列表首项所指的索引
    int cur_total;
    FILE *file;
    struct vfscan *fs;
    FS_DIR_INFO *dir_buf;
    int    show_temp;
#if (TCFG_LFN_EN)
    u8  lfn_buf[512];
#endif//TCFG_LFN_EN

};

extern music_info *music_handler;
void music_change_layout_css(u8 tmp, struct element *elm);
void music_ui_layout_switch(u8 ui_temp);
#endif /*#if TCFG_UI_SD_MUSIC_ENABLE*/
#endif /*#if (defined (CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE))*/
