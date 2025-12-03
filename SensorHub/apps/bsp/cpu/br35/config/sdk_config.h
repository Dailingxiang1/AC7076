#ifndef __APP_CONFIG_H__
#define __APP_CONFIG_H__




//==========================================================//
//                      串口打印使能开关                    //
//==========================================================//
#define CONFIG_UART_DEBUG_ENABLE  				0
#define CONFIG_DEBUG_UART_TX_PIN                IO_PORTB_00
#define CONFIG_DEBUG_UART_BAUD    		        115200L


//==========================================================//
//                 		低功耗触摸                      	//
//==========================================================//

#define CONFIG_LPCTMU_ENABLE       0


//==========================================================//
//                 		SENSOR驱动                      	//
//==========================================================//

#define CONFIG_SENSOR_DRIVER_ENABLE       0
#define CONFIG_SENSOR_SLEEP_ENABLE        0 //自动让SENSOR睡眠，需要SENSOR支持唤醒检测并IO中断唤醒MCU

#define TCFG_SC7A20_ENABLE               0

#define TCFG_MMC5603_ENABLE               0

#define TCFG_VCHR11S_ENABLE               0
#define TCFG_HRS3602_ENABLE               0
#define TCFG_HR_SENSOR_READ_BY_INT        0

//==========================================================//
//                 		soft iic                        	//
//==========================================================//
#define TCFG_SW_I2C0_CLK_PORT      IO_PORTB_02  //spft IIC CLK
#define TCFG_SW_I2C0_DAT_PORT      IO_PORTB_01  //spft IIC DAT
#define TCFG_SW_I2C0_DELAY_CNT     1           //软件IIC延时参数，影响通讯时钟频率



//========================================================== //
//                 		hw  iic                   	        //
//==========================================================//
#define P11_HW_IIC_EN			    0
#define P11_HW_IIC_SCL				IO_PORTB_02
#define P11_HW_IIC_SDA				IO_PORTB_01
#define	P11_HW_IIC_FREQ				(400*1000)
#define P11_HW_IIC_PU_EN			1

#endif
