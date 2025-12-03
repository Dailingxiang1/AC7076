#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".board_config.data.bss")
#pragma data_seg(".board_config.data")
#pragma const_seg(".board_config.text.const")
#pragma code_seg(".board_config.text")
#endif


#include "app_config.h"

#ifdef CONFIG_BOARD_JL7074_DEMO

#include "system/includes.h"
#include "app_main.h"
#include "rtc/rtc.h"
#include "spi.h"
#include "asm/sdmmc.h"
#include "asm/spi_hw.h"
#include "asm/lpctmu_hw.h"
#include "asm/psram_api.h"
#include "linein_dev.h"
#include "usb/device/usb_stack.h"
#include "usb/host/usb_storage.h"
#include "ui/ui_api.h"
#include "ui_manage.h"
#include "iic_api.h"
#include "ui/lcd/lcd_drive.h"
#include "include/norflash_sfc.h"
#include "fs/virfat_flash.h"
#include "alarm.h"
#include "data_storage.h"
#include "tp_api.h"
#include "gpadc.h"
#include "rdec_key.h"
#include "asm/espi.h"
#include "../sdk_config.c"

#if (CONFIG_BT_MODE != BT_NORMAL) || TCFG_NORMAL_SET_DUT_MODE
#if ((TCFG_RDEC0_ECODEA_PORT==IO_PORT_DP) || (TCFG_RDEC0_ECODEA_PORT==IO_PORT_DM)) || \
	((TCFG_RDEC0_ECODEB_PORT==IO_PORT_DP) || (TCFG_RDEC0_ECODEB_PORT==IO_PORT_DM))
// 蓝牙测试会用到DPDM
#undef TCFG_RDEC_KEY_ENABLE
#define TCFG_RDEC_KEY_ENABLE					DISABLE_THIS_MOUDLE
#endif
#endif


#if TCFG_APP_RTC_EN
static struct sys_time def_sys_time = {  //初始化系统时间
    .year = 2020,
    .month = 1,
    .day = 1,
    .hour = 0,
    .min = 0,
    .sec = 0,
};

static struct sys_time def_alarm = {     //初始化闹钟时间
    .year = 2020,
    .month = 1,
    .day = 1,
    .hour = 0,
    .min = 5,
    .sec = 0,
};

void rtc_event_isr(u32 event)    //闹钟回调函数测试
{
    if (event == MSYS_ALARM_WKUP_EVENT) {
        alm_wakeup_isr();
    } else if (event == MSYS_RTC_1HZ_EVENT) {

    }
}

struct rtc_config_init rtc_dev_config = {   //RTC初始化结构体
    .default_sys_time = &def_sys_time,   //配置默认系统时钟
    .default_alarm = &def_alarm,        //配置默认闹钟
    .rtc_clk = CLK_SEL_LRC,       // 配置时钟源

    .alm_en = 1,            //闹钟使能
    .cbfun = rtc_event_isr,
    //rtc闹钟回调函数
};
#endif

#if TCFG_PSRAM_DEV_ENABLE
PSRAM_PLATFORM_DATA_BEGIN(psram_config)
.power_port = TCFG_PSRAM_POWER_PORT,//power io
 .port = TCFG_PSRAM_PORT_SEL	,//默认A口
  .mode = TCFG_PSRAM_MODE,//4线读写
   .init_clk = TCFG_PSRAM_INIT_CLK,//hz
    PSRAM_PLATFORM_DATA_END()
#endif//TCFG_PSRAM_DEV_ENABLE

#if TCFG_SD0_ENABLE

    extern void sdpg_config(int enable);

#define SDX_POWER_ALONE		1

#if (TCFG_SD0_POWER_SEL == SD_PWR_SDPG)
#define sd_power_config		sdpg_config
#else
void sd_power_config(int enable)
{
    //TODO: 使用普通io供电
    if (enable) {
        /* printf("\n\n sd power enable \n\n"); */
        gpio_set_mode(IO_PORT_SPILT(TCFG_SD0_POWER_PORT), PORT_OUTPUT_LOW);
    } else {
        /* printf("\n\n sd power disable \n\n"); */
        gpio_set_mode(IO_PORT_SPILT(TCFG_SD0_POWER_PORT), PORT_HIGHZ);
    }
}
#endif

#if SDX_POWER_ALONE
static u8 sdx_power_enable = 0xff;
void sd_set_power(u8 enable)
{
    /* enable = !!enable; */
    // y_printf("sd_set_power:%d, %d \n", sdx_power_enable, enable);
    if (sdx_power_enable == enable) {
        return;
    }
    sd_power_config(enable);
    sdx_power_enable = enable;
}
#endif /* #if SDX_POWER_ALONE */

#if TCFG_SD_ALWAY_ONLINE_ENABLE
int sdmmc_0_io_detect(const struct sdmmc_platform_data *data)
{
    return 1;
}
#endif /* #if TCFG_SD_ALWAY_ONLINE_ENABLE */

SD0_PLATFORM_DATA_BEGIN(sd0_data) = {
    .port = {
        TCFG_SD0_PORT_CMD,
        TCFG_SD0_PORT_CLK,
        TCFG_SD0_PORT_DA0,
        TCFG_SD0_PORT_DA1,
        TCFG_SD0_PORT_DA2,
        TCFG_SD0_PORT_DA3,
    },
    .data_width             = TCFG_SD0_DAT_MODE,
    .speed                  = TCFG_SD0_CLK,
    .detect_mode            = TCFG_SD0_DET_MODE,
    .priority				= 3,

#if (TCFG_SD0_DET_MODE == SD_IO_DECT)
    .detect_io              = TCFG_SD0_DET_IO,
    .detect_io_level        = TCFG_SD0_DET_IO_LEVEL,
    .detect_func            = sdmmc_0_io_detect,
    .power                  = sd_set_power,
    /* .power                  = NULL, */
#elif (TCFG_SD0_DET_MODE == SD_CLK_DECT)
    .detect_io_level        = TCFG_SD0_DET_IO_LEVEL,
    .detect_func            = sdmmc_0_clk_detect,
    .power                  = sd_set_power,
    /* .power                  = NULL, */
#else
    .detect_func            = sdmmc_cmd_detect,
    .power                  = NULL,
#endif

    SD0_PLATFORM_DATA_END()
};
#endif /* #if TCFG_SD0_ENABLE */


// *INDENT-OFF*
#if TCFG_LPCTMU_ENABLE
LPCTMU_PLATFORM_DATA_BEGIN(lpctmu_pdata)
#if LPCTMU_ANA_CFG_ADAPTIVE
    .aim_vol_delta              = 800,
    .aim_charge_khz             = 7,//2500,
#else
    .hv_level                   = 3,
    .lv_level                   = 0,
    .cur_level                  = 7,
#endif
LPCTMU_PLATFORM_DATA_END();

LPCTMU_CFG_DATA_BEGIN(lpctmu_cfg)
    .ch_en = ((TCFG_LPCTMU_CH0_EN << 0) | \
              (TCFG_LPCTMU_CH1_EN << 1) | \
              (TCFG_LPCTMU_CH2_EN << 2) | \
              (TCFG_LPCTMU_CH3_EN << 3) | \
              (TCFG_LPCTMU_CH4_EN << 4)),
    .pdata = &lpctmu_pdata,
LPCTMU_CFG_DATA_END();

#endif


/************************** linein KEY ****************************/
#if TCFG_APP_LINEIN_EN
struct linein_dev_data linein_data = {
    .enable = TCFG_APP_LINEIN_EN,
    .port   = NO_CONFIG_PORT,
    .up     = 1,
    .down   = 0,
    .ad_channel = NO_CONFIG_PORT,
    .ad_vol = 0,
};
#endif

#if TCFG_UI_ENABLE

#if TCFG_SPI_LCD_ENABLE
//推屏使用有专门硬件模块,不是普通spi模块,io固定,根据屏幕驱动类似输出时序
LCD_SPI_PLATFORM_DATA_BEGIN(lcd_spi_data) = {
    .pin_reset = IO_PORTA_05,
    .pin_en = IO_PORTA_03,
    .pin_en_ex = IO_PORTC_02,
    .pin_te = TCFG_LCD_TE_IO,
    .pin_bl = TCFG_LCD_BL_IO,

    LCD_SPI_PLATFORM_DATA_END()
};

const struct ui_devices_cfg ui_cfg_data = {
    .type = TFT_LCD,
    .private_data = (void *) &lcd_spi_data,
};
#endif /*TCFG_SPI_LCD_ENABLE*/

#endif /*TCFG_UI_ENABLE*/

const struct iic_master_config soft_iic_cfg_const[MAX_SOFT_IIC_NUM] = {
    //soft iic0
#if 0
    {
        .role = IIC_MASTER,
        .scl_io = TCFG_SW_I2C0_CLK_PORT,
        .sda_io = TCFG_SW_I2C0_DAT_PORT,
        .io_mode = PORT_INPUT_PULLUP_10K,      //上拉或浮空，如果外部电路没有焊接上拉电阻需要置上拉
        .hdrive = PORT_DRIVE_STRENGT_2p4mA,    //IO口强驱
        .master_frequency = TCFG_SW_I2C0_DELAY_CNT,  //IIC通讯波特率 未使用
        .io_filter = 0,                        //软件iic无滤波器
    },
#endif
#if 0
    //soft iic1
    {
        .role = IIC_MASTER,
        .scl_io = TCFG_SW_I2C1_CLK_PORT,
        .sda_io = TCFG_SW_I2C1_DAT_PORT,
        .io_mode = PORT_INPUT_PULLUP_10K,      //上拉或浮空，如果外部电路没有焊接上拉电阻需要置上拉
        .hdrive = PORT_DRIVE_STRENGT_2p4mA,    //IO口强驱
        .master_frequency = TCFG_SW_I2C1_DELAY_CNT,  //IIC通讯波特率 未使用
        .io_filter = 0,                        //软件iic无滤波器
    },
#endif
};

const struct iic_master_config hw_iic_cfg_const[MAX_HW_IIC_NUM] = {
    {
        .role = IIC_MASTER,
        .scl_io = TCFG_HW_I2C0_CLK_PORT,
        .sda_io = TCFG_HW_I2C0_DAT_PORT,
        .io_mode = PORT_INPUT_PULLUP_10K,      //上拉或浮空，如果外部电路没有焊接上拉电阻需要置上拉
        .hdrive = PORT_DRIVE_STRENGT_2p4mA,    //IO口强驱
        .master_frequency = TCFG_HW_I2C0_CLK,  //IIC通讯波特率
        .io_filter = 1,                        //是否打开滤波器（去纹波）
    },//iic0
    {
        .role = IIC_MASTER,
        .scl_io = TCFG_HW_I2C_P11_CLK_PORT,
        .sda_io = TCFG_HW_I2C_P11_DAT_PORT,
        .io_mode = PORT_INPUT_PULLUP_10K,      //上拉或浮空，如果外部电路没有焊接上拉电阻需要置上拉
        .hdrive = PORT_DRIVE_STRENGT_2p4mA,    //IO口强驱
        .master_frequency = TCFG_HW_I2C_P11_CLK,  //IIC通讯波特率
        .io_filter = 1,                        //是否打开滤波器（去纹波）
    },//iic_p11
};

#if (TCFG_HW_SPI1_ENABLE || TCFG_HW_SPI2_ENABLE)
const struct spi_platform_data spix_p_data[HW_SPI_MAX_NUM] = {
    {
        //spi0
    },
#if 0
    {
        //spi1
        .port = {
            TCFG_HW_SPI1_PORT_CLK, //clk any io
            TCFG_HW_SPI1_PORT_DO, //do any io
            TCFG_HW_SPI1_PORT_DI, //di any io
            0xff, //d2 any io
            0xff, //d3 any io
            0xff, //cs any io(主机不操作cs)
        },
        .role = TCFG_HW_SPI1_ROLE,//SPI_ROLE_MASTER,
        .clk  = TCFG_HW_SPI1_BAUD,
        .mode = TCFG_HW_SPI1_MODE,//SPI_MODE_BIDIR_1BIT,//SPI_MODE_UNIDIR_2BIT,
        .bit_mode = SPI_FIRST_BIT_MSB,
        .cpol = 0,//clk level in idle state:0:low,  1:high
        .cpha = 0,//sampling edge:0:first,  1:second
        .ie_en = 0, //ie enbale:0:disable,  1:enable
        .irq_priority = 3,
        .spi_isr_callback = NULL,  //spi isr callback
    },
#endif
#if SUPPORT_SPI2
    {
        //spi2
        .port = {
            TCFG_HW_SPI2_PORT_CLK, //clk any io
            TCFG_HW_SPI2_PORT_DO, //do any io
            TCFG_HW_SPI2_PORT_DI, //di any io
            0xff, //d2 any io
            0xff, //d3 any io
            0xff, //cs any io(主机不操作cs)
        },
        .role = TCFG_HW_SPI2_ROLE,//SPI_ROLE_MASTER,
        .clk  = TCFG_HW_SPI2_BAUD,
        .mode = TCFG_HW_SPI2_MODE,//SPI_MODE_BIDIR_1BIT,//SPI_MODE_UNIDIR_2BIT,
        .bit_mode = SPI_FIRST_BIT_MSB,
        .cpol = 0,//clk level in idle state:0:low,  1:high
        .cpha = 0,//sampling edge:0:first,  1:second
        .ie_en = 0, //ie enbale:0:disable,  1:enable
        .irq_priority = 3,
        .spi_isr_callback = NULL,  //spi isr callback
    },
#endif
};
#endif

#if TCFG_NANDFLASH_DEV_ENABLE
#include "nandflash.h"
NANDFLASH_DEV_PLATFORM_DATA_BEGIN(nandflash_dev_data) = {
    .spi_hw_num     = TCFG_FLASH_DEV_SPI_HW_NUM,
    .spi_cs_port    = TCFG_FLASH_DEV_SPI_CS_PORT,
    .spi_read_width = TCFG_FLASH_DEV_FLASH_READ_WIDTH,//flash读数据的线宽
    .start_addr     = 0,
    .size           = 256 * 1024 * 1024,
#if (TCFG_FLASH_DEV_SPI_HW_NUM == 1)
    .spi_pdata      = &spi1_p_data,
#elif (TCFG_FLASH_DEV_SPI_HW_NUM == 2)
    .spi_pdata      = &spi2_p_data,
#endif
};
#endif

const u32 g_res_nor_unencry_start_addr = 0; //temp, todo...

#if TCFG_SDFILE_INSERT_FLASH_ENABLE
NORFLASH_SFC_DEV_PLATFORM_DATA_BEGIN(norflash_norfs_inside_dev_mode_data)
.path                 = (const u8 *)"mnt/sdfile/app/MODE",
 .start_addr           = TCFG_MODE_INSERT_FLASH_BASE,
  .size     = TCFG_MODE_INSERT_FLASH_SIZE,
   NORFLASH_SFC_DEV_PLATFORM_DATA_END()
#endif


#if TCFG_VIRFAT_INSERT_FLASH_ENABLE
   NORFLASH_SFC_DEV_PLATFORM_DATA_BEGIN(norflash_norfs_inside_dev_data)
   .path                 = (const u8 *)"mnt/sdfile/app/FATFSI",
    .start_addr           = TCFG_WATCH_INSERT_FLASH_BASE,
     .size     = TCFG_WATCH_INSERT_FLASH_SIZE,
      NORFLASH_SFC_DEV_PLATFORM_DATA_END()
#endif

#if TCFG_DATA_STORAGE_FDB_ENABLE
      NORFLASH_SFC_DEV_PLATFORM_DATA_BEGIN(norflash_norfs_inside_watch_data_dev_data)
      .start_addr           = TCFG_DATA_INSERT_FLASH_BASE,
       .size     = TCFG_DATA_INSERT_FLASH_SIZE,
        NORFLASH_SFC_DEV_PLATFORM_DATA_END()
#endif



        //这里是外挂flash分区的配置
#if TCFG_NORFLASH_SFC_DEV_ENABLE
        SFC_SPI_PLATFORM_DATA_BEGIN(sfc_spi_data)
        .sfc_data_width  = SFC_DATA_WIDTH_4,
         .sfc_read_mode   = SFC_RD_IO,
          SFC_SPI_PLATFORM_DATA_END()

          NORFLASH_SFC_DEV_PLATFORM_DATA_BEGIN(norflash_sfc_dev_data)
          .sfc_spi_pdata     = &sfc_spi_data,
           .start_addr     = 0,
            .size           = CONFIG_EXTERN_FLASH_SIZE,
             NORFLASH_SFC_DEV_PLATFORM_DATA_END()
#endif



REGISTER_DEVICES(device_table) = {
#if TCFG_SD0_ENABLE
    { "sd0", 	&sd_dev_ops,	(void *) &sd0_data},
#endif
#if TCFG_APP_LINEIN_EN
    { "linein",  &linein_dev_ops, (void *) &linein_data},
#endif

#if TCFG_UDISK_ENABLE
    { "udisk0",   &mass_storage_ops, NULL},
#endif
#if TCFG_NANDFLASH_DEV_ENABLE
    {"nand_flash",   &nandflash_dev_ops, (void *) &nandflash_dev_data},
    {"nandflash_ftl",   &ftl_dev_ops, NULL },
#endif

#if TCFG_VIRFAT_FLASH_ENABLE
    //虚拟文件系统对接jl sdfile fat 文件系统设备入口,往下对接文件系统，往上对接物理设备
    { "virfat_flash", 	&virfat_flash_dev_ops,	(void *)"res_nor"},
    //res_nor 是物理设备入口
#endif

#if TCFG_VIRFAT_INSERT_FLASH_ENABLE
    //使用内置flash  跑ui
    { "res_nor",   &inside_norflash_fs_dev_ops, (void *) &norflash_norfs_inside_dev_data},
#endif//TCFG_VIRFAT_INSERT_FLASH_ENABLE

#if TCFG_SDFILE_INSERT_FLASH_ENABLE
    //内置只读文件系统
    { "res_nor_mode",   &inside_norflash_fs_dev_ops, (void *) &norflash_norfs_inside_dev_mode_data},
#endif

#if TCFG_NORFLASH_SFC_DEV_ENABLE
#if TCFG_VIRFAT_EXT_FLASH_ENABLE
    //使用外挂flash  跑ui
    { "res_nor",   &norflash_sfc_fs_dev_ops, (void *) &norflash_sfc_dev_data},
#endif
#endif /*TCFG_NORFLASH_SFC_DEV_ENABLE*/




#if TCFG_DATA_STORAGE_FDB_ENABLE
    { TCFG_DATA_DEV_NAME,   &inside_norflash_fs_dev_ops, (void *) &norflash_norfs_inside_watch_data_dev_data},
#endif//TCFG_DATA_STORAGE_FDB_ENABLE




};


#if TCFG_SD_ALWAY_ONLINE_ENABLE

extern int sdx_dev_entry_lowpower(const char *sdx_name);

static int sdx_entry_lowpower(int param)
{
    /* putchar('s'); */
#if TCFG_SD0_ENABLE
    sdx_dev_entry_lowpower("sd0");
#endif /* #if TCFG_SD1_ENABLE */
#if TCFG_SD1_ENABLE
    sdx_dev_entry_lowpower("sd1");
#endif /* #if TCFG_SD1_ENABLE */
    return 0;
}
static void sdx_sleep_callback(void)
{
    int argv[3];

    argv[0] = (int)sdx_entry_lowpower;
    argv[1] = 1;
    /* argv[2] = ; */

    int ret = os_taskq_post_type("app_core", Q_CALLBACK, 3, argv);
    if (ret) {
        printf("post ret:%d \n", ret);
    }
}

#else /*#if TCFG_SD_ALWAY_ONLINE_ENABLE*/

extern void sdx_dev_detect_modify(u32 modify_time);
void sniff_hook(u32 slot, u8 num, u8 first_conn, int t_sniff)
{
    if (num) {
        if (first_conn) {
            /* printf("..%d..\n", t_sniff); */
            sdx_dev_detect_modify(t_sniff - 12);
        }
    } else {
        /* printf("..%d..\n", t_sniff); */
        sdx_dev_detect_modify(t_sniff - 3);
    }
}
#endif /*#if TCFG_SD_ALWAY_ONLINE_ENABLE*/


void board_init()
{

    board_power_init();

#if TCFG_APP_RTC_EN
    rtc_dev_init(&rtc_dev_config);
#endif

    adc_init();

#if TCFG_APP_FM_EN
    y_printf(">> Func:%s, Line:%d, call: fm_dev_init Func!\n", __func__, __LINE__);
    fm_dev_init((void *)(&fm_dev_data));
#endif

#if TCFG_RDEC_KEY_ENABLE
    rdec_key_init();
#endif

#if TCFG_LPCTMU_ENABLE
    lpctmu_init(&lpctmu_cfg);
#endif


#if TCFG_PAY_ALIOS_ENABLE
    extern void alipay_upay_init(void);
    alipay_upay_init();
#endif /* #if TCFG_PAY_ALIOS_ENABLE */

#if TCFG_SD0_ENABLE
#if TCFG_SD_ALWAY_ONLINE_ENABLE
    sd_power_config(1);
#endif
#endif

#if TCFG_PSRAM_DEV_ENABLE
	psram_init(&psram_config);
#endif
}



/*-----------------------------------------------------------------------
 *进入、退出低功耗函数回调状态，函数单核操作、关中断，请勿做耗时操作
 *
 */
#include "iokey.h"
#include "irkey.h"
#include "adkey.h"
#include "gpio_config.h"



void board_sleep_enter_callback()
{
    /* 此函数禁止添加打印 */
    putchar('<');
}


void board_sleep_exit_callback()
{
    putchar('>');
#if TCFG_SD_ALWAY_ONLINE_ENABLE
#if SDX_POWER_ALONE // sd使用单独的电源才能用
    if (sdx_power_enable && (sdx_power_enable != 0xff)) {
        sdx_sleep_callback();
    }
#endif /* #if SDX_POWER_ALONE */
#endif /*#if TCFG_SD_ALWAY_ONLINE_ENABLE*/
}

////关机回调执行顺序
//power_soff_callback()->do_platform_uninitcall()->board_poweroff_uninit->gpio_config_soft_poweroff()

//关机的注册的段
void board_poweroff_uninit()
{

    //用户进行自己模块的关闭操作

#if TCFG_SD0_ENABLE
    sdx_dev_entry_lowpower("sd0");
#endif
#if TCFG_SD1_ENABLE
    sdx_dev_entry_lowpower("sd1");
#endif

#if TCFG_SD_ALWAY_ONLINE_ENABLE
    sd_power_config(0);
#endif
}
platform_uninitcall(board_poweroff_uninit);



//设置关机保持状态的io
void board_gpio_config_soft_poweroff(u32 *gpio_config)
{
//设置后关机io会保持原来状态
//    PORT_PROTECT(IO_PORTB_01);
//    PORT_PROTECT(IO_PORTB_01);
}







#endif
