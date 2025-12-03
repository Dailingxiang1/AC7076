#include  "ui_sd_music.h"
#include  "dev_manager.h"
#include  "app_task.h"
#include  "app_music.h"
#include  "sys_app_msg.h"
#include  "key_event_deal.h"
#include "smartbox_user_app.h"

#if (defined (CONFIG_UI_STYLE_JL_CSC_PUBLIC_MODLS_ENABLE))
#if TCFG_UI_SD_MUSIC_ENABLE
#if TCFG_APP_MUSIC_EN

#define STYLE_NAME  JL

#define BS_DIR_TYPE_FORLDER   	0
#define BS_DIR_TYPE_FILE   		1

static struct grid_set_info *ghandler = NULL;
u8 file_enter_depth = 0;

extern char lyrics_artist_name_array[STR_MAX_SIZE]; /*jlui框架使用不显示内容 使用'\n'*/
#define __this 	(ghandler)
#define sizeof_this     (sizeof(struct grid_set_info))

static int close_file_handler()
{
    if (!__this) {
        return -1;
    }

    if (__this->fs) {
        fscan_release(__this->fs);
        __this->fs = NULL;
    }

    if (__this->file) {
        fclose(__this->file);
        __this->file = NULL;
    }

    if (__this->dir_buf) {
        free(__this->dir_buf);
        __this->dir_buf = NULL;
    }

    free(__this);
    __this = NULL;
    return 0;
}

static FS_DIR_INFO *file_list_read_by_index(u32 index)
{
    index = index - 1;//index 是 无符号 ,0 -1 会变成-1

    if (!__this || !__this->file) {
        return NULL;
    }
    if (index >= __this->cur_total) {
        return NULL;
    }
    index ++;
    index = index - __this->flist_index;
    int i = index % __this->show_temp;
    return &__this->dir_buf[i];
}

/*
storage/sd0/C/
*/

static int open_file_handler(int show_temp)
{
    close_file_handler();
    if (!dev_manager_get_total(1)) {// 获取有效可播放设备数量
        return -1;
    }
    struct __dev *dev = dev_manager_find_active(1);//在有效设备中获取活跃设备
    if (!dev) {
        return -1;
    }
    __this = zalloc(sizeof_this);
    __this->show_temp = show_temp + !show_temp;
    __this->dir_buf = zalloc(sizeof(FS_DIR_INFO) *  __this->show_temp);

    printf("dev_root:%s \n", dev_manager_get_root_path(dev));
    fset_ext_type(dev_manager_get_root_path(dev), DEC_EXT_NAME);	//设置后缀类型
    fopen_dir_info(dev_manager_get_root_path(dev), &__this->file, 0);//打开目录

    if (!__this->file) {
        return -1;
    }
    __this->cur_total = fenter_dir_info(__this->file, __this->dir_buf);//进入目录
    __this->flist_index = 1;//记录索引
    if (!__this->cur_total) {
        close_file_handler();
    }
    return 0;
}

static int handler_prepare_cb(void *ctrl, int count, int start)
{
    FILE *f = NULL;
    struct vfs_attr attr;

    if (!__this || !__this->file) {
        return 0;
    }

    if (!start  && (1 == __this->flist_index)) {
        //针对start 是 0情况判断是否需要更新buf，减小重刷
        return 0;
    }

    start = !start + start;//针对0的情况

    if (start > __this->cur_total) {
        return 0;
    }

    int index = start - __this->flist_index;
    int i = index % __this->show_temp;

    fget_dir_info(__this->file, start, 1, &__this->dir_buf[i]);

    return 0;
}

static int grid_child_cb(void *_ctrl, int id, int type, int index)
{
    FS_DIR_INFO *info = file_list_read_by_index(index);
    switch (type) {
    case CTRL_TYPE_PROGRESS:
        break;
    case CTRL_TYPE_MULTIPROGRESS:
        break;
    case CTRL_TYPE_TEXT:
        struct ui_text *text = (struct ui_text *)_ctrl;
        if (!strcmp(text->source, "title")) {
            text->elm.css.invisible = !!index;
            break;
        }
        if (!index) {
            text->elm.css.invisible = !index;
            break;
        }

        text->elm.css.invisible = !!index;

        if (!info) {
            return 0;
        }
        if (!strcmp(text->source, "name")) {
            text->attrs.offset = 0;
            text->attrs.format = UI_TEXT_ENCODE_TEXT;
            text->attrs.flags  = FONT_DEFAULT | FONT_SHOW_SCROLL;

            if (info->fn_type) {
                text->attrs.endian = FONT_ENDIAN_SMALL;
                text->attrs.encode = FONT_ENCODE_UNICODE;
                text->attrs.str    = info->lfn_buf.lfn;
                text->attrs.strlen = info->lfn_buf.lfn_cnt;
                /* put_buf((u8 *)info->lfn_buf.lfn,512); */
            } else {
                text->attrs.endian = 0;//FONT_ENDIAN_SMALL;
                text->attrs.encode = FONT_ENCODE_ANSI;
                text->attrs.str    = info->lfn_buf.lfn;
                /* put_buf((u8 *)info->lfn_buf.lfn,512); */
                text->attrs.strlen = strlen(text->attrs.str);
                if (text->attrs.strlen > 11) { //兼容文件系统对文件名支持支持问题
                    info->lfn_buf.lfn[12] = 0;
                    text->attrs.strlen = 11;
                }
            }
            text->elm.css.invisible = 0;
        }
        break;
    case CTRL_TYPE_NUMBER:
        struct ui_number *number = (struct ui_number *)_ctrl;
        break;
    case CTRL_TYPE_PIC:
        struct ui_pic *pic = (struct ui_pic *)_ctrl;

        if (!index) {
            pic->elm.css.invisible = !index;
            break;
        }

        pic->elm.css.invisible = !!index;

        if (!info) {
            return 0;
        }
        ui_pic_set_image_index(pic, !!(info->dir_type == BS_DIR_TYPE_FORLDER));
        pic->elm.css.invisible = 0;
        break;
    case CTRL_TYPE_TIME:
        break;
    }
    return 0;
}

static int browse_enter_child_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct element *k ;
    struct element *elm = (struct element *)_ctrl;
    if (event == ON_CHANGE_UPDATE_ITEM) {
        int index = (u32)arg;
        handler_prepare_cb(elm, 1, index + 1);
        grid_child_cb(elm, elm->id, ui_id2type(elm->id), index + 1);
    }
    return 0;
}

extern void app_audio_set_volume(u8 state, s16 volume, u8 fade);
static int file_browse_enter_onchane(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        /* app_audio_set_volume(APP_AUDIO_STATE_MUSIC, 40, 0); */
        ui_grid_set_slide_direction(grid, SCROLL_DIRECTION_UD);
        open_file_handler(grid->avail_item_num);
        int row = 0;
        int col = 1;
        if (__this) {
            ui_set_default_handler(&(grid->elm), NULL, NULL, browse_enter_child_onchange);
            row = __this->cur_total;
        }
        struct scroll_area area = {0, 0, 10000, 10000};
        ui_grid_set_scroll_area(grid, &area);
        ui_grid_flick_ctrl_close(grid, 1);
        ui_grid_init_dynamic(grid, &row, &col);
        break;
    case ON_CHANGE_RELEASE:
        file_enter_depth = 0;
        close_file_handler();
        ui_set_default_handler(&(grid->elm), NULL, NULL, NULL);
        break;
    case ON_CHANGE_FIRST_SHOW:
        break;
    default:
        return false;
    }
    return false;
}

#if TCFG_LFN_EN
static u8 music_file_name[128] = {0}; //长文件名
u16    music_file_name_len = 0;
#else
static u8 music_file_name[12 + 1] = {0}; //8.3+\0
u16    music_file_name_len = 0;
#endif

const char *music_file_get_cur_name(int *len, int *is_unicode)
{
    if (music_file_name[0] == '\\' && music_file_name[1] == 'U') {
        *is_unicode = 1 ;
        *len = music_file_name_len - 2;
        return (const char *)(music_file_name + 2);
    }
    *is_unicode = 0 ;
    *len = music_file_name_len;
    return (const char *)music_file_name;
}
static void music_name_update()
{
    ui_text_set_textu_by_id(MUSIC_TITLE_TEXT, music_handler->name, strlen(music_handler->name), FONT_DEFAULT);
}
static int file_select_enter(u32 index)
{

    int len = 0;
    int is_unicode = 0;
    FS_DIR_INFO *info = file_list_read_by_index(index);
    if (!info) {
        return -1;
    }

    if (!__this || !__this->file) {
        return -1;
    }

    if (info->dir_type == BS_DIR_TYPE_FORLDER) {
        __this->cur_total = fenter_dir_info(__this->file, info); //使用open获得的file，无需重新申请。
        file_enter_depth++;
        if (__this->cur_total == 0) {//如果是空目录直接返回上一层不做处理

            file_enter_depth--;
            __this->cur_total = fexit_dir_info(__this->file);
        }

        __this->flist_index = 1;//记录索引
    } else {
        if (app_get_curr_task() != APP_MUSIC_TASK) {
            music_task_set_parm(MUSIC_TASK_START_BY_SCLUST, info->sclust);
            extern void music_set_start_auto_play(u8 on);
            music_set_start_auto_play(1);
            app_task_switch_to(APP_MODE_MUSIC, 0);
        } else {
            /* app_task_put_key_msg(KEY_MUSIC_PLAYE_BY_DEV_SCLUST, info->sclust); */
            app_send_message(APP_MSG_MUSIC_PLAY_START_BY_SCLUST, info->sclust);
            /* app_send_message(APP_MSG_MUSIC_PLAY_START_BY_SCLUST, 0); */
        }
        music_ui_layout_switch(MUSIC_PLAYER);
        ui_pic_show_image_by_id(MUSIC_PLAY_PIC, 2);
        int len = strlen(music_handler->name);
        /* printf(">>>>>>>func %s line %d name %s len  %d \n",__func__,__LINE__,music_handler->name,len); */
        sys_timeout_add(NULL, music_name_update, 700);
        return 0;
    }

    struct ui_grid *grid = (struct ui_grid *)ui_core_get_element_by_id(MUSIC_LIST);
    struct ui_grid_dynamic *dynamic = grid->dynamic;
    dynamic->drow_num  = 0;
    int row = __this->cur_total;
    int col = 0;
    ui_grid_add_dynamic(grid, &row, &col, 1);
    return 0;
}

static int file_switch_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    int sel_item;
    static u8 move_flag = 0;
    switch (e->event) {
    case ELM_EVENT_TOUCH_R_MOVE:
    case ELM_EVENT_TOUCH_L_MOVE:
        int count = 0;
        if (file_enter_depth == 0) {//没有文件深度情况左右滑则认为返回到播放页面
            music_ui_layout_switch(MUSIC_PLAYER);
            return TRUE;
        }
        if (__this && __this->file && file_enter_depth) {
            file_enter_depth--;
            count = fexit_dir_info(__this->file);
            __this->cur_total = count;
        }
        fget_dir_info(__this->file, 1, __this->show_temp, __this->dir_buf);
        __this->flist_index = 1;//记录索引
        struct ui_grid_dynamic *dynamic = grid->dynamic;
        dynamic->drow_num  = 0;
        int row = __this->cur_total;
        int col = 0;
        ui_grid_add_dynamic(grid, &row, &col, 1);
        return TRUE;
        break;
    case ELM_EVENT_TOUCH_MOVE:
        if (!__this) {
            return TRUE;
        }
        move_flag = 1;
        return false;
        break;
    case ELM_EVENT_TOUCH_DOWN:
        move_flag = 0;
        return false;//不接管消息
        break;
    case ELM_EVENT_TOUCH_UP:
        if (move_flag) {
            move_flag = 0;
            return false;//不接管消息
        }
        sel_item = ui_grid_cur_item_dynamic(grid);
        sel_item += 1; //文件浏览索引从1开始
        if (!sel_item) {
            return false;    //不接管消息
        }
        /* app_send_message(APP_MSG_MUSIC_PLAY_BY_NUM, sel_item); */
        file_select_enter(sel_item);
        return false;//不接管消息
        break;
    default:
        return false;
        break;
    }
    return false ;//不接管
}

REGISTER_UI_EVENT_HANDLER(MUSIC_LIST)
.onchange = file_browse_enter_onchane,
 .onkey = NULL,
  .ontouch = file_switch_ontouch,
};

void music_player_get_sd_music_name(char *src)
{
    if (music_handler != NULL) {
        int len = snprintf(music_handler->name, sizeof(music_handler->name), "%s", src);
    }

}

#endif /*#if TCFG_APP_MUSIC_EN*/
#endif /*#if TCFG_UI_SD_MUSIC_ENABLE*/
#endif /*#if (defined (CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE))*/
