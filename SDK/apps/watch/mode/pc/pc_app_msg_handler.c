#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".pc_app_msg_handler.data.bss")
#pragma data_seg(".pc_app_msg_handler.data")
#pragma const_seg(".pc_app_msg_handler.text.const")
#pragma code_seg(".pc_app_msg_handler.text")
#endif
#include "key_driver.h"
#include "app_main.h"
#include "init.h"
#include "audio_config.h"
#include "usb/device/hid.h"

#if TCFG_APP_PC_EN
int pc_app_msg_handler(int *msg)
{
    if (false == app_in_mode(APP_MODE_PC)) {
        return 0;
    }

    switch (msg[0]) {
    case APP_MSG_CHANGE_MODE:
        printf("app msg key change mode\n");
        app_send_message(APP_MSG_GOTO_NEXT_MODE, 0);
        break;
#if TCFG_USB_SLAVE_HID_ENABLE
    case APP_MSG_MUSIC_PP:
        printf("APP_MSG_MUSIC_PP\n");
        hid_key_handler(0, USB_AUDIO_PP);
        break;
    case APP_MSG_MUSIC_PREV:
        printf("APP_MSG_MUSIC_PREV\n");
        hid_key_handler(0, USB_AUDIO_PREFILE);
        break;
    case APP_MSG_MUSIC_NEXT:
        printf("APP_MSG_MUSIC_NEXT\n");
        hid_key_handler(0, USB_AUDIO_NEXTFILE);
        break;
    case APP_MSG_VOL_UP:
        printf("APP_MSG_VOL_UP\n");
        hid_key_handler(0, USB_AUDIO_VOLUP);
        printf(">>>pc vol+: %d", app_audio_get_volume(APP_AUDIO_CURRENT_STATE));
        break;
    case APP_MSG_VOL_DOWN:
        printf("APP_MSG_VOL_DOWN\n");
        hid_key_handler(0, USB_AUDIO_VOLDOWN);
        printf(">>>pc vol-: %d", app_audio_get_volume(APP_AUDIO_CURRENT_STATE));
        break;
#endif
    default:
        break;
    }

    return 0;
}

#endif
