#ifndef __P33_SFR_H__
#define __P33_SFR_H__

//#ifdef PMU_SYSTEM
#if 1
#define P33_ACCESS(x) (*(volatile u32 *)(0xc000 + x*4))
#else
#define P33_ACCESS(x) (*(volatile u32 *)(0xf20000 + 0xc000 + x*4))
#endif

//#ifdef PMU_SYSTEM
#if 1
#define RTC_ACCESS(x) (*(volatile u32 *)(0xd000 + x*4))
#else
#define RTC_ACCESS(x) (*(volatile u32 *)(0xf20000 + 0xd000 + x*4))
#endif

//===========
//===============================================================================//
//
//
//
//===============================================================================//
//............. 0x0000 - 0x000f............
#define P3_VLMT_CON                   P33_ACCESS(0x01)
#define P3_POR_CON                    P33_ACCESS(0x02)
#define P3_VLVD_CON0                  P33_ACCESS(0x03)
#define P3_VLVD_CON1                  P33_ACCESS(0x04)
#define P3_VLVD_FLT                   P33_ACCESS(0x05)
#define P3_WDT_CON                    P33_ACCESS(0x06)
#define P3_OCP_CON0                   P33_ACCESS(0x07)

#define P3_ANA_FLOW0                  P33_ACCESS(0x08)
#define P3_ANA_FLOW1                  P33_ACCESS(0x09)
#define P3_ANA_FLOW2                  P33_ACCESS(0x0a)

#define P3_ANA_KEEP0                  P33_ACCESS(0x0c)
#define P3_ANA_KEEP1                  P33_ACCESS(0x0d)
#define P3_ANA_KEEP2                  P33_ACCESS(0x0e)

//............. 0X0010 - 0X001F.........for analog others
#define P3_OSL_CON                    P33_ACCESS(0x10)
#define P3_RST_FLAG                   P33_ACCESS(0x11)
#define P3_VBAT_TYPE                  P33_ACCESS(0x12)
#define P3_LRC_CON0                   P33_ACCESS(0x13)
#define P3_LRC_CON1                   P33_ACCESS(0x14)
#define P3_RST_CON0                   P33_ACCESS(0x15)
#define P3_RST_CON1                   P33_ACCESS(0x16)
#define P3_RST_CON2                   P33_ACCESS(0x17)
#define P3_VLD_KEEP                   P33_ACCESS(0x18)
#define P3_CLK_CON0                   P33_ACCESS(0x19)
#define P3_ANA_READ                   P33_ACCESS(0x1a)
#define P3_CHG_CON0                   P33_ACCESS(0x1b)
#define P3_CHG_CON1                   P33_ACCESS(0x1c)
#define P3_CHG_CON2                   P33_ACCESS(0x1d)
#define P3_CHG_CON3                   P33_ACCESS(0x1e)
#define P3_CHG_CON4                   P33_ACCESS(0x1f)

//............. 0X0020 - 0X002F............ for buck circuit
//#define P3_BUCK1_CON0                 P33_ACCESS(0x20)
//#define P3_BUCK1_CON1                 P33_ACCESS(0x21)
//#define P3_BUCK1_CON2                 P33_ACCESS(0x22)
//#define P3_BUCK1_CON3                 P33_ACCESS(0x23)
//#define P3_BUCK1_CON4                 P33_ACCESS(0x24)
//#define P3_BUCK1_CON5                 P33_ACCESS(0x25)
//#define P3_BUCK1_CON6                 P33_ACCESS(0x26)
//#define P3_BUCK1_CON7                 P33_ACCESS(0x27)
#define P3_BUCK2_CON0                 P33_ACCESS(0x20)
#define P3_BUCK2_CON1                 P33_ACCESS(0x21)
#define P3_BUCK2_CON2                 P33_ACCESS(0x22)
#define P3_BUCK2_CON3                 P33_ACCESS(0x23)
#define P3_BUCK2_CON4                 P33_ACCESS(0x24)
#define P3_BUCK2_CON5                 P33_ACCESS(0x25)
#define P3_BUCK2_CON6                 P33_ACCESS(0x26)
#define P3_BUCK2_CON7                 P33_ACCESS(0x27)
//#define P3_BUCK3_CON0                 P33_ACCESS(0x28)
//#define P3_BUCK3_CON1                 P33_ACCESS(0x29)
//#define P3_BUCK3_CON2                 P33_ACCESS(0x2a)
//#define P3_BUCK3_CON3                 P33_ACCESS(0x2b)
//#define P3_BUCK3_CON4                 P33_ACCESS(0x2c)
//#define P3_BUCK3_CON5                 P33_ACCESS(0x2d)
//#define P3_BUCK3_CON6                 P33_ACCESS(0x2e)
//#define P3_BUCK3_CON7                 P33_ACCESS(0x2f)

//............. 0X0030 - 0X003F............ for PMU manager
#define P3_SFLAG0                     P33_ACCESS(0x30)
#define P3_SFLAG1                     P33_ACCESS(0x31)
#define P3_SFLAG2                     P33_ACCESS(0x32)
#define P3_SFLAG3                     P33_ACCESS(0x33)
#define P3_SFLAG4                     P33_ACCESS(0x34)
#define P3_SFLAG5                     P33_ACCESS(0x35)
#define P3_SFLAG6                     P33_ACCESS(0x36)
#define P3_SFLAG7                     P33_ACCESS(0x37)
#define P3_SFLAG8                     P33_ACCESS(0x38)
#define P3_SFLAG9                     P33_ACCESS(0x39)
#define P3_SFLAGA                     P33_ACCESS(0x3a)
#define P3_SFLAGB                     P33_ACCESS(0x3b)

//............. 0X0040 - 0X004F............ for
#define P3_IVS_RD                     P33_ACCESS(0x40)
#define P3_IVS_SET                    P33_ACCESS(0x41)
#define P3_IVS_CLR                    P33_ACCESS(0x42)
#define P3_PVDD0_AUTO                 P33_ACCESS(0x43)
#define P3_PVDD1_AUTO                 P33_ACCESS(0x44)
#define P3_WKUP_DLY                   P33_ACCESS(0x45)

#define P3_PCNT_FLT                   P33_ACCESS(0x48)
#define P3_PCNT_CON                   P33_ACCESS(0x49)
#define P3_PCNT_SET0                  P33_ACCESS(0x4a)
#define P3_PCNT_SET1                  P33_ACCESS(0x4b)
#define P3_PCNT_DAT0                  P33_ACCESS(0x4c)
#define P3_PCNT_DAT1                  P33_ACCESS(0x4d)

#define P3_P11_CPU                    P33_ACCESS(0x4f)

//............. 0X0050 - 0X005F............ for port wake up
#define P3_WKUP_FLT_EN0               P33_ACCESS(0x50)
#define P3_WKUP_P_IE0                 P33_ACCESS(0x51)
#define P3_WKUP_N_IE0                 P33_ACCESS(0x52)
#define P3_WKUP_LEVEL0                P33_ACCESS(0x53)
#define P3_WKUP_P_CPND0               P33_ACCESS(0x54)
#define P3_WKUP_N_CPND0               P33_ACCESS(0x55)
#define P3_WKUP_P_PND0                P33_ACCESS(0x56)
#define P3_WKUP_N_PND0                P33_ACCESS(0x57)
#define P3_WKUP_FLT_EN1               P33_ACCESS(0x58)
#define P3_WKUP_P_IE1                 P33_ACCESS(0x59)
#define P3_WKUP_N_IE1                 P33_ACCESS(0x5a)
#define P3_WKUP_LEVEL1                P33_ACCESS(0x5b)
#define P3_WKUP_P_CPND1               P33_ACCESS(0x5c)
#define P3_WKUP_N_CPND1               P33_ACCESS(0x5d)
#define P3_WKUP_P_PND1                P33_ACCESS(0x5e)
#define P3_WKUP_N_PND1                P33_ACCESS(0x5f)

//............. 0X0060 - 0X006F............ for analog wake up
#define P3_AWKUP_FLT_EN               P33_ACCESS(0x60)
#define P3_AWKUP_P_IE                 P33_ACCESS(0x61)
#define P3_AWKUP_N_IE                 P33_ACCESS(0x62)
#define P3_AWKUP_LEVEL                P33_ACCESS(0x63)
#define P3_AWKUP_P_PND                P33_ACCESS(0x64)
#define P3_AWKUP_N_PND                P33_ACCESS(0x65)
#define P3_AWKUP_P_CPND               P33_ACCESS(0x66)
#define P3_AWKUP_N_CPND               P33_ACCESS(0x67)
#define P3_WKUP_CLK_SEL               P33_ACCESS(0x68)
#define P3_AWKUP_CLK_SEL              P33_ACCESS(0x69)
#define P3_SYS_PWR0                   P33_ACCESS(0x6a)
#define P3_SYS_PWR1                   P33_ACCESS(0x6b)
#define P3_SYS_PWR2                   P33_ACCESS(0x6c)
#define P3_SYS_PWR3                   P33_ACCESS(0x6d)
#define P3_SYS_PWR4                   P33_ACCESS(0x6e)
#define P3_SYS_PWR5                   P33_ACCESS(0x6f)

//............. 0X0070 - 0X007F............ for
#define P3_PGDR_CON0                  P33_ACCESS(0x70)
#define P3_PGDR_CON1                  P33_ACCESS(0x71)
#define P3_PGSD_CON                   P33_ACCESS(0x72)

#define P3_LP_CTL                     P33_ACCESS(0x74)
#define P3_LP_CFG                     P33_ACCESS(0x75)
#define P3_NVRAM_PWR                  P33_ACCESS(0x76)
#define P3_WVD_CON0                   P33_ACCESS(0x77)
#define P3_PVD_CON0                   P33_ACCESS(0x78)
#define P3_EVD_CON0                   P33_ACCESS(0x79)
#define P3_PMU_CON0                   P33_ACCESS(0x7a)

#define P3_PMU_CON4                   P33_ACCESS(0x7e)
#define P3_PMU_CON5                   P33_ACCESS(0x7f)

//............. 0X0080 - 0X008F............ for
#define P3_PINR_CON                   P33_ACCESS(0x80)
#define P3_PINR_CON1                  P33_ACCESS(0x81)
#define P3_PINR_SAFE                  P33_ACCESS(0x82)
#define P3_PINR_SAFE1                 P33_ACCESS(0x83)
#define P3_PINR_PND1                  P33_ACCESS(0x84)

#define P3_RST_SRC0                   P33_ACCESS(0x8e)
#define P3_RST_SRC1                   P33_ACCESS(0x8f)

//............. 0X0090 - 0X009F............ for
#define P3_PSW_CON0                   P33_ACCESS(0x90)
#define P3_PSW_CON1                   P33_ACCESS(0x91)
#define P3_PSW_CON2                   P33_ACCESS(0x92)
#define P3_PMU_ADC0                   P33_ACCESS(0x93)
#define P3_PMU_ADC1                   P33_ACCESS(0x94)
#define P3_VBG_CON0                   P33_ACCESS(0x95)
#define P3_VBG_CON1                   P33_ACCESS(0x96)
#define P3_IOV_CON0                   P33_ACCESS(0x97)
#define P3_IOV_CON1                   P33_ACCESS(0x98)
#define P3_PAVD_CON0                  P33_ACCESS(0x99)
#define P3_DCV_CON0                   P33_ACCESS(0x9a)
#define P3_DVD_CON0                   P33_ACCESS(0x9b)
#define P3_DVD2_CON0                  P33_ACCESS(0x9c)
#define P3_RVD_CON0                   P33_ACCESS(0x9d)
#define P3_RVD_CON1                   P33_ACCESS(0x9e)
#define P3_RVD2_CON0                  P33_ACCESS(0x9f)

//............. 0X00A0 - 0X00AF............
#define P3_PR_PWR                     P33_ACCESS(0xa0)
#define P3_VPWR_CON0                  P33_ACCESS(0xa1)
#define P3_VPWR_CON1                  P33_ACCESS(0xa2)
#define P3_RTC_ADC0                   P33_ACCESS(0xa3)
#define P3_LS_P11                     P33_ACCESS(0xa4)
#define P3_LS_EN                      P33_ACCESS(0xa5)

#define P3_EXT_EFUSE_CON              P33_ACCESS(0xa6)

#define P3_WKUP_SRC                   P33_ACCESS(0xa8)
#define P3_ANA_MFIX                   P33_ACCESS(0xa9)
#define P3_DBG_CON0                   P33_ACCESS(0xaa)
#define P3_DBG_CON1                   P33_ACCESS(0xab)
#define P3_MFIX_OPT                   P33_ACCESS(0xac)

//............. 0X00B0 - 0X00BF............ for EFUSE
#define P3_EFUSE_CON0                 P33_ACCESS(0xb0)
#define P3_EFUSE_CON1                 P33_ACCESS(0xb1)
#define P3_EFUSE_CON2                 P33_ACCESS(0xb2)
#define P3_EFUSE_RDAT                 P33_ACCESS(0xb3)
#define P3_EFUSE_PU_DAT0              P33_ACCESS(0xb4)
#define P3_EFUSE_PU_DAT1              P33_ACCESS(0xb5)
#define P3_EFUSE_PU_DAT2              P33_ACCESS(0xb6)
#define P3_EFUSE_PU_DAT3              P33_ACCESS(0xb7)

#define P3_FUNC_EN                    P33_ACCESS(0xb8)
#define P3_FUNC_CTL0                  P33_ACCESS(0xb9)
#define P3_FUNC_CTL1                  P33_ACCESS(0xba)
#define P3_FUNC_CTL2                  P33_ACCESS(0xbb)
#define P3_EFUSE_ANA0                 P33_ACCESS(0xbc)

//............. 0X00C0 - 0X00CF............ for port input select
#define P3_PORT_SEL0                  P33_ACCESS(0xc0)
#define P3_PORT_SEL1                  P33_ACCESS(0xc1)
#define P3_PORT_SEL2                  P33_ACCESS(0xc2)
#define P3_PORT_SEL3                  P33_ACCESS(0xc3)
#define P3_PORT_SEL4                  P33_ACCESS(0xc4)
#define P3_PORT_SEL5                  P33_ACCESS(0xc5)
#define P3_PORT_SEL6                  P33_ACCESS(0xc6)
#define P3_PORT_SEL7                  P33_ACCESS(0xc7)
#define P3_PORT_SEL8                  P33_ACCESS(0xc8)

//............. 0x00d0 - 0x00df............
#define P3_LS_IO_USR                  P33_ACCESS(0xd0)    //TODO: check sync with verilog head file chip_def.v  LEVEL_SHIFTER
#define P3_LS_IO_ROM                  P33_ACCESS(0xd1)
#define P3_LS_IO_PINR                 P33_ACCESS(0xd2)
#define P3_LS_CTMU                    P33_ACCESS(0xd3)
#define P3_LS_IO_SHA                  P33_ACCESS(0xd4)
#define P3_LS_LRC24M                  P33_ACCESS(0xd5)
#define P3_LS_BT                      P33_ACCESS(0xd6)
#define P3_LS_PLL                     P33_ACCESS(0xd7)

//............. 0X00E0 - 0X00FF............     for p33 lp timer
#define P3_LP_RSC00                   P33_ACCESS(0xe0)
#define P3_LP_RSC01                   P33_ACCESS(0xe1)
#define P3_LP_RSC02                   P33_ACCESS(0xe2)
#define P3_LP_RSC03                   P33_ACCESS(0xe3)
#define P3_LP_PRD00                   P33_ACCESS(0xe4)
#define P3_LP_PRD01                   P33_ACCESS(0xe5)
#define P3_LP_PRD02                   P33_ACCESS(0xe6)
#define P3_LP_PRD03                   P33_ACCESS(0xe7)
#define P3_LP_RSC10                   P33_ACCESS(0xe8)
#define P3_LP_RSC11                   P33_ACCESS(0xe9)
#define P3_LP_RSC12                   P33_ACCESS(0xea)
#define P3_LP_RSC13                   P33_ACCESS(0xeb)
#define P3_LP_RSC14                   P33_ACCESS(0xec)
#define P3_LP_RSC15                   P33_ACCESS(0xed)
#define P3_LP_PRD10                   P33_ACCESS(0xee)
#define P3_LP_PRD11                   P33_ACCESS(0xef)
#define P3_LP_PRD12                   P33_ACCESS(0xf0)
#define P3_LP_PRD13                   P33_ACCESS(0xf1)
#define P3_LP_PRD14                   P33_ACCESS(0xf2)
#define P3_LP_PRD15                   P33_ACCESS(0xf3)
#define P3_LP_TMR0_CLK                P33_ACCESS(0xf4)
#define P3_LP_TMR1_CLK                P33_ACCESS(0xf5)
#define P3_LP_TMR0_CON                P33_ACCESS(0xf6)
#define P3_LP_TMR1_CON                P33_ACCESS(0xf7)
#define P3_LP_TMR_CFG                 P33_ACCESS(0xf8)
#define P3_LP_CNTRD0                  P33_ACCESS(0xf9)
#define P3_LP_CNT0                    P33_ACCESS(0xfa)
#define P3_LP_CNT1                    P33_ACCESS(0xfb)
#define P3_LP_CNT2                    P33_ACCESS(0xfc)
#define P3_LP_CNT3                    P33_ACCESS(0xfd)
#define P3_LP_CNT4                    P33_ACCESS(0xfe)
#define P3_LP_CNT5                    P33_ACCESS(0xff)



//===============================================================================//
//
//      P33 RTCVDD
//
//===============================================================================//

//............. 0X0080 - 0X008F............ for RTC
#define R3_ALM_CON                    RTC_ACCESS(0x80)

#define R3_RTC_CON0                   RTC_ACCESS(0x84)
#define R3_RTC_CON1                   RTC_ACCESS(0x85)
#define R3_RTC_DAT0                   RTC_ACCESS(0x86)
#define R3_RTC_DAT1                   RTC_ACCESS(0x87)
#define R3_RTC_DAT2                   RTC_ACCESS(0x88)
#define R3_RTC_DAT3                   RTC_ACCESS(0x89)
#define R3_RTC_DAT4                   RTC_ACCESS(0x8a)
#define R3_ALM_DAT0                   RTC_ACCESS(0x8b)
#define R3_ALM_DAT1                   RTC_ACCESS(0x8c)
#define R3_ALM_DAT2                   RTC_ACCESS(0x8d)
#define R3_ALM_DAT3                   RTC_ACCESS(0x8e)
#define R3_ALM_DAT4                   RTC_ACCESS(0x8f)

//............. 0X0090 - 0X009F............ for wake up
#define R3_WKUP_EN                    RTC_ACCESS(0x90)
#define R3_WKUP_EDGE                  RTC_ACCESS(0x91)
#define R3_WKUP_CPND                  RTC_ACCESS(0x92)
#define R3_WKUP_PND                   RTC_ACCESS(0x93)
#define R3_WKUP_LEVEL                 RTC_ACCESS(0x94)

//............. 0X00A0 - 0X00AF............ for system
#define R3_TIME_CON                   RTC_ACCESS(0xa0)
#define R3_TIME_CPND                  RTC_ACCESS(0xa1)
#define R3_TIME_PND                   RTC_ACCESS(0xa2)

#define R3_ADC_CON                    RTC_ACCESS(0xa4)
#define R3_OSL_CON                    RTC_ACCESS(0xa5)

#define R3_WKUP_SRC                   RTC_ACCESS(0xa8)
#define R3_RST_SRC                    RTC_ACCESS(0xa9)

#define R3_RST_CON                    RTC_ACCESS(0xab)
#define R3_CLK_CON                    RTC_ACCESS(0xac)

//............. 0X00B0 - 0X00BF............ for PORT control
#define R3_PR_IN                      RTC_ACCESS(0xb0)
#define R3_PR_OUT                     RTC_ACCESS(0xb1)
#define R3_PR_DIR                     RTC_ACCESS(0xb2)
#define R3_PR_DIE                     RTC_ACCESS(0xb3)
#define R3_PR_PU0                     RTC_ACCESS(0xb4)
#define R3_PR_PU1                     RTC_ACCESS(0xb5)
#define R3_PR_PD0                     RTC_ACCESS(0xb6)
#define R3_PR_PD1                     RTC_ACCESS(0xb7)
#define R3_PR_HD0                     RTC_ACCESS(0xb8)
#define R3_PR_HD1                     RTC_ACCESS(0xb9)

#endif
