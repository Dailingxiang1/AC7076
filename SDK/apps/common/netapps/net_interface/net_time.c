#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".net_interface.data.bss")
#pragma data_seg(".net_interface.data")
#pragma const_seg(".net_interface.text.const")
#pragma code_seg(".net_interface.text")
#endif
#include "net_time.h"

#if NET_INTERFACE_EN
//获取rtc时间
void net_get_sys_time(struct sys_time *time)
{
    rtc_read_time(time);
}
#endif//NET_INTERFACE_EN
