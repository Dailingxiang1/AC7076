#ifndef PCM_DATA_H
#define PCM_DATA_H

void *pcm_data_init(int sample_rate, int frame_size, int buf_size);
void pcm_data_exit(void *data_hdl);
int pcm_data_read(void *data_hdl, u8 **pp_data, int *p_len);
int pcm_data_read_done(void *data_hdl, u8 *data_buf);


#endif /* PCM_DATA_H */

