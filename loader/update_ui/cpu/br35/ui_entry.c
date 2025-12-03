#include "typedef.h"
#include "app_config.h"
#include "ui.h"
#include "generic/gpio.h"
#include "ui/lcd_spi/lcd_drive.h"
#include "update_status.h"
struct _user_api_param_t {
    u8 version;         //版本号：0
    u8 progress_bar;    //等待中的循环进度，可用于ui显示
    u8 addr[6];         //蓝牙地址
} user_api_param;

extern int _bss_begin[];
extern int _bss_size[];

extern void draw_ring(int center_x, int center_y, int radius, u16 color, int percent);
extern void ui_buffer_init(u8 * buf, u32 len);
ex_api_t *p_ex_api = NULL;

static void (*my_irq_enable)(void) = NULL;
static void (*my_irq_disable)(void) = NULL;
static void (*my_mdelay)(u32) = NULL;

typedef struct _user_api_t {
	char magic[8];
	void (*ex_api_reg_hdl)(ex_api_t *api);
	void (*ui_disp_hdl)(UPDATA_STATUS st, u32 param);
} user_api_t;

struct ui_display {
	struct lcd_interface *lcd;
};
static struct ui_display display = {0};
#define __this (&display)


enum ui_devices_type {
	LED_7,
	LCD_SEG3X9,
	TFT_LCD,//²ÊÆÁ
	DOT_LCD,//µãÕóÆÁ
};

//°å¼¶ÅäÖÃÊý¾Ý½á¹¹
struct ui_devices_cfg {
	enum ui_devices_type type;
	void *private_data;
};



LCD_SPI_PLATFORM_DATA_BEGIN(lcd_spi_data)
    .pin_reset = TCFG_LCD_PIN_RESET,
    .pin_cs = TCFG_LCD_PIN_CS,
    .pin_bl = TCFG_LCD_PIN_BL,
    .pin_dc = TCFG_LCD_PIN_DC,
    .pin_en = TCFG_LCD_PIN_EN,
    .pin_en_ex = TCFG_LCD_PIN_EN_EX,
    .pin_te = TCFG_LCD_PIN_TE,
LCD_SPI_PLATFORM_DATA_END()

const struct ui_devices_cfg ui_cfg_data = {
	.type = TFT_LCD,
	.private_data = (void *) &lcd_spi_data,
};


void my_irq_enable_func(void)
{
    if (my_irq_enable) {
        my_irq_enable();
    }
}

void my_irq_disable_func(void)
{
    if (my_irq_disable) {
        my_irq_disable();
    }
}

int printf(const char *format, ...){
#if LOG_DEBUG_ENABLE
    if(p_ex_api && p_ex_api->print){

        va_list args;
        va_start(args, format);
        int ret = p_ex_api->print(0,format, args);
        va_end(args);
        return ret;
    }
#endif//LOG_DEBUG_ENABLE
    return 0;
}
void put_buf(const u8 *buf, u32 len)
{
	int i;
	for (i = 0; i < len; i++) {
		if (i && (i % 0x10 == 0)) {
			printf(" \n");
		}
		printf("%02x ", buf[i]);
	}
	printf(" \n");
}
int puts(const char *out){
    return 0;
}

int sprintf(char *out, const char *format, ...){
    if(p_ex_api && p_ex_api->print){
        va_list args;
        va_start(args, format);
        int ret = p_ex_api->print(&out,format, args);
        va_end(args);
        return ret;
    }
    return 0;
}



void delay_2ms(int cnt)
{
	int count;
	for (int i = 0; i < cnt; i++) {
        if (my_mdelay) {
            my_mdelay(2);
        } else {
            count = 110 * 50;
            while (count--) {
                __asm__ volatile("nop");
                __asm__ volatile("nop");
                __asm__ volatile("nop");
            }
        }
	}

}


static void delay_nop(int cnt)
{
	while (cnt--) {
		__asm__ volatile("nop");
	}
}

static void ui_flash_handler(void)
{
	if (p_ex_api && p_ex_api->printf) {
		p_ex_api->printf("@");
	}
}

/***************************************************
                    ÆÁ³õÊ¼»¯
***************************************************/
void lcd_screen_init(void *arg)
{
    printf("%s now!!!\n",__func__);
	struct lcd_info info = {0};
	//»ñÈ¡lcd²Ù×÷¾ä±ú
	__this->lcd = lcd_get_hdl();
	ASSERT(__this->lcd);
    //ÖØÐÂ³õÊ¼»¯ÆÁÄ»
	if (__this->lcd->power_ctrl) {
		__this->lcd->power_ctrl(true);
	}
    //³õÊ¼»¯Ó²¼þ¡¢ÆÁÄ»
	if (__this->lcd->init) {
		__this->lcd->init(arg);
	}
    if (__this->lcd->get_screen_info) {
		__this->lcd->get_screen_info(&info);
	}
    //ÇåÆÁÄ»
	if (__this->lcd->clear_screen) {
		__this->lcd->clear_screen(0x000000,0,info.width-1,0,info.height-1);
	}
    //µ÷ÁÁ¶È
	if (__this->lcd->backlight_ctrl) {
		__this->lcd->backlight_ctrl(100);
	}
    //ÉêÇëÆÁÄ»buf
	if (__this->lcd->buffer_malloc) {
		u8 *buf = NULL;
		u32 len;
		__this->lcd->buffer_malloc(&buf, &len);
        //Ìí¼Óbuffer¹ÜÀí
		ui_buffer_init(buf, len);
	}
    printf("%s succ!!!\n",__func__);
}


extern u8 update_text_buf[10][20];
#define DRAW_POINT_MAX  10

static void ui_disp(UPDATA_STATUS st, u32 param)
{
    printf("[%s] st : %d, param : %d\n", __FUNCTION__, st, param);
    struct lcd_info info = {0};
    __this->lcd = lcd_get_hdl();
    if (__this->lcd && __this->lcd->get_screen_info) {
		__this->lcd->get_screen_info(&info);
        ASSERT(info.width*info.height);
	}

    static u8 last_status = 0;
    int text_x,text_y,text_width,text_height;
    u8 str_buf[32] = {0};
    int i = 0;

	switch (st) {
    case UPDATE_PROCESS:
        printf("UPDATE_PROCESS.\n");
		draw_ring(info.width/2, info.height/2, info.width/2, 0x07e0, param / 100);
		break;

	case UPDATE_START:
        printf("UPDATE_START.param:%d\n",param);
        if(param == 0){
            ui_draw_rect(0,0,info.width,info.height,0);
            draw_ring(info.width/2, info.height/2, info.width/2, 0x07e0, 0);
        }else if(param == 1){
            ui_draw_rect(0,0,info.width,info.height,0);
            get_text_rect(&update_text_buf[7][0],strlen(&update_text_buf[7][0]), &text_width, &text_height);
            text_x = info.width/2 - text_width/2;
            text_x = (text_x<0) ? 0:text_x;
            text_y = info.height/2 - text_height/2;
            ui_draw_text(text_x,text_y,text_width,text_height,0xffff,&update_text_buf[7][0],strlen(&update_text_buf[7][0]));

        }else if(param == 2){
            ui_draw_rect(0,0,info.width,info.height,0);
            get_text_rect(&update_text_buf[8][0],strlen(&update_text_buf[8][0]), &text_width, &text_height);
            text_x = info.width/2 - text_width/2;
            text_x = (text_x<0) ? 0:text_x;
            text_y = info.height/2 - text_height/2;
            ui_draw_text(text_x,text_y,text_width,text_height,0xffff,&update_text_buf[8][0],strlen(&update_text_buf[8][0]));

        }
		break;


	case UPDATE_STOP:
	    printf("UPDATE_STOP.\n");
		if (param == UPDATE_ERR_NONE) {

		} else {
			printf("error code:0x%x\n", param);
		}
		break;
    case EX_API_UPDATE_TIPS_WAIT_CONN:
        if((!user_api_param.progress_bar)||(last_status!=st))
        {
            ui_draw_rect(0,0,info.width,info.height,0);
        }

        printf("EX_API_UPDATE_TIPS_WAIT_CONN\n");
        printf("ADDR:%X:%X:%X:%X:%X:%X",user_api_param.addr[0],user_api_param.addr[1],user_api_param.addr[2],user_api_param.addr[3],user_api_param.addr[4],user_api_param.addr[5]);
        memcpy(&user_api_param,param,sizeof(struct _user_api_param_t));

        get_text_rect(&update_text_buf[0][0],strlen(&update_text_buf[0][0]), &text_width, &text_height);
        text_x = info.width/2 - text_width/2;
        ui_draw_text(text_x,100,text_width,text_height,0xffff,&update_text_buf[0][0],strlen(&update_text_buf[0][0]));

        memcpy(str_buf, &update_text_buf[4][0],strlen(&update_text_buf[4][0]));
        bt_addr_to_str(&str_buf[strlen(str_buf)],user_api_param.addr);

//		printf("str_buf[%d]",strlen(str_buf));
//		put_buf(str_buf,strlen(str_buf));
        get_text_rect(str_buf,strlen(str_buf), &text_width, &text_height);
        text_x = info.width/2 - text_width/2;
        ui_draw_text(text_x,160,text_width,text_height,0xffff,str_buf,strlen(str_buf));

        for(i = 0;i < user_api_param.progress_bar;i++){
            get_text_rect(&update_text_buf[6][0],strlen(&update_text_buf[6][0]), &text_width, &text_height);
            ui_draw_text(50 + (text_width+5) * i,220,text_width,text_height,0xffff,&update_text_buf[6][0],strlen(&update_text_buf[6][0]));
        }

        break;
    case EX_API_UPDATE_TIPS_WAIT_UPDATE:
        if((!user_api_param.progress_bar)||(last_status!=st))
        {
            ui_draw_rect(0,0,info.width,info.height);
        }
        printf("EX_API_UPDATE_TIPS_WAIT_UPDATE\n");
        memcpy(&user_api_param,param,sizeof(struct _user_api_param_t));

        get_text_rect(&update_text_buf[1][0],strlen(&update_text_buf[1][0]), &text_width, &text_height);
        text_x = info.width/2 - text_width/2;
        ui_draw_text(text_x,100,text_width,text_height,0xffff,&update_text_buf[1][0],strlen(&update_text_buf[1][0]));

        memcpy(str_buf, &update_text_buf[4][0],strlen(&update_text_buf[4][0]));
        bt_addr_to_str(&str_buf[strlen(str_buf)],user_api_param.addr);

        get_text_rect(str_buf,strlen(str_buf), &text_width, &text_height);
        text_x = info.width/2 - text_width/2;
        ui_draw_text(text_x,160,text_width,text_height,0xffff,str_buf,strlen(str_buf));
		for(i = 0;i < user_api_param.progress_bar;i++){
            get_text_rect(&update_text_buf[6][0],strlen(&update_text_buf[6][0]), &text_width, &text_height);
            ui_draw_text(50 + (text_width+5) * i,220,text_width,text_height,0xffff,&update_text_buf[6][0],strlen(&update_text_buf[6][0]));
        }
        break;
    case EX_API_UPDATE_TIPS_WAIT_START_UPDATE:
        if((!user_api_param.progress_bar)||(last_status!=st))
        {
            ui_draw_rect(0,0,info.width,info.height);
        }
        printf("EX_API_UPDATE_TIPS_WAIT_START_UPDATE\n");
        memcpy(&user_api_param,param,sizeof(struct _user_api_param_t));

        get_text_rect(&update_text_buf[2][0],strlen(&update_text_buf[2][0]), &text_width, &text_height);
        text_x = info.width/2 - text_width/2;
        ui_draw_text(text_x,80,text_width,text_height,0xffff,&update_text_buf[2][0],strlen(&update_text_buf[2][0]));

        get_text_rect(&update_text_buf[3][0],strlen(&update_text_buf[3][0]), &text_width, &text_height);
        text_x = info.width/2 - text_width/2;
        ui_draw_text(text_x,140,text_width,text_height,0xffff,&update_text_buf[3][0],strlen(&update_text_buf[3][0]));
        memcpy(str_buf, &update_text_buf[4][0],strlen(&update_text_buf[4][0]));
        bt_addr_to_str(&str_buf[strlen(str_buf)],user_api_param.addr);

		//put_buf(str_buf,strlen(str_buf));
        get_text_rect(str_buf,strlen(str_buf), &text_width, &text_height);
        text_x = info.width/2 - text_width/2;
        ui_draw_text(text_x,200,text_width,text_height,0xffff,str_buf,strlen(str_buf));

		for(i = 0;i < user_api_param.progress_bar;i++){
            get_text_rect(&update_text_buf[6][0],strlen(&update_text_buf[6][0]), &text_width, &text_height);
            ui_draw_text(50 + (text_width+5) * i,260,text_width,text_height,0xffff,&update_text_buf[6][0],strlen(&update_text_buf[6][0]));
        }
        break;
    case EX_API_UPDATE_TIPS_UPDATEING:
        printf("EX_API_UPDATE_TIPS_UPDATEING\n");
        memcpy(&user_api_param,param,sizeof(struct _user_api_param_t));
        if((last_status!=st))
        {
            ui_draw_rect(0,0,info.width,info.height,0);

            get_text_rect(&update_text_buf[9][0],strlen(&update_text_buf[9][0]), &text_width, &text_height);
            text_x = info.width/2 - text_width/2;
            ui_draw_text(text_x,100,text_width,text_height,0xffff,&update_text_buf[9][0],strlen(&update_text_buf[9][0]));
            memcpy(str_buf, &update_text_buf[4][0],strlen(&update_text_buf[4][0]));
            bt_addr_to_str(&str_buf[strlen(str_buf)],user_api_param.addr);

            put_buf(str_buf,strlen(str_buf));
            get_text_rect(str_buf,strlen(str_buf), &text_width, &text_height);
            text_x = info.width/2 - text_width/2;
            ui_draw_text(text_x,160,text_width,text_height,0xffff,str_buf,strlen(str_buf));
//            get_text_rect(&update_text_buf[6][0],strlen(&update_text_buf[6][0]), &text_width, &text_height);
//            for(int point = 0;point<DRAW_POINT_MAX;point++){
//                ui_draw_text(100 + (text_width+4) * point,320,text_width,text_height,0xffff,&update_text_buf[6][0],strlen(&update_text_buf[6][0]));
//            }
        }
        //printf("%x %x %x %x",update_text_buf[9][0],update_text_buf[9][1],update_text_buf[9][2],update_text_buf[9][3]);

        int count = user_api_param.progress_bar % (2*DRAW_POINT_MAX);


        get_text_rect(&update_text_buf[6][0],strlen(&update_text_buf[6][0]), &text_width, &text_height);
        if(count%2 == 1){
            ui_draw_text(50 + (text_width+5) * count/2,220,text_width,text_height,0xffff,&update_text_buf[6][0],strlen(&update_text_buf[6][0]));
        }else{
            ui_draw_rect(50 + (text_width+5) * count/2,220,text_width,text_height,0);
        }

	default:

		break;
	}
	last_status = st;
}

//ÓÉota loaderµ÷ÓÃ£¬Íê³É½Ó¿ÚºÍÆÁÄ»³õÊ¼»¯
static void ex_api_register(ex_api_t *api)
{
	p_ex_api = api;
    printf("bss_begin :0x%x, bss_size : 0x%x\n", _bss_begin, _bss_size);
	//p_ex_api->printf("bss_begin :0x%x, bss_size : 0x%x\n", _bss_begin, _bss_size);

	//memset((void *)_bss_begin, 0, (int)_bss_size);

	//printf("[%s] 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", __FUNCTION__, api->printf, api->put_buf, api->sys_timer_add, api->sys_timeout_add, api->sys_timer_del, api->irq_disable, api->irq_enable, api->mdelay, api->request_irq_func);

	if (api->irq_disable) {
        my_irq_disable = api->irq_disable;
	}

	if (api->irq_enable) {
        my_irq_enable = api->irq_enable;
	}

	if (api->mdelay) {
        my_mdelay = api->mdelay;
	}

	lcd_screen_init(&ui_cfg_data);
//printf("%s %d\n", __func__, __LINE__);
	//draw_ring(160, 160, 160, 0x07e0, 100);//²ÎÊý ÊäÈë£ºx×ø±ê ¡¢y×ø±ê¡¢°ë¾¶¡¢ÑÕÉ«¡¢°Ù·Ö°Ù(0~100)
//printf("%s %d\n", __func__, __LINE__);
};

__attribute__((section(".api_tab"), used))
static  const user_api_t user_api_ins = {
	.magic = {'E', 'X', 'A', 'P',},
	.ex_api_reg_hdl = ex_api_register,
	.ui_disp_hdl = ui_disp,
};


