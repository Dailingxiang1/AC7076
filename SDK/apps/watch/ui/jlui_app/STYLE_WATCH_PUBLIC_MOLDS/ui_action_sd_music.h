#pragma once
#include "asm/includes.h"
#include "system/includes.h"
#include "ui.h"
#include "ui_api.h"
#include "jlui_app/ui_style.h"
#include "jlui_app/ui_sys_param.h"


#if (defined (CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE))
#if TCFG_UI_SD_MUSIC_ENABLE
typedef struct music_sd_data {
    u8 core;
    char name[128];
    u32 timer;
} music_sd;

enum MUSIC_LAYOUT {
    LOCAL_LIST,
    MUSIC_PLAYER,
};

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

extern music_sd *music_handler;
void music_change_layout_css(u8 tmp, struct element *elm);
void music_ui_layout_switch(u8 ui_temp);
#endif /*#if TCFG_UI_SD_MUSIC_ENABLE*/
#endif /*#if (defined (CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE))*/
