#ifndef CAMERA_QUEUE_BUF_H
#define CAMERA_QUEUE_BUF_H

#include "system/includes.h"

struct queue_buf {
    u8 **buffer;
    int buf_size;
    int capacity;
    volatile int head;
    volatile int tail;
    int abort;
};


struct queue_buf *queue_buf_create(int buf_size, int capacity);
void queue_buf_destroy(struct queue_buf *q);

u8 *queue_buf_get_isr(struct queue_buf *q);
void queue_buf_push_isr(struct queue_buf *q);
u8 *queue_buf_pop(struct queue_buf *q);
void queue_buf_release(struct queue_buf *q);
void queue_buf_reset(struct queue_buf *q);

#endif /* CAMERA_QUEUE_BUF_H */


