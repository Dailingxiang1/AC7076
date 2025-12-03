#ifndef BF30A2_CFG_H
#define BF30A2_CFG_H

#include "system/includes.h"

//分辨率设置
#define BF30A2_INPUT_W        240           //摄像头图像宽
#define BF30A2_INPUT_H	      320           //摄像头图像高
//根据帧率调整SPI PCLK 15->12M 30->24M
#define BF30A2_INPUT_FPS      30            //(摄像头实际帧率只有此设置值的一半)帧率

//摄像头裁剪窗口
#define START_ADDRW 0
#define START_ADDRH 0
#define END_ADDRW (BF30A2_INPUT_W + START_ADDRW)
#define END_ADDRH (BF30A2_INPUT_H + START_ADDRH)

//IIC
#define IIC_ID  2                           //IIC ID号
#define IIC_SCL_IO IO_PORTC_03              //IIC SCL
#define IIC_SDA_IO IO_PORTC_10              //IIC SDA
#define DELAY_TIME	10                      //IIC 延时
#define BF30A2_WRCMD 0xdc                   //摄像头IIC写地址
#define BF30A2_RDCMD 0xdd                   //摄像头IIC读地址

//IO
#define CAM_PWR_IO      IO_PORTC_11         //摄像头电源提供
#define SPI_XCLK_PORT   IO_PORTC_00         //摄像头时钟提供

struct reginfo {
    u8 reg;
    u8 val;
};


int bf30a2_check(void);
void bf30a2_config_sensor(void);
void bf30a2_out_sleep(void);
void bf30a2_in_sleep(void);
int bf30a2_io_init(void);
int bf30a2_io_exit(void);


#endif /* BF30A2_CFG_H */

