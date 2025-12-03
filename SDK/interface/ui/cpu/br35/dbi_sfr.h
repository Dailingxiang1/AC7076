#ifndef __DBI_SFR_H__
#define __DBI_SFR_H__
//===============================================================================//
//
//      config code mapping
//
//===============================================================================//

//  dbi_img_con -> fmt
//**********************************
typedef enum {
    DBI_IN_RGB565
    , DBI_IN_RGB888
} DBI_IN_FMT_ENUM;

//  dbi_hdl_con -> op
//**********************************
typedef enum {
    DBI_OP_SET_START   // 开启CS
    , DBI_OP_SET_END    // 关闭CS
    , DBI_OP_SET_TXC    // 切换到命令发送模式
    , DBI_OP_SET_TXD    // 切换到数据发送模式
    , DBI_OP_SET_RXD    // 切换到数据接收模式
    , DBI_OP_SEND_DLY   // 发送delay，无时钟输出
    , DBI_OP_SEND_DMY   // 发送dummy，有时钟输出
    , RESERVED
    , DBI_OP_SEND_DAT   // 发送数据
    , DBI_OP_RECE_DAT   // 接收数据
} DBI_OPCODE_ENUM;

//  dbi_flow_con0 -> protocol
//**********************************
typedef enum {
    DBI_TA
    , DBI_TB
    , DBI_TC_O1
    , DBI_TC_O3
    , DBI_TC_O4
    , DBI_MSPI
} DBI_PROTOCOL_ENUM;

//  dbi_flow_con0 -> pix_fmt
//**********************************
typedef enum {
    DBI_OUT_RGB565
    , DBI_OUT_RGB666
    , DBI_OUT_RGB888
} DBI_OUT_FMT_ENUM;

//  dbi_flow_con0 -> pix_line
//  dbi_flow_con0 -> cmd_line
//  dbi_flow_con0 -> adr_line
//  dbi_flow_con0 -> dma_line
//  dbi_hdl_con   -> line
//**********************************
typedef enum {
    DBI_LINE1
    , DBI_LINE2
    , DBI_LINE4
    , DBI_LINE8
    , DBI_LINE9
    , DBI_LINE16
    , DBI_LINE18
} DBI_LINE_ENUM;

//  dbi_flow_con0 -> pix_num
//**********************************
typedef enum {
    DBI_1_PIX
    , DBI_2_PIX
} DBI_PNUM_ENUM;

//  dbi_flow_con0 -> pix_spl
//**********************************
typedef enum {
    DBI_1_TRANS
    , DBI_2_TRANS
    , DBI_3_TRANS
} DBI_TNUM_ENUM;

//  dbi_flow_sline -> seg
//  dbi_flow_pline -> seg
//  dbi_flow_aline -> seg
//**********************************
#define DBI_SEG_START   BIT(0)
#define DBI_SEG_CMD     BIT(1)
#define DBI_SEG_DMY0    BIT(2)
#define DBI_SEG_DCX1    BIT(3)
#define DBI_SEG_ADR0    BIT(4)
#define DBI_SEG_ADR1    BIT(5)
#define DBI_SEG_ADR2    BIT(6)
#define DBI_SEG_DMY1    BIT(7)
#define DBI_SEG_HBP     BIT(8)
#define DBI_SEG_HACT    BIT(9)
#define DBI_SEG_HFP     BIT(10)
#define DBI_SEG_DMY2    BIT(11)
#define DBI_SEG_END     BIT(12)

//===============================================================================//
//
//      SFR sub function mapping
//
//===============================================================================//
typedef struct {
    __RW __u32 cf_hdl_done  :  1; // [0]
    __RW __u32 cf_task_done :  1; // [1]
    __RW __u32 tg_task_done :  1; // [2]
    __RW __u32 tg_task_tout :  1; // [3]
} t_dbi_pnd;

typedef struct {
    __RW __u32              :  1; // [0]
    __RW __u32 cf_task_kick :  1; // [1]
    __RW __u32 tg_task_kick :  1; // [2]
    __RO __u32              :  1; // [3]
    __RO __u32 xcvr_sta     :  4; // [7:4]
    __RO __u32 cf_hdl_busy  :  1; // [8]
    __RO __u32 cf_task_busy :  1; // [9]
    __RO __u32 tg_task_busy :  1; // [10]
    __RO __u32 fc_task_busy :  1; // [11]
    __RO __u32 fc_sbuf_busy :  1; // [12]
} t_dbi_sta_con;

typedef struct {
    __RW __u32 bus_nrst     :  1; // [0]
    __RW __u32 vdo_mode     :  1; // [1]
    __RW __u32 dma_mode     :  1; // [2]
    __RW __u32 test_mode    :  1; // [3]
    __RO __u32              :  4; // [7:4]
    __RW __u32 test_color   : 24; // [31:8]
} t_dbi_com_con;

typedef struct {
    __RW __u32 dat_en       : 18; // [17:0]
    __RW __u32 csx_en       :  1; // [18]
    __RW __u32 dcx_en       :  1; // [19]
    __RW __u32 rwx_en       :  1; // [20]
    __RW __u32 ck_en        :  1; // [21]
    __RW __u32 csx_pol      :  1; // [22]
    __RW __u32 dcx_pol      :  1; // [23]
    __RW __u32 rwx_pol      :  1; // [24]
    __RW __u32 ck_pol       :  1; // [25]
    __RW __u32 ck_phase     :  1; // [26]
    __RW __u32 force_cken   :  1; // [27]
} t_dbi_port_con;

typedef struct {
    __RW __u32 fmt          :  1; // [0]
    __RW __u32 big_end      :  1; // [1]
    __RW __u32 rb_swap      :  1; // [2]
} t_dbi_img_con;

typedef struct {
    __RW __u32 continue_mode:  1; // [0]
    __RW __u32 vsyn_oe      :  1; // [1]
    __RW __u32 vbp_oe       :  1; // [2]
    __RW __u32 vfp_oe       :  1; // [3]
    __RW __u32 vact_oe      :  1; // [4]
} t_dbi_tgen_con;

typedef struct {
    __RW __u32 protocol     :  3; // [2:0]
    __RW __u32 pix_line     :  3; // [5:3]
    __RW __u32 pix_num      :  1; // [6]
    __RW __u32 pix_spl      :  2; // [8:7]
    __RW __u32 pix_fmt      :  2; // [10:9]
    __RW __u32 cmd_line     :  3; // [13:11]
    __RW __u32 adr_line     :  3; // [16:14]
    __RW __u32 dma_line     :  3; // [19:17]
} t_dbi_flow_con0;

typedef struct {
    __RW __u32 dmy_cfg0     :  8; // [7:0]
    __RW __u32 dmy_cfg1     :  8; // [15:8]
    __RW __u32 dmy_cfg2     :  8; // [23:16]
    __RW __u32 dly_mod0     :  1; // [24]
    __RW __u32 dly_mod1     :  1; // [25]
    __RW __u32 dly_mod2     :  1; // [26]
} t_dbi_flow_con1;

typedef struct {
    __RW __u32 seg          : 13; // [12:0]
    __RO __u32              : 19; // [31:13]
    __RW __u32 cmd          :  8; // [7:0]
    __RW __u32 adr0         :  8; // [15:8]
    __RW __u32 adr1         :  8; // [23:16]
    __RW __u32 adr2         :  8; // [31:24]
} t_dbi_xline;

typedef struct {
    __RW __u32 cs_setup     :  8; // [7:0]
    __RW __u32 cs_hold      :  8; // [15:8]
    __RW __u32 sta_setup    :  8; // [23:16]
} t_dbi_flow_con8;

typedef struct {
    __RW __u32 op           :  4; // [3:0]
    __RW __u32 line         :  3; // [6:4]
    __RW __u32 num          :  2; // [8:7]
    __RW __u32 rx_mode      :  1; // [9]
    __RW __u32 rx_div       :  8; // [17:10]
    __RW __u32 rx_bsel      :  1; // [18]
} t_dbi_hdl_con;

typedef struct {
    __RW __u32 pix_fmt      :  2; // [1:0]
    __RW __u32 pix_cyc      :  2; // [3:2]
    __RW __u32 blk_mod      :  1; // [4]
    __RW __u32 rb_swap      :  1; // [5]
    __RW __u32 big_end      :  1; // [6]
} t_dbi_rgb_con;

//===============================================================================//
//
//      SFR address mapping
//
//===============================================================================//
#define dip_prp0_base   (hs_base + map_adr(0x22, 0x00))
#define dbi_msfr(i,j)   ((j *)(u32)(dip_prp0_base + i*4))
#define dbi_wsfr(i)      (*(volatile u32 *)(dip_prp0_base + i*4))

#define dbi_pnd_con     dbi_msfr(0x01, t_dbi_pnd)
#define dbi_pnd_clr     dbi_msfr(0x02, t_dbi_pnd)
#define dbi_pnd_ie      dbi_msfr(0x03, t_dbi_pnd)

#define dbi_sta_con     dbi_msfr(0x08, t_dbi_sta_con)
#define dbi_com_con     dbi_msfr(0x09, t_dbi_com_con)
#define dbi_port_con    dbi_msfr(0x0a, t_dbi_port_con)

#define dbi_img_con     dbi_msfr(0x10, t_dbi_img_con)
#define dbi_img_high    dbi_wsfr(0x11)
#define dbi_img_burst   dbi_wsfr(0x12)
#define dbi_img_stride  dbi_wsfr(0x13)
#define dbi_img_addr    dbi_wsfr(0x14)

#define dbi_tgen_con    dbi_msfr(0x20, t_dbi_tgen_con)
#define dbi_tgen_vsyn   dbi_wsfr(0x21)
#define dbi_tgen_vbp    dbi_wsfr(0x22)
#define dbi_tgen_vfp    dbi_wsfr(0x23)
#define dbi_tgen_vact   dbi_wsfr(0x24)
#define dbi_tgen_httl   dbi_wsfr(0x25)

#define dbi_flow_hsyn   dbi_wsfr(0x30)
#define dbi_flow_hbp    dbi_wsfr(0x31)
#define dbi_flow_hfp    dbi_wsfr(0x32)
#define dbi_flow_hact   dbi_wsfr(0x33)
#define dbi_flow_con0   dbi_msfr(0x34, t_dbi_flow_con0)
#define dbi_flow_con1   dbi_msfr(0x35, t_dbi_flow_con1)
#define dbi_flow_sline  dbi_msfr(0x36, t_dbi_xline)
//#define dbi_flow_con2   dbi_wsfr(0x36)
//#define dbi_flow_con3   dbi_wsfr(0x37)
#define dbi_flow_pline  dbi_msfr(0x38, t_dbi_xline)
//#define dbi_flow_con4   dbi_wsfr(0x38)
//#define dbi_flow_con5   dbi_wsfr(0x39)
#define dbi_flow_aline  dbi_msfr(0x3a, t_dbi_xline)
//#define dbi_flow_con6   dbi_wsfr(0x3a)
//#define dbi_flow_con7   dbi_wsfr(0x3b)
#define dbi_flow_con8   dbi_msfr(0x3c, t_dbi_flow_con8)

#define dbi_hdl_con     dbi_wsfr(0x3d) //, t_dbi_hdl_con)
#define dbi_hdl_buf     dbi_wsfr(0x3e)

#define dbi_rgb_con     dbi_msfr(0x3f, t_dbi_rgb_con)

#endif
