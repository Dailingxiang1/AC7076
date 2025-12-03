#ifndef  __EFUSE_H__
#define  __EFUSE_H__


u32 get_chip_version();
u32 efuse_get_chip_id();
u16 get_chip_id();
u32 efuse_get_gpadc_vbg_trim();
void   efuse_init();

u8 get_en_act();
u8 get_vddio_lvd_en();
u8 efuse_get_lvd_act();
u8 efuse_get_vddio_lvd_lev();
u8 efuse_get_vio_act();
u8 efuse_get_vddio_lev();
u8 efuse_get_mclr_en_dis();
u8 efuse_get_xosc_pin_auto();
u8 efuse_get_xosc_pin_mode();
u8 efuse_get_sfc_fast_boot_dis();
u8 efuse_get_pin_reset_en();
u8 efuse_get_fast_up();
u8 efuse_get_flash_io_select();
u8 efuse_get_vbg_act();
u8 efuse_get_lvd_bg_trim();
u8 efuse_get_mvbg_lev();
u8 efuse_get_en_wvbg_lev();

u8 efuse_get_cp_pass();
u8 efuse_get_ft_pass();
u8 efuse_get_wvdd_level_trim();
u8 efuse_get_vbat_trim_4p2(void);
u8 efuse_get_vbat_trim_4p4(void);
u8 efuse_get_vbat_trim_4p5(void);
u8 efuse_get_charge_cur_trim(void);
u8 efuse_get_io_pu_100k(void);
u8 efuse_get_flash_type_select(void);//1: NOR 0:NAND

u8 efuse_get_apa_vb17_vbg();
u8 efuse_get_xosc_ldo();
u8 efuse_get_xosc_ext_init();
u8 efuse_get_lrc24m_caps();
u8 efuse_get_lrc24m_rs();

#endif  /*EFUSE_H*/
