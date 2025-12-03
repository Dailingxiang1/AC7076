#ifdef SUPPORT_MS_EXTENSIONS_APP
#pragma bss_seg(".testbox_ble_update.data.bss")
#pragma data_seg(".testbox_ble_update.data")
#pragma const_seg(".testbox_ble_update.text.const")
#pragma code_seg(".testbox_ble_update.text")
#endif
#include "update_main.h"
#include "power/p33.h"
#include "common.h"
#include "msg.h"
#include "sys_timer.h"
#include "btctrler_api_for_update.h"
#include "testbox_update.h"
#include "timer.h"
#include "delay.h"
#include "uart.h"
#include "wdt.h"
#include "clock.h"

#define LOG_TAG_CONST       BLE_UPDATE
#define LOG_TAG             "[BLE_UPDATE]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_DUMP_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"

//-----------------------------extern start-------------------------------//
extern int le_controller_set_mac(void *addr);
extern void bt_ble_init(void);
extern void btstack_init(void);
extern void dynamic_mem_init(void *malloc_pool, int malloc_size);
extern void ble_rx_update_handle_register(void (* handle)(void *, u8 *, u32));
extern void ll_vendor_update_req_handle_register(void (*handle)(u8 *data, u16 len));
extern void ll_update_send_data_handle(u8 btupdate_sw, u16 len, u8 seek_type, u32 seek_offsize);
extern void update_set_trim_info(u8 *trim);
extern void register_ll_update_handle(void (*rx_handler)(void *, u8 *, u32), void (*hook)(void *));
//-----------------------------extern end--------------------------------//

//-----------------------------local start--------------------------------//
static u16 ble_f_open(void);
static u16 ble_f_read(void *fp, u8 *buf, u16 len);
static int ble_f_seek(void *fp, u8 type, u32 offset);
static u16 ble_send_update_len(u32 update_len);
static void ble_update_state_cbk(u32 status, void *priv);

static testbox_update_info update_info = {
    .mutil_ufw_offset = 0,
};
#define __this		(&update_info)

#ifdef CONFIG_BTCTRLE_V2_ENABLE
#if defined(CONFIG_CPU_BD47)
#define MALLOC_POOL_SIZE    ((1024 + 308) * 4)
#else
#define MALLOC_POOL_SIZE    ((1024 + 308) * 6)
#endif
#else
#define MALLOC_POOL_SIZE    ((1024 + 308) * 4)
#endif
u32 malloc_pool[MALLOC_POOL_SIZE / 4];
u32 malloc_pool_size = MALLOC_POOL_SIZE;

update_op_api_t ble_op_api = {
    .f_open = ble_f_open,
    .f_read = ble_f_read,
    .f_seek = ble_f_seek,
    .notify_update_content_size = ble_send_update_len,
};

update_mode_info_t update_mode_info = {
    .type      = BLE_TEST_UPDATA,
    .state_cbk = ble_update_state_cbk,
    .file_op   = &ble_op_api,
};

//-----------------------------local end--------------------------------//

static void ble_update_control(u8 btupdata_sw, u16 len, u8 seek_type, u32 seek_offsize)
{
    ll_update_send_data_handle(btupdata_sw, len, seek_type, seek_offsize);
}

//ble升级数据回调
static int ble_updata_handle(void *priv, void *buf, int len)
{
    if (__this->read_buf && (__this->state == UPDATA_REV_DATA)) {
        if (__this->need_rx_len >= __this->data_len + len) {
            memcpy(__this->read_buf + __this->data_len, buf, len);
            __this->data_len += len;
        }

        if (__this->need_rx_len == __this->data_len) {
            __this->state = 0;
        }
    }

    return 0;
}

u16 ble_f_open(void)
{
    log_info("---------UPDATA_OPEN----------\n");
    __this->file_offset = 0;
    __this->seek_type = SEEK_SET;
    ble_update_control(UPDATA_OPEN, 0, 0, 0);
    return 0;
}

u16 ble_f_read(void *fp, u8 *buff, u16 len)
{
    __this->state = UPDATA_REV_DATA;
    __this->need_rx_len = len;
    __this->data_len = 0;
    __this->read_buf = buff;

    ble_update_control(UPDATA_READ_OFFSIZE, len, __this->seek_type, __this->file_offset);

    __this->bt_time_timeout = 0;

    while (__this->state && __this->bt_time_timeout < UPDATE_CMD_WAIT_TIMEOUT);

    if (__this->data_len == len) {
        __this->file_offset += len;
        return len;
    } else {
        return -1;
    }

    return 0;//FR_OK;
}

int ble_f_seek(void *fp, u8 type, u32 offset)
{
    if (type == SEEK_SET) {
        offset += __this->mutil_ufw_offset;
        __this->file_offset = offset;
    } else if (type == SEEK_CUR) {
        __this->file_offset += offset;
    }

    __this->seek_type = type;

    return 0;//FR_OK;
}

//ufw嵌套ufw格式处理
void mutil_cpu_set_offset(u32 offset)
{
    __this->mutil_ufw_offset = offset;
    __this->file_offset = __this->mutil_ufw_offset; //预先设置好偏移
}

u16 ble_f_stop(u8 err)
{
    log_info("---------bt_f_stop <<---------- 0x%x\n", err);
    if (BT_UPDATE_OVER == err) {
        ble_update_control(UPDATA_STOP, 0, 0, 0);
    } else if (BT_UPDATE_KEY_ERR == err) {
        ble_update_control(UPDATA_STOP_KEYERR, 0, 0, 0);
    } else {
        ble_update_control(UPDATA_STOP, err, 0, 0);
    }
    ble_rx_update_handle_register(NULL);
    /* delay_2ms(500); */
    mdelay(1000);
    return 1;
}

u16 ble_send_update_len(u32 update_len)
{
    log_info("---------send_update_len:%x----------\n", update_len);
    ble_update_control(UPDATA_SEEK, 0, BT_SEEK_TYPE_UPDATE_LEN, update_len);
    return 1;
}

void ble_update_result_report(u8 err)
{
    u8 res;
    switch (err) {
    case UPDATE_ERR_NONE:
        res = BT_UPDATE_OVER;
        break;

    case UPDATE_ERR_KEY_ERR:
        res = BT_UPDATE_KEY_ERR;
        break;

    default:
        res = fs_update_result_transition(err);
        break;
    }

    log_info("f_stop:%x\n", res);
    ble_f_stop(res);
}

static void update_test_user_handler(u8 *data, u16 len)
{
    log_info("--set update handle--\n");
    ble_rx_update_handle_register(ble_updata_handle);
    register_ll_update_handle(ble_updata_handle, NULL);
    __this->state = UPDATA_START;
}

void ble_updata_init()
{
    dynamic_mem_init(malloc_pool, malloc_pool_size);
    ll_vendor_update_req_handle_register(update_test_user_handler);
    task_message_init();
    btstack_init();
    bt_ble_init();
    sys_timer_add(NULL, stack_run_loop_resume, 5);
    init_soft_interrupt();                   //btstack loop软中断初始化
}

void bt_update_timeout(void *priv)
{
    __this->bt_time_timeout ++;
}

static u32 ble_connect_check_time = 0;
static void ble_update_connect_check(void *priv)
{
    ble_connect_check_time ++;
    while (__this->state == UPDATA_START) {         //wait testbox start update
        ble_connect_check_time = 0;
    }
    if (ble_connect_check_time > 10) {
#if OTA_LOADER_RECORD_RST_INFO_CONFIG
        ota_loader_record_rst_info();
#endif
        update_reset();
    }
}

void ble_update_state_cbk(u32 status, void *priv)
{
    UPDATA_PARM *p = priv;
    switch (status) {
    case UPDATE_PARM:       //升级需要的参数
        log_info("ble_mac_addr:\n");
        log_info_hexdump(p->parm_priv, 6);
        le_controller_set_mac(p->parm_priv);//BLE广播地址
#ifdef CONFIG_BTCTRLE_V2_ENABLE
        update_set_trim_info(update_param_ext_get(priv, EXT_LDO_TRIM_RES));
#endif
        u8 *wla_data = update_param_ext_get(priv, EXT_BT_WLA_INFO);
        if (wla_data) {
            u32 wla_con[3];
            printf("wla data:\n");
            printf_buf(wla_data, 12);
            memcpy((u8 *)wla_con, wla_data, sizeof(wla_con));
            btosc_upgrade_cfg(wla_con);
        }
        break;
    case UPDATE_START:
        log_info("BT_UPDATE_START...\n");
        ble_updata_init();
        sys_timer_add(NULL, bt_update_timeout, 10);
        sys_timer_add(NULL, ble_update_connect_check, 60000);
        while (__this->state != UPDATA_START) {         //wait testbox start update
            wdt_clear();//清看门狗
        }
        break;

    case UPDATE_END:		//升级结束需要保存结果到Ram给SDK获取，并回复主机升级结果
        log_info("UPDATE_END\n");
        if (*((u8 *)priv) == UPDATE_ERR_NONE) {
            set_updata_result(BLE_TEST_UPDATA, UPDATA_SUCCESSFULLY);
        } else {
            set_updata_result(BLE_TEST_UPDATA, UPDATA_DEV_ERR);
        }
        ble_update_result_report(*((u8 *)priv));
        /* cpu_reset(); */
        update_reset();
        break;
    }
}
