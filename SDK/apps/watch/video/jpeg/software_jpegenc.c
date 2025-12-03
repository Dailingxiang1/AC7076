#include "system/includes.h"
#include "app_config.h"

// 如果定义了 USE_PSRAM，就把 malloc/free 映射到 malloc_psram/free_psram
#if TCFG_PSRAM_DEV_ENABLE
#define malloc(size)       malloc_psram(size)
#define free(ptr)          free_psram(ptr)
#else
#define malloc(size)       malloc(size)
#define free(ptr)          free(ptr)
#endif


extern int  jpgenc_get_heapsize(int width, int height);
extern void *jpgenc_init(unsigned char *heap, int width, int height, int stride, int quality);
extern void jpgenc_free(void *ctxt);
extern void jpgenc_set_quality(void *ctxt, int quality);
extern void jpgenc_get_header(void *ctxt, unsigned char **output, int *out_size);
extern void jpgenc_encode_yuyv(void *ctxt, unsigned char *data, unsigned char *output, int *out_size, int vlen, int voff);

struct software_jpegenc_hdl {
    void *jfif;
    u8 *heap;
    u8 *jpeg_header;
    int jpeg_header_size;
};



/* ------------------------------------------------------------------------------------*/
/**
 * @brief software_jpegenc_init jpeg软件分行编码初始化
 *
 * @param img_width	    图像宽
 * @param img_height    图像高
 * @param img_stride
 * @param q_val	        编码Q值，范围(1-100)
 * @return enc_hdl_ptr:成功 NULL:失败
 */
/* ------------------------------------------------------------------------------------*/
void *software_jpegenc_init(int img_width, int img_height, int img_stride, int q_val)
{
    struct software_jpegenc_hdl *hdl = malloc(sizeof(struct software_jpegenc_hdl));
    if (!hdl) {
        printf("jpgenc hdl malloc fail \n");
        return NULL;
    }
    memset(hdl, 0x00, sizeof(struct software_jpegenc_hdl));

    //获取jpg头信息大小
    int heapsize = jpgenc_get_heapsize(img_width, img_height);
    /* printf("jpgenc heapsize:%d \n",heapsize); */

    // 申请jpg头部控件
    u8 *heap = malloc(heapsize);
    if (!heap) {
        printf("jpgenc heap malloc fail \n");
        free(hdl);
        return NULL;
    }

    //JPG图像编码初始化
    u8 *header;
    int header_size;
    void *jfif = jpgenc_init(heap, img_width, img_height, img_stride, q_val);
    jpgenc_get_header(jfif, &header, &header_size);
    /* printf("jpgenc header_size:%d \n",header_size); */

    hdl->jfif = jfif;
    hdl->heap = heap;
    hdl->jpeg_header = header;
    hdl->jpeg_header_size = header_size;

    return (void *)hdl;
}

/* ------------------------------------------------------------------------------------*/
/**
 * @brief software_jpegenc_line jpeg软件分行编码
 *
 * @param jpgenc_hdl	    编码句柄,software_jpegenc_init获取
 * @param out_buf			编码位流保存buf
 * @param out_buf_size	    编码位流保存buf大小
 * @param in_data			待编码的YUYV数据buf
 * @param vlen	            编码行数
 * @param voff				编码行数偏移
 * @param jpgenc_out_size	此次编码输出位流的大小
 *
 * @return 0:成功 -1:失败
 */
/* ------------------------------------------------------------------------------------*/
int software_jpegenc_line(void *jpgenc_hdl, u8 *out_buf, int out_buf_size, u8 *in_data, int vlen, int voff, int *jpgenc_out_size)
{
    if (!jpgenc_hdl || !out_buf || !in_data) {
        printf("jpgenc ptr err \n");
        return -1;
    }

    struct software_jpegenc_hdl *hdl = (struct software_jpegenc_hdl *)jpgenc_hdl;
    int total_size = 0;
    u8 *buf_ptr = out_buf;
    int buf_size = out_buf_size;

    if (voff == 0) {
        if (buf_size < hdl->jpeg_header_size) {
            printf("jpgenc header buf_size err buf_size:%d need size:%d  \n",
                   buf_size, hdl->jpeg_header_size);
            return -1;
        }
        memcpy(out_buf, hdl->jpeg_header, hdl->jpeg_header_size);

        buf_ptr += hdl->jpeg_header_size;
        buf_size -= hdl->jpeg_header_size;
        total_size += hdl->jpeg_header_size;
    }

    int out_size = buf_size;
    jpgenc_encode_yuyv(hdl->jfif, in_data, buf_ptr, &out_size, vlen, voff);
    if (out_size == buf_size) {
        printf("jpgenc encode buf_size err \n");
        return -1;
    }
    total_size += out_size;

    *jpgenc_out_size = total_size;

    return 0;
}

/* ------------------------------------------------------------------------------------*/
/**
 * @brief software_jpegenc_exit jpeg软件编码退出流程，释放内存
 *
 * @param jpgenc_hdl			init时申请的句柄
 *
 */
/* ------------------------------------------------------------------------------------*/
void software_jpegenc_exit(void *jpgenc_hdl)
{
    if (!jpgenc_hdl) {
        return;
    }
    struct software_jpegenc_hdl *hdl = (struct software_jpegenc_hdl *)jpgenc_hdl;
    if (hdl->heap) {
        free(hdl->heap);
    }
    if (hdl->jfif) {
        jpgenc_free(hdl->jfif);
    }
    free(hdl);
}


//测试代码
//os_task_create(software_jpegenc_demo_task,NULL,10,2048,1024,"jpegenc_demo_task");
void software_jpegenc_demo_task(void *priv)
{
    mem_stats();

    //1.128x128  YUYV
    int img_width = 240; //图片宽
    int img_height = 320; //图片高
    int vlen = 32;      //每次编码的行数

    void *fp = NULL, *w_fp = NULL;
    int jpeg_buf_size, yuv_buf_size, cur_enc_line, out_buf_size, jpgenc_out_total_size;
    u8 *jpeg_buf = NULL, *yuv_buf = NULL, *out_buf = NULL;
    void *enc_hdl = NULL;
    uint32_t enc_time;

    fp = (void *)fopen("storage/sd0/C/output1.yuy", "r");
    if (!fp) {
        printf("fopen err \n");
        goto exit;
    }
    printf("yuv file len:%d \n", flen(fp));

    //确保内存足够装下, 测试图片240*320 Q值50,编码出来8K内存
    jpeg_buf_size = 20 * 1024;
    jpeg_buf = malloc(jpeg_buf_size);
    if (!jpeg_buf) {
        printf("malloc jpg buf err \n");
        mem_stats();
        goto exit;
    }

    yuv_buf_size = img_width * vlen * 2;
    yuv_buf = malloc(yuv_buf_size);
    if (!yuv_buf) {
        printf("malloc yuv buf err \n");
        mem_stats();
        goto exit;
    }

    enc_hdl = software_jpegenc_init(img_width, img_height, img_width, 50);
    if (!enc_hdl) {
        printf("software jpegenc init err \n");
        goto exit;
    }

    cur_enc_line = 0;
    out_buf = jpeg_buf;
    out_buf_size = jpeg_buf_size;
    jpgenc_out_total_size = 0;
    enc_time = 0;

    while (cur_enc_line < img_height) {
        int jpgenc_out_size = 0;

        if (fread(yuv_buf, 1, yuv_buf_size, fp) != yuv_buf_size) {
            printf("yuv fread err \n");
            goto exit;
        }

        uint32_t timer_get_ms(void);
        uint32_t enc_start_time = timer_get_ms();
        if (software_jpegenc_line(enc_hdl, out_buf, out_buf_size, yuv_buf, vlen, cur_enc_line, &jpgenc_out_size)) {
            printf("software jpgenc line err \n");
            goto exit;
        }
        enc_time += timer_get_ms() - enc_start_time;

        out_buf += jpgenc_out_size;
        out_buf_size -= jpgenc_out_size;
        jpgenc_out_total_size += jpgenc_out_size;
        cur_enc_line += vlen;
    }
    printf("jpgenc finish use time: %dms jpg size:%d \n", enc_time, jpgenc_out_total_size);

    w_fp = fopen("storage/sd0/C/output1.jpg", "w+");
    if (!w_fp) {
        printf("jpeg fopen err \n");
        goto exit;
    }
    if (fwrite(jpeg_buf, 1, jpgenc_out_total_size, w_fp) != jpgenc_out_total_size) {
        printf("jpeg fwrite err \n");
        goto exit;
    }
exit:
    printf("software_jpegenc_demo_task exit \n");
    if (w_fp) {
        fclose(w_fp);
        w_fp = NULL;
    }
    if (fp) {
        fclose(fp);
        fp = NULL;
    }
    if (enc_hdl) {
        software_jpegenc_exit(enc_hdl);
        enc_hdl = NULL;
    }
    if (yuv_buf) {
        free(yuv_buf);
        yuv_buf = NULL;
    }
    if (jpeg_buf) {
        free(jpeg_buf);
        jpeg_buf = NULL;
    }
    mem_stats();
    os_time_dly(-1);
}


