#ifndef _TP_API_H_
#define _TP_API_H_

#include "app_config.h"

#if TCFG_TOUCH_PANEL_ENABLE


#if TCFG_TP_CST816D_ENABLE
extern void cst816d_init();
#define  TP_INIT()  cst816d_init()
#endif


#if TCFG_TP_FT3X68_ENABLE
extern void fts_ts_init();
#define  TP_INIT()  fts_ts_init()
#endif


#else

#define  TP_INIT()

#endif /* #if TCFG_TOUCH_PANEL_ENABLE */

#endif


