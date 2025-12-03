#include "cpu.h"
#include "rdec.h"
#include "clock.h"
#include "gpio.h"

const static u8 rdec_index_table[] = {IRQ_RDECx_IDX_LIST};

static void (*rdec_cb_table[RDEC_MAX_NUM])(u32 tid);

/* AT(.gptimer.text.cache.L1) */
___interrupt
static void rdec0_irq()
{
    /* RDEC0->CON |= BIT(RDEC_CPND); */
    if (rdec_cb_table[0]) {
        rdec_cb_table[0](0);
    }
}

static void (* const rdecx_irq_table[])(void) = {
    rdec0_irq,
};

_INLINE_ static RDEC *rdec_id2rdec(u32 id)
{
    ASSERT(id < RDEC_MAX_NUM, "func:%s(),line:%d, rdecx max num %d", __func__, __LINE__, RDEC_MAX_NUM);
    return (RDEC_REG_BASE_ADDR + id * RDEC_REG_OFFSET);
}

u32 rdec_init(const struct rdec_config *cfg)
{
    u32 id = cfg->rdec_x;
    ASSERT(id < RDEC_MAX_NUM, "func:%s(),line:%d, rdecx max num %d", __func__, __LINE__, RDEC_MAX_NUM);

    u32 con = 0;
    u32 smp = 0;
    u32 rdec_spnd = clk_get("lsb") / RDEC_FREQ / 16; //计算采样频率
    SFR(con, RDEC_SPND, 16, rdec_spnd); //设置采样频率

    if (cfg->irq_cb) {
        rdec_cb_table[id] = cfg->irq_cb;
        request_irq(rdec_index_table[id], cfg->irq_priority, rdecx_irq_table[id], 0);
    }
    if (cfg->work_mode == RDEC_WORK_AUTO) {
        con &= ~BIT(RDEC_INT_MODE);
        unrequest_irq(rdec_index_table[id]); //防止意外注册了中断
    } else if (cfg->work_mode == RDEC_WORK_TIMER) {
        con |= BIT(RDEC_INT_MODE);
        smp = RDEC_TIME * RDEC_FREQ / 1000000 - 1; //T = (smp+1)/sr_clk
    } else if (cfg->work_mode == RDEC_WORK_INTR) {
        con &= ~BIT(RDEC_INT_MODE);
    }

    SFR(con, RDEC_MODE, 2, cfg->phase_mode); //设置phase模式
    con &= ~BIT(RDEC_POL); //设置极性翻转
    con &= ~BIT(RDEC_EN); //初始化默认关闭

    gpio_set_function(IO_PORT_SPILT(cfg->rdec_a), PORT_FUNC_RDEC0_PORTA + 2 * id);
    gpio_set_function(IO_PORT_SPILT(cfg->rdec_b), PORT_FUNC_RDEC0_PORTB + 2 * id);


    RDEC *RDECx = rdec_id2rdec(id);
    RDECx->SMP = 0;
    RDECx->CON = BIT(RDEC_CPND);
    RDECx->SMP = smp;
    RDECx->CON = con;

    return id;
}

u32 rdec_start(u32 id)
{
    RDEC *RDECx = rdec_id2rdec(id);
    RDECx->CON |= BIT(RDEC_EN);
    return 0;
}
u32 rdec_pause(u32 id)
{
    RDEC *RDECx = rdec_id2rdec(id);
    RDECx->CON &= ~BIT(RDEC_EN);
    return 0;
}
u32 rdec_resume(u32 id)
{
    RDEC *RDECx = rdec_id2rdec(id);
    RDECx->CON |= BIT(RDEC_EN);
    return 0;
}
int rdec_get_value(u32 id)
{
    s8 data = 0;
    RDEC *RDECx = rdec_id2rdec(id);
    if (RDECx->CON & BIT(RDEC_PND)) {
        RDECx->CON |= BIT(RDEC_CPND);
        asm("csync");
        data = RDECx->DAT;
    }
    return data;
}
void rdec_dump()
{
    for (u8 id = RDEC_0; id < RDEC_MAX_NUM; id++) {
        RDEC *RDECx = rdec_id2rdec(id);
        printf("RDEC%d->CON:0x%x\n", id, RDECx->CON);
        printf("RDEC%d->SMP:0x%x\n", id, RDECx->SMP);
    }
}

struct _rdec_reg {
    u32 con;
    u32 smp;
};

static struct _rdec_reg rdec_reg[RDEC_MAX_NUM] AT_VOLATILE_RAM_BSS_LOWPOWER;
static u8 rdec_enter_deepsleep(void)
{
    for (u32 i = 0; i < RDEC_MAX_NUM; i++) {
        RDEC *RDECx = (RDEC *)rdec_id2rdec(i);
        if (RDECx->CON == 0) {
            continue;
        }
        rdec_reg[i].con = RDECx->CON;
        rdec_reg[i].smp = RDECx->SMP;
    }
    return 0;
}

AT_VOLATILE_CACHE_CODE_LOWPOWER
static u8 rdec_exit_deepsleep(void)
{
    for (u32 i = 0; i < RDEC_MAX_NUM; i++) {
        RDEC *RDECx = (RDEC *)(RDEC_REG_BASE_ADDR + i * RDEC_REG_OFFSET);
        if (rdec_reg[i].con == 0) {
            continue;
        }
        RDECx->CON = rdec_reg[i].con;
        RDECx->SMP = rdec_reg[i].smp;
    }
    return 0;
}

DEEPSLEEP_TARGET_REGISTER(rdec) = {
    .name   = "rdec",
    .enter  = rdec_enter_deepsleep,
    .exit   = rdec_exit_deepsleep,
};
