#ifndef _DUER_AI_GET_FRAME_H_
#define _DUER_AI_GET_FRAME_H_

#include "duer_common.h"
#include "jlstream.h"
#include "ai_rx_player.h"

#if INTELLIGENT_DUER

extern enum stream_node_state ai_rx_get_frame_duer(struct ai_rx_file_handle *hdl, struct stream_frame **pframe);

extern void my_duer_audio_play();

extern void my_duer_audio_stop(void *arg);

#endif

#endif
