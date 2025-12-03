#ifndef __ALL_INCLUDE_H__
#define __ALL_INCLUDE_H__

//#include <string.h>
#include "typedef.h"
#include "common.h"
#include "printf.h"

#define UPDATE_GET_POWER_PIN 1  //获取power_pin脚
#define MUTIL_CPU_ENABLE  1 //多芯片升级支持
#define OTA_LOADER_RECORD_RST_INFO_CONFIG  1 //升级记录复位信息
#define OSC_1PIN_STABLE_EN   1 //osc 单脚牵引

//控制起来省空间。
#if defined(CONFIG_CPU_BR28) || defined(CONFIG_CPU_BR27) || defined(CONFIG_CPU_BR29) || defined(CONFIG_CPU_BR35)
#define UPDATE_COMPATIBILITY_EN  1 //升级兼容性处理
#else
#define UPDATE_COMPATIBILITY_EN  0 //升级兼容性处理
#endif

#if defined(CONFIG_CPU_BD49) || defined(CONFIG_CPU_SH58)
#undef UPDATE_GET_POWER_PIN
#undef MUTIL_CPU_ENABLE
#undef OTA_LOADER_RECORD_RST_INFO_CONFIG

#define UPDATE_GET_POWER_PIN 0  //获取power_pin脚
#define MUTIL_CPU_ENABLE  0 //多芯片升级支持
#define OTA_LOADER_RECORD_RST_INFO_CONFIG  0 //升级记录复位信息
#endif

#if OTA_LOADER_RECORD_RST_INFO_CONFIG
typedef struct {
    ///////////reset info////////////
    u32 p3_rst_flag;
    u32 p3_rst_src;
    u32 p11_clock_pwr_con;
    u32 p11_closk_rst_src;
    u32 jl_lsbclk_pwr_con;
    u32 jl_lsbrst_rst_src;
    ///////////exeception info////////////
    u32 rets;
    u32 reti;
    u32 icfg;
    u32 pc_trace5;
    u32 pc_trace4;
    u32 pc_trace3;
    u32 pc_trace2;
    u32 pc_trace1;
    u32 pc_trace0;
    u32 emu_msg;
    u32 cemu_msg0;
    u32 cemu_msg1;
    u32 cemu_msg2;
    u32 cemu_id;
    u32 hemu_msg0;
    u32 hemu_id;
    u32 lemu_msg0;
    u32 lemu_id;
} RESET_INFO;

u8 ota_loader_record_rst_info();
#endif



/*FPGA调试*/
//#define FPGA_MODE_DEBUG

/*串口打印控制器IO口占据*/
#define UART_ENABLE
//br23 用dp,br30yong usbdp

#define UART_DEBUG_IO     "PA05"
#define UART_DEBUG_BAUD   1000000
/*打印调试信息*/
#if defined(CONFIG_CPU_BR22) || defined(CONFIG_CPU_BR26)
#define UART_API_USING_MASKROM_EN 	0
#else
#define UART_API_USING_MASKROM_EN 	0
#endif

/*升级设备在线检测使能位*/
#define UPDATA_DEV_ONLINE_CHECK_EN		0
#define UPDATA_FLASH_CONTROL			1//升级使能

#define BT_BREDR_EN  BIT(0)
#define BT_BLE_EN    BIT(1)
#define BT_NOMALE_POWER_CFG				0    //模拟正常配置
#define BT_LOW_POWER_CFG				1    //模拟低功耗配置

#define BT_ANALOG_CFG  BT_NOMALE_POWER_CFG   //模拟配置

enum {
    BT_UPDATA_POWER_ON,
    BT_UPDATA_JUMP_APP,

};
enum {
    REQUES_CHECK_DATA = 0,
    REQUES_READ_DATA = 1,
    REQUES_STOP_DATA = 2,
};


#ifdef BLE_UPDATA_SUPPORT_CONNECT
// huayue add 2017/1/12
#define BT_POWER_MODE_ENABLE	    0
#define BLE_ENCRYPT_ENABLE		0
#define BLE_PRIVACY_ENABLE		0
#define BLE_MASTER_ENABLE		    0
#define THREAD_DEBUG_EN			1
#define BQB_RF_SET_DEBUG_EN		1
#define BQB_SYS_TIMER_DEBYG_EN	1
#define BLE_ATT_ENCRYPT_EN		0
#else
#define BT_POWER_MODE_ENABLE	    0
#define BLE_ENCRYPT_ENABLE		0
#define BLE_PRIVACY_ENABLE		0
#define BLE_MASTER_ENABLE		    0
#define THREAD_DEBUG_EN			0
#define BQB_RF_SET_DEBUG_EN		0
#define BQB_SYS_TIMER_DEBYG_EN	0
#define BLE_ATT_ENCRYPT_EN		0

#endif

//配置是否支持uboot升级
#if defined(CONFIG_CPU_BR22) || defined(CONFIG_CPU_BR21) || defined(CONFIG_CPU_BR18)
#define ALL_FLASH_INCLUDE_UBOOT_UPDATE_EN	0
#else //BR23/BR25/BR26/BR30/BD29
#define ALL_FLASH_INCLUDE_UBOOT_UPDATE_EN	1
#endif

//br18 & br22 origin version
#define FLASH_FRAMEWORK_VERSION_V1_EN		0
//br22 new flash framwork
#define FLASH_FRAMEWORK_VERSION_V2_EN		1


//配置单独配置升级采用什么格式
#define FORMAT_BR22_NEW		0
#define FORMAT_BR18_V2		1

#ifdef CONFIG_CPU_BR22
#define CFG_TOOL_BIN_FORMAT_SEL	FORMAT_BR22_NEW
#else 	//BR18/BR26/BR23/BD29/BR21/BR25/BR30
#define CFG_TOOL_BIN_FORMAT_SEL	FORMAT_BR18_V2
#endif

//配置是否支持外部用户接口
#if defined(CONFIG_CPU_BR23) || defined(CONFIG_CPU_BR28) || defined(CONFIG_CPU_BR35)
#define USER_API_CODE_SUPPORT_EN		1
#else
#define USER_API_CODE_SUPPORT_EN		0
#endif

//是否SPI0只采用TEAM A;
#define SPI0_PORT_TEAM_A_ONLY_EN		1

/*是否开启电量检测*/
#define LOW_POWER_DET_EN				0

//是否支持升级外部DSP
#if defined(CONFIG_CPU_BR22) && (EDR_UPDATA_SUPPORT_CONNECT)
#define EX_DSP_UPDATE_SUPPORT_EN		0
#else
#define EX_DSP_UPDATE_SUPPORT_EN		0
#endif

// #if defined(CONFIG_CPU_BR23)
// #define EX_FLASH_UPDATE_SUPPORT_EN      1
// #else
// #define EX_FLASH_UPDATE_SUPPORT_EN      0
// #endif

#if defined(CONFIG_CPU_BR23) && (EDR_UPDATA_SUPPORT_CONNECT)
//配置:经典蓝牙升级速度优化方案
//1.utilized the time waiting for rsp to program flash;
#define OTA_UPDATE_SPEED_OPTIMIZE_METHOD_1		1
//2.utilized the time waiting for programming to receive rsp data;
#define OTA_UPDATE_SPEED_OPTIMIZE_METHOD_2		1
#else
//配置:经典蓝牙升级速度优化方案
//1.utilized the time waiting for rsp to program flash;
#define OTA_UPDATE_SPEED_OPTIMIZE_METHOD_1		0
//2.utilized the time waiting for programming to receive rsp data;
#define OTA_UPDATE_SPEED_OPTIMIZE_METHOD_2		0

#endif

//支持ANC配置和系数升级使能
#if (defined(CONFIG_CPU_BR30) \
		|| defined(CONFIG_CPU_BR34) \
		|| defined(CONFIG_CPU_BR36) \
		|| defined(CONFIG_CPU_BR28) \
	 	|| defined(CONFIG_CPU_WL80)) \
		|| defined(CONFIG_CPU_BR50) \
		|| defined(CONFIG_CPU_BR35) \
		|| defined(CONFIG_CPU_BR52) \
		&& defined(EDR_UPDATA_SUPPORT_CONNECT) || defined(UART_UPDATA_MODULE_CONTROL)
#define ANC_CFG_AND_COEF_UPDATE_EN		1
#else
#define ANC_CFG_AND_COEF_UPDATE_EN		0
#endif

#if defined(CONFIG_CPU_BR42) || defined(CONFIG_CPU_BR46)
#define UPDATE_EFUSE                    1
#else
#define UPDATE_EFUSE                    0
#endif

//配置:是否升级保留区域文件(anc param .ect)
#if (CONFIG_CPU_BR30) || (CONFIG_CPU_BR34) || (CONFIG_CPU_BR36) || (CONFIG_CPU_BR28) || (CONFIG_CPU_BR50) || defined(CONFIG_CPU_BR35) || defined(CONFIG_CPU_BR52) || defined(CONFIG_CPU_BR56)
#define RESERVED_FILE_UPDATE_EN			1
#else
#define RESERVED_FILE_UPDATE_EN			0
#endif

//支持免晶振方案
#if (SD_MODULE_CONTROL || USB_HOST_MODULE_CONTROL || UART_UPDATA_USER_MODULE_CONTROL) && (defined(CONFIG_CPU_BR23) || defined(CONFIG_CPU_BR25))
#define CRYSTAL_FREE_EN					1
#else
#define CRYSTAL_FREE_EN					0
#endif

//仅支持串口测试模式
#define UART_UPDATE_ONLY_TEST_MODE		0

//蓝牙升级仅测试模式
#define BT_UPDATE_ONLY_TEST_MODE		0
//蓝牙测试模式是否不需要回连
#define BT_UPDATE_ONLY_TEST_WITHOUT_RECONNECT    0

#if FLASH_FRAMEWORK_VERSION_V1_EN && FLASH_FRAMEWORK_VERSION_V2_EN
#error "FLASH_FRAMEWORK_VERSION_1&2 NOT OPEN IN SAME TIME!!!\n"
#endif
#endif
