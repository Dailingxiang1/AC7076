#ifndef  __GPADC_HW_H__
#define  __GPADC_HW_H__
//br35
#include "gpadc_hw_v4.h"
#include "generic/typedef.h"
#include "gpio.h"
#include "clock.h"

#define ADC_CH_MASK_TYPE_SEL	0xffff0000
#define ADC_CH_MASK_CH_SEL	    0x000000ff
#define ADC_CH_MASK_PMU_VBG_CH_SEL   0x0000ff00

#define ADC_CH_TYPE_BT     	(0x0<<16)
#define ADC_CH_TYPE_AUDIO  	(0x1<<16)
#define ADC_CH_TYPE_PMU    	(0x2<<16)
#define ADC_CH_TYPE_LRC200K (0x3<<16)
#define ADC_CH_TYPE_LRC24M  (0x4<<16)
#define ADC_CH_TYPE_SYSPLL  (0x5<<16)
#define ADC_CH_TYPE_LPCTM   (0x6<<16)
#define ADC_CH_TYPE_CALSSD  (0x7<<16)
#define ADC_CH_TYPE_WAT     (0x8<<16)
#define ADC_CH_TYPE_IO		(0x10<<16)
#define ADC_CH_TYPE_DIFF	(0x11<<16)

#define ADC_CH_BT_      (ADC_CH_TYPE_BT | 0x0)
#define ADC_CH_AUDIO_IREF	    (ADC_CH_TYPE_AUDIO | 0x0)
#define ADC_CH_AUDIO_VCM  	    (ADC_CH_TYPE_AUDIO | 0x1)
#define ADC_CH_AUDIO_MICBIAS  	(ADC_CH_TYPE_AUDIO | 0x2)
#define ADC_CH_AUDIO_MICLDO     (ADC_CH_TYPE_AUDIO | 0x3)
#define ADC_CH_AUDIO_ADCVDD	    (ADC_CH_TYPE_AUDIO | 0x4)
#define ADC_CH_AUDIO_QTLDO	    (ADC_CH_TYPE_AUDIO | 0x5)
#define ADC_CH_AUDIO_QTREF	    (ADC_CH_TYPE_AUDIO | 0x6)
#define ADC_CH_AUDIO_BUFOUT	    (ADC_CH_TYPE_AUDIO | 0x7)
#define ADC_CH_PMU_VBG_WBG04  	(ADC_CH_TYPE_PMU | (0x0<<8) | 0x0)//WBG04
#define ADC_CH_PMU_VBG_MBG08  	(ADC_CH_TYPE_PMU | (0x1<<8) | 0x0)//MBG08
#define ADC_CH_PMU_VBG_LVDVBG  	(ADC_CH_TYPE_PMU | (0x2<<8) | 0x0)//LVDVBG
#define ADC_CH_PMU_VBG_MVBG  	(ADC_CH_TYPE_PMU | (0x3<<8) | 0x0)//MVBG
#define ADC_CH_PMU_VSW  	    (ADC_CH_TYPE_PMU | 0x1)
#define ADC_CH_PMU_PROGI	    (ADC_CH_TYPE_PMU | 0x2)
#define ADC_CH_PMU_OCP_OUT	    (ADC_CH_TYPE_PMU | 0x3)
#define ADC_CH_PMU_VTEMP	    (ADC_CH_TYPE_PMU | 0x4)
#define ADC_CH_PMU_VPWR_4 	    (ADC_CH_TYPE_PMU | 0x5) //1/4vpwr
#define ADC_CH_PMU_VBAT_4 	    (ADC_CH_TYPE_PMU | 0x6)  //1/4vbat
#define ADC_CH_PMU_VBAT_2	    (ADC_CH_TYPE_PMU | 0x7)
#define ADC_CH_PMU_VP17_DCDC    (ADC_CH_TYPE_PMU | 0x8)
#define ADC_CH_PMU_PVDD  	    (ADC_CH_TYPE_PMU | 0x9)
#define ADC_CH_PMU_DCVDD	    (ADC_CH_TYPE_PMU | 0xa)
#define ADC_CH_PMU_DVDD		    (ADC_CH_TYPE_PMU | 0xb)
#define ADC_CH_PMU_WVDD  	    (ADC_CH_TYPE_PMU | 0xc)
#define ADC_CH_PMU_PADC0  	    (ADC_CH_TYPE_PMU | 0xd)
#define ADC_CH_PMU_PVD_PORB_11V (ADC_CH_TYPE_PMU | 0xe)
#define ADC_CH_PMU_VIN_4 	    (ADC_CH_TYPE_PMU | 0xf) //1/4VIN

#define ADC_CH_LRC200K_ (ADC_CH_TYPE_LRC200K | 0x0)
#define ADC_CH_LRC24M_  (ADC_CH_TYPE_LRC24M | 0x0)
#define ADC_CH_SYSPLL_  (ADC_CH_TYPE_SYSPLL | 0x0)
#define ADC_CH_LPCTM_   (ADC_CH_TYPE_LPCTM | 0x0)
#define ADC_CH_CALSSD_  (ADC_CH_TYPE_CALSSD | 0x0)
#define ADC_CH_WAT_     (ADC_CH_TYPE_WAT | 0x0)

#define ADC_CH_IO_PA0	(ADC_CH_TYPE_IO | 0x0)
#define ADC_CH_IO_PA1	(ADC_CH_TYPE_IO | 0x1)
#define ADC_CH_IO_PA5	(ADC_CH_TYPE_IO | 0x2)
#define ADC_CH_IO_PA6	(ADC_CH_TYPE_IO | 0x3)
#define ADC_CH_IO_PA10	(ADC_CH_TYPE_IO | 0x4)
#define ADC_CH_IO_PA11	(ADC_CH_TYPE_IO | 0x5)
#define ADC_CH_IO_PA13	(ADC_CH_TYPE_IO | 0x6)
#define ADC_CH_IO_PB0	(ADC_CH_TYPE_IO | 0x7)
#define ADC_CH_IO_PB1	(ADC_CH_TYPE_IO | 0x8)
#define ADC_CH_IO_PC2	(ADC_CH_TYPE_IO | 0x9)
#define ADC_CH_IO_PC3	(ADC_CH_TYPE_IO | 0xa)
#define ADC_CH_IO_PC10	(ADC_CH_TYPE_IO | 0xb)
#define ADC_CH_IO_PC11	(ADC_CH_TYPE_IO | 0xc)
#define ADC_CH_IO_DP	(ADC_CH_TYPE_IO | 0xd)
#define ADC_CH_IO_DM	(ADC_CH_TYPE_IO | 0xe)
#define ADC_CH_IO_FSPG	(ADC_CH_TYPE_IO | 0xf)

#define ADC_CH_DIFF_ (ADC_CH_TYPE_DIFF | 0x0)

enum AD_CH {
    AD_CH_BT =   ADC_CH_BT_,

    AD_CH_AUDIO_IREF = ADC_CH_AUDIO_IREF,
    AD_CH_AUDIO_VCM	= ADC_CH_AUDIO_VCM,
    AD_CH_AUDIO_MICBIAS = ADC_CH_AUDIO_MICBIAS,
    AD_CH_AUDIO_MICLDO = ADC_CH_AUDIO_MICLDO,
    AD_CH_AUDIO_ADCVDD = ADC_CH_AUDIO_ADCVDD,
    AD_CH_AUDIO_QTLDO = ADC_CH_AUDIO_QTLDO,
    AD_CH_AUDIO_QTREF = ADC_CH_AUDIO_QTREF,
    AD_CH_AUDIO_BUFOUT = ADC_CH_AUDIO_BUFOUT,

    AD_CH_PMU_VBG_WBG04 = ADC_CH_PMU_VBG_WBG04, //WBG04
    AD_CH_PMU_VBG_MBG08 = ADC_CH_PMU_VBG_MBG08, //MBG08
    AD_CH_PMU_VBG_LVDVBG = ADC_CH_PMU_VBG_LVDVBG, //LVDVBG
    AD_CH_PMU_VBG_MVBG = ADC_CH_PMU_VBG_MVBG, //MVBG

    AD_CH_PMU_VSW,
    AD_CH_PMU_PROGI,
    AD_CH_PMU_OCP_OUT,
    AD_CH_PMU_VTEMP,
    AD_CH_PMU_VPWR_4, //1/4vpwr
    AD_CH_PMU_VBAT_4, //1/4vbat
    AD_CH_PMU_VBAT_2,
    AD_CH_PMU_VP17_DCDC,
    AD_CH_PMU_PVDD,
    AD_CH_PMU_DCVDD,
    AD_CH_PMU_DVDD,
    AD_CH_PMU_WVDD,
    AD_CH_PMU_PADC0,
    AD_CH_PMU_PVD_PORB_11V,
    AD_CH_PMU_VIN_4, //1/4VIN

    AD_CH_LRC200K = ADC_CH_LRC200K_,

    AD_CH_LRC24M = ADC_CH_LRC24M_,

    AD_CH_SYSPLL = ADC_CH_SYSPLL_,

    AD_CH_LPCTM = ADC_CH_LPCTM_,

    AD_CH_CALSSD_ = ADC_CH_CALSSD_,

    AD_CH_WAT   = ADC_CH_WAT_,

    AD_CH_IO_PA0 = ADC_CH_IO_PA0,
    AD_CH_IO_PA1,
    AD_CH_IO_PA5,
    AD_CH_IO_PA6,
    AD_CH_IO_PA10,
    AD_CH_IO_PA11,
    AD_CH_IO_PA13,
    AD_CH_IO_PB0,
    AD_CH_IO_PB1,
    AD_CH_IO_PC2,
    AD_CH_IO_PC3,
    AD_CH_IO_PC10,
    AD_CH_IO_PC11,
    AD_CH_IO_DP,
    AD_CH_IO_DM,
    AD_CH_IO_FSPG,

    AD_CH_DIFF  = ADC_CH_DIFF_,

    AD_CH_IOVDD = 0xffffffff,
};

#define ADC_VBG_CENTER      800
#define ADC_VBG_TRIM_STEP   3
#define ADC_VBG_DATA_WIDTH  0

//防编译报错
#define AD_CH_PMU_VBG   AD_CH_PMU_VBG_MVBG
#define AD_CH_LDOREF    AD_CH_PMU_VBG
#define AD_CH_PMU_VBAT  AD_CH_PMU_VBAT_2
#define AD_CH_LPCTMU    AD_CH_LPCTM
#define AD_CH_PMU_VBAT_DIV  2

#define AD_CH_IO_PB7   AD_CH_PMU_PADC0

//gpadc_hw.c 实现
void adc_pmu_vbg_enable();
void adc_pmu_vbg_disable();
void adc_pmu_ch_select(u32 ch);
void adc_audio_ch_select(u32 ch_sel);
void adc_adjust_div(void);
_INLINE_ u8 adc_get_clk_div();


//gpadc_hw_v4.c 实现
void adc_close();
void adc_sample(enum AD_CH ch, u32 ie, u32 calibrate_en);
void adc_wait_enter_idle();
u32 adc_wait_pnd();
void adc_set_enter_idle();
u32 adc_get_res();
_INLINE_ u32 adc_idle_query();
_INLINE_ void adc_register_clear();
void adc_data_res_check();
void adc_suspend();
void adc_resume();
void adc_internal_signal_to_io(enum AD_CH analog_ch, u16 gpio);
void adc_delay_init();
void adc_delay_set(enum AD_CH ch, u32 trig_delay_us, u32 sample_delay);

#endif  /*GPADC_HW_H*/

