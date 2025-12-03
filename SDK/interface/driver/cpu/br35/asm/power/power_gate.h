#ifndef __POWER_GATE_H__
#define __POWER_GATE_H__


#include "typedef.h"

//STPG define bit
#define     STPG_A              0   //STPG output bit
#define     STPG_HD0            2   //STPG HD0
#define     STPG_HD1            3   //STPG HD1
#define     STPG_OE             4   //STPG output enable bit
#define     STPG_PD             5   //STPG pull down enable bit
#define     STPG_PD1            6   //STPG pull down enable bit

/*
 *@brief  初始化pg_io送出高阻+低电平的pwm
 *@param  pg_io : 可选IO_LCD_PG, IO_MT_PG
 *@param  freq  : pwm的频率
 *@param  duty  : pwm的低电平的占空比，0~10000对应0%~100%
 *@return 0:成功  非0:失败
 */
int power_gate_pwm_init(u32 pg_io, u32 freq, u32 duty);

/*
 *@brief  设置pg_io的pwm的低电平占空比
 *@param  pg_io : 可选IO_LCD_PG, IO_MT_PG
 *@param  duty  : pwm的低电平的占空比，0~10000对应0%~100%
 */
void power_gate_pwm_set_duty(u32 pg_io, u32 duty);

/*
 *@brief  关闭pwm, pg_io为高阻
 *@param  pg_io : 可选IO_LCD_PG, IO_MT_PG
 */
void power_gate_pwm_close(u32 pg_io);

/*
 *@brief  设置pg_io的开漏输出
 *@param  pg_io : 可选IO_LCD_PG, IO_MT_PG
 *@param  value : 0,输出低电平  1,则高阻
 */
void power_gate_open_drain_output(u32 pg_io, u32 value);

/*
 *@brief  开启stpg供电
 *@param  ms : ms延时
*/
void fspg2_poweron(u32 ms);

/*
 *@brief  关闭stpg供电
 *@param  ms : ms延时
*/
void fspg2_poweroff(u32 ms);
#endif
