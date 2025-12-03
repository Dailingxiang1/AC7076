#ifdef SUPPORT_MS_EXTENSIONS_APP
#pragma bss_seg(".ex_flash_update.data.bss")
#pragma data_seg(".ex_flash_update.data")
#pragma const_seg(".ex_flash_update.text.const")
#pragma code_seg(".ex_flash_update.text")
#endif
#include "update_main.h"
#include "power/p33.h"
#include "common.h"
#include "norflash.h"
#include "device.h"
#include "jlfs.h"
#include "dec.h"
#include "gpio.h"
/* #include "uart_protocol_analyze.h" */

#define LOG_TAG_CONST       EX_NOR_UPDATE
#define LOG_TAG             "[EX_NOR_UPDATE]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

//============================================================//
/* APP NorFlash 升级模型(APP_NORFLASH_UPDATA)
  ________               ________              ________
 |        |             |        |            |        |
 |        |    ufw      |        |    ufw     |        |
 |   APP  | --------->  |  CHIP  | -------->  |NorFlash|
 |        |    ops      |        |    ops     |        |
 |________|             |________|            |________|
                            ^                      |
                            |______________________|
							         ota.bin
 */
//============================================================//

//需要获取信息:
//whick SPI HW
//which CS
//src addr, len
//dest addr, len
typedef struct _ex_flash_update_info {
    u32 src_file_addr;
    u32 src_file_len;
} ex_flash_update_info_parm;
static ex_flash_update_info_parm ex_update_info = {0};

#define EX_FLAHS_NEW_OPS  1

#if EX_FLAHS_NEW_OPS
static void *device;
static struct flash_platform_data ex_norflash_data = {
    .flash_type = DEV_FLASH_EXTERNAL_NORFLASH,
    .read_mode = 1,
    .power_io_level = 1,
    .spi_pdata = {
        .clk_div = 3,
    },
};
#else // EX_FLAHS_NEW_OPS
#include "ex_norflash.h"
#define EX_FLASH_USE_SFC  1
#if (EX_FLASH_USE_SFC == 0)
extern struct spi_platform_data spi_p_data;
static struct norflash_dev_platform_data  ex_norflash_data  = {
    .spi_hw_num = 0,                   //只支持SPI1或SPI2
    .spi_cs_port = IO_PORTC_03,        //cs的引脚
    .spi_read_width = 1,     //flash读数据的线宽
    .spi_pdata = &spi_p_data,
};
#endif
#endif // EX_FLAHS_NEW_OPS

static u8 ex_norflash_init = 0;
static u32 ex_nor_file_offset = 0;
static volatile u32 mutil_ufw_offset = 0;
extern struct flash_platform_data inside_flash_pdata;
extern int sfc_spi_init(u32 data_width, u32 read_mode, u32 div);
extern int sfc_spi_read(u32 addr, void *buf, u32 len);
void DcuInitial(void);
void IcuInitial(void);
void mdelay(u32 msec);


static u16 ex_flash_open(void)
{
    return !ex_norflash_init;
}

#if EX_FLAHS_NEW_OPS
static u16 ex_flash_read(void *fp, void *buf, u32 rlen)
{
    if (ex_norflash_init == 0) {
        return (u16) - 1;
    }
    u32 len;
    wdt_clear();

    if (device) {
        norflash_select_device(ex_norflash_data.spi_pdata.spi_idx, ex_norflash_data.spi_pdata.port);
        len = dev_bulk_read(device, buf, ex_update_info.src_file_addr + ex_nor_file_offset, rlen);
        norflash_select_device(0, inside_flash_pdata.spi_pdata.port);
    }
    return len;
}
#else // EX_FLAHS_NEW_OPS
static u16 ex_flash_read(void *fp, void *buf, u32 rlen)
{
    if (ex_norflash_init == 0) {
        return (u16) - 1;
    }
    u32 len;
    wdt_clear();

#if (EX_FLASH_USE_SFC == 0)
    len = _norflash_read(ex_update_info.src_file_addr + ex_nor_file_offset, buf, rlen);
#else
    len = sfc_spi_read(ex_update_info.src_file_addr + ex_nor_file_offset, buf, rlen);
#endif
    if (len != rlen) {
        return 0;
    }
    return len;
}
#endif // EX_FLAHS_NEW_OPS

static int ex_flash_seek(void *fp, u8 type, u32 offset)
{
    if (type == SEEK_SET) {
        offset += mutil_ufw_offset;
        ex_nor_file_offset = offset;
    } else if (type == SEEK_CUR) {
        ex_nor_file_offset += offset;
    }
    /* printf(">>>[test]:ex_nor_file_offset = %d\n", ex_nor_file_offset); */
    return 0;//FR_OK;
}

//ufw嵌套ufw格式处理
void mutil_cpu_set_offset(u32 offset)
{
    mutil_ufw_offset = offset;
    ex_nor_file_offset = mutil_ufw_offset; //预先设置好偏移
}

static void ex_flash_update_parm_set(void *priv)
{
    UPDATA_PARM *p  = (UPDATA_PARM *)priv;
    memcpy((u8 *)(&ex_update_info), p->parm_priv, sizeof(ex_flash_update_info_parm));
    put_buf(&ex_update_info, sizeof(ex_flash_update_info_parm));
    printf("\n >>>[test]:func = %s,line= %d, file_addr = %d\n", __FUNCTION__, __LINE__, ex_update_info.src_file_addr);
}

/* #if (EX_FLASH_USE_SFC == 0) */
static u8 ex_flash_find_delimiter(u8 *string)
{
    u8 i = 0;
    for (; i < strlen(string); i++) {
        if (string[i] == '_') {
            return i + 1;
        }
    }
    return 0;
}

static u32 ex_flash_get_cfg_from_isd_config(struct flash_platform_data *exflash_cfg)
{
    u8 *ptr = jlfs_get_isd_cfg_ptr();
    u8 val[32] = {0};
    memset(val, 0, sizeof(val));
    u8 *pdata = val;
    if (dec_isd_cfg_ini("EX_FLASH", val, ptr)) {
        exflash_cfg->spi_pdata.cs_port = get_gpio(pdata);
        pdata += ex_flash_find_delimiter(pdata);
        exflash_cfg->spi_pdata.spi_idx = pdata[0] - '0';
        exflash_cfg->spi_pdata.port = pdata[1] - 'A';
        pdata += ex_flash_find_delimiter(pdata);
        exflash_cfg->power_io = get_gpio(pdata);
    } else {
        return 0;
    }

    pdata = val;
    if (dec_isd_cfg_ini("EX_FLASH_IO", val, ptr)) {
        exflash_cfg->spi_pdata.width = pdata[0] - '0';
        pdata += ex_flash_find_delimiter(pdata);
        exflash_cfg->spi_pdata.clk_port = get_gpio(pdata);
        pdata += ex_flash_find_delimiter(pdata);
        exflash_cfg->spi_pdata.do_port = get_gpio(pdata);
        pdata += ex_flash_find_delimiter(pdata);
        exflash_cfg->spi_pdata.di_port = get_gpio(pdata);
        pdata += ex_flash_find_delimiter(pdata);
        exflash_cfg->spi_pdata.d2_port = get_gpio(pdata);
        pdata += ex_flash_find_delimiter(pdata);
        exflash_cfg->spi_pdata.d3_port = get_gpio(pdata);
    } else {
        return 0;
    }

    return 1;
}
/* #endif */

#if EX_FLAHS_NEW_OPS
int ex_flash_update_init(void)
{
    int ret = -1;
    if (ex_flash_get_cfg_from_isd_config(&ex_norflash_data)) {
        device = dev_open("norflash", &ex_norflash_data);
        if (device) {
            ex_norflash_init = 1;
            ret = 0;
        } else {
            printf("ex flash open fail\n");
        }
    }
    return ret;
}
#else // EX_FLAHS_NEW_OPS
/* u8 t[1024]; */
void sfc1_io_init(u32 data_width);
int ex_flash_update_init(void)
{
    sfc1_io_init(4);
    if (ex_flash_get_cfg_from_isd_config(&ex_norflash_data)) {
        //flash POWER ON
        if (ex_norflash_data.spi_power_pin < IO_PORT_MAX) {
            gpio_set_direction(ex_norflash_data.spi_power_pin, 0);
            gpio_set_output_value(ex_norflash_data.spi_power_pin, 1);
        }

        log_info("spi_power_pin = %d\n", ex_norflash_data.spi_power_pin);
        log_info("spi_cs_pin = %d\n", ex_norflash_data.spi_cs_port);
        log_info("spi_hw_num = %d\n", ex_norflash_data.spi_hw_num);
        log_info("spi_port_num = %d\n", ex_norflash_data.spi_pdata->port);

        _norflash_init(NULL, &ex_norflash_data);
        _norflash_open();
        ex_norflash_init = 1;
    } else {
        log_info("EXFLASH tag find fail\n");
        return -1;
    }

#if EX_FLASH_USE_SFC
    /* JL_PORTC->OUT |= BIT(8); */
    /* JL_PORTC->HD |= BIT(8); */
    /* JL_PORTC->HD0 |= BIT(8); */
    /* JL_PORTC->DIR &= ~BIT(8); */

    sfc_spi_init(4, 0, 3);
    mdelay(20);

    IcuInitial();
    DcuInitial();

    /* sfc_spi_read(0, t, 512); */
    /* put_buf(t, 512); */
    /* printf(">>>[test]:aaaaaaaaaaaaa!\n"); */
    /* sfc_spi_read(512, t, 512); */
    /* put_buf(t, 512); */
    /* printf(">>>[test]:bbbbbbbb!\n"); */
    /* sfc_spi_read(512 * 2, t, 512); */
    /* put_buf(t, 512); */
    /* printf(">>>[test]:cccccccc!\n"); */
    /* sfc_spi_read(512 * 3, t, 512); */
    /* put_buf(t, 512); */
    /* printf(">>>[test]:dddddddd!\n"); */
    /* while(1); */

    ex_norflash_init = 1;
    return 0;
#else
    return 0;
#endif
}
#endif // EX_FLAHS_NEW_OPS

#if (EX_FLASH_USE_SFC == 0)
static void ex_flash_close(void)
{
    if (ex_norflash_init) {
#if (0 == EX_FLAHS_NEW_OPS)
        _norflash_close();
#endif // EX_FLAHS_NEW_OPS
        ex_norflash_init = 0;
    }
}
#endif

void ex_flash_update_state_cbk(u32 status, void *priv)
{
    switch (status) {
    case UPDATE_PARM:       //升级需要的参数
        log_info("EX_FLASH_UPDATE_SPARM_SET...\n");
        ex_flash_update_parm_set(priv);
        break;
    case UPDATE_START:      //测试盒edr升级需要根据priv判断是否恢复基带
        log_info("EX_FLASH_UPDATE_START...\n");
        ex_flash_update_init();
        mdelay(20);		//初始化后做延时，否则可能因为不稳定导致读取外置flash数据有问题
        norflash_select_device(0, inside_flash_pdata.spi_pdata.port);		//后续需要获取内置flash数据的话，这里需要做切换
        break;
    case UPDATE_END:		//升级结束需要保存结果到Ram给SDK获取，并回复主机升级结果
        if (*((u8 *)priv) == UPDATE_ERR_NONE) {
            set_updata_result(USER_NORFLASH_UFW_UPDATA, UPDATA_SUCCESSFULLY);
        } else {
            set_updata_result(USER_NORFLASH_UFW_UPDATA, UPDATA_DEV_ERR);
        }
        /* ex_flash_update_result_report(*((u8 *)priv)); */
#if (EX_FLASH_USE_SFC == 0)
        ex_flash_close();
#endif
        /* cpu_reset(); */
        update_reset();
        break;
    }
}

update_op_api_t ex_flash_op_api = {
    .f_open = ex_flash_open,
    .f_read = ex_flash_read,
    .f_seek = ex_flash_seek,
    .notify_update_content_size = NULL,
};

update_mode_info_t update_mode_info = {
    .type      = USER_NORFLASH_UFW_UPDATA,
    .state_cbk = ex_flash_update_state_cbk,
    .file_op   = &ex_flash_op_api,
};

#if EX_FLAHS_NEW_OPS
REGISTER_DEVICES(device_table) = {
    {"norflash", &norflash_dev_ops, NULL},
};
#endif // EX_FLAHS_NEW_OPS

