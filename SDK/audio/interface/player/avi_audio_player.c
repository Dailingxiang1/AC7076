

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".avi_audio_player.data.bss")
#pragma data_seg(".avi_audio_player.data")
#pragma const_seg(".avi_audio_player.text.const")
#pragma code_seg(".avi_audio_player.text")
#endif
#include "jlstream.h"
#include "media/audio_base.h"
#include "system/includes.h"
/* #include "sdk_config.h" */
#include "app_config.h"
#include "audio_config_def.h"
#include "avi_audio_player.h"
//#include "avi_audio_player.h"
#include "jldemuxer.h"
/* #include "rcsp_translator.h" */
/* #include "classic/tws_api.h" */

#if  TCFG_VIDEO_DIAL_ENABLE

struct avi_audio_player {
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

static struct list_head g_avi_audio_player_list = LIST_HEAD_INIT(g_avi_audio_player_list);
const struct stream_file_ops avi_file_ops;

#define list_for_each_avi_audio_player(p) \
    list_for_each_entry(p, &g_avi_audio_player_list, entry)

static void avi_audio_player_callback(void *private_data, int event)
{
    struct avi_audio_player *player = (struct avi_audio_player *)private_data;

    printf("avi_audio_player_callback, event: %d\n", event);
    switch (event) {
    case STREAM_EVENT_START:
        break;
    }
}


static int avi_file_read(void *file, u8 *buf, int len)
{
    int offset = 0;
    struct avi_audio_player *player = (struct avi_audio_player *)file;

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

static int avi_file_seek(void *file, int offset, int fromwhere)
{
    struct avi_audio_player *player = (struct avi_audio_player *)file;
    return fseek(player->file, offset, fromwhere);
}

int avi_file_flen(void *file)
{
    struct avi_audio_player *player = (struct avi_audio_player *)file;
    u32 len = 0;
    if (player->file) {
        len = flen(player->file);
    }
    return len;
}

static int avi_file_close(void *file)
{
    struct avi_audio_player *player = (struct avi_audio_player *)file;

    if (player->file) {
        fclose(player->file);
        player->file = NULL;
    }
    return 0;
}

static int avi_file_get_fmt(void *file, struct stream_fmt *fmt)
{
    u8 name[16];
    struct avi_audio_player *player = (struct avi_audio_player *)file;

    fget_name(player->file, name, 16);
    struct stream_file_info info = {
        .file = player,
        .fname = (char *)name,
        .ops  = &avi_file_ops,
        .scene = player->scene,
    };
    int err = jldemuxer_get_tone_file_fmt(&info, fmt);
    printf("avi_file_get_fmt%p, err: %d\n", fmt, err);
    return err;
}

const struct stream_file_ops avi_file_ops = {
    .read       = avi_file_read,
    .seek       = avi_file_seek,
    .close      = avi_file_close,
    .get_fmt    = avi_file_get_fmt,
};



int avi_audio_player_open(void *file, u8 source, struct avi_audio_player_param *param)
{
    printf("avi_audio_player_open, source: %x\n", source);
    int err;
    struct avi_audio_player *player;
    u16 uuid = 0;
    if (!param) {
        printf("avi_audio_player param is NULL \n");
        return -EFAULT;
    }

    u8 type = param->type;

    list_for_each_avi_audio_player(player) {
        if (player->source == source) {
            printf("avi_audio_player id %x is opened\n", (u32)source);
            return 0;
        }
    }

    if (type == AVI_SERVICE_CALL_DOWNSTREAM || type == AVI_SERVICE_CALL_UPSTREAM) {
        uuid = jlstream_event_notify(STREAM_EVENT_GET_PIPELINE_UUID, (int)"avi_rx_call");
    } else if (type == AVI_SERVICE_MEDIA) {
        uuid = jlstream_event_notify(STREAM_EVENT_GET_PIPELINE_UUID, (int)"avi_rx_media");
    } else {
        uuid = jlstream_event_notify(STREAM_EVENT_GET_PIPELINE_UUID, (int)"avi_voice");
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

    if (type == AVI_SERVICE_CALL_DOWNSTREAM) {
        player->stream = jlstream_pipeline_parse_by_node_name(uuid, "AVI_RX_ESCO_DOWN");
    } else if (type == AVI_SERVICE_CALL_UPSTREAM) {
        player->stream = jlstream_pipeline_parse_by_node_name(uuid, "AVI_RX_ESCO_UP");
    } else {
        player->stream = jlstream_pipeline_parse(uuid, NODE_UUID_VIDEO_DEC);
    }
    if (!player->stream) {
        err = -ENOMEM;
        goto __exit0;
    }

    list_add_tail(&player->entry, &g_avi_audio_player_list);
    /* player->bt_addr = bt_addr; */
    player->source = source;
    player->type = type;

    jlstream_set_callback(player->stream, player, avi_audio_player_callback);
    if (type == AVI_SERVICE_CALL_DOWNSTREAM || type == AVI_SERVICE_CALL_UPSTREAM) {
        jlstream_set_scene(player->stream, STREAM_SCENE_ESCO);
    } else if (type == AVI_SERVICE_MEDIA) {
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
                        NODE_IOC_SET_FILE_LEN, (int)avi_file_flen(player));


#if TCFG_DEC_OGG_OPUS_ENABLE
    jlstream_set_dec_file(player->stream, player, &avi_file_ops);
#else
    // jlstream_set_dec_file(player->stream, player->file, &avi_file_ops);

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

bool avi_audio_player_runing(u8 source)
{
    struct avi_audio_player *player;
    list_for_each_avi_audio_player(player) {
        if (player->source == source) {
            return 1;
        }
    }
    return 0;
}
void avi_audio_player_close(u8 source)
{
    struct avi_audio_player *player;
    u8 found = 0;
    const char *pipeline;

    list_for_each_avi_audio_player(player) {
        if (player->source == source) {
            found = 1;
            break;
        }
    }
    if (!found) {
        return;
    }
    if (player->type == AVI_SERVICE_CALL_DOWNSTREAM || player->type == AVI_SERVICE_CALL_UPSTREAM) {
        pipeline = "avi_rx_call";
    } else if (player->type == AVI_SERVICE_MEDIA) {
        pipeline = "avi_rx_media";
    } else {
        pipeline = "avi_voice";
    }
    jlstream_stop(player->stream, 0);
    jlstream_release(player->stream);

    list_del(&player->entry);
    free(player);

    jlstream_event_notify(STREAM_EVENT_CLOSE_PLAYER, (int)pipeline);
}
#endif /*TCFG_AVU_RX_NODE_ENABLE*/
