// *INDENT-OFF*
/*
#-------------------------------------------------------+
#               br35 RAM mapping                        |
#-------------------------------------------0x137000 ---| _RAM_LIMIT_H
#                                                       |
#                                                       |
#              ota-loader              (194K:0x30A00)   |
#                                                       |
#                                                       |
#-------------------------------------------0x106600 ---|
#                                                       |
#               uboot                   (16K:0x4000)    |
#                                                       |
#-------------------------------------------0x102600 ---|
#                                                       |
#                                                       |              ---------> user_api.bin end (0x10054c + 32K)
#-------------------------------------------0x10054c ---|              ---------> user_api.bin start(0x10054c)
#              Maskrom_export_RAM             (0x34c)   |
#-------------------------------------------0x100200 ---|
#                                                       |
#-------------------------------------------0x100000 ---| _RAM_LIMIT_L
*/

#include  "maskrom_stubs.ld"

//from mask export
ISR_BASE        = _IRQ_MEM_ADDR;

UPDATA_SIZE     = 0x200;

UPDATE_BEG    = _RAM_LIMIT_H - UPDATA_SIZE;

UPDATA_BREDR_BASE_BEG = 0x137000 - 0x1000;

EX_CODE_BEGIN  = 0x10054c;         /*用户自定义UI代码的地址，根据不同CPU来配置*/
EX_CODE_SIZE    = 0xc2b4;

#if defined(DEV_UPDATE_MINI) && DEV_UPDATE_MINI
SEC_OTA_CODE_BEGIN   = 0x106600;         /*第二级loader RAM地址*/
OTA_CODE_BEGIN  = 0x137000 - 0x6000;         /*注意不要和uboot重叠*/
/* OTA_CODE_BEGIN  = 0x175000;         #<{(|uart 测试盒串口升级开debug时修改|)}># */
OTA_CODE_SIZE   = 24K;

#else

OTA_CODE_BEGIN  = EX_CODE_BEGIN + EX_CODE_SIZE;         /*注意不要和uboot重叠*/
OTA_CODE_SIZE   = UPDATA_BREDR_BASE_BEG - OTA_CODE_BEGIN;
#endif

// EX_CODE_BEGIN 	= 0x104000;         /*用户自定义UI代码的地址，根据不同CPU来配置*/
// EX_CODE_SIZE    = 460K;

MEMORY
{
	text_ram   : ORIGIN = OTA_CODE_BEGIN,  			LENGTH = OTA_CODE_SIZE
#if !defined(DEV_UPDATE_MINI) || (!DEV_UPDATE_MINI)
	bt_ram     : ORIGIN = UPDATA_BREDR_BASE_BEG,  	LENGTH = 0x1000
	/* ex_code    : ORIGIN = EX_CODE_BEGIN,		    LENGTH = EX_CODE_SIZE */
	reserved   : ORIGIN = 0x0,						LENGTH = 0x100
	version_ram :ORIGIN = 0x8000000,  				LENGTH = 128
#endif

}

ENTRY(_start);
EXTERN(
	lib_ota_loader_version
#if (EX_FLASH_UPDATE_SUPPORT_EN)
	device_table
#endif
);

SECTIONS
{
    . = ORIGIN(text_ram);
    .text :
    {
        *(.start*)
        *(.text*)
        *(.*.text)
        *(.data*)
        *(.*.data)
        *(.rodata*)
        *(.*.text.const)
#if !defined(DEV_UPDATE_MINI) || (!DEV_UPDATE_MINI)
        *(.bredr_irq_code)
        *(.frame_irq_code)
        *(.classic_irq_code)
        *(.classic_irq_const)
        *(.volatile_ram_code)
#endif
        . = ALIGN(32);

#if (EX_FLASH_UPDATE_SUPPORT_EN)
		_device_node_begin = .;
        PROVIDE(device_node_begin = .);
        *(.device)
        _device_node_end = .;
        PROVIDE(device_node_end = .);
        . = ALIGN(32);
#endif
    } > text_ram

    .bss ALIGN(32) (NOLOAD):
    {
        _cpu0_sstack_begin = .;
        *(.stack*)
        _cpu0_sstack_end = .;
        _cpu0_ustack_begin = .;
        *(.ustack*)
        _cpu0_ustack_end = .;

        *(.bss)
        *(.*.data.bss)
        *(.usb_ep0)
        *(.usb_config_var)
        *(.update_buf)
        *(.bredr_rxtx_bulk)
        . = ALIGN(32);
    } > text_ram

    .bss_noinit ALIGN(32) (NOLOAD):
    {
        *(.bss_noinit)
        . = ALIGN(32);
    } > text_ram

#if !defined(DEV_UPDATE_MINI) || (!DEV_UPDATE_MINI)
	. = ORIGIN(bt_ram);
	.bt_bss (NOLOAD) :SUBALIGN(4)
	{
		*(.bd_base)
		*(.comm_rf_para)
		*(.bd_base1)
	} > bt_ram

	// . = ORIGIN(ex_code);
	// .ex_code (NOLOAD) :SUBALIGN(4)
	// {
	// 	. = LENGTH(ex_code);
	// } > ex_code

	. = ORIGIN(reserved);
	.ver_tag ALIGN(4):
	{
		KEEP(*(.version_tag1))
		KEEP(*(.version_tag2))
		KEEP(*(.version_tag3))
		. = ALIGN(4);
	} > reserved

	. = ORIGIN(version_ram);
    .version ALIGN(32):
	{
        *(.ota_loader.version)
        . = ALIGN(32);
	} > version_ram


    bt_ram_begin = ADDR(.bt_bss);
    bt_ram_size  = SIZEOF(.bt_bss);
#endif
}

//================== Section Info Export ====================//
text_begin  = ADDR(.text);
text_size   = SIZEOF(.text);
text_end    = text_begin + text_size;

bss_begin  = ADDR(.bss);
bss_size   = SIZEOF(.bss);

