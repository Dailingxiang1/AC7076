#ifndef __LP_IPC__
#define __LP_IPC__

//===========================================================================//
//                              P2M MESSAGE TABLE                            //
//===========================================================================//
#define P11_RAM_ACCESS(x)   							(*(volatile u8 *)(x))
#define M2P_MESSAGE_ACCESS(x)      						P11_RAM_ACCESS(M2P_MESSAGE_RAM_BEGIN + x)
#define P2M_MESSAGE_ACCESS(x)      						P11_RAM_ACCESS(P2M_MESSAGE_RAM_BEGIN + x)

//==================power=============================
#define P2M_WKUP_SRC                                    P2M_MESSAGE_ACCESS(0)
#define P2M_WKUP_P_PND                                  P2M_MESSAGE_ACCESS(1)
#define P2M_WKUP_N_PND                                  P2M_MESSAGE_ACCESS(2)
#define P2M_AWKUP_P_PND                                 P2M_MESSAGE_ACCESS(3)
#define P2M_AWKUP_N_PND                                 P2M_MESSAGE_ACCESS(4)
#define P2M_WKUP_RTC                                    P2M_MESSAGE_ACCESS(5)
#define P2M_WKUP_CNT0									P2M_MESSAGE_ACCESS(6)
#define P2M_WKUP_CNT1									P2M_MESSAGE_ACCESS(7)
#define P2M_WKUP_CNT2									P2M_MESSAGE_ACCESS(8)
#define P2M_WKUP_CNT3									P2M_MESSAGE_ACCESS(9)
#define P2M_OSC_CNT0									P2M_MESSAGE_ACCESS(10)
#define P2M_OSC_CNT1									P2M_MESSAGE_ACCESS(11)
#define P2M_OSC_CNT2									P2M_MESSAGE_ACCESS(12)
#define P2M_OSC_CNT3									P2M_MESSAGE_ACCESS(13)

//==================system===========================
#define P2M_MESSAGE_BANK_ADR_L                          P2M_MESSAGE_ACCESS(15)
#define P2M_MESSAGE_BANK_ADR_H                          P2M_MESSAGE_ACCESS(16)
#define P2M_MESSAGE_BANK_INDEX                          P2M_MESSAGE_ACCESS(17)
#define P2M_MESSAGE_BANK_ACK                            P2M_MESSAGE_ACCESS(18)
#define P2M_P11_HEAP_BEGIN_ADDR_L    					P2M_MESSAGE_ACCESS(19)
#define P2M_P11_HEAP_BEGIN_ADDR_H    					P2M_MESSAGE_ACCESS(20)
#define P2M_P11_HEAP_SIZE_L    							P2M_MESSAGE_ACCESS(21)
#define P2M_P11_HEAP_SIZE_H    							P2M_MESSAGE_ACCESS(22)
#define P2M_REPLY_SYNC_CMD                            	P2M_MESSAGE_ACCESS(23)
#define P2M_CBUF_ADDR0									P2M_MESSAGE_ACCESS(24)
#define P2M_CBUF_ADDR1                                  P2M_MESSAGE_ACCESS(25)
#define P2M_CBUF_ADDR2                                  P2M_MESSAGE_ACCESS(26)
#define P2M_CBUF_ADDR3                                  P2M_MESSAGE_ACCESS(27)
#define P2M_CBUF1_ADDR0                                 P2M_MESSAGE_ACCESS(28)
#define P2M_CBUF1_ADDR1                                 P2M_MESSAGE_ACCESS(29)
#define P2M_CBUF1_ADDR2                                 P2M_MESSAGE_ACCESS(30)
#define P2M_CBUF1_ADDR3                                 P2M_MESSAGE_ACCESS(31)

//==================clock===========================
#define P2M_BTOSC_OK									P2M_MESSAGE_ACCESS(35)

//==================lpctmu===========================
#define P2M_CTMU_CMD_ACK                                P2M_MESSAGE_ACCESS(39)
#define P2M_MASSAGE_CTMU_CH0_L_RES                                         40
#define P2M_MASSAGE_CTMU_CH0_H_RES                                         41
#define P2M_CTMU_CH0_L_RES                              P2M_MESSAGE_ACCESS(40)
#define P2M_CTMU_CH0_H_RES                              P2M_MESSAGE_ACCESS(41)
#define P2M_CTMU_CH1_L_RES                              P2M_MESSAGE_ACCESS(42)
#define P2M_CTMU_CH1_H_RES                              P2M_MESSAGE_ACCESS(43)
#define P2M_CTMU_CH2_L_RES                              P2M_MESSAGE_ACCESS(44)
#define P2M_CTMU_CH2_H_RES                              P2M_MESSAGE_ACCESS(45)
#define P2M_CTMU_CH3_L_RES                              P2M_MESSAGE_ACCESS(46)
#define P2M_CTMU_CH3_H_RES                              P2M_MESSAGE_ACCESS(47)
#define P2M_CTMU_CH4_L_RES                              P2M_MESSAGE_ACCESS(48)
#define P2M_CTMU_CH4_H_RES                              P2M_MESSAGE_ACCESS(49)



//===========================================================================//
//                              M2P MESSAGE TABLE                            //
//===========================================================================//
//==================power=============================
#define M2P_LRC_PRD                                     M2P_MESSAGE_ACCESS(0)
#define M2P_WDVDD                                		M2P_MESSAGE_ACCESS(1)
#define M2P_LRC_FEQ0									M2P_MESSAGE_ACCESS(2)
#define M2P_LRC_FEQ1									M2P_MESSAGE_ACCESS(3)
#define M2P_LRC_FEQ2									M2P_MESSAGE_ACCESS(4)
#define M2P_LRC_FEQ3									M2P_MESSAGE_ACCESS(5)
#define M2P_VDDIO_KEEP                                  M2P_MESSAGE_ACCESS(6)
#define M2P_LRC_KEEP                                    M2P_MESSAGE_ACCESS(7)
#define M2P_RCH_FEQ_L                                   M2P_MESSAGE_ACCESS(8)
#define M2P_RCH_FEQ_H                                   M2P_MESSAGE_ACCESS(9)
#define M2P_MEM_CONTROL                                 M2P_MESSAGE_ACCESS(10)
#define M2P_BTOSC_KEEP                                  M2P_MESSAGE_ACCESS(11)
#define M2P_CTMU_KEEP									M2P_MESSAGE_ACCESS(12)
#define M2P_RTC_KEEP                                    M2P_MESSAGE_ACCESS(13)
#define M2P_SF_MODE										M2P_MESSAGE_ACCESS(14)
#define M2P_DCV_MODE									M2P_MESSAGE_ACCESS(15)
#define M2P_LIGHT_PDOWN_DVDD_VOL						M2P_MESSAGE_ACCESS(16)
#define M2P_LRC24M_MODE									M2P_MESSAGE_ACCESS(17)

//==================system===========================
#define M2P_SYNC_CMD                                  	M2P_MESSAGE_ACCESS(25)
#define M2P_WDT_SYNC                                   	M2P_MESSAGE_ACCESS(26)
#define M2P_WAIT_RELEASE								M2P_MESSAGE_ACCESS(27)

//==================clock===========================
#define M2P_LRC24M_CFG0                                 M2P_MESSAGE_ACCESS(35)
#define M2P_LRC24M_CFG1                                 M2P_MESSAGE_ACCESS(36)
#define M2P_BTOSC_CFG0                                  M2P_MESSAGE_ACCESS(37)
#define M2P_BTOSC_CFG1                                  M2P_MESSAGE_ACCESS(38)

#define M2P_LRC24M_FEQ0                                 M2P_MESSAGE_ACCESS(39)
#define M2P_LRC24M_FEQ1                                 M2P_MESSAGE_ACCESS(40)
#define M2P_LRC24M_FEQ2                                 M2P_MESSAGE_ACCESS(41)
#define M2P_LRC24M_FEQ3                                 M2P_MESSAGE_ACCESS(42)


//==================lpctmu===========================
#define M2P_CTMU_CMD  									M2P_MESSAGE_ACCESS(50)
#define M2P_CTMU_CH_ENABLE								M2P_MESSAGE_ACCESS(51)
#define M2P_CTMU_CH_WAKEUP_EN					        M2P_MESSAGE_ACCESS(52)
#define M2P_CTMU_SCAN_TIME          					M2P_MESSAGE_ACCESS(53)
#define M2P_CTMU_LOWPOER_SCAN_TIME     					M2P_MESSAGE_ACCESS(54)


/*
 *  Must Sync to P11 code
 */
enum {
    M2P_LP_INDEX    = 0,
    M2P_PF_INDEX,
    M2P_LLP_INDEX,
    M2P_P33_INDEX,
    M2P_SF_INDEX,
    M2P_CTMU_INDEX,
    M2P_CCMD_INDEX,       //common cmd
    M2P_VAD_INDEX,
    M2P_USER_INDEX,
    M2P_WDT_INDEX,
    M2P_SYNC_INDEX,
    M2P_APP_INDEX,
    M2P_WKUP_INDEX,
};

enum {
    P2M_LP_INDEX    = 0,
    P2M_PF_INDEX,
    P2M_LLP_INDEX,
    P2M_WK_INDEX,
    P2M_WDT_INDEX,
    P2M_LP_INDEX2,
    P2M_CTMU_INDEX,
    P2M_CTMU_POWUP,
    P2M_REPLY_CCMD_INDEX,  //reply common cmd
    P2M_VAD_INDEX,
    P2M_USER_INDEX,
    P2M_BANK_INDEX,
    P2M_REPLY_SYNC_INDEX,
    P2M_APP_INDEX,
    P2M_OSC_INDEX,
};

enum {
    CLOSE_P33_INTERRUPT = 1,
    OPEN_P33_INTERRUPT,
    LOWPOWER_PREPARE,
    M2P_SPIN_LOCK,
    M2P_SPIN_UNLOCK,
    P2M_SPIN_LOCK,
    P2M_SPIN_UNLOCK,

};

#include "lp_msg.h"

#endif
