/*********************************************************************************************
    *   Filename        : gpio.h

    *   Description     : 本文件存放p11 gpio的接口函数和宏定义

    *   Author          : MoZhiYe

    *   Email           : mozhiye@zh-jieli.com

    *   Last modifiled  : 2021-05-17 09:00

    *   Copyright:(c)JIELI  2021-2029  @ , All Rights Reserved.
*********************************************************************************************/

#ifndef __GPIO_H__
#define __GPIO_H__

#define IO_GROUP_NUM 				16

//PB0 ~ PB13
#define IO_PORTB_00 				(IO_GROUP_NUM * 1 + 0)
#define IO_PORTB_01 				(IO_GROUP_NUM * 1 + 1)
#define IO_PORTB_02 				(IO_GROUP_NUM * 1 + 2)
#define IO_PORTB_03 				(IO_GROUP_NUM * 1 + 3)
#define IO_PORTB_04 				(IO_GROUP_NUM * 1 + 4)
#define IO_PORTB_05 				(IO_GROUP_NUM * 1 + 5)
#define IO_PORTB_06 				(IO_GROUP_NUM * 1 + 6)
#define IO_PORTB_07 				(IO_GROUP_NUM * 1 + 7)
#define IO_PORTB_08 				(IO_GROUP_NUM * 1 + 8)

#define IO_MAX_NUM 					(IO_PORTB_08 + 1)

#ifndef EINVAL
#define EINVAL      22  /* Invalid argument */
#endif /* #ifndef EINVAL */

#ifndef EFAULT
#define	EFAULT		14	/* Bad address */
#endif /* #ifndef EFAULT */

enum gpio_drive_strength {
    PORT_DRIVE_STRENGT_2p4mA,		///< 最大驱动电流  2.4mA
    PORT_DRIVE_STRENGT_8p0mA,		///< 最大驱动电流  8.0mA
    PORT_DRIVE_STRENGT_24p0mA,		///< 最大驱动电流 24.0mA
    PORT_DRIVE_STRENGT_64p0mA,		///< 最大驱动电流 64.0mA
};

//===================================================//
// BR27 P11 Crossbar API
//===================================================//
enum PFI_TABLE {
    PFI_GP_ICH0 = ((u32)(&(P11_IMAP->P11_FI_GP_ICH0))),
    PFI_GP_ICH1 = ((u32)(&(P11_IMAP->P11_FI_GP_ICH1))),
    PFI_GP_ICH2 = ((u32)(&(P11_IMAP->P11_FI_GP_ICH2))),
    PFI_UART0_RX = ((u32)(&(P11_IMAP->P11_FI_UART0_RX))),
    PFI_IIC_SCL = ((u32)(&(P11_IMAP->P11_FI_IIC_SCL))),
    PFI_IIC_SDA = ((u32)(&(P11_IMAP->P11_FI_IIC_SDA))),
};

int gpio_set_direction(u32 gpio, u32 dir);

int gpio_set_output_value(u32 gpio, u32 value);

int gpio_set_pull_up(u32 gpio, u32 value);

int gpio_set_pull_down(u32 gpio, int value);

int gpio_set_hd(u32 gpio, int value);

int gpio_set_die(u32 gpio, int value);

int gpio_set_dieh(u32 gpio, int value);

int gpio_set_spl(u32 gpio, int value);

int gpio_read(u32 gpio);

int gpio_set_fun_output_port(u32 gpio, u32 fun_index, u8 dir_ctl, u8 data_ctl);
int gpio_disable_fun_output_port(u32 gpio);

int gpio_set_fun_input_port(u32 gpio, enum PFI_TABLE pfun);
int gpio_disable_fun_input_port(enum PFI_TABLE pfun);


#endif /* #ifndef __GPIO_H__ */
