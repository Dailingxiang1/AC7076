


























@echo off
Setlocal enabledelayedexpansion
@echo ********************************************************************************
@echo SDK BR35
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
REM %OBJDUMP% -t %ELFFILE% > symbol_tbl.txt

copy /b text.bin + data.bin + data_code.bin + aec.bin + aac.bin + psr_data_code.bin + d_ram_data.bin + i_ram_data_code.bin app.bin



set TONE_EN_ENABLE=1





set TONE_ZH_ENABLE=1






call download/watch/download_jlui_demo.bat
