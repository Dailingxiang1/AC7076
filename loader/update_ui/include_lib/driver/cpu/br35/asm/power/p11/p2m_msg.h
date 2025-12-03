#ifndef __P2M_MSG_H__
#define __P2M_MSG_H__


#define REGISTER_P2M_MSG_HANDLER(pri, _type, fn) \
	const struct lp_msg_handler _##fn SEC_USED(.p2m_msg_handler)= { \
		.handler = fn, \
		.priv = pri,\
		.type = _type, \
	}

extern struct lp_msg_handler p2m_msg_handler_begin[];
extern struct lp_msg_handler p2m_msg_handler_end[];

#define list_for_each_p2m_msg_handler(p) \
	for (p = p2m_msg_handler_begin; p < p2m_msg_handler_end; p++)

int p2m_get_msg(struct lp_msg_head *head, void *msg, u32 len);
int p2m_post_msg(u32 type, u32 ack, const void *msg, u32 len);

int p2m_msg_hdl();

u32 set_p2m_ack_flag(u32 index);

#endif
