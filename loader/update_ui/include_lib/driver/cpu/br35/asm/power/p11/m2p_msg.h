#ifndef __M2P_MSG_H__
#define __M2P_MSG_H__

#define REGISTER_M2P_MSG_HANDLER(pri, _type, fn) \
	const struct lp_msg_handler _##fn SEC_USED(.m2p_msg_handler)= { \
		.handler = fn, \
		.priv = pri, \
		.type = _type, \
	}

extern struct lp_msg_handler m2p_msg_handler_begin[];
extern struct lp_msg_handler m2p_msg_handler_end[];

#define list_for_each_m2p_msg_handler(p) \
	for (p = m2p_msg_handler_begin; p < m2p_msg_handler_end; p++)

int m2p_get_msg(struct lp_msg_head *head, void *msg, u32 len);
int m2p_post_msg(u32 type, u32 ack, const void *msg, u32 len);

void msys_to_p11_sys_cmd(u8 cmd);

int m2p_msg_hdl();

u32 msys_ack_p11(u32 index);


/*
 * function:主系统向P11发送消息，往消息池丢数据
 *
 * param name: null
 * param argc：参数个数
 */

int task_post_msg2p11(char *name, int argc, ...);

#endif
