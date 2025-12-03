/* Copyright(C)
 * not free
 * All right reserved
 *
 * @file dbi.h
 * @brief DBI模块驱动API头文件。DBI模块为专门用于推屏的独立硬件模块，与硬件SPI无关。其配置仅在屏幕驱动的结构体中，推屏时钟的频率为内部通过推屏帧率计算而得，但时钟是由PLL进行分频而来，因此实际时钟频率仅会在分频组合允许的前提下尽可能接近所需的目标频率(实际测量的推屏时钟与计算并非绝对一致)。
 * @author zhuhaifang@zh-jieli.com
 * @version V201
 * @date 2023-01-10
 */

#ifndef __DBI_H__
#define __DBI_H__

#include "generic/typedef.h"
#include "os/os_api.h"
#include "gpio.h"

extern const u8 lvgl_ui_enable;

enum OUTPUT_FORMAT {
    /* 0 */OUTPUT_FORMAT_RGB888,
    /* 1 */OUTPUT_FORMAT_RGB565,
};


/* ------------------------------------------------------------------------------------*/
/**
 * @brief DBI模块私有变量。注意：该变量内部使用，外部禁止操作，否则会导致不可预测的问题。
 */
/* ------------------------------------------------------------------------------------*/
extern struct dbi_variable dbi_var;


/* ------------------------------------------------------------------------------------*/
/**
 * @brief 模块驱动接口组，DBI模块下管理了SPI, MCU, RGB三组驱动接口，具体驱动接口在这里
 * 注册给DBI模块管理使用。注意：该变量内部使用，外部无需修改，否则会导致不可预测的问题。
 */
/* ------------------------------------------------------------------------------------*/
struct dbi_driver {
    void (*init)(struct dbi_variable *priv);
    void (*write)(u32, u8 *, u32);
    void (*read)(u32, u8 *, u32);
    void (*set_draw_area)(int, int, int, int);
    void (*clear)(u32, int, int, int, int);
    void (*draw)(u8 *, int, int, int, int);
    void (*draw_continue)(u8 *, int, int, int, int);
    void (*enter_sleep)();
    void (*exit_sleep)();
};


//[DBI接口IO定义]
#define DBI_CSX_VSYNC   IO_PORTA_07
#define DBI_DCX_HSYNC   IO_PORTA_13
#define DBI_SCL_WRX     IO_PORTA_12
#define DBI_D0          IO_PORTA_08
#define DBI_D1          IO_PORTA_09
#define DBI_D2          IO_PORTA_10
#define DBI_D3          IO_PORTA_11
#define DBI_D4          IO_PORTC_00
#define DBI_D5          IO_PORTC_01
#define DBI_D6          IO_PORTC_02
#define DBI_D7          IO_PORTC_03
#define DBI_RDX_DEN     IO_PORTC_10



// <<<[lcd接口配置]>>>
#define SPI_MODE  (0<<4)
#define DSPI_MODE (1<<4)
#define QSPI_MODE (2<<4)

// SPI线数标志(clk + dat)
#define SPI_WIRE3  0
#define SPI_WIRE4  1

// QSPI的子模式标志(除SUBMODE0为1 dat线之外，其余模式都为4 dat线)
#define QSPI_SUBMODE0 0//0x02
#define QSPI_SUBMODE1 1//0x32
#define QSPI_SUBMODE2 2//0x12
#define QSPI_RAMLESS  3//0xde
#define QSPI_FT2388   4//0x32 1ch cmd + 4ch data
#define QSPI_SD3302   5

// LCD时序标志(根据LCD数据手册配置，常用时序均已组合到LCD_DRIVE_CONFIG宏，在LCD驱动文件配置)
#define PIXEL_1P1T (0<<5)
#define PIXEL_1P2T (1<<5)
#define PIXEL_1P3T (2<<5)
#define PIXEL_2P3T (3<<5)

#define PIXEL_1T2B  1
#define PIXEL_1T6B  5
#define PIXEL_1T8B  7
#define PIXEL_1T9B  8
#define PIXEL_1T12B 11
#define PIXEL_1T16B 15
#define PIXEL_1T18B 17
#define PIXEL_1T24B 23

#define FORMAT_RGB565 1//0//1P2T
#define FORMAT_RGB666 2//1//1P3T
#define FORMAT_RGB888 0//2//1P3T


#define SPI_MODE_UNIDIR  0//半双工，d0分时发送接收
#define SPI_MODE_BIDIR   1//全双工，d0发送、d1接收

enum {
    CMD_MODE,
};

enum {
    CMD_8BIT,
    CMD_16BIT,
    CMD_24BIT,
};

/*
** dbi私有全局变量定义
*/
struct dbi_variable {
    volatile u8 dbi_busy;	// dbi忙碌标志
    u8 clock_init: 1;	// dbi时钟初始化标志
    u8 clock_div_sel: 7;	   // dbi时钟
    u8 sfr_save;	// 寄存器保存标志
    u8 row_align;   // 行对齐
    u8 column_align;// 列对齐
    int (*te_stat)();	// TE 信号检测，返回值为TE脚电平，使用内部等待TE信号时，这个函数须注册
    void (*te_wait)();	// TE 信号等待，如果使用外部TE等待函数，注册这个回调，则等TE时会调用这
    struct dbi_param *param;
    int (*dbi_callback)(void *priv);//dbi回调
    u8 cmd_mode;
    OS_SEM draw_sem;
    u8 *frameBufferOld;
    u8 *frameBuffer;
    u32 frameBufferSize;
    u32 clearColor;
    int frameBufferChanged;
    int xstart;
    int xend;
    int ystart;
    int yend;
    float actual_fps;

    int soft_cs_pin;
    volatile int cs_high;
};

/* ------------------------------------------------------------------------------------*/
/**
 * @brief 屏幕驱动接口类型定义，根据屏幕的接口类型在屏驱内配置使用
 */
/* ------------------------------------------------------------------------------------*/
typedef enum lcd_type_cfg {
    LCD_TYPE_SPI,	        // SPI屏
    LCD_TYPE_SPI_RAMLESS,	// SPI RAMLESS屏
    LCD_TYPE_MCU,	        // MCU屏
    LCD_TYPE_RGB,	        // RGB屏
    LCD_TYPE_MAX,
} LCD_TYPE;

typedef enum {
    DC_PIN_SEL_PA9,
    DC_PIN_SEL_PA13,
    DC_PIN_SEL_SOFT,
} DC_PIN_SEL;

typedef enum {
    CS_PIN_SEL_PA7,
    CS_PIN_SEL_SOFT,
} CS_PIN_SEL;

// 以下配置只对SPI_MODE_UNIDIR模式生效, SPI_MODE_BIDIR模式固定PA9
// 具体使用SPI/DSPI/QSPI哪个通道读数据，可查询屏DriverIC手册或者咨询屏厂,
// 读IO肯定是PA8/PA9/PA10/PA11其中的某一个, 也可以逐个试一下，看读0x0A寄存器的值是否正常
typedef enum {
    READ_PIN_SEL_PA8,
    READ_PIN_SEL_PA9,
    READ_PIN_SEL_SOFT,
} READ_PIN_SEL;

// 只适用于MCU屏配置
// 若需要读寄存器，RDX引脚固定选PA13，由硬件控制
// 若没有读寄存器的需求，RDX引脚可不打开，可以省一个IO，屏驱上的DCX脚需要接高电平
typedef enum {
    RDX_PIN_SEL_PA13,
    RDX_PIN_SEL_NONE,
} RDX_PIN_SEL;

typedef enum {
    CLOCK_POLARITY_IDLE_LOW,  // 空闲时为低电平
    CLOCK_POLARITY_IDLE_HIGH, // 空闲时为高电平
} CLOCK_POLARITY;

struct dbi_param {
    // 显示区域相关(用于配置推屏区域，可不推全屏)
    int scr_x;
    int scr_y;
    int scr_w;
    int scr_h;

    // lcd配置
    int lcd_width;
    int lcd_height;
    LCD_TYPE lcd_type;	// LCD接口类型：SPI, SPI Ramless, MCU, RGB

    // 显存配置
    int buffer_num;
    int buffer_size;

    // dbi模块的输入，与imb模块一起用时，也是imb模块的输出
    int in_width;
    int in_height;
    int in_format;
    int in_stride;

    // debug模式(推新屏时，可使能debug模式，先推纯色测试)
    int debug_mode_en;
    int debug_mode_color;

    // 帧率配置(用于计算DBI模块时钟频率，但实际推屏帧率仅会接近配置，并非100%一致)
    int  fps;

    // 以下为三种屏驱动相关配置，out_format为屏幕的像素格式类型
    struct spi_param {
        int spi_mode;
        int pixel_type;
        int out_format;
        int spi_dat_mode;
        DC_PIN_SEL dc_pin_select;  //若选择SPI_4WIRE_RGB888_1T8B/SPI_4WIRE_RGB888_1T24B/SPI_4WIRE_RGB666_1T18B/SPI_4WIRE_RGB565_1T8B/SPI_4WIRE_RGB565_1T16B时序,
        //dc_pin_select = DC_PIN_SEL_PA9, dc信号从PA9输出(硬件控制)
        //dc_pin_select = DC_PIN_SEL_PA13, dc信号从PA13输出(硬件控制)
        //dc_pin_select = DC_PIN_SEL_SOFT, dc信号从soft_dc_pin指定的引脚输出(软件控制，只对4线spi有效)
        int soft_dc_pin;    //软件dc脚, 可指定任意gpio, IO_PORTX_xx
        CS_PIN_SEL cs_pin_select;
        //cs_pin_select = CS_PIN_SEL_PA7, cs信号从PA7输出(硬件控制)
        //cs_pin_select = CS_PIN_SEL_SOFT, cs信号从soft_cs_pin指定的引脚输出(软件控制, 对spi/dspi/qspi有效, qspi ramless时序不支持)
        int soft_cs_pin;    //软件cs脚, 可指定任意gpio, IO_PORTX_xx
        READ_PIN_SEL read_pin_select;
        //read_pin_select = READ_PIN_SEL_PA8, 从PA8接收sdo信号(硬件控制)
        //read_pin_select = READ_PIN_SEL_PA9, 从PA9接收sdo信号(硬件控制)
        //read_pin_select = READ_PIN_SEL_SOFT, 从soft_read_pin指定的引脚接收sdo信号(软件控制)
        int soft_read_pin;  //软件输入脚, 可指定任意gpio, IO_PORTX_xx
        struct qspi_cmd {
            u8 write_cmd;
            u8 read_cmd;
            u8 submode0_cmd;//0x2c/0x3c/数据单线, default value 0x02
            u8 submode1_cmd;//0x2c/0x3c单线，数据四线，default value 0x32
            u8 submode2_cmd;//0x2c/0x3c/数据四线, default value 0x12
        } qspi_cmd;
        struct ramless {
            u32 frame_sync_cmd;//ramless cmd
            u32 porch_sync_cmd;//ramless cmd
            u32 line_sync_cmd;//ramless cmd
            u16 hsync;
            u16 hbp;    //hori back porch
            u16 hact;   //hori actual
            u16 hfp;    //hori front porch
            u16 vsync;
            u16 vbp;    //vert back porch
            u16 vact;   //vert actual line
            u16 vfp;    //vert front porch
        } ramless;
        u8 caset_cmd;//Column Address Set, default value 0x2a
        u8 paset_cmd;//Page Address Set, default value 0x2b
        u8 ramwr_cmd;//Memory Write Start, default value 0x2c
        u8 ramwrc_cmd;//Memory Write Continue, default valus 0x3c

        CLOCK_POLARITY clk_pol; // 时钟极性
    } spi;

    struct pap_param {
        RDX_PIN_SEL rdx_pin_select; //dcx 引脚配置
        int out_format;
    } pap;

    struct rgb_param {
        int use_spi;
        int out_format;
        int continue_frames;

        int hpw_prd;
        int hbw_prd;
        int hfw_prd;
        int hact_prd;

        int vpw_prd;
        int vbw_prd;
        int vfw_prd;
        int vact_prd;
    } rgb;
};


struct dbi_ramless_callback_param {
    int lcd_buff_update;
    u32 lcd_buff_old;
    u32 lcd_buff_new;
};


/* ------------------------------------------------------------------------------------*/
/**
 * @brief lcd_init DBI模块初始化(注意：需要操作DBI或从DBI获取信息时，必须先初始化)
 *
 * @param param DBI配置句柄
 *
 * @return 0
 */
/* ------------------------------------------------------------------------------------*/
int  lcd_init(struct dbi_param *param);


/* ------------------------------------------------------------------------------------*/
/**
 * @brief lcd_write_cmd DBI模块向LCD指定地址写数据
 *
 * @param cmd 待操作的LCD命令地址
 * @param buf 数据缓存buf（如果不需要写数据，可传入NULL）
 * @param len 数据缓存长度（如果不写数据，需将len配置为0）
 */
/* ------------------------------------------------------------------------------------*/
void lcd_write_cmd(u32 cmd, u8 *buf, u32 len);


/* ------------------------------------------------------------------------------------*/
/**
 * @brief lcd_read_cmd DBI模块从LCD指定地址读数据
 *
 * @param cmd 待操作的LCD命令地址
 * @param buf 数据缓存buf（输出）
 * @param len 读取的数据长度（需与buf匹配，不可超过buf大小）
 */
/* ------------------------------------------------------------------------------------*/
void lcd_read_cmd(u32 cmd, u8 *buf, u32 len);


/* ------------------------------------------------------------------------------------*/
/**
 * @brief lcd_set_draw_area 设置LCD显示区域（2A, 2B命令）
 *
 * @param xstart X起始像素坐标
 * @param xend   X结束像素坐标
 * @param ystart Y起始像素坐标
 * @param yend   Y结束像素坐标
 */
/* ------------------------------------------------------------------------------------*/
void lcd_set_draw_area(int xstart, int xend, int ystart, int yend);


/* ------------------------------------------------------------------------------------*/
/**
 * @brief lcd_draw DBI硬件写显示数据到LCD, 非阻塞接口，与lcd_set_draw_area配合使用
 *
 * @param buf     待写出数据
 * @param xstart  X起始像素坐标
 * @param xend    X结束像素坐标
 * @param ystart  Y起始像素坐标
 * @param yend    Y结束像素坐标
 */
/* ------------------------------------------------------------------------------------*/
void lcd_draw(u8 *buf, int xstart, int xend, int ystart, int yend);

/* ------------------------------------------------------------------------------------*/
/**
 * @brief lcd_draw_continue DBI硬件以续传方式写显示数据到LCD, 非阻塞接口，与lcd_set_draw_area、lcd_draw配合使用
 *
 * @param buf     待写出数据
 * @param xstart  X起始像素坐标
 * @param xend    X结束像素坐标
 * @param ystart  Y起始像素坐标
 * @param yend    Y结束像素坐标
 */
/* ------------------------------------------------------------------------------------*/
void lcd_draw_continue(u8 *buf, int xstart, int xend, int ystart, int yend);


/* ------------------------------------------------------------------------------------*/
/**
 * @brief lcd_clear 纯色填充LCD指定区域
 *
 * @param color  待填充的RGB888颜色值
 * @param xstart 待填充的X起始像素坐标
 * @param xend   待填充的X结束像素坐标
 * @param ystart 待填充的Y起始像素坐标
 * @param yend   待填充的Y结束像素坐标
 */
/* ------------------------------------------------------------------------------------*/
void lcd_clear(u32 color, int xstart, int xend, int ystart, int yend);


/* ------------------------------------------------------------------------------------*/
/**
 * @brief lcd_set_ctrl_pin_func 配置LCD控制脚操作函数
 *
 * @param te_stat TE脚控制函数
 */
/* ------------------------------------------------------------------------------------*/
void lcd_set_ctrl_pin_func(int (*te_stat)());


/* ------------------------------------------------------------------------------------*/
/**
 * @brief lcd_set_te_wait_cb 配置TE信号等待回调函数，如果注册，等待TE时将调用回调进行等待
 *
 * @Params te_wait 应用层实现的TE等待函数
 */
/* ------------------------------------------------------------------------------------*/
void lcd_set_te_wait_cb(void (*te_wait)());

/* ------------------------------------------------------------------------------------*/
/**
 * @brief lcd_set_align 设置LCD对齐参数
 *
 * @param row_align    设置行对齐参数
 * @param column_align 设置列对齐参数
 */
/* ------------------------------------------------------------------------------------*/
void lcd_set_align(u8 row_align, u8 column_align);

/* ------------------------------------------------------------------------------------*/
/**
 * @brief lcd_get_align 设置LCD对齐参数
 *
 * @param row_align    获取行对齐参数
 * @param column_align 获取列对齐参数
 */
/* ------------------------------------------------------------------------------------*/
void lcd_get_align(u8 *row_align, u8 *column_align);

/* ------------------------------------------------------------------------------------*/
/**
 * @brief lcd_check_align 检查LCD刷屏区域参数是否对齐
 *
 * @param xstart X起始像素坐标
 * @param xend   X结束像素坐标
 * @param ystart Y起始像素坐标
 * @param yend   Y结束像素坐标
 */
/* ------------------------------------------------------------------------------------*/
int lcd_check_align(int xstart, int xend, int ystart, int yend);


/* ------------------------------------------------------------------------------------*/
/**
 * @brief lcd_wait_busy 等待lcd空闲
 * 注意：本函数一般在推屏buffer释放前调用，等待推屏结束
 */
/* ------------------------------------------------------------------------------------*/
void lcd_wait_busy();

/* ------------------------------------------------------------------------------------*/

/* ------------------------------------------------------------------------------------*/
/**
 * @brief lcd_draw_kistart 无等待推屏接口
 */
/* ------------------------------------------------------------------------------------*/
void lcd_draw_kistart(uint8_t *buf, int xstart, int xend, int ystart, int yend);

/* ------------------------------------------------------------------------------------*/
/**
 * @brief lcd_draw_set_callback 设置推屏结束回调, lcd_draw、lcd_draw_continue推屏结束时调用该回调
 */
/* ------------------------------------------------------------------------------------*/
int  lcd_draw_set_callback(int (*callback)(void *priv));


/* ------------------------------------------------------------------------------------*/
/**

 * @brief lcd_te_trig_kistart_set_busy 设置LCD繁忙状态，需TE中断调用lcd_draw_kistart接口推屏，推屏结束后会自动消除繁忙状态,
 * 另外附加作用是打印调试接口，如果是kistart是由TE来引导的,那么wait lcd busy就是正常的, 不用打印出来
 */
/* ------------------------------------------------------------------------------------*/
void lcd_te_trig_kistart_set_busy(void);

/* ------------------------------------------------------------------------------------*/
/**
 * @brief lcd_set_param 设置LCD模式
 */
/* ------------------------------------------------------------------------------------*/
void lcd_set_param(u8 mode, int priv);

/* ------------------------------------------------------------------------------------*/
/**
 * @brief lcd_get_param 获取LCD模式
 */
/* ------------------------------------------------------------------------------------*/
int lcd_get_param(u8 mode);

float dbi_get_actual_fps();

/* ------------------------------------------------------------------------------------*/
/**
 * @brief lcd_clock_reinit LCD时钟重新初始化（必须在lcd空闲时调用！！！）
 */
/* ------------------------------------------------------------------------------------*/
void lcd_clock_reinit(int fps);

void lcd_enter_sleep();
void lcd_exit_sleep();


/* ------------------------------------------------------------------------------------*/
/* --------------------------------  SPI/DSPI/QSPI专用接口  ---------------------------*/
/* ------------------------------------------------------------------------------------*/

/* ------------------------------------------------------------------------------------*/
/**
 * @brief lcd_spi_set_cs_pin 设置软件cs脚(cs_pin_select指定CS_PIN_SEL_SOFT时生效)
 *
 * @param gpio    指定普通io作为软件cs脚
 *
 */
/* ------------------------------------------------------------------------------------*/
void lcd_spi_set_cs_pin(int gpio);

/* ------------------------------------------------------------------------------------*/
/**
 * @brief lcd_spi_get_cs_pin 获取软件cs脚(cs_pin_select指定CS_PIN_SEL_SOFT时生效)
 *
 * @return gpio :   软件cs脚
 *
 */
/* ------------------------------------------------------------------------------------*/
int lcd_spi_get_cs_pin();

#endif







