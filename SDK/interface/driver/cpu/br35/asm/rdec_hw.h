#ifndef _RDEC_HW_H
#define _RDEC_HW_H

#include "typedef.h"

typedef JL_QDEC_TypeDef     RDEC;
#define RDEC0               JL_QDEC0

#define RDEC_MAX_NUM        1
#define RDEC_REG_BASE_ADDR  JL_QDEC0
#define RDEC_REG_OFFSET     0
#define IRQ_RDECx_IDX_LIST  IRQ_QDEC0_IDX


//RDECx_CON reg
#define RDEC_SPND       16 //bit16~bit31
// #define RDEC_RESERVED   11 //bit11~bit15
#define RDEC_INT_MODE   10
#define RDEC_MODE       8 //bit8~bit9
#define RDEC_PND        7
#define RDEC_CPND       6
// #define RDEC_RESERVED   2 //bit2~bit5
#define RDEC_POL        1
#define RDEC_EN         0

//RDECx_SMP     8bit
//RDECx_DAT     8bit
//RDECx_DBE     8bit

typedef enum : u8 {
    RDEC_0 = 0,
    RDEC_x,
} rdec_dev;

#endif
