#ifndef _FRAME_LENGTH_ADAPTIVE_H_
#define _FRAME_LENGTH_ADAPTIVE_H_

#include "system/includes.h"
#include "circular_buf.h"

struct frame_length_adaptive_hdl {

    u16 adj_len;
    u8 *sw_buf;  //长度转换buf
    cbuffer_t sw_cbuf;
};

/* adj_len : 需要输出的数据长度
 * in_len : 输入的数据长度
 * */
struct frame_length_adaptive_hdl *frame_length_adaptive_open(int adj_len, int in_len);

int frame_length_adaptive_run(void *_hdl, void *in_data, void *out_data, u16 in_len);

void frame_length_adaptive_close(void *_hdl);


#endif/*FRAME_LENGTH_ADAPTIVE_H_*/

