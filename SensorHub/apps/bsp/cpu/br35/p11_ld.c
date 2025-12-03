// *INDENT-OFF*
#include "p11_rom_stubs.ld"

#include "power/p11/p11_mmap.h"

/*
p11 memory地址图

0xf28000, end
0xf27fe0, poff ram, 0x20
0xf27f00, m2p_message, 0xe0
0xf27ec0, p2m_message, 0x40
0xf2xxxx, stack, 0x600
0xf2xxxx, bss
0xf20080, code/data
0xf20000, isr_vertor, 0x80
*/

MEMORY
{
    p11_message_ram(rw)		:	ORIGIN = P2M_MESSAGE_RAM_BEGIN, LENGTH = (P2M_MESSAGE_SIZE+M2P_MESSAGE_SIZE)
	msys_poweroff_ram(rw)	:	ORIGIN = MSYS_POFF_RAM_BEGIN, LENGTH = MSYS_POFF_RAM_SIZE
	p11_ram0(rw)			:	ORIGIN = P11_RAM0_BEGIN, LENGTH = P11_RAM0_SIZE


}

ENTRY(_start)

SECTIONS
{
    . = ORIGIN(p11_ram0);
    .text ALIGN(32):
    {
        *startup.o(.text)
        *(.text*)
		*(.*.text)
		*(.*.const)

		*(.*.text)
        *(.ipc_spin_lock.text.cache.L1)

        . = ALIGN(4);
        sensor_dev_begin = .;
        KEEP(*(.sensor_dev))
       sensor_dev_end = .;

        . = ALIGN(4);
        *(.rodata*)

#include "power/ld/power_text.ld"

		. = ALIGN(4);
		*(.*.data)
		. = ALIGN(4);
        *(.data*)
		. = ALIGN(4);
    } > p11_ram0

	.data ALIGN(32):
	{
		/* *(.data*) */
	} > p11_ram0

    .bss ALIGN(4):
    {
        *(.bss)
		*(.*.bss)
        *(COMMON)
    } > p11_ram0

	.stack_bss ALIGN(32):
    {
        *(.stack_magic0)
        PROVIDE(_stack_begin = .);
        PROVIDE(_sstack_begin = .);
        *(.sstack)
        PROVIDE(_sstack_end = .);

        PROVIDE(_ustack_begin = .);
        *(.ustack)
        PROVIDE(_ustack_end = .);

        PROVIDE(_stack_end = .);
        *(.stack_magic1)
    } > p11_ram0

#if 0
	PROVIDE(overlay_demo_begin = .);
	OVERLAY : AT(0x200000) SUBALIGN(4)
	{
		.overlay_bank_demo_init
		{
            LONG(0xffffffff);
            *(.demo.code.bank.0)
            . = ALIGN(4);
		}
		.overlay_bank_demo_normal0
		{
            LONG(0xffffffff);
            *(.demo.code.bank.1)
		}

		.overlay_bank_demo_normal1
		{
            LONG(0xffffffff);
            *(.demo.code.bank.2)
		}
	} > p11_ram0
	PROVIDE(overlay_demo_end = .);


	PROVIDE(overlay_sensor_begin = .);
	OVERLAY : AT(0x210000) SUBALIGN(4)
	{
		/* TODO: */
		.overlay_bank_sensor_init
		{
            LONG(0xffffffff);
            *(.sensor.code.bank.0)
            . = ALIGN(4);
		}
		.overlay_bank_sensor_normal0
		{
            LONG(0xffffffff);
            *(.sensor.code.bank.1)
		}
		.overlay_bank_sensor_normal1
		{
            LONG(0xffffffff);
            *(.sensor.code.bank.2)
		}

	} > p11_ram0
	PROVIDE(overlay_sensor_end = .);

	PROVIDE(overlay_sys_begin = .);
	OVERLAY : AT(0x220000) SUBALIGN(4)
	{
		/* TODO: */
		.overlay_bank_sys_init
		{
            LONG(0xffffffff);
            *(.sys.code.bank.0)
            . = ALIGN(4);
		}
		.overlay_bank_sys_normal0
		{
            LONG(0xffffffff);
            *(.sys.code.bank.1)
		}
	} > p11_ram0
	PROVIDE(overlay_sys_end = .);
#endif

	PROVIDE(p11_heap_begin = .);
 	p11_heap_end = P2M_MESSAGE_RAM_BEGIN;

	. = ORIGIN(p11_message_ram);
    .p11_message ALIGN(32):
    {
        *(.p2m_data)
        *(.m2p_data)
    } > p11_message_ram

	. = ORIGIN(msys_poweroff_ram);
    .p11_poweroff ALIGN(32):
    {
        *(.msys_poff_data)
    } > msys_poweroff_ram
}

text_begin  = ADDR(.text);
text_size   = SIZEOF(.text);
text_end    = text_begin + text_size;

bss_begin = ADDR(.bss);
bss_size  = SIZEOF(.bss);

p11_message_begin = ADDR(.p11_message);
p11_message_size = SIZEOF(.p11_message);

data_addr = ADDR(.data);
data_begin = text_begin + text_size;
data_size =  SIZEOF(.data);
