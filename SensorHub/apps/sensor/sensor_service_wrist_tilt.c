#include "sdk_config.h"
#include "sensor_driver.h"
#include "includes.h"
#include "sensor_service_wrist_tilt.h"


#if CONFIG_SENSOR_DRIVER_ENABLE

#define LOG(fmt,...)     printf("[algo] %s() " fmt "\n",__func__, ##__VA_ARGS__)

enum {
    MSYS_STATE_NORMAL = 0,
    MSYS_STATE_PWR_DOWN,
    MSYS_STATE_PWR_OFF,
    MSYS_STATE_SOFT_PWR_OFF,
    MSYS_STATE_SOFT_PWR_OFF_KEEP_NVRAM,
    MSYS_STATE_LIGHT_PWR_DOWN,
};

#define MSYS_STATE_GET()            (P11_SYSTEM->P11_SYS_CON0 & 0xf)


static sensor_info_t sensor_info = {
    .type = SENSOR_ALGO_WRIST_TILT,
    .name = "JL GESTURE V1.0",
};

static s8 wrist_tilt_online(void)
{
    return RET_OK;
}

static void wrist_tilt_event_response(u8 event)
{
    static u8 p11_algo_event;

    p11_algo_event = event;
    u8 low_power_mode  = MSYS_STATE_GET() != MSYS_STATE_NORMAL;

    if (p11_algo_event == ALGO_WRIST_DOWN && low_power_mode) {
        return;
    }
    LOG(" %d", event);

    //发送 算法事件 给大核
    int msg[2];
    msg[0] = MSG_P11_ALGORITHM_EVENT;
    msg[1] = p11_algo_event;
    p2m_post_msg(MSG_APP, 0, (u8 *)msg, sizeof(msg));
}

s8 wrist_tilt_run(void *arg, u16 *len)
{
    axis_data_t *accel_data = arg;
    u8        accel_point   = *len / sizeof(axis_data_t);

    jl_gesture_event_t gesture_event = ALGO_NOTHING;

    for (u8 i = 0; i < accel_point; i++) {
        gesture_event = sensor_jl_gesture_run(accel_data[i].x, accel_data[i].y, accel_data[i].z);
        if (sensor_info.state == SENSOR_STATE_OPEN &&   gesture_event != ALGO_NOTHING) {
            wrist_tilt_event_response(gesture_event);
        }
        // LOG("gesture=%d,xyz:%d,%d,%d",gesture_event,accel_data[i].x,accel_data[i].y,accel_data[i].z);
    }
    return RET_OK;
}

static s8 wrist_tilt_open(u16 range, u16 odr)
{
    return RET_OK;
}

static s8 wrist_tilt_close(void)
{
    return RET_OK;
}


REGISTER_SENSOR(wrist_tilt) = {
    .info    = &sensor_info,
    .online  = wrist_tilt_online,
    .open    = wrist_tilt_open,
    .close   = wrist_tilt_close,
    .run     = wrist_tilt_run,
};

#endif