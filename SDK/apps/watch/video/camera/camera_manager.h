#ifndef __CAMERA_MANAGER__
#define __CAMERA_MANAGER__
#include "app_config.h"

struct camera_device {
    u8 fps;
    u16 width;
    u16 height;
    int (*check)(void);
    int (*init)(void);
    int (*deinit)(void);
    void (*supend)(void);
    void (*resume)(void);
};

#define REGISTER_CAMERA_DEVICE(dev) \
        const struct camera_device camera_device_##dev sec(.camera_device) =

extern const struct camera_device camera_device_module_begin[];
extern const struct camera_device camera_device_module_end[];

#define list_for_each_camera_device_module(p) \
    for (p =(struct camera_device*)camera_device_module_begin; p < (struct camera_device*)camera_device_module_end; p++)


int camera_manager_start(void);
int camera_manager_stop(void);

void camera_manager_resume(void);
void camera_manager_suspend(void);

int camera_manager_init(void);
int camera_manager_deinit(void);

int camera_manager_data_read(u8 **pp_data, int *p_len);
int camera_manager_read_done(u8 *data_buf);

int camera_manager_get_dev_width_height(int *width, int *height);
int camera_manager_get_dev_fps(int *fps);

#endif//__CAMERA_MANAGER__
