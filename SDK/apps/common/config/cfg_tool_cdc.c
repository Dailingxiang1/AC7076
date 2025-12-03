#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".cfg_tool_cdc.data.bss")
#pragma data_seg(".cfg_tool_cdc.data")
#pragma const_seg(".cfg_tool_cdc.text.const")
#pragma code_seg(".cfg_tool_cdc.text")
#endif
#include "cfg_tool_cdc.h"
#include "app_config.h"
#include "cfg_tool.h"
#if TCFG_USB_SLAVE_CDC_ENABLE
#include "usb/device/cdc.h"
#endif

#if TCFG_CFG_TOOL_ENABLE && (TCFG_COMM_TYPE == TCFG_USB_COMM)

#define CFG_TOOL_PROTOCOL_HEAD_SIZE 7

static u16 rx_len_count = 0;
static u16 tool_buf_total_len = 0;
static u8 *buf_rx = NULL;

/**
 *	@brief 获取工具cdc最大支持的协议包大小
 */
u16 cfg_tool_cdc_rx_max_mtu()
{
    return 1024 * 4 + 22;
}

static void reset_rx_resource()
{
    if (buf_rx) {
        free(buf_rx);
        buf_rx = NULL;
    }
    rx_len_count = 0;
    tool_buf_total_len = 0;
}

/**
 *	@brief 获取cdc的配置/调音工具相关数据
 */
void cfg_tool_data_from_cdc(u8 *buf, u32 rlen) // in irq
{
    /* printf("cfg_tool cdc rx:\n"); */
    /* put_buf(buf, rlen); */

    if ((buf[0] == 0x5A) && (buf[1] == 0xAA) && (buf[2] == 0xA5)) {
        reset_rx_resource();
        tool_buf_total_len = CFG_TOOL_READ_LIT_U16(buf + 5);
        buf_rx = zalloc(CFG_TOOL_PROTOCOL_HEAD_SIZE + tool_buf_total_len);
        ASSERT(buf_rx);
        memcpy(buf_rx, buf, rlen);
        /* printf("cfg_tool cdc need total len1 = %d\n", tool_buf_total_len); */
        /* printf("cfg_tool cdc rx len1 = %d\n", rlen); */
        if ((CFG_TOOL_PROTOCOL_HEAD_SIZE + tool_buf_total_len) == rlen) {
            /* printf("cfg_tool cdc rx1:\n"); */
            /* put_buf(buf_rx, rlen); */
            online_cfg_tool_data_deal(buf_rx, rlen);
            reset_rx_resource();
        } else {
            rx_len_count += rlen;
        }
    } else {
        if (!buf_rx) {
            printf("%s, buf_rx null!\n", __FUNCTION__);
            reset_rx_resource();
            return;
        }
        if ((rx_len_count + rlen) > (CFG_TOOL_PROTOCOL_HEAD_SIZE + tool_buf_total_len)) {
            reset_rx_resource();
            return;
        }
        memcpy(buf_rx + rx_len_count, buf, rlen);
        /* printf("cfg_tool cdc need total len2 = %d\n", tool_buf_total_len); */
        /* printf("cfg_tool cdc rx len2 = %d\n", rlen + rx_len_count); */
        if ((tool_buf_total_len + CFG_TOOL_PROTOCOL_HEAD_SIZE) == (rx_len_count + rlen)) {
            /* printf("cfg_tool cdc rx2:\n"); */
            /* put_buf(buf_rx, rx_len_count + rlen); */
            online_cfg_tool_data_deal(buf_rx, rx_len_count + rlen);
            reset_rx_resource();
        } else {
            rx_len_count += rlen;
        }
    }
}

#endif

