#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

// 配置信息基本规则:
// 1、sdk_config.h   是由杰理可视化工具在线配置生成，原则上不建议客户直接修改sdk_config.h的文件，如果客户希望可视化工具不覆盖sdk_config.h，可以配置可视化工具脱机模式
// 2、board_config.h 是唯一包含有sdk_config.h和板卡配置的文件的，用户可以在自己的板卡对配置信息进行完善
// 3、app_config.h   包含有board_config.h
//
#include "sdk_config.h"
#include "../../bringup/ac7076a3_demo_config.h"
#include "audio_type.h"

/*
 *  板级配置选择
 */

#define CONFIG_BOARD_JL707N_DEMO
// #define CONFIG_BOARD_JL7074_DEMO
// #define CONFIG_BOARD_JL707N_CSC_DEMO    //彩屏仓配置

#include "media/audio_def.h"
#include "board_ac707n_demo/board_ac707n_demo_cfg.h"
#include "board_ac7074_demo/board_ac7074_demo_cfg.h"
#include "board_ac707n_csc_demo/board_ac707n_csc_demo_cfg.h"
#include "../../bringup/ac7076a3_demo_overrides.h"


#define  DUT_AUDIO_DAC_LDO_VOLT                 DACVDD_LDO_1_35V

#endif
