#ifndef  __GPIO_HW_H__
#define  __GPIO_HW_H__

#include "typedef.h"
#include "asm/power_interface.h"


#define IO_GROUP_NUM 		16
#define IO_PORTA_00 				(IO_GROUP_NUM * 0 + 0)
#define IO_PORTA_01 				(IO_GROUP_NUM * 0 + 1)
#define IO_PORTA_02 				(IO_GROUP_NUM * 0 + 2)
#define IO_PORTA_03 				(IO_GROUP_NUM * 0 + 3)
#define IO_PORTA_04 				(IO_GROUP_NUM * 0 + 4)
#define IO_PORTA_05 				(IO_GROUP_NUM * 0 + 5)
#define IO_PORTA_06 				(IO_GROUP_NUM * 0 + 6)
#define IO_PORTA_07 				(IO_GROUP_NUM * 0 + 7)
#define IO_PORTA_08 				(IO_GROUP_NUM * 0 + 8)
#define IO_PORTA_09 				(IO_GROUP_NUM * 0 + 9)
#define IO_PORTA_10 				(IO_GROUP_NUM * 0 + 10)
#define IO_PORTA_11 				(IO_GROUP_NUM * 0 + 11)
#define IO_PORTA_12 				(IO_GROUP_NUM * 0 + 12)
#define IO_PORTA_13 				(IO_GROUP_NUM * 0 + 13)
#define IO_PORT_PA_MASK              0x3fff

#define IO_PORTB_00 				(IO_GROUP_NUM * 1 + 0)
#define IO_PORTB_01 				(IO_GROUP_NUM * 1 + 1)
#define IO_PORTB_02 				(IO_GROUP_NUM * 1 + 2)
#define IO_PORTB_03 				(IO_GROUP_NUM * 1 + 3)
#define IO_PORTB_04 				(IO_GROUP_NUM * 1 + 4)
#define IO_PORTB_05 				(IO_GROUP_NUM * 1 + 5)
#define IO_PORTB_06 				(IO_GROUP_NUM * 1 + 6)
#define IO_PORTB_07 				(IO_GROUP_NUM * 1 + 7)
#define IO_PORTB_08 				(IO_GROUP_NUM * 1 + 8)
#define IO_PORT_PB_MASK              0x01ff

#define IO_PORTC_00 				(IO_GROUP_NUM * 2 + 0)
#define IO_PORTC_01 				(IO_GROUP_NUM * 2 + 1)
#define IO_PORTC_02 				(IO_GROUP_NUM * 2 + 2)
#define IO_PORTC_03 				(IO_GROUP_NUM * 2 + 3)
#define IO_PORTC_04 				(IO_GROUP_NUM * 2 + 4)
#define IO_PORTC_05 				(IO_GROUP_NUM * 2 + 5)
#define IO_PORTC_06 				(IO_GROUP_NUM * 2 + 6)
#define IO_PORTC_07 				(IO_GROUP_NUM * 2 + 7)
#define IO_PORTC_08 				(IO_GROUP_NUM * 2 + 8)
#define IO_PORTC_09 				(IO_GROUP_NUM * 2 + 9)
#define IO_PORTC_10 				(IO_GROUP_NUM * 2 + 10)
#define IO_PORTC_11 				(IO_GROUP_NUM * 2 + 11)
#define IO_PORT_PC_MASK              0x0fff

#define IO_PORTD_00 				(IO_GROUP_NUM * 3 + 0)
#define IO_PORTD_01 				(IO_GROUP_NUM * 3 + 1)
#define IO_PORTD_02 				(IO_GROUP_NUM * 3 + 2)
#define IO_PORTD_03 				(IO_GROUP_NUM * 3 + 3)
// #define IO_PORTD_04 				(IO_GROUP_NUM * 3 + 4)
// #define IO_PORTD_05 				(IO_GROUP_NUM * 3 + 5)
// #define IO_PORTD_06 				(IO_GROUP_NUM * 3 + 6)
// #define IO_PORTD_07 				(IO_GROUP_NUM * 3 + 7)
// #define IO_PORTD_08 				(IO_GROUP_NUM * 3 + 8)
#define IO_PORTD_09 				(IO_GROUP_NUM * 3 + 9)
#define IO_PORT_PD_MASK              0x0000

#define IO_PORTF_00 				(IO_GROUP_NUM * 5 + 0)
#define IO_PORTF_01 				(IO_GROUP_NUM * 5 + 1)
#define IO_PORTF_02 				(IO_GROUP_NUM * 5 + 2)
#define IO_PORTF_03 				(IO_GROUP_NUM * 5 + 3)
#define IO_PORTF_04 				(IO_GROUP_NUM * 5 + 4)
#define IO_PORTF_05 				(IO_GROUP_NUM * 5 + 5)
#define IO_PORT_PF_MASK              0x003f

#define IO_PORTP_00 				(IO_GROUP_NUM * 13 + 0)
#define IO_PORT_PP_MASK              0x0001

#define IO_PORT_LDOIN   IO_PORTP_00

#define IO_MAX_NUM 					(IO_PORTP_00 + 1)

#define IO_PORT_DP                  (IO_GROUP_NUM * 14 + 0)
#define IO_PORT_DM                  (IO_GROUP_NUM * 14 + 1)
#define IO_PORT_USB_MASK            0x03
#define IS_PORT_USB(x)              (x <= IO_PORT_DM)//无usb赋0

//br35无pr
// #define IO_PORT_PR_00               (IO_GROUP_NUM * 15 + 0)//pr固定15
// #define IO_PORT_PR_01               (IO_GROUP_NUM * 15 + 1)
// #define IO_PORT_PR_MASK              0x03

#define IO_PORT_MAX					(IO_PORT_DM + 1)

#define P33_IO_OFFSET               0
#define IO_CHGFL_DET                (IO_PORT_MAX + P33_IO_OFFSET + 0)
#define IO_VBGOK_DET                (IO_PORT_MAX + P33_IO_OFFSET + 1)
#define IO_VBTCH_DET                (IO_PORT_MAX + P33_IO_OFFSET + 2)
#define IO_LDOIN_DET                (IO_PORT_MAX + P33_IO_OFFSET + 3)
#define IO_VBATDT_DET               (IO_PORT_MAX + P33_IO_OFFSET + 4)

#define PG_IO_OFFSET                5
#define IO_LCD_PG                   (IO_PORT_MAX + PG_IO_OFFSET + 0)
#define IO_MT_PG                    (IO_PORT_MAX + PG_IO_OFFSET + 1)
#define IO_FS_PG2                    (IO_PORT_MAX + PG_IO_OFFSET + 2)



#define GPIOA                       (IO_GROUP_NUM * 0)
#define GPIOB                       (IO_GROUP_NUM * 1)
#define GPIOC                       (IO_GROUP_NUM * 2)
// #define GPIOD                       (IO_GROUP_NUM * 3)//br35无PDx_OUT/IN
// #define GPIOE                       (IO_GROUP_NUM * 4)//无
#define GPIOF                       (IO_GROUP_NUM * 5)
#define GPIOP                       (IO_GROUP_NUM * 13)
#define GPIOUSB                     (IO_GROUP_NUM * 14)
// #define GPIOR                       (IO_GROUP_NUM * 15) //br35 no pr
#define GPIOP33                     (IO_PORT_MAX + P33_IO_OFFSET)

enum gpio_port {
    PORTA = 0,
    PORTB = 1,
    PORTC = 2,
    PORTF = 5,
    PORTP = 13,
    PORTUSB = 14,
    // PORTR = 15,  //br35 无pr
};
#define IS_PORT_ALL_PERIPH(PORT) (((PORT) == PORTA) || \
                                  ((PORT) == PORTB) || \
                                  ((PORT) == PORTC) || \
                                  ((PORT) == PORTF) || \
                                  ((PORT) == PORTP) || \
                                  ((PORT) == PORTUSB))

enum port_op_mode {
    PORT_SET = 1,
    PORT_AND,
    PORT_OR,
    PORT_XOR,
};

struct port_reg {
    volatile unsigned int in;
    volatile unsigned int out;
    volatile unsigned int dir;
    volatile unsigned int die;
    volatile unsigned int dieh;
    volatile unsigned int pu0;
    volatile unsigned int pu1;
    volatile unsigned int pd0;
    volatile unsigned int pd1;
    volatile unsigned int hd0;
    volatile unsigned int hd1;
    volatile unsigned int spl;
    volatile unsigned int con;
    volatile unsigned int out_bsr;
    volatile unsigned int dir_bsr;
    volatile unsigned int die_bsr;
    volatile unsigned int dieh_bsr;
    volatile unsigned int pu0_bsr;
    volatile unsigned int pu1_bsr;
    volatile unsigned int pd0_bsr;
    volatile unsigned int pd1_bsr;
    volatile unsigned int hd0_bsr;
    volatile unsigned int hd1_bsr;
    volatile unsigned int spl_bsr;
    volatile unsigned int con_bsr;
};
#define GPIO_PX_PU_REG_NUM 2
#define GPIO_PX_PD_REG_NUM 2
#define GPIO_PX_HD_REG_NUM 2
#define GPIO_PX_DIEH_REG_NUM 1
#define GPIO_PX_SPL_REG_NUM  1
#define GPIO_PX_BSR_REG_NUM  1
#define usb_reg port_reg
#define GPIO_USB_PU_REG_NUM 1
#define GPIO_USB_PD_REG_NUM 1
#define GPIO_USB_HD_REG_NUM 0
#define GPIO_USB_DIEH_REG_NUM 1
#define GPIO_USB_SPL_REG_NUM  1
#define GPIO_USB_BSR_REG_NUM  1
//无PR
// struct port_pr_reg {
//     volatile unsigned int in;
//     volatile unsigned int out;
//     volatile unsigned int dir;
//     volatile unsigned int die;
//     volatile unsigned int pu0;
//     // volatile unsigned int pu1;
//     volatile unsigned int pd0;
//     // volatile unsigned int pd1;
//     volatile unsigned int hd0;
//     // volatile unsigned int hd1;
// };
// #define GPIO_PR_PU_REG_NUM 0
// #define GPIO_PR_PD_REG_NUM 0
// #define GPIO_PR_HD_REG_NUM 0
// #define GPIO_PR_DIEH_REG_NUM 0
// #define GPIO_PR_SPL_REG_NUM  0

#define GPIO_PU_REG_NUM 2 //max_num
#define GPIO_PD_REG_NUM 2 //max_num
#define GPIO_HD_REG_NUM 2 //max_num


//===================================================//
// BR35 Crossbar API
//===================================================//
enum PFI_TABLE {
    PFI_GP_ICH0 = ((u32)(&(JL_IMAP->FI_GP_ICH0))),
    PFI_GP_ICH1 = ((u32)(&(JL_IMAP->FI_GP_ICH1))),
    PFI_GP_ICH2 = ((u32)(&(JL_IMAP->FI_GP_ICH2))),
    PFI_GP_ICH3 = ((u32)(&(JL_IMAP->FI_GP_ICH3))),
    PFI_GP_ICH4 = ((u32)(&(JL_IMAP->FI_GP_ICH4))),
    PFI_GP_ICH5 = ((u32)(&(JL_IMAP->FI_GP_ICH5))),

    // PFI_SPI0_CLK = ((u32)(&(JL_IMAP->FI_SPI0_CLK))),
    // PFI_SPI0_DA0 = ((u32)(&(JL_IMAP->FI_SPI0_DA0))),
    // PFI_SPI0_DA1 = ((u32)(&(JL_IMAP->FI_SPI0_DA1))),
    // PFI_SPI0_DA2 = ((u32)(&(JL_IMAP->FI_SPI0_DA2))),
    // PFI_SPI0_DA3 = ((u32)(&(JL_IMAP->FI_SPI0_DA3))),
    PFI_SPI1_CLK = ((u32)(&(JL_IMAP->FI_SPI1_CLK))),
    PFI_SPI1_DA0 = ((u32)(&(JL_IMAP->FI_SPI1_DA0))),
    PFI_SPI1_DA1 = ((u32)(&(JL_IMAP->FI_SPI1_DA1))),
    PFI_SPI1_DA2 = ((u32)(&(JL_IMAP->FI_SPI1_DA2))),
    PFI_SPI1_DA3 = ((u32)(&(JL_IMAP->FI_SPI1_DA3))),
    PFI_SPI2_CLK = ((u32)(&(JL_IMAP->FI_SPI2_CLK))),
    PFI_SPI2_DA0 = ((u32)(&(JL_IMAP->FI_SPI2_DA0))),
    PFI_SPI2_DA1 = ((u32)(&(JL_IMAP->FI_SPI2_DA1))),
    PFI_SPI2_DA2 = ((u32)(&(JL_IMAP->FI_SPI2_DA2))),
    PFI_SPI2_DA3 = ((u32)(&(JL_IMAP->FI_SPI2_DA3))),

    PFI_SD0_CMD = ((u32)(&(JL_IMAP->FI_SD0_CMD))),
    PFI_SD0_DA0 = ((u32)(&(JL_IMAP->FI_SD0_DA0))),
    PFI_SD0_DA1 = ((u32)(&(JL_IMAP->FI_SD0_DA1))),
    PFI_SD0_DA2 = ((u32)(&(JL_IMAP->FI_SD0_DA2))),
    PFI_SD0_DA3 = ((u32)(&(JL_IMAP->FI_SD0_DA3))),

    PFI_IIC0_SCL = ((u32)(&(JL_IMAP->FI_IIC0_SCL))),
    PFI_IIC0_SDA = ((u32)(&(JL_IMAP->FI_IIC0_SDA))),
    PFI_UART0_RX = ((u32)(&(JL_IMAP->FI_UART0_RX))),
    PFI_UART1_RX = ((u32)(&(JL_IMAP->FI_UART1_RX))),
    // PFI_UART1_CTS = ((u32)(&(JL_IMAP->FI_UART1_CTS))),
    PFI_UART2_RX = ((u32)(&(JL_IMAP->FI_UART2_RX))),

    // PFI_TDM_S_WCK  = ((u32)(&(JL_IMAP->FI_TDM_S_WCK))),
    // PFI_TDM_S_BCK  = ((u32)(&(JL_IMAP->FI_TDM_S_BCK))),
    // PFI_TDM_M_DA  = ((u32)(&(JL_IMAP->FI_TDM_M_DA))),
    // PFI_RDEC0_DAT0 = ((u32)(&(JL_IMAP->FI_RDEC0_DAT0))),
    // PFI_RDEC0_DAT1 = ((u32)(&(JL_IMAP->FI_RDEC0_DAT1))),
    // PFI_RDEC1_DAT0 = ((u32)(&(JL_IMAP->FI_RDEC1_DAT0))),
    // PFI_RDEC1_DAT1 = ((u32)(&(JL_IMAP->FI_RDEC1_DAT1))),
    // PFI_RDEC2_DAT0 = ((u32)(&(JL_IMAP->FI_RDEC2_DAT0))),
    // PFI_RDEC2_DAT1 = ((u32)(&(JL_IMAP->FI_RDEC2_DAT1))),
    // PFI_ALNK0_MCLK = ((u32)(&(JL_IMAP->FI_ALNK0_MCLK))),
    // PFI_ALNK0_LRCK = ((u32)(&(JL_IMAP->FI_ALNK0_LRCK))),
    // PFI_ALNK0_SCLK = ((u32)(&(JL_IMAP->FI_ALNK0_SCLK))),
    // PFI_ALNK0_DAT0 = ((u32)(&(JL_IMAP->FI_ALNK0_DAT0))),
    // PFI_ALNK0_DAT1 = ((u32)(&(JL_IMAP->FI_ALNK0_DAT1))),
    // PFI_ALNK0_DAT2 = ((u32)(&(JL_IMAP->FI_ALNK0_DAT2))),
    // PFI_ALNK0_DAT3 = ((u32)(&(JL_IMAP->FI_ALNK0_DAT3))),
    // PFI_PLNK_DAT0 = ((u32)(&(JL_IMAP->FI_PLNK_DAT0))),
    // PFI_PLNK_DAT1 = ((u32)(&(JL_IMAP->FI_PLNK_DAT1))),
    // PFI_SPDIF_DIA = ((u32)(&(JL_IMAP->FI_SPDIF_DIA))),
    // PFI_SPDIF_DIB = ((u32)(&(JL_IMAP->FI_SPDIF_DIB))),
    // PFI_SPDIF_DIC = ((u32)(&(JL_IMAP->FI_SPDIF_DIC))),
    // PFI_SPDIF_DID = ((u32)(&(JL_IMAP->FI_SPDIF_DID))),
    // PFI_CAN_RX = ((u32)(&(JL_IMAP->FI_CAN_RX))),
    PFI_QDEC0_A = ((u32)(&(JL_IMAP->FI_QDEC0_A))),
    PFI_QDEC0_B = ((u32)(&(JL_IMAP->FI_QDEC0_B))),
    PFI_CHAIN_IN0 = ((u32)(&(JL_IMAP->FI_CHAIN_IN0))),
    PFI_CHAIN_IN1 = ((u32)(&(JL_IMAP->FI_CHAIN_IN1))),
    PFI_CHAIN_IN2 = ((u32)(&(JL_IMAP->FI_CHAIN_IN2))),
    PFI_CHAIN_IN3 = ((u32)(&(JL_IMAP->FI_CHAIN_IN3))),
    PFI_CHAIN_RST = ((u32)(&(JL_IMAP->FI_CHAIN_RST))),
    PFI_TOTAl = ((u32)(&(JL_IMAP->FI_TOTAL))),
};

#define  INPUT_GP_ICH_MAX  6
#define  OUTPUT_GP_OCH_MAX 8

enum OUTPUT_CH_SIGNAL {
    OUTPUT_CH_SIGNAL_TIMER0_PWM,//8
    OUTPUT_CH_SIGNAL_TIMER1_PWM,
    OUTPUT_CH_SIGNAL_TIMER2_PWM,
    OUTPUT_CH_SIGNAL_TIMER3_PWM,
    // OUTPUT_CH_SIGNAL_TIMER4_PWM,
    // OUTPUT_CH_SIGNAL_TIMER5_PWM,
    OUTPUT_CH_SIGNAL_GP_ICH0,
    OUTPUT_CH_SIGNAL_GP_ICH1,
    OUTPUT_CH_SIGNAL_UART1_RTS,
    OUTPUT_CH_SIGNAL_PLNK_CLK,
    OUTPUT_CH_SIGNAL_WL_AMPE,
    OUTPUT_CH_SIGNAL_WL_LNAE,
    OUTPUT_CH_SIGNAL_WLC_INT_ACTIVE,
    OUTPUT_CH_SIGNAL_WLC_INT_STATUS,
    OUTPUT_CH_SIGNAL_WLC_INT_FREQ,
    OUTPUT_CH_SIGNAL_AUD_DBG_CLKO,
    OUTPUT_CH_SIGNAL_AUD_DBG_DATO0,
    OUTPUT_CH_SIGNAL_AUD_DBG_DATO1,
    OUTPUT_CH_SIGNAL_AUD_DBG_DATO2,
    OUTPUT_CH_SIGNAL_AUD_DBG_DATO3,
    OUTPUT_CH_SIGNAL_AUD_DBG_DATO4,
    OUTPUT_CH_SIGNAL_CLOCK_OUT0,
    OUTPUT_CH_SIGNAL_CLOCK_OUT1,
    OUTPUT_CH_SIGNAL_CLOCK_OUT2,
    OUTPUT_CH_SIGNAL_P33_CLK_DBG,
    OUTPUT_CH_SIGNAL_P33_SIG_DBG0,
    OUTPUT_CH_SIGNAL_P33_SIG_DBG1,
    OUTPUT_CH_SIGNAL_USB_DBG_OUT,
    OUTPUT_CH_SIGNAL_P11_DBG_OUT,
    OUTPUT_CH_SIGNAL_WL_DBG_PORTx,//fix wl0~7对应och0~7

    // OUTPUT_CH_SIGNAL_MCPWM0_H,
    // OUTPUT_CH_SIGNAL_MCPWM0_L,
    // OUTPUT_CH_SIGNAL_MCPWM1_H,
    // OUTPUT_CH_SIGNAL_MCPWM1_L,
    // OUTPUT_CH_SIGNAL_LEDC0_OUT,
    // OUTPUT_CH_SIGNAL_LEDC1_OUT,
};

enum INPUT_CH_TYPE {
    INPUT_CH_TYPE_GP_ICH = 0,
    INPUT_CH_TYPE_TIME2_PWM = 6,
    INPUT_CH_TYPE_TIME3_PWM,
    INPUT_CH_TYPE_WL_AMPE,
    INPUT_CH_TYPE_WL_LNAE,
    INPUT_CH_TYPE_MAX,
};

enum INPUT_CH_SIGNAL {
    //ICH_CON0
    INPUT_CH_SIGNAL_TIMER0_CIN = 0,//5
    INPUT_CH_SIGNAL_TIMER1_CIN,
    INPUT_CH_SIGNAL_TIMER2_CIN,
    INPUT_CH_SIGNAL_TIMER3_CIN,
    // INPUT_CH_SIGNAL_TIMER4_CIN,
    // INPUT_CH_SIGNAL_TIMER5_CIN,
    INPUT_CH_SIGNAL_TIMER0_CAPTURE,
    INPUT_CH_SIGNAL_TIMER1_CAPTURE,
    //ICH_CON1
    INPUT_CH_SIGNAL_TIMER2_CAPTURE,
    INPUT_CH_SIGNAL_TIMER3_CAPTURE,
    // INPUT_CH_SIGNAL_TIMER4_CAPTURE,
    // INPUT_CH_SIGNAL_TIMER5_CAPTURE,
    INPUT_CH_SIGNAL_MCPWM0_CK,
    INPUT_CH_SIGNAL_MCPWM1_CK,
    INPUT_CH_SIGNAL_MCPWM0_FP,
    INPUT_CH_SIGNAL_MCPWM1_FP,
    //ICH_CON2
    INPUT_CH_SIGNAL_UART1_CTS,
    INPUT_CH_SIGNAL_PLNK_IDAT0,
    INPUT_CH_SIGNAL_PLNK_IDAT1,
    INPUT_CH_SIGNAL_CAP,//CAP_MUX_OUT
    INPUT_CH_SIGNAL_CLK_PIN,  //CLK_MUX_IN
    INPUT_CH_SIGNAL_EXT_CLK,  //EXT_CLK_P
    // INPUT_CH_SIGNAL_IRFLT,
    //ICH_CON3
    INPUT_CH_SIGNAL_IMD_TE,
    INPUT_CH_SIGNAL_WLC_EXT_ACT,
    INPUT_CH_SIGNAL_AUD_DBG_DATI,
    INPUT_CH_SIGNAL_SPI1_CS,
    INPUT_CH_SIGNAL_SPI2_CS,
    INPUT_CH_SIGNAL_RESERVE0,
    //ICH_CON4
    // INPUT_CH_SIGNAL_QDEC_SIN0,
    // INPUT_CH_SIGNAL_QDEC_SIN1,
};
enum gpio_function {
    PORT_FUNC_NULL,    //null
//uart
    PORT_FUNC_UART0_TX, //out
    PORT_FUNC_UART0_RX,//in
    PORT_FUNC_UART1_TX, //out
    PORT_FUNC_UART1_RX,//in
    PORT_FUNC_UART2_TX, //out
    PORT_FUNC_UART2_RX,//in
    PORT_FUNC_UART1_RTS,//out
    PORT_FUNC_UART1_CTS,//in

//spi
    // PORT_FUNC_SPI0_CLK,
    // PORT_FUNC_SPI0_DA0,
    // PORT_FUNC_SPI0_DA1,
    // PORT_FUNC_SPI0_DA2,
    // PORT_FUNC_SPI0_DA3,
    PORT_FUNC_SPI1_CS,//ich slave
    PORT_FUNC_SPI1_CLK,
    PORT_FUNC_SPI1_DA0,
    PORT_FUNC_SPI1_DA1,
    PORT_FUNC_SPI1_DA2,
    PORT_FUNC_SPI1_DA3,
    PORT_FUNC_SPI2_CS,//ich slave
    PORT_FUNC_SPI2_CLK,
    PORT_FUNC_SPI2_DA0,
    PORT_FUNC_SPI2_DA1,
    PORT_FUNC_SPI2_DA2,
    PORT_FUNC_SPI2_DA3,

//iic
    PORT_FUNC_IIC_SCL,
    PORT_FUNC_IIC_SDA,

//sd
    PORT_FUNC_SD0_CLK,//out
    PORT_FUNC_SD0_CMD,
    PORT_FUNC_SD0_DA0,
    PORT_FUNC_SD0_DA1,
    PORT_FUNC_SD0_DA2,
    PORT_FUNC_SD0_DA3,

//timer
    PORT_FUNC_TIMER0_PWM,
    PORT_FUNC_TIMER1_PWM,
    PORT_FUNC_TIMER2_PWM,
    PORT_FUNC_TIMER3_PWM,
    // PORT_FUNC_TIMER4_PWM,
    // PORT_FUNC_TIMER5_PWM,
    PORT_FUNC_TIMER0_CAPTURE,
    PORT_FUNC_TIMER1_CAPTURE,
    PORT_FUNC_TIMER2_CAPTURE,
    PORT_FUNC_TIMER3_CAPTURE,
    // PORT_FUNC_TIMER4_CAPTURE,
    // PORT_FUNC_TIMER5_CAPTURE,
    PORT_FUNC_TIMER0_CIN,
    PORT_FUNC_TIMER1_CIN,
    PORT_FUNC_TIMER2_CIN,
    PORT_FUNC_TIMER3_CIN,

//mcpwm
    PORT_FUNC_MCPWM0_H,
    PORT_FUNC_MCPWM0_L,
    PORT_FUNC_MCPWM1_H,
    PORT_FUNC_MCPWM1_L,
    PORT_FUNC_MCPWM0_FP,
    PORT_FUNC_MCPWM1_FP,
    PORT_FUNC_MCPWM0_CK,
    PORT_FUNC_MCPWM1_CK,

//clk_out
    PORT_FUNC_OCH_CLOCK_OUT0,
    PORT_FUNC_OCH_CLOCK_OUT1,//PORT_FUNC_OCH_RESERVED0,//不连续
    PORT_FUNC_OCH_CLOCK_OUT2,
    // PORT_FUNC_OCH_CLOCK_OUT3,

//other
    PORT_FUNC_IRFLT_0, //实际只有1个IRFLT
    PORT_FUNC_IRFLT_1,
    PORT_FUNC_IRFLT_2,
    PORT_FUNC_IRFLT_3,

    PORT_FUNC_CLK_PIN,//CLK_MUX_IN
    // PORT_FUNC_PORT_WKUP,
    PORT_FUNC_GPADC,    //in
    PORT_FUNC_PWM_LED,

//plnk
    PORT_FUNC_PLNK_SCLK,//out
    PORT_FUNC_PLNK_DAT0,//in
    PORT_FUNC_PLNK_DAT1,//in

//ledc
    // PORT_FUNC_LEDC0_OUT,
    // PORT_FUNC_LEDC1_OUT,

//rdec
    // PORT_FUNC_RDEC0_PORT0,
    // PORT_FUNC_RDEC0_PORT1,

//qdec
    PORT_FUNC_RDEC0_PORTA,
    PORT_FUNC_RDEC0_PORTB,
//chain
};
/**************************************************/
#define __struct(x) (struct x##_reg *)
#define _struct(x) __struct(x)
#ifdef GPIOA
#define __PORTPA ((struct port_reg *)JL_PORTA)
#endif
#ifdef GPIOB
#define __PORTPB ((struct port_reg *)JL_PORTB)
#endif
#ifdef GPIOC
#define __PORTPC ((struct port_reg *)JL_PORTC)
#endif
#ifdef GPIOD
#define __PORTPD ((struct port_reg *)JL_PORTD)
#endif
#ifdef GPIOE
#define __PORTPE ((struct port_reg *)JL_PORTE)
#endif
#ifdef GPIOF
#define __PORTPF ((struct port_reg *)JL_PORTF)
#endif
#ifdef GPIOG
#define __PORTPG ((struct port_reg *)JL_PORTG)
#endif
#ifdef GPIOH
#define __PORTPH ((struct port_reg *)JL_PORTH)
#endif
#ifdef GPIOP
#define __PORTPP ((struct port_reg *)JL_PORTP)
#endif
#ifdef GPIOR
#define __PORTPR ((struct port_pr_reg *)R3_PR_IO_P)
#endif
#ifdef GPIOUSB
#define __PORTPU ((struct usb_reg *)JL_PORTUSB)
#endif
#define __portx(x,y) __PORT##x->y
#define _portx(x,y)  __portx(x,y)
#define __toggle_port(x,y) __PORT##x->out ^= y;
#define _toggle_port(port,pin) __toggle_port(port,pin)

//log:
#define GPIO_LOG_FORMAT "0x%04x  0x%04x  0x%04x  0x%04x  0x%04x,0x%04x  0x%04x,0x%04x  0x%04x,0x%04x  0x%04x"
#define GPIO_NO_SUPPORT_FUN "------"
#define GPIO_LOG_PORT(x,y) JL_PORT##x->OUT&y,JL_PORT##x->DIR&y,JL_PORT##x->DIE&y,JL_PORT##x->DIEH&y,JL_PORT##x->PU0&y,JL_PORT##x->PU1&y,JL_PORT##x->PD0&y,JL_PORT##x->PD1&y,JL_PORT##x->HD0&y,JL_PORT##x->HD1&y,JL_PORT##x->SPL&y
#ifdef GPIOP  //no use
#define GPIO_LOG_PORTP JL_PORTP->OUT,JL_PORTP->DIR,JL_PORTP->DIE,JL_PORTP->DIEH,JL_PORTP->PU0,JL_PORTP->PU1,JL_PORTP->PD0,JL_PORTP->PD1,JL_PORTP->HD0,JL_PORTP->HD1
#endif
#ifdef GPIOR
#define GPIO_LOG_FORMAT_R "0x%04x  0x%04x  0x%04x  %s  0x%04x,0x%04x  0x%04x,0x%04x  0x%04x,0x%04x  %s"
#define GPIO_LOG_PORTR R3_PR_OUT,R3_PR_DIR,R3_PR_DIE,GPIO_NO_SUPPORT_FUN,R3_PR_PU0,R3_PR_PU1,R3_PR_PD0,R3_PR_PD1,R3_PR_HD0,R3_PR_HD1,GPIO_NO_SUPPORT_FUN
#endif
#ifdef GPIOUSB
#define GPIO_LOG_FORMAT_U "0x%04x  0x%04x  0x%04x  0x%04x  0x%04x,%s  0x%04x,%s  %s,%s  0x%04x"
#define GPIO_LOG_PORTU _portx(PU,out),_portx(PU,dir),_portx(PU,die),_portx(PU,dieh),_portx(PU,pu0),GPIO_NO_SUPPORT_FUN,_portx(PU,pd0),GPIO_NO_SUPPORT_FUN,GPIO_NO_SUPPORT_FUN,GPIO_NO_SUPPORT_FUN,_portx(PU,spl)
#endif
/*************************function*************************/
struct port_reg *gpio2reg(u32 gpio);
void usb_iomode(const u32 enable);

int gpio_hw_write(const u32 gpio, const u32 value);//return <0:error
int gpio_hw_read(const u32 gpio);//return <0:error
int get_gpio(const char *p);//return <0:error

/**************************************************************/
/*********************multi pin interface***************************/
//多io同一模式
int gpio_hw_port_pin_judge(const enum gpio_port port, u32 pin);
int gpio_hw_set_direction(const enum gpio_port port, u32 pin, const u32 value);
int gpio_hw_direction_input(const enum gpio_port port, u32 pin);
int gpio_hw_direction_output(const enum gpio_port port, u32 pin, const int value);/////////
int gpio_hw_write_port(const enum gpio_port port, u32 pin, const u32 value);
int gpio_hw_set_output_value(const enum gpio_port port, u32 pin, const u32 value);
int gpio_hw_set_pull_up(const enum gpio_port port, u32 pin, const enum gpio_pullup_mode value);
int gpio_hw_set_pull_down(const enum gpio_port port, u32 pin, const enum gpio_pulldown_mode value);//portabcdpr:pd0,pd1,usb:pd0
int gpio_hw_set_drive_strength(const enum gpio_port port, u32 pin, const enum gpio_drive_strength value);
int gpio_hw_set_die(const enum gpio_port port, u32 pin, const int value);
int gpio_hw_set_dieh(const enum gpio_port port, u32 pin, const u32 value);
int gpio_hw_set_spl(const enum gpio_port port, u32 pin, const u32 value);
//read
int gpio_hw_read_port(const enum gpio_port port, u32 pin);
int gpio_hw_read_out_level(const enum gpio_port port, u32 pin);
u32 gpio_hw_read_drive_strength(const enum gpio_port port, u32 pin);//return hd1:高16位,  hd0:低16位

//多io不同模式
int gpio_hw_op_dir(const enum gpio_port port, u32 pin, u32 value, const enum port_op_mode op);
int gpio_hw_op_out(const enum gpio_port port, u32 pin, u32 value, const enum port_op_mode op);
int gpio_hw_op_die(const enum gpio_port port, u32 pin, u32 value, const enum port_op_mode op);
int gpio_hw_op_dieh(const enum gpio_port port, u32 pin, u32 value, const enum port_op_mode op);
int gpio_hw_op_pu0(const enum gpio_port port, u32 pin, u32 value, const enum port_op_mode op);
int gpio_hw_op_pu1(const enum gpio_port port, u32 pin, u32 value, const enum port_op_mode op);
int gpio_hw_op_pd0(const enum gpio_port port, u32 pin, u32 value, const enum port_op_mode op);
int gpio_hw_op_pd1(const enum gpio_port port, u32 pin, u32 value, const enum port_op_mode op);

//=================================================================================//
//@brief: CrossBar 获取某IO的输出映射寄存器
//=================================================================================//
u32 *gpio2crossbar_outreg(u32 gpio);
u32 gpio2crossbar_inport(u32 gpio);
int gpio_set_fun_output_port(u32 gpio, u32 fun_index, u8 dir_ctl, u8 data_ctl);
int gpio_disable_fun_output_port(u32 gpio);
int gpio_set_fun_input_port(u32 gpio, enum PFI_TABLE pfun);
int gpio_disable_fun_input_port(enum PFI_TABLE pfun);

//=================================================================================//
//@brief: Output/input Channel输出设置 API, 将指定IO口设置为某个外设的输出
//=================================================================================//
int gpio_och_sel_output_signal(u32 gpio, enum OUTPUT_CH_SIGNAL signal);
int gpio_och_disable_output_signal(u32 gpio, enum OUTPUT_CH_SIGNAL signal);
int gpio_ich_sel_input_signal(u32 gpio, enum INPUT_CH_SIGNAL signal, enum INPUT_CH_TYPE type);
int gpio_ich_disable_input_signal(u32 gpio, enum INPUT_CH_SIGNAL signal, enum INPUT_CH_TYPE type);

u32 gpio_get_ich_use_flag();
//获取空闲的gp_ich
//return: 0xff:error
u8 gpio_get_unoccupied_gp_ich();
//value:gp_ich号
void gpio_release_gp_ich(u8 value);

u32 get_sfc_port(void);
//打印指定组别指定pin的crossbar信息
void gpio_crossbar_fo_dump(char px_name[], u8 max_px_out_num, u16 px_mask, u32 *omap_ptr);
void gpio_crossbar_fi_dump(char px_name[], u8 max_px_in_num, u16 px_mask, u8 px_in);

#endif  /*GPIO_H*/
