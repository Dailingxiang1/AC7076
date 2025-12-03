#ifndef __POWER_INTERFACE_H__
#define __POWER_INTERFACE_H__

#include "includes.h"

//-------------------------------------------------------
/* p33
 */
#include "power/p33/p33_sfr.h"
#include "power/p33/p33_api.h"
#include "power/p33/p33_access.h"

//-------------------------------------------------------
/* p11
 */
#include "power/p11/p11_csfr.h"
#include "power/p11/p11_sfr.h"
#include "power/p11/lp_ipc.h"
#include "power/p11/p11_mmap.h"
#include "power/p11/p11_api.h"
#include "power/p11/p11_rom_api.h"
#include "power/p11/p11_clock_hw.h"

//-------------------------------------------------------
/* power
 */
#include "power/power_app.h"
#include "power/power_api.h"
#include "power/power_wakeup.h"
#include "power/power_port.h"

#endif
