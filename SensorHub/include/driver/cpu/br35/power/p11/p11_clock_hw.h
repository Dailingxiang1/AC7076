/**@file  		p11_hw.h
* @brief        hw sfr layer
* @date     	2024-05-16
* @version    	V1.0
* @copyright  	Copyright(c)2010-2031  JIELI
* @details		该文件对p11 clock相关寄存器封装
 */
#ifndef __P11_CLOCK_HW_H__
#define __P11_CLOCK_HW_H__

enum P11_SYS_CLK_TABLE {
    P11_SYS_CLK_RC16M = 0,
    P11_SYS_CLK_RC250K,
    P11_SYS_CLK_LRC_OSC,
    P11_SYS_CLK_BTOSC_24M,
    P11_SYS_CLK_BTOSC_48M,
    P11_SYS_CLK_LRC24M,
    P11_SYS_CLK_CLK_X2,
    P11_SYS_CLK_TEST,
};

#define P11_SYS_CLK_SEL(x) 			(P11_CLOCK->CLK_CON0 = x)

//p11 btosc use d2sh
#define CLOCK_KEEP(en)							  \
	if(en){										  \
		P11_CLOCK->CLK_CON1 &= ~(3<<15);		  \
		P11_CLOCK->CLK_CON1 |= BIT(14);			  \
		P11_CLOCK->CLK_CON1 |= (2<<15);			  \
	}else{  									  \
		P11_CLOCK->CLK_CON1 &= ~(3<<15);		  \
		P11_CLOCK->CLK_CON1 &= ~BIT(14);		  \
	}

#define P2M_CLK_SET(x)              SFR(P11_SYSTEM->P2M_CLK_CON0,0,8,x)

#define P2M_CLK_GET(x)              (P11_SYSTEM->P2M_CLK_CON0 & 0xff)

#define RC_250k_EN(a)       \
    if (a) {    \
        P11_CLOCK->CLK_CON1 |= BIT(10); \
    } else { \
        P11_CLOCK->CLK_CON1 &= ~BIT(10); \
    }

#endif
