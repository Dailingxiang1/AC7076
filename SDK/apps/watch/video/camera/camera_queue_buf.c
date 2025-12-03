#include "camera_queue_buf.h"
#include "app_config.h"


//SPI传输数据量大的时候不能使用psram作为SPI DMA的地址，psram速度不够会导致异常
#if TCFG_PSRAM_DEV_ENABLE
#define malloc(size)       malloc_psram(size)
#define free(ptr)          free_psram(ptr)
#else
#define malloc(size)       malloc(size)
#define free(ptr)          free(ptr)
#endif

struct queue_buf *queue_buf_create(int buf_size, int capacity)
{
    if (capacity <= 0 || buf_size <= 0) {
        printf(NULL, "queue create fail cap or size <= 0 \n");
        return NULL;
    }
    struct queue_buf *q =  malloc(sizeof(struct queue_buf));
    if (!q) {
        printf("queue create fail malloc err \n");
        return NULL;
    }
    memset(q, 0x00, sizeof(struct queue_buf));

    q->buffer = malloc(capacity * sizeof(u8 *));
    if (!q->buffer) {
        printf("queue create fail malloc err \n");
        goto err;
    }
    memset(q->buffer, 0x00, capacity * sizeof(u8 *));

    for (int i = 0; i < capacity; i++) {
        q->buffer[i] = malloc(buf_size);
        if (!q->buffer[i]) {
            printf("queue create fail malloc err \n");
            goto err;
        }
    }

    q->capacity = capacity;
    q->buf_size = buf_size;

    return q;

err:
    if (q) {
        if (q->buffer) {
            for (int i = 0; i < capacity; i++) {
                if (q->buffer[i]) {
                    free(q->buffer[i]);
                }
            }
            free(q->buffer);
        }
        free(q);
    }
    return NULL;
}

void queue_buf_destroy(struct queue_buf *q)
{
    if (q) {
        if (q->buffer) {
            for (int i = 0; i < q->capacity; i++) {
                if (q->buffer[i]) {
                    free(q->buffer[i]);
                } else {
                    break;
                }
            }
            free(q->buffer);
        }
        free(q);
    }
}

u8 *queue_buf_get_isr(struct queue_buf *q)
{
    int next_tail = (q->tail + 1) % q->capacity;
    // 如果 next_tail == head，则说明队列已满
    if (next_tail == q->head) {
        /* printf("F %d %d \n", q->head, q->tail); */
        return NULL;
    }
    return q->buffer[q->tail];
}

void queue_buf_push_isr(struct queue_buf *q)
{
    q->tail = (q->tail + 1) % q->capacity;
}

u8 *queue_buf_pop(struct queue_buf *q)
{
    if (q->head == q->tail) {
        return NULL;
    }
    return q->buffer[q->head];
}

void queue_buf_release(struct queue_buf *q)
{
    q->head = (q->head + 1) % q->capacity;
}

void queue_buf_reset(struct queue_buf *q)
{
    q->head = 0;
    q->tail = 0;
}



