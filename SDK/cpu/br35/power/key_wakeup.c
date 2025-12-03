#include "asm/power_interface.h"
#include "app_config.h"
#include "gpio.h"

void key_active_set(u8 port);
static void key_wakeup_callback(P33_IO_WKUP_EDGE edge)
{
    key_active_set(0);
}

//只读配置，用户后续新增io唤醒、ad唤醒注意使用const修饰，节约静态ram
static const struct _p33_io_wakeup_config port0 = {
    .pullup_down_mode  = PORT_INPUT_PULLUP_10K,
    .filter      		= PORT_FLT_DISABLE,
    .edge               = FALLING_EDGE,
    .gpio              = IO_PORTB_07,
    .callback          = key_wakeup_callback,
};

void key_wakeup_init()
{
#if (!TCFG_LP_TOUCH_KEY_ENABLE)
    p33_io_wakeup_port_init(&port0);
    p33_io_wakeup_enable(IO_PORTB_07, 1);

#endif
}
