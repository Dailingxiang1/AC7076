#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".app_config.data.bss")
#pragma data_seg(".app_config.data")
#pragma const_seg(".app_config.text.const")
#pragma code_seg(".app_config.text")
#endif
/*********************************************************************************************
    *   Filename        : log_config.c

    *   Description     : Optimized Code & RAM (编译优化Log配置)

    *   Author          : Bingquan

    *   Email           : caibingquan@zh-jieli.com

    *   Last modifiled  : 2019-03-18 14:45

    *   Copyright:(c)JIELI  2011-2019  @ , All Rights Reserved.
*********************************************************************************************/
#include "system/includes.h"

/**
 * @brief Bluetooth Controller Log
 */
/*-----------------------------------------------------------*/
const char log_tag_const_v_SETUP  = FALSE;
const char log_tag_const_i_SETUP  = FALSE;
const char log_tag_const_w_SETUP  = FALSE;
const char log_tag_const_d_SETUP  = TRUE;
const char log_tag_const_e_SETUP  = TRUE;

const char log_tag_const_v_BOARD  = FALSE;
const char log_tag_const_i_BOARD  = TRUE;
const char log_tag_const_d_BOARD  = TRUE;
const char log_tag_const_w_BOARD  = TRUE;
const char log_tag_const_e_BOARD  = TRUE;

const char log_tag_const_v_EARPHONE   = FALSE;
const char log_tag_const_i_EARPHONE   = TRUE;
const char log_tag_const_d_EARPHONE   = FALSE;
const char log_tag_const_w_EARPHONE   = TRUE;
const char log_tag_const_e_EARPHONE   = TRUE;

const char log_tag_const_v_UI  = FALSE;
const char log_tag_const_i_UI  = TRUE;
const char log_tag_const_d_UI  = FALSE;
const char log_tag_const_w_UI  = TRUE;
const char log_tag_const_e_UI  = TRUE;

const char log_tag_const_v_APP_CHARGE  = FALSE;
const char log_tag_const_i_APP_CHARGE  = TRUE;
const char log_tag_const_d_APP_CHARGE  = FALSE;
const char log_tag_const_w_APP_CHARGE  = TRUE;
const char log_tag_const_e_APP_CHARGE  = TRUE;

const char log_tag_const_v_APP_ANCBOX  = FALSE;
const char log_tag_const_i_APP_ANCBOX  = TRUE;
const char log_tag_const_d_APP_ANCBOX  = FALSE;
const char log_tag_const_w_APP_ANCBOX  = TRUE;
const char log_tag_const_e_APP_ANCBOX  = TRUE;

const char log_tag_const_v_KEY_EVENT_DEAL  = FALSE;
const char log_tag_const_i_KEY_EVENT_DEAL  = TRUE;
const char log_tag_const_d_KEY_EVENT_DEAL  = FALSE;
const char log_tag_const_w_KEY_EVENT_DEAL  = TRUE;
const char log_tag_const_e_KEY_EVENT_DEAL  = TRUE;

const char log_tag_const_v_APP_CHARGESTORE  = FALSE;
const char log_tag_const_i_APP_CHARGESTORE  = TRUE;
const char log_tag_const_d_APP_CHARGESTORE  = FALSE;
const char log_tag_const_w_APP_CHARGESTORE  = TRUE;
const char log_tag_const_e_APP_CHARGESTORE  = TRUE;

const char log_tag_const_v_APP_UMIDIGI_CHARGESTORE  = FALSE;
const char log_tag_const_i_APP_UMIDIGI_CHARGESTORE  = TRUE;
const char log_tag_const_d_APP_UMIDIGI_CHARGESTORE  = FALSE;
const char log_tag_const_w_APP_UMIDIGI_CHARGESTORE  = TRUE;
const char log_tag_const_e_APP_UMIDIGI_CHARGESTORE  = TRUE;

const char log_tag_const_v_APP_TESTBOX  = FALSE;
const char log_tag_const_i_APP_TESTBOX  = TRUE;
const char log_tag_const_d_APP_TESTBOX  = FALSE;
const char log_tag_const_w_APP_TESTBOX  = TRUE;
const char log_tag_const_e_APP_TESTBOX  = TRUE;

const char log_tag_const_v_APP_IDLE  = FALSE;
const char log_tag_const_i_APP_IDLE  = TRUE;
const char log_tag_const_d_APP_IDLE  = FALSE;
const char log_tag_const_w_APP_IDLE  = TRUE;
const char log_tag_const_e_APP_IDLE  = TRUE;

const char log_tag_const_v_APP_POWER  = FALSE;
const char log_tag_const_i_APP_POWER  = TRUE;
const char log_tag_const_d_APP_POWER  = FALSE;
const char log_tag_const_w_APP_POWER  = TRUE;
const char log_tag_const_e_APP_POWER  = TRUE;

const char log_tag_const_v_APP  = FALSE;
const char log_tag_const_i_APP  = TRUE;
const char log_tag_const_d_APP  = FALSE;
const char log_tag_const_w_APP  = TRUE;
const char log_tag_const_e_APP  = TRUE;

const char log_tag_const_v_USER_CFG  = FALSE;
const char log_tag_const_i_USER_CFG  = TRUE;
const char log_tag_const_d_USER_CFG  = FALSE;
const char log_tag_const_w_USER_CFG  = TRUE;
const char log_tag_const_e_USER_CFG  = TRUE;

const char log_tag_const_v_APP_TONE  = FALSE;
const char log_tag_const_i_APP_TONE  = TRUE;
const char log_tag_const_d_APP_TONE  = FALSE;
const char log_tag_const_w_APP_TONE  = TRUE;
const char log_tag_const_e_APP_TONE  = TRUE;

const char log_tag_const_v_BT_TWS  = FALSE;
const char log_tag_const_i_BT_TWS  = TRUE;
const char log_tag_const_d_BT_TWS  = FALSE;
const char log_tag_const_w_BT_TWS  = TRUE;
const char log_tag_const_e_BT_TWS  = TRUE;

const char log_tag_const_v_AEC_USER  = FALSE;
const char log_tag_const_i_AEC_USER  = TRUE;
const char log_tag_const_d_AEC_USER  = FALSE;
const char log_tag_const_w_AEC_USER  = TRUE;
const char log_tag_const_e_AEC_USER  = TRUE;

const char log_tag_const_v_BT_BLE  = FALSE;
const char log_tag_const_i_BT_BLE  = FALSE;
const char log_tag_const_d_BT_BLE  = TRUE;
const char log_tag_const_w_BT_BLE  = TRUE;
const char log_tag_const_e_BT_BLE  = TRUE;

const char log_tag_const_v_EARTCH_EVENT_DEAL  = FALSE;
const char log_tag_const_i_EARTCH_EVENT_DEAL  = TRUE;
const char log_tag_const_d_EARTCH_EVENT_DEAL  = FALSE;
const char log_tag_const_w_EARTCH_EVENT_DEAL  = TRUE;
const char log_tag_const_e_EARTCH_EVENT_DEAL  = TRUE;

const char log_tag_const_v_ONLINE_DB  = FALSE;
const char log_tag_const_i_ONLINE_DB  = FALSE;
const char log_tag_const_d_ONLINE_DB  = FALSE;
const char log_tag_const_w_ONLINE_DB  = FALSE;
const char log_tag_const_e_ONLINE_DB  = TRUE;

const char log_tag_const_v_MUSIC  = FALSE;
const char log_tag_const_i_MUSIC  = TRUE;
const char log_tag_const_w_MUSIC  = FALSE;
const char log_tag_const_d_MUSIC  = TRUE;
const char log_tag_const_e_MUSIC  = TRUE;

const char log_tag_const_v_APP_MUSIC  = FALSE;
const char log_tag_const_i_APP_MUSIC  = TRUE;
const char log_tag_const_d_APP_MUSIC  = FALSE;
const char log_tag_const_w_APP_MUSIC  = TRUE;
const char log_tag_const_e_APP_MUSIC  = TRUE;

const char log_tag_const_v_PC  = FALSE;
const char log_tag_const_i_PC  = TRUE;
const char log_tag_const_w_PC  = FALSE;
const char log_tag_const_d_PC  = TRUE;
const char log_tag_const_e_PC  = TRUE;

const char log_tag_const_v_WIRELESSMIC  = FALSE;
const char log_tag_const_i_WIRELESSMIC  = TRUE;
const char log_tag_const_w_WIRELESSMIC  = FALSE;
const char log_tag_const_d_WIRELESSMIC  = TRUE;
const char log_tag_const_e_WIRELESSMIC  = TRUE;

const char log_tag_const_v_KWS_VOICE_EVENT  = FALSE;
const char log_tag_const_i_KWS_VOICE_EVENT  = TRUE;
const char log_tag_const_d_KWS_VOICE_EVENT  = TRUE;
const char log_tag_const_w_KWS_VOICE_EVENT  = TRUE;
const char log_tag_const_e_KWS_VOICE_EVENT  = TRUE;

const char log_tag_const_v_APP_CFG_TOOL  = FALSE;
const char log_tag_const_i_APP_CFG_TOOL  = TRUE;
const char log_tag_const_d_APP_CFG_TOOL  = FALSE;
const char log_tag_const_w_APP_CFG_TOOL  = TRUE;
const char log_tag_const_e_APP_CFG_TOOL  = TRUE;

const char log_tag_const_v_APP_FM  = FALSE;
const char log_tag_const_i_APP_FM  = TRUE;
const char log_tag_const_d_APP_FM  = FALSE;
const char log_tag_const_w_APP_FM  = TRUE;
const char log_tag_const_e_APP_FM  = TRUE;

const char log_tag_const_v_APP_LINEIN  = FALSE;
const char log_tag_const_i_APP_LINEIN  = TRUE;
const char log_tag_const_d_APP_LINEIN  = FALSE;
const char log_tag_const_w_APP_LINEIN  = TRUE;
const char log_tag_const_e_APP_LINEIN  = TRUE;

const char log_tag_const_v_APP_PC  = FALSE;
const char log_tag_const_i_APP_PC  = TRUE;
const char log_tag_const_d_APP_PC  = FALSE;
const char log_tag_const_w_APP_PC  = TRUE;
const char log_tag_const_e_APP_PC  = TRUE;

const char log_tag_const_v_APP_RTC  = FALSE;
const char log_tag_const_i_APP_RTC  = TRUE;
const char log_tag_const_d_APP_RTC  = FALSE;
const char log_tag_const_w_APP_RTC  = TRUE;
const char log_tag_const_e_APP_RTC  = TRUE;

const char log_tag_const_v_UI_TASK  = FALSE;
const char log_tag_const_i_UI_TASK  = TRUE;
const char log_tag_const_d_UI_TASK  = FALSE;
const char log_tag_const_w_UI_TASK  = TRUE;
const char log_tag_const_e_UI_TASK  = TRUE;
const char log_tag_const_c_UI_TASK  = TRUE;

const char log_tag_const_v_DATA_STORAGE  = FALSE;
const char log_tag_const_i_DATA_STORAGE  = TRUE;
const char log_tag_const_d_DATA_STORAGE  = FALSE;
const char log_tag_const_c_DATA_STORAGE  = TRUE;
const char log_tag_const_w_DATA_STORAGE  = TRUE;
const char log_tag_const_e_DATA_STORAGE  = TRUE;

const char log_tag_const_v_RCSP_ADAPTOR  = FALSE;
const char log_tag_const_i_RCSP_ADAPTOR  = TRUE;
const char log_tag_const_d_RCSP_ADAPTOR  = TRUE;
const char log_tag_const_w_RCSP_ADAPTOR  = TRUE;
const char log_tag_const_e_RCSP_ADAPTOR  = TRUE;

const char log_tag_const_v_SPORT_HEALTH_MANAGE  = FALSE;
const char log_tag_const_i_SPORT_HEALTH_MANAGE  = FALSE;
const char log_tag_const_d_SPORT_HEALTH_MANAGE  = FALSE;
const char log_tag_const_w_SPORT_HEALTH_MANAGE  = TRUE;
const char log_tag_const_e_SPORT_HEALTH_MANAGE  = TRUE;

const char log_tag_const_v_SENSOR_HUB  = FALSE;
const char log_tag_const_i_SENSOR_HUB  = FALSE;
const char log_tag_const_d_SENSOR_HUB  = FALSE;
const char log_tag_const_w_SENSOR_HUB  = TRUE;
const char log_tag_const_e_SENSOR_HUB  = TRUE;

const char log_tag_const_v_RCSP  = FALSE;
const char log_tag_const_i_RCSP  = TRUE;
const char log_tag_const_d_RCSP  = TRUE;
const char log_tag_const_c_RCSP  = TRUE;
const char log_tag_const_w_RCSP  = TRUE;
const char log_tag_const_e_RCSP  = TRUE;

const char log_tag_const_v_PRODUCT_TEST  = FALSE;
const char log_tag_const_i_PRODUCT_TEST  = TRUE;
const char log_tag_const_d_PRODUCT_TEST  = FALSE;
const char log_tag_const_w_PRODUCT_TEST  = TRUE;
const char log_tag_const_e_PRODUCT_TEST  = TRUE;

const char log_tag_const_v_APP_CHGBOX = FALSE;
const char log_tag_const_i_APP_CHGBOX = TRUE;
const char log_tag_const_d_APP_CHGBOX = FALSE;
const char log_tag_const_w_APP_CHGBOX = TRUE;
const char log_tag_const_e_APP_CHGBOX = TRUE;

const char log_tag_const_v_EARPHONE_PROT = FALSE;
const char log_tag_const_i_EARPHONE_PROT = TRUE;
const char log_tag_const_d_EARPHONE_PROT = FALSE;
const char log_tag_const_w_EARPHONE_PROT = TRUE;
const char log_tag_const_e_EARPHONE_PROT = TRUE;

const char log_tag_const_v_RCSP_CLIENT  = FALSE;
const char log_tag_const_i_RCSP_CLIENT  = TRUE;
const char log_tag_const_d_RCSP_CLIENT  = TRUE;
const char log_tag_const_c_RCSP_CLIENT  = TRUE;
const char log_tag_const_w_RCSP_CLIENT  = TRUE;
const char log_tag_const_e_RCSP_CLIENT  = TRUE;

const char log_tag_const_v_NET_IFLY  = FALSE;
const char log_tag_const_i_NET_IFLY  = TRUE;
const char log_tag_const_d_NET_IFLY  = FALSE;
const char log_tag_const_w_NET_IFLY  = TRUE;
const char log_tag_const_e_NET_IFLY  = TRUE;


const char log_tag_const_v_NET_DUER  = FALSE;
const char log_tag_const_i_NET_DUER  = TRUE;
const char log_tag_const_d_NET_DUER  = FALSE;
const char log_tag_const_w_NET_DUER  = TRUE;
const char log_tag_const_e_NET_DUER  = TRUE;

const char log_tag_const_v_NET_INTERFACE  = FALSE;
const char log_tag_const_i_NET_INTERFACE  = TRUE;
const char log_tag_const_d_NET_INTERFACE  = FALSE;
const char log_tag_const_w_NET_INTERFACE  = TRUE;
const char log_tag_const_e_NET_INTERFACE  = TRUE;

