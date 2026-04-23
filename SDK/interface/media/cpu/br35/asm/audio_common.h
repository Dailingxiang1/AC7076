#ifndef _AUDIO_COMMON_H_
#define _AUDIO_COMMON_H_

#include "generic/typedef.h"
#include "system/includes.h"

/************************************
             dac时钟
************************************/
#define	AUDIO_COMMON_CLK_DIG_SINGLE			(0)
#define	AUDIO_COMMON_CLK_DIF_XOSC			(1)

#define	AUDIO_COMMON_VCM_CAP_LEVEL1		(0)	// VCM-cap, 要求VDDIO >= 2.7V
#define	AUDIO_COMMON_VCM_CAP_LEVEL2		(1)	// VCM-cap, 要求VDDIO >= 2.9V
#define	AUDIO_COMMON_VCM_CAP_LEVEL3		(2)	// VCM-cap, 要求VDDIO >= 3.1V
#define	AUDIO_COMMON_VCM_CAP_LEVEL4		(3)	// VCM-cap, 要求VDDIO >= 3.3V
#define	AUDIO_COMMON_VCM_CAP_LEVEL5		(4)	// VCM-cap, 要求VDDIO >= 3.5V
#define	AUDIO_COMMON_DACLDO_CAPLESS_LEVEL1	(5)	// VCM-capless, 要求VDDIO >= 2.7v
#define	AUDIO_COMMON_DACLDO_CAPLESS_LEVEL2	(6)	// VCM-capless, 要求VDDIO >= 2.9v
#define	AUDIO_COMMON_DACLDO_CAPLESS_LEVEL3	(7)	// VCM-capless, 要求VDDIO >= 3.1v
#define	AUDIO_COMMON_DACLDO_CAPLESS_LEVEL4	(8)	// VCM-capless, 要求VDDIO >= 3.3v

typedef struct {
    u8 clock_mode;
    u8 power_level;
    u8 pmu_vbg_value;
    u8 aud_vbg_value;
    u8 vcm_cap_en;
} audio_common_param_t;

typedef struct {
    u8 pmu_vbg_value;
    u8 aud_vbg_value;
} audio_vbg_trim_t;

void audio_delay(int time_ms);
void audio_common_init(audio_common_param_t *param);
audio_common_param_t *audio_common_get_param(void);

void audio_common_audio_init(void *adc_data, void *dac_data);
void *audio_common_get_dac_data(void);
void *audio_common_get_adc_data(void);

void audio_common_clock_open(u8 clk_mode);
void audio_common_clock_close(void);
void audio_common_power_open(audio_common_param_t *param);
void audio_common_power_close(void);

int audio_adc_digital_status_add_check(int add);
int audio_adc_analog_status_add_check(u8 ch_index, int add);
int audio_dac_digital_status_add_check(int add);
int audio_dac_analog_status_add_check(int add);

int audio_common_power_trim(audio_vbg_trim_t *vbg_trim, u8 vcm_level);

void audio_adc_dmic_clock_open(u8 dmic_cken, u8 dmic_div);
#endif // _AUDIO_COMMON_H_

