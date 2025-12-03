#include "asm/power_interface.h"
#include "clock.h"
#include "os/os_api.h"

#define MSG_ENTER_CRITICAL()	lp_lock()
#define MSG_EXIT_CRITICAL()		lp_unlock()

#define P11_WKUP_MSYS(msg)		P11_SYSTEM->P2M_INT_SET = BIT(P2M_APP_INDEX)
#define MSYS_WKUP_P11(msg)		P11_SYSTEM->M2P_INT_SET = BIT(M2P_APP_INDEX)

static bool post_ack_flag = 1;

/*
function：  从缓存buf读取指定长度数据
return val: -2,没有数据 -1,数据不足：0,读取到数据
param len： 读取数据长度
 */
static int lp_buf_read(LP_Q *q, void *buf, u32 len)
{
    if (q->count == 0) {
        return LP_BUF_READ_NO_DATA;
    }

    if (q->count < len) {
        return LP_BUF_READ_NOT_ENOUGH_DATA;
    }

    u32 end = (q->out + len) % q->size;
    if (end > q->out) {
        memcpy(buf, (void *)(P11_RAM_BASE + q->start + q->out), len);
    } else {
        memcpy(buf, (void *)(P11_RAM_BASE + q->start + q->out), q->size - q->out);
        memcpy((void *)((u8 *)buf + (q->size - q->out)), (void *)(P11_RAM_BASE + q->start), end);
    }

    q->out = end;
    q->count -= len;

    return LP_BUF_NO_ERR;
}

static int lp_buf_write_enable(LP_Q *q, u32 len)
{
    if ((q->count + len) > q->size) {
        return LP_BUF_WRITE_OVER;
    } else {
        return LP_BUF_NO_ERR;
    }
}

/*
function：  从缓存buf写入指定长度数据
return val: 0,写入成功
param len： 写入的数据长度
 */
static int lp_buf_write(LP_Q *q, const void *buf, u32 len)
{
    u32 end = (q->in + len) % q->size;
    if (end > q->in) {
        memcpy((void *)(P11_RAM_BASE + q->start + q->in), buf, len);
    } else {
        memcpy((void *)(P11_RAM_BASE + q->start + q->in), buf, q->size - q->in);
        memcpy((void *)(P11_RAM_BASE + q->start), (u8 *)buf + (q->size - q->in), end);
    }

    q->in = end;
    q->count += len;

    return LP_BUF_NO_ERR;
}

/*
 * 接收到ack消息处理过程中调用
 */
void config_post_ack_flag(u32 enable)
{
    local_irq_disable();
    if (enable) {
        post_ack_flag = 1;
    } else {
        post_ack_flag = 0;
    }
    local_irq_enable();
}

static u32 m2p_get_ack_flag(u32 index)
{
    LP_Q *msg_q_p = (LP_Q *)(P11_RAM_BASE + (P2M_CBUF_ADDR0 | P2M_CBUF_ADDR1 << 8 | P2M_CBUF_ADDR2 << 16 | P2M_CBUF_ADDR3 << 24));

    MSG_ENTER_CRITICAL();
    u32 ret = msg_q_p->ack_flag & BIT(index);
    MSG_EXIT_CRITICAL();

    return ret ? 0 : 1;
}

u32 msys_ack_p11(u32 index)
{
    LP_Q *msg_q_p = (LP_Q *)(P11_RAM_BASE + (P2M_CBUF1_ADDR0 | P2M_CBUF1_ADDR1 << 8 | P2M_CBUF1_ADDR2 << 16 | P2M_CBUF1_ADDR3 << 24));

    MSG_ENTER_CRITICAL();
    msg_q_p->ack_flag &= ~BIT(index);
    MSG_EXIT_CRITICAL();

    return 0;
}

/*
 * function:主系统向P11发送消息，往消息池丢数据
 *
 * param type: 消息类型，P11收到消息后根据消息类型匹配回调函数
 * param ack：P11收到消息执行完成后是否发送应答消息给主系统，此时主系统死等消息
 * 并处理P11发送过来的所有消息
 * param len：发送消息的长度，单位为一个字节
 * param msg：发送的消息
 *
 */
int m2p_post_msg(u32 type, u32 ack, const void *msg, u32 len)
{
    struct lp_msg_head head;
    LP_Q *msg_q_p = (LP_Q *)(P11_RAM_BASE + (P2M_CBUF_ADDR0 | P2M_CBUF_ADDR1 << 8 | P2M_CBUF_ADDR2 << 16 | P2M_CBUF_ADDR3 << 24));

    if (ack == 1) {
        ASSERT(!(cpu_in_irq() || cpu_irq_disabled()), "m2p_post_msg ack");
    }

    //关中断，接口互斥
    local_irq_disable();
    MSG_ENTER_CRITICAL();

    u32 index = 0;
    if (ack) {
        msg_q_p->ack_flag |= BIT(index);
        /*收到ACK消息处理函数，不允许发送ACK消息*/
        if ((post_ack_flag != 1) && (!strcmp(os_current_task(), "pmu_task"))) {
            ASSERT(0, "post_ack_flag check fail!");
        }
    }

    head.type = type;
    head.ack = ack;
    head.len = len;
    head.index = index;

    if (lp_buf_write_enable(msg_q_p, len + sizeof(struct lp_msg_head)) == -1) {
        goto __err;
        /* return MSG_BUF_WRITE_OVER; */
    }

    //写头
    lp_buf_write(msg_q_p, &head,  sizeof(struct lp_msg_head));

    //写数据
    lp_buf_write(msg_q_p, msg, len);

    MSYS_WKUP_P11();

    MSG_EXIT_CRITICAL();

    if (ack) {
        while (1) {
            if (m2p_get_ack_flag(index)) {
                break;
            }
            //需要延时100us，否则p11开打印后会收不到消息导致卡死
            udelay(100);
            //P11等待ACK同时处理消息，防止死锁
            //p2m_msg_hdl();
        }
    }


    local_irq_enable();
    return MSG_NO_ERROR;
__err:
    MSG_EXIT_CRITICAL();
    local_irq_enable();
    return MSG_BUF_WRITE_OVER;
}

/*
 * function：从消息池中获取P11发送过来的消息
 * param head：消息的头部，包含消息类型、长度等信息
 * param len：读取消息内存的长度
 * param msg：读取到消息
 *
 * return val: MSG_NO_MSG
 *			   MSG_NO_ERROR
 * 			   MSG_BUF_ERROR
 * 			   MSG_BUF_READ_OVER
 *
 */
int p2m_get_msg(struct lp_msg_head *head, void *msg,  u32 len)
{
    LP_Q *msg_q_p = (LP_Q *)(P11_RAM_BASE + (P2M_CBUF1_ADDR0 | P2M_CBUF1_ADDR1 << 8 | P2M_CBUF1_ADDR2 << 16 | P2M_CBUF1_ADDR3 << 24));

    //长度至少是头+1byte数据
    if (len < sizeof(struct lp_msg_head) + 1) {
        return MSG_BUF_READ_OVER;
    }

    local_irq_disable();
    MSG_ENTER_CRITICAL();

    //先读取头部数据
    int ret = lp_buf_read(msg_q_p, head, sizeof(struct lp_msg_head));
    if (ret == LP_BUF_READ_NO_DATA) {
        //若没有数据则进入低功耗
        MSG_EXIT_CRITICAL();

        //确保每次事件都能唤醒p11
        //get no msg,cpu enter lowpower
        //p11_lowpower_schedule();

        local_irq_enable();

        return MSG_NO_MSG;

    } else if (ret == LP_BUF_READ_NOT_ENOUGH_DATA) {
        MSG_EXIT_CRITICAL();
        local_irq_enable();
        //读取头部的数据长度不够返回错误
        return MSG_BUF_ERROR;

    } else {

        //判断长度够不够
        if (len < head->len) {
            MSG_EXIT_CRITICAL();
            local_irq_enable();
            //读了头，数据读不了返回错误
            return MSG_BUF_READ_OVER;
        }

        ret = lp_buf_read(msg_q_p, msg, head->len);
        if (ret != LP_BUF_NO_ERR) {
            MSG_EXIT_CRITICAL();
            local_irq_enable();
            return MSG_BUF_ERROR;
        }

    }

    MSG_EXIT_CRITICAL();
    local_irq_enable();

    return MSG_NO_ERROR;
}



/*
 * function:主系统向P11发送消息，往消息池丢数据
 *
 * param name: null
 * param argc：参数个数
 */
int task_post_msg2p11(char *name, int argc, ...)
{
    int msg[16];
    va_list argptr;
    va_start(argptr, argc);
    int  param;
    ASSERT(argc < sizeof(msg) / sizeof(msg[0]))
    for (int i = 0; i < argc; ++i) {
        param = va_arg(argptr, int);
        msg[i] = param;
    }
    int err = m2p_post_msg(MSG_APP, 0, (u8 *)msg, argc * sizeof(int));
    ASSERT(!err);
    va_end(argptr);
    return 0;
}


