#ifndef  __NORFLASH_H__
#define  __NORFLASH_H__

#include "typedef.h"

//spi_width:
#define    SPI_MODE_UNIDIR_1BIT   0//半双工，do分时发送/接收
#define    SPI_MODE_BIDIR_1BIT    1//全双工，di接收，do发送
#define    SPI_MODE_UNIDIR_2BIT   2//半双工，di & do共2bit分时发送/接收
#define    SPI_MODE_UNIDIR_4BIT   4//半双工，di & do & wp & hold 共4bit分时发送/接收


#define WINBOND_WRITE_ENABLE                                0x06
#define WINBOND_WRITE_SR_ENABLE                             0x50
#define WINBOND_JEDEC_ID                                    0x9F
#define WINBOND_UUID                                        0x4B
#define WINBOND_READ_SR1                                    0x05
#define WINBOND_READ_SR2                                    0x35
#define WINBOND_READ_SR3                                    0x15
#define WINBOND_WRITE_SR1                                   0x01
#define WINBOND_WRITE_SR2                                   0x31
#define WINBOND_WRITE_SR3                                   0x11
#define WINBOND_FAST_READ                                   0x0B
#define WINBOND_FAST_READ_DUAL_OUTPUT                       0x3B
#define WINBOND_FAST_READ_QUAD_OUTPUT                       0x6B
#define WINBOND_FAST_READ_DUAL_IO                           0xBB
#define WINBOND_FAST_READ_QUAD_IO                           0xEB
#define WINBOND_CONTINUOUS_READ_NORMAL_MODE                 0x00
#define WINBOND_CONTINUOUS_READ_ENHANCE_MODE                0x20
#define WINBOND_PAGE_PROGRAM                                0x02
#define WINBOND_PAGE_ERASE                                  0x81
#define WINBOND_SECTOR_ERASE                                0x20
#define WINBOND_BLOCK_ERASE                                 0xD8
#define WINBOND_CHIP_ERASE                                  0xC7
#define WINBOND_POWER_DOWN		                            0xB9
#define WINBOND_RELEASE			                            0xAB


struct flash_info_t {
    u8 uuid[16];
    u32 id;
    u32 capacity;
};

struct flash_wp_arg {
    u32 id;
    u8 split_mode: 1;       //0 连续写，1，分开写
    u8 write_en_use_50h: 1; //0:06H     1:50H
    u8 res: 2;
    u8 numOfwp_array: 4;    //写保护参数的个数
    struct {
        u8 sr1;
        u8 sr2;
        u16 wp_addr;        //写保护结束地址,单位K
    } wp_array[15];
} __attribute__((packed));

struct custom_data_t {
    u16 u16crc;
    u16 len;
    u8 otp_mode;//0xa0:flash_mode   0xa1:otp mode
    union {
        struct {
            u8 wr_sr_cmd[2];//寄存器的写命令，当两个命令是一样的，就使用连续写模式
            u8 otp_lock_sr1;
            u8 otp_lock_sr1_mask;//mask ==0 就忽略此寄存器
            u8 otp_lock_sr2;
            u8 otp_lock_sr2_mask;//mask ==0 就忽略此寄存器

            u16 otp_page_num;
            u16 otp_page_size;
            u32 otp_offset[5];
        } __attribute__((packed))  otp_info;
        u8 user_info_data[59];
    } __attribute__((packed)) u;
} __attribute__((packed));

struct spi_platform_data {
    u8 spi_idx;
    u8 width;
    u8 clk_div;
    u8 port;
    u8 cs_port;
    u8 clk_port;
    u8 do_port;
    u8 di_port;
    u8 d2_port;
    u8 d3_port;
};

enum DEV_FLASH_TYPE {
    DEV_FLASH_INTERNAL_NORFLASH,
    DEV_FLASH_EXTERNAL_NORFLASH,
    DEV_FLASH_NANDFLASH,
    DEV_FLASH_NANDFLASH_FTL,
};

struct flash_platform_data {
    enum DEV_FLASH_TYPE flash_type;
    u8 read_mode;
    u8 power_io;
    u8 power_io_level;
    u8 power_ldo_io;
    u8 power_ldo_io_level;
    u8 qe_position;
    struct spi_platform_data spi_pdata;
};

struct flash_info_t *get_flash_info(void);

void flash_poweron_base(u32 port, u32 out0_time, u32 poweron_time, void (*custom_udelay)(u32), void (*custom_mdelay)(u32));
void flash_poweroff_base(u32 port, u32 out0_time, u32 discharge_time, void (*custom_udelay)(u32), void (*custom_mdelay)(u32));

u8 get_flash_is4byte_mode(void);
u32 get_flash_capacity(void);
u32 norflash_get_id(void);
void norflash_get_uuid(u8 *buffer);
u16 norflash_cmd_custom(void);

void norflash_read_status_reg(u8 *status_reg1, u8 *status_reg2);
u8 norflash_spi_write_sr_reg(u8 reg1, u8 reg2) ;
int norflash_write_protect(u8 sr_mode, u8 wr_en_mode, u32 sr1, u32 sr2);
int norflash_write_protect_config(u32 addr, const struct flash_wp_arg *p);

#define TZFLASH_STA_NUM 4
struct flash_reg {
    u8 cmd[4];
    u8 sr_value[4];
    u8 sr_mask[4];
    u8 num_of_reg: 2;
    u8 wr_en_mode: 1;
    u8 continue_mode: 4;
    u8 rev: 1;
};

struct flash_func_len {
    u16 all_len;
    u8 version;
    u8 func_len;//lb
    u8 sta_len;//lb
    u8 func_otp;//lb
    u8 func_dtr;
    u8 func_wp;//bp,tb,sec,cmp
    u8 func_wps;
    u8 func_wps_op;
    u8 func_qe;
    u8 func_srp;
    u8 func_sus;
    u8 func_dc;
    u8 func_drv;
    u8 func_mpm;//dp

    u8 func_qpi;
    u8 func_init;
    u8 func_read_continue;
} __attribute__((packed));

#define TZFLASH_STA_NUM 4
struct flash_sta_cmd {
    u8 sta_cmd_r[TZFLASH_STA_NUM];
    u8 sta_cmd_w[TZFLASH_STA_NUM];
} __attribute__((packed));

#define  FLASH_SUPPORT_MPM_FUNC 0
struct flash_dev_sta_info {
    struct flash_func_len func_cfg;
    struct flash_sta_cmd sta_cfg;
    u8 *cfg_ptr;
};

struct flash_otp_cfg {//flash otp信息
    u32 otp_offset[5];//otp page的偏移地址组数
    u16 otp_page_size;//otp的page大小
    u8 otp_NumberOfpage;//otp的page数量
    // struct flash_reg lock_cfg[2];//read,write
    u8 wr_en_cmd;//0x50h:sr,0x06h:wr_en
    u8 sr_mask[TZFLASH_STA_NUM];
    u8 sr_value[TZFLASH_STA_NUM];//注意:锁定后不可擦写,sr的值也常是1
} __attribute__((packed));

struct flash_wps_cfg {//flash wps信息
    u8 wr_en_cmd;//0x50h:sr,0x06h:wr_en
    u8 sr_mask[TZFLASH_STA_NUM];
    u8 sr_value[TZFLASH_STA_NUM];
    // struct flash_reg r_reg_cfg;//read
    // struct flash_reg w_reg_cfg;//write
} __attribute__((packed));

struct flash_wp_cfg {   //写保护配置信息
    u8 numOfwp_array;//写保护参数的个数
    u8 wr_en_cmd;//0x50h:sr,06h:
    u8 sr_mask[TZFLASH_STA_NUM]; //sr要保留或修改的bit
    struct {
        u8 sr_value[TZFLASH_STA_NUM]; //写保护sr取值
        u16 wp_addr;//写保护结束地址,单位K
    } wp_array[0]; //写保护的组数，修改可变长
} __attribute__((packed));

struct flash_dtr_cfg {
    s8 x1_dummy;//-1:使用默认值
    s8 x2_dummy;//-1:使用默认值
    s8 x4_dummy;//-1:使用默认值,-2:不支持
} __attribute__((packed));

struct flash_reg_param {
    int reg_mask;//需配置的reg映射
    u8 sr_mask[TZFLASH_STA_NUM];
    u8 sr_value[TZFLASH_STA_NUM];
    u8 wr_en_cmd;//0x50h:sr,0x06h:wr_en
};

struct wps_addr_info {
    u8 lock_en;
    u32 wp_saddr;
    u32 wp_eaddr;
};

int norflash_cfg_wpl(struct flash_wps_cfg *cfg);
int norflash_set_wps_addr(u8 lock, u32 saddr, u32 eaddr);
int norflash_params_v3_wp(u32 wp_addr, int index, struct flash_wp_cfg *cfg);
int norflash_get_flash_v3_wp_func_value(u8 *cfg_buf);
int norflash_get_flash_v3_dtr_func_value(u8 *cfg_buf);

void norflash_exit_continue_mode(void);
#ifdef  __SFC_DTR
void __dtr_norflash_exit_continue_mode(void);
#endif
int norflash_read(u8 *buf, u32 addr, u32 len);
int norflash_write(u8 *buf, u32 addr, u32 len);
int norflash_write_async(u8 *buf, u32 offset, u32 len);
u8 norflash_eraser(u32 eraser_cmd, u32 addr);

u8 norflash_read_otp_lb(void);
u32 norflash_erase_otp(u32 page);
u32 norflash_write_otp(u8 *buffer, u32 len, u32 page, u32 offset);
void norflash_read_otp(u8 *buffer, u32 len, u32 page, u32 offset);

void norflash_read_otp_page(u8 *buffer, u32 len, u32 addr);

#ifdef BFLT_AUTH_BURN
u32 bflt_norflash_read_otp(u8 *buffer, u32 len, u32 offset) ;
u32 bflt_norflash_write_otp(u8 *buffer, u32 len, u32 offset) ;
u32 bflt_norflash_erase_otp(u32 offset) ;
u32 norflash_otp_lock(u8 *custom) ;
#endif

void norflash_enter_powerdown(void);
void norflash_powerdown_release(u32 delay);
void norflash_soft_reset(u32 delay);

void norflash_init(struct flash_platform_data *pdata);

void norflash_read_bt_mac(u8 *mac);

int norflash_ioctl(u32 cmd, u32 arg);

void norflash_select_device(u32 index, u32 port);

u8 P25QXXH_flash_close_quad_page_mode();

extern const struct device_operations norflash_dev_ops;

#endif

