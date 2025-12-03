#ifndef SOFTWARE_JPEGENC_H
#define SOFTWARE_JPEGENC_H

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
void *software_jpegenc_init(int img_width, int img_height, int img_stride, int q_val);

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
int software_jpegenc_line(void *jpgenc_hdl, u8 *out_buf, int out_buf_size, u8 *in_data, int vlen, int voff, int *jpgenc_out_size);

/* ------------------------------------------------------------------------------------*/
/**
 * @brief software_jpegenc_exit jpeg软件编码退出流程，释放内存
 *
 * @param jpgenc_hdl			init时申请的句柄
 *
 */
/* ------------------------------------------------------------------------------------*/
void software_jpegenc_exit(void *jpgenc_hdl);


#endif /* SOFTWARE_JPEGENC_H */

