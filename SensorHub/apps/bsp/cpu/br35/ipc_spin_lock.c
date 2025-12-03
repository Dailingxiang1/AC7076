#include "includes.h"
#include "ipc_spin_lock.h"

void ipc_spin_lock_init()
{
    for (u8 i = 0; i < 16; i++) {
        P11_RESLOCK->LOCK[i] = 0;
    }
}


volatile u16 ipc_debug_bit;


AT(.ipc_spin_lock.text.cache.L1)
__attribute__((noinline))
void ipc_spin_lock(enum ipc_spin_lock_event event)//0~15
{
    ASSERT(event <= 15);

    if (cpu_in_irq()) {
        ipc_debug_bit |= BIT(event);
    } else {
        if (ipc_debug_bit & BIT(event)) {
            ASSERT(cpu_irq_disable(), "%x\n", event);
        }
    }

    while (P11_RESLOCK->LOCK[event]);
}
AT(.ipc_spin_lock.text.cache.L1)
__attribute__((noinline))
void ipc_spin_unlock(enum ipc_spin_lock_event event)//0~15
{
    ASSERT(event <= 15);
    P11_RESLOCK->LOCK[event] = 0;
}

