#ifndef  __IRQ_H__
#define  __IRQ_H__

#include "typedef.h"

//=================================================
#define IRQ_EMUEXCPT_IDX        0
#define IRQ_EXCEPTION_IDX       1
#define IRQ_SYSCALL_IDX         2
#define IRQ_TICK_TMR_IDX        3
#define IRQ_TIME0_IDX           4
#define IRQ_TIME1_IDX           5
#define IRQ_TIME2_IDX           6
#define IRQ_UART0_IDX           7
#define IRQ_UART1_IDX           8

#define IRQ_SPI1_IDX            10
#define IRQ_IIC0_IDX            11
#define IRQ_PORT_IDX            12
#define IRQ_GPADC_IDX           13
#define IRQ_UART2_IDX           14
#define IRQ_LRCT_IDX            15

#define IRQ_GPCNT0_IDX          16
#define IRQ_QDEC0_IDX           17
#define IRQ_SD0_IDX             18
#define IRQ_USB_SOF_IDX         19
#define IRQ_USB_CTRL_IDX        20

#define IRQ_TIME3_IDX           22
#define IRQ_LED_IDX             23
#define IRQ_SD0_BRK_IDX         24
#define IRQ_MCPWM_TMR_IDX       25
#define IRQ_MCPWM_CHX_IDX       26
#define IRQ_PMU_TMR0_IDX        27
#define IRQ_PMU_TMR1_IDX        28

#define IRQ_SPI2_IDX            31

#define IRQ_P33_IDX             33
#define IRQ_PINR_IDX            34
#define IRQ_PMU_SOFT0_IDX       35
#define IRQ_PMU_SOFT1_IDX       36
#define IRQ_PMU_SOFT2_IDX       37
#define IRQ_PMU_SOFT3_IDX       38

#define IRQ_PMU_TMR2_IDX        40
#define IRQ_PMU_TMR3_IDX        41

#define IRQ_SRC0_IDX            66
#define IRQ_SPI0_IDX            68

#define IRQ_DCP_IDX             70

#define IRQ_IMD_IDX             72
#define IRQ_JPG_IDX             73
#define IRQ_GPU_IDX             74

#define IRQ_ADC_IDX             88
#define IRQ_AUDIO_IDX           89

#define IRQ_BT_TIMEBASE_IDX     100
#define IRQ_BLE_RX_IDX          101
#define IRQ_BLE_EVENT_IDX       102
#define IRQ_BT_CLKN_IDX         103
#define IRQ_BREDR_IDX           104
#define IRQ_BT_RXMCH            105
#define IRQ_SYNC_IDX            106

#define IRQ_SOFT0_IDX           120
#define IRQ_SOFT1_IDX           121
#define IRQ_SOFT2_IDX           122
#define IRQ_SOFT3_IDX           123
#define IRQ_SOFT4_IDX           124
#define IRQ_SOFT5_IDX           125
#define IRQ_SOFT6_IDX           126
#define IRQ_SOFT7_IDX           127

#define MAX_IRQ_ENTRY_NUM       128





extern u32 _IRQ_MEM_ADDR[];
#define IRQ_MEM_ADDR    (_IRQ_MEM_ADDR)

extern int ISR_BASE;
#define ISR_ENTRY  (u32)&ISR_BASE



void interrupt_init();


//---------------------------------------------//
// interrupt cli/sti
//---------------------------------------------//
static inline int int_cli(void)
{
    int msg;
    asm volatile("cli %0" : "=r"(msg) :);
    return msg;
}

static inline void int_sti(int msg)
{
    asm volatile("sti %0" :: "r"(msg));
}

void irq_common_handler(u32 irq_idx);
void irq_set_pending(u32 irq_idx);
//---------------------------------------------//
// low power waiting
//---------------------------------------------//
__attribute__((always_inline))
static inline void lp_waiting(int *ptr, int pnd, int cpd, char inum)
{
#if 0
    q32DSP(core_num())->IWKUP_NUM = inum;
    while (!(*ptr & pnd)) {
        asm volatile("idle");
    }
    *ptr |= cpd;
#else
    int con;
    q32DSP(core_num())->IWKUP_NUM = inum;
    asm volatile(
        " goto 2f                      \n\t"
        " 1:                           \n\t"
        " idle                         \n\t"
        " 2:                           \n\t"
        " %0 = [%1]                    \n\t"
        " rep 1 {                      \n\t"  // disable_bpu
        "     if((%0 & %2)==0) goto 1b \n\t"
        " }                            \n\t"
        :"=&r"(con)
        :"r"(ptr), "r"(pnd), "0"(con)
        :
    );
    *ptr = con | cpd;
#endif
}

// void irq_unmask_set(u8 index, u8 priority, u8 cpu_id);
// bool irq_read(u32 index);



void bit_clr_ie(unsigned char index);
void bit_set_ie(unsigned char index);
void request_irq(u8 index, u8 priority, void (*handler)(void), u8 cpu_id);
void unrequest_irq(u8 index);

void HWI_Install(unsigned int index, unsigned int isr, unsigned int priority);
void reg_set_ip(unsigned char index, unsigned char priority, u8 cpu_id);
void irq_init();
void irq_set_pending(u32 irq_idx);
void irq_common_handler(u32 irq_idx);


void ENABLE_INT(void);
#define enable_int ENABLE_INT

void DISABLE_INT(void);
#define disable_int DISABLE_INT

void local_irq_disable();

void local_irq_enable();


#ifdef IRQ_TIME_COUNT_EN

void irq_handler_enter(int irq);
void irq_handler_exit(int irq);
void irq_handler_times_dump();

#else

#define irq_handler_enter(irq)      do { }while(0)
#define irq_handler_exit(irq)       do { }while(0)
#define irq_handler_times_dump()    do { }while(0)

#endif



#endif

