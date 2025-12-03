#include "asm/power_interface.h"
#include "power/p11_app_msg.h"
#include "init.h"
#include "app_config.h"
#include "cpu/includes.h"
#include "gpio_config.h"
#include "rtc/rtc.h"

//-------------------------------------------------------------------
/*config
 */
#define CONFIG_UART_DEBUG_ENABLE	(CONFIG_DEBUG_ENABLE||CONFIG_DEBUG_LITE_ENABLE)
#ifdef TCFG_DEBUG_UART_TX_PIN
#define CONFIG_UART_DEBUG_PORT		TCFG_DEBUG_UART_TX_PIN
#else
#define CONFIG_UART_DEBUG_PORT		-1
#endif

#define DO_PLATFORM_UNINITCALL()	do_platform_uninitcall()


/*-----------------------------------------------------------------------
 *进入、退出低功耗函数回调状态，函数单核操作、关中断，请勿做耗时操作
 *
 */

static u32 p11_pb_sel;
void gpio_portb_control(u32 p11_sys)
{
    ipc_spin_lock(IPC_SPIN_LOCK_EVENT_P11_GPIO);

    if (p11_sys) {
        p11_pb_sel = P11_PORT->PB_SEL;

        for (int i = 0; i < 9; i++) {
            //进入低功耗主系统控制的IO需要由P11来控制
            if ((p11_pb_sel & BIT(i)) == 0) {
                SFR(P11_PORT->PB_DIR,  i, 1, ((JL_PORTB->DIR & BIT(i)) ? 1 : 0));
                SFR(P11_PORT->PB_DIE,  i, 1, ((JL_PORTB->DIE & BIT(i)) ? 1 : 0));
                SFR(P11_PORT->PB_DIEH, i, 1, ((JL_PORTB->DIEH & BIT(i)) ? 1 : 0));
                SFR(P11_PORT->PB_PU0,  i, 1, ((JL_PORTB->PU0 & BIT(i)) ? 1 : 0));
                SFR(P11_PORT->PB_PU1,  i, 1, ((JL_PORTB->PU1 & BIT(i)) ? 1 : 0));
                SFR(P11_PORT->PB_PD0,  i, 1, ((JL_PORTB->PD0 & BIT(i)) ? 1 : 0));
                SFR(P11_PORT->PB_PD1,  i, 1, ((JL_PORTB->PD1 & BIT(i)) ? 1 : 0));
                SFR(P11_PORT->PB_HD0,  i, 1, ((JL_PORTB->HD0 & BIT(i)) ? 1 : 0));
                SFR(P11_PORT->PB_HD1,  i, 1, ((JL_PORTB->HD1 & BIT(i)) ? 1 : 0));
                SFR(P11_PORT->PB_OUT,  i, 1, ((JL_PORTB->OUT & BIT(i)) ? 1 : 0));
                SFR(P11_PORT->PB_SPL,  i, 1, ((JL_PORTB->SPL & BIT(i)) ? 1 : 0));
                P11_PORT->PB_SEL |= BIT(i);
            }
        }
    } else {
        for (int i = 0; i < 9; i++) {
            if ((p11_pb_sel & BIT(i)) == 0) {
                P11_PORT->PB_SEL &= ~BIT(i);
            }
        }
    }

    ipc_spin_unlock(IPC_SPIN_LOCK_EVENT_P11_GPIO);
}



__attribute__((weak))
void board_sleep_enter_callback()
{
    //注册在板卡
}

__attribute__((weak))
void board_sleep_exit_callback()
{
    //注册在板卡
}

__attribute__((weak))
void board_poweroff_uninit()
{
    //注册在板卡
}

void sleep_enter_callback()
{
    board_sleep_enter_callback();
    gpio_portb_control(1);
}

void sleep_exit_callback()
{
    gpio_portb_control(0);
    board_sleep_exit_callback();
}

//maskrom 使用到的io
static void __mask_io_cfg()
{
    struct app_soft_flag_t app_soft_flag = {0};

    mask_softflag_config(&app_soft_flag);
}

void msys_soff_to_p11_msg(void)
{
    u8 msg[3];
    msg[0] = MSG_P11_SOFF_EVENT;
    m2p_post_msg(MSG_APP, 1, (u8 *)msg, sizeof(msg));
}

u8 power_soff_callback()
{
    DO_PLATFORM_UNINITCALL();

    board_poweroff_uninit();

    rtc_set_1s_read_time_switch(0);

    rtc_save_context_to_vm();

    msys_soff_to_p11_msg();

    return 0;
}

void gpio_config_soft_poweroff(void);
void board_set_soft_poweroff()
{
    __mask_io_cfg();

    gpio_config_soft_poweroff();

    gpio_config_uninit();

}

void power_early_flowing()
{
    /*默认把低功耗部分代码加载到power overlay段*/
    lowpower_init();

    PORT_TABLE(g);

    init_boot_rom();

    printf("get_boot_rom(): %d", get_boot_rom());

    // 默认关闭长按复位0，由key_driver配置
    gpio_longpress_pin0_reset_config(PINR_DEFAULT_IO, 0, 0, 1, 1, 0);
    //长按复位1默认配置2s，写保护
#if TCFG_CHARGE_ENABLE
    gpio_longpress_pin1_reset_config(IO_LDOIN_DET, 1, 2, 1);
#endif

#if CONFIG_UART_DEBUG_ENABLE
    PORT_PROTECT(CONFIG_UART_DEBUG_PORT);
#endif

    power_early_init((u32)gpio_config);


}

//early_initcall(_power_early_init);

static int power_later_flowing()
{
    pmu_trim(0, 0);

    power_later_init(0);

#if TCFG_APP_RTC_EN
    if (rtc_is_alarm_wkup() == RTC_OVERFLOW_WKUP) {
        rtc_debug_dump();
        power_set_soft_poweroff();
    }
#endif

    return 0;
}

late_initcall(power_later_flowing);

