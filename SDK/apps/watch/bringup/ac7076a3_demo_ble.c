#include "app_config.h"
#include "system/includes.h"
#include "system/timer.h"
#include "app_msg.h"
#include "user_cfg.h"
#include "btstack/btstack_task.h"
#include "btstack/avctp_user.h"
#include "btstack/le/le_user.h"
#include "btcontroller_modules.h"
#include "ble_user.h"
#include "le_common.h"
#include "clock.h"
#include "ac7076a3_demo.h"

#if AC7076A3_DEMO_ENABLE && DEMO_BLE_ENABLE
#if !(BT_AI_SEL_PROTOCOL & TRANS_DATA_EN)
#error "Bluetooth demo requires the SDK TRANS_DATA BLE profile"
#endif
extern BT_CONFIG bt_cfg;
static OS_MUTEX ble_mutex;
static u8 ble_mutex_ready;
static struct demo_ble_sample ble_sample;
static u32 ble_started_at;

static void demo_ble_state(void *priv, u8 state)
{
    (void)priv;
    os_mutex_pend(&ble_mutex, 0);
    ble_sample.state = state;
    os_mutex_post(&ble_mutex);
}

static void demo_ble_receive(void *priv, u8 *data, u16 len)
{
    (void)priv;
    (void)data;
    os_mutex_pend(&ble_mutex, 0);
    ++ble_sample.rx_packets;
    ble_sample.rx_bytes += len;
    os_mutex_post(&ble_mutex);
    /* SDK AE01 -> AE02 echo is already implemented in le_trans_data.c. */
}

void ac7076a3_demo_ble_snapshot(struct demo_ble_sample *sample)
{
    memset(sample, 0, sizeof(*sample));
    if (!ble_mutex_ready) {
        return;
    }
    os_mutex_pend(&ble_mutex, 0);
    *sample = ble_sample;
    os_mutex_post(&ble_mutex);
}

void ac7076a3_demo_ble_start(void)
{
    if (ble_mutex_ready || os_mutex_create(&ble_mutex)) {
        return;
    }
    ble_mutex_ready = 1;
    u8 addr[6], all_ff = 1, all_zero = 1;
    int len = syscfg_read(CFG_BT_MAC_ADDR, addr, sizeof(addr));
    for (int i = 0; i < sizeof(addr); ++i) {
        if (len == sizeof(addr)) {
            all_ff &= addr[i] == 0xff;
            all_zero &= addr[i] == 0;
        }
    }
    if (len != sizeof(addr) || all_ff || all_zero) {
        /* RAM-only fallback follows SDK random MAC generation, no VM/OTP write. */
        get_random_number(addr, sizeof(addr));
    }
    bt_update_mac_addr(addr);
    memset(bt_cfg.edr_name, 0, sizeof(bt_cfg.edr_name));
    memcpy(bt_cfg.edr_name, "AC7076A3", 8);
    le_controller_set_mac(addr);
    bt_max_pwr_set(10, 5, 8, TCFG_BT_BLE_TX_POWER);
    bt_pll_para(TCFG_CLOCK_OSC_HZ, clk_get("sys"), 0, 0);

    struct ble_server_operation_t *ops = NULL;
    ble_get_server_operation_table(&ops);
    if (!ops || !ops->regist_state_cbk || !ops->regist_recieve_cbk) {
        os_mutex_pend(&ble_mutex, 0);
        ble_sample.started = 1;
        ble_sample.init_return = -1;
        os_mutex_post(&ble_mutex);
        return;
    }
    ops->regist_state_cbk(NULL, demo_ble_state);
    ops->regist_recieve_cbk(NULL, demo_ble_receive);
    os_mutex_pend(&ble_mutex, 0);
    memcpy(ble_sample.mac, addr, sizeof(addr));
    ble_sample.started = 1;
    os_mutex_post(&ble_mutex);
    ble_started_at = sys_timer_get_ms();
    int ret = btstack_init(); /* SDK calls ble_profile_init during stack setup. */
    os_mutex_pend(&ble_mutex, 0);
    ble_sample.init_return = ret;
    os_mutex_post(&ble_mutex);
}

void ac7076a3_demo_ble_message(int *msg)
{
    if (!ble_mutex_ready || msg[0] != MSG_FROM_BT_STACK) {
        return;
    }
    struct bt_event *event = (struct bt_event *)(msg + 1);
    if (event->event == BT_STATUS_INIT_OK) {
        os_mutex_pend(&ble_mutex, 0);
        u8 already = ble_sample.init_ok;
        ble_sample.init_ok = 1;
        ble_sample.timed_out = 0;
        os_mutex_post(&ble_mutex);
        if (!already) {
            bt_ble_init(); /* Initialize SDK name/advertising only once. */
        }
    }
}

void ac7076a3_demo_ble_poll(void)
{
    if (!ble_mutex_ready) {
        return;
    }
    os_mutex_pend(&ble_mutex, 0);
    if (ble_sample.started && !ble_sample.init_ok &&
        (u32)(sys_timer_get_ms() - ble_started_at) > 15000) {
        ble_sample.timed_out = 1;
    }
    os_mutex_post(&ble_mutex);
}
#endif
