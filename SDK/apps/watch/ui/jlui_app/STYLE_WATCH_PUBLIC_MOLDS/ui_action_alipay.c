#include "ui/ui_api.h"
#include "jlui/ui.h"
#include "jlui_app/ui_style.h"
#include "jlui_app/qr_code.h"
#include "key_event_deal.h"
//#include "le_smartbox_adv.h"
#include "app_config.h"
#include "custom_cfg.h"
#include "system/includes.h"
#include "qr_code.h"
#include "jlgpu_driver.h"
#include "gpu_task.h"

#define LOG_TAG_CONST       UI
#define LOG_TAG     		"[UI_ALIPAY]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ui_action_alipay.data.bss")
#pragma data_seg(".ui_action_alipay.data")
#pragma const_seg(".ui_action_alipay.text.const")
#pragma code_seg(".ui_action_alipay.text")
#endif

#if (defined (CONFIG_UI_STYLE_JL_PUBLIC_MODLS_ENABLE))
#if TCFG_UI_ENABLE_QR_CODE

#define STYLE_NAME  JL

static u8 *row_img = 0;
static u8 *copy_row_img = 0;


#define QR_CODE_MESS_SIZE   			(180 * sizeof(u8))
static char *qr_code_mess = NULL;

#define alipay_QRcode_mess				qr_code_mess
#define ALIPAY_QRCODE_BUF_LEN			QR_CODE_MESS_SIZE
#define BAR_CODE_REMAIN_LEN		        8

#define QR_CODE_OUTPUT_A1               1      // A1格式绘制二维码

/* #define log_info  printf */
struct qr_code_draw {
    u8 format;				//图像格式，A1或者A8
    u16 color565;			//二维码颜色
    u16 data_w;				//图像宽度
    u16 data_h;				//图像高度
    float ratio_w;			//缩放系数
    float ratio_h;			//缩放系数
    u8 *data;				//图像数据
};

struct _QR_CODE {
    u8 qr_version;
    u8 qr_max_version;
    u8 qr_ecc_level;
    int code128_mode;
    int qr_code_max_input_len;
    int qr_buf_size;
    int img_w;
    int out_size;
    int line_size;
    struct qr_code_draw code_draw;//绘图结构体
};

struct _BAR_CODE {
    int code128_mode;
    int img_w;
    int img_h;
    int out_size;
    int line_size;
    struct qr_code_draw code_draw;//绘图结构体
};

static struct _QR_CODE *bt_con_qr_code = NULL;

static struct _QR_CODE *alipay_qr_code = NULL;

static struct _BAR_CODE *alipay_bar_code = NULL;

static jl_code_param_t *jl_code_param;

void *jl_qr_code_malloc(int size)
{
    return malloc(size);
}

int alipay_enter_check(int *window)
{
    return true;
}

int ui_qrcode_init()
{
    if (!qr_code_mess) {
        qr_code_mess = zalloc(ALIPAY_QRCODE_BUF_LEN);
        if (!qr_code_mess) {
            ASSERT(0, "qr_code_mess malloc err!");
            return -ENOMEM;
        }
    }
    return 0;
}

int ui_qrcode_uninit()
{
    if (qr_code_mess) {
        free(qr_code_mess);
        qr_code_mess = NULL;
    }
    return 0;
}

void jl_qr_code_free(void *p)
{
    free(p);
}

void bt_addr2string(u8 *addr, u8 *buf)
{
    u8 len = 0;
    for (s8 i = 5; i >= 0; i--) {
        if ((addr[i] / 16) >= 10) {
            buf[len] = 'A' + addr[i] / 16 - 10;
        } else {
            buf[len] = '0' + addr[i] / 16;
        }
        if ((addr[i] % 16) >= 10) {
            buf[len + 1] = 'A' + addr[i] % 16 - 10;
        } else {
            buf[len + 1] = '0' + addr[i] % 16;
        }
        len += 2;
        buf[len] = ':';
        len += 1;
    }
    buf[len - 1] = '\0';
    log_info("%s", buf);
}
void qr_code_message_init(void)
{
    u8 ble_addr_ptr[18] = {0};
    u8 edr_addr_ptr[18] = {0};
#if (RCSP_CHANNEL_SEL==RCSP_USE_SPP)
    u8 conway = 1;
#else /* #if (RCSP_CHANNEL_SEL==RCSP_USE_SPP) */
    u8 conway = 0;
#endif /* #if (RCSP_CHANNEL_SEL==RCSP_USE_SPP) */
    /* u16 vid = get_vid_pid_ver_from_cfg_file(GET_VID_FROM_EX_CFG); */
    /* u16 pid = get_vid_pid_ver_from_cfg_file(GET_PID_FROM_EX_CFG); */
    u16 vid = 2;
    u16 pid = 130;
    u8 ble_addr[6];
    extern u8 *bt_get_mac_addr();
    u8 *edr_addr = bt_get_mac_addr();
    extern char *bt_get_local_name();
    char *name = bt_get_local_name();
    extern int le_controller_get_mac(void *addr);
    le_controller_get_mac(ble_addr);
    bt_addr2string(ble_addr, ble_addr_ptr);
    bt_addr2string(edr_addr, edr_addr_ptr);
    snprintf(qr_code_mess, QR_CODE_MESS_SIZE,
             "{\"bleAddr\":\"%s\",\"connectWay\":\"%d\",\"edrAddr\":\"%s\",\"name\":\"%s\",\"pid\":\"%d\",\"vid\":\"%d\"}",
             ble_addr_ptr, conway, edr_addr_ptr, name, pid, vid);
}


#define _RGB565(r,g,b)  (u16)((((r)>>3)<<11)|(((g)>>2)<<5)|((b)>>3))
#define UI_RGB565(c)  \
    _RGB565((c>>16)&0xff,(c>>8)&0xff,c&0xff)


static void bilinear_inter(void *_dc, int src_w, int src_h, uint8_t *src, struct rect *dest)
{


    struct draw_context *dc = (struct draw_context *)_dc;
    struct rect *rect = &dc->disp;
    struct rect *draw_r = &dc->draw;
    u8 *pdispbuf = dc->buf;
    struct rect r;
    struct rect draw;
    int dst_stride = (rect->width * 2 + 3) / 4 * 4;

    memcpy(&draw, dest, sizeof(struct rect));
    if (get_rect_cover(&draw, draw_r, &r)) {


        int j = r.top -  dest->top;
        int i = r.left - dest->left;

        int dst_h_end = j + r.height;
        int dst_w_end = i + r.width;

        int dst_w = dest->width;
        int dst_h = dest->height;

        float scale_x = src_w * 1.0 / dst_w;
        float scale_y = src_h * 1.0 / dst_h;
        float x, y;
        float dx, dy;
        float v;


        for (; j < dst_h_end; j++) {
            y = j * scale_y;
            dy = y - (int)(y);
            for (i = 0; i < dst_w_end; i++) {
                x = i * scale_x;
                dx = x - (int)(x);
                if (((int)(y) + 1) > src_h - 1 || ((int)(x) + 1) > src_w - 1) {
                    /* dst[j * dst_w + i] = src[(int)(y)*src_w + (int)(x)]; */
                    /*  */
                    if (src[(int)(y)*src_w + (int)(x)]) {
                        pdispbuf[(j + dest->top - rect->top) *dst_stride + (r.left + i - rect->left) * 2    ] =  0xff;
                        pdispbuf[(j + dest->top - rect->top) *dst_stride + (r.left + i - rect->left) * 2 + 1] =  0xff;
                    } else {

                        pdispbuf[(j + dest->top - rect->top) *dst_stride + (r.left + i - rect->left) * 2    ] =  0x00;
                        pdispbuf[(j + dest->top - rect->top) *dst_stride + (r.left + i - rect->left) * 2 + 1] =  0x00;

                    }
                    continue;
                }
                /*if (y >= src_h - 1)
                  {
                  y = y - 1;
                  }
                  if (x >= src_w - 1)
                  {
                  x = x - 1;
                  }*/
                v = src[(int)(y) * src_w + (int)(x)] * (1 - dx) * (1 - dy) + src[(int)(y) * src_w + (int)(x) + 1] * dx * (1 - dy) + \
                    src[((int)(y) + 1) * src_w + (int)(x)] * (1 - dx) * dy + src[((int)(y) + 1) * src_w + (int)(x) + 1] * dx * dy;



                //边缘加深
                if (v < 30) {
                    v = 0;
                } else if (v > 200) {
                    v = 255;
                }


                u32 rgb = v;
                rgb = (rgb << 8) | (rgb << 16) | rgb;
                rgb = UI_RGB565(rgb);
                pdispbuf[(j + dest->top - rect->top) *dst_stride + (r.left + i - rect->left) * 2    ] =  rgb >> 8;
                pdispbuf[(j + dest->top - rect->top) *dst_stride + (r.left + i - rect->left) * 2 + 1] =  rgb & 0xff;

            }
        }
    }
}

/* ------------------------------------------------------------------------------------*/
/**
 * @brief qr_code_2_A1 二维码图像转A1格式
 *
 * @param dst_w			输出图像宽度
 * @param dst_h			输出图像高度
 * @param src			输入图像数据
 * @param src_w			输入图像宽度
 * @param src_h			输入图像高度
 * @param format		输出图像格式
 *
 * @return
 */
/* ------------------------------------------------------------------------------------*/
u8 *qr_code_2_A1(u16 *dst_w, u16 *dst_h, u8 *src, u16 src_w, u16 src_h, int format)
{
    if (format == GPU_FORMAT_A1) {
        u16 dst_out_w = (src_w + 7) / 8;
        u16 dst_out_h = src_h;
        /* u16 dst_out_h = (src_h + 7)/8; */
        /* dst_out_w = (dst_out_w + 3)/4*4; */
        /* dst_out_h = (dst_out_h + 3)/4*4; */
        /* printf("%s %d %d ",__func__,dst_out_w,dst_out_h); */
        u8 *dst_out = zalloc(dst_out_w * dst_out_h);
        /* printf("%s %d %d 0x%x",__func__,dst_out_w,dst_out_h,(u32)dst_out); */
        ASSERT(dst_out);
        int dst_w_cnt = 0;
        int dst_h_cnt = 0;
        int dst_bit_cnt = 8;
        for (int h = 0; h < src_h; h++) {
            for (int w = 0; w < src_w; w++) {
                /* printf("%s %d %d %d",__func__,dst_h_cnt,dst_w_cnt,dst_bit_cnt); */
                if (src[h * src_w + w] & 0x01) {
                    dst_out[dst_h_cnt + dst_w_cnt] |= BIT(dst_bit_cnt - 1);
                }
                dst_bit_cnt --;
                if (dst_bit_cnt == 0) {
                    dst_bit_cnt = 8;
                    dst_w_cnt ++;
                }
            }
            dst_h_cnt += dst_out_w;
            dst_w_cnt = 0;
            dst_bit_cnt = 8;
        }
        *dst_h = dst_out_h;
        *dst_w = (dst_out_w * 8);

        return dst_out;
    } else if (format == GPU_FORMAT_A8) {
        u16 dst_out_w = src_w;
        u16 dst_out_h = src_h;
        u8 *dst_out = zalloc(dst_out_w * dst_out_h);
        ASSERT(dst_out);
        for (int h = 0; h < src_h; h++) {
            for (int w = 0; w < src_w; w++) {
                if (src[h * src_w + w] & 0x01) {
                    dst_out[h * src_w + w] = 0xff;
                }
            }
        }
        *dst_h = dst_out_h;
        *dst_w = dst_out_w;
        return dst_out;
    } else {
        printf("err format_not_support!!!<%s> ", __func__);
        ASSERT(0);
    }

    return NULL;
}

static void jlui_qr_code_dump(struct _QR_CODE *info, int ret)
{
    printf("[qr_code]ret:%d mode:%d ver:%d max_ver:%d ecc_level:%d max_input:%d buf_size:%d img_w:%d out_size:%d line_size:%d out_data:0x%x\n", \
           ret, info->code128_mode, info->qr_version, info->qr_max_version, info->qr_ecc_level, \
           info->qr_code_max_input_len, info->qr_buf_size, info->img_w, info->out_size, info->line_size, (int)info->code_draw.data);
}

void jl_qr_code_process(char *qr_code_str, int qr_code_len, struct _QR_CODE *qr_code)
{
    int ret;
    u8 num = 0;
    log_info("before init:");
    log_info("qr:%s \n", qr_code_str);
    //qr_code需要消耗一定的内存：qr_buf_size+4*(21+(qr_max_version-1)*4)*(21+(qr_max_version-1)*4)
    if (!jl_code_param) {
        jl_code_param = zalloc(sizeof(jl_code_param_t));
    }
    jl_code_init(qr_code->code128_mode, qr_code->qr_version, qr_code->qr_max_version, qr_code->qr_ecc_level,  qr_code->qr_code_max_input_len, qr_code->qr_buf_size, qr_code->img_w);
    ret = jl_code_process(JL_CODE_MODE_QR_CODE, qr_code_str, qr_code_len, &qr_code->out_size, &qr_code->line_size);  //返回值为 1，说明编码成功。
    log_info("out_size:%d line_size:%d ret %d\n", qr_code->out_size, qr_code->line_size, ret);
    if (ret == 1) {                                   //out_size为基础数据大小，也就是原始数据大小
        jl_code_param->l_size = 1;
        if (jl_code_param->l_size > qr_code->line_size) {
            jl_code_param->l_size = qr_code->line_size;
        }
        jl_code_set_info(jl_code_param);  //设置一个输出数据占用几个像素点，这个地方同比放大了数据
#if QR_CODE_OUTPUT_A1
        //获取二维码原始数据
        u8 *src = get_qrcode_rawdata();
        jlui_qr_code_dump(qr_code, ret);
        u16 src_w = qr_code->out_size;
        u16 src_h = qr_code->out_size;

        qr_code->code_draw.format = GPU_FORMAT_A1;
        //转换为A1图像
        qr_code->code_draw.data = qr_code_2_A1(&qr_code->code_draw.data_w, &qr_code->code_draw.data_h, src, src_w, src_h, qr_code->code_draw.format);
#else
        /* row_img = zalloc(qr_code->out_size * jl_code_param.l_size); */
        row_img = zalloc(qr_code->out_size * jl_code_param->l_size);
        if (row_img == 0) {
            jl_code_deinit();
            log_info("after deinit:");
            return;
        }
        /* copy_row_img = zalloc(qr_code->out_size * jl_code_param.l_size * qr_code->out_size * jl_code_param.l_size); */
        copy_row_img = zalloc(qr_code->out_size * jl_code_param->l_size * qr_code->out_size * jl_code_param->l_size);
        if (copy_row_img == 0) {
            jl_code_deinit();
            log_info("after deinit:");
            return;
        }
        log_info("remain heap size:");
        for (unsigned char j = 0; j < qr_code->out_size; j++) {
            //读取输出数据，这里实际上得到的是 out_size* jl_code_param.l_size个数据，是将原数据同比放大jl_code_param.l_size倍
            jl_code_get_data(qr_code->out_size, j, row_img);
            /* put_buf(row_img, qr_code->out_size * jl_code_param.l_size); */
            for (int k = 0; k < jl_code_param->l_size; k++) {
                memcpy(copy_row_img + (j * jl_code_param->l_size + k) * qr_code->out_size * jl_code_param->l_size, row_img, qr_code->out_size * jl_code_param->l_size);
            }
        }
#endif
    }
    jl_code_deinit();
    log_info("after deinit:");
}

void jl_bar_code_process(char *bar_code_str, int bar_code_len, struct _BAR_CODE *bar_code)
{
    int ret;
    u8 num = 0;
    log_info("before init:");
    log_info("bar:%s \n", bar_code_str);
    if (!jl_code_param) {
        jl_code_param = zalloc(sizeof(jl_code_param_t));
    }
    jl_code_init(bar_code->code128_mode, 1, 0, 0, 0, 4096, bar_code->img_w);
    /* jl_code_init(bar_code->code128_mode, 1, 0, 0, 0, 4096, 244); */
    ret = jl_code_process(JL_CODE_MODE_CODE_128, bar_code_str, bar_code_len, &bar_code->out_size, &bar_code->line_size);  //返回值为 1，说明编码成功。
    log_info("out_size:%d line_size:%d", bar_code->out_size, bar_code->line_size);
    if (ret == 1) {                                   //out_size为基础数据大小，也就是原始数据大小
        jl_code_param->l_size = 1;
        if (jl_code_param->l_size > bar_code->line_size) {
            jl_code_param->l_size = bar_code->line_size;
        }
        jl_code_set_info(jl_code_param);  //设置一个输出数据占用几个像素点，这个地方同比放大了数据

        u8 *row_img = zalloc(bar_code->out_size * jl_code_param->l_size);
        if (!row_img) {
            jl_code_deinit();
            log_info("after deinit:");
            return;
        }
        if (bar_code->code_draw.data) {
            free(bar_code->code_draw.data);
        }
        u32 len = bar_code->out_size * jl_code_param->l_size;
        printf("%s buflen:%d", __func__, len);
        bar_code->code_draw.data = zalloc(len);
        if (!bar_code->code_draw.data) {
            jl_code_deinit();
            log_info("after deinit:");
            return;
        }
        log_info("remain heap size:");

        jl_code_get_data(bar_code->out_size, 0, row_img);
        for (int k = 0; k < bar_code->out_size; k++) {
            if (!row_img[k]) {
                bar_code->code_draw.data[k] = 0xff;
            }
        }
        if (row_img) {
            free(row_img);
            row_img = 0;
        }
        bar_code->code_draw.data_w = bar_code->out_size;
        bar_code->code_draw.data_h = 1;
        bar_code->code_draw.format = GPU_FORMAT_A8;

    }
    jl_code_deinit();
    if (jl_code_param) {
        free(jl_code_param);
        jl_code_param = NULL;
    }
    log_info("after deinit:");
}

void jlui_blend_mask(u8 *mask_buf, struct rect *mask_r, u8 *disp_buf, struct rect *disp_r, struct rect *draw_r, u32 color, float ratio_w, float ratio_h, int in_format);
/* ------------------------------------------------------------------------------------*/
/**
 * @brief ui_qr_code_draw_gpu 二维码绘图回调
 *
 * @param id
 * @param dst_buf
 * @param dst_r
 * @param src_r
 * @param bytes_per_pixel
 * @param priv
 */
/* ------------------------------------------------------------------------------------*/
static void ui_qr_code_draw_gpu(int id, u8 *dst_buf, struct rect *dst_r, struct rect *src_r, u8 bytes_per_pixel, void *priv)
{
    struct _QR_CODE *info = (struct _QR_CODE *)priv;
    //struct _BAR_CODE *info = (struct _BAR_CODE*)priv;
    struct rect r;
    if (!get_rect_cover(src_r, dst_r, &r)) {
        return;
    }
    struct rect disp_r = {
        .left = 0,
        .top = r.top,
        .width = dst_r->width,
        .height = r.height,
    };
    struct rect mask_r = {
        .left = 0,
        .top = 0,
        .width = info->code_draw.data_w,
        .height = info->code_draw.data_h,
    };

    struct rect draw_r;
    memcpy(&draw_r, src_r, sizeof(struct rect));
    int dst_stride = ((dst_r->width * bytes_per_pixel) + 3) / 4 * 4;
    int src_stride = (src_r->width);

    u8 *disp_buf = dst_buf;
    if (r.top != dst_r->top) {
        disp_buf += dst_stride * (r.top - dst_r->top);
    }
    u8 *mask_buf = info->code_draw.data;
    printf("%s %d %d %d %d r.top:%d dst_t:%d src_t:%d ", __func__, dst_r->width, dst_stride, src_r->width, src_stride, r.top, dst_r->top, src_r->top);
    jlui_blend_mask(mask_buf, &mask_r, disp_buf, &disp_r, &draw_r, info->code_draw.color565, info->code_draw.ratio_w, info->code_draw.ratio_h, info->code_draw.format);
}

/* ------------------------------------------------------------------------------------*/
/**
 * @brief ui_qr_code_draw_gpu 条形码绘图回调
 *
 * @param id
 * @param dst_buf
 * @param dst_r
 * @param src_r
 * @param bytes_per_pixel
 * @param priv
 */
/* ------------------------------------------------------------------------------------*/
static void ui_bar_code_draw_gpu(int id, u8 *dst_buf, struct rect *dst_r, struct rect *src_r, u8 bytes_per_pixel, void *priv)
{
    struct _BAR_CODE *info = (struct _BAR_CODE *)priv;
    struct rect r;
    if (!get_rect_cover(src_r, dst_r, &r)) {
        return;
    }
    struct rect disp_r = {
        .left = 0,
        .top = r.top,
        .width = dst_r->width,
        .height = r.height,
    };
    struct rect mask_r = {
        .left = 0,
        .top = 0,
        .width = info->code_draw.data_w,
        .height = info->code_draw.data_h,
    };

    struct rect draw_r;
    memcpy(&draw_r, src_r, sizeof(struct rect));
    int dst_stride = ((dst_r->width * bytes_per_pixel) + 3) / 4 * 4;
    int src_stride = (src_r->width);

    u8 *disp_buf = dst_buf;
    if (r.top != dst_r->top) {
        disp_buf += dst_stride * (r.top - dst_r->top);
    }
    u8 *mask_buf = info->code_draw.data;
    printf("%s %d %d %d %d r.top:%d dst_t:%d src_t:%d ", __func__, dst_r->width, dst_stride, src_r->width, src_stride, r.top, dst_r->top, src_r->top);
    jlui_blend_mask(mask_buf, &mask_r, disp_buf, &disp_r, &draw_r, info->code_draw.color565, info->code_draw.ratio_w, info->code_draw.ratio_h, info->code_draw.format);
}

void fill_rect(void *_dc, struct rect *rectangle, u16 color);
static void qr_fill_rect(void *_dc, int x, int y, int width, int height, int color)
{
    struct rect rectangle;
    rectangle.left = x;
    rectangle.top = y;
    rectangle.width = width;
    rectangle.height = height;

    fill_rect(_dc, &rectangle, color);
}


static void ui_draw_qrcode(int id, u8 *dst_buf, struct rect *dst_r, struct rect *src_r, u8 bytes_per_pixel, void *priv)
{
    int w, h;
    struct rect r;
    struct rect rect = {0};
    int dst_stride = (dst_r->width * bytes_per_pixel + 3) / 4 * 4;
    int src_stride = (src_r->width * bytes_per_pixel + 3) / 4 * 4;
    u8 step_size;
    u8 remain_size;
    struct _QR_CODE *qr_code = (struct _QR_CODE *)priv;

    struct draw_context dc = {0};
    dc.buf = dst_buf;
    memcpy(&dc.disp, dst_r, sizeof(struct rect));
    memcpy(&dc.draw, dst_r, sizeof(struct rect));    // 这里传递什么值
    memcpy(&rect, src_r, sizeof(struct rect));

    //添加白底画图区域
    qr_fill_rect(&dc, rect.left, rect.top, rect.width, rect.height, 0xffff);



    /* #if TCFG_PAY_TRANSITCODE_ENABLE */
#if 0
    //使用固定边框插值算法
    //缺点是会变模糊
    struct rect img;
    //设置15为边框
    memcpy(&img, &rect, sizeof(struct rect));
    img.top = img.top + 15;
    img.left = img.left + 15;
    img.width = img.width - 30;
    img.height = img.height - 30;
    bilinear_inter((void *)&dc, qr_code->out_size * jl_code_param->l_size, qr_code->out_size * jl_code_param->l_size, copy_row_img, &img);
    return;
#endif
    /*
     *
     *
     *
     * 以下绘图函数只能在该控件的范围内显示，超出控件区域不显示
     * */
    if ((qr_code->out_size == 0) || (jl_code_param->l_size == 0)) {
        step_size = 0;
        remain_size = 0;
    } else {
        step_size = rect.width / (qr_code->out_size * jl_code_param->l_size);
        remain_size = (rect.width % (qr_code->out_size * jl_code_param->l_size)) / 2;
    }
    /* log_info("step_size:%d %d %d",step_size,remain_size,qr_code->out_size * jl_code_param.l_size); */
    for (int i = 0; i < qr_code->out_size * jl_code_param->l_size; i++) {
        for (int j = 0; j < qr_code->out_size * jl_code_param->l_size; j++) {
            /* if (img[i][j] == 0xff) { */
            if (copy_row_img[i * qr_code->out_size * jl_code_param->l_size + j] == 0xff) {
                qr_fill_rect(&dc, rect.left + 0 + remain_size + step_size * j, rect.top + 0 + remain_size + step_size * i, step_size, step_size, 0xffff);
            }
            /* if (img[i][j] == 0x00) { */
            if (copy_row_img[i * qr_code->out_size * jl_code_param->l_size + j] == 0x00) {
                qr_fill_rect(&dc, rect.left + 0 + remain_size + step_size * j, rect.top + 0 + remain_size + step_size * i, step_size, step_size, 0x0000);
            }
        }
    }
}

static void ui_draw_barcode(int id, u8 *dst_buf, struct rect *dst_r, struct rect *src_r, u8 bytes_per_pixel, void *priv)
{
    int w, h;
    struct rect r;
    struct rect rect = {0};
    int dst_stride = (dst_r->width * bytes_per_pixel + 3) / 4 * 4;
    int src_stride = (src_r->width * bytes_per_pixel + 3) / 4 * 4;
    u8 step_size;
    u8 w_remain_size;
    u8 h_remain_size;
    u8 height_size;
    struct _BAR_CODE *bar_code = (struct _BAR_CODE *)priv;

    struct draw_context dc = {0};
    dc.buf = dst_buf;
    memcpy(&dc.disp, dst_r, sizeof(struct rect));
    memcpy(&dc.draw, dst_r, sizeof(struct rect));
    memcpy(&rect, src_r, sizeof(struct rect));

    //添加白底画图区域
    qr_fill_rect(&dc, rect.left, rect.top, rect.width, rect.height, 0xffff);
    /*
     * 以下绘图函数只能在该控件的范围内显示，超出控件区域不显示
     * */
    if ((bar_code->out_size == 0) || (jl_code_param->l_size == 0)) {
        step_size = 0;
        w_remain_size = 0;
        h_remain_size = 0;
    } else {
        step_size = rect.width / (bar_code->out_size * jl_code_param->l_size);
        w_remain_size = (rect.width % (bar_code->out_size * jl_code_param->l_size)) / 2;
        h_remain_size = BAR_CODE_REMAIN_LEN;
    }
    height_size = rect.height - h_remain_size * 2;
    log_info("rect:%d,%d,%d,%d \n", rect.left, rect.top, rect.width, rect.height);
    log_info("step_size:%d %d %d %d", step_size, w_remain_size, height_size, bar_code->out_size * jl_code_param->l_size);
    for (int j = 0; j < bar_code->out_size * jl_code_param->l_size; j++) {
        if (copy_row_img[j] == 0xff) {
            qr_fill_rect(&dc, rect.left + 0 + w_remain_size + step_size * j, rect.top + h_remain_size, step_size, height_size, 0xffff);
        }
        if (copy_row_img[j] == 0x00) {
            qr_fill_rect(&dc, rect.left + 0 + w_remain_size + step_size * j, rect.top + h_remain_size, step_size, height_size, 0x0000);
        }
    }
}


static int layout_qr_code_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    // APP二维码
    struct element *elm = (struct element *)_ctrl;
    struct draw_context *dc = (struct draw_context *)arg;
    struct rect rect = {0};
    u8 step_size;
    u8 remain_size;

    switch (event) {
    case ON_CHANGE_INIT:
        ui_qrcode_init();

        if (!bt_con_qr_code) {
            bt_con_qr_code = zalloc(sizeof(struct _QR_CODE));
            if (!bt_con_qr_code) {
                ASSERT(0, "malloc err!");
                return -ENOMEM;
            }
        }
        // 初始化部分参数
        bt_con_qr_code->code128_mode = 60;
        bt_con_qr_code->qr_version = 3;
        bt_con_qr_code->qr_max_version = 8;
        bt_con_qr_code->qr_ecc_level = 2;
        bt_con_qr_code->qr_code_max_input_len = 384;
        bt_con_qr_code->qr_buf_size = 4096;
        bt_con_qr_code->img_w = 300;

        qr_code_message_init();
        jl_qr_code_process(qr_code_mess, strlen(qr_code_mess) + 1, bt_con_qr_code);
        bt_con_qr_code->code_draw.color565 = _RGB565(255, 255, 255);
        break;
    case ON_CHANGE_SHOW:
        /*根据需要调用以下三个接口:ui_remove_backcolor,ui_remove_backimage,ui_remove_border*/
        /* ui_remove_backcolor(elm);//移除控件背景颜色 */
        /* ui_remove_backimage(elm);//移除控件背景图像 */
        /* ui_remove_border(elm);//移除控件边界 */
        break;
    case ON_CHANGE_SHOW_POST:
        ui_core_get_element_abs_rect(elm, &rect); //跟随控件移动,注释掉这句则不跟随控件移动
        int draw_width = (rect.width > rect.height) ? rect.height : rect.width;
        if (draw_width <= bt_con_qr_code->img_w) {
            ASSERT(0);
        }
        bt_con_qr_code->code_draw.ratio_w = (float)bt_con_qr_code->img_w / bt_con_qr_code->out_size;
        bt_con_qr_code->code_draw.ratio_h = (float)bt_con_qr_code->img_w / bt_con_qr_code->out_size;

        ui_draw(dc,
                NULL,
                rect.left + (draw_width - bt_con_qr_code->img_w) / 2,
                rect.top + (draw_width - bt_con_qr_code->img_w) / 2,
                bt_con_qr_code->img_w,
                bt_con_qr_code->img_w,
                ui_qr_code_draw_gpu,
                (void *)bt_con_qr_code,
                sizeof(struct _QR_CODE),
                0);
        break;
    case ON_CHANGE_RELEASE:
        if (jlgpu_scheduler_wait_sync() == -OS_TIMEOUT) {
            log_error("<%s> ON_CHANGE_RELEASE error!", __func__, __LINE__);
        }
        if (row_img) {
            /* br28_free(row_img); */
            free(row_img);
            row_img = 0;
        }
        if (copy_row_img) {
            /* br28_free(copy_row_img); */
            free(copy_row_img);
            copy_row_img = 0;
        }
        if (bt_con_qr_code && bt_con_qr_code->code_draw.data) {
            free(bt_con_qr_code->code_draw.data);
            bt_con_qr_code->code_draw.data = NULL;
        }
        if (bt_con_qr_code) {
            free(bt_con_qr_code);
            bt_con_qr_code = NULL;
        }
        if (jl_code_param) {
            free(jl_code_param);
            jl_code_param = NULL;
        }
        ui_qrcode_uninit();
        log_info("qr_code release:");
        break;
    default:
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(APP_QR_PIC)
.onchange = layout_qr_code_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};


#if TCFG_PAY_ALIOS_ENABLE

#include "alipay.h"
#include "alipay_bind.h"
#include "alipay_common.h"
#include "csi_common.h"
#include "gpio.h"

#define ALIPAY_CHECK_TIME_MS				200
#define ALIPAY_CHECK_TIME_OUT_CNT			(30*1000/ALIPAY_CHECK_TIME_MS)	// 绑定超时
#define ALIPAY_WARN_TIME_OUT_CNT			(5*1000/ALIPAY_CHECK_TIME_MS)	// 提示时间
#define ALIPAY_REFRESH_TIME_OUT_CNT			(60*1000/ALIPAY_CHECK_TIME_MS)	// 超时刷新支付码
#define ALIPAY_BINDING_TIME_OUT_CNT         (8*1000/ALIPAY_CHECK_TIME_MS)   // 绑定码超时刷新

#define ALIPAY_QRCODE_NAME_LEN			(ALIPAY_QRCODE_BUF_LEN/2)
#define ALIPAY_QRCODE_ID_LEN			(ALIPAY_QRCODE_BUF_LEN - ALIPAY_QRCODE_NAME_LEN)

#define ALIPAY_QRCODE_NAME_OFFSET		(0)
#define ALIPAY_QRCODE_ID_OFFSET			(ALIPAY_QRCODE_NAME_OFFSET + ALIPAY_QRCODE_NAME_LEN)

#define BINDING_STU_E					binding_status_e
#define GET_BINDING_STU()				alipay_get_binding_status()
#define GET_PAYCODE(x,y)				alipay_get_paycode(x,y)
#define GET_NICK_NAME(x,y)				alipay_get_nick_name(x,y)
#define GET_LOGON_ID(x,y)				alipay_get_logon_ID(x,y)

#define BINDING_STU_OK					ALIPAY_STATUS_BINDING_OK
#define BINDING_STU_FAIL				ALIPAY_STATUS_BINDING_FAIL
#define BINDING_STU_GETTING_PROFILE		ALIPAY_STATUS_START_BINDING
#define BINDING_STU_FINISH_OK			ALIPAY_STATUS_BINDING_OK
#define BINDING_STU_UNKNOWN             ALIPAY_STATUS_UNKNOWN

#define GET_BINDING_STRING(x,y)			alipay_get_binding_code(x,y)//       alipay_get_binding_string(x,y)//alipay_get_aid_code(x,y)
#define GET_VERSION()					printf("SE2.0,SDK Hard alipay V201")
#define ENV_INIT()				        alipay_pre_init()//	alipay_bind_env_init()//	alipay_pre_init()
#define UNBINDING()						alipay_unbinding()

#define ALIOS_CACHE_INIT()				alipay_cache_buf_init()
#define ALIOS_CACHE_UNINIT()			alipay_cache_buf_uninit()
#define POWER_ON()                      alipay_power_on()
#define POWER_OFF()                     alipay_power_off()

struct alipay_info_t {
    u8  alipay_check_status;
    u8  alipay_show_barcode;
    u16 alipay_check_cnt;
    u16 alipay_time_id;
    u16 alipay_binding_check_cnt;
    u32 bind_status;
    char *account_name;
    char *account_id;
#if TCFG_PAY_TRANSITCODE_ENABLE
    u8 transit_qr_ok;     //更新二维码时候不能刷新
    u8 ui_transitcode_flag;
    u16 transit_loading_timer;
    int rorate_cnt;
#endif
};
struct alipay_info_t *alipay_info = NULL;

u8 alipay_check_open_status = 0;

extern void alipay_cache_buf_init(void);
extern void alipay_cache_buf_uninit(void);
extern void upay_ble_mode_enable(u8 enable);
extern u8 get_is_paycode_enter();
extern void set_is_paycode_enter(u8 flag);

void upay_recv_data_handle(const uint8_t *data, u16 len)
{
    extern int upay2ali_ibuf_to_cbuf(u8 * buf, u32 len);
    upay2ali_ibuf_to_cbuf((u8 *)data, len);
    extern void upay2ali_send_event(void);
    upay2ali_send_event();
}

void alipay_upay_init()
{
    extern void upay_ble_regiest_recv_handle(void (*handle)(const uint8_t *data, u16 len));
    upay_ble_regiest_recv_handle(upay_recv_data_handle);
}

void alipay_power_on(void)
{
#if ALIPAY_SE_USE_RESET_PIN
    HS_IIC_Init();
    log_info("%s,%d", __func__, __LINE__);
    os_time_dly(5);
    if (alipay_check_open_status == 0) {
        csi_exit_lpm();
    }
    alipay_check_open_status = 1;
#else
    log_info("%s,%d", __func__, __LINE__);
    alipay_check_open_status = 1;
    printf("alipay_check_open_status =%d", alipay_check_open_status);
#if 1  //SE 电源配置管脚
    gpio_hw_set_pull_down(SE_POWER_GPIO / 16, BIT(SE_POWER_GPIO % 16), 0);
    gpio_hw_set_pull_up(SE_POWER_GPIO / 16, BIT(SE_POWER_GPIO % 16), 0);
    gpio_hw_set_drive_strength(SE_POWER_GPIO / 16, BIT(SE_POWER_GPIO % 16), 0);//看需求是否需要开启强推,会导致芯片功耗大
    //gpio_set_hd0(SE_POWER_GPIO, 0);
    gpio_hw_set_direction(SE_POWER_GPIO / 16, BIT(SE_POWER_GPIO % 16), 0);
    gpio_hw_set_output_value(SE_POWER_GPIO / 16, BIT(SE_POWER_GPIO % 16), 1); //1高0低
    os_time_dly(5);
#endif
    extern void HS_IIC_Init(void);
    HS_IIC_Init();
#endif
}

void alipay_power_off(void)
{
#if  ALIPAY_SE_USE_RESET_PIN
    log_info("%s,%d", __func__, __LINE__);
    csi_enter_lpm();
    alipay_check_open_status = 0;
    os_time_dly(2);
    extern void HS_IIC_Uninit(void);
    HS_IIC_Uninit();
    Set_GPIO_RESET_DLE();//设为高阻态

#else
    log_info("%s,%d", __func__, __LINE__);
    alipay_check_open_status = 0;
    printf("alipay_check_open_status =%d", alipay_check_open_status);
    extern void HS_IIC_Uninit(void);
    HS_IIC_Uninit();
    os_time_dly(1);
    gpio_hw_set_output_value(SE_POWER_GPIO / 16, BIT(SE_POWER_GPIO % 16), 0); //1高0低
#endif
}


static void lcd_alipay_sleep_enter(void)
{
    if (alipay_check_open_status) {
        alipay_power_off();
    }
}

REGISTER_LCD_SLEEP_HEADLER(lcd_alipay_sleep) = {
    .name = "alipay_sleep",
    .enter = lcd_alipay_sleep_enter,
};


void alipay_component_show()
{
    struct element *p;
    struct element *elm = ui_core_get_element_by_id(ALIPAY_LAYER);
    int id;

    list_for_each_child_element(p, (struct element *)elm) {
        if (p->id == ALIPAY_NO_BIND_LAYOUT) {
            if (p->css.invisible == 0) {
                id = ALIPAY_NO_BIND_LAYOUT;
                ui_hide(id);
                ui_show(ALIPAY_COMPONENT_EXIT_LAYOUT);
                return;
            }
        }
        if (p->id == ALIPAY_PAY_QR_LAYOUT) {
            if (p->css.invisible == 0) {
                id = ALIPAY_PAY_QR_LAYOUT;
                ui_hide(id);
                ui_show(ALIPAY_COMPONENT_EXIT_LAYOUT);
                return;
            }
        }
        if (p->id == ALIPAY_PAY_BAR_LAYOUT) {
            if (p->css.invisible == 0) {
                id = ALIPAY_PAY_BAR_LAYOUT;
                ui_hide(id);
                ui_show(ALIPAY_COMPONENT_EXIT_LAYOUT);
                return;
            }
        }
    }
}

int ui_show_transitcodes_list(const char *type, u32 arg);
static int qr_msg_info_handler(const char *type, u32 arg);

const struct uimsg_handl alipay_ui_msg_handler[] = {
#if TCFG_PAY_TRANSITCODE_ENABLE
    { "transit_show",                   ui_show_transitcodes_list     },
    { "transitcode_finsh",                   qr_msg_info_handler     },
#endif
    { NULL, NULL},      /* 必须以此结尾！ */
};

#if TCFG_PAY_TRANSITCODE_ENABLE
static u16 transit_check_qr_timer = 0;
static u8 transit_param_can_release = 0;
extern void transitcode_kill();
extern void transit_task_param_deinit();
extern u8 transit_task_is_init();
void transit_check_qr_cb(void *priv)
{
    if (transit_param_can_release) {
        if (transit_task_is_init()) {
            transitcode_kill();
        }
        transit_task_param_deinit();
        if (transit_check_qr_timer) {
            sys_timer_del(transit_check_qr_timer);
            transit_check_qr_timer = 0;
        }
        ui_auto_shut_down_enable();//开启自动灭屏
    }
}
#endif

static int alipay_page_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct window *window = (struct window *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        puts("\n***alipay_onchange***\n");
        ALIOS_CACHE_INIT();
        if (!alipay_info) {
            alipay_info = zalloc(sizeof(struct alipay_info_t));
        }

#if TCFG_PAY_TRANSITCODE_ENABLE
        extern void transit_task_param_init();
        transit_task_param_init();
        transit_param_can_release = 1;
#endif
        ui_register_msg_handler(ID_WINDOW_ALIPAY, alipay_ui_msg_handler);
        break;
    case ON_CHANGE_RELEASE:
        if (alipay_info && alipay_info->alipay_time_id) {
            sys_timer_del(alipay_info->alipay_time_id);
            alipay_info->alipay_time_id = 0;
        }
        if (alipay_info && alipay_info->account_id) {
            free(alipay_info->account_id);
            alipay_info->account_id = NULL;
        }
        if (alipay_info && alipay_info->account_name) {
            free(alipay_info->account_name);
            alipay_info->account_name = NULL;
        }
#if TCFG_PAY_TRANSITCODE_ENABLE
        extern int menu_enter_app_state();
        if (!transit_check_qr_timer && (menu_enter_app_state() != 1)) {     // 不为进入动画的时候创建
            transit_check_qr_timer = sys_timer_add(NULL, transit_check_qr_cb, 500);
        }
        if (alipay_info && alipay_info->transit_loading_timer) {
            sys_timer_del(alipay_info->transit_loading_timer);
            alipay_info->transit_loading_timer = 0;
        }
#endif
        if (alipay_info) {
            free(alipay_info);
            alipay_info = NULL;
        }
        ALIOS_CACHE_UNINIT();
        upay_ble_mode_enable(0);
        ui_auto_shut_down_enable();
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ID_WINDOW_ALIPAY)
.onchange = alipay_page_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static void upay_startup_time(void *priv)    // 初次进入支付宝应用，判断有无绑定
{
#if TCFG_PAY_ALIOS_ENABLE
    alipay_info->bind_status = GET_BINDING_STU();
    printf("binding status:0x%x \n", (int)alipay_info->bind_status);
#endif /* #if TCFG_PAY_ALIOS_ENABLE */
}

static void alipay_show_layout(void *priv)
{
    int layout = ALIPAY_NO_BIND_LAYOUT;

    if (alipay_info->bind_status) {      // 已绑定
        if (get_is_paycode_enter() == 1) {
            layout = ALIPAY_PAY_QR_LAYOUT;   // 从快捷菜单的支付码进入，直接到付款码界面
            set_is_paycode_enter(0);
        } else {
            layout = ALIPAY_BINDED_LAYOUT;
        }
    } else {
        layout = ALIPAY_NO_BIND_LAYOUT;
    }

    ui_hide(ALIPAY_START_LAYOUT);
    ui_show(layout);
}

static int alipay_startup_onchange(void *_ctrl, enum element_change_event event, void *arg)  // 支付宝初始化页面
{
    switch (event) {
    case ON_CHANGE_INIT:
#if TCFG_PAY_ALIOS_ENABLE
        POWER_ON();
#endif
        upay_startup_time(NULL);
        if (!alipay_info->alipay_time_id) {
            alipay_info->alipay_time_id = sys_timeout_add(NULL, alipay_show_layout, 1000);
        }
        break;
    case ON_CHANGE_RELEASE:
        if (alipay_info && alipay_info->alipay_time_id) {
            sys_timeout_del(alipay_info->alipay_time_id);
            alipay_info->alipay_time_id = 0;
        }
        break;
    default:
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(ALIPAY_START_LAYOUT)
.onchange = alipay_startup_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};


static int alipay_bind_qrcode_layout_onchange(void *_ctrl, enum element_change_event event, void *arg) // 显示支付宝绑定码
{
    struct element *elm = (struct element *)_ctrl;
    struct draw_context *dc = (struct draw_context *)arg;
    struct rect rect = {0};

    switch (event) {
    case ON_CHANGE_INIT:
        ui_qrcode_init();

        if (!alipay_qr_code) {
            alipay_qr_code = zalloc(sizeof(struct _QR_CODE));
            if (!alipay_qr_code) {
                ASSERT(0, "malloc err!");
                return -ENOMEM;
            }
        }
        // 初始化部分参数
        alipay_qr_code->code128_mode = 60;
        alipay_qr_code->qr_version = 3;
        alipay_qr_code->qr_max_version = 8;
        alipay_qr_code->qr_ecc_level = 2;
        alipay_qr_code->qr_code_max_input_len = 384;
        alipay_qr_code->qr_buf_size = 4096;
        alipay_qr_code->img_w = 260;

        alipay_info->alipay_check_cnt = 0;
        alipay_info->alipay_check_status = 0;
        ui_auto_shut_down_disable();
        printf("Before init ,status is %02X\n", GET_BINDING_STU());
        upay_ble_mode_enable(1);
        GET_VERSION();
        u8 ret = ENV_INIT();    //环境初始化，此时状态应该为STATUS_START_BINDING
        printf("After init ,status is %02X, ret:%d \n", GET_BINDING_STU(), ret);
        u32 msg_len = ALIPAY_QRCODE_BUF_LEN;
        ret = GET_BINDING_STRING(alipay_QRcode_mess, (int *)&msg_len);   //获取绑定码
        printf("upay_get_binding_string, len:%d, ret:%d \n", msg_len, ret);
        printf("%s\n", alipay_QRcode_mess);
        jl_qr_code_process(alipay_QRcode_mess, strlen(alipay_QRcode_mess) + 1, alipay_qr_code);
        alipay_qr_code->code_draw.color565 = _RGB565(255, 255, 255);
        break;
    case ON_CHANGE_SHOW:
        /*根据需要调用以下三个接口:ui_remove_backcolor,ui_remove_backimage,ui_remove_border*/
        /* ui_remove_backcolor(elm);//移除控件背景颜色 */
        /* ui_remove_backimage(elm);//移除控件背景图像 */
        /* ui_remove_border(elm);//移除控件边界 */
        break;
    case ON_CHANGE_SHOW_POST:
        ui_core_get_element_abs_rect(elm, &rect); //跟随控件移动,注释掉这句则不跟随控件移动
        int draw_width = (rect.width > rect.height) ? rect.height : rect.width;
        if (draw_width <= alipay_qr_code->img_w) {
            ASSERT(0);
        }
        alipay_qr_code->code_draw.ratio_w = 0.8f * (float)alipay_qr_code->img_w / alipay_qr_code->out_size;
        alipay_qr_code->code_draw.ratio_h = 0.8f * (float)alipay_qr_code->img_w / alipay_qr_code->out_size;

        ui_draw(dc,
                NULL,
                rect.left + (draw_width - alipay_qr_code->img_w) / 2 + 30,
                rect.top + (draw_width - alipay_qr_code->img_w) / 2 + 20,
                alipay_qr_code->img_w,
                alipay_qr_code->img_w,
                ui_qr_code_draw_gpu,
                (void *)alipay_qr_code,
                sizeof(struct _QR_CODE),
                0);
        break;

    case ON_CHANGE_RELEASE:
        if (row_img) {
            free(row_img);
            row_img = 0;
        }
        if (copy_row_img) {
            free(copy_row_img);
            copy_row_img = 0;
        }
        if (alipay_qr_code && alipay_qr_code->code_draw.data) {
            free(alipay_qr_code->code_draw.data);
            alipay_qr_code->code_draw.data = NULL;
        }
        if (alipay_qr_code) {
            free(alipay_qr_code);
            alipay_qr_code = NULL;
        }
        if (jl_code_param) {
            free(jl_code_param);
            jl_code_param = NULL;
        }
        ui_qrcode_uninit();
        log_info("qr_code release:");
        break;
    default:
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ALIPAY_BINDING_QR_LAYOUT)
.onchange = alipay_bind_qrcode_layout_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};


static void upay_check_binding_time(void *priv)
{
    struct ui_pic *pic = NULL;

    if (alipay_info->alipay_time_id == 0) {
        return;
    }

    if (alipay_info->alipay_check_status == 2) { //bind ok
        alipay_info->alipay_check_cnt ++;
        if (alipay_info->alipay_check_cnt > ALIPAY_WARN_TIME_OUT_CNT) {
            ui_hide(ALIPAY_NO_BIND_LAYOUT);
            ui_hide(ALIPAY_BINDING_LAYOUT);
            ui_hide(ALIPAY_BIND_SUCC_LAYOUT);
            ui_show(ALIPAY_BINDED_LAYOUT);
        }
        return ;
    } else if (alipay_info->alipay_check_status == 3) { //bind fail
        alipay_info->alipay_check_cnt ++;
        if (alipay_info->alipay_check_cnt > ALIPAY_WARN_TIME_OUT_CNT) {
            ui_hide(ALIPAY_BIND_FAIL_LAYOUT);
            ui_show(ALIPAY_NO_BIND_LAYOUT);
        }
        return ;
    }
#if TCFG_PAY_ALIOS_ENABLE
    int status = 0xff;

    alipay_query_binding_result(&status);
    log_info("quiry once, status is %02X .%x,now\n", status);
#endif
    alipay_info->alipay_binding_check_cnt++;
    /* if ((alipay_info->alipay_binding_check_cnt > ALIPAY_BINDING_TIME_OUT_CNT) && (status == BINDING_STU_UNKNOWN)) { */
    /*     log_info("Bind code timeout!\n"); */
    /*     alipay_info->alipay_check_status = 3; */
    /*     alipay_info->alipay_check_cnt = 0; */
    /*     ui_hide(ALIPAY_NO_BIND_LAYOUT); */
    /*     ui_hide(ALIPAY_BINDING_LAYOUT); */
    /*     ui_show(ALIPAY_BIND_FAIL_LAYOUT); */
    /* } */

    if (BINDING_STU_OK == status) {
        log_info("Bind OK!\n");
        alipay_info->alipay_check_status = 2;
        alipay_info->alipay_check_cnt = 0;
        ui_hide(ALIPAY_NO_BIND_LAYOUT);
        ui_hide(ALIPAY_BINDING_LAYOUT);
        ui_show(ALIPAY_BIND_SUCC_LAYOUT);

    } else if (BINDING_STU_FAIL == status) {
        log_info("Bind FAIL!\n");
        alipay_info->alipay_check_status = 3;
        alipay_info->alipay_check_cnt = 0;
        ui_hide(ALIPAY_NO_BIND_LAYOUT);
        ui_hide(ALIPAY_BINDING_LAYOUT);
        ui_show(ALIPAY_BIND_FAIL_LAYOUT);
    } else {
        printf("%x %x\n", BINDING_STU_GETTING_PROFILE, BINDING_STU_FINISH_OK);
        if ((status >= BINDING_STU_GETTING_PROFILE) && (status <= BINDING_STU_FINISH_OK)) {
            alipay_info->alipay_check_cnt ++;
            log_info("Bind ing \n");
            if (alipay_info->alipay_check_cnt > ALIPAY_CHECK_TIME_OUT_CNT) {
                alipay_info->alipay_check_status = 3;
                alipay_info->alipay_check_cnt = 0;
                ui_hide(ALIPAY_NO_BIND_LAYOUT);
                ui_hide(ALIPAY_BINDING_LAYOUT);
                ui_show(ALIPAY_BIND_FAIL_LAYOUT);
                return;
            }
            if (alipay_info->alipay_check_status == 0) {
                alipay_info->alipay_check_status = 1;
                alipay_info->alipay_check_cnt = 0;
                ui_hide(ALIPAY_NO_BIND_LAYOUT);
                ui_show(ALIPAY_BINDING_LAYOUT);
            } else {
                pic = ui_pic_for_id(ALIPAY_BINDING_PIC);
                if (pic) {
                    ui_core_set_element_rotate(pic, 81, 81, 156, 138, 36 * alipay_info->alipay_check_cnt, true);
                    ui_core_redraw(pic);
                }
            }
        }
    }
}


static int layout_alipay_bind_onchange(void *_ctrl, enum element_change_event event, void *arg)  // 获取支付宝绑定码
{
    switch (event) {
    case ON_CHANGE_INIT:
#if TCFG_PAY_ALIOS_ENABLE
        printf("@@@@@@@@@@@timer add\n");
        alipay_info->alipay_binding_check_cnt = 0;
        if (!alipay_info->alipay_time_id) {
            alipay_info->alipay_time_id = sys_timer_add(NULL, upay_check_binding_time, ALIPAY_CHECK_TIME_MS);
        }
#endif /* #if TCFG_PAY_ALIOS_ENABLE */
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ALIPAY_NO_BIND_LAYOUT)
.onchange = layout_alipay_bind_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};


static int alipay_bind_res_onchange(void *_ctrl, enum element_change_event event, void *arg)  // 绑定结果显示
{
    switch (event) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
#if TCFG_PAY_ALIOS_ENABLE
        if (alipay_info && alipay_info->alipay_time_id) {
            sys_timer_del(alipay_info->alipay_time_id);
            alipay_info->alipay_time_id = 0;
        }
        ui_auto_shut_down_enable();
#endif /* #if TCFG_PAY_ALIOS_ENABLE */
        break;
    default:
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ALIPAY_BIND_SUCC_LAYOUT)
.onchange = alipay_bind_res_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(ALIPAY_BIND_FAIL_LAYOUT)
.onchange = alipay_bind_res_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int alipay_set_menu(void *_ctrl, enum element_change_event event, void *arg)
{
    // 支付宝设置界面
    switch (event) {
    case ON_CHANGE_INIT:
#if TCFG_PAY_ALIOS_ENABLE
        if (alipay_info && alipay_info->alipay_time_id) {
            sys_timer_del(alipay_info->alipay_time_id);
            alipay_info->alipay_time_id = 0;
        }
        ui_auto_shut_down_enable();
#endif /* #if TCFG_PAY_ALIOS_ENABLE */
        break;
    case ON_CHANGE_RELEASE:
        printf("%s,%d \n", __func__, __LINE__);
        break;
    default:
        break;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(ALIPAY_BINDED_LAYOUT)
.onchange = alipay_set_menu,
 .onkey = NULL,
  .ontouch = NULL,
};

static int alipay_set_ontouch(void *ctr, struct element_touch_event *e)
{
    // 支付宝设置界面
    struct ui_grid *grid = (struct ui_grid *)ctr;
    static u8 touch_action = 0;
    int sel_item;

    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        if (touch_action != 1) {
            break;
        }

        sel_item = ui_grid_cur_item(grid);
        if (sel_item < 0) {
            break;
        }
        switch (sel_item) {
        case 0:// 付款码
            ui_hide(ALIPAY_BINDED_LAYOUT);
            ui_show(ALIPAY_PAY_QR_LAYOUT);
            break;
        case 1:// 乘车码
            ui_hide(ALIPAY_BINDED_LAYOUT);
            ui_show(ALIPAY_TRANSIT_LAYOUT);
            break;
        case 2:// 设置
            ui_hide(ALIPAY_BINDED_LAYOUT);
            ui_show(ALIPAY_UNBIND_LAYOUT);
            break;
        }

        break;
    case ELM_EVENT_TOUCH_HOLD:
        break;
    case ELM_EVENT_TOUCH_MOVE:
        touch_action = 2;
        return true;
        break;
    case ELM_EVENT_TOUCH_DOWN:
        touch_action = 1;
        break;
    case ELM_EVENT_TOUCH_U_MOVE:
        return true;
        break;
    case ELM_EVENT_TOUCH_D_MOVE:
        return true;
        break;
    case ELM_EVENT_TOUCH_ENERGY:
        return true;
        break;
    }

    return false;
}
static int vlist_default_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)_ctrl;

    switch (event) {
    case ON_CHANGE_INIT:
        ui_grid_set_slide_direction(grid, SCROLL_DIRECTION_UD);
        struct scroll_area area = {0, 0, 10000, 10000};
        ui_grid_set_scroll_area(grid, &area);
        ui_grid_flick_ctrl_close(grid, 1);
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        break;
    }
    return 0;
}
REGISTER_UI_EVENT_HANDLER(ALIPAY_BINDED_VLIST)//设置-垂直列表
.onchange =  vlist_default_onchange,
 .onkey = NULL,
  .ontouch = alipay_set_ontouch,
};

static int unbind_sure_button_ontouch(void *ctr, struct element_touch_event *e)
{
    // 支付宝接触绑定界面
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
        break;
    case ELM_EVENT_TOUCH_UP:
        ui_hide(ALIPAY_UNBIND_LAYOUT);
        ui_show(ALIPAY_UNBIND_SURE_LAYOUT);
        return true;
        break;
    default:
        return false;
        break;
    }
    return false;//不接管消息
}

REGISTER_UI_EVENT_HANDLER(ALIPAY_UNBIND_SURE_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = unbind_sure_button_ontouch,
};

static int alipay_unbind_text_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_text *text = (struct ui_text *)_ctrl;
    u32 account_name_len = 128;
    u32 account_id_len = 128;

    switch (event) {
    case ON_CHANGE_INIT:
        switch (text->elm.id) {
        case ALIPAY_ACCOUNT_NAME:
            alipay_info->account_name = zalloc(account_name_len);
            alipay_get_nick_name((u8 *)alipay_info->account_name, &account_name_len);
            ui_text_set_text_attrs(text, (const char *)alipay_info->account_name, strlen(alipay_info->account_name), FONT_ENCODE_UTF8, 0, FONT_DEFAULT);
            break;
        case ALIPAY_ACCOUNT_ID:
            alipay_info->account_id = zalloc(account_id_len);
            alipay_get_logon_ID((u8 *)alipay_info->account_id, &account_id_len);
            ui_text_set_text_attrs(text, (const char *)alipay_info->account_id, strlen(alipay_info->account_id), FONT_ENCODE_UTF8, 0, FONT_DEFAULT);
            break;
        default:
            break;
        }
        break;
    case ON_CHANGE_RELEASE:
        if (alipay_info && alipay_info->account_id) {
            free(alipay_info->account_id);
            alipay_info->account_id = NULL;
        }
        if (alipay_info && alipay_info->account_name) {
            free(alipay_info->account_name);
            alipay_info->account_name = NULL;
        }
        break;
    default:
        break;
    }
    return 0;
}
REGISTER_UI_EVENT_HANDLER(ALIPAY_ACCOUNT_NAME)
.onchange = alipay_unbind_text_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
REGISTER_UI_EVENT_HANDLER(ALIPAY_ACCOUNT_ID)
.onchange = alipay_unbind_text_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int alipay_unbind_layout_ontouch(void *ctr, struct element_touch_event *e)
{
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
        break;
    case ELM_EVENT_TOUCH_R_MOVE:
        ui_hide(ALIPAY_UNBIND_LAYOUT);
        ui_show(ALIPAY_BINDED_LAYOUT);
        return true;
        break;
    default:
        return false;
        break;
    }
    return false;//不接管消息
}

REGISTER_UI_EVENT_HANDLER(ALIPAY_UNBIND_LAYOUT)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = alipay_unbind_layout_ontouch,
};

static int alipay_unbind_sure_button_ontouch(void *ctr, struct element_touch_event *e)
{
    // 支付宝解绑确认按键
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
        break;
    case ELM_EVENT_TOUCH_UP:
#if TCFG_PAY_ALIOS_ENABLE
#if TCFG_PAY_TRANSITCODE_ENABLE
        extern retval_e alipay_transit_unbind(void);
        alipay_transit_unbind();
#endif
        UNBINDING();
        extern csi_error_t csi_clear_assets(void);
        csi_clear_assets();

#endif /* #if TCFG_PAY_ALIOS_ENABLE */
        ui_hide(ALIPAY_UNBIND_SURE_LAYOUT);
        ui_show(ALIPAY_UNBIND_SUCC_LAYOUT);
        return true;
        break;
    default:
        return false;
        break;
    }
    return false;//不接管消息
}

REGISTER_UI_EVENT_HANDLER(ALIPAY_UNBIND_SURE_YES_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = alipay_unbind_sure_button_ontouch,
};

static int alipay_unbind_no_button_ontouch(void *ctr, struct element_touch_event *e)
{
    // 支付宝解绑取消按键

    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
        break;
    case ELM_EVENT_TOUCH_UP:
        ui_hide(ALIPAY_UNBIND_SURE_LAYOUT);
        ui_show(ALIPAY_UNBIND_LAYOUT);
        return true;
        break;
    default:
        return false;
        break;
    }
    return false;//不接管消息
}

REGISTER_UI_EVENT_HANDLER(ALIPAY_UNBIND_SURE_NO_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = alipay_unbind_no_button_ontouch,
};

static int alipay_unbind_sure_ontouch(void *ctr, struct element_touch_event *e)
{
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_R_MOVE:
        ui_hide(ALIPAY_UNBIND_SURE_LAYOUT);
        ui_show(ALIPAY_UNBIND_LAYOUT);
        return true;
        break;
    default:
        return false;
        break;
    }
    return false;//不接管消息
}

REGISTER_UI_EVENT_HANDLER(ALIPAY_UNBIND_SURE_LAYOUT)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = alipay_unbind_sure_ontouch,
};

static int alipay_unbind_succ_button_ontouch(void *ctr, struct element_touch_event *e)
{
    // 支付宝解绑成功确认按键
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
        break;
    case ELM_EVENT_TOUCH_UP:
        ui_hide(ALIPAY_UNBIND_SUCC_LAYOUT);
        ui_show(ALIPAY_NO_BIND_LAYOUT);
        return true;
        break;
    default:
        return false;
        break;
    }
    return false;//不接管消息
}

REGISTER_UI_EVENT_HANDLER(ALIPAY_UNBIND_SUCC_SURE_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = alipay_unbind_succ_button_ontouch,
};

static void upay_check_pay_time(void *priv)
{
    if (alipay_info->alipay_time_id == 0) {
        return;
    }

    alipay_info->alipay_check_cnt ++;
    if (alipay_info->alipay_check_cnt > ALIPAY_REFRESH_TIME_OUT_CNT) {
        alipay_info->alipay_check_cnt = 0;
        log_info("refresh\n");
        u32 msg_len = ALIPAY_QRCODE_BUF_LEN;
        memset(alipay_QRcode_mess, 0, ALIPAY_QRCODE_BUF_LEN);
        GET_PAYCODE((uint8_t *)alipay_QRcode_mess, (uint32_t *) &msg_len); //获取支付码
        ui_hide(ALIPAY_PAY_QR_LAYOUT);
        ui_hide(ALIPAY_PAY_BAR_LAYOUT);
        if (alipay_info->alipay_show_barcode == 0) {
            ui_show(ALIPAY_PAY_QR_LAYOUT);
        } else {
            ui_show(ALIPAY_PAY_BAR_LAYOUT);
        }
    }
}

static int alipay_pay_qrcode_layout_ontouch(void *ctr, struct element_touch_event *e)
{
    switch (e->event) {
    case ELM_EVENT_TOUCH_R_MOVE:
        ui_hide(ALIPAY_PAY_QR_LAYOUT);
        ui_show(ALIPAY_BINDED_LAYOUT);
        return true;
        break;
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_UP:
        break;
    default:
        return false;
        break;
    }
    return false;//不接管消息
}

REGISTER_UI_EVENT_HANDLER(ALIPAY_PAY_QR_LAYOUT)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = alipay_pay_qrcode_layout_ontouch,
};

static int alipay_pay_qrcode_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    // 支付宝支付二维码显示
    struct element *elm = (struct element *)_ctrl;
    struct draw_context *dc = (struct draw_context *)arg;
    struct rect rect = {0};

    switch (event) {
    case ON_CHANGE_INIT:
        ui_qrcode_init();

        if (!alipay_qr_code) {
            alipay_qr_code = zalloc(sizeof(struct _QR_CODE));
            if (!alipay_qr_code) {
                ASSERT(0, "malloc err!");
                return -ENOMEM;
            }
        }
        // 初始化部分参数
        alipay_qr_code->code128_mode = 60;
        alipay_qr_code->qr_version = 3;
        alipay_qr_code->qr_max_version = 8;
        alipay_qr_code->qr_ecc_level = 2;
        alipay_qr_code->qr_code_max_input_len = 384;
        alipay_qr_code->qr_buf_size = 4096;
        alipay_qr_code->img_w = 260;

        printf("%s,%d \n", __func__, __LINE__);
#if TCFG_PAY_ALIOS_ENABLE
        alipay_info->alipay_check_cnt = 0;
        alipay_info->alipay_check_status = 0;
        alipay_info->alipay_show_barcode = 0;
        ui_auto_shut_down_disable();
        u32 msg_len = ALIPAY_QRCODE_BUF_LEN;
        memset(alipay_QRcode_mess, 0, ALIPAY_QRCODE_BUF_LEN);
        GET_PAYCODE((uint8_t *)alipay_QRcode_mess, &msg_len);   //获取支付码
        if (!alipay_info->alipay_time_id) {
            alipay_info->alipay_time_id = sys_timer_add(NULL, upay_check_pay_time, ALIPAY_CHECK_TIME_MS);
        }
#endif
        jl_qr_code_process(alipay_QRcode_mess, strlen(alipay_QRcode_mess) + 1, alipay_qr_code);
        alipay_qr_code->code_draw.color565 = _RGB565(255, 255, 255);
        break;
    case ON_CHANGE_SHOW:
        /*根据需要调用以下三个接口:ui_remove_backcolor,ui_remove_backimage,ui_remove_border*/
        /* ui_remove_backcolor(elm);//移除控件背景颜色 */
        /* ui_remove_backimage(elm);//移除控件背景图像 */
        /* ui_remove_border(elm);//移除控件边界 */
        break;
    case ON_CHANGE_SHOW_POST:
        ui_core_get_element_abs_rect(elm, &rect); //跟随控件移动,注释掉这句则不跟随控件移动
        int draw_width = (rect.width > rect.height) ? rect.height : rect.width;
        if (draw_width <= alipay_qr_code->img_w) {
            ASSERT(0);
        }
        alipay_qr_code->code_draw.ratio_w = 0.8f * (float)alipay_qr_code->img_w / alipay_qr_code->out_size;
        alipay_qr_code->code_draw.ratio_h = 0.8f * (float)alipay_qr_code->img_w / alipay_qr_code->out_size;

        ui_draw(dc,
                NULL,
                rect.left + (draw_width - alipay_qr_code->img_w) / 2 + 40,
                rect.top + (draw_width - alipay_qr_code->img_w) / 2 + 30,
                alipay_qr_code->img_w,
                alipay_qr_code->img_w,
                ui_qr_code_draw_gpu,
                (void *)alipay_qr_code,
                sizeof(struct _QR_CODE),
                0);
        break;

    case ON_CHANGE_RELEASE:
        if (row_img) {
            free(row_img);
            row_img = 0;
        }
        if (copy_row_img) {
            free(copy_row_img);
            copy_row_img = 0;
        }
        if (alipay_qr_code && alipay_qr_code->code_draw.data) {
            free(alipay_qr_code->code_draw.data);
            alipay_qr_code->code_draw.data = NULL;
        }
        if (alipay_qr_code) {
            free(alipay_qr_code);
            alipay_qr_code = NULL;
        }
        if (jl_code_param) {
            free(jl_code_param);
            jl_code_param = NULL;
        }
        ui_qrcode_uninit();
        if (alipay_info && alipay_info->alipay_time_id) {
            sys_timer_del(alipay_info->alipay_time_id);
            alipay_info->alipay_time_id = 0;
        }
        log_info("qr_code release:");
        break;
    default:
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ALIPAY_PAY_QR_DRAW_LAYOUT)
.onchange = alipay_pay_qrcode_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int alipay_pay_switch_barcode_button_ontouch(void *ctr, struct element_touch_event *e)
{
    // 支付码切换条形码
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
        break;
    case ELM_EVENT_TOUCH_UP:
        alipay_info->alipay_show_barcode = 1;
        ui_hide(ALIPAY_PAY_QR_LAYOUT);
        ui_show(ALIPAY_PAY_BAR_LAYOUT);
        return true;
        break;
    default:
        return false;
        break;
    }
    return false;//不接管消息
}

REGISTER_UI_EVENT_HANDLER(ALIPAY_PAY_SWITCH_BARCODE_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = alipay_pay_switch_barcode_button_ontouch,
};

static int alipay_pay_barcode_layout_ontouch(void *ctr, struct element_touch_event *e)
{
    switch (e->event) {
    case ELM_EVENT_TOUCH_R_MOVE:
        ui_hide(ALIPAY_PAY_BAR_LAYOUT);
        ui_show(ALIPAY_BINDED_LAYOUT);
        return true;
        break;
    case ELM_EVENT_TOUCH_DOWN:
        break;
    case ELM_EVENT_TOUCH_UP:
        break;
    default:
        return false;
        break;
    }
    return false;//不接管消息
}

REGISTER_UI_EVENT_HANDLER(ALIPAY_PAY_BAR_LAYOUT)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = alipay_pay_barcode_layout_ontouch,
};

static int alipay_pay_barcode_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct element *elm = (struct element *)_ctrl;
    struct draw_context *dc = (struct draw_context *)arg;
    struct rect rect = {0};

    switch (event) {
    case ON_CHANGE_INIT:
        ui_qrcode_init();

        if (!alipay_bar_code) {
            alipay_bar_code = zalloc(sizeof(struct _BAR_CODE));
            if (!alipay_bar_code) {
                ASSERT(0, "malloc err!");
                return -ENOMEM;
            }
        }
        // 初始化部分参数
        alipay_bar_code->code128_mode = 0;
        alipay_bar_code->img_w = 260;
        alipay_bar_code->img_h = 120;

        printf("%s,%d \n", __func__, __LINE__);
#if TCFG_PAY_ALIOS_ENABLE
        alipay_info->alipay_check_cnt = 0;
        alipay_info->alipay_check_status = 0;
        alipay_info->alipay_show_barcode = 1;
        ui_auto_shut_down_disable();
        u32 msg_len = ALIPAY_QRCODE_BUF_LEN;
        memset(alipay_QRcode_mess, 0, ALIPAY_QRCODE_BUF_LEN);
        GET_PAYCODE((uint8_t *)alipay_QRcode_mess, &msg_len);   //获取支付码
        if (!alipay_info->alipay_time_id) {
            alipay_info->alipay_time_id = sys_timer_add(NULL, upay_check_pay_time, ALIPAY_CHECK_TIME_MS);
        }
#endif
        jl_bar_code_process(alipay_QRcode_mess, strlen(alipay_QRcode_mess), alipay_bar_code);
        alipay_bar_code->code_draw.color565 = _RGB565(0, 0, 0);
        break;
    case ON_CHANGE_SHOW:
        /*根据需要调用以下三个接口:ui_remove_backcolor,ui_remove_backimage,ui_remove_border*/
        /* ui_remove_backcolor(elm);//移除控件背景颜色 */
        /* ui_remove_backimage(elm);//移除控件背景图像 */
        /* ui_remove_border(elm);//移除控件边界 */
        break;
    case ON_CHANGE_SHOW_POST:
        ui_core_get_element_abs_rect(elm, &rect); //跟随控件移动,注释掉这句则不跟随控件移动
        if (rect.width <= alipay_bar_code->img_w) {
            ASSERT(0);
        }
        if (rect.height <= alipay_bar_code->img_h) {
            ASSERT(0);
        }
        alipay_bar_code->code_draw.ratio_w = 0.8f * (float)alipay_bar_code->img_w / alipay_bar_code->code_draw.data_w;
        alipay_bar_code->code_draw.ratio_h = 2.0f * (float)alipay_bar_code->img_h / alipay_bar_code->code_draw.data_h;

        ui_draw(dc,
                NULL,
                rect.left + (rect.width - alipay_bar_code->img_w) / 2 + 30,
                rect.top + (rect.height - alipay_bar_code->img_h) / 2,
                alipay_bar_code->img_w,
                alipay_bar_code->img_h,
                ui_bar_code_draw_gpu,
                (void *)alipay_bar_code,
                sizeof(struct _BAR_CODE),
                0);
        break;

    case ON_CHANGE_RELEASE:
        if (row_img) {
            free(row_img);
            row_img = 0;
        }
        if (copy_row_img) {
            free(copy_row_img);
            copy_row_img = 0;
        }
        if (alipay_bar_code && alipay_bar_code->code_draw.data) {
            free(alipay_bar_code->code_draw.data);
            alipay_bar_code->code_draw.data = NULL;
        }
        if (alipay_bar_code) {
            free(alipay_bar_code);
            alipay_bar_code = NULL;
        }
        if (jl_code_param) {
            free(jl_code_param);
            jl_code_param = NULL;
        }
        ui_qrcode_uninit();
        if (alipay_info && alipay_info->alipay_time_id) {
            sys_timer_del(alipay_info->alipay_time_id);
            alipay_info->alipay_time_id = 0;
        }
        log_info("qr_code release:");
        break;
    default:
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ALIPAY_PAY_BAR_DRAW_LAYOUT)
.onchange = alipay_pay_barcode_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int alipay_barcode_number_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_number *number = (struct ui_number *)_ctrl;
    struct unumber param = {0};

    switch (event) {
    case ON_CHANGE_INIT:
        printf("%s,%d \n", __func__, __LINE__);
        printf("str:%s \n", alipay_QRcode_mess);
        param.type = TYPE_STRING;
        param.num_str = (u8 *)alipay_QRcode_mess;
        ui_number_update(number, &param);
        break;
    default:
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ALIPAY_PAY_BARNUM_TEXT)
.onchange = alipay_barcode_number_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int alipay_pay_switch_qrcode_button_ontouch(void *ctr, struct element_touch_event *e)
{
    // 支付码切换二维码
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
        break;
    case ELM_EVENT_TOUCH_UP:
        alipay_info->alipay_show_barcode = 0;
        ui_hide(ALIPAY_PAY_BAR_LAYOUT);
        ui_show(ALIPAY_PAY_QR_LAYOUT);
        return true;
        break;
    default:
        return false;
        break;
    }
    return false;//不接管消息
}

REGISTER_UI_EVENT_HANDLER(ALIPAY_PAY_SWITCH_QR_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = alipay_pay_switch_qrcode_button_ontouch,
};



//===================乘车码========================//
#if TCFG_PAY_TRANSITCODE_ENABLE
#include "alipay_common.h"

extern void set_cardcode_no_release();
extern u8 cardcode_is_need_release();
extern void clear_cardcode_no_release();
extern void transitcode_kill();
extern u8 net_get_dhcp_flag();
extern char *alipay_transit_name_get_by_index(int index);
static int close_file_handler();
static int brow_children_redraw(int id);
extern int local_transitcode_get_qr_data(int (*callback)(int, int, int, u8 *, int));

static struct _QR_CODE *alipay_transit_qr_code = NULL;

struct grid_set_info {
    int flist_index;  //文件列表首项所指的索引
    int cur_total;
    FILE *file;
    struct vfscan *fs;
    FS_DIR_INFO *dir_buf;
    int    show_temp;
#if (TCFG_LFN_EN)
    u8  lfn_buf[512];
#endif//TCFG_LFN_EN
};

static struct grid_set_info *ghandler = NULL;
#define __this 	(ghandler)
#define sizeof_this     (sizeof(struct grid_set_info))



static int qr_msg_info_handler(const char *type, u32 arg)
{
    printf("_func_  == %s %d err %d\n", __func__, __LINE__, arg);
    struct element *elm = ui_core_get_element_by_id(ALIPAY_TRANSIT_DRAW_LAYOUT);
    if (!elm) {
        return -1;
    }
    return ui_core_redraw(elm);
}



//0直接显示二维码
//1显示卡片列表
//2显示进度
//3显示选择界面
//4城市卡片获取失败
//5二维码卡片获取失败
//6乘车码未开通
int ui_show_transitcodes_list(const char *type, u32 arg)
{
    if (!strcmp(type, "event")) {
        u32 rets;
        __asm__ volatile("%0 = rets":"=r"(rets));
        printf("__func__ %s %x %d\n", __func__, rets, arg);
        extern void set_cardcode_no_release();
        set_cardcode_no_release();//主动行为告诉ui不用释放线程
        alipay_info->ui_transitcode_flag = arg;
        switch (arg) {
        case 0:
            //UI_WINDOW_BACK_SPEC_SHOW(PAGE_101);
            ui_hide(ALIPAY_TRANSIT_LOADING_LAYOUT);
            ui_show(ALIPAY_TRANSIT_LAYOUT);
            break;
        case 1:
            //UI_WINDOW_BACK_SPEC_SHOW(PAGE_100);
            ui_hide(ALIPAY_TRANSIT_LOADING_LAYOUT);
            ui_show(ALIPAY_TRANSIT_LIST_LAYOUT);
            break;
        case 2:
            //UI_WINDOW_BACK_SPEC_SHOW(PAGE_102);
            ui_show(ALIPAY_TRANSIT_LOADING_LAYOUT);
            break;
        case 3:
            //UI_WINDOW_BACK_SPEC_SHOW(PAGE_92);
            ui_hide(ALIPAY_TRANSIT_LOADING_LAYOUT);
            ui_show(ALIPAY_BINDED_LAYOUT);
            break;
        case 4:
        case 5:
        case 6:
            //UI_WINDOW_BACK_SPEC_SHOW(PAGE_102);
            ui_hide(ALIPAY_TRANSIT_LOADING_LAYOUT);
            ui_hide(ALIPAY_TRANSIT_LAYOUT);
            ui_show(ALIPAY_TRANSIT_ERROR_LAYOUT);
            break;
        }
    }
    return 0;
}

static int open_file_handler(int show_temp)
{
    extern int alipay_tansit_num_get();
    __this = zalloc(sizeof_this);
    __this->cur_total = alipay_tansit_num_get();
    if (!__this->cur_total) {
        return -1;
    }
    return 0;

}

static int close_file_handler()
{

    if (!__this) {
        return -1;
    }

    free(__this);
    __this = NULL;
    return 0;
}

static int file_select_enter(u32 index)
{
    char *info = alipay_transit_name_get_by_index(index);
    if (!info) {
        return -1;
    }
    if (!__this || !__this->cur_total) {
        return -1;
    }
    extern int local_transitcode_set_default_card_info_by_index(int index);
    local_transitcode_set_default_card_info_by_index(index);

    //0直接显示二维码
    //1显示卡片列表
    //2显示进度
    ui_hide(ALIPAY_TRANSIT_LIST_LAYOUT);
    if (!net_get_dhcp_flag()) {
        UI_MSG_POST("transit_show:event=%4", 2);
        return 0;
    }
    UI_MSG_POST("transit_show:event=%4", 0);
    /* ui_show_transitcodes_list(0); */
    return 0;
}

static int grid_child_cb(void *_ctrl, int id, int type, int index)
{
    char *info = alipay_transit_name_get_by_index(index);
    switch (type) {
    case CTRL_TYPE_PROGRESS:
        break;
    case CTRL_TYPE_MULTIPROGRESS:
        break;
    case CTRL_TYPE_TEXT:
        struct ui_text *text = (struct ui_text *)_ctrl;
        if (!strcmp(text->source, "title")) {
            text->elm.css.invisible = !!index;
            break;
        }
        if (!info) {
            return 0;
        }
        if (!strcmp(text->source, "name")) {
            text->attrs.offset = 0;
            text->attrs.format = UI_TEXT_ENCODE_TEXT;
            text->attrs.flags  = FONT_DEFAULT;
            if (info) {
                text->attrs.encode = FONT_ENCODE_UTF8;
                text->attrs.str    = info;
                text->attrs.strlen = strlen(info);
            }
            text->elm.css.invisible = 0;
        }
        break;
    case CTRL_TYPE_NUMBER:
        struct ui_number *number = (struct ui_number *)_ctrl;
        break;
    case CTRL_TYPE_PIC:
        struct ui_pic *pic = (struct ui_pic *)_ctrl;
        break;
    case CTRL_TYPE_TIME:
        break;
    }
    return 0;
}


static int alipay_transitcode_file_switch_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    int sel_item;
    switch (e->event) {
    case ELM_EVENT_TOUCH_R_MOVE:
        extern int alipay_tansit_num_get();
        set_cardcode_no_release();//主动行为不用释放
        ui_hide(ALIPAY_TRANSIT_LIST_LAYOUT);
        UI_MSG_POST("transit_show:event=%4", 3);
        /* ui_show_transitcodes_list(3); */
        return TRUE;

    case ELM_EVENT_TOUCH_L_MOVE:
        log_info("line:%d", __LINE__);
        break;
    case ELM_EVENT_TOUCH_MOVE:
        if (!__this) {
            return TRUE;
        }
        return false;
        break;
    case ELM_EVENT_TOUCH_DOWN:
        return false;//不接管消息
        break;
    case ELM_EVENT_TOUCH_UP:
        if (e->move_flag) {
            return false;//不接管消息
        }
        sel_item = ui_grid_cur_item_dynamic(grid);
        file_select_enter(sel_item);
        return false;//不接管消息

        break;
    default:
        return false;
        break;
    }
    return false ;//不接管
    //return true;//接管消息
}

static int brows_children_init(struct ui_grid *grid)
{
    struct element *k;
    int count =  0;//
    if (!grid) {
        return 0;
    }

    if (__this) {
        count += __this->cur_total;
    }

    for (int i = 0; i < grid->avail_item_num; i++) {
        if (i < count) {
            /* list_for_each_child_element(k, &grid->item[i].elm) { */
            /* grid_child_cb(k, k->id, ui_id2type(k->id), i); */
            /* } */
            grid->item[i].elm.css.invisible = 0;
        } else {
            grid->item[i].elm.css.invisible = 1;
        }
    }
    return 0;
}

static int alipay_browse_enter_child_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct element *k ;
    struct element *elm = (struct element *)_ctrl;
    if (event == ON_CHANGE_UPDATE_ITEM) {
        int index = (u32)arg;
        grid_child_cb(elm, elm->id, ui_id2type(elm->id), index);

    }
    return 0;
}

static int browse_enter_child_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct element *k ;
    struct element *elm = (struct element *)_ctrl;
    if (event == ON_CHANGE_UPDATE_ITEM) {
        int index = (u32)arg;
        grid_child_cb(elm, elm->id, ui_id2type(elm->id), index);

    }
    return 0;
}

static int alipay_transitcode_browse_enter_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    int fnum = 0;

    switch (e) {
    case ON_CHANGE_INIT:
        ui_grid_set_slide_direction(grid, SCROLL_DIRECTION_UD);
        /* struct scroll_area area = {0, 0, 10000, 10000}; */
        /* ui_grid_set_scroll_area(grid, &area); */
        /* ui_grid_flick_ctrl_close(grid, 1); */
        clear_cardcode_no_release();//进去时候清标
        open_file_handler(grid->avail_item_num);
        {
            int row = __this->cur_total;
            int col = 1;
            ui_set_default_handler(&grid->elm, NULL, NULL, browse_enter_child_onchange);
            ui_grid_init_dynamic(grid, &row, &col);

        }
        break;
    case ON_CHANGE_RELEASE:
        close_file_handler();
        ui_set_default_handler(&grid->elm, NULL, NULL, NULL);
        //判断是否需要释放任务
        if (cardcode_is_need_release()) {
            transitcode_kill();
            ui_auto_shut_down_enable();//开启自动灭屏
        }
        clear_cardcode_no_release();
        //清标
        break;
    case ON_CHANGE_FIRST_SHOW:
        break;
    default:
        return false;
    }
    return false;
}


REGISTER_UI_EVENT_HANDLER(ALIPAY_TRANSIT_VLIST)
.onchange = alipay_transitcode_browse_enter_onchange,
 .onkey = NULL,
  .ontouch = alipay_transitcode_file_switch_ontouch,
};

static int alipay_transitcode_list_layout_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        close_file_handler();
        //判断是否需要释放任务
        if (cardcode_is_need_release()) {
            transitcode_kill();
            ui_auto_shut_down_enable();//开启自动灭屏
        }
        clear_cardcode_no_release();
        //清标
        break;
    case ON_CHANGE_FIRST_SHOW:
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ALIPAY_TRANSIT_LIST_LAYOUT)
.onchange = alipay_transitcode_list_layout_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int qr_get_callback(int event, int err, int num, u8 *data, int len)
{
    printf("%s,%d err %d\n", __func__, __LINE__, err);
    if (!err) {
        printf("%s %d\n", data, len);
        put_buf(data, len);
        //释放的原因是里面会重新申请
        if (row_img) {
            free(row_img);
            row_img = 0;
        }
        if (copy_row_img) {
            free(copy_row_img);
            copy_row_img = 0;
        }
        if (alipay_transit_qr_code && alipay_transit_qr_code->code_draw.data) {
            free(alipay_transit_qr_code->code_draw.data);
            alipay_transit_qr_code->code_draw.data = NULL;
        }
        jl_qr_code_process((char *)data, len, alipay_transit_qr_code);
        if (alipay_info) {
            alipay_info->transit_qr_ok = 1;//在这之前ui不会操作二维码buf
        }
        UI_MSG_POST("transitcode_finsh:err=%4", err);
    } else {
        //过期情况
        if (err == RV_CARD_DATA_OVERDUE) {
            if (net_get_dhcp_flag()) {
                return 0;//带网络情况下可以更新
            }
        }
        //0直接显示二维码
        //1显示卡片列表
        //2显示进度
        //3显示选择界面
        //4城市卡片获取失败
        //5二维码卡片获取失败
        //6乘车码未开通
        UI_MSG_POST("transit_show:event=%4", 5);
        /* ui_show_transitcodes_list(5); */
        return true;//不带带网络情况退界面
    }
    return 0;
}


static int alipay_sel_transitcode_layout_onchange(void *_ctrl, enum element_change_event event, void *arg)
{

    struct element *elm = (struct element *)_ctrl;
    switch (event) {
    case ON_CHANGE_INIT:
        clear_cardcode_no_release();//进去前必须要清
        break;
    case ON_CHANGE_RELEASE:
        /* if (cardcode_is_need_release()) { //不是自主行为 需要释放任务 */
        /*     transitcode_kill(); */
        /*     ui_auto_shut_down_enable();//开启自动灭屏 */
        /* } */
        /* clear_cardcode_no_release();//出去前也清 */
        break;
    default:
        break;
    }
    return FALSE;
}

static int alipay_sel_transitcode_layout_ontouch(void *ctr, struct element_touch_event *e)
{
    // 支付宝设置页面的乘车码layout
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
        break;
    case ELM_EVENT_TOUCH_UP:
        set_cardcode_no_release();//主动行为
        //UI_HIDE_CURR_WINDOW();
        ui_hide(ALIPAY_BINDED_LAYOUT);
        ui_auto_shut_down_disable();
        extern int local_transitcode_get_default_card_info();
        local_transitcode_get_default_card_info();
        return true;
        break;
    default:
        return false;
        break;
    }
    return false;//不接管消息
}
REGISTER_UI_EVENT_HANDLER(ALIPAY_SEL_TRANSIT_LAYOUT)
.onchange = alipay_sel_transitcode_layout_onchange,
 .onkey = NULL,
  .ontouch = alipay_sel_transitcode_layout_ontouch,
};

static int alipay_transitcode_qrcode_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct element *elm = (struct element *)_ctrl;
    struct draw_context *dc = (struct draw_context *)arg;
    struct rect rect = {0};

    switch (event) {
    case ON_CHANGE_INIT:
        printf("%s,%d \n", __func__, __LINE__);

        transit_param_can_release = 0;
        ui_auto_shut_down_disable();
        if (!alipay_transit_qr_code) {
            alipay_transit_qr_code = zalloc(sizeof(struct _QR_CODE));
            if (!alipay_transit_qr_code) {
                ASSERT(0, "malloc err!");
                return -ENOMEM;
            }
        }
        // 初始化部分参数
        alipay_transit_qr_code->code128_mode = 60;
        alipay_transit_qr_code->qr_version = 3;
        alipay_transit_qr_code->qr_max_version = 16;
        alipay_transit_qr_code->qr_ecc_level = 2;
        alipay_transit_qr_code->qr_code_max_input_len = 512;
        alipay_transit_qr_code->qr_buf_size = 4096 + 2048 + 1024;
        alipay_transit_qr_code->img_w = 260;

        alipay_info->transit_qr_ok = 0;
        local_transitcode_get_qr_data(qr_get_callback);
        alipay_transit_qr_code->code_draw.color565 = _RGB565(255, 255, 255);
        printf("%s,%d \n", __func__, __LINE__);
        break;
    case ON_CHANGE_SHOW:
        /*根据需要调用以下三个接口:ui_remove_backcolor,ui_remove_backimage,ui_remove_border*/
        /* ui_remove_backcolor(elm);//移除控件背景颜色 */
        /* ui_remove_backimage(elm);//移除控件背景图像 */
        /* ui_remove_border(elm);//移除控件边界 */
        break;
    case ON_CHANGE_SHOW_POST:
        ui_core_get_element_abs_rect(elm, &rect); //跟随控件移动,注释掉这句则不跟随控件移动
        if (alipay_info && alipay_info->transit_qr_ok) {
            int draw_width = (rect.width > rect.height) ? rect.height : rect.width;

            if (draw_width <= alipay_transit_qr_code->img_w) {
                ASSERT(0);
            }
            alipay_transit_qr_code->code_draw.ratio_w = 0.8f * (float)alipay_transit_qr_code->img_w / alipay_transit_qr_code->out_size;
            alipay_transit_qr_code->code_draw.ratio_h = 0.8f * (float)alipay_transit_qr_code->img_w / alipay_transit_qr_code->out_size;
            ui_draw(dc,
                    NULL,
                    rect.left + (draw_width - alipay_transit_qr_code->img_w) / 2 + 55,
                    rect.top + (draw_width - alipay_transit_qr_code->img_w) / 2 + 20,
                    alipay_transit_qr_code->img_w,
                    alipay_transit_qr_code->img_w,
                    ui_qr_code_draw_gpu,
                    (void *)alipay_transit_qr_code,
                    sizeof(struct _QR_CODE),
                    0);
        }
        break;

    case ON_CHANGE_RELEASE:
        if (row_img) {
            free(row_img);
            row_img = 0;
        }
        if (copy_row_img) {
            free(copy_row_img);
            copy_row_img = 0;
        }
        if (alipay_transit_qr_code && alipay_transit_qr_code->code_draw.data) {
            free(alipay_transit_qr_code->code_draw.data);
            alipay_transit_qr_code->code_draw.data = NULL;
        }
        if (alipay_transit_qr_code) {
            free(alipay_transit_qr_code);
            alipay_transit_qr_code = NULL;
        }
        if (jl_code_param) {
            free(jl_code_param);
            jl_code_param = NULL;
        }
        log_info("qr_code release:");
        transit_param_can_release = 1;
        if (alipay_info) {
            alipay_info->transit_qr_ok = 0;
        }
        ui_auto_shut_down_enable();
        break;
    default:
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ALIPAY_TRANSIT_DRAW_LAYOUT)
.onchange = alipay_transitcode_qrcode_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};


static int alipay_transitcode_qr_layout_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    // 支付宝乘车码二维码layout

    struct element *elm = (struct element *)_ctrl;
    switch (event) {
    case ON_CHANGE_INIT:
        /* ui_register_msg_handler(ID_WINDOW_ALIPAY, ui_msg_handler);//注册消息交互的回调 */
        clear_cardcode_no_release();//进去前必须要清
        break;
    case ON_CHANGE_RELEASE:
        if (cardcode_is_need_release()) {
            transitcode_kill();
            ui_auto_shut_down_enable();//开启自动灭屏
        }
        clear_cardcode_no_release();//出去前也清
        break;
    default:
        break;
    }
    return FALSE;
}

static int alipay_transitcode_qr_layout_ontouch(void *ctr, struct element_touch_event *e)
{
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
        break;

    case ELM_EVENT_TOUCH_UP:
        break;

    case ELM_EVENT_TOUCH_R_MOVE:
        if (!alipay_info->transit_qr_ok) {
            return true;
        }
        jlgpu_scheduler_wait_sync();   // 等待二维码显示完成
        extern int alipay_tansit_num_get();
        set_cardcode_no_release();//主动行为不用释放
        if (alipay_tansit_num_get()) { //有数量
            //0直接显示二维码
            //1显示卡片列表
            //2显示进度
            ui_hide(ALIPAY_TRANSIT_LAYOUT);
            UI_MSG_POST("transit_show:event=%4", 1);
            /* ui_show_transitcodes_list(1); */
        } else {
            ui_hide(ALIPAY_TRANSIT_LAYOUT);
            extern int local_transitcode_cardlist_get();
            local_transitcode_cardlist_get();//获取卡片再显示
        }

        return true;

        break;
    default:
        return false;
        break;
    }
    return false;//不接管消息
}

REGISTER_UI_EVENT_HANDLER(ALIPAY_TRANSIT_LAYOUT)
.onchange = alipay_transitcode_qr_layout_onchange,
 .onkey = NULL,
  .ontouch = alipay_transitcode_qr_layout_ontouch,
};


static int alipay_transitcode_button_ontouch(void *ctr, struct element_touch_event *e)
{
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
        break;

    case ELM_EVENT_TOUCH_UP:
        if (!alipay_info->transit_qr_ok) {    // 二维码没加载出来前不能返回
            return true;
        }
        extern int alipay_tansit_num_get();
        set_cardcode_no_release();//主动行为不用释放
        if (alipay_tansit_num_get()) { //有数量
            //0直接显示二维码
            //1显示卡片列表
            //2显示进度
            ui_hide(ALIPAY_TRANSIT_LAYOUT);
            UI_MSG_POST("transit_show:event=%4", 1);
            /* ui_show_transitcodes_list(1); */
        } else {
            ui_hide(ALIPAY_TRANSIT_LAYOUT);
            extern int local_transitcode_cardlist_get();
            local_transitcode_cardlist_get();//获取卡片再显示
        }

        return true;

        break;
    default:
        return false;
        break;
    }
    return false;//不接管消息
}
static int alipay_transitcode_button_onchange(void *_ctrl, enum element_change_event event, void *arg)
{
    struct ui_text *text = (struct ui_text *)_ctrl;

    switch (event) {
    case ON_CHANGE_INIT:
        extern char *alipay_transit_get_global_default_card_name();
        char *name = alipay_transit_get_global_default_card_name();
        ui_text_set_text_attrs(text, name, strlen(name), FONT_ENCODE_UTF8, 0, FONT_DEFAULT);
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        break;
    }
    return FALSE;
}

REGISTER_UI_EVENT_HANDLER(ALIPAY_TRANSIT_BUTTON)
.onchange = alipay_transitcode_button_onchange,
 .onkey = NULL,
  .ontouch = alipay_transitcode_button_ontouch,
};

static int alipay_transitcode_flash_buttono_ontouch(void *ctr, struct element_touch_event *e)
{
    // 支付宝乘车码刷新按键
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
        break;
    case ELM_EVENT_TOUCH_UP:
        if (!alipay_info->transit_qr_ok) {    // 二维码没加载出来前不能刷新
            return true;
        }
        alipay_info->transit_qr_ok = 0;
        if (!alipay_transit_qr_code) {
            alipay_transit_qr_code = zalloc(sizeof(struct _QR_CODE));
            if (!alipay_transit_qr_code) {
                ASSERT(0, "malloc err!");
                return -ENOMEM;
            }
        }

        // 初始化部分参数
        alipay_transit_qr_code->code128_mode = 60;
        alipay_transit_qr_code->qr_version = 3;
        alipay_transit_qr_code->qr_max_version = 16;
        alipay_transit_qr_code->qr_ecc_level = 2;
        alipay_transit_qr_code->qr_code_max_input_len = 512;
        alipay_transit_qr_code->qr_buf_size = 4096 + 2048 + 1024;
        alipay_transit_qr_code->img_w = 260;

        jlgpu_scheduler_wait_sync();   // 等待二维码显示完成

        if (alipay_transit_qr_code && alipay_transit_qr_code->code_draw.data) {
            free(alipay_transit_qr_code->code_draw.data);
            alipay_transit_qr_code->code_draw.data = NULL;
        }

        local_transitcode_get_qr_data(qr_get_callback);//更新二维码
        return true;
        break;
    default:
        return false;
        break;
    }
    return false;//不接管消息
}
REGISTER_UI_EVENT_HANDLER(ALIPAY_TRANSIT_REFLASH_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = alipay_transitcode_flash_buttono_ontouch,
};

static int alipay_transitcode_err_button_onotuch(void *ctr, struct element_touch_event *e)
{
    printf("__FUNCTION__ = %s %d\n", __FUNCTION__, e->event);
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        return true;
    case ELM_EVENT_TOUCH_UP:
        //3显示选择界面
        ui_hide(ALIPAY_TRANSIT_ERROR_LAYOUT);
        UI_MSG_POST("transit_show:event=%4", 3);
        /* ui_show_transitcodes_list(3); */
        return true;
        break;
    default:
        break;
    }
    return false;

}

REGISTER_UI_EVENT_HANDLER(ALIPAY_TRANSIT_ERROR_SURE_BUTTON)
.onchange = NULL,
 .onkey = NULL,
  .ontouch = alipay_transitcode_err_button_onotuch,
};

static int alipay_transitcode_err_layout_onchane(void *ctr, enum element_change_event e, void *arg)
{
    struct element *elm = (struct element *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        printf("%s %d\n", __FUNCTION__, __LINE__);
        clear_cardcode_no_release();//进去前必须要清
        //0直接显示二维码
        //1显示卡片列表
        //2显示进度
        //3显示选择界面
        //4城市卡片获取失败
        //5二维码卡片获取失败
        //6乘车码未开通
        //7未知错误

        if (alipay_info->ui_transitcode_flag == 2) {
            elm->css.invisible = 1;
        }
        break;
    case ON_CHANGE_RELEASE:
        //判断是否需要释放任务
        if (cardcode_is_need_release()) {
            transitcode_kill();
            ui_auto_shut_down_enable();//开启自动灭屏
        }
        clear_cardcode_no_release();
        //清标
        break;
    case ON_CHANGE_FIRST_SHOW:
        printf("%s %d\n", __FUNCTION__, __LINE__);
        struct ui_text *text = ui_text_for_id(ALIPAY_TRANSIT_ERROR_TEXT);
        if (text) {
            if (alipay_info->ui_transitcode_flag == 4) {
                ui_text_set_index(text, 0);
            }
            if (alipay_info->ui_transitcode_flag == 6) {
                ui_text_set_index(text, 2);
            }
            if (alipay_info->ui_transitcode_flag == 5) {
                ui_text_set_index(text, 1);
            }
            if (alipay_info->ui_transitcode_flag == 7) {
                ui_text_set_index(text, 3);
            }
        }
        break;
    default:
        return false;
    }
    return false;
}

static int alipay_transitcode_err_layout_ontouch(void *ctr, struct element_touch_event *e)
{
    printf("__FUNCTION__ = %s %d\n", __FUNCTION__, e->event);
    switch (e->event) {
    case ELM_EVENT_TOUCH_R_MOVE:
        //3显示选择界面
        ui_hide(ALIPAY_TRANSIT_ERROR_LAYOUT);
        UI_MSG_POST("transit_show:event=%4", 3);
        /* ui_show_transitcodes_list(3); */
        return true;
        break;
    default:
        break;
    }
    return false;

}
REGISTER_UI_EVENT_HANDLER(ALIPAY_TRANSIT_ERROR_LAYOUT)
.onchange = alipay_transitcode_err_layout_onchane,
 .onkey = NULL,
  .ontouch = alipay_transitcode_err_layout_ontouch,
};

void pic_rorate_cb(void *priv)
{
    struct ui_pic *pic = ui_pic_for_id(TRANSIT_LOADING_PIC);
    if (pic) {
        alipay_info->rorate_cnt++;
        ui_core_set_element_rotate(pic, 81, 81, 167, 181, 36 * alipay_info->rorate_cnt, true);
        ui_core_redraw(pic);
    }
    if (alipay_info && alipay_info->rorate_cnt > 75) {     // 15s
        ui_hide(ALIPAY_TRANSIT_LOADING_LAYOUT);
        ui_show(ALIPAY_TRANSIT_ERROR_LAYOUT);
        if (alipay_info && alipay_info->transit_loading_timer) {
            sys_timer_del(alipay_info->transit_loading_timer);
            alipay_info->transit_loading_timer = 0;
        }
    }
}

static int alipay_transitcode_loading_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct element *elm = (struct element *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        printf("%s %d\n", __FUNCTION__, __LINE__);
        ui_auto_shut_down_disable();
        //0直接显示二维码
        //1显示卡片列表
        //2显示进度
        //3显示选择界面
        //4城市卡片获取失败
        //5二维码卡片获取失败
        //6乘车码未开通
        //7未知错误
        if (alipay_info->ui_transitcode_flag != 2) {
            elm->css.invisible = 1;
        }

        if (!alipay_info->transit_loading_timer) {
            alipay_info->rorate_cnt = 0;
            alipay_info->transit_loading_timer = sys_timer_add(NULL, pic_rorate_cb, 200);
        }

        break;
    case ON_CHANGE_RELEASE:
        if (alipay_info && alipay_info->transit_loading_timer) {
            sys_timer_del(alipay_info->transit_loading_timer);
            alipay_info->transit_loading_timer = 0;
        }
        ui_auto_shut_down_enable();//开启自动灭屏
        break;
    case ON_CHANGE_FIRST_SHOW:
        break;
    default:
        return false;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(ALIPAY_TRANSIT_LOADING_LAYOUT)
.onchange = alipay_transitcode_loading_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
#endif
#endif
#endif
#endif
