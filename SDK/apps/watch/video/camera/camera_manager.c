#include "system/includes.h"
#include "app_config.h"
#include "software_jpegenc.h"
#include "spi.h"
#include "generic/lbuf.h"
#include "camera_queue_buf.h"
#include "camera_manager.h"
#include "bf30a2_cfg.h"
#include "avilib.h"
#include "asm/dma_copy.h"

#define LOG_TAG_CONST      	CAMERA_MANAGER
#define LOG_TAG     		"[CAMERA_MANAGER]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CHAR_ENABLE		//log_char enable
#define LOG_CLI_ENABLE
#include "debug.h"

#define offsetof(type, memb) \
    ((unsigned long)(&((type *)0)->memb))

// SPI
#define SPI_FEND_SIZE 4  // 包头
#define SPI_LEND_SIZE 4  // 包尾
#define SPI_FHEAD_SIZE 4 // 行头
#define SPI_LHEAD_SIZE 4 // 行尾
#define SPI_ID HW_SPI1         // SPI ID号,（spi2可能用于nandflash）
#define QBUF_EXT_SIZE	4//qbuf拓展数据

// 接收队列BUF总大小  DMA_REVC_SIZE * DMA_REVC_BUF_NUM
#define INPUT_WIDTH			240
#define OUTPUT_HEIGHT		320
#define DMA_LINE_SIZE ( INPUT_WIDTH* 2 + SPI_FHEAD_SIZE + SPI_LHEAD_SIZE) // 摄像头一行大小 格式:YUVY
#define DMA_CAMERA_SIZE (16)                                                    // 一次SPI DMA接收的行数
#define DMA_REVC_SIZE (DMA_LINE_SIZE * DMA_CAMERA_SIZE + SPI_FEND_SIZE)      // 一次SPI DMA接收的大小,帧头、帧尾会多四个字节
#define DMA_REVC_BUF_NUM (20)                                                   // 接收BUF的数量

// JPEG缓存
#define JPEG_LBUF_SIZE (100 * 1024)   			// LBUF总大小，编码出的jpeg存放在lbuf中
#define ONE_JPEG_MAX_SIZE (10 * 1024) 			// 单张JPEG的大小，申请LBUF缓存时用到，可根据情况调整
#define JPEG_QVAL 50                  			// jpeg编码默认Q值(0-100)

enum {
    SPI_NEW_CHUNK = 1,
    SPI_DROP_FRAME,
    JPEG_TASK_KILL,
};

struct lbuf_data_head {
    int len;
    u8 data[0];
};

struct camera_handle {
    //设备驱动
    struct camera_device *dev;

    // JPEG 编码线程
    char task_name[32];
    u8 task_runing;
    OS_SEM task_kill_sem;

    // SPI接收行数统计
    int spi_recv_line_cnt;

    // jpeg lbuf
    u8 *lbuf_ptr;
    struct lbuff_head *lbuf_handle;

    // yuv buf 队列
    struct queue_buf *qbuf;

    // SPI DMA 接收buf,中转buf,接收完后拷贝到队列buf
    u8 *dma_recv_buf[2];
    int dma_recv_buf_index;

};
static struct camera_handle *camera_hdl;
#define __this (camera_hdl)

AT(.spi.text.cache.L1)
__attribute__((interrupt("")))
static void camera_hw_spi_isr()
{
    /*
     	在不可屏蔽中断调用，此函数及调用的子函数均需要放ram
    	特别注意打印。
    	否则在写flash的时候，会触发死机
     */
    JL_SPI_TypeDef *SPI_REG = (SPI_ID == HW_SPI1) ? JL_SPI1 : JL_SPI2;
    //clr pnd
    SPI_REG->CON |= BIT(14);
    if (SPI_REG->CON & BIT(21)) {
        // slave rx dma overflow
        SPI_REG->CON &= ~BIT(20); // clr
        /* log_char('O'); */
    }
    if (SPI_REG->CON & BIT(18)) {
        // slave tx dma underflow
        SPI_REG->CON &= ~BIT(17); // clr
        /* log_char('U'); */
    }
    // dma copy buf
    u8 *recv_done_buf = __this->dma_recv_buf[__this->dma_recv_buf_index];
    u8 *chunk = queue_buf_get_isr(__this->qbuf);
    if (chunk) {
        dma_memcpy_async_inirq(chunk, recv_done_buf, DMA_REVC_SIZE);
        //标记行号
        chunk[DMA_REVC_SIZE + 0] = 0x55;
        chunk[DMA_REVC_SIZE + 1] = 0xaa;
        u16 *chunk_rec_line = (u16 *)&chunk[DMA_REVC_SIZE + 2];
        *chunk_rec_line = __this->spi_recv_line_cnt;
        queue_buf_push_isr(__this->qbuf);
    }
    /* memcpy(chunk, recv_done_buf, DMA_REVC_SIZE); */
    __this->dma_recv_buf_index = (__this->dma_recv_buf_index + 1) % ARRAY_SIZE(__this->dma_recv_buf);
    __this->spi_recv_line_cnt = (DMA_CAMERA_SIZE + __this->spi_recv_line_cnt) % (__this->dev->height);
    // 帧中间,减去帧头or帧尾的长度

    u32 recv_cnt = DMA_REVC_SIZE - SPI_LEND_SIZE;
    if (__this->spi_recv_line_cnt == 0 || __this->spi_recv_line_cnt == (__this->dev->height - DMA_CAMERA_SIZE)) {
        recv_cnt = DMA_REVC_SIZE;
    }

    u8 *buf = __this->dma_recv_buf[__this->dma_recv_buf_index];

    SPI_REG->CON |= BIT(12);
    SPI_REG->ADR = (u32)buf;
    SPI_REG->CNT = recv_cnt;
}

static void camera_jpeg_dec_task(void *priv)
{
    log_debug("%s enter>>>>>>>>>>>>>>>>>!\n", __func__);

    struct camera_handle *hdl = (struct camera_handle *)priv;
    u8 jpeg_qval = JPEG_QVAL;
    log_debug("%s dev:%x", __func__, (u32)__this->dev);
    void *jpegenc_hdl = software_jpegenc_init(
                            __this->dev->width,
                            __this->dev->height,
                            __this->dev->width  + SPI_FHEAD_SIZE,
                            jpeg_qval);

    if (!jpegenc_hdl) {
        log_error("software jpegenc init err \n");
        //下面禁止加打印{
        os_sem_post(&__this->task_kill_sem);
        os_time_dly(-1);
        //}
    }

    struct lbuf_data_head *lbuf_data = NULL;
    int ret = 0;
    int jpeg_buf_size = ONE_JPEG_MAX_SIZE;
    int line_cnt = 0;
    u8 *cur_bits = NULL;
    int total_bits = 0;

    u8 fps = 0;
    u32 fps_time = jiffies_msec();
    u32 enc_time;

    while (hdl->task_runing) {
        u8 *chunk = NULL;
        /* dma_memcpy_wait_idle(); */
        chunk = queue_buf_pop(__this->qbuf);
        if (!chunk) {//pass
            log_char('P');
            os_time_dly(1);
            continue;
        }

        if ((chunk[DMA_REVC_SIZE + 0]) == 0x55 && (chunk[DMA_REVC_SIZE + 1] == 0xaa)) {
            u16 *chunk_rec_line = (u16 *)&chunk[DMA_REVC_SIZE + 2];

            if (*chunk_rec_line == line_cnt) {
                //正常执行
            } else {
                //pass当前帧
                queue_buf_reset(__this->qbuf);
                line_cnt = 0;
                log_warn("pop[c:%d l:%d]", (*chunk_rec_line), line_cnt);
                if (lbuf_data) {
                    lbuf_free(lbuf_data);
                    lbuf_data = NULL;
                    os_time_dly(1);
                    continue;
                }
            }
        } else {
            log_error("%s line_cnt:%d", __func__, line_cnt);
            put_buf(chunk + DMA_CAMERA_SIZE, 4);
        }

        //帧首，申请jpeg lbuf
        if (line_cnt == 0) {
            while (!lbuf_data) {
                lbuf_data = lbuf_alloc(__this->lbuf_handle, jpeg_buf_size);
                if (!lbuf_data) {
                    log_char('L');
                    lbuf_data = lbuf_pop(__this->lbuf_handle, BIT(0));
                    if (lbuf_data) {
                        lbuf_free(lbuf_data);
                        lbuf_data = NULL;
                    }
                    continue;
                }
            }
            total_bits = 0;
            enc_time = jiffies_msec();
        }

        cur_bits = lbuf_data->data + total_bits;
        int remain_size = jpeg_buf_size - total_bits;
        int out_bits_size = 0;
        u8 *in_buf = line_cnt ? chunk + SPI_FHEAD_SIZE :
                     chunk + SPI_FHEAD_SIZE + SPI_LHEAD_SIZE;

        if (software_jpegenc_line(jpegenc_hdl, cur_bits, remain_size,
                                  in_buf, DMA_CAMERA_SIZE, line_cnt, &out_bits_size)) {
            log_error("software jpgenc line err \n");
            continue;
        }

        total_bits += out_bits_size;
        line_cnt += DMA_CAMERA_SIZE;

        queue_buf_release(__this->qbuf);
        //帧尾 push jpeg_buf
        if (line_cnt == __this->dev->height) {
            lbuf_data->len = total_bits;
            lbuf_push(lbuf_data, BIT(0));
            lbuf_data = NULL;
            line_cnt = 0;
            //fps info
            fps++;
            if ((jiffies_msec() - fps_time) > 1000) {
                log_info("jpeg fps:%d enc_time:%lu \n", fps, jiffies_msec() - enc_time);
                fps = 0;
                fps_time = jiffies_msec();
            }
        }
    }

    log_debug("%s exit>>>>>>>>>>>>>>>>!\n", __func__);
    software_jpegenc_exit(jpegenc_hdl);
    //下面禁止加打印{
    os_sem_post(&__this->task_kill_sem);
    os_time_dly(-1);
    //}
}


int camera_spi_init()
{
    int ret  = 0;
    hw_spi_dev spi = SPI_ID;
    struct spi_platform_data spix_p_data = {
        .port = {
            IO_PORTC_01, // clk any io
            IO_PORTC_02, // do any io
            IO_PORTC_02, // di any io
            0xff,        // d2 any io
            0xff,        // d3 any io
            0xff,        // cs any io(主机不操作cs)
        },
        .role = SPI_ROLE_SLAVE,
        .mode = SPI_MODE_BIDIR_1BIT, // SPI_MODE_UNIDIR_2BIT,//SPI_MODE_BIDIR_1BIT,
        .bit_mode = SPI_FIRST_BIT_MSB,
        .cpol = 0,  // clk level in idle state:0:low,  1:high
        .cpha = 0,  // sampling edge:0:first,  1:second
        .ie_en = 0, // ie enbale:0:disable,  1:enable
        .irq_priority = 3,
        .spi_isr_callback = NULL,
        .clk = 24000000L,
    };

    ret = spi_open(spi, &spix_p_data);
    if (ret < 0) {
        log_error("spi master init error(%d)!", ret);
        return ret;
    }
    spi_set_ie(spi, 1);
    if (spi == HW_SPI1) {
        request_irq(IRQ_SPI1_IDX, 7, camera_hw_spi_isr, 0);
        irq_unmask_set(IRQ_SPI1_IDX, 0, 0);
    } else {
        request_irq(IRQ_SPI2_IDX, 7, camera_hw_spi_isr, 0);
        irq_unmask_set(IRQ_SPI2_IDX, 0, 0);
    }

    os_time_dly(2);

    u8 *buf = __this->dma_recv_buf[0];
    spi_dma_transmit_for_isr(spi, (void *)buf, DMA_REVC_SIZE, 1);
    return 0;
}

int camera_spi_deinit()
{
    hw_spi_dev spi = SPI_ID;
    spi_deinit(spi);
    return 0;
}

int camera_manager_start(void)
{
    int ret = 0;

    mem_stats();
    struct camera_device *dev = __this->dev;
    memset(__this, 0x00, sizeof(struct camera_handle));
    __this->dev = dev;
    __this->qbuf = queue_buf_create(DMA_REVC_SIZE + QBUF_EXT_SIZE, DMA_REVC_BUF_NUM);
    if (!__this->qbuf) {
        return -1;
    }

    for (int i = 0; i < ARRAY_SIZE(__this->dma_recv_buf); i++) {
        __this->dma_recv_buf[i] = malloc(DMA_REVC_SIZE);
        if (!__this->dma_recv_buf[i]) {
            log_error("dma recv buf malloc fail \n");
            goto __err;
        }
    }

    __this->lbuf_ptr = malloc_psram(JPEG_LBUF_SIZE);
    if (!__this->lbuf_ptr) {
        log_error("jpeg lbuf_ptr malloc fail \n");
        goto __err;
    }

    __this->lbuf_handle = lbuf_init(__this->lbuf_ptr, JPEG_LBUF_SIZE, 4, sizeof(struct lbuf_data_head));

    os_sem_create(&__this->task_kill_sem, 0);

    __this->task_runing = 1;

    snprintf(__this->task_name, sizeof(__this->task_name), "camera_jpeg_task");
    if (os_task_create(camera_jpeg_dec_task, __this, 10, 1024, 1024, __this->task_name)) {
        log_error(" read task create fail \n");
        __this->task_runing = 0;
        goto __err;
    }

    if (camera_spi_init()) {
        goto __err;
    }

    camera_manager_resume();

    return 0;

__err:
    if (__this->task_runing) {
        __this->task_runing = 0;
        os_sem_pend(&__this->task_kill_sem, 0);
        os_sem_del(&__this->task_kill_sem, OS_DEL_ALWAYS);

        task_kill(__this->task_name);
    }

    if (__this->qbuf) {
        queue_buf_destroy(__this->qbuf);
        __this->qbuf = NULL;
    }
    if (__this->lbuf_ptr) {
        free_psram(__this->lbuf_ptr);
        __this->lbuf_ptr = NULL;
    }

    for (int i = 0; i < ARRAY_SIZE(__this->dma_recv_buf); i++) {
        if (__this->dma_recv_buf[i]) {
            free(__this->dma_recv_buf[i]);
            __this->dma_recv_buf[i] = NULL;
        }
    }

    return -1;
}

int camera_manager_stop(void)
{

    printf("%s %d", __func__, __LINE__);
    camera_manager_suspend();
    camera_spi_deinit();


    if (__this->task_runing) {
        __this->task_runing = 0;
        os_sem_pend(&__this->task_kill_sem, 0);
        os_sem_del(&__this->task_kill_sem, OS_DEL_ALWAYS);

        task_kill(__this->task_name);
    }

    if (__this->qbuf) {
        queue_buf_destroy(__this->qbuf);
        __this->qbuf = NULL;
    }
    if (__this->lbuf_ptr) {
        free_psram(__this->lbuf_ptr);
        __this->lbuf_ptr = NULL;
    }

    for (int i = 0; i < ARRAY_SIZE(__this->dma_recv_buf); i++) {
        if (__this->dma_recv_buf[i]) {
            free(__this->dma_recv_buf[i]);
            __this->dma_recv_buf[i] = NULL;
        }
    }

    return 0;
}

void camera_manager_resume(void)
{
    if (!__this) {
        return ;
    }
    if (!__this->dev) {
        return ;
    }
    if (!__this->dev->resume) {
        return ;
    }
    __this->dev->resume();
}

void camera_manager_suspend(void)
{
    if (!__this) {
        return ;
    }
    if (!__this->dev) {
        return ;
    }
    if (!__this->dev->supend) {
        return ;
    }
    __this->dev->supend();
}

static int camera_manager_dev_check(void)
{
    struct camera_device *dev = NULL;
    list_for_each_camera_device_module(dev) {
        if (dev->check) {
            if (!dev->check()) {
                log_debug("%s find device:%x\n", __func__, (u32)dev);
                __this->dev = dev;
                return 0;
            }
        }
    }
    return -1;
}

int camera_manager_init(void)
{
    ASSERT(!__this);
    __this = malloc(sizeof(struct camera_handle));
    camera_manager_dev_check();

    if (!__this->dev) {
        return -1;
    }
    if (!__this->dev->init) {
        return -1;
    }
    return __this->dev->init();

}

int camera_manager_deinit(void)
{
    if (!__this) {
        return -1;
    }
    if (!__this->dev) {
        return -1;
    }
    if (!__this->dev->deinit) {
        return -1;
    }
    int  ret = __this->dev->deinit();
    free(__this);
    __this = NULL;
    return ret;

}

int camera_manager_data_read(u8 **pp_data, int *p_len)
{
    struct lbuf_data_head *node;

    if (!pp_data || !p_len) {
        log_error("camera read ptr err \n");
        return -1;
    }

    node = lbuf_pop(__this->lbuf_handle, BIT(0));
    if (!node) {
        return -2;
    }

    *pp_data = node->data;
    *p_len = node->len;
    return 0;
}

int camera_manager_read_done(u8 *data_buf)
{
    struct lbuf_data_head *node;

    if (!data_buf) {
        log_error("camera read done ptr err \n");
        return -1;
    }

    node = (struct lbuf_data_head *)((u8 *)data_buf - offsetof(struct lbuf_data_head, data));

    lbuf_free(node);
    return 0;
}

int camera_manager_get_dev_width_height(int *width, int *height)
{
    if (__this && __this->dev) {
        *width = __this->dev->width;
        *height = __this->dev->height;
        return 0;
    }
    return -1;
}
int camera_manager_get_dev_fps(int *fps)
{
    if (__this && __this->dev) {
        *fps = __this->dev->fps;
        return 0;
    }
    return -1;
}
