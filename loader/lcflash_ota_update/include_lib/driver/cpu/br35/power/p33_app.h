#ifndef __P33_APP_H__
#define __P33_APP_H__


#include "typedef.h"

#include "p33_sfr.h"


//=============================================================//
//                       P33 SFR                           //
//=============================================================//

//ROM
u8 p33_buf(u8 buf);

#define p33_xor_1byte(addr, data0)      (*((volatile u8 *)&addr + 0x300*4)  = data0); asm volatile ("csync")
//#define p33_xor_1byte(addr, data0)      (*((volatile u8 *)&addr + 0x300*4)  = data0)
// #define p33_xor_1byte(addr, data0)      addr ^= (data0)

#define p33_or_1byte(addr, data0)       (*((volatile u8 *)&addr + 0x200*4)  = data0); asm volatile ("csync")
//#define p33_or_1byte(addr, data0)       (*((volatile u8 *)&addr + 0x200*4)  = data0)
// #define p33_or_1byte(addr, data0)       addr |= (data0)

#define p33_and_1byte(addr, data0)      (*((volatile u8 *)&addr + 0x100*4)  = (data0)); asm volatile ("csync")
//#define p33_and_1byte(addr, data0)      (*((volatile u8 *)&addr + 0x100*4)  = (data0))
//#define p33_and_1byte(addr, data0)      addr &= (data0)

// void p33_tx_1byte(u16 addr, u8 data0);
#define p33_tx_1byte(addr, data0)       addr = data0

// u8 p33_rx_1byte(u16 addr);
#define p33_rx_1byte(addr)              addr

#define P33_CON_SET(sfr, start, len, data)  (sfr = (sfr & ~((~(0xffffffff << (len))) << (start))) | \
	 (((data) & (~(0xffffffff << (len)))) << (start)))

#define P33_CON_GET(sfr)    (sfr)



#if 1

#define p33_fast_access(reg, data, en)           \
{ 												 \
    if (en) {                                    \
		p33_or_1byte(reg, (data));               \
    } else {                                     \
		p33_and_1byte(reg, ~(data));             \
    }                                            \
}

#else

#define p33_fast_access(reg, data, en)         \
{                                              \
	if (en) {                                  \
       	reg |= (data);                         \
	} else {                                   \
		reg &= ~(data);                        \
    }                                          \
}

#endif


//
//
//					for p33_analog
//
//
//
/************************P3_PSW_CON0*****************************/
#define DVD2SVD_SHORT_EN(en)      	p33_fast_access(P3_PSW_CON0, BIT(6), en)

#define SVD2RVD2_SHORT_EN(en)      	p33_fast_access(P3_PSW_CON0, BIT(5), en)

#define SVD2RVD_SHORT_EN(en)      	p33_fast_access(P3_PSW_CON0, BIT(4), en)
#define RVDD2_CAP_EN(en)      	    p33_fast_access(P3_PSW_CON0, BIT(3), en)
#define RVD2_EN(en)    	            p33_fast_access(P3_PSW_CON0, BIT(2), en)

#define RVDD_CAP_EN(en)      	    p33_fast_access(P3_PSW_CON0, BIT(1), en)

#define RVD_EN(en)    	            p33_fast_access(P3_PSW_CON0, BIT(0), en)

#define NVD2IO_SHORT_EN(en)		    p33_fast_access(P3_PSW_CON1, BIT(7), en)

#define DVDD2_IFULL_EN(en)      	p33_fast_access(P3_PSW_CON1, BIT(6), en)
#define DVDD2_BYPASS_EN(en)      	p33_fast_access(P3_PSW_CON1, BIT(5), en)
#define DVDD2_EN(en)				p33_fast_access(P3_PSW_CON1, BIT(4), en)

#define RVDD2_BYPASS_EN(en)      	p33_fast_access(P3_PSW_CON1, BIT(3), en)

#define RVDD_BYPASS_EN(en)      	p33_fast_access(P3_PSW_CON1, BIT(2), en)
#define WVD2SVD_SHORT_EN(en)     	p33_fast_access(P3_PSW_CON1, BIT(1), en)
#define WVDD_EN(en)     			p33_fast_access(P3_PSW_CON1, BIT(0), en)
#define WVDD_PDOWN_ENTER()			P33_CON_SET(P3_PSW_CON1, 0, 2, 0x03); P33_CON_SET(P3_PSW_CON0, 4, 3, 0x7)

#define WVDD_POFF_ENTER()			P33_CON_SET(P3_PSW_CON1, 0, 2, 0x03); P33_CON_SET(P3_PSW_CON0, 4, 3, 0x1)

#define WVDD_PDOWN_POFF_EXIT() 		P33_CON_SET(P3_PSW_CON0, 4, 3, 0x07); P33_CON_SET(P3_PSW_CON1, 1, 1, 0x00)

/************************P3_PSW_CON2*****************************/
//SS: soft start of VPQS
#define VQPS_SS_EN(en)      		p33_fast_access(P3_PSW_CON2, BIT(1), en)

#define VQPS_EN(en)					p33_fast_access(P3_PSW_CON2, BIT(0), en)

/************************P3_ANA_CON*****************************/
//#define EVDD_IFULL_EN(en)      		p33_fast_access(P3_ANA_CON, BIT(0), en)

//DS: decrease undershoot and overshoot
//#define DVD_DS_EN(en)				p33_fast_access(P3_ANA_CON, BIT(1), en)

//#define RVD_DS_EN(en)      			p33_fast_access(P3_ANA_CON, BIT(2), en)

/************************P3_PMU_ADC0*****************************/
enum {
    VBG_TEST_SEL_WBG04,
    VBG_TEST_SEL_MBG04,
    VBG_TEST_SEL_MBG08,
    VBG_TEST_SEL_LVDBG,
};
#define VBG_TEST_SEL(sel)			P33_CON_SET(P3_PMU_ADC0, 4, 2, sel)

#define VBG_TEST_EN(en)				p33_fast_access(P3_PMU_ADC0, BIT(3), en)

#define VBG_BUFFER_EN(en)			p33_fast_access(P3_PMU_ADC0, BIT(2), en)

#define PMU_TOADC_OE(en)			p33_fast_access(P3_PMU_ADC0, BIT(1), en)

#define PMU_TOADC_EN(en)			p33_fast_access(P3_PMU_ADC0, BIT(0), en)

/************************P3_PMU_ADC1*****************************/
#define ADC_CHANNEL_SEL(ch)     	P33_CON_SET(P3_PMU_ADC1, 0, 4, ch)

/************************P3_VBG_CON0*****************************/
#define MVBG_SEL(sel)				P33_CON_SET(P3_VBG_CON0, 0, 4, sel)

#define WVBG_SEL(sel)				P33_CON_SET(P3_VBG_CON0, 4, 4, sel)

/************************P3_IOV_CON0*****************************/

#define VDDIOW_VOL_SEL(lev)     	P33_CON_SET(P3_IOV_CON0, 4, 4, lev)

#define GET_VDDIOW_VOL_SEL()			(P33_CON_GET(P3_IOV_CON0)>>4 & 0xf)

//vddiom_lev

#define VDDIOM_VOL_SEL(lev)     	P33_CON_SET(P3_IOV_CON0, 0, 4, lev)

#define GET_VDDIOM_VOL_SEL()        	(P33_CON_GET(P3_IOV_CON0) & 0xf)

/************************P3_IOV_CON1*****************************/
#define VDDIO_HD_SEL(hd)       		P33_CON_SET(P3_IOV_CON1, 0, 2, hd)

/************************P3_DCV_CON0*****************************/
#define DCVD_DEOVSHOT_EN(en)       	p33_fast_access(P3_DCV_CON0, BIT(7), en)

#define DCVD_CAP_EN(en)       		p33_fast_access(P3_DCV_CON0, BIT(6), en)

#define DCVD_HD_SEL(sel)			P33_CON_SET(P3_DCV_CON0, 4, 2, sel)


#define DCVDD_DEFAULT_VOL			DCVDD_VOL_SEL_120V

#define GET_DCVDD_VOL_SEL()      	(P33_CON_GET(P3_DCV_CON0) & 0xf)

#define DCVDD_VOL_SEL(sel)      	P33_CON_SET(P3_DCV_CON0, 0, 4, sel)


/*******************************************************************/
/*
 *-------------------P3_DCV_CON0
 */
enum {
    DCVDD_VOL_SEL_100V = 0,
    DCVDD_VOL_SEL_1025V,
    DCVDD_VOL_SEL_105V,
    DCVDD_VOL_SEL_1075V,
    DCVDD_VOL_SEL_110V,
    DCVDD_VOL_SEL_1125V,
    DCVDD_VOL_SEL_115V,
    DCVDD_VOL_SEL_1175V,
    DCVDD_VOL_SEL_120V,
    DCVDD_VOL_SEL_1225V,
    DCVDD_VOL_SEL_125V,
    DCVDD_VOL_SEL_1275V,
    DCVDD_VOL_SEL_130V,
    DCVDD_VOL_SEL_1325V,
    DCVDD_VOL_SEL_135V,
    DCVDD_VOL_SEL_1375V,
};

#define DCVD_SEL(sel)        P3_DCV_CON0 = (P3_DCV_CON0 & (~0xf)) | sel
#define DCVDD_DEFAULT_VOL    DCVDD_VOL_SEL_120V

/************************P3_DCD_CON0*****************************/
enum {
    BTDCDC_OSC_SEL0520KHz = 0,
    BTDCDC_OSC_SEL0762KHz,
    BTDCDC_OSC_SEL0997KHz,
    BTDCDC_OSC_SEL1220KHz,
    BTDCDC_OSC_SEL1640KHz,
    BTDCDC_OSC_SEL1840KHz,
    BTDCDC_OSC_SEL2040KHz,
    BTDCDC_OSC_SEL2220MHz,
};

#define BTDCDC_OSC_SEL(sel)     	P33_CON_SET(P3_DCD_CON0, 5, 3, sel)

#define BTDCDC_DUTY_SEL(sel)    	P33_CON_SET(P3_DCD_CON0, 3, 2, sel)

#define BTDCDC_V17_TEST_OE(en)		p33_fast_access(P3_DCD_CON0, BIT(2), en);

#define BTDCDC_RAMP_SHORT(en)       p33_fast_access(P3_DCD_CON0, BIT(1), en)

#define BTDCDC_PFM_MODE(en)     	p33_fast_access(P3_DCD_CON0, BIT(0), en)

#define GET_BTDCDC_PFM_MODE()   	(P33_CON_GET(P3_DCD_CON0) & BIT(0) ? 1 : 0)

/************************P3_DCD_CON1*****************************/
#define BTDCDC_COMP_HD(sel)     	P33_CON_SET(P3_DCD_CON1, 6, 2, sel)

#define BTDCDC_ISENSE_HD(sel) 		P33_CON_SET(P3_DCD_CON1, 4, 2, sel)

#define BTDCDC_DT_S(sel)          	P33_CON_SET(P3_DCD_CON1, 2, 2, sel)

#define BTDCDC_V21_RES_S(sel) 		P33_CON_SET(P3_DCD_CON1, 0, 2, sel)

/************************P3_DCD_CON2*****************************/
#define BTDCDC_PMOS_S(sel)    		P33_CON_SET(P3_DCD_CON2, 5, 3, sel)

#define BTDCDC_NMOS_S(sel)    		P33_CON_SET(P3_DCD_CON2, 1, 3, sel)

/************************P3_DCD_CON3*****************************/
#define BTDCDC_OSC_TEST_OE(en)  	p33_fast_access(P3_DCD_CON3, BIT(7), en)

#define BTDCDC_HD_BIAS_SEL(sel) 	P33_CON_SET(P3_DCD_CON3, 5, 2, sel)

#define BTDCDC_CLK_SEL(sel)     	p33_fast_access(P3_DCD_CON3, BIT(4), sel)

#define GET_BTDCDC_CLK_SEL()    	((P33_CON_GET(P3_DCD_CON3) & BIT(4) ? 1 : 0))

#define BTDCDC_ZCD_RES(sel)     	P33_CON_SET(P3_DCD_CON3, 2, 2, sel)

#define BTDCDC_ZCD_EN(en)       	p33_fast_access(P3_DCD_CON3, BIT(0), en)

/************************P3_DCD_CON4*****************************/
#define BTDCDC_VHH_SEL(sel)			P33_CON_SET(P3_DCD_CON4, 4, 3, sel)

#define BTDCDC_PFM_HYS_SEL(sel)		P33_CON_SET(P3_DCD_CON4, 0, 2, sel)

/*******************************************************************/
/*
 *-------------------P3_DVD_CON0
 */
enum {
    DVDD_VOL_SEL_0725V = 0,
    DVDD_VOL_SEL_075V,
    DVDD_VOL_SEL_0775V,
    DVDD_VOL_SEL_080V,
    DVDD_VOL_SEL_0825V,
    DVDD_VOL_SEL_0850V,
    DVDD_VOL_SEL_0875V,
    DVDD_VOL_SEL_090V,
    DVDD_VOL_SEL_0925V,
    DVDD_VOL_SEL_0950V,
    DVDD_VOL_SEL_0975V,
    DVDD_VOL_SEL_100V,
    DVDD_VOL_SEL_1025V,
    DVDD_VOL_SEL_105V,
    DVDD_VOL_SEL_1075V,
    DVDD_VOL_SEL_110V,
};

/************************P3_DVD_CON0*****************************/

#define DVDD_DEFAULT_VOL			DVDD_VOL_SEL_090V

#define DVDD_VOL_SEL(sel)     		P33_CON_SET(P3_DVD_CON0, 0, 4, sel)

#define GET_DVDD_VOL_SEL()     		(P33_CON_GET(P3_DVD_CON0) & 0xf)

#define DVDD_HD_SEL(sel)  			P33_CON_SET(P3_DVD_CON0, 4, 2, sel)

#define DVDD_CAP_EN(en)       		p33_fast_access(P3_DVD_CON0, BIT(6), en)

/*******************************************************************/
/*
 *-------------------P3_RVD_CON0
 */
enum {
    RVDD_VOL_SEL_0725V = 0,
    RVDD_VOL_SEL_075V,
    RVDD_VOL_SEL_0775V,
    RVDD_VOL_SEL_080V,
    RVDD_VOL_SEL_0825V,
    RVDD_VOL_SEL_0850V,
    RVDD_VOL_SEL_0875V,
    RVDD_VOL_SEL_090V,
    RVDD_VOL_SEL_0925V,
    RVDD_VOL_SEL_0950V,
    RVDD_VOL_SEL_0975V,
    RVDD_VOL_SEL_100V,
    RVDD_VOL_SEL_1025V,
    RVDD_VOL_SEL_105V,
    RVDD_VOL_SEL_1075V,
    RVDD_VOL_SEL_110V,
};


#define RVDD_VOL_SEL(sel)       	P33_CON_SET(P3_RVD_CON0, 0, 4, sel)

#define RVDD_DEFAULT_VOL			RVDD_VOL_SEL_090V

#define GET_RVDD_VOL_SEL()      	(P33_CON_GET(P3_RVD_CON0) & 0xf)

#define RVDD_HD_SEL(en)     		P33_CON_SET(P3_RVD_CON0, 4, 2, en)

/************************P3_RVD_CON1*****************************/
#define RVDD_CMP_EN(en)				p33_fast_access(P3_RVD_CON1, BIT(4), en)

#define PVDD_DCDC_LEV_SEL(sel)		P33_CON_SET(P3_RVD_CON1, 0, 4, sel)

#define GET_PVDD_DCDC_LEV_SEL()		(P33_CON_GET(P3_RVD_CON1) & 0xf)

/************************P3_WVD_CON0*****************************/
#define WVDD_LOAD_EN(en)        	p33_fast_access(P3_WVD_CON0, BIT(4), en)

#define WVDD_VOL_SEL(sel)       	P33_CON_SET(P3_WVD_CON0, 0, 4, sel)


/************************P3_PVD_CON0*****************************/
#define PVD_DEUDSHT_EN(en)      	p33_fast_access(P3_PVD_CON0, BIT(3), en)

#define GET_PVD_DEUDST_EN()			((P33_CON_GET(P3_PVD_CON0) & BIT(3)) ? 1:0)

#define PVDD_HD_SEL(sel)         	P33_CON_SET(P3_PVD_CON0, 0, 3, sel)

#define GET_PVDD_HD_SEL()			(P33_CON_GET(P3_PVD_CON0) & 0x7)

/************************P3_EVD_CON0*****************************/
enum {
    EVD_VOL_SEL_100V = 0,
    EVD_VOL_SEL_105V,
    EVD_VOL_SEL_110V,
    EVD_VOL_SEL_115V,
};

#define EVD_CAP_EN(en)          	p33_fast_access(P3_EVD_CON0, BIT(4), en)

#define EVD_HD_SEL(sel)         	P33_CON_SET(P3_EVD_CON0, 2, 2, sel)

#define EVD_VOL_SEL(sel)       		P33_CON_SET(P3_EVD_CON0, 0, 2, sel)

#define RVD_DEUDSHT_EN(en)			p33_fast_access(P3_ANA_MFIX, BIT(2), en)
#define DVD_DEUDSHT_EN(en)			p33_fast_access(P3_ANA_MFIX, BIT(1), en)
#define EVD_ILMT_EN(en)				p33_fast_access(P3_ANA_MFIX, BIT(0), en)

/************************P3_CHG_CON0*****************************/
#define LRC_Hz_DEFAULT    (200 * 1000L)

#define LRC_CON0_INIT                                     \
        /*                               */     (0 << 7) |\
        /*                               */     (0 << 6) |\
        /*RC32K_RPPS_S1_33v              */     (1 << 5) |\
        /*RC32K_RPPS_S0_33v              */     (1 << 4) |\
        /*                               */     (0 << 3) |\
        /*                               */     (0 << 2) |\
        /*RC32K_RN_TRIM_33v              */     (1 << 1) |\
        /*RC32K_EN_33v                   */     (1 << 0)

#define LRC_CON1_INIT                                     \
        /*                               */     (0 << 7) |\
        /*RC32K_CAP_S2_33v               */     (0 << 6) |\
        /*RC32K_CAP_S1_33v               */     (1 << 5) |\
        /*RC32K_CAP_S0_33v               */     (1 << 4) |\
        /*                        2bit   */     (0 << 2) |\
        /*RC32K_RNPS_S1_33v              */     (1 << 1) |\
        /*RC32K_RNPS_S0_33v              */     (0 << 0)
/************************P3_LRC_CON0*****************************/

#define LRC32K_RPPS_SEL(sel)     	P33_CON_SET(P3_LRC_CON0, 4, 2, sel)

#define LRC32K_RN_TRIM(en)       	p33_fast_access(P3_LRC_CON0, BIT(1), en)

#define LRC_EN(en)            		p33_fast_access(P3_LRC_CON0, BIT(0), en);\
                                    p33_fast_access(P3_LRC_CON0, BIT(1), en)

/************************P3_LRC_CON1*****************************/
#define LRC32K_CAP_SEL(sel)      	P33_CON_SET(P3_LRC_CON1, 4, 3, sel)

#define LRC32K_PNPS_SEL(sel)     	P33_CON_SET(P3_LRC_CON1, 0, 2, sel)

#define CLOSE_LRC()					p33_tx_1byte(P3_LRC_CON0, 0);\
									p33_tx_1byte(P3_LRC_CON1, 0)

/*******************************************************************/
/************************P3_VLVD_CON0*****************************/
#define VLVD_PND()          		((P33_CON_GET(P3_VLVD_CON0) & BIT(7)) ? 1 : 0)

#define VLVD_PND_CLR()       		p33_fast_access(P3_VLVD_CON0, BIT(6), 1)

#define P33_VLVD_PS(en)         	p33_fast_access(P3_VLVD_CON0, BIT(2), en)

#define P33_VLVD_OE(en)         	p33_fast_access(P3_VLVD_CON0, BIT(1), en)

#define P33_VLVD_EN(en)         	p33_fast_access(P3_VLVD_CON0, BIT(0), en)

#define GET_P33_VLVD_EN()			((P33_CON_GET(P3_VLVD_CON0) & BIT(0)) ? 1:0)

/************************P3_VLVD_FLT*****************************/
#define VLVD_FLT(sel)				P33_CON_SET(P3_VLVD_FLT, 0, 2, sel);

/************************P3_RST_CON0*****************************/
#define PVDDOK_OE(en) 				p33_fast_access(P3_RST_CON0, BIT(7), en)

#define DVDDOK2_OE(en) 				p33_fast_access(P3_RST_CON0, BIT(6), en)

#define DVDDOK_OE(en) 				p33_fast_access(P3_RST_CON0, BIT(5), en)

#define PPOR_MASK(en)           	p33_fast_access(P3_RST_CON0, BIT(4), en)

#define DPOR2_MASK(en)           	p33_fast_access(P3_RST_CON0, BIT(3), en)

#define DPOR_MASK(en)           	p33_fast_access(P3_RST_CON0, BIT(2), en)

#define P11_TO_P33_RST_MASK(en) 	p33_fast_access(P3_RST_CON0, BIT(1), en)

#define FAST_PU_SYS(en)           	p33_fast_access(P3_RST_CON0, BIT(0), en)

/************************P3_RST_CON1*****************************/
#define IS_VCM_DET_EN() 			((P33_CON_GET(P3_RST_CON1) & BIT(0)) ? 1: 0 )
#define DVD2_DRST_MASK(en)			p33_fast_access(P3_RST_CON1, BIT(5), en)
#define DVD_DRST_MASK(en)			p33_fast_access(P3_RST_CON1, BIT(4), en)
#define VLVD_WKUP_EN(en)        	p33_fast_access(P3_RST_CON1, BIT(3), en)
#define VLVD_RST_EN(en)         	p33_fast_access(P3_RST_CON1, BIT(2), en)
#define VLVD_EXPT_EN(en)         	p33_fast_access(P3_RST_CON1, BIT(1), en)

#define VCM_DET_EN(en)          	p33_fast_access(P3_RST_CON1, BIT(0), en)

/************************P3_CLK_CON0*****************************/
#define RC_250K_EN(a)          		p33_fast_access(P3_CLK_CON0, BIT(0), a)

/************************P3_VLD_KEEP*****************************/
#define RTC_WKUP_KEEP(a)        	p33_fast_access(P3_VLD_KEEP, BIT(1), a)

#define P33_WKUP_P11_EN(a)          p33_fast_access(P3_VLD_KEEP, BIT(2), a)


//
//
//					for pmu flow
//
//
//
/************************P3_P11_CPU*****************************/
#define P11_CPU_BRANCH_POWEROFF(en) p33_fast_access(P3_P11_CPU, BIT(1), en)

#define P11_CPU_RELEASE(en)			p33_fast_access(P3_P11_CPU, BIT(0), en)

/************************P3_LP_CTL*****************************/
//控制p11的低功耗
#define LP_FLOW_EN(en)				p33_fast_access(P3_LP_CTL, BIT(0), en)

#define LP_FLOW_CPND()				p33_fast_access(P3_LP_CTL, BIT(6), 1)

#define POWER_ON_END() 				((P33_CON_GET(P3_LP_CTL) & BIT(5)) ? 1: 0 )


#define MBG_EN_EN(a)       \
   if (a) {    \
		p33_or_1byte(P3_ANA_FLOW0, BIT(7)); \
	} else { \
		p33_and_1byte(P3_ANA_FLOW0, ~BIT(7)); \
}

/************************P3_ANA_FLOW0*****************************/
#define DVD_EN(en)    	            p33_fast_access(P3_ANA_FLOW0, BIT(0), en)


#define DCVD_TO_DIG_EN(en)			p33_fast_access(P3_ANA_FLOW0, BIT(0)|BIT(1), en)

#define DCVD_LDO_EN(en)            	p33_fast_access(P3_ANA_FLOW0, BIT(2), en)

#define PAVD_LDO_EN(en)       	    p33_fast_access(P3_ANA_FLOW0, BIT(3), en)

#define GET_DCVD_STA()				((P33_CON_GET(P3_ANA_FLOW0) & (BIT(2))) ? 1:0)

#define GET_PAVD_LDO_EN()			((P33_CON_GET(P3_ANA_FLOW0) & BIT(3)) ? 1:0)

#define PVDD_EN(en)             	p33_fast_access(P3_ANA_FLOW0, BIT(4), en)

#define MVIO_VBAT_EN(en)        	p33_fast_access(P3_ANA_FLOW0, BIT(5), en)

#define MVIO_VPWR_EN(en)        	p33_fast_access(P3_ANA_FLOW0, BIT(6), en)

#define PW_GATE_EN(en)				p33_fast_access(P3_ANA_FLOW0, BIT(5)|BIT(6), en)

#define MBG_EN(en)					p33_fast_access(P3_ANA_FLOW0, BIT(7), en)

#define MVIO_PVDD_MVBG_ONLY()  		p33_tx_1byte(P3_ANA_FLOW0, BIT(4) | BIT(5) | BIT(6) | BIT(7))

#define PVDD_MVBG_ONLY()  			p33_tx_1byte(P3_ANA_FLOW0, BIT(4) | BIT(7))

#define PVDD_ONLY()  				p33_tx_1byte(P3_ANA_FLOW0, BIT(4))

/************************P3_ANA_FLOW1*****************************/
#define MIOV_VLMT_EN(en)			p33_fast_access(P3_ANA_FLOW1, BIT(4), en)

#define MIOV_IFULL_EN(en)			p33_fast_access(P3_ANA_FLOW1, BIT(3), en)

#define PAVD_IFULL_EN(en)			p33_fast_access(P3_ANA_FLOW1, BIT(2), en)

#define DCVD_IFULL_EN(en)			p33_fast_access(P3_ANA_FLOW1, BIT(1), en)


#define DVD_IFULL_EN(en)			p33_fast_access(P3_ANA_FLOW1, BIT(0), en)

/************************P3_ANA_FLOW2*****************************/
#define NVD2PVD_WSHORT_EN(en)		p33_fast_access(P3_ANA_FLOW2, BIT(1), en)

#define NVD2PVD_SHORT_EN(en)		p33_fast_access(P3_ANA_FLOW2, BIT(0), en)


/************************P3_NVRAM_PWR*****************************/
#define NVRAM_PWR_MODE(sel)         P33_CON_SET(P3_NVRAM_PWR, 4, 2, sel)


/************************P3_PVDD0_AUTO*****************************/

#define PVDD_AUTO_PRD(sel)			P33_CON_SET(P3_PVDD0_AUTO, 5, 3, sel);

#define PVDD_LEVEL_AUTO(en)			p33_fast_access(P3_PVDD0_AUTO, BIT(4), en);

#define PVDD_LEVEL_LOW(sel)			P33_CON_SET(P3_PVDD0_AUTO, 0, 4, sel);


/************************P3_PVDD1_AUTO*****************************/
#define PVDD_LEVEL_HIGH_NOW(sel)	p33_tx_1byte(P3_PVDD1_AUTO, (sel<<4)|sel);

#define PVDD_LEVEL_HIGH(sel)		P33_CON_SET(P3_PVDD1_AUTO, 4, 4, sel)

#define PVDD_LEVEL_NOW(sel)			P33_CON_SET(P3_PVDD1_AUTO, 0, 4, sel)

#define GET_PVDD_LEVEL_NOW()		(P33_CON_GET(P3_PVDD1_AUTO) & 0x0f)


//
//
//			for ANA_control
//
//
/////
/*******************************************************************/
/************************P3_LS_XX*****************************/
enum PWR_LAT {
    DVDDLS_LAT,
    PVDDLS_LAT,
};

#define DVDDLS_OR_BIT(a)    \
    p33_or_1byte(P3_LS_IO_USR    , BIT(a));  \
    p33_or_1byte(P3_LS_IO_ROM    , BIT(a));

#define SHALS_OR_BIT(a)    \
    p33_or_1byte(P3_LS_IO_SHA    , BIT(a));  \
    p33_or_1byte(P3_LS_IO_PINR   , BIT(a));

#define PVDDLS_OR_BIT(a)    \
    p33_or_1byte(P3_LS_CTMU      , BIT(a));  \
    p33_or_1byte(P3_LS_P11       , BIT(a));

#define DVDDLS_AND_NBIT(a)    \
    p33_and_1byte(P3_LS_IO_USR    , (u8)~BIT(a));  \
    p33_and_1byte(P3_LS_IO_ROM    , (u8)~BIT(a));

#define SHALS_AND_NBIT(a)	\
    p33_and_1byte(P3_LS_IO_SHA    , (u8)~BIT(a));  \
    p33_and_1byte(P3_LS_IO_PINR   , (u8)~BIT(a));

#define PVDDLS_AND_NBIT(a)    \
    p33_and_1byte(P3_LS_CTMU      , (u8)~BIT(a));  \
    p33_and_1byte(P3_LS_P11       , (u8)~BIT(a));

#define DVDDLS_TX_BYTE(a)    \
    p33_tx_1byte(P3_LS_IO_USR    , a);  \
    p33_tx_1byte(P3_LS_IO_ROM    , a);

#define SHALS_TX_BYTE(a)    \
    p33_tx_1byte(P3_LS_IO_SHA    , a);  \
    p33_tx_1byte(P3_LS_IO_PINR   , a);

#define PVDDLS_TX_BYTE(a)    \
    p33_tx_1byte(P3_LS_CTMU      , a);  \
    p33_tx_1byte(P3_LS_P11       , a);

#define DVDDLS_ANA_NBIT(a)    \
    p33_and_1byte(P3_LS_IO_USR    , ~BIT(a));  \
    p33_and_1byte(P3_LS_IO_ROM    , ~BIT(a));  \
    p33_and_1byte(P3_LS_IO_SHA    , ~BIT(a));  \
    p33_and_1byte(P3_LS_CTMU      , ~BIT(a));  \

#define SHALS_ANA_NBIT(a)    \
    p33_and_1byte(P3_LS_IO_SHA    , (u8)~BIT(a));  \
    p33_and_1byte(P3_LS_CTMU      , (u8)~BIT(a));  \

#define PVDDLS_ANA_NBIT(a)    \
    p33_and_1byte(P3_LS_P11       , ~BIT(a));



//
//
//			for reset_source
//
//
//
/************************P3_PR_PWR*****************************/
#define	P3_SOFT_RESET()				P33_CON_SET(P3_PR_PWR, 4, 1, 1)

/************************P3_IVS_CLR*****************************/
#define PWVLD(a)       \
    if (a) {    \
		p33_tx_1byte(P3_IVS_SET, BIT(7)); \
	} else { \
		p33_tx_1byte(P3_IVS_CLR, BIT(7)); \
	}

#define	P33_SF_KICK_START()			P33_CON_SET(P3_IVS_CLR, 0, 8, 0b00101010)

#define PWR_BY_SOFTWARE(a)       \
    if (a) {    \
        p33_or_1byte(P3_RST_CON1, BIT(6)); \
        p33_tx_1byte(P3_ANA_KEEP0, 0xff); \
        p33_tx_1byte(P3_ANA_KEEP1, 0xff); \
        PWVLD(0); \
	} else { \
        PWVLD(1); \
        p33_and_1byte(P3_RST_CON1, ~BIT(6)); \
	}


/************************P3_RST_SRC*****************************/
#define GET_P33_SYS_RST_SRC()		P33_CON_GET(P3_RST_SRC)

/************************P3_RST_FLAG*****************************/
#define GET_P33_SYS_POWER_FLAG() 	((P33_CON_GET(P3_RST_FLAG) & BIT(3)) ? 1 : 0)

#define GET_P33_SYS_RST_LEVEL2()	((P33_CON_GET(P3_RST_FLAG) & BIT(3)) ? 1 : 0)
#define GET_P33_SYS_RST_LEVEL1()	((P33_CON_GET(P3_RST_FLAG) & BIT(5)) ? 1 : 0)
#define GET_P33_SYS_RST_LEVEL0()	((P33_CON_GET(P3_RST_FLAG) & BIT(7)) ? 1 : 0)

#define P33_LV2_RST_FLAG_CLR()			(p33_fast_access(P3_RST_FLAG, BIT(2), 1))
#define P33_LV1_RST_FLAG_CLR()			(p33_fast_access(P3_RST_FLAG, BIT(4), 1))
#define P33_LV0_RST_FLAG_CLR()			(p33_fast_access(P3_RST_FLAG, BIT(6), 1))


//-----special operation----------
#define P33_WKUP_ENABLE()     \
    p33_or_1byte(P3_VLD_KEEP , BIT(2))

#define P33_WKUP_DISABLE()     \
    p33_and_1byte(P3_VLD_KEEP , ~BIT(2))

#define P33_IE_ENABLE()     \
    bit_set_ie(IRQ_P33_IDX)

#define P33_IE_DISABLE()     \
    bit_clr_ie(IRQ_P33_IDX)

#define IS_CHARGE_EN_NOW()          (P3_CHG_CON0 & BIT(0))

/*
 *-------------------P3_ANA_FLOW0
 */


#define DCVDD_EN(a)       \
    if (a) {    \
		p33_or_1byte(P3_ANA_FLOW0, BIT(2)); \
	} else { \
        p33_and_1byte(P3_ANA_FLOW0, ~BIT(2)); \
}

#define DCDC13_EN(a)       \
    if (a) {    \
		p33_or_1byte(P3_ANA_FLOW0, BIT(3)); \
	} else { \
        p33_and_1byte(P3_ANA_FLOW0, ~BIT(3)); \
}

#define IS_DCDC13_EN() 			((P3_ANA_FLOW0 & BIT(3)) ? 1: 0 )




/*
 *-------------------P3_ANA_FLOW1
 */

#define IOVD_IFULL_EN(a)       \
    if (a) {    \
		p33_or_1byte(P3_ANA_FLOW1, (BIT(0) | BIT(1))); \
	} else { \
		p33_and_1byte(P3_ANA_FLOW1, ~(BIT(0) | BIT(1))); \
	}

#define IOVD_VLMT_EN(a)       \
    if (a) {    \
		p33_or_1byte(P3_ANA_FLOW1, (BIT(2) | BIT(3))); \
	} else { \
		p33_and_1byte(P3_ANA_FLOW1, ~(BIT(2) | BIT(3))); \
	}


#define DVD_POR_EN(a)       \
    if (a) {    \
		p33_or_1byte(P3_ANA_FLOW1, BIT(6)); \
	} else { \
		p33_and_1byte(P3_ANA_FLOW1, ~BIT(6)); \
	}

#define PVD_POR_EN(a)       \
    if (a) {    \
		p33_or_1byte(P3_ANA_FLOW1, BIT(7)); \
	} else { \
		p33_and_1byte(P3_ANA_FLOW1, ~BIT(7)); \
	}






/*******************************************************************/


/*******************************************************************/


/*
 *-------------------P3_VLVD_CON1
 */
#define VLVD_SEL(a)             P3_VLVD_CON1 = (P3_VLVD_CON1 & (~0xf)) | a
//macro for VLVD_SEL
enum {
    VLVD_sel_16V = 0,
    VLVD_sel_17V,
    VLVD_sel_18V,
    VLVD_sel_19V,
    VLVD_sel_20V,
    VLVD_sel_21V,
    VLVD_sel_22V,
    VLVD_sel_23V,
    VLVD_sel_24V,
    VLVD_sel_25V,
    VLVD_sel_26V,
    VLVD_sel_27V,
    VLVD_sel_28V,
    VLVD_sel_29V,
    VLVD_sel_30V,
    VLVD_sel_31V,
};

/*******************************************************************/


/*******************************************************************/
/*
 *-------------------P3_IOV_CON0
 */
//vddiom_lev
enum {
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

#define GET_MVIO_SEL()          (P3_IOV_CON0 & 0xf)

#define MVIO_SEL(a)             P3_IOV_CON0 = (P3_IOV_CON0 & (~0xf)) | a







/*
 *  *-------------------P3_RST_FLAG
 *   */
#define P33_SYS_POWERUP_CLEAR()     p33_or_1byte(P3_RST_FLAG, BIT(2)|BIT(4)|BIT(6))




//need 1mS to recorver
//P3_ANA_KEEP0 = 0; \
//P3_ANA_KEEP1 = 0;

//while(POWER_ON_END());
//P3_ANA_KEEP0 = 0; \
//P3_ANA_KEEP1 = 0;



#endif



