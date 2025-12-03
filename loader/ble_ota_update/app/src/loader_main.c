#ifdef SUPPORT_MS_EXTENSIONS_APP
#pragma bss_seg(".loader_main.data.bss")
#pragma data_seg(".loader_main.data")
#pragma const_seg(".loader_main.text.const")
#pragma code_seg(".loader_main.text")
#endif
#include "common.h"
#include "clock.h"
#include "irq.h"
#include "wdt.h"
#include "uart.h"
#include "printf.h"
#include "dec.h"
#include "jlfs.h"
#include "delay.h"
#include "upgrade.h"
#include "sys_timer.h"
#include "exception.h"
#include "update_main.h"
#include "norflash.h"
#include "mask_api.h"

#if (UPDIFF_FLASH_UPDATE_SUPPORT_EN||COMBAK_FLASH_UPDATE_SUPPORT_EN) && \
        (defined(CONFIG_CPU_BR52))
#include "charge_hw.h"
#endif

#define LOG_TAG_CONST       LOADER_MAIN
#define LOG_TAG             "[LOADER_MAIN]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

/* #define IO_N 3 */
/* void test_io(int x) */
/* { */
/*     JL_PORTB->DIR &= ~BIT(IO_N); */
/*     #<{(| mdelay(20); |)}># */
/*     for (int i = 0; i < x; i++) */
/*     { */
/*         JL_PORTB->OUT ^= BIT(IO_N); */
/*     } */
/*     JL_PORTB->OUT |= BIT(IO_N); */
/* } */


#include "power/p33.h"
#include "gpio.h"
extern u32 get_power_pin_multi(u32 *gpio, u8 *level, u32 cnt);
#define PP_N  3//最多记录power_pin脚数
typedef struct  {
    u32 gpio[PP_N];
    u8 level[PP_N];
} PowerPin;
static PowerPin pp_handle = {0};

//================================================//
//          不可屏蔽中断使能配置(UNMASK_IRQ)      //
//          	Only For AC701N                   //
//================================================//
const int CONFIG_CPU_UNMASK_IRQ_ENABLE = 0;

struct flash_platform_data inside_flash_pdata;

void  soft_irq_handler(void)
{
    printf("%s\n", __func__);
}

u32 stack_magic[4] sec(.stack_magic);
u32 stack_magic0[4] sec(.stack_magic0);
extern void flash_fs_v2_update_init(void);
extern void p33_pinr_tmr_restart(void);
extern u8 p33_get_reset_pin(void);
void exception_analyze(unsigned int *sp);
void exception_irq_handler(void);
void port_init();

int norflash_dev_open(struct dev_node *node, struct device **device, void *arg);
int norflash_no_sr_mode_write_protect(u8 sr_mode, u8 wr_en_mode, u32 sr1, u32 sr2);

__attribute__((weak))
void save_sfr_before_soft_reset(void)
{

}

int main(void)
{

    u8 val[16];
    u8 pll_scr = 0;

    save_sfr_before_soft_reset();

#if defined(CONFIG_CPU_BD49) || defined(CONFIG_CPU_BD47) || defined(CONFIG_CPU_BD45) || defined(CONFIG_CPU_SH58)
    mask_api_init(putchar, exception_analyze);
#else
    mask_api_init(putchar, NULL);
#endif

#if defined(CONFIG_CPU_WL83)
    switch_sysclk(HSB_SEL_PLL_96M);
#else
    /* JL_CLOCK->CLK_CON0 |= BIT(8); //OSC24M -> STD24M  */
    uart_init("PA05", 2000000); //debug串口
    /* uart_init("PC05", 2000000); //debug串口 */
#endif

    printf("\n >>>[test]:func = %s,line= %d\n", __FUNCTION__, __LINE__);

    memset((u8 *)&inside_flash_pdata, 0, sizeof(struct flash_platform_data));
    inside_flash_pdata.spi_pdata.width = 2;
    inside_flash_pdata.spi_pdata.clk_div = 3;
    inside_flash_pdata.read_mode = 0;
#if defined(CONFIG_CPU_BR35)
    inside_flash_pdata.spi_pdata.port = !!(JL_SFC_IOMC->IOMC0 & BIT(1));
#endif

    inside_flash_pdata.flash_type = DEV_FLASH_INTERNAL_NORFLASH;
    void *device;
    norflash_dev_open(NULL, (void *)&device, &inside_flash_pdata);
    /* norflash_init(&inside_flash_pdata); */

    // by xuebo
#if !defined(CONFIG_CPU_WL83)
    sys_clk_init(OSC_FREQ, SYS_CLK);
#endif

#if defined(CONFIG_CPU_SH58) && USB_HOST_MODULE_CONTROL
    set_sys_clk(96000000);  //SH58 U盘升级需要提高系统时钟
#endif

    printf(">>>[test]:+++++++++++++\n");

#if (UPDIFF_FLASH_UPDATE_SUPPORT_EN||COMBAK_FLASH_UPDATE_SUPPORT_EN) && \
        (defined(CONFIG_CPU_BR52) || defined(CONFIG_CPU_BR56) || defined(CONFIG_CPU_BR50))
    asm("btbclr");
    q32DSP(0)->PMU_CON1 &= ~BIT(8); //open bpu
    SFR(JL_HSBCLK->HSB_SEL, 0, 3, 6);//sys 96MHz
#endif

    flash_fs_v2_update_init(); //存在偏移情况，先提前获取。

#ifdef __DEBUG
#if !defined(CONFIG_CPU_WL83)
    u8 *ptr = jlfs_get_isd_cfg_ptr();
    memset(val, 0, sizeof(val));
    dec_isd_cfg_ini("PLL_SRC", val, ptr);
    if (strcmp((char *)val, "LRC") == 0) {
        pll_scr = 2;
    }
    u32 ut_buad = 0;
    char uttx[8] = {0};
    memset(uttx, 0, sizeof(uttx));
    dec_isd_cfg_ini("UTTX", uttx, ptr);
    dec_isd_cfg_ini("UTBD", &ut_buad, ptr);

    uart_init(uttx, ut_buad);

    /* reset_source_dump(); */
#endif
#endif

#if (UPDIFF_FLASH_UPDATE_SUPPORT_EN||COMBAK_FLASH_UPDATE_SUPPORT_EN) && \
        (defined(CONFIG_CPU_BR52))
    loader_charge_init();
#endif
    /* uart_init("USBDP", 1000000); //debug串口 */
    log_info("\n******************  Hello Ota loader DATE:%s TIME:%s *****************\n\n", __DATE__, __TIME__);

#if defined(CONFIG_CPU_BD49) || defined(CONFIG_CPU_SH58)
    power_early_init(NULL);
#endif


    //解除写保护
    norflash_no_sr_mode_write_protect(0, 0, 0, 0);

    //关闭长按复位,打印复位寄存器值方便查看  2023_10_07 by phewlee
    p33_get_reset_pin();
    reset_pin_close();
    p33_get_reset_pin();

    extern void lrc_init(void);
    lrc_init();

    wdt_init(WDT_4s);



    /*初始化异常中断*/
    request_irq(IRQ_EXCEPTION_IDX, 2, exception_irq_handler, 0);
    debug_init();

    /* request_irq(IRQ_SOFT0_IDX, 2, soft_irq_handler, 0); */
    /*  */
    /* irq_set_pending(IRQ_SOFT0_IDX); */
    /* while(1); */
    /*初始化sys_timer*/
    sys_timer_init();

    //ldoin长按复位功能如果开启，定时出现启动
#if !defined(CONFIG_CPU_BD49) && !defined(CONFIG_CPU_BD47) && !defined(CONFIG_CPU_BD45)
    sys_timer_add(NULL, (void *)p33_pinr_tmr_restart, 500);
#endif


    //获取POWER_PIN参数, 防止部分触摸ic掉电一半再上电状态异常
#if (0 == UART_UPDATE_ONLY_TEST_MODE && UPDATE_GET_POWER_PIN)
    if (get_power_pin_multi(&pp_handle.gpio[0], &pp_handle.level[0], PP_N)) {
        for (int i = 0; i < PP_N; i++) {
            printf(">>>[test]:gpio[%d] = %d\n", i, pp_handle.gpio[i]);
            if (pp_handle.gpio[i] == 0) {
                break;
            }
            gpio_set_direction(pp_handle.gpio[i], 0);
            gpio_set_die(pp_handle.gpio[i], 1);
            //先输出低是为了释放该io的电容的电，达到快速拉低目的,防止部分触摸ic掉电一半再上电状态异常;
            gpio_set_output_value(pp_handle.gpio[i], 0);
            udelay(100);
            gpio_set_output_value(pp_handle.gpio[i], pp_handle.level[i]);
        }
    }
#endif

    update_main();

    while (1) {
        putchar('o');
        udelay(1000 * 1000);
    };

    return 0;
}

