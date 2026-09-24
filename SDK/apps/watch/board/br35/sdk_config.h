/**
* 注意点：
* 0.此文件变化，在工具端会自动同步修改到工具配置中
* 1.功能块通过【---------xxx------】和 【#endif // xxx 】，是工具识别的关键位置，请勿随意改动
* 2.目前工具暂不支持非文件已有的C语言语法，此文件应使用文件内已有的语法增加业务所需的代码，避免产生不必要的bug
* 3.修改该文件出现工具同步异常或者导出异常时，请先检查文件内语法是否正常
**/

#ifndef SDK_CONFIG_H
#define SDK_CONFIG_H

#include "jlstream_node_cfg.h"

// ------------JLStudio Auto generate------------
#define PROJECT_CONFIG_NAME                       ""               // 默认配置为空，非默认配置为配置名称
// ------------JLStudio Auto generate------------

// ------------板级配置.json------------
#define TCFG_DEBUG_UART_ENABLE                    1                // 调试串口
#if TCFG_DEBUG_UART_ENABLE
#define TCFG_DEBUG_UART_TX_PIN                    IO_PORTB_03      // 输出IO
#define TCFG_DEBUG_UART_BAUDRATE                  2000000          // 波特率
#define TCFG_EXCEPTION_LOG_ENABLE                 1                // 打印异常信息
#define TCFG_EXCEPTION_RESET_ENABLE               0                // 异常自动复位
#define TCFG_CONFIG_DEBUG_RECORD_ENABLE           0                // 异常存flash
#define TCFG_EXCEPTION_CODE_AT_RAM                1                // 异常代码放RAM
#endif // TCFG_DEBUG_UART_ENABLE

#define TCFG_CFG_TOOL_ENABLE                      0                // FW编辑、在线调音
#if TCFG_CFG_TOOL_ENABLE
#define TCFG_ONLINE_TX_PORT                       IO_PORT_DP       // 串口引脚TX
#define TCFG_ONLINE_RX_PORT                       IO_PORT_DM       // 串口引脚RX
#define TCFG_COMM_TYPE                            TCFG_USB_COMM    // 通信方式
#endif // TCFG_CFG_TOOL_ENABLE

#define CONFIG_SPI_DATA_WIDTH                     4                // flash通信
#define CONFIG_SPI_MODE                           1                // flash模式
#define CONFIG_FLASH_SIZE                         0x800000         // flash容量
#define TCFG_VM_SIZE                              32               // VM大小（K）
#define CONFIG_FLASH_DTR_EN                       1                // flash-dtr使能
#define CONFIG_FLASH_WPS_EN                       0                // flash-上电写保护使能
#define CONFIG_VOTP_SIZE                          256              // votp size（Byte）
#define TCFG_NORFLASH_DEV_ENABLE                  0                // 外挂flash使能
#define CONFIG_EXTERN_FLASH_SIZE                  0                // 外挂flash容量
#define TCFG_PSRAM_DEV_ENABLE                     0                // PSRAM 使能
#define TCFG_PSRAM_SIZE                           0                // PSRAM容量
#define TCFG_NANDFLASH_DEV_ENABLE                 0                // NANDFLASH使能

#define TCFG_SD0_ENABLE                           0                // SD配置
#if TCFG_SD0_ENABLE
#define TCFG_SD0_DAT_MODE                         1                // 线数设置
#define TCFG_SD0_DET_MODE                         SD_IO_DECT       // 检测方式
#define TCFG_SD0_CLK                              12000000         // SD时钟频率
#define TCFG_SD0_DET_IO                           NO_CONFIG_PORT   // 检测IO
#define TCFG_SD0_DET_IO_LEVEL                     0                // IO检测方式
#define TCFG_SD_ALWAY_ONLINE_ENABLE               1                // 卡检测功耗优化
#define TCFG_SD0_POWER_SEL                        SD_PWR_NULL      // SD卡电源
#define TCFG_SDX_CAN_OPERATE_MMC_CARD             0                // 支持MMC卡
#define TCFG_KEEP_CARD_AT_ACTIVE_STATUS           0                // 保持卡活跃状态
#define TCFG_SD0_POWER_PORT                       IO_PORTC_10      // SD卡电源（非sdpg）
#define TCFG_SD0_PORT_CMD                         IO_PORTB_04      // SD_PORT_CMD
#define TCFG_SD0_PORT_CLK                         IO_PORTB_03      // SD_PORT_CLK
#define TCFG_SD0_PORT_DA0                         IO_PORTB_06      // SD_PORT_DATA0
#define TCFG_SD0_PORT_DA1                         NO_CONFIG_PORT   // SD_PORT_DATA1
#define TCFG_SD0_PORT_DA2                         NO_CONFIG_PORT   // SD_PORT_DATA2
#define TCFG_SD0_PORT_DA3                         NO_CONFIG_PORT   // SD_PORT_DATA3
#endif // TCFG_SD0_ENABLE

#define FUSB_MODE                                 1                // USB工作模式
#define TCFG_USB_HOST_ENABLE                      0                // USB主机总开关
#define USB_H_MALLOC_ENABLE                       1                // 主机使用malloc
#define TCFG_USB_HOST_MOUNT_RESET                 40               // usb reset时间
#define TCFG_USB_HOST_MOUNT_TIMEOUT               50               // 握手超时时间
#define TCFG_USB_HOST_MOUNT_RETRY                 3                // 枚举失败重试次数
#define TCFG_UDISK_ENABLE                         1                // U盘使能
#define USB_MALLOC_ENABLE                         1                // 从机使用malloc
#define TCFG_USB_SLAVE_MSD_ENABLE                 1                // 读卡器使能
#define TCFG_USB_SLAVE_HID_ENABLE                 1                // HID使能
#define MSD_BLOCK_NUM                             1                // MSD缓存块数
#define USB_AUDIO_VERSION                         USB_AUDIO_VERSION_1_0 // UAC协议版本
#define TCFG_USB_SLAVE_AUDIO_SPK_ENABLE           1                // USB扬声器使能
#define SPK_AUDIO_RATE_NUM                        1                // SPK采样率列表
#define SPK_AUDIO_RES                             16               // SPK位宽1
#define SPK_AUDIO_RES_2                           0                // SPK位宽2
#define TCFG_USB_SLAVE_AUDIO_MIC_ENABLE           1                // USB麦克风使能
#define MIC_AUDIO_RATE_NUM                        1                // MIC采样率列表
#define MIC_AUDIO_RES                             16               // MIC位宽1
#define MIC_AUDIO_RES_2                           0                // MIC位宽2
#define TCFG_SW_I2C0_CLK_PORT                     IO_PORTA_09      // 软件iic-0 CLK脚
#define TCFG_SW_I2C0_DAT_PORT                     IO_PORTA_10      // 软件iic-0 DATA脚
#define TCFG_SW_I2C0_DELAY_CNT                    50               // iic-0 延时
#define TCFG_SW_I2C1_CLK_PORT                     NO_CONFIG_PORT   // 软件iic-1 CLK脚
#define TCFG_SW_I2C1_DAT_PORT                     NO_CONFIG_PORT   // 软件iic-1 DATA脚
#define TCFG_SW_I2C1_DELAY_CNT                    50               // iic-1 延时
#define TCFG_HW_I2C0_CLK_PORT                     IO_PORTA_06      // 硬件iic-0 CLK脚
#define TCFG_HW_I2C0_DAT_PORT                     IO_PORTA_05      // 硬件iic-0 DATA脚
#define TCFG_HW_I2C0_CLK                          100000           // 硬件iic-0波特率
#define TCFG_HW_I2C1_CLK_PORT                     IO_PORTB_02      // 硬件iic-1 CLK脚
#define TCFG_HW_I2C1_DAT_PORT                     IO_PORTB_01      // 硬件iic -1DATA脚
#define TCFG_HW_I2C1_CLK                          100000           // 硬件iic-1波特率

#define TCFG_HW_SPI1_ENABLE                       1                // 硬件SPI1
#if TCFG_HW_SPI1_ENABLE
#define TCFG_HW_SPI1_PORT_CLK                     IO_PORTA_07      // SPI1 CLK脚
#define TCFG_HW_SPI1_PORT_DO                      IO_PORTA_08      // SPI1  DO脚
#define TCFG_HW_SPI1_PORT_DI                      NO_CONFIG_PORT   // SPI1  DI脚
#define TCFG_HW_SPI1_MODE                         SPI_MODE_BIDIR_1BIT // SPI1 模式
#define TCFG_HW_SPI1_ROLE                         SPI_ROLE_MASTER  // SPI1  ROLE
#define TCFG_HW_SPI1_BAUD                         24000000         // SPI1  时钟
#endif // TCFG_HW_SPI1_ENABLE

#define TCFG_HW_SPI2_ENABLE                       1                // 硬件SPI2
#if TCFG_HW_SPI2_ENABLE
#define TCFG_HW_SPI2_PORT_CLK                     IO_PORTC_00      // SPI2 CLK脚
#define TCFG_HW_SPI2_PORT_DO                      IO_PORTC_10      // SPI2  DO脚
#define TCFG_HW_SPI2_PORT_DI                      IO_PORTC_02      // SPI2  DI脚
#define TCFG_HW_SPI2_PORT_D2                      IO_PORTC_03      // SPI2  D2脚
#define TCFG_HW_SPI2_PORT_D3                      IO_PORTA_13      // SPI2  D3脚
#define TCFG_HW_SPI2_MODE                         SPI_MODE_BIDIR_1BIT // SPI2 模式
#define TCFG_HW_SPI2_ROLE                         SPI_ROLE_MASTER  // SPI2  ROLE
#define TCFG_HW_SPI2_BAUD                         32000000         // SPI2  时钟
#endif // TCFG_HW_SPI2_ENABLE
// ------------板级配置.json------------

// ------------功能配置.json------------
#define TCFG_APP_BT_EN                            1                // 蓝牙模式
#define TCFG_APP_MUSIC_EN                         0                // 音乐模式
#define TCFG_APP_LINEIN_EN                        0                // LINEIN模式
#define TCFG_APP_FM_EN                            0                // FM模式
#define TCFG_APP_PC_EN                            0                // PC模式
#define TCFG_APP_RECORD_EN                        0                // 录音模式
#define TCFG_MIC_EFFECT_ENABLE                    0                // 混响使能
#define TCFG_DEC_ID3_V2_ENABLE                    0                // ID3_V2
#define TCFG_DEC_ID3_V1_ENABLE                    0                // ID3_V1
#define FILE_DEC_REPEAT_EN                        0                // 无缝循环播放
#define FILE_DEC_DEST_PLAY                        0                // 指定时间播放
#define FILE_DEC_AB_REPEAT_EN                     0                // AB点复读
#define TCFG_DEC_DECRYPT_ENABLE                   0                // 加密文件播
#define TCFG_DEC_DECRYPT_KEY                      0x12345678       // 加密KEY
#define MUSIC_PLAYER_CYCLE_ALL_DEV_EN             0                // 循环播放模式是否循环所有设备
#define MUSIC_PLAYER_PLAY_FOLDER_PREV_FIRST_FILE_EN 0                // 切换文件夹播放时从第一首歌开始
#define TCFG_MUSIC_DEVICE_TONE_EN                 0                // 设备提示音
#define TWFG_APP_POWERON_IGNORE_DEV               4000             // 设备忽略时间（单位：ms）
#define TCFG_FIX_CLOCK_FREQ                       0                // 固定时钟频率
#define TCFG_MIX_RECORD_ENABLE                    0                // 本地混响录音使能
#define TCFG_RECORD_AUDIO_REPLAY_EN               0                // 本地录音回播使能
#define TCFG_AI_VOICE_ENABLE                      0                // AI VOICE录音使能
// ------------功能配置.json------------

// ------------按键配置.json------------
#define TCFG_LONG_PRESS_RESET_ENABLE              0                // 非按键长按复位配置
#if TCFG_LONG_PRESS_RESET_ENABLE
#define TCFG_LONG_PRESS_RESET_PORT                IO_PORTB_01      // 复位IO
#define TCFG_LONG_PRESS_RESET_TIME                8                // 复位时间
#define TCFG_LONG_PRESS_RESET_LEVEL               1                // 复位电平
#define TCFG_LONG_PRESS_RESET_INSIDE_PULL_UP_DOWN 0                // 使用IO内置上下拉
#endif // TCFG_LONG_PRESS_RESET_ENABLE

#define TCFG_IOKEY_ENABLE                         0                // IO按键配置

#define TCFG_ADKEY_ENABLE                         1                // AD按键配置

#define TCFG_IRKEY_ENABLE                         0                // IR按键配置

// ------------按键配置.json------------

// ------------电源配置.json------------
#define TCFG_CLOCK_OSC_HZ                         24000000         // 晶振频率
#define TCFG_LOWPOWER_POWER_SEL                   PWR_DCDC15       // 电源模式
#define TCFG_LOWPOWER_OSC_TYPE                    OSC_TYPE_LRC     // 低功耗时钟源
#define TCFG_CLOCK_MODE                           CLOCK_MODE_ADAPTIVE // 时钟模式
#define TCFG_LOWPOWER_VDDIOM_LEVEL                VDDIOM_VOL_32V   // IOVDD
#define TCFG_LOWPOWER_VDDIOW_LEVEL                VDDIOM_VOL_26V   // 弱IOVDD
#define CONFIG_LVD_LEVEL                          2.5v             // LVD档位
#define TCFG_LOWPOWER_LOWPOWER_SEL                2                // 低功耗模式
#define TCFG_AUTO_POWERON_ENABLE                  1                // 上电自动开机

#define TCFG_SYS_LVD_EN                           1                // 电池电量检测
#if TCFG_SYS_LVD_EN
#define TCFG_POWER_OFF_VOLTAGE                    3500             // 关机电压(mV)
#define TCFG_POWER_WARN_VOLTAGE                   3600             // 低电电压(mV)
#endif // TCFG_SYS_LVD_EN

#define TCFG_CHARGE_ENABLE                        0                // 充电配置
#if TCFG_CHARGE_ENABLE
#define TCFG_CHARGE_TRICKLE_MA                    CHARGE_mA_20     // 涓流电流
#define TCFG_CHARGE_MA                            CHARGE_mA_200    // 恒流电流
#define TCFG_CHARGE_FULL_MA                       20               // 截止电流(mA)
#define TCFG_CHARGE_FULL_V                        CHARGE_FULL_V_4200_4P2V // 截止电压
#define TCFG_CHARGE_POWERON_ENABLE                1                // 开机充电
#define TCFG_CHARGE_OFF_POWERON_EN                1                // 拔出开机
#define TCFG_CHARGE_NVDC_EN                       1                // NVDC架构使能
#define TCFG_RECHARGE_ENABLE                      0                // 复充使能
#define TCFG_RECHARGE_VOLTAGE                     4000             // 复充电压(mV)
#define TCFG_LDOIN_PULLDOWN_EN                    1                // 下拉电阻开关
#define TCFG_LDOIN_PULLDOWN_LEV                   CHARGE_PULLDOWN_200K // 下拉电阻档位
#define TCFG_LDOIN_PULLDOWN_KEEP                  0                // 下拉电阻保持开关
#define TCFG_LDOIN_ON_FILTER_TIME                 100              // 入舱滤波时间(ms)
#define TCFG_LDOIN_OFF_FILTER_TIME                200              // 出舱滤波时间(ms)
#define TCFG_LDOIN_KEEP_FILTER_TIME               440              // 维持电压滤波时间(ms)
#endif // TCFG_CHARGE_ENABLE

#define TCFG_BATTERY_CURVE_ENABLE                 1                // 电池曲线配置

// ------------电源配置.json------------

// ------------LCD_UI配置.json------------
#define TCFG_UI_ENABLE                            1                // UI配置
#if TCFG_UI_ENABLE
#define CONFIG_JL_UI_ENABLE                       1                // 杰理UI
#define CONFIG_LVGL_UI_ENABLE                     0                // LVGL（未完善）
#define TCFG_LCD_PIN_RESET                        IO_PORTC_03      // LCD RESET 
#define TCFG_LCD_PIN_CS                           IO_PORTA_07      // LCD CS
#define TCFG_LCD_PIN_BL                           IO_LCD_PG        // LCD BLCKLIGHT
#define TCFG_LCD_PIN_DC                           IO_PORTA_02      // LCD DC
#define TCFG_LCD_PIN_EN                           NO_CONFIG_PORT   // LCD EN
#define TCFG_LCD_PIN_TE                           IO_PORTC_00      // LCD TE
#endif // TCFG_UI_ENABLE
// ------------LCD_UI配置.json------------

// ------------蓝牙配置.json------------
#define TCFG_BT_NAME_SEL_BY_AD_ENABLE             0                // 蓝牙名(AD采样)

#define TCFG_USER_BT_CLASSIC_ENABLE               1                // 经典蓝牙开关
#define TCFG_BT_PAGE_TIMEOUT                      8                // 单次回连时间(s)
#define TCFG_BT_POWERON_PAGE_TIME                 30               // 开机回连超时(s)
#define TCFG_BT_TIMEOUT_PAGE_TIME                 120              // 超距断开回连超时(s)
#define TCFG_DUAL_CONN_INQUIRY_SCAN_TIME          120              // 开机可被发现时间 (s)
#define TCFG_AUTO_SHUT_DOWN_TIME                  0                // 无连接关机时间(s)
#define TCFG_BT_VOL_SYNC_ENABLE                   1                // 音量同步
#define TCFG_BT_MUSIC_INFO_ENABLE                 1                // 歌曲信息显示
#define TCFG_BT_DISPLAY_BAT_ENABLE                0                // 电量显示
#define TCFG_BT_INBAND_RING                       1                // 手机铃声
#define TCFG_BT_PHONE_NUMBER_ENABLE               0                // 来电报号
#define TCFG_BT_SUPPORT_AAC                       0                // AAC
#define TCFG_BT_MSBC_EN                           1                // MSBC
#define TCFG_BT_SBC_BITPOOL                       38               // sbcBitPool
#define TCFG_AAA_BITRATE                          320000           // AAC码率
#define TCFG_BT_SUPPORT_HFP                       1                // HFP
#define TCFG_BT_SUPPORT_AVCTP                     1                // AVRCP
#define TCFG_BT_SUPPORT_A2DP                      1                // A2DP
#define TCFG_BT_SUPPORT_HID                       1                // HID
#define TCFG_BT_SUPPORT_SPP                       1                // SPP
#define TCFG_BT_SUPPORT_PNP                       1                // PNP
#define TCFG_BT_SUPPORT_PBAP                      0                // PBAP
#define TCFG_BT_SUPPORT_MAP                       1                // MAP
#define TCFG_BT_SUPPORT_OPP                       0                // OPP
#define TCFG_BT_BACKGROUND_ENABLE                 1                // 蓝牙后台
#define TCFG_BT_SUPPORT_PBAP_LIST                 1                // pbap单条查询
#define TCFG_BT_CALL_PHONE_BY_WATCH               1                // 通话拦截
#define TCFG_USER_EMITTER_ENABLE                  0                // 发射器开关
#define CONFIG_BT_MODE                            1                // 模式选择
#define TCFG_NORMAL_SET_DUT_MODE                  0                // NORMAL模式下使能DUT测试

#define TCFG_BT_SNIFF_ENABLE                      1                // sniff
#if TCFG_BT_SNIFF_ENABLE
#define TCFG_SNIFF_CHECK_TIME                     5                // 检测时间
#define CONFIG_LRC_WIN_SIZE                       400              // LRC窗口初值
#define CONFIG_LRC_WIN_STEP                       400              // LRC窗口步进
#define CONFIG_OSC_WIN_SIZE                       400              // OSC窗口初值
#define CONFIG_OSC_WIN_STEP                       400              // OSC窗口步进
#endif // TCFG_BT_SNIFF_ENABLE

#define TCFG_USER_BLE_ENABLE                      1                // BLE
#if TCFG_USER_BLE_ENABLE
#define TCFG_BT_BLE_TX_POWER                      5                // 最大发射功率
#define TCFG_BT_BLE_BREDR_SAME_ADDR               1                // 与2.1同地址
#define TCFG_BLE_BRIDGE_EDR_ENALBE                1                // CTKD一键连接
#define TCFG_USER_BLE_CTRL_BREDR_EN               1                // BLE控制EDR
#define TCFG_BLE_ADV_DYNAMIC_SWITCH               1                // BLE广播动态调整
#endif // TCFG_USER_BLE_ENABLE

#define TCFG_BT_AI_ENABLE                         1                // AI配置
#if TCFG_BT_AI_ENABLE
#define TCFG_BT_AI_SEL_PROTOCOL                   RCSP_MODE_EN     // AI协议选择
#endif // TCFG_BT_AI_ENABLE
// ------------蓝牙配置.json------------

// ------------升级配置.json------------
#define TCFG_UPDATE_ENABLE                        1                // 升级选择
#if TCFG_UPDATE_ENABLE
#define TCFG_TEST_BOX_ENABLE                      0                // 测试盒串口升级
#define TCFG_UPDATE_UART_IO_EN                    0                // 普通io串口升级
#define TCFG_UPDATE_UART_ROLE                     0                // 串口升级主从机选择
#define TCFG_DEV_UPDATE_EN                        0                // 设备升级（USB、SD卡）
#define TCFG_COMPELTE_UFW_NAND_EN                 0                // 升级文件存放本地触发升级（nandflash）
#define TCFG_COMPELTE_UFW_NOR_EN                  0                // 升级文件存放本地触发升级（norflash升级）
#define TCFG_COMPELTE_UFW_LC_EN                   0                // 升级文件存放本地触发升级（内置flash升级）
#define TCFG_NORFLASH_UPDATE_EN                   0                // norflash_flash升级，目前功能没用到
#define TCFG_UPDATE_RESOURCE_EN                   1                // 升级资源（除代码以外的其他部分）
#define TCFG_TEST_BOX_WIRELESS_EN                 1                // 测试盒无线升级
#define TCFG_APP_UPDATE_EN                        1                // APP升级
#endif // TCFG_UPDATE_ENABLE
// ------------升级配置.json------------

// ------------音频配置.json------------
#define TCFG_AUDIO_VCM_LEVEL                      1                // VCM电压等级
#define TCFG_AUDIO_VCM_CAP_EN                     0                // VCM电容
#define TCFG_AUDIO_CLASSD_OUTPUT                  1                // 输出模式
#define TCFG_AUDIO_CLASSD_BUFFER_TIME_MS          50               // 缓冲长度(ms)

#define TCFG_AUDIO_ADC_ENABLE                     1                // ADC配置
#if TCFG_AUDIO_ADC_ENABLE
#define TCFG_AUDIO_MIC_LDO_VSEL                   4                // MIC LDO 电压
#define TCFG_ADC_PERFORMANCE_MODE                 0                // 性能模式
#define TCFG_AUDIO_PLNK_SCLK_PIN                  NO_CONFIG_PORT   // SCLK_IO
#define TCFG_AUDIO_PLNK_DAT0_PIN                  NO_CONFIG_PORT   // DAT0_IO
#define TCFG_AUDIO_PLNK_DAT1_PIN                  NO_CONFIG_PORT   // DAT1_IO
#define TCFG_AUDIO_PLNK_PWR_PORT                  NO_CONFIG_PORT   // IO供电选择
#define TCFG_ADC0_ENABLE                          1                // 使能
#define TCFG_ADC0_MODE                            2                // 模式
#define TCFG_ADC0_AIN_SEL                         1                // 输入端口
#define TCFG_ADC0_BIAS_SEL                        1                // 供电端口
#define TCFG_ADC0_BIAS_RSEL                       4                // MIC BIAS上拉电阻挡位
#define TCFG_ADC0_DCC_LEVEL                       8                // DCC 档位
#define TCFG_ADC0_POWER_IO                        0                // IO供电选择
#endif // TCFG_AUDIO_ADC_ENABLE

#define TCFG_AUDIO_GLOBAL_SAMPLE_RATE             48000            // 全局采样率
#define TCFG_AEC_TOOL_ONLINE_ENABLE               0                // 手机APP在线调试
#define TCFG_AUDIO_CVP_SYNC                       0                // 通话上行同步
#define TCFG_AUDIO_DMS_DUT_ENABLE                 1                // 通话产测
#define TCFG_ESCO_DL_CVSD_SR_USE_16K              1                // ESCO通话全局采样率
#define TCFG_AUDIO_SMS_SEL                        SMS_DEFAULT      // 1mic算法选择
#define TCFG_MUSIC_PLC_TYPE                       1                // PLC类型选择
#define TCFG_DEC_WAV_ENABLE                       0                // WAV
#define TCFG_DEC_MP3_ENABLE                       0                // MP3
#define TCFG_DEC_AAC_ENABLE                       0                // AAC
#define TCFG_DEC_F2A_ENABLE                       0                // F2A
#define TCFG_DEC_WTG_ENABLE                       1                // WTG
#define TCFG_DEC_MTY_ENABLE                       0                // MTY
#define TCFG_DEC_WTS_ENABLE                       0                // WTS
#define TCFG_DEC_JLA_ENABLE                       0                // JLA
#define TCFG_DEC_STREAM_MP3_ENABLE                0                // STREAM MP3
#define TCFG_ENC_MSBC_ENABLE                      1                // MSBC
#define TCFG_ENC_CVSD_ENABLE                      1                // CVSD
#define TCFG_ENC_JLA_ENABLE                       0                // JLA
#define TCFG_ENC_SBC_ENABLE                       1                // SBC
#define TCFG_ENC_PCM_ENABLE                       0                // PCM
#define TCFG_ENC_OPUS_ENABLE                      0                // OPUS
#define TCFG_ENC_SPEEX_ENABLE                     0                // SPEEX
#define TCFG_ENC_MP3_ENABLE                       0                // MP3
#define TCFG_ENC_MP3_TYPE                         0                // MP3格式选择
#define TCFG_ENC_ADPCM_ENABLE                     0                // ADPCM
#define TCFG_ENC_ADPCM_TYPE                       0                // ADPCM格式选择
// ------------音频配置.json------------
#endif
