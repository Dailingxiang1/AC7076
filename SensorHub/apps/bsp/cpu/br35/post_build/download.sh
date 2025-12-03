##!/bin/sh
${OBJDUMP} -D -address-mask=0x1ffffff -print-imm-hex -print-dbg $1.elf > $1.lst

${OBJCOPY} -O binary -j .text $1.elf text.bin
##${OBJCOPY} -O binary -j .p11_poweroff_code $1.elf p11_poweroff_code.bin
${OBJCOPY} -O binary -j .overlay_bank_demo_init $1.elf bank_demo_init.bin
${OBJCOPY} -O binary -j .overlay_bank_demo_normal0 $1.elf bank_demo_normal0.bin
${OBJCOPY} -O binary -j .overlay_bank_demo_normal1 $1.elf bank_demo_normal1.bin
${OBJCOPY} -O binary -j .overlay_bank_demo_normal2 $1.elf bank_demo_normal2.bin

${OBJCOPY} -O binary -j .overlay_bank_sensor_init $1.elf bank_sensor_init.bin
${OBJCOPY} -O binary -j .overlay_bank_sensor_normal0 $1.elf bank_sensor_normal0.bin
${OBJCOPY} -O binary -j .overlay_bank_sensor_normal1 $1.elf bank_sensor_normal1.bin



${OBJDUMP} -section-headers -address-mask=0x1ffffff $1.elf > segment_list.txt

${OBJSIZEDUMP} -lite -skip-zero -enable-dbg-info $1.elf | sort -k 1 > symbol_tbl.txt

${OBJSIZEDUMP} -dump-stack-size -enable-dbg-info $1.elf > dump_stack_size.txt
${OBJSIZEDUMP} -dump-function-call -enable-dbg-info $1.elf > dump_func_call.txt
/opt/utils/calc_min_stack_size.py --stack dump_stack_size.txt --call dump_func_call.txt > analyze.txt

compress_tool=lz4_packet

#bank_files="0x80 p11_poweroff_code.bin "
bank_files=${bank_files}"0x80 text.bin "

$compress_tool -input text.bin 0 -o text.lz4
bank_lz4_files=${bank_lz4_files}"0x80 text.lz4 "

if [ -s  bank_demo_init.bin ]
    then
        bank_files=${bank_files}"0xAA55AA55 bank_demo_init.bin "
		$compress_tool -input bank_demo_init.bin 0 -o bank_demo_init.lz4
        bank_lz4_files=${bank_lz4_files}"0xAA55AA55 bank_demo_init.lz4 "
fi

if [ -s  bank_demo_normal0.bin ]
    then
        bank_files=${bank_files}"0xAA55AA55 bank_demo_normal0.bin "
		$compress_tool -input bank_demo_normal0.bin 0 -o bank_demo_normal0.lz4
        bank_lz4_files=${bank_lz4_files}"0xAA55AA55 bank_demo_normal0.lz4 "
fi

if [ -s  bank_demo_normal1.bin ]
    then
        bank_files=${bank_files}"0xAA55AA55 bank_demo_normal1.bin "
		$compress_tool -input bank_demo_normal1.bin 0 -o bank_demo_normal1.lz4
        bank_lz4_files=${bank_lz4_files}"0xAA55AA55 bank_demo_normal1.lz4 "
fi

if [ -s  bank_sensor_init.bin ]
    then
        bank_files=${bank_files}"0xAA55AA55 bank_sensor_init.bin "
		$compress_tool -input bank_sensor_init.bin 0 -o bank_sensor_init.lz4
        bank_lz4_files=${bank_lz4_files}"0xAA55AA55 bank_sensor_init.lz4 "
fi

if [ -s  bank_sensor_normal0.bin ]
    then
        bank_files=${bank_files}"0xAA55AA55 bank_sensor_normal0.bin "
		$compress_tool -input bank_sensor_normal0.bin 0 -o bank_sensor_normal0.lz4
        bank_lz4_files=${bank_lz4_files}"0xAA55AA55 bank_sensor_normal0.lz4 "
fi

if [ -s  bank_sensor_normal1.bin ]
    then
        bank_files=${bank_files}"0xAA55AA55 bank_sensor_normal1.bin "
		$compress_tool -input bank_sensor_normal1.bin 0 -o bank_sensor_normal1.lz4
        bank_lz4_files=${bank_lz4_files}"0xAA55AA55 bank_sensor_normal1.lz4 "
fi

if [ -s  bank_sys_init.bin ]
    then
        bank_files=${bank_files}"0xAA55AA55 bank_sys_init.bin "
		$compress_tool -input bank_sys_init.bin 0 -o bank_sys_init.lz4
        bank_lz4_files=${bank_lz4_files}"0xAA55AA55 bank_sys_init.lz4 "
fi

if [ -s  bank_sys_normal0.bin ]
    then
        bank_files=${bank_files}"0xAA55AA55 bank_sys_normal0.bin "
		$compress_tool -input bank_sys_normal0.bin 0 -o bank_sys_normal0.lz4
        bank_lz4_files=${bank_lz4_files}"0xAA55AA55 bank_sys_normal0.lz4 "
fi

echo ${bank_files}
echo ${bank_lz4_files}

BankLink ${bank_files} p11_bank_code.bin
BankLink ${bank_lz4_files} p11_bank_code.lz4

cat p11_bank_code.bin > ${NICKNAME}_code.bin
cat p11_bank_code.lz4 > ${NICKNAME}_code.lz4

# cp ${NICKNAME}_code.bin /home/chenrixin/work_space/code_refacter/wsdk/watch_sdk/cpu/br35/tools/p11_code.bin

cat segment_list.txt
/opt/utils/report_segment_usage --sdk_path ${ROOT} \
--tbl_file symbol_tbl.txt \
--seg_file segment_list.txt \
--map_file p11.map \
--module_depth "{\"app\":1,\"lib\":2,\"[lib]\":2}"

host-client -project ${NICKNAME}$2 -f ${NICKNAME}_code.bin ${NICKNAME}_code.lz4

