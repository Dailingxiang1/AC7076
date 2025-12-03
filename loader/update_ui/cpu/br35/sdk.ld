


























  __local_irq_enable = ABSOLUTE(0xffd5c);
  __local_irq_disable = ABSOLUTE(0xffd60);
  request_irq = ABSOLUTE(0xffd64);
  lp_signature_set = ABSOLUTE(0xffd68);
  memcpy = ABSOLUTE(0xffd6c);
  memmove = ABSOLUTE(0xffd70);
  memcmp = ABSOLUTE(0xffd74);
  memset = ABSOLUTE(0xffd78);
  memchr = ABSOLUTE(0xffd7c);
  strcmp = ABSOLUTE(0xffd80);
  strcpy = ABSOLUTE(0xffd84);
  strncpy = ABSOLUTE(0xffd88);
  strlen = ABSOLUTE(0xffd8c);
  strncmp = ABSOLUTE(0xffd90);
  strstr = ABSOLUTE(0xffd94);
  strchr = ABSOLUTE(0xffd98);
  strrchr = ABSOLUTE(0xffd9c);
  strcat = ABSOLUTE(0xffda0);
  __divdi3 = ABSOLUTE(0xffda4);
  __umoddi3 = ABSOLUTE(0xffda8);
  __muldi3 = ABSOLUTE(0xffdac);
  __udivdi3 = ABSOLUTE(0xffdb0);
  __udivmoddi4 = ABSOLUTE(0xffdb4);
  ic_pll_init = ABSOLUTE(0xffdb8);
  sfc_suspend = ABSOLUTE(0xffdbc);
  sfc_resume = ABSOLUTE(0xffdc0);
  sfc_drop_cache = ABSOLUTE(0xffdc4);
  sfc_boot = ABSOLUTE(0xffdc8);
  sfc_entry_continue_mode = ABSOLUTE(0xffdcc);
  ota_jump_mode = ABSOLUTE(0xffdd0);
  DcuInitial = ABSOLUTE(0xffdd4);
  DcuFlushinvAll = ABSOLUTE(0xffdd8);
  DcuWaitIdle = ABSOLUTE(0xffddc);
  IcuInitial = ABSOLUTE(0xffde0);
  IcuFlushinvAll = ABSOLUTE(0xffde4);
  IcuWaitIdle = ABSOLUTE(0xffde8);
  DcuEnable = ABSOLUTE(0xffdec);
  DcuFlushRegion = ABSOLUTE(0xffdf0);
  chip_crc16 = ABSOLUTE(0xffdf4);
  CrcDecode = ABSOLUTE(0xffdf8);
  mask_init = ABSOLUTE(0xffdfc);
  wdt_clear = ABSOLUTE(0xffe00);
  nvram_set_boot_state = ABSOLUTE(0xffe04);
  chip_reset = ABSOLUTE(0xffe08);
  flash_poweron = ABSOLUTE(0xffe0c);
  flash_poweron_base = ABSOLUTE(0xffe10);
  flash_poweroff = ABSOLUTE(0xffe14);
  flash_poweroff_base = ABSOLUTE(0xffe18);
  spi_io_mount = ABSOLUTE(0xffe1c);
  spi_io_unmount = ABSOLUTE(0xffe20);
  get_sfc_read_mode = ABSOLUTE(0xffe24);
  spi_port_io_init = ABSOLUTE(0xffe28);
  spi_port_io_uninit = ABSOLUTE(0xffe2c);
  usb_slave_mode = ABSOLUTE(0xffe30);
  uart_set_dma_dir = ABSOLUTE(0xffe34);
  doe = ABSOLUTE(0xffe38);
  LZ4_setStreamDecode = ABSOLUTE(0xffe3c);
  LZ4_decompress_safe_continue = ABSOLUTE(0xffe40);
  LZ4_decompress_generic = ABSOLUTE(0xffe44);
  LZ4_decompress_safe_usingDict = ABSOLUTE(0xffe48);
  dec_isd_cfg_ini = ABSOLUTE(0xffe4c);
  rom_nand_flash_get_features = ABSOLUTE(0xffe50);
  rom_nand_flash_set_features = ABSOLUTE(0xffe54);
  pmalloc_init = ABSOLUTE(0xfff84);
  pmalloc = ABSOLUTE(0xfff88);
  pfree = ABSOLUTE(0xfff8c);
  pmalloc_get_unused_pages = ABSOLUTE(0xfff90);
  get_physic_address = ABSOLUTE(0xfff94);
  pmalloc_get_hdl = ABSOLUTE(0xfff98);
  vmm_init_ = ABSOLUTE(0xfff9c);
  vmalloc_ = ABSOLUTE(0xfffa0);
  vfree_ = ABSOLUTE(0xfffa4);
  vmem_get_phy_adr = ABSOLUTE(0xfffa8);
  vmem_start = ABSOLUTE(0xfffac);
  vmem_end = ABSOLUTE(0xfffb0);
  vmalloc_get_hdl = ABSOLUTE(0xfffb4);
  vmalloc_v2 = ABSOLUTE(0xfffb8);
  vfree_v2 = ABSOLUTE(0xfffbc);
  vcopy_ = ABSOLUTE(0xfffc0);
  lock_vir_mem = ABSOLUTE(0xfffc4);
  unlock_vir_mem = ABSOLUTE(0xfffc8);
  hw_mmu_disable = ABSOLUTE(0xfffcc);
  get_sbc_frame_info = ABSOLUTE(0xfffd0);
  get_sbc_dec_ops = ABSOLUTE(0xfffd4);
  sbc_dec_loop_u = ABSOLUTE(0xfffd8);
  sbc_unpack_frame_u = ABSOLUTE(0xfffdc);
  sbc_synthesize_audio_u = ABSOLUTE(0xfffe0);
  sbc_oput_deal_normal = ABSOLUTE(0xfffe4);
  sbc_oput_deal_To_ch2 = ABSOLUTE(0xfffe8);
  sbc_bit_offset4 = ABSOLUTE(0xf88c0);
  sbc_bit_offset8 = ABSOLUTE(0xf8900);
  sbc_proto4_m0 = ABSOLUTE(0xf8980);
  sbc_proto4_m1 = ABSOLUTE(0xf89d0);
  sbc_proto8_m0 = ABSOLUTE(0xf8a20);
  sbc_proto8_m1 = ABSOLUTE(0xf8ac0);
  syn_matrix4 = ABSOLUTE(0xf8b60);
  syn_matrix8 = ABSOLUTE(0xf8be0);
  crc8_table = ABSOLUTE(0xf87c0);
  sbc_div_new_table = ABSOLUTE(0xf8de0);
  need_fix_iir_filter_buf = ABSOLUTE(0xfffec);
  fix_iir_filter_init = ABSOLUTE(0xffff0);
  fix_iir_filter_update = ABSOLUTE(0xffff4);
  fix_iir_filter_run = ABSOLUTE(0xffff8);
  fix_iir_filter_clear_mem = ABSOLUTE(0xffffc);
  _SFC_MEMORY_START_ADDR = ABSOLUTE(0xc000000);
  _IRQ_MEM_ADDR = ABSOLUTE(0x100000);
  _RAM_LIMIT_L = ABSOLUTE(0x100000);
  _RAM_LIMIT_H = ABSOLUTE(0x137000);
  _MASK_USED_MEM_END = ABSOLUTE(0x101310);
  _MASK_EXPORT_MEM_BEGIN = ABSOLUTE(0x100200);
  _MASK_EXPORT_MEM_SIZE = ABSOLUTE(0x34c);
  _UBOOT_LOADER_RAM_START = ABSOLUTE(0x102600);
  _UBOOT_LOADER_RAM_END = ABSOLUTE(0x137000);
  _UBOOT_LOADER_RAM_SIZE = ABSOLUTE(0x34a00);
  g_boot_soft_flag = ABSOLUTE(0x100204);
  irq_lock_cnt = ABSOLUTE(0x1002b8);
  efuse_page0 = ABSOLUTE(0x1002ac);
  efuse_page1 = ABSOLUTE(0x1002b0);
  efuse_page2 = ABSOLUTE(0x1002b4);
  nvram_list = ABSOLUTE(0x100214);
  exception_irq_handler = ABSOLUTE(0xf8004);

EXTERN(
_start



source_node_adapter
mixer_node_adapter
dac_node_adapter
classd_node_adapter
clk_sync_node_adapter
decoder_node_adapter
resample_node_adapter
bt_audio_sync_node_adapter
adc_file_plug
tone_file_plug
ring_file_plug
key_tone_file_plug
sbc_decoder_plug
msbc_decoder_plug
sine_dec_plug
cvsd_decoder_plug
pcm_dec_plug
zero_file_plug
msbc_encoder_soft_plug






cvsd_encoder_plug
music_file_plug
wdrc_node_adapter
g729_dec_plug
fat_vfs_ops



fat_sdfile_fat_ops



fat_sdfile_fat_ops
sdfile_resfile_ops
sdfile_vfs_ops
ex_sdfile_vfs_ops
wav_package
sof_eq_node_adapter
);

UPDATE_SIZE = 0x80;
UPDATE_BEG = 0x10c800;


EX_CODE_BEGIN = 0x10054c;
EX_CODE_SIZE  = 0xa000;
EX_CODE_END	  = EX_CODE_BEGIN + EX_CODE_SIZE;
EX_RAM_BEGIN  = EX_CODE_BEGIN + EX_CODE_SIZE;
EX_RAM_SIZE   = UPDATE_BEG - EX_RAM_BEGIN;

RAM_LIMIT_L = _UBOOT_LOADER_RAM_START;
RAM_LIMIT_H = _UBOOT_LOADER_RAM_START + 0x8000;
PHY_RAM_SIZE = 0x8000;


ISR_BASE = _IRQ_MEM_ADDR;
ROM_RAM_SIZE = _MASK_EXPORT_MEM_SIZE;
ROM_RAM_BEG = _MASK_EXPORT_MEM_BEGIN;

RAM0_BEG = RAM_LIMIT_L;
RAM0_END = RAM_LIMIT_H;
RAM0_SIZE = RAM0_END - RAM0_BEG;

RAM1_BEG = RAM_LIMIT_L;
RAM1_END = RAM0_BEG;
RAM1_SIZE = RAM1_END - RAM1_BEG;



PSRAM_BEGIN = 0x8000000;
PSRAM_SIZE = 0x0;
PSRAM_END = PSRAM_BEGIN + PSRAM_SIZE;


FREE_DACHE_WAY = 2;
FREE_IACHE_WAY = 0;
DCACHE_RAM_SIZE = FREE_DACHE_WAY*8K;
ICACHE_RAM_SIZE = FREE_IACHE_WAY*8K;
MEMORY
{
 code0(rx) : ORIGIN = EX_CODE_BEGIN, LENGTH = EX_CODE_SIZE
 ram0(rwx) : ORIGIN = EX_RAM_BEGIN, LENGTH = EX_RAM_SIZE




    psram(rwx) : ORIGIN = PSRAM_BEGIN, LENGTH = PSRAM_SIZE

    dcache_ram(rw) : ORIGIN = 0x370000+((4-FREE_DACHE_WAY)*8K), LENGTH = DCACHE_RAM_SIZE
    icache_ram(rw) : ORIGIN = 0x3C0000+((4-FREE_IACHE_WAY)*8K), LENGTH = ICACHE_RAM_SIZE

}





SECTIONS
{
    . = ORIGIN(code0);
    .text ALIGN(4):
    {
		. = ALIGN(4);
        API_TAB_BEGIN = .;
		KEEP(*(.api_tab))
		API_TAB_END = .;

        . = ALIGN(4);
        lcd_interface_begin = .;
        KEEP(*(.lcd_if_info))
        lcd_interface_end = .;

        . = ALIGN(4);
        lcd_device_begin = .;
        KEEP(*(.lcd_device_info))
        lcd_device_end = .;

        . = ALIGN(4);
        clock_critical_handler_begin = .;
        KEEP(*(.clock_critical_txt))
        clock_critical_handler_end = .;


		. = ALIGN(4);
		KEEP(*(.text*))
		. = ALIGN(4);
        KEEP(*(.rodata*))
		. = ALIGN(4);
		KEEP(*(.data))
		. = ALIGN(4);
		_bss_begin = . ;
        KEEP(*(.bss))
		_bss_end = . ;
		. = ALIGN(4);

	 _HEAP_BEGIN = . ;
	_HEAP_END = EX_CODE_END;
   } > code0

    . = ORIGIN(ram0);
    .mmu_tlb ALIGN(0x1000):
    {
        *(.mmu_tlb_segment);
    } > ram0


 .data ALIGN(32):
 {

        . = ALIGN(4);
        *(.data_magic)

  . = ALIGN(4);
 } > ram0


 .data_code ALIGN(32):
 {
  data_code_pc_limit_begin = .;
  *(.flushinv_icache)
        *(.cache)
        *(.os_critical_code)
        *(.fat_data_code)
        *(.fat_data_code_ex)

        *(.ui_ram)
        *(.math_fast_funtion_code)

  . = ALIGN(4);
        _SPI_CODE_START = . ;
        *(.spi_code)
  . = ALIGN(4);
        _SPI_CODE_END = . ;
  . = ALIGN(4);

 } > ram0
 .moveable_slot ALIGN(4):
 {
     *(movable.slot.*)

 } >ram0
 __report_overlay_begin = .;


 data_code_pc_limit_end = .;
 __report_overlay_end = .;


}
text_begin = ADDR(.text);
text_size = SIZEOF(.text);
text_end = text_begin + text_size;

/*PROVIDE(bss_begin = _bss_begin);*/
/*PROVIDE(bss_end = _bss_end);*/
_bss_size = _bss_end - _bss_begin;
/*PROVIDE(bss_size = _bss_end - _bss_begin);*/

data_addr = ADDR(.data);
data_begin = text_begin + text_size;
data_size = SIZEOF(.data);

moveable_slot_addr = ADDR(.moveable_slot);
moveable_slot_begin = data_begin + data_size;
moveable_slot_size = SIZEOF(.moveable_slot);

data_code_addr = ADDR(.data_code);
data_code_begin = moveable_slot_begin + moveable_slot_size;
data_code_size = SIZEOF(.data_code);


PROVIDE(HEAP_BEGIN = _HEAP_BEGIN);
PROVIDE(HEAP_END = _HEAP_END);
_MALLOC_SIZE = _HEAP_END - _HEAP_BEGIN;
PROVIDE(MALLOC_SIZE = _HEAP_END - _HEAP_BEGIN);

ASSERT(MALLOC_SIZE >= 0x8000, "heap space too small !")


