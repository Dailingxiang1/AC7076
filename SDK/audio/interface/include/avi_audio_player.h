#ifndef _AVI_AUDIO_PLAYER_H_
#define _AVI_AUDIO_PLAYER_H_


enum AVI_SERVICE {
    AVI_SERVICE_MEDIA,
    AVI_SERVICE_CALL_DOWNSTREAM,
    AVI_SERVICE_CALL_UPSTREAM,
    AVI_SERVICE_VOICE
};

struct avi_audio_player_param {
    u8 type;
    u8 channel_mode;
    u16 frame_dms;		//帧长时间，单位 deci-ms (ms/10)
    u32 coding_type;
    u32 sample_rate;
    u32 bit_rate;
};

enum  avi_dec_frame_len {
    FRAME_LEN_40 = 0,
    FRAME_LEN_80,
    FRAME_LEN_160,
};

int avi_audio_player_open(void *file, u8 source, struct avi_audio_player_param *param);

void avi_audio_player_close(u8 source);

bool avi_audio_player_runing(u8 source);

#endif
