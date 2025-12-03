#ifndef __GPU_DEMO_H
#define __GPU_DEMO_H

#include "app_config.h"

#if defined GPU_DEMO_ENABLE && GPU_DEMO_ENABLE

#define GPU_DEMO_RAM_565_ENABLE				0			//从ram加载图像显示
#define GPU_DEMO_FLASH_565_ENABLE			0			//从flash加载图像显示
#define GPU_DEMO_RAM_L1_ENABLE				0			//L1格式图像显示
#define GPU_DEMO_RES_ENABLE					0			//
#define GPU_DEMO_COMPRESS_ENABLE			0			//压缩图像显示
#define GPU_DEMO_FOOTBALL_ENABLE			0			//足球demo
#define DEMO_LCD_DUAL_BUF_ENABLE			0			//双buffer推屏demo
#define GPU_DEMO_PERSPECTIVE_ENABLE			0			//透视变换demo
#define JPEG_DEMO_ENABLE					0			//jpeg
#define GPU_DEMO_COLOR_SWAP_ENABLE          0			//图像大小端、交换red-blue等
#define GPU_DEMO_BLEND_ENABLE				0			//8种blend方式
#define GPU_DEMO_ALPHA_PREMULT_ENABLE		0			//alpha图像预乘对比
#define GPU_DEMO_TASK_TEXTURE				0			//图像变换扩边对比
#define GPU_DEMO_OUTPUT_FORMAT				0			//6种输出格式
#define GPU_DEMO_MASK_SWAP					0			//mask 取反、交换大小端
#define GPU_DEMO_AFFINE_ENABLE			    0           //指定仿射变换矩阵进行仿射变换
#define GPU_DEMO_SCALE_INHERIT_ENABLE		0           //背景以及背景上面的图标整体缩放
#define GPU_DEMO_GAUSSIANBLUR               0           //高斯模糊
#define GPU_DEMO_CUSTOM_DRAW				0			//自定义绘图(用户回调)

#define GPU_DEMO_TASK_FILL					0			//GPU纯色矩形填充
#define GPU_DEMO_TASK_FILL_MASK				0			//GPU纯色MASK填充
#define GPU_DEMO_TASK_FILL_AFFINE			0			//GPU纯色矩阵仿射变换
#define GPU_DEMO_TASK_LINEGRAD				0			//GPU线性渐变矩形填充
#define GPU_DEMO_WITH_PSRAM					1			//GPU与PSRAM联动（必须使用带PSRAM的芯片）

#if (GPU_DEMO_TASK_FILL || GPU_DEMO_TASK_FILL_AFFINE || GPU_DEMO_TASK_FILL_MASK \
	|| GPU_DEMO_TASK_LINEGRAD || GPU_DEMO_COLOR_SWAP_ENABLE || GPU_DEMO_BLEND_ENABLE \
	|| GPU_DEMO_ALPHA_PREMULT_ENABLE || GPU_DEMO_TASK_TEXTURE || GPU_DEMO_OUTPUT_FORMAT \
	|| GPU_DEMO_MASK_SWAP || GPU_DEMO_CUSTOM_DRAW || JPEG_DEMO_ENABLE || GPU_DEMO_WITH_PSRAM)
#define GPU_DEMO_TASK_MAIN					1	// GPU任务DEMO共用刷屏逻辑代码
#else
#define GPU_DEMO_TASK_MAIN					0
#endif

#define GPU_DEMO_TEXTURE_INPUT_ENABLE       0   // 纹理填充
#define GPU_DEMO_TEXTURE_AFFINE_ENABLE      0   // 纹理+仿射
#define GPU_DEMO_TEXTURE_PERSPECTIVE_ENABLE 0   // 纹理+透视
#define GPU_DEMO_TEXTURE_CROP_ENABLE        0   // 纹理+裁剪
#define GPU_DEMO_TEXTURE_MASK_ENABLE        0   // 纹理+mask
#define GPU_DEMO_TEXTURE_EDGE_ENABLE        0   // 纹理边缘插值
#define GPU_DEMO_DITHER_ENABLE              0   // dither功能

extern void gpu_demo_run(void);


#endif
#endif


