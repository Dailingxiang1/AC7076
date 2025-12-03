#include "asm/power_interface.h"
#include "system/includes.h"
#include "asm/mcpwm.h"
#include "app_config.h"


static int lcdpg_pwm_id = -1;
static int mtpg_pwm_id = -1;


/*
 *@brief  初始化pg_io送出高阻+低电平的pwm
 *@param  pg_io : 可选IO_LCD_PG, IO_MT_PG
 *@param  freq  : pwm的频率
 *@param  duty  : pwm的低电平的占空比，0~10000对应0%~100%
 *@return 0:成功  非0:失败
 */
int power_gate_pwm_init(u32 pg_io, u32 freq, u32 duty)
{
    struct mcpwm_config pg_mcpwm_cfg;
    memset(&pg_mcpwm_cfg, 0, sizeof(struct mcpwm_config));

    pg_mcpwm_cfg.aligned_mode = MCPWM_EDGE_ALIGNED;     //边沿对齐
    pg_mcpwm_cfg.frequency = freq;
    pg_mcpwm_cfg.duty = duty;                           //占空比
    pg_mcpwm_cfg.h_pin = pg_io;                         //任意引脚
    pg_mcpwm_cfg.l_pin = -1;                            //任意引脚,不需要就填-1
    pg_mcpwm_cfg.complementary_en = 0;                  //两个引脚的波形, 0: 同步,  1: 互补
    pg_mcpwm_cfg.detect_port = -1;                      //任意引脚,不需要就填-1
    if (pg_io == IO_LCD_PG) {
        pg_mcpwm_cfg.ch = MCPWM_CH0;                    //通道号
        lcdpg_pwm_id = mcpwm_init(&pg_mcpwm_cfg);
        if (lcdpg_pwm_id < 0) {
            return 1;
        }
        mcpwm_start(lcdpg_pwm_id);
    } else {
        pg_mcpwm_cfg.ch = MCPWM_CH1;                    //通道号
        mtpg_pwm_id = mcpwm_init(&pg_mcpwm_cfg);
        if (mtpg_pwm_id < 0) {
            return 1;
        }
        mcpwm_start(mtpg_pwm_id);
    }
    return 0;
}

/*
 *@brief  设置pg_io的pwm的低电平占空比
 *@param  pg_io : 可选IO_LCD_PG, IO_MT_PG
 *@param  duty  : pwm的低电平的占空比，0~10000对应0%~100%
 */
void power_gate_pwm_set_duty(u32 pg_io, u32 duty)
{
    if ((pg_io == IO_LCD_PG) && (lcdpg_pwm_id >= 0)) {
        mcpwm_set_duty(lcdpg_pwm_id, duty);
    }
    if ((pg_io == IO_MT_PG) && (mtpg_pwm_id >= 0)) {
        mcpwm_set_duty(mtpg_pwm_id, duty);
    }
}

/*
 *@brief  关闭pwm, pg_io为高阻
 *@param  pg_io : 可选IO_LCD_PG, IO_MT_PG
 */
void power_gate_pwm_close(u32 pg_io)
{
    if ((pg_io == IO_LCD_PG) && (lcdpg_pwm_id >= 0)) {
        mcpwm_deinit(lcdpg_pwm_id);
    }
    if ((pg_io == IO_MT_PG) && (mtpg_pwm_id >= 0)) {
        mcpwm_deinit(mtpg_pwm_id);
    }
}

/*
 *@brief  设置pg_io的开漏输出
 *@param  pg_io : 可选IO_LCD_PG, IO_MT_PG
 *@param  value : 0,输出低电平  1,则高阻
 */
void power_gate_open_drain_output(u32 pg_io, u32 value)
{
    if (pg_io == IO_LCD_PG) {
        SFR(JL_IOMC->LCDPG_CON, 1, 1, !value);
        JL_IOMC->LCDPG_CON &= ~BIT(0);
    }
    if (pg_io == IO_MT_PG) {
        SFR(JL_IOMC->MTPG_CON, 1, 1, !value);
        JL_IOMC->MTPG_CON &= ~BIT(0);
    }
}

static void power_fspg2_step_1(void)
{
    //out0 slowly
    JL_IOMC->STPG_CON |=  BIT(STPG_HD0);
    JL_IOMC->STPG_CON &= ~BIT(STPG_HD1);
    JL_IOMC->STPG_CON &= ~BIT(STPG_A);
    JL_IOMC->STPG_CON |=  BIT(STPG_OE);
}

static void power_fspg2_step_2()
{
    //STPG highz
    JL_IOMC->STPG_CON &= ~(BIT(STPG_PD) | BIT(STPG_PD1));
    JL_IOMC->STPG_CON &= ~BIT(STPG_OE);

    //STPG out1 slowly
    JL_IOMC->STPG_CON |=  BIT(STPG_A);
    JL_IOMC->STPG_CON &= ~BIT(STPG_HD1);
    JL_IOMC->STPG_CON |=  BIT(STPG_HD0);
    JL_IOMC->STPG_CON |=  BIT(STPG_OE);
}

static void power_fspg2_step_3()
{
    //out1 driving max
    u32 STPG_CON = JL_IOMC->STPG_CON;
    STPG_CON   |= BIT(STPG_HD1);
    STPG_CON   &= ~BIT(STPG_HD0);
    JL_IOMC->STPG_CON = STPG_CON;

}

/*
 *@brief  开启stpg供电
 *@param  udly_time : us延时
 *@param  mdly_time : ms延时
 *@param  *custom_udelay :us延时回调函数
 *@param  *custom_mdelay :ms延时回调函数
 */
static void power_gate_fspg2_base(u32 udly_time, u32 mdly_time, void (*custom_udelay)(u32), void (*custom_mdelay)(u32))
{
    //if stpg power have been 1, don't need poweron again
    if (JL_IOMC->STPG_CON & BIT(STPG_A)) {
        return;
    }

    //FIXME:输出0延时100us后，io是否需要设置为高阻
    power_fspg2_step_1();
    if (custom_udelay) {
        custom_udelay(udly_time);
    }

    power_fspg2_step_2();

    if (custom_udelay) {
        custom_udelay(udly_time);
    }

    power_fspg2_step_3();

    if (custom_mdelay) {
        custom_mdelay(mdly_time);
    }
}

void fspg2_poweron(u32 ms)
{
    power_gate_fspg2_base(100, ms, udelay, mdelay);
}

/*
 *@brief  关闭stpg供电
 *@param  udly_time : us延时
 *@param  mdly_time : ms延时
 *@param  *custom_udelay :us延时回调函数
 *@param  *custom_mdelay :ms延时回调函数
 */
static void poweroff_gate_fspg2_base(u32 udly_time, u32 mdly_time, void (*custom_udelay)(u32), void (*custom_mdelay)(u32))
{

    //out0 slowly
    u32 stpg_con = JL_IOMC->STPG_CON;
    stpg_con |= BIT(STPG_HD0);
    stpg_con &= ~BIT(STPG_HD1);
    JL_IOMC->STPG_CON = stpg_con;
    JL_IOMC->STPG_CON &= ~BIT(STPG_A);
    JL_IOMC->STPG_CON |=  BIT(STPG_OE);

    if (custom_udelay) {
        custom_udelay(udly_time);
    }

    //pulldown 200k
    JL_IOMC->STPG_CON |=  BIT(STPG_PD);
    JL_IOMC->STPG_CON &= ~BIT(STPG_PD1);
    JL_IOMC->STPG_CON &= ~BIT(STPG_OE);
    JL_IOMC->STPG_CON &= ~BIT(STPG_HD0);

    /* log_i("Delay 100ms after fspg2_poweroff !\n"); */
    if (custom_mdelay) {
        custom_mdelay(mdly_time);
    }
}

void fspg2_poweroff(u32 ms)
{
    poweroff_gate_fspg2_base(100, ms, udelay, mdelay);
}

