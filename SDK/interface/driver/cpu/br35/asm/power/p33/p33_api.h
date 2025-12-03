#ifndef __P33_API_H__
#define __P33_API_H__


//
//
//					vol
//
//
//
/****************************************************************/

enum DVDD_VOL {
    DVDD_VOL_0840MV = 0,
    DVDD_VOL_0870MV,
    DVDD_VOL_0900MV,
    DVDD_VOL_0930MV,
    DVDD_VOL_0960MV,
    DVDD_VOL_0990MV,
    DVDD_VOL_1020MV,
    DVDD_VOL_1050MV,
    DVDD_VOL_1080MV,
    DVDD_VOL_1110MV,
    DVDD_VOL_1140MV,
    DVDD_VOL_1170MV,
    DVDD_VOL_1200MV,
    DVDD_VOL_1230MV,
    DVDD_VOL_1260MV,
    DVDD_VOL_1290MV,
};

/*enum DVDD2_VOL {*/
/*};*/

/*enum RVDD_VOL {*/
/*};*/

/*enum RVDD2_VOL {*/
/*};*/

/*enum BTVDD_VOL {*/
/*};*/

enum DCVDD_VOL {
    DCVDD_VOL_1000MV = 0,
    DCVDD_VOL_1050MV,
    DCVDD_VOL_1100MV,
    DCVDD_VOL_1150MV,
    DCVDD_VOL_1200MV,
    DCVDD_VOL_1250MV,
    DCVDD_VOL_1300MV,
    DCVDD_VOL_1350MV,
    DCVDD_VOL_1400MV,
    DCVDD_VOL_1450MV,
    DCVDD_VOL_1500MV,
    DCVDD_VOL_1550MV,
    DCVDD_VOL_1600MV,
};

enum VDDIOM_VOL {
    VDDIOM_VOL_21V = 0,
    VDDIOM_VOL_22V,
    VDDIOM_VOL_23V,
    VDDIOM_VOL_24V,
    VDDIOM_VOL_25V,
    VDDIOM_VOL_26V,
    VDDIOM_VOL_27V,
    VDDIOM_VOL_28V,
    VDDIOM_VOL_29V,
    VDDIOM_VOL_30V,
    VDDIOM_VOL_31V,
    VDDIOM_VOL_32V,
    VDDIOM_VOL_33V,
    VDDIOM_VOL_34V,
    VDDIOM_VOL_35V,
    VDDIOM_VOL_36V,
};

enum VDDIOW_VOL {
    VDDIOW_VOL_21V = 0,
    VDDIOW_VOL_22V,
    VDDIOW_VOL_23V,
    VDDIOW_VOL_24V,
    VDDIOW_VOL_25V,
    VDDIOW_VOL_26V,
    VDDIOW_VOL_27V,
    VDDIOW_VOL_28V,
    VDDIOW_VOL_29V,
    VDDIOW_VOL_30V,
    VDDIOW_VOL_31V,
    VDDIOW_VOL_32V,
    VDDIOW_VOL_33V,
    VDDIOW_VOL_34V,
    VDDIOW_VOL_35V,
    VDDIOW_VOL_36V,
};

enum WVDD_VOL {
    WVDD_VOL_0500MV = 0,
    WVDD_VOL_0550MV,
    WVDD_VOL_0600MV,
    WVDD_VOL_0650MV,
    WVDD_VOL_0700MV,
    WVDD_VOL_0750MV,
    WVDD_VOL_0800MV,
    WVDD_VOL_0850MV,
    WVDD_VOL_0900MV,
    WVDD_VOL_0950MV,
    WVDD_VOL_1000MV,
    WVDD_VOL_1050MV,
    WVDD_VOL_1100MV,
    WVDD_VOL_1150MV,
    WVDD_VOL_1200MV,
    WVDD_VOL_1250MV,
};

enum PVDD_VOL {
    PVDD_VOL_0500MV = 0,
    PVDD_VOL_0550MV,
    PVDD_VOL_0600MV,
    PVDD_VOL_0650MV,
    PVDD_VOL_0700MV,
    PVDD_VOL_0750MV,
    PVDD_VOL_0800MV,
    PVDD_VOL_0850MV,
    PVDD_VOL_0900MV,
    PVDD_VOL_0950MV,
    PVDD_VOL_1000MV,
    PVDD_VOL_1050MV,
    PVDD_VOL_1100MV,
    PVDD_VOL_1150MV,
    PVDD_VOL_1200MV,
    PVDD_VOL_1250MV,
};

void dvdd_vol_sel(enum DVDD_VOL vol);
enum DVDD_VOL get_dvdd_vol_sel();
/*void dvdd2_vol_sel(enum DVDD2_VOL vol);*/
/*enum DVDD2_VOL get_dvdd2_vol_sel();*/

/*void rvdd_vol_sel(enum RVDD_VOL vol);*/
/*enum RVDD_VOL get_rvdd_vol_sel();*/
/*void rvdd2_vol_sel(enum RVDD2_VOL vol);*/
/*enum RVDD2_VOL get_rvdd2_vol_sel();*/

void dcvdd_vol_sel(enum DCVDD_VOL vol);
enum DCVDD_VOL get_dcvdd_vol_sel();

/*void btvdd_vol_sel(enum BTVDD_VOL vol);*/
/*enum BTVDD_VOL get_btvdd_vol_sel();*/

void pvdd_config(u32 lev, u32 low_lev, u32 output);
void pvdd_output(u32 output);

void vddiom_vol_sel(enum VDDIOM_VOL vol);
enum VDDIOM_VOL get_vddiom_vol_sel();
void vddiow_vol_sel(enum VDDIOW_VOL vol);
enum VDDIOW_VOL get_vddiow_vol_sel();
u32 get_vddiom_vol();

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

void lvd_en(u8 en);
void lvd_config(LVD_VOL vol, u8 expin_en, LVD_MODE mode, void (*callback));

//
//
//                    pinr
//
//
//
//******************************************************************
void gpio_longpress_pin0_reset_config(u32 pin, u32 level, u32 time, u32 release, u32 pull_enable, u32 latch_en);
void gpio_longpress_pin1_reset_config(u32 pin, u32 level, u32 time, u32 release);



//
//
//                    dcdc
//
//
//
//******************************************************************
enum POWER_MODE {
    //LDO模式
    PWR_LDO15,
    //DCDC模式
    PWR_DCDC15,
};

enum POWER_DCDC_TYPE {
    PWR_DCDC12 = 2,
    PWR_DCDC18_DCDC12 = 6,
    PWR_DCDC18_DCDC12_DCDC09 = 7,
};

enum {
    DCDC09 = 1,
    DCDC12 = 2,
    DCDC18 = 4,
};

void power_set_dcdc_type(enum POWER_DCDC_TYPE type);
void power_set_mode(enum POWER_MODE mode);


enum DVD_SHORT_DCV_MODE {
    DVDD_SHORT_DCVDDDIS = 0,
    DVDD_SHORT_DCVDD_EN,
};
void dcvdd_level_cfg(u8 dcvdd_level_set);
void dvdd_short_dcvdd(enum DVD_SHORT_DCV_MODE short_mode, u8 dcvdd_level_set);

//每个滤波参数不一样
#define MAX_WAKEUP_PORT     8  //最大同时支持数字io输入个数
#define MAX_WAKEUP_ANA_PORT 3  //最大同时支持模拟io输入个数

#endif
