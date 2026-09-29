#ifndef AC7076A3_DEMO_H
#define AC7076A3_DEMO_H
#include "typedef.h"
enum demo_accel_status {
    DEMO_ACC_INIT, DEMO_ACC_BUS_LOW, DEMO_ACC_NO_DEVICE, DEMO_ACC_BAD_ID,
    DEMO_ACC_CONFIG_ERROR, DEMO_ACC_WAIT_DATA, DEMO_ACC_READY, DEMO_ACC_READ_ERROR
};
struct demo_accel_sample {
    enum demo_accel_status status;
    u8 chip_id;
    u8 valid;
    s32 mg[3];
    u32 samples;
    u32 errors;
    u32 sampled_at;
};
void ac7076a3_demo_accel_snapshot(struct demo_accel_sample *sample);
struct demo_i2c_fault {
    u8 code;
    u8 stage;
    u8 bit;
    u8 addr;
    u8 raw;
    u8 dir;
    u8 die;
    u8 dieh;
    s32 api[2];
    s32 mode_return;
};
struct demo_i2c_sample {
    u8 bus_ready;
    u8 sda;
    u8 scl;
    u8 irq;
    u8 tp_id_valid;
    u8 tp_id[3];
    u8 scan_addr;
    u8 ack[128]; /* Cumulative: bit 0 write ACK, bit 1 read ACK. */
    u32 rounds;
    u32 probe_errors;
    u8 bus_fault; /* GPIO backend: 1=SCL timeout, 2=START SDA low, 3=STOP low. */
    s32 pin_api[2];
    u8 pin_raw;
    u8 pin_dir;
    u8 pin_die;
    u8 pin_dieh;
    u8 pin_out;
    u8 pin_config_error;
    u8 edge_done;
    u8 edge_pre;
    u8 edge_low;
    u8 edge_up;
    u8 edge_late;
    u8 edge_config_error;
    struct demo_i2c_fault first_fault;
};
void ac7076a3_demo_i2c_snapshot(struct demo_i2c_sample *sample);
int ac7076a3_demo_clock_init(void);
struct demo_ble_sample {
    u8 started, init_ok, state, timed_out;
    u8 mac[6];
    s32 init_return;
    u32 rx_packets, rx_bytes;
};
void ac7076a3_demo_ble_start(void);
void ac7076a3_demo_ble_poll(void);
void ac7076a3_demo_ble_message(int *msg);
void ac7076a3_demo_ble_snapshot(struct demo_ble_sample *sample);
void ac7076a3_demo_task(void *priv);
int ac7076a3_demo_audio_init(void);
int ac7076a3_demo_mic_start(void);
void ac7076a3_demo_audio_poll(void);
void ac7076a3_demo_beep_start(void);
void ac7076a3_demo_lcd_start(void);
void ac7076a3_demo_lcd_poll(void);
void ac7076a3_demo_lcd_task(void *priv);
void ac7076a3_demo_usb_start(void);
void ac7076a3_demo_usb_poll(void);
#endif
