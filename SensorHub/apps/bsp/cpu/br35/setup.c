#include "includes.h"
#include "gpio.h"
#include "ipc_spin_lock.h"

void app_main();

int main(void)
{
    local_irq_disable();
    ipc_spin_lock_init();

    clock_early_init();

    power_early_flowing(0);

#if CONFIG_UART_DEBUG_ENABLE
    debug_uart_init(CONFIG_DEBUG_UART_TX_PIN);
#endif /* #if CONFIG_UART_DEBUG_ENABLE */

    printf("\n============ Hello BR35 P11 ============\n");

    //p11_q32s(0)->PMU_CON0 |= BIT(2); //P11 WKUP EN
    printf("p11_q32s(0)->PMU_CON0 = 0x%x\n", p11_q32s(0)->PMU_CON0);

    debug_init(); //异常检测初始化

    board_power_init();

    local_irq_enable();

    while (M2P_WAIT_RELEASE == 0) {
        asm("csync");
    }
    M2P_WAIT_RELEASE = 0;

    app_main();
}



