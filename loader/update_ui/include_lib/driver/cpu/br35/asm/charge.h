#ifndef _CHARGE_H_
#define _CHARGE_H_

#include "typedef.h"
#include "device.h"

/*------充满电电压选择 4.044V-4.634V-------*/
//4.2V电池配置0~15
#define CHARGE_FULL_V_4040_4P2V  	0
#define CHARGE_FULL_V_4060_4P2V		1
#define CHARGE_FULL_V_4080_4P2V		2
#define CHARGE_FULL_V_4100_4P2V		3
#define CHARGE_FULL_V_4120_4P2V		4
#define CHARGE_FULL_V_4140_4P2V		5
#define CHARGE_FULL_V_4160_4P2V		6
#define CHARGE_FULL_V_4180_4P2V		7
#define CHARGE_FULL_V_4200_4P2V		8
#define CHARGE_FULL_V_4220_4P2V		9
#define CHARGE_FULL_V_4240_4P2V		10
#define CHARGE_FULL_V_4260_4P2V		11
#define CHARGE_FULL_V_4280_4P2V		12
#define CHARGE_FULL_V_4300_4P2V		13
#define CHARGE_FULL_V_4320_4P2V		14
#define CHARGE_FULL_V_4340_4P2V		15
//4.4V电池配置16~31
#define CHARGE_FULL_V_4240_4P4V		16
#define CHARGE_FULL_V_4260_4P4V		17
#define CHARGE_FULL_V_4280_4P4V		18
#define CHARGE_FULL_V_4300_4P4V		19
#define CHARGE_FULL_V_4320_4P4V		20
#define CHARGE_FULL_V_4340_4P4V		21
#define CHARGE_FULL_V_4360_4P4V		22
#define CHARGE_FULL_V_4380_4P4V		23
#define CHARGE_FULL_V_4400_4P4V		24
#define CHARGE_FULL_V_4420_4P4V		25
#define CHARGE_FULL_V_4440_4P4V		26
#define CHARGE_FULL_V_4460_4P4V		27
#define CHARGE_FULL_V_4480_4P4V		28
#define CHARGE_FULL_V_4500_4P4V		29
#define CHARGE_FULL_V_4520_4P4V		30
#define CHARGE_FULL_V_4540_4P4V		31
//4.5V电池配置32-47
#define CHARGE_FULL_V_4340_4P5V		32
#define CHARGE_FULL_V_4360_4P5V		33
#define CHARGE_FULL_V_4380_4P5V	 	34
#define CHARGE_FULL_V_4400_4P5V		35
#define CHARGE_FULL_V_4420_4P5V		36
#define CHARGE_FULL_V_4440_4P5V		37
#define CHARGE_FULL_V_4460_4P5V		38
#define CHARGE_FULL_V_4480_4P5V		39
#define CHARGE_FULL_V_4500_4P5V		40
#define CHARGE_FULL_V_4520_4P5V		41
#define CHARGE_FULL_V_4540_4P5V		42
#define CHARGE_FULL_V_4560_4P5V		43
#define CHARGE_FULL_V_4580_4P5V		44
#define CHARGE_FULL_V_4600_4P5V		45
#define CHARGE_FULL_V_4620_4P5V		46
#define CHARGE_FULL_V_4640_4P5V		47
#define CHARGE_FULL_V_MAX           48

/*
 	充电电流选择
	恒流：40-300mA
*/
#define CHARGE_mA_40			0
#define CHARGE_mA_50			1
#define CHARGE_mA_60			2
#define CHARGE_mA_70			3
#define CHARGE_mA_80			4
#define CHARGE_mA_100			5
#define CHARGE_mA_120			6
#define CHARGE_mA_140			7
#define CHARGE_mA_160			8
#define CHARGE_mA_180			9
#define CHARGE_mA_200			10
#define CHARGE_mA_220			11
#define CHARGE_mA_240			12
#define CHARGE_mA_260			13
#define CHARGE_mA_280			14
#define CHARGE_mA_300			15
#define CHARGE_mA_4             (BIT(4)|CHARGE_mA_40)
#define CHARGE_mA_5             (BIT(4)|CHARGE_mA_50)
#define CHARGE_mA_6             (BIT(4)|CHARGE_mA_60)
#define CHARGE_mA_7             (BIT(4)|CHARGE_mA_70)
#define CHARGE_mA_8             (BIT(4)|CHARGE_mA_80)
#define CHARGE_mA_10            (BIT(4)|CHARGE_mA_100)
#define CHARGE_mA_12            (BIT(4)|CHARGE_mA_120)
#define CHARGE_mA_14            (BIT(4)|CHARGE_mA_140)
#define CHARGE_mA_16            (BIT(4)|CHARGE_mA_160)
#define CHARGE_mA_18            (BIT(4)|CHARGE_mA_180)
#define CHARGE_mA_20            (BIT(4)|CHARGE_mA_200)
#define CHARGE_mA_22            (BIT(4)|CHARGE_mA_220)
#define CHARGE_mA_24            (BIT(4)|CHARGE_mA_240)
#define CHARGE_mA_26            (BIT(4)|CHARGE_mA_260)
#define CHARGE_mA_28            (BIT(4)|CHARGE_mA_280)
#define CHARGE_mA_30            (BIT(4)|CHARGE_mA_300)
#define CHARGE_TRICKLE_EN       BIT(4)
/* 充电口下拉电阻 50k ~ 200k */
#define CHARGE_PULLDOWN_50K     0
#define CHARGE_PULLDOWN_100K    1
#define CHARGE_PULLDOWN_150K    2
#define CHARGE_PULLDOWN_200K    3

#define CHARGE_CCVOL_V			300		//涓流充电向恒流充电的转换点

#define DEVICE_EVENT_FROM_CHARGE	(('C' << 24) | ('H' << 16) | ('G' << 8) | '\0')

struct charge_platform_data {
    u8 charge_en;	        //内置充电使能
    u8 charge_poweron_en;	//开机充电使能
    u8 charge_full_V;	    //充满电电压大小
    u8 charge_full_mA;	    //充满电电流大小
    u16 charge_mA;	        //恒流充电电流大小
    u16 charge_trickle_mA;  //涓流充电电流大小
    u8 ldo5v_pulldown_en;   //下拉使能位
    u8 ldo5v_pulldown_lvl;	//ldo5v的下拉电阻配置项,若充电舱需要更大的负载才能检测到插入时，请将该变量置为对应阻值
    u8 ldo5v_pulldown_keep; //下拉电阻在softoff时是否保持,ldo5v_pulldown_en=1时有效
    u16 ldo5v_off_filter;	//ldo5v拔出过滤值，过滤时间 = (filter*2 + 20)ms,ldoin<0.6V且时间大于过滤时间才认为拔出,对于充满直接从5V掉到0V的充电仓，该值必须设置成0，对于充满由5V先掉到0V之后再升压到xV的充电仓，需要根据实际情况设置该值大小
    u16 ldo5v_on_filter;    //ldo5v>vbat插入过滤值,电压的过滤时间 = (filter*2)ms
    u16 ldo5v_keep_filter;  //1V<ldo5v<vbat维持电压过滤值,过滤时间= (filter*2)ms
    u16 charge_full_filter; //充满过滤值,连续检测充满信号恒为1才认为充满,过滤时间 = (filter*2)ms
};

#define CHARGE_PLATFORM_DATA_BEGIN(data) \
		struct charge_platform_data data  = {

#define CHARGE_PLATFORM_DATA_END()  \
};


enum {
    CHARGE_EVENT_CHARGE_START,
    CHARGE_EVENT_CHARGE_CLOSE,
    CHARGE_EVENT_CHARGE_FULL,
    CHARGE_EVENT_LDO5V_KEEP,
    CHARGE_EVENT_LDO5V_IN,
    CHARGE_EVENT_LDO5V_OFF,
};

//使用的电池类型
enum {
    BAT_4P2, //4.2V的电池
    BAT_4P4,
    BAT_4P5,
};

void set_charge_event_flag(u8 flag);
void set_charge_online_flag(u8 flag);
void set_charge_event_flag(u8 flag);
u8 get_charge_online_flag(void);
u8 get_charge_poweron_en(void);
void set_charge_poweron_en(u32 onOff);
void charge_start(void);
void charge_close(void);
u8 get_charge_mA_config(void);
void set_charge_mA(u8 charge_mA);
u8 get_ldo5v_pulldown_en(void);
u8 get_ldo5v_pulldown_res(void);
u8 get_ldo5v_online_hw(void);
u8 get_lvcmp_det(void);
void charge_check_and_set_pinr(u8 mode);
u16 get_charge_full_value(void);
void charge_module_stop(void);
void charge_module_restart(void);
void ldoin_wakeup_isr(void);
int charge_init(const struct charge_platform_data *data);
void charge_set_ldo5v_detect_stop(u8 stop);
u8 check_pinr_shutdown_enable(void);

#endif    //_CHARGE_H_
