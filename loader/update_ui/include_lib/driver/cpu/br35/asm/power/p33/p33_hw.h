/**@file  		p33_app.h
* @brief        hw sfr layer
* @details
* @author		app / ic
* @date     	2021-10-13
* @version    	V1.0
* @copyright  	Copyright(c)2010-2031  JIELI
 */
#ifndef __P33_HW_H__
#define __P33_HW_H__

//
//
//					for p33_analog
//
//
//
/************************P3_PSW_CON0*****************************/
#define DVD2SVD_SHORT_EN(en)      	p33_fast_access(P3_PSW_CON0, BIT(6), en)

//#define SVD2RVD2_SHORT_EN(en)      	p33_fast_access(P3_PSW_CON0, BIT(5), en)
//
//#define SVD2RVD_SHORT_EN(en)      	p33_fast_access(P3_PSW_CON0, BIT(4), en)
//#define RVDD2_CAP_EN(en)      	    p33_fast_access(P3_PSW_CON0, BIT(3), en)
//#define RVD2_EN(en)    	            p33_fast_access(P3_PSW_CON0, BIT(2), en)
//
//#define RVDD_CAP_EN(en)      	    p33_fast_access(P3_PSW_CON0, BIT(1), en)
//
//#define RVD_EN(en)    	            p33_fast_access(P3_PSW_CON0, BIT(0), en)

#define NVD2IO_SHORT_EN(en)		    p33_fast_access(P3_PSW_CON1, BIT(7), en)

//#define DVDD2_IFULL_EN(en)      	p33_fast_access(P3_PSW_CON1, BIT(6), en)
//#define DVDD2_BYPASS_EN(en)      	p33_fast_access(P3_PSW_CON1, BIT(5), en)
//#define DVDD2_EN(en)				p33_fast_access(P3_PSW_CON1, BIT(4), en)
//
//#define RVDD2_BYPASS_EN(en)      	p33_fast_access(P3_PSW_CON1, BIT(3), en)
//
//#define RVDD_BYPASS_EN(en)      	p33_fast_access(P3_PSW_CON1, BIT(2), en)
#define WVD2SVD_SHORT_EN(en)     	p33_fast_access(P3_PSW_CON1, BIT(1), en)
#define WVDD_EN(en)     			p33_fast_access(P3_PSW_CON1, BIT(0), en)
#define WVDD_PDOWN_ENTER()			P33_CON_SET(P3_PSW_CON1, 0, 2, 0x03); P33_CON_SET(P3_PSW_CON0, 4, 3, 0x7)

#define WVDD_POFF_ENTER()			P33_CON_SET(P3_PSW_CON1, 0, 2, 0x03); P33_CON_SET(P3_PSW_CON0, 4, 3, 0x1)

#define WVDD_PDOWN_POFF_EXIT() 		P33_CON_SET(P3_PSW_CON0, 4, 3, 0x07); P33_CON_SET(P3_PSW_CON1, 1, 1, 0x00)

/************************P3_PSW_CON2*****************************/
//SS: soft start of VPQS
//#define VQPS_SS_EN(en)      		p33_fast_access(P3_PSW_CON2, BIT(1), en)
//
//#define VQPS_EN(en)					p33_fast_access(P3_PSW_CON2, BIT(0), en)

/************************P3_ANA_CON*****************************/
//#define EVDD_IFULL_EN(en)      		p33_fast_access(P3_ANA_CON, BIT(0), en)

//DS: decrease undershoot and overshoot
//#define DVD_DS_EN(en)				p33_fast_access(P3_ANA_CON, BIT(1), en)

//#define RVD_DS_EN(en)      			p33_fast_access(P3_ANA_CON, BIT(2), en)

/************************P3_VBG_CON0*****************************/
#define MVBG_SEL(sel)				P33_CON_SET(P3_VBG_CON0, 0, 4, sel)

#define WVBG_SEL(sel)				P33_CON_SET(P3_VBG_CON1, 4, 4, sel)

/************************P3_CLK_CON0*****************************/
#define LRC24M_GATE_EN(a)          	p33_fast_access(P3_CLK_CON0, BIT(6), a)
#define WDT_CLK_SEL(sel)          	P33_CON_SET(P3_CLK_CON0, 5, 1, sel)
#define SOFF_RC250K_GATE(en)        p33_fast_access(P3_CLK_CON0, BIT(4), en)
#define D2SH_EN(a)          		p33_fast_access(P3_CLK_CON0, BIT(3), a)
#define RCLK_SEL(sel)          		P33_CON_SET(P3_CLK_CON0, 2, 1, sel)
#define RC_200K_SW_EN(a)          	p33_fast_access(P3_CLK_CON0, BIT(1), a)
#define RC_250K_SW_EN(a)          	p33_fast_access(P3_CLK_CON0, BIT(0), a)

/************************P3_VLD_KEEP*****************************/
#define RTC_WKUP_KEEP(a)        	p33_fast_access(P3_VLD_KEEP, BIT(1), a)

#define P33_WKUP_P11_EN(a)          p33_fast_access(P3_VLD_KEEP, BIT(2), a)

#define VLD_KEEP_WDT_EXPT(en)		p33_fast_access(P3_VLD_KEEP, BIT(6), en)

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

/************************P3_ANA_FLOW0*****************************/
#define DVD_EN(en)    	            p33_fast_access(P3_ANA_FLOW0, BIT(0), en)

#define DCVD_TO_DIG_EN(en)			p33_fast_access(P3_ANA_FLOW0, BIT(0)|BIT(1), en)

#define DCVD_LDO_EN(en)            	p33_fast_access(P3_ANA_FLOW0, BIT(2), en)

#define PAVD_LDO_EN(en)       	    p33_fast_access(P3_ANA_FLOW0, BIT(3), en)

#define GET_DCDC12_STA()			((P33_CON_GET(P3_BUCK2_CON1) & (BIT(0))) ? 1:0)
#define GET_LDO12_STA()				((P33_CON_GET(P3_ANA_FLOW0) & (BIT(2))) ? 1:0)
#define GET_DCVD_STA()				(GET_DCDC12_STA()|GET_LDO12_STA())

#define GET_PAVD_LDO_EN()			((P33_CON_GET(P3_ANA_FLOW0) & BIT(3)) ? 1:0)

#define PVDD_EN(en)             	p33_fast_access(P3_ANA_FLOW0, BIT(4), en)

#define MVIO_EN(en)        	        p33_fast_access(P3_ANA_FLOW0, BIT(5), en)

#define VIN_EN(en)        	        p33_fast_access(P3_ANA_FLOW0, BIT(6), en)

#define PW_GATE_EN(en)				p33_fast_access(P3_ANA_FLOW0, BIT(5)|BIT(6), en)

#define MBG_EN(en)					p33_fast_access(P3_ANA_FLOW0, BIT(7), en)

#define MVIO_PVDD_MVBG_ONLY()  		p33_tx_1byte(P3_ANA_FLOW0, BIT(4) | BIT(5) | BIT(6) | BIT(7))

#define PVDD_MVBG_ONLY()  			p33_tx_1byte(P3_ANA_FLOW0, BIT(4) | BIT(7))

#define PVDD_ONLY()  				p33_tx_1byte(P3_ANA_FLOW0, BIT(4))

/************************P3_ANA_FLOW1*****************************/
#define WVIO_SHORT_EN(en)			p33_fast_access(P3_ANA_FLOW1, BIT(5), en)

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

#endif
