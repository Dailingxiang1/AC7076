// *INDENT-OFF*
#include "app_config.h"

#ifdef __SHELL__

##!/bin/sh

${OBJDUMP} -D -address-mask=0xfffffff -print-imm-hex -print-dbg -mcpu=r3 $1.elf > $1.lst
${OBJCOPY} -O binary -j .text $1.elf text.bin
${OBJCOPY} -O binary -j .data  $1.elf data.bin
${OBJCOPY} -O binary -j .data_code $1.elf data_code.bin
${OBJCOPY} -O binary -j .overlay_aec $1.elf aec.bin
${OBJCOPY} -O binary -j .overlay_aac $1.elf aac.bin
${OBJCOPY} -O binary -j .ps_ram_data_code $1.elf psr_data_code.bin
${OBJCOPY} -O binary -j .dcache_ram_data $1.elf d_ram_data.bin
${OBJCOPY} -O binary -j .icache_ram_data_code $1.elf i_ram_data_code.bin

${OBJDUMP} -section-headers -address-mask=0x7ffffff $1.elf > segment_list.txt
${OBJSIZEDUMP} -lite -skip-zero -enable-dbg-info $1.elf | sort -k 1 >  symbol_tbl.txt


cat segment_list.txt

/opt/utils/report_segment_usage --sdk_path ${ROOT} \
--tbl_file symbol_tbl.txt \
--seg_file segment_list.txt \
--map_file $1.map \
--module_depth "{\"apps\":1,\"lib\":2,\"[lib]\":2}"


cat text.bin data.bin data_code.bin aec.bin aac.bin psr_data_code.bin d_ram_data.bin i_ram_data_code.bin > app.bin

/* if [ -f version ]; then */
    /* host-client -project ${NICKNAME}$2 -f app.bin version $1.elf p11_code.bin br28loader.bin br28loader.uart uboot.boot uboot.boot_debug ota.bin ota_debug.bin isd_config.ini */
/* else */
    /* host-client -project ${NICKNAME}$2 -f app.bin $1.elf  p11_code.bin br28loader.bin br28loader.uart uboot.boot uboot.boot_debug ota.bin ota_debug.bin isd_config.ini */

/* fi */

/* /opt/utils/strip-ini -i isd_config.ini -o isd_config.ini */

files="app.bin ota*.bin p11_code.bin isd_config.ini uboot.boot br35loader.bin isd_download.exe flash_params_v*.bin"
//files="app.bin isd_config.ini"


host-client -project ${NICKNAME}$2 -f ${files} $1.elf

#else

@echo off
Setlocal enabledelayedexpansion
@echo ********************************************************************************
@echo           SDK BR35
@echo ********************************************************************************
@echo %date%


cd /d %~dp0


set OBJDUMP=C:\JL\pi32\bin\llvm-objdump.exe
set OBJCOPY=C:\JL\pi32\bin\llvm-objcopy.exe
set ELFFILE=sdk.elf
set LZ4_PACKET=.\lz4_packet.exe

%OBJDUMP% -D -address-mask=0x7ffffff -print-imm-hex -mcpu=r3 -print-dbg sdk.elf > sdk.lst
%OBJCOPY% -O binary -j .text %ELFFILE% text.bin
%OBJCOPY% -O binary -j .data %ELFFILE% data.bin
%OBJCOPY% -O binary -j .data_code %ELFFILE% data_code.bin
%OBJCOPY% -O binary -j .overlay_aec %ELFFILE% aec.bin
%OBJCOPY% -O binary -j .overlay_aac %ELFFILE% aac.bin
%OBJCOPY% -O binary -j .ps_ram_data_code %ELFFILE% psr_data_code.bin
%OBJCOPY% -O binary -j .dcache_ram_data %ELFFILE% d_ram_data.bin
%OBJCOPY% -O binary -j .icache_ram_data_code %ELFFILE% i_ram_data_code.bin

%OBJDUMP% -section-headers -address-mask=0x7ffffff %ELFFILE%
%OBJDUMP% -t %ELFFILE% >  symbol_tbl.txt

copy /b text.bin + data.bin + data_code.bin + aec.bin + aac.bin + psr_data_code.bin + d_ram_data.bin + i_ram_data_code.bin app.bin


#if TCFG_TONE_EN_ENABLE
set TONE_EN_ENABLE=1
#else
set TONE_EN_ENABLE=0
#endif

#if TCFG_TONE_ZH_ENABLE
set TONE_ZH_ENABLE=1
#else
set TONE_ZH_ENABLE=0
#endif


#if ((defined(TCFG_NORFLASH_SFC_DEV_ENABLE)) && TCFG_NORFLASH_SFC_DEV_ENABLE)
set EXT_FLASH_ENABLE=1
#else
set EXT_FLASH_ENABLE=0
#endif

#if ((defined(TCFG_NANDFLASH_DEV_ENABLE)) && TCFG_NANDFLASH_DEV_ENABLE)
set NAND_FLASH_ENABLE=1
#else
set NAND_FLASH_ENABLE=0
#endif


#ifdef CONFIG_WATCH_CASE_ENABLE
#if CONFIG_JL_UI_ENABLE
#if GPU_DEMO_ENABLE
call download/watch/download_jlui_demo.bat
#elif TCFG_COLOR_SCREEN_CHARGING_CASE_ENABLE
call download/watch/download_jlui_csc.bat
#else
call download/watch/download_jlui.bat
#endif
#elif CONFIG_LVGL_UI_ENABLE
call download/watch/download_lvgl.bat
#else
call download/watch/download.bat
#endif
#elif (defined CONFIG_SERIAL_HMI_DISPLAY_CASE_ENABLE)
call download/serial_hmi_display/download_lvgl.bat
#elif (defined CONFIG_SOUNDBOX_CASE_ENABLE)
call download/soundbox/download.bat
#elif (defined CONFIG_EARPHONE_CASE_ENABLE)
call download/earphone/download.bat
#else // #ifdef CONFIG_WATCH_CASE_ENABLE
//to do other case
#endif  //endif app_case

#endif

