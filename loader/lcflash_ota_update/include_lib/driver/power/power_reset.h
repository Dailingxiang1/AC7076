#ifndef __POWER_RESET_H__
#define __POWER_RESET_H__

enum RESET_FLAG {
    RESET_FLAG_RESERVE,
    EXCEPTION_FLAG,
    ASSERT_FLAG,
    UPDATE_FLAG,
    BT_FLAG,
    LP_OSC_UP_TO,
};

void system_reset(enum RESET_FLAG flag);

#endif
