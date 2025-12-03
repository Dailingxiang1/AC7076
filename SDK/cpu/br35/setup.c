#include "asm/includes.h"
#include "system/includes.h"
#include "app_config.h"

#define LOG_TAG             "[SETUP]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"

extern void sys_timer_init(void);
extern void tick_timer_init(void);
extern void exception_irq_handler(void);
extern int __crc16_mutex_init();
extern void debug_uart_init();
extern void boot_power_init();
extern void perf_counter_init(void);
/* --------------------------------------------------------------------------*/
/**
 * @brief 裸机程序入口
 */
/* ----------------------------------------------------------------------------*/
void system_main(void)
{
    //分支预测
    q32DSP(core_num())->PMU_CON1 &= ~BIT(8);

    wdt_close();

#if (TCFG_MAX_LIMIT_SYS_CLOCK==MAX_LIMIT_SYS_CLOCK_160M)
    clk_early_init(PLL_REF_XOSC_DIFF, TCFG_CLOCK_OSC_HZ, 240 * MHz);//  240:max clock 160
#else
    clk_early_init(PLL_REF_XOSC_DIFF, TCFG_CLOCK_OSC_HZ, 192 * MHz);
#endif

    //heap内存管理，STDIO需要heap
    memory_init();

    //uart init
    debug_uart_init();

    //STDIO init
    log_early_init(1024 * 2);

    //debug info
    printf("%s : %s %s", __FUNCTION__, __DATE__, __TIME__);

    while (1) {
        asm("idle");
    }
}

/* --------------------------------------------------------------------------*/
/**
 * @brief 裸机环境启动，进入无操作系统环境
 */
/* ----------------------------------------------------------------------------*/
#define SYSTEM_SSP_SIZE 512
u32 system_ssp_stack[SYSTEM_SSP_SIZE];//2Kbyte
void system_start(void)
{
    asm("ssp = %0"::"i"(system_ssp_stack + SYSTEM_SSP_SIZE));
    asm("r0 = %0"::"i"(system_main));
    asm("reti = r0");
    asm("rti");
}


#if CONFIG_DEBUG_ENABLE || CONFIG_DEBUG_LITE_ENABLE
#endif

#if 0
___interrupt
void exception_irq_handler(void)
{
    ___trig;

    exception_analyze();

    log_flush();
    while (1);
}
#endif



/*
 * 此函数在cpu0上电后首先被调用,负责初始化cpu内部模块
 *
 * 此函数返回后，操作系统才开始初始化并运行
 *
 */

#if 0
static void early_putchar(char a)
{
    if (a == '\n') {
        UT2_BUF = '\r';
        __asm_csync();
        while ((UT2_CON & BIT(15)) == 0);
    }
    UT2_BUF = a;
    __asm_csync();
    while ((UT2_CON & BIT(15)) == 0);
}

void early_puts(char *s)
{
    do {
        early_putchar(*s);
    } while (*(++s));
}
#endif

void cpu_assert_debug()
{
#if CONFIG_DEBUG_ENABLE
    log_flush();
    local_irq_disable();
    while (1);
#else
    P3_PCNT_SET0 = 0xac;
    cpu_reset();
#endif
}


extern void sputchar(char c);
extern void sput_buf(const u8 *buf, int len);
void sput_u32hex(u32 dat);



__attribute__((weak))
void maskrom_init(void)
{
    return;
}

extern u32 FREE_DACHE_WAY;
extern u32 FREE_IACHE_WAY;

extern u32 dcache_ram_bss_begin;
extern u32 dcache_ram_bss_size;
extern u32 dcache_ram_bss_end;

extern u32 dcache_ram_data_addr;
extern u32 dcache_ram_data_begin;
extern u32 dcache_ram_data_size;

extern u32 icache_ram_bss_begin;
extern u32 icache_ram_bss_size;
extern u32 icache_ram_bss_end;

extern u32 icache_ram_data_code_addr;
extern u32 icache_ram_data_code_begin;
extern u32 icache_ram_data_code_size;

__attribute__((weak))
AT(.must_ram_code)
void cache_ram_init(void)
{
#if (TCFG_FREE_ICACHE_WAY || TCFG_FREE_DCACHE_WAY)
    // 用 volatile 避免被优化而导致出错
    volatile u8 dcache_way = 4 - (u32)(&FREE_DACHE_WAY);
    volatile u8 icache_way = 4 - (u32)(&FREE_IACHE_WAY);

    if (dcache_way != 4) {
        /* memset((void *)(0x370000 + 0xF800), 0, 0xFC00 - 0xF800);   // 清空 lru */
        DcuInitial();
        // 空出way当ram用
        DcuSetWayNum(dcache_way);

        memset((u8 *)(&dcache_ram_bss_begin), 0, (u32)&dcache_ram_bss_size);
        memcpy((u8 *)&dcache_ram_data_addr, (u8 *)&dcache_ram_data_begin, (u32)&dcache_ram_data_size);
    }

    if (icache_way != 4) {
        IcuFlushinvAll();
        DcuFlushinvAll();

        IcuDisable();
        /* memset((void *)(0x3c0000 + 0xF800), 0, 0xFC00 - 0xF800);  // 清空 lru */
        IcuInitial();
        // 空出way当ram用
        IcuSetWayNum(icache_way);

        memset((u8 *)(&icache_ram_bss_begin), 0, (u32)&icache_ram_bss_size);
        memcpy((u8 *)&icache_ram_data_code_addr, (u8 *)&icache_ram_data_code_begin, (u32)&icache_ram_data_code_size);
    }
#endif
}

AT(.must_ram_code)
void icache_way_switch(u8 num)
{
    local_irq_disable();

    IcuFlushinvAll();
    IcuDisable();
    IcuInitial();
    IcuSetWayNum(num);
    IcuEnable();

    local_irq_enable();
}

#if (CPU_CORE_NUM > 1)
void cpu1_setup_arch()
{
    q32DSP(core_num())->PMU_CON1 &= ~BIT(8); //open bpu

    request_irq(IRQ_EXCEPTION_IDX, 7, exception_irq_handler, 1);

    //用于控制其他核进入停止状态。
    extern void cpu_suspend_handle(void);
    request_irq(IRQ_SOFT0_IDX, 7, cpu_suspend_handle, 0);
    request_irq(IRQ_SOFT1_IDX, 7, cpu_suspend_handle, 1);
    irq_unmask_set(IRQ_SOFT0_IDX, 0, 0); //设置CPU0软中断0为不可屏蔽中断
    irq_unmask_set(IRQ_SOFT1_IDX, 0, 1); //设置CPU1软中断1为不可屏蔽中断

    debug_init();
}

void cpu1_main()
{
    extern void cpu1_run_notify(void);
    cpu1_run_notify();

    interrupt_init();

    q32DSP(core_num())->PMU_CON1 &= ~BIT(8); //分支预测
    cpu1_setup_arch();

    os_start();

    log_e("os err \r\n") ;
    while (1) {
        __asm__ volatile("idle");
    }
}

#else

void cpu1_main()
{

}
#endif /* #if (CPU_CORE_NUM > 1) */

//==================================================//

/* extern void gpio_longpress_pin0_reset_config(u32 pin, u32 level, u32 time); */
void memory_init(void);

__attribute__((weak))
void app_main()
{
    while (1) {
        asm("idle");
    }
}

#define     IIO_DEBUG_TOGGLE(i,x)  {JL_PORT##i->DIR &= ~BIT(x), JL_PORT##i->OUT ^= BIT(x);}

void io_toggle(void)
{
    while (1) {
        IIO_DEBUG_TOGGLE(A, 6);
        IIO_DEBUG_TOGGLE(A, 6);
    }
}

// 读取示例
int get_clk_trim_val(void)
{
    int ret = 0;
    u32 trim_val = 0;

    ret = sdfile_early_init();
    if (ret) {
        printf("sdfile_early_init ret %d\n", ret);
    }

    ret = syscfg_tools_early_init();
    if (ret) {
        printf("syscfg_tools_early_init ret %d\n", ret);
    }

    // ID号必需满足 CFG_STORE_VM_ONLY_BEGIN < id < CFG_STORE_VM_ONLY_END
    ret = syscfg_early_read(53, &trim_val, sizeof(trim_val));
    printf("trim read ret %d, val 0x%x\n", ret, trim_val);

    return ret;
}

// 读取示例
int set_clk_trim_val(u32 trim_val)
{
    // !!! 必须要注意, trim值只能写一次, 即只有读取trim失败, 重新trim完后再写到vm; 如果是读取trim值成功了,则不能再次trim并写vm(需要擦除vm后才可再次写)
    // ID号必需满足 CFG_STORE_VM_ONLY_BEGIN < id < CFG_STORE_VM_ONLY_END
    int ret = syscfg_write(53, &trim_val, sizeof(trim_val));
    printf("clk trim val set ret %d\n", ret);
    return 0;
}


void setup_arch(void)
{
    boot_power_init();

    //system_start();

    //关闭所有timer的ie使能
    bit_clr_ie(IRQ_TIME0_IDX);
    bit_clr_ie(IRQ_TIME1_IDX);
    bit_clr_ie(IRQ_TIME2_IDX);
    bit_clr_ie(IRQ_TIME3_IDX);

    /* gpio_longpress_pin0_reset_config(IO_PORTB_01, 0, 0); */

    memory_init();

#if CONFIG_DEBUG_ENABLE || CONFIG_DEBUG_LITE_ENABLE
    debug_uart_init();
#if CONFIG_DEBUG_ENABLE
    log_early_init(1024 * 2);
#endif
#endif

    wdt_init(WDT_APP_INIT_TIME);
    /* wdt_close(); */
    q32DSP(core_num())->PMU_CON1 &= ~BIT(8); //分支预测

    efuse_init();
    /* clk_voltage_init(TCFG_CLOCK_MODE, SYSVDD_VOL_SEL_126V); */
    /* xosc_hcs_trim(); */

    debug_init();

    sdfile_init();
    syscfg_tools_init();

    /* clk_early_init(PLL_REF_XOSC_DIFF, TCFG_CLOCK_OSC_HZ, 480 * MHz); */
    clk_early_init(PLL_REF_XOSC_DIFF, TCFG_CLOCK_OSC_HZ, 576 * MHz);

#if 0
    //临时添加：by IC wangjie
    SFR(JL_LSBCLK->PRP_CON1,  0, 3, 1);
    SFR(JL_LSBCLK->PRP_CON1, 26, 1, 1);
    // bt clk config by IC luokaijie
    SFR(JL_LSBCLK->PRP_CON2, 0, 2, 1);
    SFR(JL_LSBCLK->PRP_CON2, 2, 2, 1);
    SFR(JL_LSBCLK->PRP_CON2, 4, 2, 1);
    SFR(JL_LSBCLK->PRP_CON2, 8, 1, 1);
    asm("csync");
#endif

    perf_counter_init();

    os_init();
    tick_timer_init();
    sys_timer_init();

    log_i("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    log_i("         setup_arch %s %s", __DATE__, __TIME__);
    log_i("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");

    power_early_flowing();

    clock_dump();
#if 0


    void mvbg_current_trim();        //pmu used
    mvbg_current_trim();
    void audio_vbg_current_trim();        //audio used
    audio_vbg_current_trim();
#endif

    //Register debugger interrupt
    request_irq(0, 2, exception_irq_handler, 0);
    request_irq(1, 2, exception_irq_handler, 0);

#ifdef CONFIG_CODE_MOVABLE_ENABLE
    code_movable_init();
#endif

    __crc16_mutex_init();


    app_main();
}

