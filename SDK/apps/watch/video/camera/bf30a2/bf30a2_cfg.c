

#include "camera_manager.h"
#include "bf30a2_cfg.h"
#include "iic_api.h"
#if TCFG_CAMERA_DEV_BF30A2
static const struct reginfo sensor_init_data[] = {
    0xf2, 0x01, //软复位
    0x15, 0x80,
    0x6b, 0x73,
    0x04, 0x00,
    0x06, 0x26,
    0x08, 0x07,
    0x1c, 0x12,
    0x1e, 0x26,
    0x1f, 0x01,
    0x20, 0x20,
    0x21, 0x20,
    0x34, 0x02,
    0x35, 0x02,
    0x36, 0x21,
    0x37, 0x13,
#if (BF30A2_INPUT_FPS == 15)
    0xca, 0x02, //15ps
#else
    0xca, 0x03, //30ps
#endif
    0x17, START_ADDRW,
    0x18, END_ADDRW,
    0x19, START_ADDRH,
    0x1a, (END_ADDRH >> 1) & 0xFF,
    0x12, 0x10 | ((END_ADDRH & 0x1) << 7),
    /* 0xcf, 0x90, */
    0xcf, 0x05,
    0xcb, 0x22,
    0xcc, 0x89,
    0xcd, 0x4c,
    0xce, 0x6b,
    0xa0, 0x8e,
    0x01, 0x1b,
    0x02, 0x1d,
    0x13, 0x08,
    0x87, 0x13,
    0x8a, 0x33,
    0x8b, 0x08,
    0x70, 0x1f,
    0x71, 0x40,
    0x72, 0x0a,
    0x73, 0x62,
    0x74, 0xa2,
    0x75, 0xbf,
    0x76, 0x02,
    0x77, 0xcc,
    0x40, 0x32,
    0x41, 0x28,
    0x42, 0x26,
    0x43, 0x1d,
    0x44, 0x1a,
    0x45, 0x14,
    0x46, 0x11,
    0x47, 0x0f,
    0x48, 0x0e,
    0x49, 0x0d,
    0x4B, 0x0c,
    0x4C, 0x0b,
    0x4E, 0x0a,
    0x4F, 0x09,
    0x50, 0x09,
    0x24, 0x50,
    0x25, 0x36,
    0x80, 0x00,
    0x81, 0x20,
    0x82, 0x40,
    0x83, 0x30,
    0x84, 0x50,
    0x85, 0x30,
    0x86, 0xD8,
    0x89, 0x45,
    0x8f, 0x81,
    0x91, 0xff,
    0x92, 0x08,
    0x94, 0x82,
    0x95, 0xfd,
    0x9a, 0x20,
    0x9e, 0xbc,
    0xf0, 0x83,//动态帧率关闭 0x8f 打开 0x83关
    0x51, 0x06,
    0x52, 0x25,
    0x53, 0x2b,
    0x54, 0x0F,
    0x57, 0x2A,
    0x58, 0x22,
    0x59, 0x2c,
    0x23, 0x33,
    0xa0, 0x8f,
    0xa1, 0x93,
    0xa2, 0x0f,
    0xa3, 0x2a,
    0xa4, 0x08,
    0xa5, 0x26,
    0xa7, 0x80,
    0xa8, 0x80,
    0xa9, 0x1e,
    0xaa, 0x19,
    0xab, 0x18,
    0xae, 0x50,
    0xaf, 0x04,
    0xc8, 0x10,
    0xc9, 0x15,
    0xd3, 0x0c,
    0xd4, 0x16,
    0xee, 0x06,
    0xef, 0x04,
    0x55, 0x34,
    0x56, 0x9c,
    0xb1, 0x98,
    0xb2, 0x98,
    0xb3, 0xc4,
    0xb4, 0x0C,
    0x00, 0x40,
    0x13, 0x07,
};

static u8 read_bf30a2_reg(u8 reg_id)
{
    u8 data = 0;
    iic_start(IIC_ID);
    iic_tx_byte(IIC_ID, BF30A2_WRCMD);
    delay_nops(DELAY_TIME);
    iic_tx_byte(IIC_ID, reg_id);
    delay_nops(DELAY_TIME);
    iic_stop(IIC_ID);
    delay_nops(DELAY_TIME);
    iic_start(IIC_ID);
    delay_nops(DELAY_TIME);
    iic_tx_byte(IIC_ID, BF30A2_RDCMD);
    delay_nops(DELAY_TIME);
    data = iic_rx_byte(IIC_ID, 0, NULL);
    delay_nops(DELAY_TIME);
    iic_stop(IIC_ID);
    return data;
}

static void write_bf30a2_reg(u8 reg_id, u8 data)
{
    iic_start(IIC_ID);
    iic_tx_byte(IIC_ID, BF30A2_WRCMD);
    delay_nops(DELAY_TIME);
    iic_tx_byte(IIC_ID, reg_id);
    delay_nops(DELAY_TIME);
    iic_tx_byte(IIC_ID, data);
    delay_nops(DELAY_TIME);
    iic_stop(IIC_ID);
}

int bf30a2_check(void)
{
    int ret = -1;
    u16 pid = 0x00;
    u8 id0, id1;
    id0 = read_bf30a2_reg(0xfc);
    id1 = read_bf30a2_reg(0xfd);
    pid = (u16)((id0 << 8) | id1);
    printf("BF30A2 Sensor ID : 0x%x\n", pid);
    if (pid == 0x3b02) {
        ret = 0;
    }
    return ret;
}
int bf30a2_dev_check(void)
{
    int ret = -1;
    bf30a2_io_init();
    os_time_dly(2);
    ret = bf30a2_check();
    bf30a2_io_exit();
    return ret;
}
void bf30a2_config_sensor(void)
{
    int i;
    for (i = 0; i < ARRAY_SIZE(sensor_init_data); i++) {
        write_bf30a2_reg(sensor_init_data[i].reg, sensor_init_data[i].val);
    }
}

void bf30a2_out_sleep(void)
{
    write_bf30a2_reg(0xcf, 0x90);
}

void bf30a2_in_sleep(void)
{
    write_bf30a2_reg(0xcf, 0x05);
}


int bf30a2_io_init(void)
{
    // I2C INIT
    hw_iic_dev iic_dev = IIC_ID;

    struct iic_master_config iic_config = {
        .role = IIC_MASTER,
        .scl_io = IIC_SCL_IO,
        .sda_io = IIC_SDA_IO,
        .io_mode = PORT_INPUT_PULLUP_10K,//上拉或浮空
        .hdrive = PORT_DRIVE_STRENGT_2p4mA,   //enum GPIO_HDRIVE 0:2.4MA, 1:8MA, 2:26.4MA, 3:40MA
        .master_frequency = 50,
    };

    enum iic_state_enum iic_init_state = iic_init(iic_dev, &iic_config);
    if (iic_init_state != IIC_OK) {
        printf("iic(%d) master init fail", iic_dev);
        return -1;
    }

    //XCLK
    clk_out(SPI_XCLK_PORT, CLK_OUT_STD_24M, 0);
    //PWR IO
    gpio_set_mode(IO_PORT_SPILT(CAM_PWR_IO), PORT_OUTPUT_HIGH);

    return 0;
}
int bf30a2_camera_init(void)
{
    if (bf30a2_io_init()) {
        printf("------- bf30a2 io init fail------\n\n");
        return -1;
    }

    os_time_dly(2);

    if (bf30a2_check()) {
        printf("-------not bf30a2 camera------\n\n");
        return -1;
    }
    printf("-------hello bf30a2 camera------\n\n");

    bf30a2_config_sensor();

    return 0;
}
int bf30a2_io_exit(void)
{
    // I2C
    hw_iic_dev iic_dev = IIC_ID;

    enum iic_state_enum iic_init_state = iic_deinit(iic_dev);
    if (iic_init_state != IIC_OK) {
        printf("iic(%d) master deinit fail", iic_dev);
        return -1;
    }

    //XCLK
    clk_out_close(SPI_XCLK_PORT, CLK_OUT_STD_24M);

    //PWR IO
    gpio_set_mode(IO_PORT_SPILT(CAM_PWR_IO), PORT_OUTPUT_LOW);

    return 0;
}
int bf30a2_camera_exit(void)
{
    if (bf30a2_io_exit()) {
        printf("------ bf30a2 io deinit fail------\n\n");
        return -1;
    }
    return 0;
}



REGISTER_CAMERA_DEVICE(bf30a2_cfg)
{
    .fps = BF30A2_INPUT_FPS,
     .width  = BF30A2_INPUT_W,
      .height = BF30A2_INPUT_H,
       .check =  bf30a2_dev_check,
        .init = bf30a2_camera_init,
         .deinit = bf30a2_camera_exit,
          .supend = bf30a2_in_sleep,
           .resume = bf30a2_out_sleep,
};


#endif
