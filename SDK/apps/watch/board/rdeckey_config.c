#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".rdeckey_config.data.bss")
#pragma data_seg(".rdeckey_config.data")
#pragma const_seg(".rdeckey_config.text.const")
#pragma code_seg(".rdeckey_config.text")
#endif
#include "app_config.h"
#include "rdec.h"
#include "gpio.h"
#include "key_driver.h"
#include "rdec_key.h"

#ifdef TCFG_RDEC_KEY_ENABLE

extern void lvgl_key_event_handler(struct key_event *key);

const struct rdec_device rdeckey_list[CONFIG_RDEC_KEY_MAX_NUM] = {
//RDEC0配置
    {
        .index = 0,
        .sin_portA = TCFG_RDEC0_ECODEA_PORT,
        .sin_portA_io_mode = PORT_INPUT_PULLUP_10K,
        .sin_portB = TCFG_RDEC0_ECODEB_PORT,
        .sin_portB_io_mode = PORT_INPUT_PULLUP_10K,
        .scan_time = 100,
        .phase_mode = RDEC_PHASE_1,
    },
};

const struct rdec_platform_data  rdec_key_data = {
    .enable = 1,
    .num = ARRAY_SIZE(rdeckey_list),
    .rdec = rdeckey_list,
};


#endif
