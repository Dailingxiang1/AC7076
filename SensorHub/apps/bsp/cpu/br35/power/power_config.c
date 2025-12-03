#include "power_interface.h"

void board_power_init()
{
    message_init();

    power_init(NULL);
}
