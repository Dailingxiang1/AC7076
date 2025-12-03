#ifndef  __GPADC_V4_H__
#define  __GPADC_V4_H__
//适用于 br35

//ADC_CON0 reg
// #define GPADC_CON0_RESERVED 27 //bit27~bit31
#define GPADC_CON0_DMA_PND  26
#define GPADC_CON0_OV_PND   25
#define GPADC_CON0_EOS_PND  24
#define GPADC_CON0_EOC_PND  23
#define GPADC_CON0_SMP_PND  22
#define GPADC_CON0_CAL_PND  21
#define GPADC_CON0_ADCRDY_PND   20
// #define GPADC_CON0_RESERVED     18 //bit18~bit19
#define GPADC_CON0_DMA_STATE    17
#define GPADC_CON0_STOP_REQ     16
// #define GPADC_CON0_RESERVED     11 //bit11~bit15
#define GPADC_CON0_DMA_PND_CLR  10
#define GPADC_CON0_OV_PND_CLR   9
#define GPADC_CON0_EOS_PND_CLR  8
#define GPADC_CON0_EOC_PND_CLR  7
#define GPADC_CON0_SMP_PND_CLR  6
#define GPADC_CON0_CAL_PND_CLR  5
#define GPADC_CON0_ADCRDY_PND_CLR   4
#define GPADC_CON0_ADCDIS_KST   3
#define GPADC_CON0_STOP_KST     2
#define GPADC_CON0_SOFT_KST     1
#define GPADC_CON0_CAL_KST      0


//ADC_CON1 reg
// #define GPADC_CON1_RESERVED 26 //bit26~bit31
#define GPADC_CON1_DMA_MOD      25
#define GPADC_CON1_ALIGN        24
#define GPADC_CON1_OVSMP_SHIFT  20 //bit20~bit23
#define GPADC_CON1_OVSMP_EN     19
#define GPADC_CON1_OVSMP_LEN    9 //bit9~bit18
#define GPADC_CON1_DATA_RES     7 //bit7~bit8
#define GPADC_CON1_AUTO_OFF     6
#define GPADC_CON1_OV_MOD   5
#define GPADC_CON1_CONT     4
#define GPADC_CON1_DISCEN   3
#define GPADC_CON1_WAIT_EOC 2
#define GPADC_CON1_DMA_EN   1
#define GPADC_CON1_ADC_EN   0


//ADC_CON2 reg
// #define GPADC_CON2_RESERVED 28 //bit28~bit31
#define GPADC_CON2_DMA_PND_IE   27
#define GPADC_CON2_OV_PND_IE   26
#define GPADC_CON2_EOS_PND_IE   25
#define GPADC_CON2_EOC_PND_IE   24
#define GPADC_CON2_SMP_PND_IE   23
// #define GPADC_CON2_RESERVED 22
#define GPADC_CON2_ADCRDY_PND_IE    21
#define GPADC_CON2_PWRUP_WAIT   14 //bit14~bit20
#define GPADC_CON2_SMP_DIV      6 //bit6~bit13
#define GPADC_CON2_SEQ_LEN      2 //bit2~bit5
#define GPADC_CON2_CSPT_CMP     0 //bit0~bit1


//ADC_CON3 reg
#define GPADC_CON3_TRIG_WAIT    16 //bit16~bit31
// #define GPADC_CON3_RESERVED 15 //bit8~bit15
#define GPADC_CON3_TRIG_EN      7
#define GPADC_CON3_TRIGGER_MOD  5 //bit5~bit6
#define GPADC_CON3_TRIGGER_SEL  5 //bit0~bit4








#endif


