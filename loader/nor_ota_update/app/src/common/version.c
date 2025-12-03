#ifdef SUPPORT_MS_EXTENSIONS_APP
#pragma bss_seg(".version.data.bss")
#pragma data_seg(".version.data")
#pragma const_seg(".version.text.const")
#pragma code_seg(".version.text")
#endif
#include "lib_include.h"

__attribute__((section(".version_tag1"), used))
#if defined(EDR_UPDATA_SUPPORT_CONNECT)
static const char version_type_tag[] = "edr_ota2";
#elif defined(BLE_UPDATA_SUPPORT_CONNECT)
static const char version_type_tag[] = "ble_ota";
#elif (1 == USB_HOST_MODULE_CONTROL)

//二级loader修改
#if defined(CONFIG_CPU_BR25) || defined(CONFIG_CPU_BR27)
static const char version_type_tag[] = "usb_update2/usb_sec_ota";
#else
static const char version_type_tag[] = "usb_update2";
#endif

#elif (1 == SD_MODULE_CONTROL)
//二级loader修改
#if defined(CONFIG_CPU_BR25) || defined(CONFIG_CPU_BR27)
static const char version_type_tag[] = "sd_update2/sd_sec_ota";
#else
static const char version_type_tag[] = "sd_update2";
#endif

#elif (1 == BLE_GATT_UPDATA_MODULE_CONTROL)
static const char version_type_tag[] = "ble_app_ota";
#elif (1 == SPP_UPDATA_MODULE_CONTROL)
static const char version_type_tag[] = "spp_app_ota";
#elif (1 == UART_UPDATA_MODULE_CONTROL)
static const char version_type_tag[] = "uart_update";
#elif (1 == UART_UPDATA_USER_MODULE_CONTROL)
static const char version_type_tag[] = "user_uart_update";
#elif defined(EX_FLASH_UPDATE_SUPPORT_EN)
static const char version_type_tag[] = "nor_ota";
#elif (1 == USER_LC_FLASH_UPDATA_MODULE_CONTROL)
static const char version_type_tag[] = "lcflash_ota";
#elif (1 == USB_HID_MODULE_CONTROL)
static const char version_type_tag[] = "usb_hid_ota";
#elif (1 == DEV_NORFLASH_UPDATA_MODULE_CONTROL)
static const char version_type_tag[] = "dev_nor_ota";
#elif (1 == NET_UPDATA_MODULE_CONTROL)
static const char version_type_tag[] = "net_ota";
#elif (1 == USB_HID_MODULE_CONTROL)
static const char version_type_tag[] = "usb_hid_ota";
#endif

__attribute__((section(".version_tag2"), used))
static const char version_date_tag[] = __DATE__;

__attribute__((section(".version_tag3"), used))
static const char version_time_tag[] = __TIME__;

