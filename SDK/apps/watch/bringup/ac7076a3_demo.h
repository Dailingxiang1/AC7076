#ifndef AC7076A3_DEMO_H
#define AC7076A3_DEMO_H
int ac7076a3_demo_clock_init(void);
void ac7076a3_demo_task(void *priv);
int ac7076a3_demo_audio_init(void);
int ac7076a3_demo_mic_start(void);
void ac7076a3_demo_audio_poll(void);
void ac7076a3_demo_beep_start(void);
void ac7076a3_demo_lcd_start(void);
void ac7076a3_demo_lcd_poll(void);
void ac7076a3_demo_lcd_task(void *priv);
void ac7076a3_demo_usb_start(void);
void ac7076a3_demo_usb_poll(void);
#endif
