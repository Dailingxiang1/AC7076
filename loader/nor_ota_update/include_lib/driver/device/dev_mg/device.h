#ifndef __DEVICE_H__
#define __DEVICE_H__

#include "typedef.h"
#include "atomic.h"
#include "ioctl_cmds.h"

struct dev_node {
    const char *name;
    const struct device_operations *ops;
    void *priv_data;
};

struct device {
    atomic_t ref;
    void *private_data;
    const struct device_operations *ops;
    void *platform_data;
    void *driver_data;
};

struct device_operations {
    int (*open)(struct dev_node *node, struct device **device, void *arg);
    int (*read)(struct device *device, void *buf, u32 len, u32);
    int (*ioctl)(struct device *device, u32 cmd, u32 arg);
    int (*close)(struct device *device);
    int (*write)(struct device *device, void *buf, u32 len, u32);
};


#define REGISTER_DEVICES(node) \
    const struct dev_node node[] SEC_USED(.device)



void *dev_open(const char *name, void *arg);

int dev_read(void *device, void *buf, u32 len);

int dev_write(void *device, void *buf, u32 len);

int dev_ioctl(void *device, int cmd, u32 arg);

int dev_close(void *device);

int dev_bulk_read(void *_device, void *buf, u32 offset, u32 len);

int dev_bulk_write(void *_device, void *buf, u32 offset, u32 len);

void set_device_offset(u32 offset);

u32 get_device_offset(void);

#endif


