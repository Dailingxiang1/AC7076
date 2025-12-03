#include "sensor_driver.h"
#include "includes.h"
#include "uart.h"
#include "debug.h"
#include "gpio.h"
#include "bank_switch.h"
#include "usr_timer.h"
#include "iic_api.h"


#if CONFIG_SENSOR_DRIVER_ENABLE

#define LOG(fmt,...)    printf("[driver] %s " fmt "\n",__func__, ##__VA_ARGS__)

static u8 sensor_timer_msec = 200;


u8 sensor_write(u8 iic_dev_addr, u8 register_address,  u8 *buf, u8 data_len)
{
    int ret = hw_i2c_master_write_nbytes_to_device_reg(HW_IIC_0, iic_dev_addr, &register_address, 1, buf, data_len);
    return ret == data_len ? RET_OK : RET_ERR;
}

u8 sensor_read(u8 iic_dev_addr, u8 register_address, u8 *buf, u8 data_len, u8 ignore_len)
{
    int ret = hw_i2c_master_read_nbytes_from_device_reg(HW_IIC_0, iic_dev_addr, &register_address, 1, buf, data_len);
    return ret == data_len ? RET_OK : RET_ERR;
}

SENSOR_INTERFACE *sensor_driver_find(sensor_type_t type)
{
    SENSOR_INTERFACE *sensor_driver = NULL;
    list_for_each_sensor(sensor_driver) {
        if (sensor_driver->info->type == type) {
            return sensor_driver;
        }
    }
    return NULL;
}


void sensor_info_printf(void)
{
#if CONFIG_UART_DEBUG_ENABLE
    printf("\n\n******** sensor info ********\n");
    SENSOR_INTERFACE *driver = NULL;

    list_for_each_sensor(driver) {
        printf("\nname  : %s\n", driver->info->name);
        printf("type  : %d\n", driver->info->type);
        printf("state : %d\n", driver->info->state);
        printf("range : +-%d +-%d +-%d +-%d\n", driver->info->range[0], driver->info->range[1], driver->info->range[2], driver->info->range[3]);
        printf("odr   : %d %d %d %d\n", driver->info->odr[0], driver->info->odr[1], driver->info->odr[2], driver->info->odr[3]);
    }
    printf("\n****** sensor info end ******\n\n");
#endif
}

void sensor_driver_check(void)
{
    SENSOR_INTERFACE *sensor_driver = NULL;
    list_for_each_sensor(sensor_driver) {
        if ((sensor_driver->online != NULL) && (sensor_driver->online() == RET_OK)) {
            sensor_driver->info->state = SENSOR_STATE_ONLINE;
        } else {
            sensor_driver->info->state = SENSOR_STATE_OFFLINE;
        }
    }

    //发送sensor_drvier_info给大核
    int msg[3];
    msg[0] = MSG_P11_SENSOR_INFO;
    msg[1] = (int)&sensor_dev_begin;
    msg[2] = (int)&sensor_dev_end;
    int ret = p2m_post_msg(MSG_APP, 0, (u8 *)msg, sizeof(msg));
    LOG("begin=%x end=%x ret=%d", (u32)sensor_dev_begin, (u32)sensor_dev_end, ret);
    sensor_info_printf();
}

static void sensor_timer_irq_handle(void *priv)
{
    task_post_msg(NULL, 1, MSG_P11_SYS_KICK);
}


void sensor_timer_enable(u8 enable)
{
    static u16 timer_handle = 0;

    if (enable && timer_handle == 0) {
        timer_handle = usr_timer_add(NULL, sensor_timer_irq_handle, sensor_timer_msec, 1);
        LOG("handle=%d msec=%d", timer_handle, sensor_timer_msec);
    } else if ((!enable) && (timer_handle > 0)) {
        usr_timer_del(timer_handle);
        timer_handle = 0;
        LOG("handle=%d", timer_handle);
    }
}


void sensor_timer_modify(u32 msec)
{
    sensor_timer_msec = msec;
    sensor_timer_enable(0);
    sensor_timer_enable(1);
}

void sensor_driver_sleep(u8 type, u8 enable)
{
    SENSOR_INTERFACE *sensor_driver = sensor_driver_find(type);
    if (sensor_driver->sleep) {
        LOG("type=%d enable=%d", type, enable);
        sensor_driver->sleep();
        sensor_timer_enable(!enable);
    }
}

void sensor_driver_irq_handle(void)
{
    __attribute__((weak)) s8 driver_sc7a20_is_wake(void);
    if (driver_sc7a20_is_wake) {
        if (driver_sc7a20_is_wake() == RET_OK) {
            sensor_timer_enable(1);
        }
    }
}

void sensor_driver_init(u8 type, u8 enable, u16 range, u8 odr)
{
    SENSOR_INTERFACE *sensor_driver = sensor_driver_find(type);
    LOG("type=%d enable=%d range=%d odr=%d driver=%x", type, enable, range, odr, (u32)sensor_driver);

    if ((sensor_driver != NULL) && (sensor_driver->open != NULL)) {
        if (enable) {
            if (sensor_driver->open(range, odr) == RET_OK) {
                sensor_driver->info->state = SENSOR_STATE_OPEN;
                sensor_timer_enable(1);
                LOG("type %d sucess!", type);
            } else {
                LOG("type %d failed!", type);
            }
        } else {
            if (sensor_driver->close() == RET_OK) {
                sensor_driver->info->state = SENSOR_STATE_CLOSE;
                LOG("type %d sucess!", type);
            } else {
                LOG("type %d failed!", type);
            }
        }

    } else {
        LOG("no driver for sensor %d", type);
    }
}


void sensor_driver_run(void)
{
    u16 data[30 * 3];
    u16 size = sizeof(data);

    SENSOR_INTERFACE *sensor_driver     = NULL;
    SENSOR_INTERFACE *acceler_algorithm = NULL;

    acceler_algorithm = sensor_driver_find(SENSOR_ALGO_WRIST_TILT);
    list_for_each_sensor(sensor_driver) {
        if ((sensor_driver->info->type < SENSOR_DRV_MAX_NUM) &&
            (sensor_driver->info->state == SENSOR_STATE_OPEN) && (sensor_driver->run)) {
            sensor_driver->run(data, &size);

            if ((sensor_driver->info->type == SENSOR_DRV_ACCELER) &&
                (acceler_algorithm != NULL) && (acceler_algorithm->run)) {
                acceler_algorithm->run(data, &size);
            }
        }
    }
}

#endif
