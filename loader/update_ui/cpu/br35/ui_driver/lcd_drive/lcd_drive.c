#include "app_config.h"
#include "ui/lcd_spi/lcd_drive.h"
#include "includes.h"
#include "ui/ui_api.h"
#include "clock_cfg.h"
#include "dbi.h"
#include "gpio.h"

#define lcd_debug LCD_PRINTF

static u8  backlight_status = 0;
static u8  lcd_sleep_in     = 0;
static struct lcd_drive *__lcd = NULL;
static struct dbi_param *__this = NULL;
void lcd_drv_cmd_list(u8 *cmd_list, int cmd_cnt);
static void lcd_drv_power_on(void *p);
static void lcd_drv_clear_screen(u32 color, int xstart, int xend, int ystart, int yend);
struct lcd_drive *lcd_drv_get_hdl(u8 mode, const char *logo);
struct lcd_drive *lcd_drv_get_hdl_by_logo(const char *logo);
struct lcd_drive *lcd_drv_get_hdl_by_id();
struct lcd_platform_data lcd_cfg_dat;
#define lcd_dat (&lcd_cfg_dat)

// EN 控制
void lcd_en_ctrl(u8 val)
{
    if (lcd_dat == NULL) {
        return ;
    }
    enum gpio_mode mode = val ? PORT_OUTPUT_HIGH : PORT_OUTPUT_LOW;
    enum gpio_drive_strength drive = PORT_DRIVE_STRENGT_64p0mA;
    if (lcd_dat->pin_en != NO_CONFIG_PORT) {
        gpio_set_mode(IO_PORT_SPILT(lcd_dat->pin_en), mode);
        gpio_set_drive_strength(IO_PORT_SPILT(lcd_dat->pin_en), drive);
    }
}

// BL 控制
void lcd_bl_ctrl(u8 val)
{
    if (lcd_dat == NULL) {
        return ;
    }
    if (lcd_dat->pin_bl == NO_CONFIG_PORT) {
        return;
    }
    if (lcd_dat->pin_bl == IO_LCD_PG) {
        u32 value = val ? 0 : 1;
        power_gate_open_drain_output(lcd_dat->pin_bl, value);
        return;
    }
    enum gpio_mode mode = val ? PORT_OUTPUT_HIGH : PORT_OUTPUT_LOW;
    gpio_set_mode(IO_PORT_SPILT(lcd_dat->pin_bl), mode);
}
// TE 控制
static int spi_te_stat()
{
    if (lcd_dat == NULL) {
        return -1;
    }
    if (lcd_dat->pin_te == NO_CONFIG_PORT) {
        return -1;
    }

    gpio_set_mode(IO_PORT_SPILT(lcd_dat->pin_te), PORT_INPUT_PULLUP_100K);

    return gpio_read(lcd_dat->pin_te);
}

/*
 *********************************************************************************************************
 *                                       LCD DEVICE RESET
 *
 * Description: LCD 设备复位
 *
 * Arguments  : none
 *
 * Returns    : none
 *
 * Notes      : 1、判断是否在屏驱有重新定义LCD复位函数，
 * 					是使用屏驱上定义的LCD复位函数，
 * 					否使用GPIO控制板级配置的LCD复位IO
 *********************************************************************************************************
 */
static void lcd_reset(struct lcd_drive *lcd)
{
    if (lcd->reset) {
        lcd->reset();
    } else {
		if(!lcd_dat){
			return;
		}
        if (lcd_dat->pin_reset == NO_CONFIG_PORT) {
            return ;
        }
        gpio_set_mode(IO_PORT_SPILT(lcd_dat->pin_reset), PORT_OUTPUT_HIGH);
        delay_2ms(5);
        gpio_set_mode(IO_PORT_SPILT(lcd_dat->pin_reset), PORT_OUTPUT_LOW);
        delay_2ms(5);
        gpio_set_mode(IO_PORT_SPILT(lcd_dat->pin_reset), PORT_OUTPUT_HIGH);
        delay_2ms(5);
    }
}
/*
 *********************************************************************************************************
 *                                       LCD BACKLIGHT CONTROL
 *
 * Description: LCD 背光控制
 *
 * Arguments  : on LCD 背光开关标志，0为关，其它值为开
 *
 * Returns    : none
 *
 * Notes      : 1、判断是否在屏驱有重新定义背光控制函数，
 * 					是使用屏驱上定义的背光控制函数，
 * 					否使用GPIO控制板级配置的背光IO
 *
 *              2、配置背光状态标志，打开为true，关闭为false
 *********************************************************************************************************
 */

static void lcd_mcpwm_init()
{

}
int lcd_drv_backlight_ctrl_base(u8 percent)
{
    if (__lcd->backlight_ctrl) {
        __lcd->backlight_ctrl(percent);
    } else if (lcd_dat && (lcd_dat->pin_bl != NO_CONFIG_PORT)) {
        if (percent > 0) {
            lcd_bl_ctrl(1);
        } else {
            lcd_bl_ctrl(0);
        }
    } else {
        return -1;
    }
    return 0;
}
int lcd_drv_backlight_ctrl(u8 percent)
{
    int ret = lcd_drv_backlight_ctrl_base(percent);
    if (ret < 0) {
        backlight_status = 0;
    } else {
        backlight_status = percent;
    }
    return ret;
}

int lcd_drv_power_ctrl(u8 on)
{
    if (!__lcd) {
        __lcd = lcd_drv_get_hdl(TCFG_LCD_MATCH_MODE, LCD_LOGO);
    }
    if (!__this) {
        __this = __lcd->param;
    }

    if (__lcd && __lcd->power_ctrl) {
        __lcd->power_ctrl(on);
    }

    return 0;
}

struct lcd_platform_data *lcd_get_platform_data()
{
    return lcd_dat;
}

static int find_begin(u8 *begin, u8 *end, int pos)
{
    u8 *p = &begin[pos];
    while ((p + 3) < end) {
        if ((p[0] << 24 | p[1] << 16 | p[2] << 8 | p[3]) == BEGIN_FLAG) {
            return (&p[4] - begin);
        }
        p++;
    }

    return -1;
}


static int find_end(u8 *begin, u8 *end, int pos)
{
    u8 *p = &begin[pos];
    while ((p + 3) < end) {
        if ((p[0] << 24 | p[1] << 16 | p[2] << 8 | p[3]) == END_FLAG) {
            return (&p[0] - begin);
        }
        p++;
    }

    return -1;
}

void lcd_drv_cmd_list(u8 *cmd_list, int cmd_cnt)
{
    int i;
    int k;
    u8 *p8;

    u8 *temp = NULL;
    u16 temp_len = 5 * 64;
    u16 len;
    temp = (u8 *)malloc(temp_len);

    for (i = 0; i < cmd_cnt;) {
        int begin = find_begin(cmd_list, &cmd_list[cmd_cnt], i);
        int cnt = 0;
        if ((begin != -1)) {
            int end = find_end(cmd_list, &cmd_list[cmd_cnt], begin);
            if (end != -1) {
                p8 = (u8 *)&cmd_list[begin];
                u8 *param;
                u32 addr;
                if (lcd_get_param(CMD_MODE) == CMD_24BIT) {
                    cnt = end - begin - 3;
                    param = &p8[3];
                    addr = (p8[0] << 16) | (p8[1] << 8) | p8[2];
                } else if (lcd_get_param(CMD_MODE) == CMD_16BIT) {
                    cnt = end - begin - 2;
                    param = &p8[2];
                    addr = (p8[0] << 8) | p8[1];
                } else {
                    cnt = end - begin - 1;
                    param = &p8[1];
                    addr = p8[0];
                }

                if (((p8[0] << 24) | (p8[1] << 16) | (p8[2] << 8) | p8[3]) == REGFLAG_DELAY_FLAG) {
                    LCD_PRINTF("delay %d ms\n", p8[4]);
                    delay_2ms(p8[4] / 2);
#if 0
                } else if (((p8[0] << 24) | (p8[1] << 16) | (p8[2] << 8) | p8[3]) == REGFLAG_CONFIRM_FLAG) {

                    u8 addr1 = p8[4];
                    u8 value = p8[5];
                    u8 timeout = p8[6];

                    LCD_PRINTF("addr : 0x%x, value : 0x%x, timeout : %d\n", addr1, value, timeout);

                    u8 power_mode;
                    u32 jiffies_begin = jiffies_msec();
                    int wait_timeout = jiffies + msecs_to_jiffies(timeout); //超时时间设置
                    while (1) {
                        if (time_after(jiffies, wait_timeout)) {
                            LCD_PRINTF("confirm fail! lcd power_mode status 0x%x wait timeout\n", value);
                            break;
                        }

                        lcd_read_cmd(addr, &power_mode, sizeof(power_mode));
                        if (power_mode == value) {
                            cnt++;
                            if (cnt > 10) {
                                break;
                            }
                        } else {
                            cnt = 0;
                        }
                    }
                    u32 jiffies_end = jiffies_msec();
                    LCD_PRINTF("0x%x wait %d ms\n", addr1, jiffies_end - jiffies_begin);
#endif
                }  else {
                    len = sprintf((char *)temp, "send : 0x%08x(%d), ", addr, cnt);
                    for (k = 0; k < cnt; k++) {
                        len += sprintf((char *)&temp[len], "0x%02x, ", param[k]);
                        if (len > (temp_len - 10)) {
                            len += sprintf((char *)&temp[len], "...");
                            break;
                        }
                    }
                    len += sprintf((char *)&temp[len], "\n");
                    if (len <= temp_len) {
                        LCD_PRINTF("cmd:%s", temp);
                    }

                    lcd_write_cmd(addr, param, cnt);
                }

                i = end + 4;
            }
        }
    }
    free(temp);
}

/*
 *********************************************************************************************************
 *                                       LCD DEVICE MATCH
 *
 * Description: LCD 设备初始化
 *
 * Arguments  : 匹配模式， 屏驱LOGO
 *
 * Returns    : 屏设备句柄
 *
 * Notes      : 1、当只有一个屏驱时，不进行匹配，直接返回该屏驱句柄
 *
 *              2、两种匹配方式 LOGO 或者 ID
 *
 *              3、若存在屏设备句柄时直接返回
 *********************************************************************************************************
 */
struct lcd_drive *lcd_drv_get_hdl(u8 mode, const char *logo)
{
    if (__lcd) {
        return __lcd;
    } else {
        if (mode == LCD_MATCH_BY_LOGO) {
            return lcd_drv_get_hdl_by_logo(logo);
        } else {
            return lcd_drv_get_hdl_by_id();
        }
    }
}

/*
 *********************************************************************************************************
 *                                       LCD DEVICE INIT
 *
 * Description: LCD 设备初始化
 *
 * Arguments  : *p 板级配置的 LCD SPI 信息
 *
 * Returns    : 0 初始化成功
 * 				-1 初始化失败
 *
 * Notes      : 1、判断是否在板级文件配置SPI，是继续，否进入断言，
 *
 *              2、配置SPI可操作IO给IMD操作
 *
 *              3、LCD设备复位
 *
 *              4、SPI模块初始化，IMD模块初始化
 *********************************************************************************************************
 */

int lcd_drv_init(void *p)
{
	lcd_debug("lcd_drv_init ...\n");
    struct ui_devices_cfg *cfg = (struct ui_devices_cfg *)p;
    memcpy(lcd_dat, cfg->private_data, sizeof(struct lcd_platform_data));
    lcd_debug("spi pin rest:%d, en:%d\n", lcd_dat->pin_reset, lcd_dat->pin_en);

    //可通过屏幕logo或者屏幕id匹配屏驱，任选一种匹配方式
    if (!__lcd) {
        __lcd = lcd_drv_get_hdl(TCFG_LCD_MATCH_MODE, LCD_LOGO);
    }
    if (!__this) {
        __this = __lcd->param;
    }
    ASSERT(__lcd, ", don't find lcd_device");
    ASSERT(__this, ", don't find dbi_param");

    /* 如果有使能IO，设置使能IO输出高电平 */
    lcd_en_ctrl(true);

    if (__lcd->row_addr_align && __lcd->column_addr_align) {
        lcd_set_align(__lcd->row_addr_align, __lcd->column_addr_align);
    } else {
        lcd_set_align(1, 1);
    }

    /* lcd复位 */
    lcd_reset(__lcd);

    /* 初始化硬件等 */
    lcd_init(__this);

    /* 屏幕初始化指令 */
    lcd_drv_cmd_list(__lcd->lcd_cmd, __lcd->cmd_cnt);

    u8 power_mode = 0;
    lcd_read_cmd(0x0a, &power_mode, sizeof(power_mode));
    LCD_PRINTF("power_mode : 0x%x\n", power_mode);

    u8 pixel_format = 0;
    lcd_read_cmd(0x0c, &pixel_format, sizeof(pixel_format));
    LCD_PRINTF("pixel_format : 0x%x\n", pixel_format);

    lcd_mcpwm_init();
	return 0;
}
/*
 *********************************************************************************************************
 *                                       GET LCD DEVICE INFO
 *
 * Description: 获取 LCD 设备信息
 *
 * Arguments  : *info LCD 设备信息缓存结构体，根据结构体内容赋值即可
 *
 * Returns    : 0 获取成功
 * 				-1 获取失败
 *
 * Notes      : 1、根据参数结构体的内容，将LCD对应信息赋值给结构体元素
 *********************************************************************************************************
 */
static void lcd_drv_get_screen_info(struct lcd_info *info)
{
    /* imb的宽高 */
    ASSERT(__this);
    info->width = __this->in_width;
    info->height = __this->in_height;
    info->radius = __lcd->radius;
    info->fill_argb = __lcd->fill_argb;

    /* imb的输出格式 */
    info->color_format = __this->in_format;//OUTPUT_FORMAT_RGB565;
    if (info->color_format == OUTPUT_FORMAT_RGB565) {
        info->stride = (info->width * 2 + 3) / 4 * 4;
    } else if (info->color_format == OUTPUT_FORMAT_RGB888) {
        info->stride = (info->width * 3 + 3) / 4 * 4;
    }

    /* 屏幕类型 */
    info->interface = __this->lcd_type;

    info->fps = __this->fps;

    /* 对齐 */
    info->col_align = __lcd->column_addr_align;
    info->row_align = __lcd->row_addr_align;
    if (!info->col_align) {
        info->col_align = 1;
    }
    if (!info->row_align) {
        info->row_align = 1;
    }

    /* 背光状态 */
    info->bl_status = !!backlight_status;
    info->buf_num = __this->buffer_num;

    info->buffer = __this->buffer;
    info->buffer_size = __this->buffer_total_size;

    ASSERT(info->col_align, " = 0, lcd driver column address align error, default value is 1");
    ASSERT(info->row_align, " = 0, lcd driver row address align error, default value is 1");

    /* return 0; */
}

/*$PAGE*/
/*
 *********************************************************************************************************
 *                                       MALLOC DISPLAY BUFFER
 *
 * Description: 申请 LCD 显存 buffer
 *
 * Arguments  : **buf 保存显存buffer指针
 * 				*size 保存显存buffer大小
 *
 * Returns    : 0 成功
 * 				-1 失败
 *
 * Notes      : 1、根据LCD驱动中配置的显存大小和数量申请显存BUFFER
 *
 *				2、将显存buffer指针赋值给参数**buf，显存buffer大小赋值给参数*size
 *
 *				注意：buffer默认是lock状态，此时不能推屏，需由UI框架获取并写入数据后才能推屏
 *********************************************************************************************************
 */

static void lcd_drv_buffer_malloc(u8 **buf, u32 *size)
{
    /* int buf_size = ((__this->lcd_width * __this->lcd_height * 2) + 3) / 4 * 4; */
    ASSERT(__this);
    int buf_size = (__this->buffer_size + 3) / 4 * 4;	// 把buffer大小做四字节对齐

    *buf = (u8 *)malloc(buf_size * __this->buffer_num);


    if (*buf == NULL) {
        ASSERT(0);
        // 如果buffer申请失败
        *size = 0;
        return;
    }

    *size = buf_size * __this->buffer_num;
    __this->buffer = *buf;
    __this->buffer_total_size = *size;

    return;
}



/*$PAGE*/
/*
 *********************************************************************************************************
 *                                       FREE DISPLAY BUFFER
 *
 * Description: 释放 LCD 显存 buffer
 *
 * Arguments  : *buf 显存buffer指针
 *
 * Returns    : 0 成功
 * 				-1 失败
 *
 * Notes      : 1、使用memory API 释放显存buffer
 *********************************************************************************************************
 */

static void lcd_drv_buffer_free(u8 *buf)
{
    if (buf) {
        free(buf);
    }

    return;
}


/*$PAGE*/
/*
 *********************************************************************************************************
 *                                       LCD DRAW BUFFER
 *
 * Description: 把显存 buf 推送到屏幕
 *
 * Arguments  : *buf 显存buffer指针
 * 				len 显存buffer的数据量
 * 				wait 是否等待
 *
 * Returns    : 0 成功
 * 				-1 失败
 *
 * Notes      : 1、使用 IMD 模块将显存buffer推给屏幕
 *********************************************************************************************************
 */

static void lcd_drv_draw(u8 *buf, int xstart, int xend, int ystart, int yend)
{
    lcd_draw(buf, xstart, xend, ystart, yend);
    while(lcd_busy()){
        delay_2ms(1);
    }

}


static void lcd_drv_draw_continue(u8 *buf, int xstart, int xend, int ystart, int yend)
{
    lcd_draw_continue(buf, xstart, xend, ystart, yend);
}

/*$PAGE*/
/*
 *********************************************************************************************************
 *                                       GET LCD DRIVE HANDLER
 *
 * Description: 获取LCD驱动句柄
 *
 * Arguments  : none
 *
 * Returns    : struct lcd_interface* LCD驱动接口句柄
 *
 * Notes      : 1、从LCD接口列表中找到LCD接口句柄并返回
 *********************************************************************************************************
 */

struct lcd_interface *lcd_get_hdl()
{
    struct lcd_interface *p;

    ASSERT(lcd_interface_begin != lcd_interface_end, "don't find lcd interface!");
    for (p = lcd_interface_begin; p < lcd_interface_end; p++) {
        return p;
    }
    return NULL;
}

static void lcd_drv_set_draw_area(u16 xs, u16 xe, u16 ys, u16 ye)
{
    lcd_set_draw_area(xs, xe, ys, ye);
}

static void lcd_drv_clear_screen(u32 color, int xstart, int xend, int ystart, int yend)
{
    lcd_clear(color, xstart, xend, ystart, yend);
    lcd_wait_busy();
}

struct lcd_drive *lcd_drv_get_hdl_by_logo(const char *logo)
{
    struct lcd_drive *p;
    int lcd_type;
    int spi_mode;
    int spi_submode;
    int lcd_num = 0;

    /* LCD_PRINTF("find logo %s\n", logo); */
    ASSERT(lcd_device_begin != lcd_device_end, "don't find lcd device!");

    //统计屏驱的个数
    for (p = lcd_device_begin; p < lcd_device_end; p++) {
        LCD_PRINTF("find logo:%s\n", p->logo);
        lcd_num++;
    }

    //只有一个屏驱时不进行匹配
    if (lcd_num == 1) {
        p = lcd_device_begin;
        LCD_PRINTF("Due to lcd_num = %d, don't match any lcd device. Only one lcd device %s is selected by default.\n", lcd_num, p->logo ? p->logo : "null");
        return p;
    }

    //需确保所有使能的屏驱接口保持一致
    for (p = lcd_device_begin; p < lcd_device_end; p++) {
        if (p == lcd_device_begin) {
            lcd_type = ((struct dbi_param *)p->param)->lcd_type;
            spi_mode = ((struct dbi_param *)p->param)->spi.spi_mode & 0xf0;
            spi_submode = ((struct dbi_param *)p->param)->spi.spi_mode & 0x0f;
        } else {
            ASSERT(lcd_type == ((struct dbi_param *)p->param)->lcd_type, ", all lcd interface must the same");
            ASSERT(spi_mode == (((struct dbi_param *)p->param)->spi.spi_mode & 0xf0), ", all spi_mode must the same");
            ASSERT(spi_submode == (((struct dbi_param *)p->param)->spi.spi_mode & 0x0f), ", all spi_submode must the same");
        }
    }

    for (p = lcd_device_begin; p < lcd_device_end; p++) {
        if (p->logo && logo && !strcmp(p->logo, logo)) {
            LCD_PRINTF("p->logo : %s\n", p->logo);
            return p;
        }
    }
    return NULL;
}

struct lcd_drive *lcd_drv_get_hdl_by_id()
{
    struct lcd_drive *p;
    int lcd_type;
    int spi_mode;
    int spi_submode;
    int lcd_num = 0;

    ASSERT(lcd_device_begin != lcd_device_end, "don't find lcd device!");

    //统计屏驱的个数
    for (p = lcd_device_begin; p < lcd_device_end; p++) {
        lcd_num++;
    }

    //只有一个屏驱时不进行匹配
    if (lcd_num == 1) {
        p = lcd_device_begin;
        LCD_PRINTF("Due to lcd_num = %d, don't match any lcd device. Only one lcd device %s is selected by default.\n", lcd_num, p->logo ? p->logo : "null");
        return p;
    }


    //需确保所有使能的屏驱接口保持一致
    for (p = lcd_device_begin; p < lcd_device_end; p++) {
        if (p == lcd_device_begin) {
            lcd_type = ((struct dbi_param *)p->param)->lcd_type;
            spi_mode = ((struct dbi_param *)p->param)->spi.spi_mode & 0xf0;
            spi_submode = ((struct dbi_param *)p->param)->spi.spi_mode & 0x0f;
        } else {
            ASSERT(lcd_type == ((struct dbi_param *)p->param)->lcd_type, ", all lcd interface must the same");
            ASSERT(spi_mode == (((struct dbi_param *)p->param)->spi.spi_mode & 0xf0), ", all spi_mode must the same");
            ASSERT(spi_submode == (((struct dbi_param *)p->param)->spi.spi_mode & 0x0f), ", all spi_submode must the same");
        }
    }


    for (p = lcd_device_begin; p < lcd_device_end; p++) {
        struct dbi_param *this = (struct dbi_param *)p->param;
        extern struct dbi_variable dbi_var;
        dbi_var.clock_init = false;

        lcd_en_ctrl(true);

        lcd_set_ctrl_pin_func(spi_te_stat);

        lcd_reset(p); /* lcd复位 */
        lcd_init(this); /* 初始化lcd控制器 */


        u32 lcd_read_id = 0;
        if (p->read_id) {
            lcd_read_id = p->read_id();
        }

        LCD_PRINTF("p->logo : %s, p->lcd_id : 0x%x, read_id : 0x%x\n", p->logo ? p->logo : "null", p->lcd_id, lcd_read_id);

        if (p->lcd_id == lcd_read_id) {
            return p;
        }
    }

    return NULL;
}


int lcd_drv_get_info(void *info)
{
    struct lcd_interface *lcd;
    lcd = lcd_get_hdl();
    ASSERT(lcd);
    if (lcd->get_screen_info) {
        lcd->get_screen_info(info);
    }

    return 0;
}




REGISTER_LCD_INTERFACE(lcd) = {
	.init               = lcd_drv_init,
	.draw               = lcd_drv_draw,
	.get_screen_info    = lcd_drv_get_screen_info,
	.buffer_malloc      = lcd_drv_buffer_malloc,
	.buffer_free        = lcd_drv_buffer_free,
	.backlight_ctrl     = lcd_drv_backlight_ctrl,
	.power_ctrl         = lcd_drv_power_ctrl,
	.set_draw_area      = lcd_drv_set_draw_area,
	.clear_screen       = lcd_drv_clear_screen,
};




