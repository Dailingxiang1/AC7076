#ifndef  __UPGRADE_H__
#define  __UPGRADE_H__


#include "typedef.h"


void *get_sdk_swap_addr();
void goto_uart_upgrade(void);
void goto_mask_usb_updata(void);
u8 update_loop_run(u32 flash_ota_addr);
u8 jl_check_upgrade(u8 jlfs_err);


#endif

