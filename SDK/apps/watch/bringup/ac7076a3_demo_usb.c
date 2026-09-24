#include "app_config.h"
#include "system/includes.h"
#include "system/timer.h"
#include "usb/usb_config.h"
#include "usb/usb_task.h"
#include "usb/device/usb_stack.h"
#include "usb/device/cdc.h"
#include "circular_buf.h"
#include "ac7076a3_demo.h"

#if AC7076A3_DEMO_ENABLE && DEMO_USB_CDC_ENABLE
#if !TCFG_USB_SLAVE_ENABLE || !TCFG_USB_SLAVE_CDC_ENABLE
#error "USB CDC demo requires the USB slave CDC build configuration"
#endif
static volatile u8 rx_pending;
static u8 echo_buf[512];
static u32 echo_len, echo_offset, echo_deadline;
static u32 rx_probe_at;
static int usb_poll_enabled;

/* printf/putbyte can run before enumeration and from interrupt context.
 * Queue only here; the app task owns all potentially blocking CDC writes.
 */
static cbuffer_t log_queue;
static u8 log_storage[4096];
static u8 log_tx[64];
static u32 log_tx_len, log_tx_offset;
static volatile u8 log_queue_ready;

void ac7076a3_demo_usb_log_init(void)
{
    log_queue_ready = 0;
    cbuf_init(&log_queue, log_storage, sizeof(log_storage));
    log_tx_len = 0;
    log_tx_offset = 0;
    log_queue_ready = 1;
}

void ac7076a3_demo_usb_log_putbyte(char c)
{
    if (!log_queue_ready) {
        return;
    }
    if (cbuf_write(&log_queue, &c, 1) != 1) {
        u8 oldest;
        cbuf_read(&log_queue, &oldest, 1);
        cbuf_write(&log_queue, &c, 1);
    }
}

static void demo_cdc_log_poll(void)
{
    if (log_tx_offset == log_tx_len) {
        log_tx_len = cbuf_read(&log_queue, log_tx, sizeof(log_tx));
        log_tx_offset = 0;
    }
    if (log_tx_len) {
        u32 sent = cdc_write_data(0, log_tx + log_tx_offset,
                                  log_tx_len - log_tx_offset);
        if (sent <= log_tx_len - log_tx_offset) {
            log_tx_offset += sent;
        }
    }
}

static void demo_cdc_wakeup(struct usb_device_t *device)
{
    (void)device;
    rx_pending = 1; /* IRQ: no printf, no mutexes or blocking USB writes. */
}

void ac7076a3_demo_usb_start(void)
{
    /* Schedule CDC initialization on the SDK USB stack task. Calling
     * usb_device_mode() from app_core blocked board bring-up on this PCB.
     */
    usb_message_to_stack(USBSTACK_CDC_BACKGROUND, 0, 0);
    usb_poll_enabled = 1;
    printf("[BRINGUP] USB CDC start requested; open virtual COM with DTR+RTS\n");
}

void ac7076a3_demo_usb_poll(void)
{
    if (!usb_poll_enabled) {
        return;
    }
    /* Background USB reconnects can recreate cdc_hdl and its callback. */
    cdc_set_wakeup_handler(demo_cdc_wakeup);
    u32 now = sys_timer_get_ms();
    if (!echo_len && (rx_pending || (s32)(now - rx_probe_at) >= 0)) {
        rx_pending = 0;
        rx_probe_at = now + 100;
        echo_len = cdc_read_data(0, echo_buf, sizeof(echo_buf));
        echo_offset = 0;
        echo_deadline = now + 1000;
        if (echo_len) {
            printf("[BRINGUP] USB RX %u bytes\n", echo_len);
        }
    }
    if (echo_len) {
        u32 sent = cdc_write_data(0, echo_buf + echo_offset, echo_len - echo_offset);
        if (sent > echo_len - echo_offset) {
            sent = 0;
        }
        echo_offset += sent;
        if (echo_offset == echo_len) {
            printf("[BRINGUP] USB echoed %u bytes\n", echo_len);
            echo_len = 0;
        } else if ((s32)(now - echo_deadline) >= 0) {
            printf("[BRINGUP] USB TX timeout/drop %u bytes; check DTR/RTS/USB cable\n",
                   echo_len - echo_offset);
            echo_len = 0;
        }
    }
    demo_cdc_log_poll();
}
#endif
