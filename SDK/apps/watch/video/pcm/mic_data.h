#ifndef MIC_DATA_H
#define MIC_DATA_H

#include "asm/audio_adc.h"

#define VOICE_VAD_MIC         0
#define VOICE_MCU_MIC         1
#define VOICE_DEFULT_MIC      VOICE_VAD_MIC

#define VOICE_ADC_SAMPLE_RATE               16000
#define VOICE_ADC_SAMPLE_CH                 1
#define VOICE_MIC_DATA_SAMPLE_PREIOD        10
#define VOICE_MIC_DATA_PERIOD_FRAMES        (VOICE_MIC_DATA_SAMPLE_PREIOD * VOICE_ADC_SAMPLE_RATE / 1000)

void *mic_data_open(u8 source, int buffer_size, int sample_rate);
void mic_data_close(void *mic);
int mic_data_read(void *mic, void *data, int len);
int mic_data_buffered_samples(void *mic);
void mic_data_clear(void *mic);


#endif /* MIC_DATA_H */

