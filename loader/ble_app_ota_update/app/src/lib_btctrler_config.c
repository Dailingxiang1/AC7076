#ifdef SUPPORT_MS_EXTENSIONS_APP
#pragma bss_seg(".lib_btctrler_config.data.bss")
#pragma data_seg(".lib_btctrler_config.data")
#pragma const_seg(".lib_btctrler_config.text.const")
#pragma code_seg(".lib_btctrler_config.text")
#endif
//#include "system/includes.h"
#include "btcontroller_config.h"
#include "btstack/bt_profile_config.h"
#include "printf.h"


const int config_stack_modules = BT_BTSTACK_LE;

#ifdef EDR_UPDATA_SUPPORT_CONNECT
const int config_btctler_modules =  BT_MODULE_CLASSIC;
#elif defined (BLE_UPDATA_SUPPORT_CONNECT) || defined (BLE_GATT_UPDATA_MODULE_CONTROL)
const int config_btctler_modules =  BT_MODULE_LE;
#else
const int config_btctler_modules = 0;
#endif

const int config_rf_slot_unit = 625;

const int config_btctler_le_tws      	= 0;
const int CONFIG_BTCTLER_TWS_ENABLE		= 0;
const int config_btctler_mode        	= BT_NORMAL;
const int config_btctler_hci_standard	= 0;
const int CONFIG_BLE_SYNC_WORD_BIT = 30;

const int clock_tree_auto_disable_all_pll = 0;
#ifdef BLE_UPDATA_SUPPORT_CONNECT
const int config_btctler_single_carrier_en = 1;
#else
const int config_btctler_single_carrier_en = 0;
#endif
const u8 ble_debug_enable    = 0x00;
const u8 rx_fre_offset_adjust_enable  = 1;

const int CONFIG_PAGE_POWER                 = 4;
const int CONFIG_PAGE_SCAN_POWER            = 7;
const int CONFIG_INQUIRY_POWER              = 7;
const int CONFIG_INQUIRY_SCAN_POWER         = 7;

const int CONFIG_TEST_DUT_CODE            = 0;
const int CONFIG_TEST_FCC_CODE            = 0;
const int CONFIG_TEST_DUT_ONLY_BOX_CODE   = 0;

#if defined (BLE_GATT_UPDATA_MODULE_CONTROL)
const uint64_t config_btctler_le_features = LE_2M_PHY | LE_DATA_PACKET_LENGTH_EXTENSION;//(LE_ENCRYPTION);
#else
const uint64_t config_btctler_le_features = LE_DATA_PACKET_LENGTH_EXTENSION;
#endif

const int config_btctler_le_roles    = (LE_ADV | LE_SLAVE);

//固定使用正常发射功率的等级:0-使用不同模式的各自等级;1~10-固定发射功率等级
const int config_force_bt_pwr_tab_using_normal_level  = 0;

const int config_btctler_eir_version_info_len = 21;

const int CONFIG_WIFI_DETECT_ENABLE = 0;

const int config_btctler_le_afh_en = 0;
const int config_btctler_le_rx_nums = 8;
const int config_btctler_le_acl_packet_length = 251;
const int config_btctler_le_acl_total_nums = 3;
const int config_btctler_le_hw_nums = 1;

const int config_update_mode = 0xff;

//BT
const int IRQ_BT_TIMEBASE_IP  = 6;
const int IRQ_BLE_EVENT_IP  = 5;
const int IRQ_BLE_RX_IP  = 5;
const int IRQ_BREDR_IP  = 4;
const int IRQ_BT_RXMCH_IP  = 4;

const int config_rf_oob = 0;
#if defined (BLE_GATT_UPDATA_MODULE_CONTROL) || defined (BLE_UPDATA_SUPPORT_CONNECT)
#if defined(CONFIG_CPU_BD47) || defined(CONFIG_CPU_BR29)
#if defined(BLE_APP_LOW_RAM_USED)  // bd47内存紧缺
static u8 app_le_pool[800] __attribute__((aligned(4)));
#else
static u8 app_le_pool[1300] __attribute__((aligned(4)));
#endif
#else
static u8 app_le_pool[1820] __attribute__((aligned(4)));
#endif
extern u16 get_le_pool_len(void);
u8 *get_le_pool_addr(void)
{
    u16 len = 0;

    len = get_le_pool_len();
    printf("le pool len %d\n", len);
    if (len > sizeof(app_le_pool)) {
        ASSERT(0, "le_pool is small\n");
    }

    return &app_le_pool;
}
#endif

const u32 __VERSION_BEGIN = 0;

const char log_tag_const_v_Analog  = 0;
const char log_tag_const_i_Analog  = 0;
const char log_tag_const_w_Analog  = 0;
const char log_tag_const_d_Analog  = 0;
const char log_tag_const_e_Analog  = 0;

const char log_tag_const_v_LL_ISO  = 0;
const char log_tag_const_i_LL_ISO  = 0;
const char log_tag_const_d_LL_ISO  = 0;
const char log_tag_const_w_LL_ISO  = 0;
const char log_tag_const_e_LL_ISO  = 0;

const char log_tag_const_i_LL_EXT_ADV = 0;
const char log_tag_const_e_LL_EXT_ADV = 0;
const char log_tag_const_d_LL_EXT_ADV = 0;

const char log_tag_const_i_LL_PADV = 0;

const char log_tag_const_i_AES = 0;

const int config_btctler_dual_a2dp = 0;
const int CONFIG_LMP_NAME_REQ_ENABLE = 0;
const int CONFIG_BREDR_INQUIRY = 0;
const int config_bredr_afh_user = 0;
const int CONFIG_LMP_MASTER_ESCO_ENABLE = 0;
const int config_bt_security_vulnerability = 0;
const int CONFIG_BTCTLER_FAST_CONNECT_ENABLE = 0;
const int config_btctler_bredr_master = 0;
const int CONFIG_A2DP_MAX_BUF_SIZE = 0;
const u32 config_vendor_le_bb = 0;
// Master multi-link
const int config_btctler_le_master_multilink = 0;
const int ble_disable_wait_enable = 1;

const int config_btctler_le_slave_conn_update_winden = 2500;//range:100 to 2500
const int config_le_sm_support_enable = 1; //是否支持加密配对
const int config_btctler_le_iso_tx = 1024;
const int config_btctler_le_iso_rx = 0;
const int config_bb_optimized_ctrl = 0;



