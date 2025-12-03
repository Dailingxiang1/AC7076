
#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ai_rx_player.data.bss")
#pragma data_seg(".ai_rx_player.data")
#pragma const_seg(".ai_rx_player.text.const")
#pragma code_seg(".ai_rx_player.text")
#endif
#include "jlstream.h"
#include "media/audio_base.h"
#include "system/includes.h"
/* #include "sdk_config.h" */
#include "app_config.h"
#include "audio_config_def.h"
#include "ai_rx_player.h"
#include "jldemuxer.h"
/* #include "rcsp_translator.h" */
/* #include "classic/tws_api.h" */

#if  TCFG_AI_RX_NODE_ENABLE

struct ai_rx_player {
    struct jlstream *stream;
    void *file;
    struct list_head entry;
    enum stream_scene scene;
    u8 *bt_addr;
    u8 read_err;
    u8 source;
    u8 type;
    int channel; //记录当前是tws左声道还是右声道
};

extern int CONFIG_BTCTLER_TWS_ENABLE;

static struct list_head g_ai_rx_player_list = LIST_HEAD_INIT(g_ai_rx_player_list);
const struct stream_file_ops ai_file_ops;

#define list_for_each_ai_rx_player(p) \
    list_for_each_entry(p, &g_ai_rx_player_list, entry)

static void ai_rx_player_callback(void *private_data, int event)
{
    struct ai_rx_player *player = (struct ai_rx_player *)private_data;

    printf("ai_rx_player_callback, event: %d\n", event);
    switch (event) {
    case STREAM_EVENT_START:
        break;
    }
}


static int ai_file_read(void *file, u8 *buf, int len)
{
    int offset = 0;
    struct ai_rx_player *player = (struct ai_rx_player *)file;

    while (len) {
        if (!player->file) {
            break;
        }

        int rlen = 0;

#if TCFG_DEC_DECRYPT_ENABLE
        u32 addr;
        addr = ftell(player->file);
        rlen = fread(buf + offset, len, 1, player->file);
        if (rlen && (rlen <= len)) {
            cryptanalysis_buff(&player->mply_cipher, buf + offset, addr, rlen); //解密了
        }
#else
        rlen = fread(buf + offset, len, 1, player->file);
#endif


        if (rlen < 0 || rlen == 0) {
            if (rlen == (-1)) {
                player->read_err  = 1; //file err
            } else {
                if (rlen != 0) {
                    player->read_err  = 2;    //dis err
                }
            }
            break;
        }
        player->read_err  = 0;
        offset += rlen;
        if ((len -= rlen) == 0) {
            break;
        }
    }
    return offset;
}

static int ai_file_seek(void *file, int offset, int fromwhere)
{
    struct ai_rx_player *player = (struct ai_rx_player *)file;
    return fseek(player->file, offset, fromwhere);
}

int ai_file_flen(void *file)
{
    struct ai_rx_player *player = (struct ai_rx_player *)file;
    u32 len = 0;
    if (player->file) {
        len = flen(player->file);
    }
    return len;
}

static int ai_file_close(void *file)
{
    struct ai_rx_player *player = (struct ai_rx_player *)file;

    if (player->file) {
        fclose(player->file);
        player->file = NULL;
    }
    return 0;
}

static int ai_file_get_fmt(void *file, struct stream_fmt *fmt)
{
    u8 name[16];
    struct ai_rx_player *player = (struct ai_rx_player *)file;

    fget_name(player->file, name, 16);
    struct stream_file_info info = {
        .file = player,
        .fname = (char *)name,
        .ops  = &ai_file_ops,
        .scene = player->scene,
    };
    int err = jldemuxer_get_tone_file_fmt(&info, fmt);
    return err;
}

const struct stream_file_ops ai_file_ops = {
    .read       = ai_file_read,
    .seek       = ai_file_seek,
    .close      = ai_file_close,
    .get_fmt    = ai_file_get_fmt,
};



int ai_rx_player_open(void *file, u8 source, struct ai_rx_player_param *param)
{
    int err;
    struct ai_rx_player *player;
    u16 uuid = 0;
    if (!param) {
        printf("ai_rx_player param is NULL \n");
        return -EFAULT;
    }

    u8 type = param->type;

    list_for_each_ai_rx_player(player) {
        if (player->source == source) {
            printf("ai_rx_player id %x is opened\n", (u32)source);
            return 0;
        }
    }

    if (type == AI_SERVICE_CALL_DOWNSTREAM || type == AI_SERVICE_CALL_UPSTREAM) {
        uuid = jlstream_event_notify(STREAM_EVENT_GET_PIPELINE_UUID, (int)"ai_rx_call");
    } else if (type == AI_SERVICE_MEDIA) {
        uuid = jlstream_event_notify(STREAM_EVENT_GET_PIPELINE_UUID, (int)"ai_rx_media");
    } else {
        uuid = jlstream_event_notify(STREAM_EVENT_GET_PIPELINE_UUID, (int)"ai_voice");
    }
    if (uuid == 0) {
        return -EFAULT;
    }

    player = zalloc(sizeof(*player));

    if (!player) {
        return -ENOMEM;
    }

    player->file        = (FILE *)file;
    player->scene       = STREAM_SCENE_OPUS;

    if (type == AI_SERVICE_CALL_DOWNSTREAM) {
        player->stream = jlstream_pipeline_parse_by_node_name(uuid, "AI_RX_ESCO_DOWN");
    } else if (type == AI_SERVICE_CALL_UPSTREAM) {
        player->stream = jlstream_pipeline_parse_by_node_name(uuid, "AI_RX_ESCO_UP");
    } else {
        player->stream = jlstream_pipeline_parse(uuid, NODE_UUID_AI_RX);
    }
    if (!player->stream) {
        err = -ENOMEM;
        goto __exit0;
    }

    list_add_tail(&player->entry, &g_ai_rx_player_list);
    /* player->bt_addr = bt_addr; */
    player->source = source;
    player->type = type;

    jlstream_set_callback(player->stream, player, ai_rx_player_callback);
    if (type == AI_SERVICE_CALL_DOWNSTREAM || type == AI_SERVICE_CALL_UPSTREAM) {
        jlstream_set_scene(player->stream, STREAM_SCENE_ESCO);
    } else if (type == AI_SERVICE_MEDIA) {
        jlstream_set_scene(player->stream, STREAM_SCENE_A2DP);
    } else {
        jlstream_set_scene(player->stream, STREAM_SCENE_AI_VOICE);

    }
#if 0
    if (CONFIG_BTCTLER_TWS_ENABLE) {
        if (tws_api_get_tws_state() & TWS_STA_SIBLING_CONNECTED) {
            player->channel = tws_api_get_local_channel() == 'L' ? AUDIO_CH_L : AUDIO_CH_R;
            jlstream_ioctl(player->stream, NODE_IOC_SET_CHANNEL, player->channel);
        } else {
        }
    }
#endif
    jlstream_ioctl(player->stream, NODE_IOC_SET_CHANNEL, AUDIO_CH_MIX);
    /* jlstream_node_ioctl(player->stream, NODE_UUID_SOURCE, NODE_IOC_SET_BTADDR, (int)bt_addr); */
    jlstream_set_scene(player->stream, player->scene);
    jlstream_node_ioctl(player->stream, NODE_UUID_SOURCE, NODE_IOC_SET_PARAM, (int)source);
    jlstream_node_ioctl(player->stream, NODE_UUID_SOURCE, NODE_IOC_SET_FMT, (int)param);
    jlstream_node_ioctl(player->stream, NODE_UUID_DECODER,
                        NODE_IOC_SET_FILE_LEN, (int)ai_file_flen(player));


#if TCFG_DEC_OGG_OPUS_ENABLE
    jlstream_set_dec_file(player->stream, player, &ai_file_ops);
#else
    jlstream_set_dec_file(player->stream, player->file, &ai_file_ops);

#endif
    err = jlstream_start(player->stream);
    if (err) {
        goto __exit1;
    }

    return 0;

__exit1:
    jlstream_release(player->stream);
__exit0:
    free(player);
    return err;
}

bool ai_rx_player_runing(u8 source)
{
    struct ai_rx_player *player;
    list_for_each_ai_rx_player(player) {
        if (player->source == source) {
            return 1;
        }
    }
    return 0;
}
void ai_rx_player_close(u8 source)
{
    struct ai_rx_player *player;
    u8 found = 0;
    const char *pipeline;

    list_for_each_ai_rx_player(player) {
        if (player->source == source) {
            found = 1;
            break;
        }
    }
    if (!found) {
        return;
    }
    if (player->type == AI_SERVICE_CALL_DOWNSTREAM || player->type == AI_SERVICE_CALL_UPSTREAM) {
        pipeline = "ai_rx_call";
    } else if (player->type == AI_SERVICE_MEDIA) {
        pipeline = "ai_rx_media";
    } else {
        pipeline = "ai_voice";
    }
    jlstream_stop(player->stream, 0);
    jlstream_release(player->stream);

    list_del(&player->entry);
    free(player);

    jlstream_event_notify(STREAM_EVENT_CLOSE_PLAYER, (int)pipeline);
}
/************************测试代码******************************/
#if 0
#include "system/includes.h"
#define SD_ROOT_PATH "storage/sd0/C/"
#define FILE_NAME SD_ROOT_PATH"bd.ogg"
FILE *opus_debug = NULL;

FILE *ai_get_file()
{
    opus_debug = fopen(FILE_NAME, "r");
    if (opus_debug == NULL) {
        ASSERT(0, "opus_debug %p ", opus_debug);
    }
    return opus_debug;
}
void ai_rx_debug()
{
    struct ai_rx_player_param param = {0};
    param.coding_type = AUDIO_CODING_OPUS;
    param.channel_mode =  AUDIO_CH_MIX;
    param.sample_rate = 16000;
    param.bit_rate = 16000;
    param.type = AI_SERVICE_VOICE;
    param.frame_dms = 200;  //20ms一帧
    extern FILE *ai_get_file();
    ai_rx_player_open((void *)ai_get_file(), 0, &param);

    extern void audio_app_volume_set(u8 state, s16 volume, u8 fade);
    audio_app_volume_set(APP_AUDIO_STATE_MUSIC, 100, 1);
}


void ai_rx_close()
{
    ai_rx_player_close(0);
    fclose(opus_debug);
}
#endif
#endif /*TCFG_AI_RX_NODE_ENABLE*/
