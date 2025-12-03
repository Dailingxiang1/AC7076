#include "includes.h"
#include "uart.h"
#include "debug.h"
#include "gpio.h"
#include "bank_switch.h"
#include "usr_timer.h"
#include "iic_api.h"
#include "main.h"
#include "bsp.h"
#include "sensor/sensor_driver.h"

/*
	function: m2p消息处理，主循环不会收到ack消息
	param index:等待返回ack的消息序号
	return val：1, 收到对应ack消息 0,未收到对应ack消息
 */
int m2p_msg_hdl(u32 index)
{
    struct lp_msg_handler *p;
    u8 msg[32];
    struct lp_msg_head head;

    int ret = m2p_get_msg(&head, (u8 *)msg, ARRAY_SIZE(msg));

    //成功读取到消息
    if (ret == MSG_NO_ERROR) {
        list_for_each_m2p_msg_handler(p) {
            if (p->type == head.type) {
                if (head.ack) {
                    config_post_ack_flag(0);
                }
                p->handler(p->priv, (u8 *)msg, head.len);
                if (head.ack) {
                    //若是应答消息则返回应答
                    p11_ack_msys(head.index);
                    config_post_ack_flag(1);
                }
            }
        }
    } else if (ret == MSG_NO_MSG) {
        //未读取到消息

    } else {
        ASSERT(0, "ret: %d, type: %d, ack: %d, index: %d, len: %d\n", ret, head.type, head.ack, head.index, head.len);

    }

    return 0;
}

static void callback(u32 *priv)
{
    printf("function: callback, %d\n", (u32)priv);
}


#include "circular_buf.h"
#include "ipc_spin_lock.h"
#define P11_CBUF_TEST  0

#if P11_CBUF_TEST

static cbuffer_t cbuffer;
static u8 cbuf_test[256];
static cbuffer_child_t entry[3];//支持3个成员读取

static void p11_cbuf_test_init()
{
    int msg[2];
    msg[0] = MSG_P11_SYS_RAM_INIT;
    msg[1] = (int)&cbuffer;
    p2m_post_msg(MSG_APP, 0, (u8 *)msg, sizeof(msg));
    cbuf_mult_read_init(&cbuffer, cbuf_test, sizeof(cbuf_test), 3, entry);


    cbuf_mult_entry_enable(&cbuffer, 0, 1); //开启节点
    cbuf_mult_entry_enable(&cbuffer, 1, 1);
    cbuf_mult_entry_enable(&cbuffer, 2, 1);

    printf("%s %d %x\n", __func__, __LINE__, (int)&cbuffer);

}



static void p11_cbuf_write_test()
{
    u8 test[50];
    static int count_add = 0;
    int count = count_add;

    for (int i = 0; i < 50; i++) {
        test[i] = (count++) & 0xff;
    }


    int len = cbuf_write(&cbuffer, test, 50);
    /* int len  = 0;  */
    if (len == 50) {
        printf("\nwrite succ %x\n", count_add);
        count_add += 50;
    } else {
        /* printf("falil  \n"); */
    }
}


#endif

void app_main()
{
    bsp_init();
    usr_timer_init();

    lptmr1_init();

#if CONFIG_SENSOR_DRIVER_ENABLE
    sensor_driver_check();
#endif

#if P11_CBUF_TEST
    p11_cbuf_test_init();
    p11_cbuf_write_test();
#endif
    /* task_post_msg(NULL,1,MSG_P11_SYS_KICK); */
    while (1) {
        m2p_msg_hdl(0);
    }
}

/*
	function: 主循环回调处理
 */
void app_handler_add(void (*callback)(u32 *priv), u32 *priv)
{
    u8 msg[9];
    u32 msg_cb = (u32)callback;
    u32 msg_priv = (u32)priv;

    msg[0] = MSG_APP_CALLBACK;
    memcpy(msg + 1, &msg_cb, 4);

    if (priv != NULL) {
        memcpy(msg + 5, &msg_priv, 4);
        m2p_post_msg(MSG_APP, 0, msg, 9);
    } else {
        m2p_post_msg(MSG_APP, 0, msg, 5);
    }
}



int task_post_msg(char *name, int argc, ...)
{
    int msg[16];
    va_list argptr;
    va_start(argptr, argc);
    int  param;
    ASSERT(argc < sizeof(msg) / sizeof(msg[0]))
    for (int i = 0; i < argc; ++i) {
        param = va_arg(argptr, int);
        msg[i] = param;
    }
    m2p_post_msg(MSG_APP, 0, (u8 *)msg, argc * sizeof(int));
    va_end(argptr);
    return 0;
}



static void app_msg_handler(void *priv, u8 *msg, u32 len)
{

    printf("app_msg_handler = %d\n", msg[0]);
    if (msg[0] == MSG_APP_CALLBACK) {
        u32 *msg_priv;
        void (*msg_cb)(u32 * priv);
        memcpy(&msg_cb, msg + 1, 4);
        memcpy(&msg_priv, msg + 5, 4);
        msg_cb(msg_priv);
    }


    switch (msg[0]) {
#if CONFIG_SENSOR_DRIVER_ENABLE
    case MSG_P11_SYS_KICK:
        printf("MSG_P11_SYS_KICK\n");
        sensor_driver_run();
        break;
    case MSG_P11_SENSOR_SLEEP:
        printf("MSG_P11_SENSOR_SLEEP\n");
        sensor_driver_sleep(msg[1], msg[2]);
        break;

    case MSG_P11_SENSOR_IRQ:
        printf("MSG_P11_SENSOR_IRQ\n");
        sensor_driver_irq_handle();
        break;
#endif

#if P11_CBUF_TEST
        p11_cbuf_write_test();
#endif
        //test
        break;
    case MSG_P11_SENSOR_INIT: {
        printf("m2p_post_msg  len=%d\n", len);
#if CONFIG_SENSOR_DRIVER_ENABLE
        sensor_driver_init(msg[1], msg[2], msg[3] << 8 | msg[4], msg[5]);
#endif
        break;
    }

    case MSG_P11_SOFF_EVENT: {
        lptmr1_set_wkup_time(0, P11_LPTMR_WKUP_EVENT);
    }
    }

}

REGISTER_M2P_MSG_HANDLER(0, MSG_APP, app_msg_handler);
