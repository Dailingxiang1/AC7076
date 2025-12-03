#include "app_config.h"
/* #include "app_task.h" */
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
#include "bt.h"
#include "avctp_user.h"
#include "file_player.h"
#include "app_mode_manager/app_mode_manager.h"
#include  "app_task.h"

#define LOG_TAG_CONST       UI
#define LOG_TAG     		"[UI_BT_EMITTER]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ui_action_bt_emitter.data.bss")
#pragma data_seg(".ui_action_bt_emitter.data")
#pragma const_seg(".ui_action_bt_emitter.text.const")
#pragma code_seg(".ui_action_bt_emitter.text")
#endif

#if (defined (CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE))
#if TCFG_UI_ENABLE_BT_EMITTER && TCFG_USER_EMITTER_ENABLE
#define STYLE_NAME  JL

#define BT_SEARCH_TIME_MS				100
#define BT_MAX_SEARCH_COUNT             300
#define TEXT_NAME_LEN                   32

struct BT_PAGE {
    u8 used;
    u8 mac[6];
    u8 name[32];
};

struct BT_PAGE_VM {
    int total;
    int data[0];
};

#define BT_LIST_MAX (10)

// 链表
struct bt_list_hd {
    struct list_head *head;
    int count;
};

// 记录搜索蓝牙的信息
struct bt_info {
    u8 name[32];
    u8 mac[6];
    u8 rssi;
    u8 num;
};

// 链表节点
struct bt_list {
    struct list_head entry;
    struct bt_info info;
};

struct bt_sel_item {
    u8 name[TEXT_NAME_LEN];
    u8 mac[6];
};

struct bt_emitter_t {
    u8 scan_complete;

    u32 pic_rorate_id;
    u32 pic_rotate_angle;
    u32 rotate_count;

    u16 bt_conn_timer;
    u16 bt_conn_succ_timer;

    struct bt_list_hd hd;     // 蓝牙设备链表
    struct bt_sel_item bt_sel;    // 列表选中的蓝牙设备

    struct BT_PAGE_VM *hd_vm;
};
struct bt_emitter_t *bt_emitter = NULL;


extern void bt_emitter_start_search_device();
extern void bt_search_stop();
extern u8 *get_cur_connect_emitter_mac_addr();
extern void emitter_bt_connect(u8 *mac);
extern void bt_search_device(void);
extern void delete_link_key(bd_addr_t bd_addr, u8 id);
extern struct file_player *get_music_file_player(void); //返回第一个打开的音乐播放器指针
extern int music_file_get_player_status(struct file_player *music_player);

// ========================== bt搜索api =================================
int bt_menu_list_init()
{
    if (!bt_emitter) {
        return 0;
    }

    bt_emitter->hd.head = zalloc(sizeof(struct list_head));
    bt_emitter->hd.head->prev = bt_emitter->hd.head;
    bt_emitter->hd.head->next = bt_emitter->hd.head;
    bt_emitter->hd.count = 0;

    return 0;
}


int bt_menu_list_uninit()
{
    if (!bt_emitter) {
        return 0;
    }

    if (bt_emitter->hd.head) {
        free(bt_emitter->hd.head);
        bt_emitter->hd.head = NULL;
    }
    bt_emitter->hd.count = 0;

    return 0;
}

void bt_menu_list_add(char *name, u8 *mac, u8 rssi)
{
    if (!bt_emitter) {
        return;
    }
    /* 这里过滤搜索到的蓝牙设备 */
    /* 如果mac地址已经在链表中，就不需要添加到链表里了 */
    /* 但如果新搜索到的设备有设备名称，就需更新设备名 */
    struct bt_list *p, *n;
    local_irq_disable();
    list_for_each_entry_safe(p, n, bt_emitter->hd.head, entry) {
        if (!memcmp(p->info.mac, mac, 6)) {
            log_info("get new bt device is in list, throw it:");
            put_buf(mac, 6);
            if (name) {
                log_info("device rename:%s\n", name);
                sprintf((char *)p->info.name, "%s", name);
            }
            local_irq_enable();
            return;
        }
    }
    local_irq_enable();

    struct bt_list *b_list = zalloc(sizeof(struct bt_list));
    ASSERT(b_list, "bt list malloc err");

    if (name) {
        sprintf((char *)b_list->info.name, "%s", name);
    } else {
        sprintf((char *)b_list->info.name, "%02X:%02X:%02X:%02X:%02X:%02X", mac[5], mac[4], mac[3], mac[2], mac[1], mac[0]);
    }

    memcpy(b_list->info.mac, mac, 6);
    b_list->info.num = bt_emitter->hd.count;
    b_list->info.rssi = rssi;
    local_irq_disable();
    list_add_tail(&b_list->entry, bt_emitter->hd.head);
    bt_emitter->hd.count++;
    local_irq_enable();

    UI_MSG_POST("bt_list:n=%4", b_list->info.num);
}

int bt_menu_list_count()
{
    if (!bt_emitter) {
        return 0;
    }
    return bt_emitter->hd.count;
}


void bt_list_clean()
{
    if (!bt_emitter || !(bt_emitter->hd.count)) {
        return ;
    }
    local_irq_disable();
    struct bt_list *p, *n;
    list_for_each_entry_safe(p, n, bt_emitter->hd.head, entry) {
        list_del(&p->entry);
        free(p);
    }
    bt_emitter->hd.count  = 0;
    local_irq_enable();
}

u8 *bt_list_get_name_by_number(u8 num)
{
    if (!bt_emitter || !(bt_emitter->hd.count)) {
        return NULL;
    }
    struct bt_list *p, *n;
    list_for_each_entry_safe(p, n, bt_emitter->hd.head, entry) {
        if (p->info.num == num) {
            return p->info.name;
        }

    }
    return NULL;
}

u8 *bt_list_get_mac_by_number(u8 num)
{
    if (!bt_emitter || !(bt_emitter->hd.count)) {
        return NULL;
    }
    struct bt_list *p, *n;
    list_for_each_entry_safe(p, n, bt_emitter->hd.head, entry) {
        if (p->info.num == num) {
            return p->info.mac;
        }

    }
    return NULL;
}

// ========================== bt搜索api =================================


// ========================== bt收藏api =================================
static int page_list_check(u8 *mac, u8 *name) //判断是否已经保存有
{
    if (!bt_emitter->hd_vm) {
        return -1;
    }
    struct BT_PAGE *fav = (struct BT_PAGE *)bt_emitter->hd_vm->data;
    for (int i = 0; i < BT_LIST_MAX; i++, fav++) {
        /* if (fav->used && !memcmp(mac, fav->mac, 6) && !memcmp(name, fav->name, strlen((const char *)name))) { */
        if (fav->used && !memcmp(mac, fav->mac, 6)) {
            return TRUE;
        }
    }
    return FALSE;
}

int page_list_init()
{
    int len = sizeof(struct BT_PAGE_VM) + sizeof(struct BT_PAGE) * BT_LIST_MAX;
    bt_emitter->hd_vm = (struct BT_PAGE_VM *)zalloc(len);
    struct BT_PAGE *fav = (struct BT_PAGE *)bt_emitter->hd_vm->data;
    int ret = 0;
    int count = 0;
    ret = syscfg_read(CFG_BT_PAGE_LIST, bt_emitter->hd_vm, len);
    for (int i = 0; i < BT_LIST_MAX; i++, fav++) {
        if (fav->used) {
            count++;
        }
        /* log_info("@@@@@@@@@@@@@@@ bt vm %s\n", fav->name); */
        /* put_buf(fav->mac, 6); */
    }
    if (len != ret || bt_emitter->hd_vm->total != count) {
        memset(bt_emitter->hd_vm, 0x00, len);
        syscfg_write(CFG_BT_PAGE_LIST, bt_emitter->hd_vm, len);
    }
    return 0;
}


static int page_list_find_idle()//查找idle存储
{
    if (!bt_emitter->hd_vm) {
        return -1;
    }
    struct BT_PAGE *fav = (struct BT_PAGE *)bt_emitter->hd_vm->data;
    for (int i = 0; i < BT_LIST_MAX; i++, fav++) {
        if (!fav->used) {
            return i;
        }
    }
    return -1;
}

int page_list_add(u8 *mac, u8 *name)//添加收藏
{
    int ret = 0;
    int index = 0;
    if (!bt_emitter->hd_vm) {
        return -1;
    }
    struct BT_PAGE *fav = (struct BT_PAGE *)bt_emitter->hd_vm->data;
    if (bt_emitter->hd_vm->total >= BT_LIST_MAX) {
        return -1;
    }

    ret = page_list_check(mac, name);
    if (ret) {
        return ret;
    }
    index = page_list_find_idle();
    if (index == -1) {
        return -1;
    }
    local_irq_disable();
    memcpy(fav[index].mac, mac, 6);
    sprintf((char *)fav[index].name, "%s", name);
    fav[index].used = 1;
    bt_emitter->hd_vm->total++;
    local_irq_enable();
    log_info("page_list_add success \n");

    syscfg_write(CFG_BT_PAGE_LIST, bt_emitter->hd_vm, sizeof(struct BT_PAGE_VM) + sizeof(struct BT_PAGE)*BT_LIST_MAX); //保存vm
    return 0;
}


u8 *page_list_read_name(u8 *mac, int index) //index从1开始,获取收藏的蓝牙mac
{
    if (!bt_emitter->hd_vm) {
        return 0;
    }
    int find = 1;
    u8 *name = NULL;
    struct BT_PAGE *fav = (struct BT_PAGE *)bt_emitter->hd_vm->data;
    if (!index || index > BT_LIST_MAX) {
        return NULL;
    }

    for (int i = 0; i < BT_LIST_MAX; i++, fav++) {
        if (fav->used) {
            if (find++ == index) {
                if (mac) {
                    memcpy(mac, fav->mac, 6);
                }
                name = fav->name;
                return name;
            }
        }
    }
    return NULL;
}

int page_list_del(int index)//index从1开始,删除收藏
{
    if (!bt_emitter->hd_vm) {
        return -1;
    }

    int find = 1;
    struct BT_PAGE *fav = (struct BT_PAGE *)bt_emitter->hd_vm->data;
    if (index > BT_LIST_MAX) {
        return -1;
    }


    local_irq_disable();
    for (int i = 0; i < BT_LIST_MAX; i++, fav++) {
        if (fav->used) {
            if (find++ == index) {
                fav->used = 0;
                bt_emitter->hd_vm->total--;
                local_irq_enable();
                return 0;
            }
        }
    }
    local_irq_enable();
    return -1;
}

int page_list_del_by_mac(u8 *mac)
{
    if (!bt_emitter->hd_vm || !mac) {
        return -1;
    }

    struct BT_PAGE *fav = (struct BT_PAGE *)bt_emitter->hd_vm->data;
    local_irq_disable();
    for (int i = 0; i < BT_LIST_MAX; i++, fav++) {
        if (fav->used) {
            if (!memcmp(mac, fav->mac, 6)) {
                fav->used = 0;
                bt_emitter->hd_vm->total--;
                local_irq_enable();
                syscfg_write(CFG_BT_PAGE_LIST, bt_emitter->hd_vm, sizeof(struct BT_PAGE_VM) + sizeof(struct BT_PAGE)*BT_LIST_MAX); //保存vm
                return 0;
            }
        }
    }
    local_irq_enable();
    return -1;
}


int page_list_get_count()//获得收藏数
{
    if (!bt_emitter->hd_vm) {
        return 0;
    }

    return  bt_emitter->hd_vm->total;
}
// ========================== bt收藏api =================================
void bt_conn_succ_show(void *priv)
{
    ui_pic_show_image_by_id(BT_EMITTER_PAIR_PIC, 1);
    ui_text_show_index_by_id(BT_EMITTER_PAIR_TEXT, 1);

    ui_show(BT_EMITTER_PAIR_BTN);

    if (bt_emitter->bt_conn_succ_timer) {
        sys_timeout_del(bt_emitter->bt_conn_succ_timer);
        bt_emitter->bt_conn_succ_timer = 0;
    }
}

static int bt_emitter_status_handler(const char *type, u32 arg)
{
    if (type && (!strcmp(type, "hci_event"))) {
        switch (arg) {
        case HCI_EVENT_INQUIRY_COMPLETE:
            bt_emitter->scan_complete = 1;
            if (ui_core_get_disp_status_by_id(BT_EMITTER_PAIR_LAYOUT) == TRUE) {
                break;
            }
            if (ui_core_get_disp_status_by_id(BT_EMITTER_SEARCHING_LAYOUT) == TRUE) {
                ui_hide(BT_EMITTER_SEARCHING_LAYOUT);
                ui_show(BT_EMITTER_SEARCH_RES_LAYOUT);
            }
            ui_show(BT_EMITTER_RETRY_BUTTON);
            break;
        /* case HCI_EVENT_CONNECTION_COMPLETE:  */
        /*     ui_pic_show_image_by_id(BT_EMITTER_PAIR_PIC, 1); */
        /*     ui_text_show_index_by_id(BT_EMITTER_PAIR_TEXT, 1); */
        /*  */
        /*     ui_show(BT_EMITTER_PAIR_BTN); */
        /*     break; */
        case HCI_EVENT_DISCONNECTION_COMPLETE:
            if (ui_core_get_disp_status_by_id(BT_EMITTER_VM_LAYOUT) == TRUE) {
                u8 *current_mac = get_cur_connect_emitter_mac_addr();
                if (current_mac) {
                    ui_grid_update_by_id_dynamic(BT_EMITTER_VM_VLIST, 0, 1);
                }
            }
            if (ui_core_get_disp_status_by_id(BT_EMITTER_PAIR_LAYOUT) == TRUE) {
                ui_pic_show_image_by_id(BT_EMITTER_PAIR_PIC, 2);
                ui_text_show_index_by_id(BT_EMITTER_PAIR_TEXT, 2);
                ui_show(BT_EMITTER_PAIR_BTN);
                if (bt_emitter && bt_emitter->bt_conn_timer) {
                    sys_timeout_del(bt_emitter->bt_conn_timer);
                    bt_emitter->bt_conn_timer = 0;
                }
            }
            break;
        }
    } else if (type && (!strcmp(type, "hci_value"))) {
        switch (arg) {
        case ERROR_CODE_SUCCESS:
            if (!bt_emitter->bt_conn_succ_timer) {
                bt_emitter->bt_conn_succ_timer = sys_timeout_add(NULL, bt_conn_succ_show, 1000);
            }
            break;
        case ERROR_CODE_PAGE_TIMEOUT:
        case ERROR_CODE_AUTHENTICATION_FAILURE:
        case ERROR_CODE_PIN_OR_KEY_MISSING:
        case ERROR_CODE_CONNECTION_TIMEOUT:
        case CUSTOM_BB_AUTO_CANCEL_PAGE:
        case BB_CANCEL_PAGE:
            ui_pic_show_image_by_id(BT_EMITTER_PAIR_PIC, 2);
            ui_text_show_index_by_id(BT_EMITTER_PAIR_TEXT, 2);
            ui_show(BT_EMITTER_PAIR_BTN);
            break;
        }

        if (bt_emitter && bt_emitter->bt_conn_timer) {
            sys_timeout_del(bt_emitter->bt_conn_timer);
            bt_emitter->bt_conn_timer = 0;
        }
    }
    return 0;
}


static int bt_emitter_list_handler(const char *type, u32 arg)
{
    if (bt_menu_list_count() == 1) {
        if (ui_core_get_disp_status_by_id(BT_EMITTER_SEARCHING_LAYOUT) == TRUE) {
            ui_hide(BT_EMITTER_SEARCHING_LAYOUT);
            ui_show(BT_EMITTER_SEARCH_RES_LAYOUT);
        }
        /* int row = 1; */
        /* int col = 0; */
        /* ui_grid_add_dynamic_by_id(BT_EMITTER_VLIST, &row, &col, true); */
    } else if (bt_menu_list_count() > 1) {
        if (ui_core_get_disp_status_by_id(BT_EMITTER_VLIST) == TRUE) {
            int row = 1;
            int col = 0;
            ui_grid_add_dynamic_by_id(BT_EMITTER_VLIST, &row, &col, true);
        } else if (ui_core_get_disp_status_by_id(BT_EMITTER_SEARCHING_LAYOUT) == TRUE) {
            int row = bt_menu_list_count();
            int col = 0;
            ui_grid_add_dynamic_by_id(BT_EMITTER_VLIST, &row, &col, true);
        }
    }
    return 0;
}


static const struct uimsg_handl bt_emitter_msg_handler[] = {
    { "bt_emitter_status",  bt_emitter_status_handler     }, /* 蓝牙状态 */
    { "bt_list",            bt_emitter_list_handler     }, /* 更新蓝牙列表 */
    { NULL, NULL},      /* 必须以此结尾！ */
};

static int bt_emitter_page_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct window *window = (struct window *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        if (!bt_emitter) {
            bt_emitter = zalloc(sizeof(struct bt_emitter_t));
        }

        page_list_init();
        bt_menu_list_init();

        // 先判断有无vm保存蓝牙
        if (page_list_get_count() > 0) {
            ui_show(BT_EMITTER_VM_LAYOUT);
        } else {
            ui_show(BT_EMITTER_SEARCHING_LAYOUT);
        }

        ui_register_msg_handler(window->elm.id, bt_emitter_msg_handler);//注册消息交互的回调
        ui_auto_shut_down_disable();

        break;
    case ON_CHANGE_RELEASE:
        bt_search_stop();
        bt_menu_list_uninit();
        if (bt_emitter && bt_emitter->pic_rorate_id) {
            sys_timer_del(bt_emitter->pic_rorate_id);
            bt_emitter->pic_rorate_id = 0;
        }
        if (bt_emitter) {
            free(bt_emitter);
            bt_emitter = NULL;
        }
        ui_auto_shut_down_enable();
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ID_WINDOW_BT_EMITTER)
.onchange = bt_emitter_page_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};


static void bt_emitter_pic_rotate(void *priv)
{
    struct ui_pic *pic = ui_pic_for_id(BT_EMITTER_SEARCHING_PIC);
    if (pic) {
        ui_core_set_element_rotate(pic, 34, 34, 156, 182, 18 * bt_emitter->pic_rotate_angle++, true);
        ui_core_redraw(pic);
    }
    if (bt_emitter->rotate_count > BT_MAX_SEARCH_COUNT) {    // 最长搜索时间：(BT_MAX_SEARCH_COUNT*BT_SEARCH_TIME_MS)ms
        bt_search_stop();
        ui_hide(BT_EMITTER_SEARCHING_LAYOUT);
        ui_show(BT_EMITTER_SEARCH_RES_LAYOUT);
    }
    bt_emitter->rotate_count++;
}

// 搜索中图标
static int bt_emitter_searching_pic_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_pic *pic = (struct ui_pic *)_ctrl;
    switch (event) {
    case ON_CHANGE_INIT:
        if (!bt_emitter->pic_rorate_id) {
            bt_emitter->rotate_count = 0;
            bt_emitter->pic_rorate_id = sys_timer_add(NULL, bt_emitter_pic_rotate, BT_SEARCH_TIME_MS);
        }
        bt_emitter_start_search_device();
        break;
    case ON_CHANGE_RELEASE:
        if (bt_emitter && bt_emitter->pic_rorate_id) {
            sys_timer_del(bt_emitter->pic_rorate_id);
            bt_emitter->pic_rorate_id = 0;
        }
        break;
    default:
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(BT_EMITTER_SEARCHING_PIC)
.onchange = bt_emitter_searching_pic_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int bt_emitter_search_res_layout_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    switch (event) {
    case ON_CHANGE_INIT:
        ui_auto_shut_down_disable();
        break;
    case ON_CHANGE_RELEASE:
        ui_auto_shut_down_enable();

        break;
    default:
        break;
    }
    return 0;
}

REGISTER_UI_EVENT_HANDLER(BT_EMITTER_SEARCH_RES_LAYOUT)
.onchange = bt_emitter_search_res_layout_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int bt_emitter_searching_layout_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    switch (event) {
    case ON_CHANGE_INIT:
        bt_emitter->scan_complete = 0;
        ui_auto_shut_down_disable();
        break;
    case ON_CHANGE_RELEASE:
        ui_auto_shut_down_enable();

        break;
    default:
        break;
    }
    return 0;
}
REGISTER_UI_EVENT_HANDLER(BT_EMITTER_SEARCHING_LAYOUT)
.onchange = bt_emitter_searching_layout_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

// 重新搜索按键
static int bt_emitter_retry_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;
    static u8 touch_action = 0;

    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        if (touch_action != 1) {
            break;
        }

        switch (pic->elm.id) {
        case BT_EMITTER_RETRY_BUTTON:
            // 搜索前断开已经连接的蓝牙耳机
            u8 *current_mac = get_cur_connect_emitter_mac_addr();
            if (current_mac) {
                bt_emitter_cmd_prepare(USER_CTRL_DISCONNECTION_HCI, 0, NULL);
            }

            bt_emitter_start_search_device();     // 重新发起搜索

            bt_list_clean();

            ui_hide(BT_EMITTER_SEARCH_RES_LAYOUT);
            ui_show(BT_EMITTER_SEARCHING_LAYOUT);
            break;
        default:
            return false;
        }

        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        touch_action = 2;
        break;
    case ELM_EVENT_TOUCH_DOWN:
        touch_action = 1;
        return true;
    case ELM_EVENT_TOUCH_U_MOVE:
        break;
    case ELM_EVENT_TOUCH_D_MOVE:
        break;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(BT_EMITTER_RETRY_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = bt_emitter_retry_ontouch,
};

// 蓝牙列表显示
static int list_bt_name_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_text *text = (struct ui_text *)_ctrl;
    int index;
    int count = bt_menu_list_count();

    switch (event) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_HIGHLIGHT:
        break;
    case ON_CHANGE_UPDATE_ITEM:

        index = (u32)arg;
        if (index > count) {
            break;
        }
        switch (text->elm.id) {
        case BT_EMITTER_NAME0:
            break;
        case BT_EMITTER_NAME1:
            break;
        case BT_EMITTER_NAME2:
            break;
        case BT_EMITTER_NAME3:
            break;
        default:
            return FALSE;
        }

        u8 *name = bt_list_get_name_by_number(index);
        log_info("name[%d] = %s\n", index, name);
        if (name) {
            ui_text_set_utf8_str(text, UI_TEXT_ENCODE_TEXT, (char *)name, strlen((char *)name), FONT_DEFAULT);
        }
        break;

    default:
        return false;
    }
    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(BT_EMITTER_NAME0)
.onchange = list_bt_name_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(BT_EMITTER_NAME1)
.onchange = list_bt_name_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(BT_EMITTER_NAME2)
.onchange = list_bt_name_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(BT_EMITTER_NAME3)
.onchange = list_bt_name_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

// 蓝牙动态列表
static int bt_emitter_vlist_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        int row = bt_menu_list_count();
        int col = 1;
        ui_grid_init_dynamic(grid, &row, &col);
        ui_grid_set_slide_direction(grid, SCROLL_DIRECTION_UD);
        break;
    case ON_CHANGE_RELEASE:
        if (bt_emitter && !bt_emitter->scan_complete) {
            bt_search_stop();
        }
        break;
    case ON_CHANGE_FIRST_SHOW:
        break;
    default:
        return false;
    }
    return false;
}

static int bt_emitter_vlist_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    int sel_item = 100;
    static u8 move_flag = 0;

    switch (e->event) {
    case ELM_EVENT_TOUCH_MOVE:
        move_flag = 1;
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
        u8 *mac = bt_list_get_mac_by_number(sel_item);
        if (mac) {
            log_info("bt connet start mac:\n");
            put_buf(mac, 6);
            memcpy(bt_emitter->bt_sel.mac, mac, 6);
            sprintf((char *)bt_emitter->bt_sel.name, "%s", bt_list_get_name_by_number(sel_item));

            ui_hide(BT_EMITTER_SEARCH_RES_LAYOUT);
            ui_show(BT_EMITTER_CONN_LAYOUT);
        }
        return false;//不接管消息
        break;
    default:
        return false;//不接管消息
        break;
    }

    return false;//true;//接管消息
}

REGISTER_UI_EVENT_HANDLER(BT_EMITTER_VLIST)
.onchange = bt_emitter_vlist_onchange,
 .onkey = NULL,
  .ontouch = bt_emitter_vlist_ontouch,
};


static void bt_conn_timer_handler(void *priv)
{
    ui_pic_show_image_by_id(BT_EMITTER_PAIR_PIC, 2);
    ui_text_show_index_by_id(BT_EMITTER_PAIR_TEXT, 2);
    ui_show(BT_EMITTER_PAIR_BTN);

    if (bt_emitter && bt_emitter->bt_conn_timer) {
        sys_timeout_del(bt_emitter->bt_conn_timer);
        bt_emitter->bt_conn_timer = 0;
    }
}


static int bt_emitter_sel_name_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_text *text = (struct ui_text *)_ctrl;

    switch (event) {
    case ON_CHANGE_INIT:
        ui_text_set_utf8_str(text, UI_TEXT_ENCODE_TEXT, (char *)bt_emitter->bt_sel.name, strlen((char *)bt_emitter->bt_sel.name), FONT_DEFAULT);
        break;

    default:
        return false;
    }
    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(BT_EMITTER_CONN_NAME)
.onchange = bt_emitter_sel_name_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int bt_emitter_conn_ask_layout_ontouch(void *ctr, struct element_touch_event *e)
{
    struct layout *layout = (struct layout *)ctr;

    switch (e->event) {
    case ELM_EVENT_TOUCH_R_MOVE:
        if (page_list_get_count() > 0) {
            ui_hide(BT_EMITTER_CONN_LAYOUT);
            ui_show(BT_EMITTER_VM_LAYOUT);
        } else {
            ui_hide(BT_EMITTER_CONN_LAYOUT);
            ui_show(BT_EMITTER_SEARCH_RES_LAYOUT);
        }
        return true;
        break;
    default:
        break;
    }

    return false;
}

REGISTER_UI_EVENT_HANDLER(BT_EMITTER_CONN_LAYOUT)//通用-垂直列表
.onchange = NULL,
 .onkey = NULL,
  .ontouch = bt_emitter_conn_ask_layout_ontouch,
};

static int bt_emitter_conn_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;
    static u8 touch_action = 0;

    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        if (touch_action != 1) {
            break;
        }

        switch (pic->elm.id) {
        case BT_EMITTER_CONN_NO_BTN:

            if (page_list_get_count() > 0) {
                ui_hide(BT_EMITTER_CONN_LAYOUT);
                ui_show(BT_EMITTER_VM_LAYOUT);
            } else {
                u8 *current_mac = get_cur_connect_emitter_mac_addr();
                if (current_mac) {
                    bt_emitter_cmd_prepare(USER_CTRL_DISCONNECTION_HCI, 0, NULL);
                }

                bt_emitter_start_search_device();     // 重新发起搜索

                bt_list_clean();

                ui_hide(BT_EMITTER_CONN_LAYOUT);
                ui_show(BT_EMITTER_SEARCHING_LAYOUT);
            }

            break;
        case BT_EMITTER_CONN_YES_BTN:
            struct file_player *file_player = NULL;     // 若连接蓝牙发射，先暂停本地音乐播放
            file_player = get_music_file_player();
            if (music_file_get_player_status(file_player) == FILE_PLAYER_START) {
                app_send_message(APP_MSG_MUSIC_PP, 0);
            }

            struct app_mode *cur_mode;
            cur_mode = app_get_current_mode();
            if (cur_mode->name == APP_MODE_MUSIC) {
                app_task_switch_back();
            }

            emitter_bt_connect(bt_emitter->bt_sel.mac);

            ui_hide(BT_EMITTER_CONN_LAYOUT);
            ui_show(BT_EMITTER_PAIR_LAYOUT);


            page_list_add((u8 *)bt_emitter->bt_sel.mac, (u8 *)bt_emitter->bt_sel.name);
            break;
        default:
            return false;
        }

        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        touch_action = 2;
        break;
    case ELM_EVENT_TOUCH_DOWN:
        touch_action = 1;
        return true;
    case ELM_EVENT_TOUCH_U_MOVE:
        break;
    case ELM_EVENT_TOUCH_D_MOVE:
        break;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(BT_EMITTER_CONN_NO_BTN)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = bt_emitter_conn_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BT_EMITTER_CONN_YES_BTN)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = bt_emitter_conn_ontouch,
};

static int bt_emitter_pair_layout_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    switch (event) {
    case ON_CHANGE_INIT:
        ui_auto_shut_down_disable();
        if (bt_emitter && !bt_emitter->bt_conn_timer) {
            bt_emitter->bt_conn_timer = sys_timeout_add(NULL, bt_conn_timer_handler, 15000);
        }
        break;
    case ON_CHANGE_RELEASE:
        ui_auto_shut_down_enable();

        break;
    default:
        break;
    }
    return 0;
}
static int bt_emitter_pair_layout_ontouch(void *ctr, struct element_touch_event *e)
{
    struct layout *layout = (struct layout *)ctr;

    switch (e->event) {
    case ELM_EVENT_TOUCH_R_MOVE:
        if (page_list_get_count() > 0) {
            ui_hide(BT_EMITTER_PAIR_LAYOUT);
            ui_show(BT_EMITTER_VM_LAYOUT);
        } else {
            ui_hide(BT_EMITTER_PAIR_LAYOUT);
            ui_show(BT_EMITTER_SEARCH_RES_LAYOUT);
        }
        return true;
        break;
    default:
        break;
    }

    return false;
}

REGISTER_UI_EVENT_HANDLER(BT_EMITTER_PAIR_LAYOUT)//通用-垂直列表
.onchange = bt_emitter_pair_layout_onchange,
 .onkey = NULL,
  .ontouch = bt_emitter_pair_layout_ontouch,
};

static int bt_emitter_conn_sure_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;
    static u8 touch_action = 0;

    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        if (touch_action != 1) {
            break;
        }

        switch (pic->elm.id) {
        case BT_EMITTER_PAIR_BTN:
            ui_hide(BT_EMITTER_PAIR_LAYOUT);
            ui_show(BT_EMITTER_VM_LAYOUT);
            break;
        default:
            return false;
        }

        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        touch_action = 2;
        break;
    case ELM_EVENT_TOUCH_DOWN:
        touch_action = 1;
        return true;
    case ELM_EVENT_TOUCH_U_MOVE:
        break;
    case ELM_EVENT_TOUCH_D_MOVE:
        break;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(BT_EMITTER_PAIR_BTN)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = bt_emitter_conn_sure_ontouch,
};

// ========================== bt收藏api =================================
// 蓝牙列表显示
static int list_bt_vm_name_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_text *text = (struct ui_text *)_ctrl;
    int index;

    switch (event) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_HIGHLIGHT:
        break;
    case ON_CHANGE_UPDATE_ITEM:

        index = (u32)arg;
        if (index > BT_LIST_MAX + 1) {
            break;
        }
        switch (text->elm.id) {
        case BT_EMITTER_VM_NAME0:
            break;
        case BT_EMITTER_VM_NAME1:
            break;
        case BT_EMITTER_VM_NAME2:
            break;
        case BT_EMITTER_VM_NAME3:
            break;
        default:
            return FALSE;
        }
        u8 mac[6];
        u8 *name = page_list_read_name(mac, index + 1);
        log_info("name[%d] = %s\n", index, name);
        ui_text_set_utf8_str(text, UI_TEXT_ENCODE_TEXT, (char *)name, strlen((char *)name), FONT_DEFAULT);
        break;

    default:
        return false;
    }
    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(BT_EMITTER_VM_NAME0)
.onchange = list_bt_vm_name_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(BT_EMITTER_VM_NAME1)
.onchange = list_bt_vm_name_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(BT_EMITTER_VM_NAME2)
.onchange = list_bt_vm_name_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(BT_EMITTER_VM_NAME3)
.onchange = list_bt_vm_name_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};


static int list_bt_vm_status_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_text *text = (struct ui_text *)_ctrl;
    int index;

    switch (event) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_HIGHLIGHT:
        break;
    case ON_CHANGE_UPDATE_ITEM:

        index = (u32)arg;
        if (index > BT_LIST_MAX + 1) {
            break;
        }
        switch (text->elm.id) {
        case BT_EMITTER_VM_SATUS0:
            break;
        case BT_EMITTER_VM_SATUS1:
            break;
        case BT_EMITTER_VM_SATUS2:
            break;
        case BT_EMITTER_VM_SATUS3:
            break;
        default:
            return FALSE;
        }
        u8 mac[6] = {0};
        u8 *name = page_list_read_name(mac, index + 1);  //index从1开始,获取收藏的蓝牙mac
        u8 *current_mac = get_cur_connect_emitter_mac_addr();
        if (current_mac && !memcmp(current_mac, mac, 6)) {
            ui_text_set_index(text, 0);
        } else {
            ui_text_set_index(text, 1);
        }
        break;

    default:
        return false;
    }
    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(BT_EMITTER_VM_SATUS0)
.onchange = list_bt_vm_status_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(BT_EMITTER_VM_SATUS1)
.onchange = list_bt_vm_status_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(BT_EMITTER_VM_SATUS2)
.onchange = list_bt_vm_status_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(BT_EMITTER_VM_SATUS3)
.onchange = list_bt_vm_status_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

// 蓝牙vm动态列表
static int bt_emitter_vm_vlist_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        int row = page_list_get_count();
        int col = 1;
        ui_grid_init_dynamic(grid, &row, &col);
        ui_grid_set_slide_direction(grid, SCROLL_DIRECTION_UD);
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        break;
    default:
        return false;
    }
    return false;
}

static int bt_emitter_vm_vlist_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    int sel_item = 100;
    static u8 move_flag = 0;
    u8 is_item_zero = 0;

    if (page_list_get_count() == 0) {
        is_item_zero = 1;
    }

    switch (e->event) {
    case ELM_EVENT_TOUCH_MOVE:
        if (is_item_zero) {
            return true;
        }
        move_flag = 1;
        break;
    case ELM_EVENT_TOUCH_DOWN:
        if (is_item_zero) {
            return true;
        }
        move_flag = 0;
        return false;//不接管消息
        break;
    case ELM_EVENT_TOUCH_UP:
        if (is_item_zero) {
            return true;
        }
        if (move_flag) {
            move_flag = 0;
            return false;//不接管消息
        }

        sel_item = ui_grid_cur_item_dynamic(grid);
        u8 mac[6];
        u8 *name = page_list_read_name(mac, sel_item + 1);  //index从1开始,获取收藏的蓝牙mac
        if (name) {
            memcpy(bt_emitter->bt_sel.mac, mac, 6);
            sprintf((char *)bt_emitter->bt_sel.name, "%s", name);
        }

        ui_hide(BT_EMITTER_VM_LAYOUT);
        ui_show(BT_EMITTER_CURR_BT_LAYOUT);
        return false;//不接管消息
        break;
    case ELM_EVENT_TOUCH_L_MOVE:
    case ELM_EVENT_TOUCH_D_MOVE:
    case ELM_EVENT_TOUCH_U_MOVE:
    case ELM_EVENT_TOUCH_HOLD:
    case ELM_EVENT_TOUCH_ENERGY:
    case ELM_EVENT_TOUCH_STOP:
        if (is_item_zero) {
            return true;
        }
        break;
    case ELM_EVENT_TOUCH_R_MOVE:
        if (is_item_zero) {
            return false;
        }
        break;

    default:
        return false;//不接管消息
        break;
    }

    return false;//true;//接管消息
}

REGISTER_UI_EVENT_HANDLER(BT_EMITTER_VM_VLIST)
.onchange = bt_emitter_vm_vlist_onchange,
 .onkey = NULL,
  .ontouch = bt_emitter_vm_vlist_ontouch,
};

static int bt_emitter_vm_search_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;
    static u8 touch_action = 0;

    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        if (touch_action != 1) {
            break;
        }

        switch (pic->elm.id) {
        case BT_EMITTER_VM_SEARCH_BUTTON:
            // 搜索前断开已经连接的蓝牙耳机
            u8 *current_mac = get_cur_connect_emitter_mac_addr();
            if (current_mac) {
                bt_emitter_cmd_prepare(USER_CTRL_DISCONNECTION_HCI, 0, NULL);
            }

            bt_emitter_start_search_device();     // 重新发起搜索

            bt_list_clean();

            ui_hide(BT_EMITTER_VM_LAYOUT);
            ui_show(BT_EMITTER_SEARCHING_LAYOUT);
            break;
        default:
            return false;
        }

        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        touch_action = 2;
        break;
    case ELM_EVENT_TOUCH_DOWN:
        touch_action = 1;
        return true;
    case ELM_EVENT_TOUCH_U_MOVE:
        break;
    case ELM_EVENT_TOUCH_D_MOVE:
        break;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(BT_EMITTER_VM_SEARCH_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = bt_emitter_vm_search_ontouch,
};

static int bt_emitter_curr_name_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_text *text = (struct ui_text *)_ctrl;

    switch (event) {
    case ON_CHANGE_INIT:
        ui_text_set_utf8_str(text, UI_TEXT_ENCODE_TEXT, (char *)bt_emitter->bt_sel.name, strlen((char *)bt_emitter->bt_sel.name), FONT_DEFAULT);
        break;

    default:
        return false;
    }
    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(BT_EMITTER_CURR_BT_NAME)
.onchange = bt_emitter_curr_name_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int bt_emitter_curr_status_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_text *text = (struct ui_text *)_ctrl;

    switch (event) {
    case ON_CHANGE_INIT:
        u8 *current_mac = get_cur_connect_emitter_mac_addr();
        if (current_mac && (!memcmp(current_mac, bt_emitter->bt_sel.mac, 6))) {
            ui_text_set_index(text, 0);
        } else {
            ui_text_set_index(text, 1);
        }
        break;

    default:
        return false;
    }
    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(BT_EMITTER_CURR_BT_STATUS)
.onchange = bt_emitter_curr_status_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int bt_emitter_curr_bt_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_text *text = (struct ui_text *)_ctrl;

    switch (event) {
    case ON_CHANGE_INIT:
        u8 *current_mac = get_cur_connect_emitter_mac_addr();
        put_buf(current_mac, 6);
        if (current_mac && (!memcmp(current_mac, bt_emitter->bt_sel.mac, 6))) {
            ui_text_set_index(text, 0);
        } else {
            ui_text_set_index(text, 1);
        }
        break;

    default:
        return false;
    }
    return FALSE;
}

static int bt_emitter_curr_bt_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;
    static u8 touch_action = 0;
    u8 *current_mac;

    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        if (touch_action != 1) {
            break;
        }

        switch (pic->elm.id) {
        case BT_EMITTER_CURR_BT_CONN_BTN:
            // 搜索前断开已经连接的蓝牙耳机
            current_mac = get_cur_connect_emitter_mac_addr();
            put_buf(current_mac, 6);
            put_buf(bt_emitter->bt_sel.mac, 6);
            if (current_mac && (!memcmp(current_mac, bt_emitter->bt_sel.mac, 6))) {

                struct file_player *file_player = NULL;     // 若断开蓝牙发射，先暂停本地音乐播放
                file_player = get_music_file_player();
                if (music_file_get_player_status(file_player) == FILE_PLAYER_START) {
                    app_send_message(APP_MSG_MUSIC_PP, 0);
                }

                struct app_mode *cur_mode;
                cur_mode = app_get_current_mode();
                if (cur_mode->name == APP_MODE_MUSIC) {
                    app_task_switch_back();
                }

                ui_hide(BT_EMITTER_CURR_BT_LAYOUT);
                ui_show(BT_EMITTER_VM_LAYOUT);
                bt_emitter_cmd_prepare(USER_CTRL_DISCONNECTION_HCI, 0, NULL);
            } else {
                ui_hide(BT_EMITTER_CURR_BT_LAYOUT);
                ui_show(BT_EMITTER_CONN_LAYOUT);
            }

            break;
        case BT_EMITTER_CURR_BT_UNPAIR_BTN:
            current_mac = get_cur_connect_emitter_mac_addr();
            if (current_mac && (!memcmp(bt_emitter->bt_sel.mac, current_mac, 6))) {

                struct file_player *file_player = NULL;     // 若断开蓝牙发射，先暂停本地音乐播放
                file_player = get_music_file_player();
                if (music_file_get_player_status(file_player) == FILE_PLAYER_START) {
                    app_send_message(APP_MSG_MUSIC_PP, 0);
                }

                struct app_mode *cur_mode;
                cur_mode = app_get_current_mode();
                if (cur_mode->name == APP_MODE_MUSIC) {
                    app_task_switch_back();              // 重新切回上一模式
                }

                bt_emitter_cmd_prepare(USER_CTRL_DISCONNECTION_HCI, 0, NULL);
            }


            /* delete_link_key((bd_addr_t *)bt_emitter->bt_sel.mac, 1); */
            delete_link_key(bt_emitter->bt_sel.mac, 1);
            page_list_del_by_mac(bt_emitter->bt_sel.mac);

            ui_hide(BT_EMITTER_CURR_BT_LAYOUT);
            ui_show(BT_EMITTER_VM_LAYOUT);

            break;
        default:
            return false;
        }

        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        touch_action = 2;
        break;
    case ELM_EVENT_TOUCH_DOWN:
        touch_action = 1;
        return true;
    case ELM_EVENT_TOUCH_U_MOVE:
        break;
    case ELM_EVENT_TOUCH_D_MOVE:
        break;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(BT_EMITTER_CURR_BT_CONN_BTN)
.onchange = bt_emitter_curr_bt_onchange,
 .onkey = NULL,
  .ontouch = bt_emitter_curr_bt_ontouch,
};
REGISTER_UI_EVENT_HANDLER(BT_EMITTER_CURR_BT_UNPAIR_BTN)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = bt_emitter_curr_bt_ontouch,
};


static int bt_emitter_curr_bt_layout_ontouch(void *ctr, struct element_touch_event *e)
{
    struct layout *layout = (struct layout *)ctr;

    switch (e->event) {
    case ELM_EVENT_TOUCH_R_MOVE:
        ui_hide(BT_EMITTER_CURR_BT_LAYOUT);
        ui_show(BT_EMITTER_VM_LAYOUT);
        return true;
        break;
    default:
        break;
    }

    return false;
}

REGISTER_UI_EVENT_HANDLER(BT_EMITTER_CURR_BT_LAYOUT)//通用-垂直列表
.onchange = NULL,
 .onkey = NULL,
  .ontouch = bt_emitter_curr_bt_layout_ontouch,
};

// ========================== bt收藏api =================================


#endif
#endif
