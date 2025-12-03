#include "bt.h"
#include "ui/ui_api.h"
#include "avctp_user.h"

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

#if (defined (CONFIG_UI_STYLE_JL_CSC_PUBLIC_MODLS_ENABLE))
#if TCFG_UI_ENABLE_BT_EMITTER && TCFG_USER_EMITTER_ENABLE

#define BT_SEARCH_TIME_MS				100
#define TEXT_NAME_LEN                   32

u16 emitter_disconnect_timer = 0;
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
extern void sbox_get_earphone_mac(u8 *addr);

// ========================== bt搜索api =================================
int bt_menu_list_init()
{
    if (!bt_emitter) {
        return 0;
    }

    bt_emitter->hd.head = zalloc(sizeof(struct list_head));
    if (!bt_emitter->hd.head) {
        log_error("%s bt_emitter->hd.head is null", __func__);
        return 0;
    }
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
    ASSERT(bt_emitter->hd_vm, "bt_emitter->hd_vm malloc err");

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
int bt_emitter_status_handler(const char *type, u32 arg)
{
    log_info("[msg]%s-%d>>>>>>>>>>>arg=%d type=%s", __FUNCTION__, __LINE__, arg, type);
    u8 *emitter_mac;
    if (type && (!strcmp(type, "hci_event"))) {
        switch (arg) {
        case HCI_EVENT_INQUIRY_COMPLETE:
            break;
        case HCI_EVENT_DISCONNECTION_COMPLETE:
            if (bt_emitter && bt_emitter->bt_conn_timer) {
                sys_timeout_del(bt_emitter->bt_conn_timer);
                bt_emitter->bt_conn_timer = 0;
            }

            emitter_mac = get_cur_connect_emitter_mac_addr();
            if (emitter_mac != NULL) {
                put_buf(emitter_mac, 6);
            }
            extern void bt_emitter_disconnect_deal(void *priv);
            if (emitter_disconnect_timer == 0) {
                emitter_disconnect_timer = sys_timeout_add((void *)&emitter_disconnect_timer, bt_emitter_disconnect_deal, 120 * 1000);
            }
            /* extern int clock_alloc(const char *name, u32 clk); */
            /* clock_alloc("bt_scan", 96 * 1000000); */
            break;
        }
    } else if (type && (!strcmp(type, "hci_value"))) {
        switch (arg) {
        case ERROR_CODE_SUCCESS:
            if (emitter_disconnect_timer != 0) {
                sys_timeout_del(emitter_disconnect_timer);
                emitter_disconnect_timer = 0;
                /* extern int clock_free(char *name); */
                /* clock_free("bt_scan"); */
            }
            break;


        case ERROR_CODE_PIN_OR_KEY_MISSING:
            u8 mac_buf[6];
            extern u8 bt_emitter_need_reconnect();
            if (bt_emitter_need_reconnect()) {

                sbox_get_earphone_mac(mac_buf);
                emitter_bt_connect(mac_buf);
            }
            if (emitter_disconnect_timer != 0) {
                sys_timeout_del(emitter_disconnect_timer);
                emitter_disconnect_timer = 0;
                /* extern int clock_free(char *name); */
                /* clock_free("bt_scan"); */
            }
            break;
        case ERROR_CODE_PAGE_TIMEOUT:
        case ERROR_CODE_AUTHENTICATION_FAILURE:
        case ERROR_CODE_CONNECTION_TIMEOUT:
        case CUSTOM_BB_AUTO_CANCEL_PAGE:
        case BB_CANCEL_PAGE:
            break;

        }

        if (bt_emitter && bt_emitter->bt_conn_timer) {
            sys_timeout_del(bt_emitter->bt_conn_timer);
            bt_emitter->bt_conn_timer = 0;
        }
    }
    return 0;
}


#endif
#endif
