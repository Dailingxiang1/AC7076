#include "asm/power_interface.h"
#include "iokey.h"
#include "irkey.h"
#include "adkey.h"
#include "app_config.h"



__attribute__((weak))
void board_gpio_config_soft_poweroff(u32 *gpio_config)
{
    //注册在板卡
}

void gpio_config_soft_poweroff(void)
{
    PORT_TABLE(g);

//#if TCFG_IOKEY_ENABLE
//    PORT_PROTECT(get_iokey_power_io());
//#endif
//

#if TCFG_ADKEY_ENABLE
    PORT_PROTECT(get_adkey_io());

#endif

#if ((TCFG_NORFLASH_DEV_ENABLE)&&( TCFG_EX_FLASH_POWER_IO == NO_CONFIG_PORT))
    PORT_PROTECT(ESPI_PORTA_CLK);
    PORT_PROTECT(ESPI_PORTA_D0);
    PORT_PROTECT(ESPI_PORTA_D1);
    PORT_PROTECT(ESPI_PORTA_D2);
    PORT_PROTECT(ESPI_PORTA_D3);
#endif

#if TCFG_APP_PC_EN
    JL_PORTUSB->CON = BIT(8);
#endif

    //PORT_PROTECT(IO_PORTB_01);
    board_gpio_config_soft_poweroff(gpio_config);
    __port_init((u32)gpio_config);
}
