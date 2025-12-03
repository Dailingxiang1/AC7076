
#include "app_config.h"
#include "jlui_app/ui_style.h"
#include "jlui/ui.h"
#include "ui/ui_api.h"
#include "app_task.h"
#include "system/timer.h"
#include "device/device.h"
#include "key_event_deal.h"
#include "res/resfile.h"
#include "jlui_app/res_config.h"
#include "jlui_app/ui_resource.h"
#include "system/includes.h"
#include "audio_config.h"
#include "asm/mcpwm.h"
#include "jlui_app/ui_sys_param.h"
#include "jlui_app/watch_syscfg_manage.h"
#include "font/language_list.h"
#include "bt_common.h"
#include "btstack/btstack_task.h"
#include "btstack/avctp_user.h"
#include "custom_cfg.h"
// #include "product_info_api.h"
#include "cat1/cat1_common.h"
#include "screen_trans/smartbox_user_app.h"
#include "ui_page_switch.h"
#include "utils/generic/ascii.h"
#include "gpu_task.h"

#define LOG_TAG_CONST       UI
#define LOG_TAG     		"[UI-ACTION]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

#if (defined(CONFIG_UI_STYLE_JL_CSC_PUBLIC_MODLS_ENABLE))
#if (defined TCFG_UI_BG_ENABLE) && TCFG_UI_BG_ENABLE

#include "ui_bg_manage.h"

#define CSBG_FILE_FIELD_STRING  "CSBG"
#if (defined TCFG_UI_USE_COMMON_BACKGROUND_SWITCH_ENBALE && TCFG_UI_USE_COMMON_BACKGROUND_SWITCH_ENABLE)
#define LOCKSCREEN_WALLPAPER_FILE_FIELD_STRING   WATCH_RES_NAME_SMALL
#endif

#if (defined TCFG_DIAL_RES_SDFILE_ENABLE && TCFG_DIAL_RES_SDFILE_ENABLE )
#define CSBG_FILE_PATH          "storage/res_fs_dev/C/"
#else
#define CSBG_FILE_PATH          "storage/virfat_flash/C/"
#endif
#define CSBG_ITEMS_MIN_LIMIT    1   /*背景最少数量*/



static struct csbg_manage_handle csbg_manage_info;
#define __this  (&csbg_manage_info)


extern  void csbg_reload_handler(void);
static int csbg_vm_init(void);


void dial_free(void *buf);

static int csbg_show_init(csbg_type_e type)
{
    u32 flag;
    char *bg_path = NULL;
    int max_type = CSBG_TYPE_ALL;
    int start_type = type;

    if (type < CSBG_TYPE_ALL && type >= CSBG_TYPE_WALLPAPER) {
        // 释放指定资源
        max_type = type + 1;
    } else if (type == CSBG_TYPE_ALL) {
        // 释放所有资源
        start_type = 0;
    } else {
        printf("<%s> type (%d)error\n", __func__, type);
        return -1;
    }
    for (int i = start_type; i < max_type ; i++) {
        if (__this->csbg_info[i].view_file) {
            // view_file 已经存在，不需要重新打开
            continue;
        }
        csbg_get_items_num(i);
        bg_path = csbg_get_cur_path(i);

        printf("<%s> bg_path:%s", __func__, bg_path);
        // log_debug("cur watch style %d, bgp_path:%s\n\n\n\n\n", watch_get_style(), bg_path);
        if (bg_path) {
            __this->csbg_info[i].view_file = res_fopen(bg_path, "r");
            if (!__this->csbg_info[i].view_file) {
                r_printf("<%s> res_fopen %s failed\n", __func__, bg_path);
                return -1;
            }
            if (UI_DATA_STORE_IN_NORFLASH) {
#if (defined TCFG_DIAL_RES_SDFILE_ENABLE && TCFG_DIAL_RES_SDFILE_ENABLE )
                ui_res_res_bgp_info_get(&__this->csbg_info[i].view_file_info, bg_path, "res", true);
#else
                ui_res_flash_info_get(&__this->csbg_info[CSBG_TYPE_WALLPAPER].view_file_info, bg_path, "res", true);
#endif
            }

            res_fread(__this->csbg_info[i].view_file, &flag, sizeof(flag));

            log_debug("flag : 0x%x\n", flag);
        }
    }
    return 0;
}


static int csbg_memory_free(csbg_type_e type)
{
    int max_type = CSBG_TYPE_ALL;
    int start_type = type;
    if (type < CSBG_TYPE_ALL && type >= CSBG_TYPE_WALLPAPER) {
        // 释放指定资源
        start_type = type;
        max_type = type + 1;
    } else if (type == CSBG_TYPE_ALL) {
        // 释放所有资源
        start_type = 0;
    } else {
        printf("<%s> type (%d)error\n", __func__, type);
        return -1;
    }
    for (int j = start_type; j < max_type ; j++) {
        for (int i = 0; i < CSBG_MAX_NUN; i++) {
            if (__this->csbg_info[j].csbg_path[i]) {
                free(__this->csbg_info[j].csbg_path[i]);
                __this->csbg_info[j].csbg_path[i] = NULL;
            }
        }
    }
    return 0;
}

static void csbg_get_res_fs_bg(void)
{
    csbg_memory_free(CSBG_TYPE_ALL);
    csbg_vm_init();
    for (int i = 0; i < CSBG_TYPE_ALL ; i++) {
        __this->csbg_info[i].csbg_items = 0;
        /*遍历已有的文件，添加到背景图片管理*/
    }
#if (defined TCFG_DIAL_RES_SDFILE_ENABLE && TCFG_DIAL_RES_SDFILE_ENABLE )
    struct sdfile_file_head head;
    u32 addr = 0;
    void *dev_hd = NULL;
    dev_hd = dev_open("res_fs_dev", NULL);
    if (!dev_hd) {
        r_printf("open res_fs_dev error\n");
        return;
    }
    for (int addr = 0; addr < 4096; addr += sizeof(struct sdfile_file_head)) {
        dev_bulk_read(dev_hd, (u8 *)&head, addr, sizeof(struct sdfile_file_head));
        // put_buf((u8*)&head,sizeof(struct sdfile_file_head));
        if (head.head_crc != CRC16((u8 *)&head + 2, sizeof(struct sdfile_file_head) - 2)) {
            continue;
        }
        if (strlen(head.name)) {
            log_info("%s name:%s index:%d", __func__, head.name);
            if ((!strncmp(head.name, CSBG_FILE_FIELD_STRING, strlen(CSBG_FILE_FIELD_STRING)))) {
                log_info(">>>>>>>>>>>>>>>>>>>>>>>%s", head.name);
                csbg_add_item(head.name, CSBG_TYPE_WALLPAPER);
                csbg_add_item(head.name, CSBG_TYPE_LOCKSCREEN_WALLPAPER);
            }
        }
    }
#else
    u32 file_num;
    virfat_flash_get_dirinfo(NULL, &file_num);

    char *fname_buf = zalloc(file_num * 12);
    if (!fname_buf) {
        printf("[%s]zalloc error,num:%d \n", __func__, file_num);
        return -1;
    }

    virfat_flash_get_dirinfo(fname_buf, &file_num);
    log_info("check file_num: %d\n", file_num);
    int j;
    for (int i = 0; i < file_num; i++) {
        char *fname = &fname_buf[i * 12];
        for (j = 0; j < 12; j++) {
            if (fname[j] == ' ' || fname[j] == '\0') {
                fname[j] = '\0';
                break;
            }
        }
        if (j == 12) {
            log_error("fname overflow\n");
            dial_free(fname_buf);
            return -1;
        }

        u8 fname_len = strlen(fname);
        ASCII_ToLower(fname, fname_len);
        log_info("[index:%d] name:%s \n", i, fname);
        if (strncmp(fname, CSBG_FILE_FIELD_STRING, strlen(CSBG_FILE_FIELD_STRING)) == 0) {
            log_info(">>>>>>>>>>>>>>>>>>>>>>>%s", fname);
            csbg_add_item(fname, type);
        }
    }
    free(fname_buf);
#endif
    /* dev_close(dev_hd); */
}


static int csbg_set_style_by_name(char *csbg_name, csbg_type_e type)
{
    int max_type = CSBG_TYPE_ALL;
    int start_type = type;
    if (type < CSBG_TYPE_ALL && type >= CSBG_TYPE_WALLPAPER) {
        // 操作指定资源
        start_type = type;
        max_type = type + 1;
    } else if (type == CSBG_TYPE_ALL) {
        // 操作所有资源
        start_type = 0;
    } else {
        printf("<%s> type (%d)error\n", __func__, type);
        return false;
    }
    if (!csbg_name) {
        return false;
    }

    u32 i;
#if (defined TCFG_DIAL_RES_SDFILE_ENABLE && TCFG_DIAL_RES_SDFILE_ENABLE )
    u32 res_path_len = strlen(CSBG_FILE_PATH);
#else
    u32 res_path_len = strlen(RES_PATH);
#endif
    u32 csbg_name_len = strlen(csbg_name);
    char *csbg_path = zalloc(res_path_len + csbg_name_len + 1);
    if (!csbg_path) {
        return false;
    }

    ASCII_ToLower(csbg_name, csbg_name_len);
#if (defined TCFG_DIAL_RES_SDFILE_ENABLE && TCFG_DIAL_RES_SDFILE_ENABLE )
    strcpy(csbg_path, CSBG_FILE_PATH);
#else
    strcpy(csbg_path, RES_PATH);
#endif
    strcpy(&csbg_path[res_path_len], (const char *)csbg_name);
    for (int j = start_type; j < max_type; j++) {

        for (i = 0; i < __this->csbg_info[j].csbg_items; i++) {
            if (strlen(csbg_path) != strlen(__this->csbg_info[j].csbg_path[i])) {
                continue;
            }

            if (strncmp(csbg_path, __this->csbg_info[j].csbg_path[i], strlen(csbg_path)) == 0) {
                csbg_set_style(i, j);
                free(csbg_path);
                return true;
            }
        }
    }
    free(csbg_path);
    return false;
}

static int csbg_add_item_vm(char *csbg_name)
{
    int ret = 0;
    ret = syscfg_write(VM_CSBG_SEQUENCE, &__this->vm_info, sizeof(__this->vm_info));
    if (sizeof(__this->vm_info) != ret) {
        log_error("<%s> vm err\n", __func__);
        return -1;
    }
    return ret;
}

static int csbg_set_style_vm(void)
{
    int ret = 0;
    ret = syscfg_write(VM_CSBG_SEQUENCE, &__this->vm_info, sizeof(__this->vm_info));
    if (sizeof(__this->vm_info) != ret) {
        log_error("<%s> vm err\n", __func__);
        return -1;
    }
    return ret;
}

static int csbg_vm_init(void)
{
    int ret;
    ret = syscfg_read(VM_CSBG_SEQUENCE, &__this->vm_info, sizeof(__this->vm_info));
    if ((ret != sizeof(__this->vm_info))) {
        log_error("<%s> csbg info invalid", __func__);
        for (int i = 0; i < CSBG_TYPE_ALL; i++) {
            __this->vm_info.csbg_cur_style[i] = 0;
        }
        return FALSE;
    }
    return TRUE;
}


static void update_background_image_by_ui(void)
{
    log_info("<%s>", __func__);
    int argv[3];
    argv[0] = (int)csbg_reload_handler;
    argv[1] = 0;
    os_taskq_post_type("ui", Q_CALLBACK, ARRAY_SIZE(argv), argv);
}


static int csbg_show_deinit(csbg_type_e type)
{
    int max_type = CSBG_TYPE_ALL;
    int start_type = type;

    if (type < CSBG_TYPE_ALL && type >= CSBG_TYPE_WALLPAPER) {
        // 释放指定资源
        start_type = type;
        max_type = type + 1;
    } else if (type == CSBG_TYPE_ALL) {
        // 释放所有资源
        start_type = 0;
    } else {
        printf("<%s> type (%d)error\n", __func__, type);
        return -1;
    }
    for (int i = start_type; i < max_type; i++) {
        if (__this->csbg_info[i].view_file) {
            res_fclose(__this->csbg_info[i].view_file);
            ui_res_flash_info_free(&__this->csbg_info[i].view_file_info, "res");
            __this->csbg_info[i].view_file = NULL;
        }
    }
    return 0;
}


int csbg_get_style(csbg_type_e type)
{
    if (type >= CSBG_TYPE_ALL) {
        printf("func:%s type err %d\n", __func__, type);
        return -1;
    }
    log_info("%s %d %d", __func__, __LINE__, __this->vm_info.csbg_cur_style[type]);
    return __this->vm_info.csbg_cur_style[type];
}

int csbg_set_style(int style, csbg_type_e type)
{
    int max_type = CSBG_TYPE_ALL;
    int start_type = type;
    if (type < CSBG_TYPE_ALL && type >= CSBG_TYPE_WALLPAPER) {
        // 操作指定资源
        start_type = type;
        max_type = type + 1;
    } else if (type == CSBG_TYPE_ALL) {
        // 操作所有资源
        start_type = 0;
    } else {
        printf("<%s> type (%d)error\n", __func__, type);
        return false;
    }
    for (int i = start_type; i < max_type; i++) {
        g_printf("%s %d %d %d ", __func__, __LINE__, style, i);
        if (style >= __this->csbg_info[i].csbg_items) {
            __this->vm_info.csbg_cur_style[i] = 0;
            return false;
        }
        __this->vm_info.csbg_cur_style[i] = style;
    }
    __this->csbg_update_flag = 1;
    csbg_set_style_vm();
    return true;
}

int app_set_csbg_by_name(char *csbg_name, csbg_type_e type)
{
    int ret;
    ret = csbg_set_style_by_name(csbg_name, type);
    if (ret == true) {
        update_background_image_by_ui();
        log_info("<%s> %s succ!", __func__, csbg_name);
        return true;
    } else {
        log_error("<%s> %s !!!", __func__, csbg_name);
        return false;
    }
}

int csbg_add_item(char *csbg_name, csbg_type_e type)
{
    int max_type = CSBG_TYPE_ALL;
    int start_type = type;

    if (type < CSBG_TYPE_ALL && type >= CSBG_TYPE_WALLPAPER) {
        // 释放指定资源
        start_type = type;
        max_type = type + 1;
    } else if (type == CSBG_TYPE_ALL) {
        // 释放所有资源
        start_type = 0;
    } else {
        printf("<%s> type (%d)error\n", __func__, type);
        return -1;
    }

    log_info("%s %s \n", __func__, csbg_name);
    char *new_item = NULL;
    u32 new_item_len = 64;
#if (defined TCFG_DIAL_RES_SDFILE_ENABLE && TCFG_DIAL_RES_SDFILE_ENABLE )
    char *root_path = CSBG_FILE_PATH;
#else
    char *root_path = RES_PATH;
#endif
    u8 csbg_name_len = strlen(csbg_name);

    if (__this->csbg_info[type].csbg_items >= CSBG_MAX_NUN) {
        log_info("<%s> csbg_items:%d >= CSBG_MAX_NUN", __func__, __this->csbg_info[type].csbg_items);
        return false;
    }

    new_item = zalloc(new_item_len);
    if (!new_item) {
        log_error("watch add item zalloc fail\n");
        return false;
    }

    ASSERT(((csbg_name_len + strlen(root_path) + 1) < new_item_len), "err name %s\n", csbg_name);

    ASCII_ToLower(csbg_name, csbg_name_len);
    strcpy(new_item, root_path);
    if (csbg_name[0] == '/') {
        strcpy(&new_item[strlen(root_path)], &csbg_name[1]);
    } else {
        strcpy(&new_item[strlen(root_path)], &csbg_name[0]);
    }

    for (u8 i = 0; i < __this->csbg_info[type].csbg_items; i++) {
        if (0 == strcmp(new_item, __this->csbg_info[type].csbg_path[i])) {
            log_info("repeat : %s, %s\n", new_item, __this->csbg_info[type].csbg_path[i]);
            dial_free(new_item);
            return false;
        }
    }

    __this->csbg_info[type].csbg_path[__this->csbg_info[type].csbg_items] = new_item;
    __this->csbg_info[type].csbg_items++;
    log_info("%s %s succ!\n", __func__, new_item);
    csbg_add_item_vm(csbg_name);

    return true;
}

int csbg_del_item(char *csbg_name, csbg_type_e type)
{
    u8 csbg_name_len = strlen(csbg_name);
    int del_succ = 0;
    int i, index;
    char *cur_csbg_path = __this->csbg_info[type].csbg_path[__this->vm_info.csbg_cur_style[type]];


    if (__this->csbg_info[type].csbg_items <= CSBG_ITEMS_MIN_LIMIT) {
        log_info("<%s> csbg_items:%d <= 1", __func__, __this->csbg_info[type].csbg_items);
        return false;
    }

    ASCII_ToLower(csbg_name, csbg_name_len);
    for (index = 0 ; index < __this->csbg_info[type].csbg_items; index++) {
        int cur_path_len = strlen(__this->csbg_info[type].csbg_path[index]);
        if ((!cur_path_len) || (cur_path_len < csbg_name_len)) {
            continue;
        }
        int path_check_fail = 0;
        for (int j = 0; j < csbg_name_len; j++) {
            char path_a = __this->csbg_info[type].csbg_path[index][cur_path_len - j - 1];
            char path_b = csbg_name[csbg_name_len - j - 1];
            if (path_a != path_b) {
                path_check_fail = 1;
                break;
            }
        }
        if (path_check_fail) {
            continue;
        } else {
            //删除
            if (__this->csbg_info[type].csbg_path[index]) {
                free(__this->csbg_info[type].csbg_path[index]);
                __this->csbg_info[type].csbg_path[index] = NULL;
                __this->csbg_info[type].csbg_items--;
                csbg_add_item_vm(csbg_name);
                del_succ = 1;
            }
            break;
        }
    }

    if (del_succ == 1) {
        /*重新排序*/
        for (i = index; i < __this->csbg_info[type].csbg_items; i++) {
            if (__this->csbg_info[type].csbg_path[i + 1] != NULL) {
                __this->csbg_info[type].csbg_path[i] = __this->csbg_info[type].csbg_path[i + 1];
                __this->csbg_info[type].csbg_path[i + 1] = NULL;
            } else {
                break;
            }
        }

        /*排序完后，切换回原来的背景图片*/
        for (i = 0; i < __this->csbg_info[type].csbg_items; i++) {
            if (cur_csbg_path == __this->csbg_info[type].csbg_path[i]) {
                log_info("<%s> finish set style %d, %s\n", __func__, i, __this->csbg_info[type].csbg_path[i]);
                csbg_set_style(i, type);
                break;
            }
        }

        /*原来的背景图片被删除，切换默认的背景图片*/
        if (i == __this->csbg_info[type].csbg_items) {
            log_info("end style\n");
            csbg_set_style(0, type);
        }
        return true;
    } else {
        return false;
    }

}

char *csbg_get_cur_path(csbg_type_e type)
{
    if (type >= CSBG_TYPE_ALL) {
        printf("func:%s type err %d\n", __func__, type);
        return NULL;
    }
    g_printf("csbg_get_num_path %s", __this->csbg_info[type].csbg_path[__this->vm_info.csbg_cur_style[type]]);
    return __this->csbg_info[type].csbg_path[__this->vm_info.csbg_cur_style[type]];
}

char *app_csbg_get_cur_csbg_name(csbg_type_e type)
{
    if (type >= CSBG_TYPE_ALL) {
        printf("func:%s type err %d\n", __func__, type);
        return NULL;
    }
    static char name[10] = {0};
    u8 len = strlen(CSBG_FILE_PATH);
    int curr_style = __this->vm_info.csbg_cur_style[type];
    char *curr_name = __this->csbg_info[type].csbg_path[curr_style] + len;
    int curr_name_len = strlen(curr_name);

    g_printf("app_csbg_get_cur_path %s", __this->csbg_info[type].csbg_path[curr_style]);
    memset(name, 0, sizeof(name));
    memcpy(name, curr_name, curr_name_len);
    return name;

}

int csbg_get_items_num(csbg_type_e type)
{
    // g_printf("csbg_get_items_num %d",__this->csbg_items);
    // return __this->csbg_items;
    if (type >= CSBG_TYPE_ALL) {
        printf("func:%s type err %d\n", __func__, type);
        return -1;
    }
    g_printf("csbg_get_items_num %d", __this->csbg_info[type].csbg_items);
    return __this->csbg_info[type].csbg_items;
}

char *csbg_get_num_path(u8 num, csbg_type_e type)
{
    if (type >= CSBG_TYPE_ALL) {
        printf("func:%s type err %d\n", __func__, type);
        return NULL;
    }
    static char name[10] = {0};
    g_printf("csbg_get_num_path %s", __this->csbg_info[type].csbg_path[num]);
    if (num > __this->csbg_info[type].csbg_items) {
        num = __this->csbg_info[type].csbg_items;
    }
    u8 len = strlen(CSBG_FILE_PATH);
    memset(name, 0, sizeof(name));
    memcpy(name, (__this->csbg_info[type].csbg_path[num] + len), strlen(__this->csbg_info[type].csbg_path[num] + len));
    return name;
}

int ui_csbg_res_check()
{
    log_info("%s \n", __func__);
    csbg_get_res_fs_bg();
    csbg_show_init(CSBG_TYPE_ALL);
    return 0;
}



int csbg_show(struct element *element, struct draw_context *dc, csbg_type_e type)
{
    if (type >= CSBG_TYPE_ALL) {
        printf("csbg_get_cur_path type err %d\n", type);
        return 0;
    }
    static int id = 1;
#if (defined TCFG_UI_COMMON_BACKGROUND_SWITCH_ENABLE && TCFG_UI_COMMON_BACKGROUND_SWITCH_ENABLE)
    if (__this->csbg_info[type].view_file) {
        element->css.background_image = 1;
        dc->preview.file = __this->csbg_info[type].view_file;
        dc->preview.file_info = &__this->csbg_info[type].view_file_info;
        dc->preview.id = id;
        dc->preview.page = 0;
    }
#else

#endif
    return 0;
}
void csbg_change_handler(csbg_type_e type)
{
    if (__this->csbg_update_flag == 1) {
        if (jlgpu_scheduler_wait_sync() == -OS_TIMEOUT) {
            log_error("<%s> error", __func__);
            return;
        }
        csbg_show_deinit(type);
        csbg_show_init(type);
        __this->csbg_update_flag = 0;
    }
}

void csbg_debug_printf(void)
{
    log_info("<%s>", __func__);
    for (int j = 0 ; j < CSBG_TYPE_ALL; j++) {
        for (int i = 0; i < ARRAY_SIZE(__this->csbg_info[j].csbg_path); i++) {
            log_info("__this->csbg_path[%d]:%s", i, __this->csbg_info[j].csbg_path[i]);
        }
    }

}



#endif
#endif

