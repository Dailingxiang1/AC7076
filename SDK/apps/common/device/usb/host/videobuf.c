#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".usb.data.bss")
#pragma data_seg(".usb.data")
#pragma code_seg(".usb.text")
#pragma const_seg(".usb.text.const")
#pragma str_literal_override(".usb.text.const")
#endif

#include "videobuf.h"
/* #include "video/video_ioctl.h" */
/* #include "system/malloc.h" */
#include "irq.h"
#include "asm/debug.h"

#include "debug.h"
#define LOG_TAG_CONST       USB
#define LOG_TAG             "[USB]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE


void videobuf_queue_init(struct videobuf_queue *q, int align, const char *name)
{
    q->ref = 0;
    q->buf = NULL;
    q->lbuf = NULL;
    q->align = align;
    q->streaming = 0;
    q->name = name;
    os_sem_create(&q->sem, 0);
}

void videobuf_queue_release(struct videobuf_queue *q)
{
    if (q->buf) {
        /* free(q->buf); */
        free_psram(q->buf);
        q->buf = NULL;
    }
}

int videobuf_reqbufs(struct videobuf_queue *q, struct video_reqbufs *req)
{
    if (q->lbuf) {
        return 0;
    }

    if (!req->buf) {
        /* q->buf = (void *)malloc(req->size); */
        q->buf = (void *)malloc_psram(req->size);
        if (!q->buf) {
            printf("-----------no-mem: size=%x\n", req->size);
            return -ENOMEM;
        }
        req->buf = q->buf;
    }

    q->lbuf = lbuf_init(req->buf, req->size, q->align, sizeof(struct videobuf_buffer));

    return 0;
}

int videobuf_snoop_buf(struct videobuf_queue *q, struct video_buffer *b)
{
    printf("\n [ERROR] %s -[no defined] %d\n ", __FUNCTION__, __LINE__);
    return 0;
#if 0
    int err = 0;
    struct videobuf_buffer *buf;

    while (1) {
        buf = lbuf_snoop(q->lbuf, BIT(b->index));
        if (buf) {
            if ((q->streaming & BIT(b->index))) {
                break;
            }

            log_info("\n\n\n\n\n\nmmmmmm %x, %x\n\n\n\n\n\n\n\n\n", q->streaming, b->index);
            ASSERT(buf && buf->magic == 0x56784321, ", %s, %x, %x, %s, \n", q->name,
                   (u32)buf, (u32)buf->magic, __func__);
        } else {
            if (b->noblock) {
                b->len = 0;
                return 0;
            }
            if (!(q->streaming & BIT(b->index))) {
                b->len = 0;
                return -EINVAL;
            }

            err = os_sem_pend(&q->sem, b->timeout / 10);
            if (err != OS_ERR_NONE) {
                return err;
            }
        }
    }

    ASSERT(buf && buf->magic == 0x56784321, ", %s, %x, %x, %s, \n", q->name,
           (u32)buf, (u32)buf->magic, __func__);

    b->baddr = (u32)buf->data;
    b->len = buf->len;
    b->priv = buf;
    b->time_msec = buf->msec;

    return 0;
#endif
}

int videobuf_dqbuf(struct videobuf_queue *q, struct video_buffer *b)
{
    int err = 0;
    struct videobuf_buffer *buf;

    while (1) {
        buf = lbuf_pop(q->lbuf, BIT(b->index));
        if (buf) {
            if ((q->streaming & BIT(b->index))) {
                break;
            }

            log_info("\n\n\n\n\n\nmmmmmm %x, %x\n\n\n\n\n\n\n\n\n", q->streaming, b->index);
            ASSERT(buf && buf->magic == 0x56784321, ", %s, %x, %x, %s, \n", q->name,
                   (u32)buf, (u32)buf->magic, __func__);
            lbuf_free(buf);
        } else {
            if (b->noblock) {
                b->len = 0;
                return 0;
            }
            if (!(q->streaming & BIT(b->index))) {
                b->len = 0;
                return -EINVAL;
            }

            err = os_sem_pend(&q->sem, b->timeout / 10);
            if (err != OS_ERR_NONE) {
                return err;
            }
        }
    }

    ASSERT(buf && buf->magic == 0x56784321, ", %s, %x, %x, %s, \n", q->name,
           (u32)buf, (u32)buf->magic, __func__);

    b->baddr = (u32)buf->data;
    b->len = buf->len;
    b->priv = buf;
    b->time_msec = buf->msec;

    return 0;
}


int videobuf_qbuf(struct videobuf_queue *q, struct video_buffer *b)
{
    struct videobuf_buffer *buf = (struct videobuf_buffer *)b->priv;

    ASSERT(buf && buf->magic == 0x56784321, ", %s, %x, %x, %s, \n", q->name,
           (u32)buf, (u32)buf->magic, __func__);

    lbuf_free(b->priv);

    return 0;
}


int videobuf_streamon(struct videobuf_queue *q, u8 *channel)
{
    int i;
    struct lbuff_state s;

    if (q->streaming == 0) {
        //判断之前是否有未释放的buf
        ASSERT(lbuf_empty(q->lbuf) == 1, "videobuf not free completed, %s\n", q->name);
        lbuf_state(q->lbuf, &s);
        if (s.fragment != 1) {
            lbuf_dump(q->lbuf);
            ASSERT(s.fragment == 1, "videobuf not free completed, %s\n", q->name);
        }
    } else {
        //ASSERT(lbuf_pop(q->lbuf, BIT(channel)) == NULL);
    }
    for (i = 0; i < 8; i++) {
        if (!(q->streaming & BIT(i))) {
            *channel = i;
            q->streaming |= BIT(i);
            return 0;
        }
    }

    return -EFAULT;
}

int videobuf_clear_stream(struct videobuf_queue *q, u8 channel)
{
    struct videobuf_buffer *b;

    do {
        b = lbuf_pop(q->lbuf, BIT(channel));
        if (b) {
            lbuf_free(b);
        }
    } while (b);

    return 0;
}

int videobuf_streamoff(struct videobuf_queue *q, u8 channel)
{
    struct videobuf_buffer *b;

    q->streaming &= ~BIT(channel);

    videobuf_clear_stream(q, channel);

    os_sem_post(&q->sem);

    return 0;
}


struct videobuf_buffer *videobuf_stream_alloc(struct videobuf_queue *q, u32 size)
{
    struct videobuf_buffer *b;

    ASSERT(q->lbuf, "%s\n", q->name);

    b = lbuf_alloc(q->lbuf, size);
    if (b) {
        b->magic = 0x56784321;
    }

    return b;
}

u32 videobuf_stream_free_space(struct videobuf_queue *q)
{
    ASSERT(q->lbuf, "%s\n", q->name);

    return lbuf_free_space(q->lbuf);
}

struct videobuf_buffer *videobuf_stream_realloc(struct videobuf_queue *q,
        struct videobuf_buffer *b, int size)
{
    ASSERT(b && b->magic == 0x56784321, "%s, %x, %x, %s\n", q->name, (u32)b,
           (u32)b->magic, __func__);
    return lbuf_realloc(b, size);
}

void videobuf_stream_free(struct videobuf_queue *q, struct videobuf_buffer *b)
{
    ASSERT(b && b->magic == 0x56784321, "%s, %x, %x, %s\n", q->name, (u32)b,
           (u32)b->magic, __func__);
    lbuf_free(b);
}


int videobuf_stream_finish(struct videobuf_queue *q, struct videobuf_buffer *b)
{
    ASSERT(b && b->magic == 0x56784321, "%s, %x, %x, %s\n", q->name, (u32)b,
           (u32)b->magic, __func__);

    if (q->streaming == 0) {
        /* log_i("\n\n\n\n\n\ntttt %x, %s\n\n\n\n\n\n\n\n\n", q->streaming, q->name); */
        lbuf_free(b);
        return -EINVAL;
    }

    lbuf_push(b, q->streaming);
    os_sem_set(&q->sem, 0);
    os_sem_post(&q->sem);

    return 0;
}

int videobuf_query(struct videobuf_queue *q, struct videobuf_state *sta)
{
    struct lbuff_state lbuf_sta;

    if (sta) {
        lbuf_state(q->lbuf, &lbuf_sta);

        sta->available_len = lbuf_sta.avaliable;
        sta->max_continue_len = lbuf_sta.max_continue_len;
    }

    return lbuf_traversal(q->lbuf);
}


