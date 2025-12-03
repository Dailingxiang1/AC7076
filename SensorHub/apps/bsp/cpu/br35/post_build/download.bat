rem @echo off

@echo *****************************************************************
@echo SDK BR35 P11
@echo *****************************************************************
@echo %date%

cd %~dp0

set OBJDUMP=C:\JL\pi32\bin\llvm-objdump.exe
set OBJSIZEDUMP=C:\JL\pi32\bin\llvm-objsizedump.exe
set OBJCOPY=C:\JL\pi32\bin\llvm-objcopy.exe
set BANKLINK=.\BankLink.exe
set ELFFILE=p11.elf

REM %OBJDUMP% -D -address-mask=0x1ffffff -print-imm-hex -print-dbg %ELFFILE% > p11.lst

%OBJCOPY% -O binary -j .text %ELFFILE% text.bin
%OBJCOPY% -O binary -j .overlay_bank_demo_init %ELFFILE% bank_demo_init.bin
%OBJCOPY% -O binary -j .overlay_bank_demo_normal0 %ELFFILE% bank_demo_normal0.bin
%OBJCOPY% -O binary -j .overlay_bank_demo_normal1 %ELFFILE% bank_demo_normal1.bin
%OBJCOPY% -O binary -j .overlay_bank_demo_normal2 %ELFFILE% bank_demo_normal2.bin

%OBJCOPY% -O binary -j .overlay_bank_sensor_init %ELFFILE% bank_sensor_init.bin
%OBJCOPY% -O binary -j .overlay_bank_sensor_normal0 %ELFFILE% bank_sensor_normal0.bin
%OBJCOPY% -O binary -j .overlay_bank_sensor_normal1 %ELFFILE% bank_sensor_normal1.bin



%OBJDUMP% -section-headers -address-mask=0x1ffffff %ELFFILE%
%OBJDUMP% -t %ELFFILE% > symbol_tbl.txt

%OBJSIZEDUMP% -dump-stack-size -enable-dbg-info %ELFFILE% > dump_stack_size.txt
%OBJSIZEDUMP% -dump-function-call -enable-dbg-info %ELFFILE% > dump_func_call.txt

set LZ4_PACKET=.\lz4_packet.exe

set bank_files=0x80 text.bin 

%LZ4_PACKET% -input text.bin 0 -o text.lz4
set bank_lz4_files=0x80 text.lz4 

for %%a in (bank_demo_init.bin) do if %%~za gtr 0 (
set bank_files=%bank_files% 0xAA55AA55 bank_demo_init.bin
%LZ4_PACKET% -input bank_demo_init.bin 0 -o bank_demo_init.lz4
set bank_lz4_files=%bank_lz4_files% 0xAA55AA55 bank_demo_init.lz4
)

for %%a in (bank_demo_normal0.bin) do if %%~za gtr 0 (
set bank_files=%bank_files% 0xAA55AA55 bank_demo_normal0.bin
%LZ4_PACKET% -input bank_demo_normal0.bin 0 -o bank_demo_normal0.lz4
set bank_lz4_files=%bank_lz4_files% 0xAA55AA55 bank_demo_normal0.lz4
)

for %%a in (bank_demo_normal1.bin) do if %%~za gtr 0 (
set bank_files=%bank_files% 0xAA55AA55 bank_demo_normal1.bin
%LZ4_PACKET% -input bank_demo_normal1.bin 0 -o bank_demo_normal1.lz4
set bank_lz4_files=%bank_lz4_files% 0xAA55AA55 bank_demo_normal1.lz4
)

for %%a in (bank_sensor_init.bin) do if %%~za gtr 0 (
set bank_files=%bank_files% 0xAA55AA55 bank_sensor_init.bin
%LZ4_PACKET% -input bank_sensor_init.bin 0 -o bank_sensor_init.lz4
set bank_lz4_files=%bank_lz4_files% 0xAA55AA55 bank_sensor_init.lz4
)

for %%a in (bank_sensor_normal0.bin) do if %%~za gtr 0 (
set bank_files=%bank_files% 0xAA55AA55 bank_sensor_normal0.bin
%LZ4_PACKET% -input bank_sensor_normal0.bin 0 -o bank_sensor_normal0.lz4
set bank_lz4_files=%bank_lz4_files% 0xAA55AA55 bank_sensor_normal0.lz4
)

for %%a in (bank_sensor_normal1.bin) do if %%~za gtr 0 (
set bank_files=%bank_files% 0xAA55AA55 bank_sensor_normal1.bin
%LZ4_PACKET% -input bank_sensor_normal1.bin 0 -o bank_sensor_normal1.lz4
set bank_lz4_files=%bank_lz4_files% 0xAA55AA55 bank_sensor_normal1.lz4
)

for %%a in (bank_sys_init.bin) do if %%~za gtr 0 (
set bank_files=%bank_files% 0xAA55AA55 bank_sys_init.bin
%LZ4_PACKET% -input bank_sys_init.bin 0 -o bank_sys_init.lz4
set bank_lz4_files=%bank_lz4_files% 0xAA55AA55 bank_sys_init.lz4
)

for %%a in (bank_sys_normal0.bin) do if %%~za gtr 0 (
set bank_files=%bank_files% 0xAA55AA55 bank_sys_normal0.bin
%LZ4_PACKET% -input bank_sys_normal0.bin 0 -o bank_sys_normal0.lz4
set bank_lz4_files=%bank_lz4_files% 0xAA55AA55 bank_sys_normal0.lz4
)

echo %bank_files%
echo %bank_lz4_files%

%BANKLINK% %bank_files% p11_bank_code.bin
%BANKLINK% %bank_lz4_files% p11_bank_code.lz4

if exist "..\..\..\..\..\..\sdk\cpu\br35\tools\" (
    copy .\p11_bank_code.bin     ..\..\..\..\..\..\sdk\cpu\br35\tools\p11_code.bin
    copy .\p11_bank_code.lz4     ..\..\..\..\..\..\sdk\cpu\br35\tools\p11_code.lz4
)

::pause

