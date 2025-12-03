#ifndef __P33_H__
#define __P33_H__


#include "typedef.h"

#include "p33_sfr.h"

#include "p33_app.h"


//
//
//					lvd
//
//
//
/****************************************************************/
typedef enum {
    LVD_RESET_MODE,		//复位模式
    LVD_EXCEPTION_MODE, //异常模式，进入异常中断
    LVD_WAKEUP_MODE,    //唤醒模式，进入唤醒中断，callback参数为回调函数
} LVD_MODE;

typedef enum {
    VLVD_SEL_166V = 0,
    VLVD_SEL_177V,
    VLVD_SEL_188V,
    VLVD_SEL_199V,
    VLVD_SEL_210V,
    VLVD_SEL_221V,
    VLVD_SEL_232V,
    VLVD_SEL_243V,
    VLVD_SEL_254V,
    VLVD_SEL_265V,
    VLVD_SEL_276V,
    VLVD_SEL_287V,
    VLVD_SEL_298V,
    VLVD_SEL_309V,
    VLVD_SEL_320V,
    VLVD_SEL_331V,
} LVD_VOL;

void lvd_config(LVD_VOL vol, u8 expin_en, LVD_MODE mode, void (*callback));
void reset_pin_open(void);
void reset_pin_close(void);
void reset_pin_init(u32 pin, u32 level, u32 time);
void reset_pin1_init(u32 pin, u32 level, u32 time);

void chip_reset();

void set_vddio_level(u8 level);

void set_dvdd_leve(u8 level);

void set_dcvdd_leve(u8 level);

void p33_soft_reset(void);

void latch_reset(void);


#endif




