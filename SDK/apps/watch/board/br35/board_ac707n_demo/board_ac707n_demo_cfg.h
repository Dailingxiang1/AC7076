#ifndef CONFIG_BOARD_AC707N_DEMO_CFG_H
#define CONFIG_BOARD_AC707N_DEMO_CFG_H

#include "board_ac707n_demo_global_build_cfg.h"

#ifdef CONFIG_BOARD_JL707N_DEMO

#define CONFIG_SDFILE_ENABLE
//*********************************************************************************//
//                                 配置开始                                        //
//*********************************************************************************//
#define ENABLE_THIS_MOUDLE					1
#define DISABLE_THIS_MOUDLE					0

#define ENABLE								1
#define DISABLE								0

#define NO_CONFIG_PORT						(-1)

//                                  NTC配置                                       //
//*********************************************************************************//
#define NTC_DET_EN      0
#define NTC_POWER_IO    IO_PORTC_03
#define NTC_DETECT_IO   IO_PORTC_04
#define NTC_DET_AD_CH   (0x4)   //根据adc_api.h修改通道号

#define NTC_DET_UPPER        235  //正常范围AD值上限，0度时
#define NTC_DET_LOWER        34  //正常范围AD值下限，45度时


//*********************************************************************************//
//                                 FPGA 配置                                      //
//*********************************************************************************//
#define FPGA_DEVELOP_IOKEY  0

//*********************************************************************************//
//                                 FLASH 配置                                      //
//*********************************************************************************//
//FLASH公共配置
#if TCFG_USER_BLE_ENABLE
#define TCFG_FLASH_ERASE_WAIT_BLE			ENABLE			//flash擦除前等待ble状态
#else
#define TCFG_FLASH_ERASE_WAIT_BLE			DISABLE        //flash擦除前等待ble状态
#endif

//外挂flash配置
#define TCFG_NORFLASH_SFC_DEV_ENABLE        TCFG_NORFLASH_DEV_ENABLE         //外挂flash驱动

#if TCFG_NORFLASH_SFC_DEV_ENABLE
#define TCFG_EX_FLASH_POWER_IO				IO_PORTC_10   // 外置flash电源脚
#define CONFIG_EX_FLASH_POWER_IO			PC10 // NULL  // 外置flash电源脚。不接IO时填NULL或者屏蔽掉该宏定义
#define CONFIG_EX_FLASH_POWER_IO_CTRL		1	// 0:power_io低电平时供电;1：高电平时供电	(针对io口低电平控制mos管导通来供电的情况)
#endif//TCFG_NORFLASH_SFC_DEV_ENABLE

//NANDFLASH配置
#if (defined TCFG_NANDFLASH_DEV_ENABLE && TCFG_NANDFLASH_DEV_ENABLE)
//电源配置
#define TCFG_EX_FLASH_POWER_IO				IO_PORTC_11   // 外置flash电源脚
#define CONFIG_EX_FLASH_POWER_IO			PC11 // NULL  // 外置flash电源脚。不接IO时填NULL或者屏蔽掉该宏定义
#define CONFIG_EX_FLASH_POWER_IO_CTRL		1	// 0:power_io低电平时供电;1：高电平时供电	(针对io口低电平控制mos管导通来供电的情况)

#define TCFG_FLASH_DEV_FLASH_READ_WIDTH     SPI_MODE_UNIDIR_4BIT//SPI_MODE_BIDIR_1BIT//
#define TCFG_FLASH_DEV_SPI_HW_NUM			2// 1: SPI1    2: SPI2
#define TCFG_FLASH_DEV_SPI_CS_PORT	    	IO_PORTC_01

// NANDFLASH驱动配置
#define TCFG_NANDFLASH_HEX_ID				0xc891	// flash ID, 十六进制值的字符串形式
#define TCFG_NANDFLASH_PLANE_NUMBER		    1
#define TCFG_NANDFLASH_ECC_MASK				0x30
#define TCFG_NANDFLASH_ECC_ERR				0x20
#define TCFG_NANDFLASH_PLANE_SEL			0
#define TCFG_NANDFLASH_WRITE_ENABLE_POS		0
#define TCFG_NANDFLASH_QUAD_MODE_DUM_NUM	1
#define TCFG_NANDFLASH_QUAD_MODE_QE			1
#define TCFG_NANDFLASH_BLOCK_NUMBER		    1024
#define TCFG_NANDFLASH_CAPACITY				(128 * 1024 * 1024)	// flash 总实际容量
#define TCFG_NANDFLASH_PAGE_NUM				64
#define TCFG_NANDFLASH_PAGE_SIZE			2048
#define TCFG_NANDFLASH_OOB_SIZE				64
#define TCFG_NANDFLASH_OOB_USER_OFFSET_0	0
#define TCFG_NANDFLASH_OOB_USER_OFFSET_1    0
#define TCFG_NANDFLASH_OOB_USER_SIZE_0		64
#define TCFG_NANDFLASH_OOB_USER_SIZE_1		0
#define TCFG_NANDFLASH_MAX_ERASE_CNT		100000	// 最大擦写次数，一般默认10万次

#if(!TCFG_PSRAM_DEV_ENABLE)
#error "need enable psram!!!"
#endif
#endif//TCFG_NANDFLASH_DEV_ENABLE

//*********************************************************************************//
//                                 FLASH dev分区                                   //
//*********************************************************************************//
//分区配置
#if (!TCFG_UI_ENABLE)//不开UI时，不使用文件系统

#define TCFG_VIRFAT_FLASH_ENABLE					DISABLE		//虚拟文件系统

#define TCFG_VIRFAT_INSERT_FLASH_ENABLE  			DISABLE		//内置FLASH虚拟文件系统
#define TCFG_VIRFAT_EXT_FLASH_ENABLE  	    		DISABLE 	//外挂FLASH虚拟文件系统

#define TCFG_SDFILE_INSERT_FLASH_ENABLE  			DISABLE		//内置只读文件系统
#define TCFG_SDFILE_EXTERN_FLASH_ENABLE				DISABLE 	//外挂只读文件系统

#define TCFG_DATA_STORAGE_FDB_ENABLE  		ENABLE		//用户数据存FDB
#define TCFG_DATA_STORAGE_FDB_EXFLASH_ENABLE  DISABLE		//用户数据存FDB&外挂flash
#define TCFG_DATA_STORAGE_VM_ENABLE   		DISABLE		//用户数据存VM

#define TCFG_DATA_DEV_NAME         			"watch_data"
#define TCFG_DATA_FLASH_DEV_PATY_BASE	  		0x1D6000	//
#define TCFG_DATA_FLASH_DEV_PATY_SIZE	  		0x28000 	// 160K
#else//(!TCFG_UI_ENABLE)
//------------------------------------------------------------------------------------------
//ONLY_INSIDE_FLASH//只有内置flash，按8M规划
#if ((!TCFG_NORFLASH_DEV_ENABLE)&&(!TCFG_NANDFLASH_DEV_ENABLE)&&(CONFIG_FLASH_SIZE == 8*1024*1024))
#define TCFG_VIRFAT_FLASH_ENABLE					ENABLE		//虚拟文件系统
#define TCFG_VIRFAT_INSERT_FLASH_ENABLE  			ENABLE		//内置FLASH虚拟文件系统
#define TCFG_VIRFAT_EXT_FLASH_ENABLE  	    		DISABLE 	//外挂FLASH虚拟文件系统

#define TCFG_SDFILE_INSERT_FLASH_ENABLE  			ENABLE		//内置只读文件系统
#define TCFG_SDFILE_EXTERN_FLASH_ENABLE				DISABLE 	//外挂只读文件系统

#define TCFG_DATA_STORAGE_FDB_ENABLE  		ENABLE		//用户数据存FDB
#define TCFG_DATA_STORAGE_FDB_EXFLASH_ENABLE  DISABLE		//用户数据存FDB&外挂flash
#define TCFG_DATA_STORAGE_VM_ENABLE   		DISABLE		//用户数据存VM

//sdfile文件系统
#define TCFG_SDFILE_FLASH_DEV_PATY_SIZE	  			0x500000 // 5M
#define TCFG_SDFILE_FLASH_DEV_PATY_BASE	  			0x1fe000 // 2M - 8k开始 内置FLASH虚拟文件系统空间基础地址
//virfat
#define TCFG_VIRFAT_FLASH_DEV_PATY_BASE        		0x6FE000//(TCFG_MODE_INSERT_FLASH_BASE + TCFG_MODE_INSERT_FLASH_SIZE)
#define TCFG_VIRFAT_FLASH_DEV_PATY_SIZE        		0xD8000 //864K
//watch_data
#define TCFG_DATA_DEV_NAME         			"watch_data"
#define TCFG_DATA_FLASH_DEV_PATY_BASE			0x7D6000//(TCFG_WATCH_INSERT_FLASH_BASE + TCFG_WATCH_INSERT_FLASH_SIZE)
#define TCFG_DATA_FLASH_DEV_PATY_SIZE			0x28000 //160K

//------------------------------------------------------------------------------------------
//ONLY_INSIDE_FLASH//只有内置flash，按16M规划
#elif ((!TCFG_NORFLASH_DEV_ENABLE)&&(!TCFG_NANDFLASH_DEV_ENABLE)&&(CONFIG_FLASH_SIZE == 16*1024*1024))
#define TCFG_VIRFAT_FLASH_ENABLE					ENABLE		//虚拟文件系统
#define TCFG_VIRFAT_INSERT_FLASH_ENABLE  			ENABLE		//内置FLASH虚拟文件系统
#define TCFG_VIRFAT_EXT_FLASH_ENABLE  	    		DISABLE 	//外挂FLASH虚拟文件系统

#define TCFG_SDFILE_INSERT_FLASH_ENABLE  			ENABLE		//内置只读文件系统
#define TCFG_SDFILE_EXTERN_FLASH_ENABLE				DISABLE 	//外挂只读文件系统

#define TCFG_DATA_STORAGE_FDB_ENABLE  		ENABLE		//用户数据存FDB
#define TCFG_DATA_STORAGE_FDB_EXFLASH_ENABLE  DISABLE		//用户数据存FDB&外挂flash
#define TCFG_DATA_STORAGE_VM_ENABLE   		DISABLE		//用户数据存VM

//sdfile文件系统
#define TCFG_SDFILE_FLASH_DEV_PATY_SIZE	  			0x580000 // 5.5M
#define TCFG_SDFILE_FLASH_DEV_PATY_BASE	  			0x214000 // 1.5M - 8k开始 内置FLASH虚拟文件系统空间基础地址
//virfat
#define TCFG_VIRFAT_FLASH_DEV_PATY_BASE        		0x794000//(TCFG_MODE_INSERT_FLASH_BASE + TCFG_MODE_INSERT_FLASH_SIZE)
#define TCFG_VIRFAT_FLASH_DEV_PATY_SIZE        		0xD8000 //864K
//watch_data
#define TCFG_DATA_DEV_NAME         			"watch_data"
#define TCFG_DATA_FLASH_DEV_PATY_BASE			0x86c000//(TCFG_WATCH_INSERT_FLASH_BASE + TCFG_WATCH_INSERT_FLASH_SIZE)
#define TCFG_DATA_FLASH_DEV_PATY_SIZE			0x28000 //160K
//------------------------------------------------------------------------------------------
//EXT_NOR_FLASH//外挂NOR fals
#elif (TCFG_NORFLASH_DEV_ENABLE)
#define TCFG_VIRFAT_FLASH_ENABLE					ENABLE		//虚拟文件系统
#define TCFG_VIRFAT_INSERT_FLASH_ENABLE  			DISABLE		//内置FLASH虚拟文件系统
#define TCFG_VIRFAT_EXT_FLASH_ENABLE  	    		ENABLE	 	//外挂FLASH虚拟文件系统

#define TCFG_SDFILE_INSERT_FLASH_ENABLE  			DISABLE		//内置只读文件系统
#define TCFG_SDFILE_EXTERN_FLASH_ENABLE				ENABLE 		//外挂只读文件系统

#define TCFG_DATA_STORAGE_FDB_ENABLE  		ENABLE		//用户数据存FDB
#define TCFG_DATA_STORAGE_FDB_EXFLASH_ENABLE  ENABLE		//用户数据存FDB&外挂flash
#define TCFG_DATA_STORAGE_VM_ENABLE   		DISABLE		//用户数据存VM

//watch_data
#define TCFG_DATA_DEV_NAME         			"watch_data"
#define TCFG_DATA_FLASH_DEV_PATY_SIZE			0x28000 //160K
#define TCFG_DATA_FLASH_DEV_PATY_BASE			(CONFIG_EXTERN_FLASH_SIZE -TCFG_DATA_FLASH_DEV_PATY_SIZE)
//sdfile文件系统
#define TCFG_SDFILE_FLASH_DEV_PATY_SIZE	  			0x580000 // 5.5M
#define TCFG_SDFILE_FLASH_DEV_PATY_BASE	  			(CONFIG_EXTERN_FLASH_SIZE -TCFG_SDFILE_FLASH_DEV_PATY_SIZE-TCFG_DATA_FLASH_DEV_PATY_SIZE)
//virfat
#define TCFG_VIRFAT_FLASH_DEV_PATY_BASE        		0x000000//(TCFG_MODE_INSERT_FLASH_BASE + TCFG_MODE_INSERT_FLASH_SIZE)
#define TCFG_VIRFAT_FLASH_DEV_PATY_SIZE        		(CONFIG_EXTERN_FLASH_SIZE -TCFG_SDFILE_FLASH_DEV_PATY_SIZE-TCFG_DATA_FLASH_DEV_PATY_SIZE)


//------------------------------------------------------------------------------------------
//EXT_NAND_FLASH//外挂NAND falsh
#elif (TCFG_NANDFLASH_DEV_ENABLE)
#define TCFG_VIRFAT_FLASH_ENABLE					DISABLE		//虚拟文件系统
#define TCFG_VIRFAT_INSERT_FLASH_ENABLE  			DISABLE		//内置FLASH虚拟文件系统
#define TCFG_VIRFAT_EXT_FLASH_ENABLE  	    		DISABLE	 	//外挂FLASH虚拟文件系统

#define TCFG_SDFILE_INSERT_FLASH_ENABLE  			DISABLE		//内置只读文件系统
#define TCFG_SDFILE_EXTERN_FLASH_ENABLE				DISABLE 	//外挂只读文件系统

#define TCFG_DATA_STORAGE_FDB_ENABLE  		ENABLE		//用户数据存FDB
#define TCFG_DATA_STORAGE_FDB_EXFLASH_ENABLE  ENABLE		//用户数据存FDB&外挂flash
#define TCFG_DATA_STORAGE_VM_ENABLE   		DISABLE		//用户数据存VM

#define TCFG_NANDFLASH_UI_FAT_ENABLE 				ENABLE		//nandflash ui_fat分区使能(与virfat互斥，可以超过32M)
#define TCFG_NANDFLASH_FAT_ENABLE 					ENABLE		//nandflash fat分区使能
#define TCFG_NANDFLASH_UNUSED_FILE_SYSTEM			DISABLE		//不挂文件系统
//sdfile文件系统
#define TCFG_SDFILE_FLASH_DEV_PATY_SIZE	  			0x580000  // 5.5M
#define TCFG_SDFILE_FLASH_DEV_PATY_BASE	  			0x17e000
//uifat/virfat
#define TCFG_NANDFLASH_UI_FAT_LOGO					"UI_FAT"
#define TCFG_UI_FAT_FLASH_DEV_PATY_BASE				0x00000000
#define TCFG_UI_FAT_FLASH_DEV_PATY_SIZE				0x02000000							//32M	(使用virfat时不能超过32M，使用ui_fat没有限制)
//当TCFG_VIRFAT_INSERT_FLASH_ENABLE 或 TCFG_VIRFAT_EXT_FLASH_ENABLE使能时有效
#define TCFG_VIRFAT_FLASH_DEV_PATY_BASE             0x6FE000
#define TCFG_VIRFAT_FLASH_DEV_PATY_SIZE        		0xD8000 							//32M	(使用virfat时不能超过32M，使用ui_fat没有限制)
// nandflash 分区 fat16 区域地址和大小，与ini工具文件对应，即ini文件中，fat16文件的XX_ADR参数与BASE保持参数一致
#define TCFG_NANDFLASH_FAT_ROOT						"fat_nand"							// fat16 分区的设备名(存音乐)
#define TCFG_NANDFLASH_FAT_BASE						TCFG_UI_FAT_FLASH_DEV_PATY_SIZE	    // fat16 分区起始地址
#define TCFG_NANDFLASH_FAT_SIZE						(90 * 1024 * 1024)					// fat16 分区容量大小
//watch_data
#define TCFG_DATA_DEV_NAME         			"watch_data"
#define TCFG_DATA_FLASH_DEV_PATY_BASE			(TCFG_NANDFLASH_FAT_SIZE+TCFG_NANDFLASH_FAT_BASE)
#define TCFG_DATA_FLASH_DEV_PATY_SIZE			0x28000 //160K

// #if( TCFG_VIRFAT_FLASH_ENABLE && TCFG_NANDFLASH_UI_FAT_ENABLE)
// #error"config error,only enable one"
// #endif
#if(TCFG_VIRFAT_INSERT_FLASH_ENABLE &&  TCFG_VIRFAT_EXT_FLASH_ENABLE)
#error"config error,only enable one"
#endif

//------------------------------------------------------------------------------------------
#else
#error "FLASH_DEV_PART_CONFIG_ERROR !!!"
#endif
#endif
#define TCFG_DATA_STORAGE_ENABLE      (TCFG_DATA_STORAGE_FDB_ENABLE || TCFG_DATA_STORAGE_VM_ENABLE ||TCFG_DATA_STORAGE_FDB_EXFLASH_ENABLE)


//*********************************************************************************//
//                                 PSRAM 配置                                      //
//*********************************************************************************//
#if (defined TCFG_NANDFLASH_DEV_ENABLE && TCFG_NANDFLASH_DEV_ENABLE)
#define TCFG_PSRAM_POWER_PORT			 IO_PORTB_05//7074使用pb5供电
#else
#define TCFG_PSRAM_POWER_PORT			 IO_FS_PG2
#endif

#define TCFG_PSRAM_PORT_SEL				 PSRAM_PORT_SEL_PORTA
#define TCFG_PSRAM_MODE					 PSRAM_MODE_4_WIRE_CMD4_ADR4_DAT4
#define TCFG_PSRAM_INIT_CLK 			 (144* 1000000)	//144Mhz	//内封跑144M
// #define TCFG_PSRAM_INIT_CLK				 (96*1000000)	//96Mhz		//顶板外挂考虑降频

#if TCFG_NORFLASH_SFC_DEV_ENABLE && TCFG_PSRAM_DEV_ENABLE
#error "ext norflash and psram only support one dev !!!"
#endif


//*********************************************************************************//
//                                 tp   配置                                      //
//*********************************************************************************//
#define TCFG_TOUCH_PANEL_ENABLE 				1
#define TCFG_TP_CST816D_ENABLE 					1
#define TCFG_TP_FT3X68_ENABLE 					0
#define TCFG_TP_SLEEP_EN                        1
#define TCFG_TP_INT_IO      					IO_PORTB_08 //TP中断脚
#define TCFG_TP_RESET_IO    					IO_PORTA_02 //TP复位脚


//*********************************************************************************//
//                                 LCD   配置                                      //
//*********************************************************************************//
#define TCFG_SPI_LCD_ENABLE 1//spi lcd开关
#define TCFG_LCD_SPI_ICNA3310B_ENABLE 0
#define TCFG_LCD_SPI_GC9B71_ENABLE  0
#define TCFG_LCD_SPI_ST77916_ENABLE 1
#define TCFG_LCD_SPI_SH8501A_ENABLE 0
#define TCFG_LCD_SPI_SH8601A_ENABLE 0
#define TCFG_LCD_QSPI_ST77903_V2_ENABLE 0
#define TCFG_LCD_QSPI_SD3302_ENABLE 0
#define TCFG_LCD_SPI_RM69330_ENABLE 0
#define TCFG_LCD_MCU_JD5858_ENABLE 0
#define TCFG_LCD_RGB_ST7789V_ENABLE 0
#define TCFG_LCD_RGB_ENABLE 0
#define TCFG_LCD_MATCH_MODE 0
#define LCD_MATCH_BY_LOGO   0 //通过屏驱logo来匹配
#define LCD_LOGO            "null"
#define TCFG_LCD_TE_USED_PEND 1

#define TCFG_7074_EX_NORFLASH_CONFIG			DISABLE//开发板使用，7074顶板外挂nand/nor开这个宏,改跳线

#if ((defined TCFG_NANDFLASH_DEV_ENABLE && TCFG_NANDFLASH_DEV_ENABLE)||TCFG_7074_EX_NORFLASH_CONFIG)
#define TCFG_LCD_TE_IO      IO_PORTA_00//TCFG_LCD_PIN_TE
#else
#define TCFG_LCD_TE_IO      TCFG_LCD_PIN_TE
#endif


#define TCFG_LCD_TE_QUICK_SYNC                  1//te优化策略
#define TCFG_LCD_BL_IO							TCFG_LCD_PIN_BL
#define TCFG_BACKLIGHT_PWM_MODE 				2// 0-on/off, 1-pwmled(no support), 2-mcpwm
#define TCFG_UI_SHUT_DOWN_TIME					ENABLE // 自动息屏

#define TCFG_LCD_TP_USE_SAME_PWR				ENABLE // lcd和tp 使用相同的电源
#if TCFG_LCD_TP_USE_SAME_PWR
#undef TCFG_TP_SLEEP_EN
#define TCFG_TP_SLEEP_EN                        DISABLE
#define LCD_POWER_DOWN_EN  						ENABLE
#define TP_POWER_DOWN_EN  						ENABLE
#endif
#define TCFG_GPU_DOUBLE_TASK_ENABLE             ENABLE

#define TCFG_LCD_BUF_IN_STATIC_RAM_ENABLE		ENABLE		//屏幕显存buf使用静态ram，可以优化帧率
#define TCFG_LCD_BUF_DYNAMIC_LINE_ENABLE		DISABLE		//屏幕显存高度动态计算，复杂场景下可以减少屏幕需要的ram

#if (CONFIG_JL_UI_ENABLE&&TCFG_APP_MUSIC_EN)
#define TCFG_LRC_LYRICS_ENABLE 					ENABLE	//jlui本地播放打开歌词显示
#endif
#define TCFG_PC_MODE_LOCK_UI_ENABLE				ENABLE		//PC模式锁定UI
//*********************************************************************************//
//                                 MOTO 配置                                      //
//*********************************************************************************//
#define TCFG_MOTO_PWM_IO 						IO_MT_PG

//*********************************************************************************//
//                                 rdec_key 配置                                      //
//*********************************************************************************//
#define TCFG_RDEC_KEY_ENABLE					ENABLE_THIS_MOUDLE //是否使能RDEC按键
//RDEC0配置
#define TCFG_RDEC0_ECODEA_PORT					IO_PORT_DP
#define TCFG_RDEC0_ECODEB_PORT					IO_PORT_DM
#define TCFG_RDEC0_KEY0_VALUE 				 	0
#define TCFG_RDEC0_KEY1_VALUE 				 	1

//*********************************************************************************//
//                                 p11 iic 配置                                      //
//*********************************************************************************//
#define TCFG_HW_I2C_P11_CLK_PORT					TCFG_HW_I2C1_CLK_PORT
#define TCFG_HW_I2C_P11_DAT_PORT					TCFG_HW_I2C1_DAT_PORT
#define TCFG_HW_I2C_P11_CLK					        TCFG_HW_I2C1_CLK

//*********************************************************************************//
//                                 充电 配置 (补充sdk_config_h)                                     //
//*********************************************************************************//
#ifndef   TCFG_CHARGE_NVDC_EN
#define   TCFG_CHARGE_NVDC_EN  TCFG_CHARGE_ENABLE
#endif

#define   TCFG_CHANEG_DONT_ENTER_LOW_POWER		ENABLE	//充电不进低功耗
//*********************************************************************************//
//                                 看门狗配置                                     //
//*********************************************************************************//
#define   WDT_APP_INIT_TIME							WDT_16S			//开机初始化的看门狗时间，适当给长一些
#define   WDT_APP_RUN_TIME							WDT_LRC_4S		//程序正常运行的看门狗时间，用LRC看门狗准一点


/**************
 *ANC配置
 *************/
#define TCFG_AUDIO_ANC_ENABLE				CONFIG_ANC_ENABLE		//ANC总使能,根据global_bulid_cfg板级定义
// #define TCFG_ANC_TOOL_DEBUG_ONLINE 			DISABLE_THIS_MOUDLE		//ANC工具蓝牙spp调试
#define TCFG_ANC_EXPORT_RAM_EN				DISABLE_THIS_MOUDLE		//ANCdebug数据释放RAM使能
#if TCFG_ANC_EXPORT_RAM_EN
#define TCFG_AUDIO_CVP_CODE_AT_RAM			DISABLE_THIS_MOUDLE
#define TCFG_AUDIO_AAC_CODE_AT_RAM			DISABLE_THIS_MOUDLE
#endif/*TCFG_ANC_EXPORT_RAM_EN*/

/*
 *系统音量类型选择
 *软件数字音量是指纯软件对声音进行运算后得到的
 *硬件数字音量是指dac内部数字模块对声音进行运算后输出
 */
#define VOL_TYPE_DIGITAL		0	//软件数字音量(调节解码输出数据的音量)
#define VOL_TYPE_ANALOG			1	//(暂未支持)硬件模拟音量
#define VOL_TYPE_AD				2	//(暂未支持)联合音量(模拟数字混合调节)
#define VOL_TYPE_DIGITAL_HW		3  	//硬件数字音量(调节DAC模块的硬件音量)
/*注意:ANC使能情况下使用软件数字音量*/
#if TCFG_AUDIO_ANC_ENABLE
//#define SYS_VOL_TYPE            VOL_TYPE_DIGITAL
#else
//#define SYS_VOL_TYPE            VOL_TYPE_DIGITAL_HW
#endif/*TCFG_AUDIO_ANC_ENABLE*/
/*
 *通话的时候使用数字音量
 *0：通话使用和SYS_VOL_TYPE一样的音量调节类型
 *1：通话使用数字音量调节，更加平滑
 */
#define TCFG_CALL_USE_DIGITAL_VOLUME		0

//第三方清晰语音开发使能
// #define TCFG_CVP_DEVELOP_ENABLE             DISABLE_THIS_MOUDLE

/*通话降噪模式配置*/
// #define CVP_ANS_MODE	0	[>传统降噪<]
// #define CVP_DNS_MODE	1	[>神经网络降噪<]
// #define TCFG_AUDIO_CVP_NS_MODE				CVP_ANS_MODE

/*
 * ENC(双mic降噪)配置
 * 双mic降噪包括DMS_NORMAL和DMS_FLEXIBLE，在使能TCFG_AUDIO_DUAL_MIC_ENABLE
 * 的前提下，根据具体需求，选择对应的DMS模式
 */
/*ENC(双mic降噪)使能*/
// #define TCFG_AUDIO_DUAL_MIC_ENABLE			DISABLE_THIS_MOUDLE

/*DMS模式选择*/
// #define DMS_NORMAL		1	//普通双mic降噪(mic距离固定)
// #define DMS_FLEXIBLE	2	//适配mic距离不固定且距离比较远的情况，比如头戴式话务耳机
// #define TCFG_AUDIO_DMS_SEL					DMS_NORMAL

/*ENC双mic配置主mic副mic对应的mic port*/
// #define DMS_MASTER_MIC0		0 //mic0是主mic
// #define DMS_MASTER_MIC1		1 //mic1是主mic
// #define TCFG_AUDIO_DMS_MIC_MANAGE			DMS_MASTER_MIC0
/*双mic降噪/单麦mic降噪 DUT测试模式，配合设备测试mic频响和(双mic)降噪量*/

//MIC通道配置
// #if TCFG_AUDIO_DUAL_MIC_ENABLE
//#define TCFG_AUDIO_ADC_MIC_CHA				(AUDIO_ADC_MIC_0 | AUDIO_ADC_MIC_1)
// #else
//#define TCFG_AUDIO_ADC_MIC_CHA				AUDIO_ADC_MIC_0
// #endif[>TCFG_AUDIO_DUAL_MIC_ENABLE<]

/*MIC模式配置:单端隔直电容模式/差分隔直电容模式/单端省电容模式*/
// #if TCFG_AUDIO_ANC_ENABLE
/*注意:ANC使能情况下，使用差分mic*/
// #define TCFG_AUDIO_MIC_MODE					AUDIO_MIC_CAP_DIFF_MODE
// #define TCFG_AUDIO_MIC1_MODE				AUDIO_MIC_CAP_DIFF_MODE
// #else
// #define TCFG_AUDIO_MIC_MODE					AUDIO_MIC_CAP_MODE
// #define TCFG_AUDIO_MIC1_MODE				AUDIO_MIC_CAP_MODE
// #endif[>TCFG_AUDIO_ANC_ENABLE<]

/*
 *>>MIC电源管理:根据具体方案，选择对应的mic供电方式
 *(1)如果是多种方式混合，则将对应的供电方式或起来即可，比如(MIC_PWR_FROM_GPIO | MIC_PWR_FROM_MIC_BIAS)
 *(2)如果使用固定电源供电(比如dacvdd)，则配置成DISABLE_THIS_MOUDLE
 */
#define MIC_PWR_FROM_GPIO		(1UL << 0)	//使用普通IO输出供电
#define MIC_PWR_FROM_MIC_BIAS	(1UL << 1)	//使用内部mic_ldo供电(有上拉电阻可配)
#define MIC_PWR_FROM_MIC_LDO	(1UL << 2)	//使用内部mic_ldo供电
//配置MIC电源
// #define TCFG_AUDIO_MIC_PWR_CTL				MIC_PWR_FROM_MIC_BIAS


/*提示音叠加配置*/
#define TCFG_WAV_TONE_MIX_ENABLE			DISABLE
#define TCFG_MP3_TONE_MIX_ENABLE			DISABLE
#define TCFG_WTG_TONE_MIX_ENABLE			DISABLE
#define TCFG_WTS_TONE_MIX_ENABLE			ENABLE



//*********************************************************************************//
//                         Spatial Audio Effect 空间音效配置                       //
//*********************************************************************************//
// #define TCFG_AUDIO_SPATIAL_EFFECT_ENABLE           	DISABLE_THIS_MOUDLE
// #define TCFG_TWS_SPATIAL_AUDIO_AS_CHANNEL   		'L'

/*独立任务里面跑空间音效*/
#define TCFG_AUDIO_EFFECT_TASK_EBABLE               ENABLE_THIS_MOUDLE

/*空间音效在线调试*/
// #define TCFG_SPATIAL_EFFECT_ONLINE_ENABLE           DISABLE_THIS_MOUDLE

/*空间音频传感器是否在解码任务读传感器数据*/
#define TCFG_SENSOR_DATA_READ_IN_DEC_TASK           DISABLE_THIS_MOUDLE

/*传感器数据读取的频率间隔，单位ms*/
#define TCFG_SENSOR_DATA_READ_INTERVAL              20

/*空间音频独立EQ使能*/
#define TCFG_SPATIAL_EFFECT_EQ_ENABLE               DISABLE_THIS_MOUDLE

/*陀螺仪数据导出配置:支持BT_SPP\UART载体导出*/
#define SENSOR_DATA_EXPORT_USE_UART 	1
#define SENSOR_DATA_EXPORT_USE_SPP 	    2
#define TCFG_SENSOR_DATA_EXPORT_ENABLE				DISABLE_THIS_MOUDLE

//*********************************************************************************//
//                                  IIS 配置                                     //
//*********************************************************************************//
#define TCFG_IIS_ENABLE                     DISABLE_THIS_MOUDLE

#define TCFG_IIS_MODE                       (0)     // 0:master  1:slave

#define TCFG_AUDIO_INPUT_IIS                (ENABLE && TCFG_IIS_ENABLE)
#define TCFG_IIS_INPUT_DATAPORT_SEL         ALINK_CH1

#define TCFG_AUDIO_OUTPUT_IIS               (DISABLE && TCFG_IIS_ENABLE)
#define TCFG_IIS_OUTPUT_DATAPORT_SEL        ALINK_CH0

#define TCFG_IIS_SAMPLE_RATE                (16000L)


//*********************************************************************************//
//                                  g-sensor配置                                   //
//*********************************************************************************//
#define TCFG_GSENSOR_ENABLE                       0     //gSensor使能
#define TCFG_DA230_EN                             0
#define TCFG_SC7A20_EN                            0
#define TCFG_STK8321_EN                           0
#define TCFG_IRSENSOR_ENABLE                      0
#define TCFG_JSA1221_ENABLE                       0
#define TCFG_GSENOR_USER_IIC_TYPE                 0     //0:软件IIC  1:硬件IIC

//*********************************************************************************//
//                                  imu-sensor配置                                   //
//*********************************************************************************//
#define TCFG_IMUSENSOR_ENABLE                	1    //imu Sensor使能
//mpu6887 cfg
#define TCFG_MPU6887P_ENABLE                  	0
#define TCFG_MPU6887P_INTERFACE_TYPE          	0 //0:iic, 1:spi
#define TCFG_MPU6887P_USER_IIC_TYPE           	0 //iic有效:1:硬件iic, 0:软件iic
#define TCFG_MPU6887P_USER_IIC_INDEX          	0 //IIC 序号
#define TCFG_MPU6887P_DETECT_IO               	(-1) //传感器中断io
#define TCFG_MPU6887P_AD0_SELETE_IO             IO_PORTC_03 //iic地址选择io
//icm42607p cfg
#define TCFG_ICM42670P_ENABLE                  	0
#define TCFG_ICM42670P_INTERFACE_TYPE          	0 //0:iic, 1:spi
#define TCFG_ICM42670P_USER_IIC_TYPE           	0 //iic有效:1:硬件iic, 0:软件iic
#define TCFG_ICM42670P_USER_IIC_INDEX          	0 //IIC 序号
#define TCFG_ICM42670P_DETECT_IO               	(-1) //传感器中断io
#define TCFG_ICM42670P_AD0_SELETE_IO            (-1) //iic地址选择io
//mpu9250 cfg
#define TCFG_TP_MPU9250_ENABLE                	0
#define TCFG_MPU9250_INTERFACE_TYPE           	0 //不支持.0:iic, 1:spi
#define TCFG_MPU9250_USER_IIC_TYPE            	0 //iic有效:1:硬件iic, 0:软件iic
#define TCFG_MPU9250_USER_IIC_INDEX           	0 //IIC 序号
#define TCFG_MPU9250_DETECT_IO              	IO_PORTB_03 //传感器中断io
//sh3001 cfg
#define TCFG_SH3001_ENABLE                    	0
#define TCFG_SH3001_INTERFACE_TYPE            	0 //0:iic, 1:spi
#define TCFG_SH3001_USER_IIC_TYPE		      	0 //1:硬件iic, 0:软件iic
#define TCFG_SH3001_USER_IIC_INDEX            	0 //IIC 序号
#define TCFG_SH3001_DETECT_IO                 	IO_PORTB_03 //传感器中断io
//qmi8658 cfg
#define TCFG_QMI8658_ENABLE                     0
#define TCFG_QMI8658_INTERFACE_TYPE             0 //0:iic, 1:spi, 2:i3c
#define TCFG_QMI8658_USER_IIC_TYPE              0 //1:硬件iic, 0:软件iic
#define TCFG_QMI8658_USER_IIC_INDEX          	0 //IIC 序号
#define TCFG_QMI8658_DETECT_IO               	(-1) //传感器中断io
#define TCFG_QMI8658_AD0_SELETE_IO              (-1) //iic地址选择io
//lsm6dsl cfg
#define TCFG_LSM6DSL_ENABLE                     1
#define TCFG_LSM6DSL_INTERFACE_TYPE             0 //0:iic, 1:spi
#define TCFG_LSM6DSL_USER_IIC_TYPE              0 //1:硬件iic, 0:软件iic
#define TCFG_LSM6DSL_USER_IIC_INDEX          	0 //IIC 序号
#define TCFG_LSM6DSL_DETECT_IO               	(-1) //传感器中断io
#define TCFG_LSM6DSL_AD0_SELETE_IO              (-1) //iic地址选择io
//mpu6050 cfg
#define TCFG_MPU6050_EN                     	0
//qmc5883 cfg

/*
 *imu-sensor power manager
 *不用独立IO供电，则配置 NO_CONFIG_PORT
 */
#define TCFG_IMU_SENSOR_PWR_PORT				IO_PORTD_05


//*********************************************************************************//
//                            传感器与运动健康 配置                                //
//*********************************************************************************//
//运动健康总开关
#define TCFG_SPORT_HEALTH_ENABLE			ENABLE
#define TCFG_SENSOR_HUB_TASK_ENABLE			DISABLE
//各模块开关
#define TCFG_SPORT_HEALTH_ALGO_GSENSOR		ENABLE				//gsensor算法
#define TCFG_SPORT_HEALTH_SPORT				ENABLE				//运动
#define TCFG_SPORT_HEALTH_DAILY_ACTIVE		ENABLE				//日常活动
#define TCFG_SPORT_HEALTH_SLEEP				DISABLE				//睡眠
#define TCFG_SPORT_HEALTH_HEART_RATE		DISABLE				//心率
#define TCFG_SPORT_HEALTH_BLOOD_OXYGEN		DISABLE				//血氧
#define TCFG_SPORT_HEALTH_DET_MENSE			ENABLE				//女性健康监测

//SENSOR HUB总开关
#define TCFG_SENSOR_HUB						DISABLE
//P11 sensorhub开关
#define TCFG_SENSOR_HUB_P11					DISABLE
//common
#define TCFG_ALGO_STEP_COUNETER				DISABLE
#define TCFG_ACCELER_SLEEP_ENABLE  			DISABLE
//p11
#define	TCFG_ACCELER_P11_ENABLE 			DISABLE
#define	TCFG_GYRO_P11_ENABLE 				DISABLE
#define	TCFG_MAGNETIC_P11_ENABLE          	DISABLE
#define	TCFG_VCHR11_P11_ENABLE          	DISABLE
#define	TCFG_HRS3602_P11_ENABLE          	DISABLE

//master
#define TCFG_ACCELER_MASTER_ENABLE			DISABLE
#define TCFG_GYRO_MASTER_ENABLE				DISABLE
#define	TCFG_MAGNETIC_MASTER_ENABLE         DISABLE
#define	TCFG_VCHR11_MASTER_ENABLE          	DISABLE
#define	TCFG_HRS3602_MASTER_ENABLE          DISABLE

//gsensor配置
#define TCFG_SENSOR_SC7A20_ENABLE			DISABLE

//msensor配置
#define TCFG_SENSOR_MMC5603_ENABLE			DISABLE
//*********************************************************************************//
//                                  pay 配置                                       //
//*********************************************************************************//
#if CONFIG_JL_UI_ENABLE && TCFG_USER_BLE_ENABLE && (CONFIG_BT_MODE == BT_NORMAL)
#define TCFG_PAY_ALIOS_ENABLE			    1
#else
#define TCFG_PAY_ALIOS_ENABLE			    0
#endif

#define TCFG_PAY_TRANSITCODE_ENABLE         0 // 乘车码
#define TCFG_TRANSIT_USE_PSRAM              0 // 乘车码使用psram,需使能psram

#define TCFG_PAY_ALIOS_WAY_T_HEAD			1 // 平头哥
#define TCFG_PAY_ALIOS_WAY_SEL				TCFG_PAY_ALIOS_WAY_T_HEAD

#if (TCFG_PAY_ALIOS_WAY_SEL==TCFG_PAY_ALIOS_WAY_T_HEAD)
#define TCFG_PAY_ALIOS_PRODUCT_MODEL        ""
#define TCFG_PAY_ALIOS_COMPANY_NAME         "" //需要客户申请
#define ALIPAY_SE_FW_V2_0                   1  //SE版本固件为2.0设置为1,否则设置为0
#define ALIPAY_SE_USE_RESET_PIN             0  //置1加密芯片采用reset管脚复位进低功耗，置0 上下电进低功耗
#define SE_POWER_GPIO                       0  //使用GPIO口给SE芯片电源脚供电

#endif

#if TCFG_PAY_TRANSITCODE_ENABLE
#ifdef LWIP_USE_BT
#undef  USER_SUPPORT_PROFILE_PAN
#define USER_SUPPORT_PROFILE_PAN    1 //开启pan服务
#endif
#endif

//*********************************************************************************//
//                                  触摸配置                                       //
//*********************************************************************************//
#define TCFG_LPCTMU_ENABLE                  0   //总开关
#define TCFG_LPCTMU_CH0_EN                  0   //IO_PORTB_00
#define TCFG_LPCTMU_CH1_EN                  1   //IO_PORTB_01
#define TCFG_LPCTMU_CH2_EN                  0   //IO_PORTB_02
#define TCFG_LPCTMU_CH3_EN                  0   //IO_PORTB_03
#define TCFG_LPCTMU_CH4_EN                  0   //IO_PORTB_04


//*********************************************************************************//
//                                  demo配置                                       //
//*********************************************************************************//
#define GPU_PORT_DEMO_ENABLE				0		//使能gpu_port_demo

#define GPU_DEMO_ENABLE						0		//运行gpu demo的总开关

//*********************************************************************************//
//                                 Find my 配置                                      //
//*********************************************************************************//
#define TCFG_FINDMY_ENABLE 						0

//*********************************************************************************//
//                                 emitter配置                                      //
//*********************************************************************************//
#if TCFG_USER_EMITTER_ENABLE
#ifdef TCFG_BT_SUPPORT_HFP_AG
#undef TCFG_BT_SUPPORT_HFP_AG
#endif
#define TCFG_BT_SUPPORT_HFP_AG 1 // HFP AG
#endif

//*********************************************************************************//
//                                 视频配置                                      //
//*********************************************************************************//
//
#define TCFG_VIDEO_DIAL_ENABLE 			0//视频表盘
//uvc
#define UVC_JPG_DATA_WRITE2SD           0//UVC jpg 数据写入SD卡，需要开启SD卡
//camera
#define TCFG_CAMERA_MANAGER_ENABLE		0//相机、相册、摄像头功能
#define TCFG_CAMERA_DEV_BF30A2			0//摄像头驱动bf30a2
#if TCFG_VIDEO_DIAL_ENABLE || TCFG_CAMERA_MANAGER_ENABLE
#define TCFG_APP_VIDEO_EN               1
#endif

//*********************************************************************************//
//                                 USB配置                                      //
//*********************************************************************************//
// #define TCFG_OTG_MODE					0
//*********************************************************************************//
//                                  产测配置                                       //
//*********************************************************************************//
#define PRODUCT_TEST_ENABLE					DISABLE
#define PT_IO_TX_PIN					    IO_PORT_DM
#define PT_IO_RX_PIN					    IO_PORT_DP
#define PT_CHECK_OT                         10000

#define PT_GPIO_ENABLE						DISABLE
#define PT_GSENSOR_ENABLE					DISABLE
#define PT_MOTOR_ENABLE						DISABLE
#define PT_SD_ENABLE						DISABLE
#define PT_HR_ENABLE						DISABLE
#define PT_SPEAKER_MIC_ENABLE				DISABLE

#define PT_GPIO_CHECK_LCD_TP				DISABLE // 使用GPIO检查LCD_TP

#if PRODUCT_TEST_ENABLE
//公版rdec复用了dpdm
#undef TCFG_RDEC_KEY_ENABLE
#define TCFG_RDEC_KEY_ENABLE			DISABLE
#endif

//*********************************************************************************//
//                                 网络配置                                        //
//*********************************************************************************//
#define TCFG_AI_INTERACTION_ENABLE          DISABLE // APP版科大讯飞

#define TCFG_IFLYTEK_ENABLE                 DISABLE // 科大讯飞网络版
#define TCFG_IFLYTEK_USE_PSRAM              DISABLE // 科大讯飞网络版使用psram

#if TCFG_IFLYTEK_ENABLE
#define TCFG_IFLYTEK_APP_SECRET             ""
#define TCFG_IFLYTEK_APP_KEY                ""
#define TCFG_IFLYTEK_APP_ID                 ""
#endif

#if TCFG_IFLYTEK_ENABLE
#ifdef LWIP_USE_BT
#undef  USER_SUPPORT_PROFILE_PAN
#define USER_SUPPORT_PROFILE_PAN    1 //开启pan服务
#endif
#endif




//*********************************************************************************//
//                                 配置结束                                        //
//*********************************************************************************//



#endif //CONFIG_BOARD_JL707N_DEMO
#endif //CONFIG_BOARD_JL707N_DEMO_CFG_H
