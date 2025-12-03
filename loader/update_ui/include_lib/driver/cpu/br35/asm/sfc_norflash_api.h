#ifndef __SFC_NORFLASH_API_H__
#define __SFC_NORFLASH_API_H__

#include "typedef.h"
#include "device.h"

int sfc_norflash_init(const struct dev_node *node, void *arg);
int sfc_norflash_open(const char *name, struct device **device, void *arg);
int sfc_norflash_read(struct device *device, void *buf, u32 len, u32 offset);
int sfc_norflash_write(struct device *device, void *buf, u32 len, u32 offset);
int sfc_norflash_ioctl(struct device *device, u32 cmd, u32 arg);

u32 sfc0_flash_addr2cpu_addr(u32 offset);
u32 sfc_norflash_read_uuid(u8 *uuid);
u8 *sfc_norflash_get_uuid(void);
u32 sfc_norflash_erase_otp();
u32 sfc_norflash_read_otp(void *buf, u32 len, u32 addr);
u32 sfc_norflash_write_otp(const u8 *buf, u32 len, u32 addr);

void sfc_norflash_set_early_unenc_zone(void *_arg);

#endif
