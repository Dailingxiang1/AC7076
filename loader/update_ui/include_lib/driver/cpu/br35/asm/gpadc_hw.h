#ifndef  __GPADC_HW_H__
#define  __GPADC_HW_H__
//br35专用
#include "generic/typedef.h"
#include "asm/efuse.h"
#include "gpio.h"
#include "jiffies.h"
#include "clock.h"

#define ADC_CH_MASK_TYPE_SEL	0xffff0000
#define ADC_CH_MASK_CH_SEL	    0x0000ffff

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
#define ADC_CH_AUDIO_   (ADC_CH_TYPE_AUDIO | 0x0)

#define ADC_CH_PMU_VBG  	    (ADC_CH_TYPE_PMU | 0x0)//MVBG/WVBG
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

    AD_CH_AUDIO = ADC_CH_AUDIO_,

    AD_CH_PMU_VBG = ADC_CH_PMU_VBG, //MVBG/WVBG
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

#define AD_CH_IO_CH0    IO_PORTA_00
#define AD_CH_IO_CH1    IO_PORTA_01
#define AD_CH_IO_CH2    IO_PORTA_05
#define AD_CH_IO_CH3    IO_PORTA_06
#define AD_CH_IO_CH4    IO_PORTA_10
#define AD_CH_IO_CH5    IO_PORTA_11
#define AD_CH_IO_CH6    IO_PORTA_13
#define AD_CH_IO_CH7    IO_PORTB_00
#define AD_CH_IO_CH8    IO_PORTB_01
#define AD_CH_IO_CH9    IO_PORTC_02
#define AD_CH_IO_CH10   IO_PORTC_03
#define AD_CH_IO_CH11   IO_PORTC_10
#define AD_CH_IO_CH12   IO_PORTC_11
#define AD_CH_IO_CH13   IO_PORT_DP
#define AD_CH_IO_CH14   IO_PORT_DM
#define AD_CH_IO_CH15   0xff//IO_PORT_FSPG

#define ADC_VBG_CENTER      800
#define ADC_VBG_TRIM_STEP   3
#define ADC_VBG_DATA_WIDTH  4

//防编译报错
#define AD_CH_LDOREF    AD_CH_PMU_VBG
#define AD_CH_PMU_VBAT  AD_CH_PMU_VBAT_4
#define AD_CH_LPCTMU    AD_CH_LPCTM

#define AD_CH_IO_PB7   AD_CH_PMU_PADC0 //仅br35使用

#endif  /*GPADC_HW_H*/

