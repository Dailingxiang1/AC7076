#ifndef __APP_CONFIG_H
#define __APP_CONFIG_H

#include "asm/includes.h"

#define NO_CONFIG_PORT                      (-1)



//*********************************************************************************//
//                                 配置开始                                        //
//*********************************************************************************//
#define ENABLE_THIS_MOUDLE                  1
#define DISABLE_THIS_MOUDLE                 0

#define ENABLE                              1
#define DISABLE                             0
//*********************************************************************************//
//                                 公共配置                                        //
//*********************************************************************************//
#define LOG_DEBUG_ENABLE                    1
//*********************************************************************************//
//                                 屏驱配置                                        //
//*********************************************************************************//

#define TCFG_SPI_LCD_ENABLE                 ENABLE_THIS_MOUDLE//总开关
//屏io配置
#define TCFG_LCD_PIN_RESET                  IO_PORTC_03
#define TCFG_LCD_PIN_CS                     IO_PORTA_06
#define TCFG_LCD_PIN_BL                     IO_LCD_PG
#define TCFG_LCD_PIN_DC                     NO_CONFIG_PORT
#define TCFG_LCD_PIN_EN                     IO_PORTC_01
#define TCFG_LCD_PIN_EN_EX                  IO_PORTC_02
#define TCFG_LCD_PIN_TE                     IO_PORTC_00
//屏驱配置
#define TCFG_LCD_SPI_SH8601A_ENABLE         DISABLE
#define TCFG_LCD_QSPI_ST77903_V1_ENABLE     DISABLE
#define TCFG_LCD_SPI_ST77916_ENABLE         ENABLE
#define TCFG_TFT_LCD_DEV_SPI_HW_NUM         0 //选择spi  0 ：spi0 1：spi1 2：spi2 驱动使用
//多屏驱模式定义
#define LCD_MATCH_BY_LOGO                   0 //定义通过屏驱logo匹配
#define LCD_MATCH_BY_ID                     1 //定义通过屏幕id匹配
//多屏驱模式配置
#define TCFG_LCD_MATCH_MODE                 LCD_MATCH_BY_LOGO
#define LCD_LOGO                            "null"
//根据选择的spi 开启对应于spi项目

//*********************************************************************************//
//                                 硬件SPI 配置                                        //
//*********************************************************************************//
#define TCFG_HW_SPI0_ENABLE         ENABLE_THIS_MOUDLE
//A组IO:    DI: PD2     DO: PD1     CLK: PD0   D2:PB7    D3:PD5
//B组IO:    DI: PA14    DO: PD1     CLK: PD0   D2:PA15   D3:PD5
//C组IO:    DI: PA6     DO: PA8     CLK: PA3   D2:PA7    D3:PA2
//D组IO:    DI: PB8     DO: PB10    CLK: PB5   D2:PB9    D3:PB4
#define TCFG_HW_SPI0_PORT           'C'
#define TCFG_HW_SPI0_BAUD           24000000L
#define TCFG_HW_SPI0_MODE           SPI_MODE_BIDIR_1BIT //SPI_MODE_UNIDIR_1BIT//
#define TCFG_HW_SPI0_ROLE           SPI_ROLE_MASTER

#define TCFG_HW_SPI1_ENABLE         DISABLE_THIS_MOUDLE
//A组IO:    DI: PB2     DO: PB1     CLK: PB0
//B组IO:    DI: PC3     DO: PC5     CLK: PC4
#define TCFG_HW_SPI1_PORT           'B'
#define TCFG_HW_SPI1_BAUD           24000000L
#define TCFG_HW_SPI1_MODE           SPI_MODE_BIDIR_1BIT
#define TCFG_HW_SPI1_ROLE           SPI_ROLE_MASTER

#define TCFG_HW_SPI2_ENABLE         DISABLE_THIS_MOUDLE
//A组IO:    DI: PB8     DO: PB10    CLK: PB9
//B组IO:    DI: PA13    DO: DM      CLK: DP
#define TCFG_HW_SPI2_PORT           'A'
#define TCFG_HW_SPI2_BAUD           48000000L
#define TCFG_HW_SPI2_MODE           SPI_MODE_BIDIR_1BIT
#define TCFG_HW_SPI2_ROLE           SPI_ROLE_MASTER



#define SPI0_ENABLE   TCFG_HW_SPI0_ENABLE   //是否使能SPI0，使能后需定义spi0_p_data
#define SPI1_ENABLE   TCFG_HW_SPI1_ENABLE   //是否使能SPI1，使能后需定义spi1_p_data
#define SPI2_ENABLE   TCFG_HW_SPI2_ENABLE   //是否使能SPI2，使能后需定义spi2_p_data



typedef struct _export_api_t {
    //debug接口
    int (*printf)(const char *format, ...);
	int (*print)(char **out, const char *format, va_list args);
    void (*put_buf)(const u8 *buf, u32 len);
    u16(*sys_timer_add)(void *priv, void (*func)(void *priv), u32 msec);
    u16(*sys_timeout_add)(void *priv, void (*func)(void *priv), u32 msec);
    void (*sys_timer_del)(u16 t);
    void (*irq_disable)(void);
    void (*irq_enable)(void);
    void (*mdelay)(u32 msec);
    void (*request_irq_func)(u8 index, u8 priority, void (*handler)(void), u8 cpu_id);
} ex_api_t;



extern ex_api_t *p_ex_api;
#if LOG_DEBUG_ENABLE
#define LCD_PRINTF  if(p_ex_api && p_ex_api->printf ) \
                          p_ex_api->printf
#else
#define LCD_PRINTF
#endif // LOG_DEBUG_ENABLE

#endif
