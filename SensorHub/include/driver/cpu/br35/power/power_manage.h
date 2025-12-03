#ifndef __POWER_MANAGE_H__
#define __POWER_MANAGE_H__

//******************************************************************************************
/*p11低功耗等级
 */
enum LOW_POWER_LEVEL {
    P11_POWER_MODE_RUN = 0,
    P11_POWER_MODE_IDLE,
    P11_POWER_MODE_STANDBY,
    P11_POWER_MODE_SLEEP,
    P11_POWER_MODE_DEEP_SLEEP,
    P11_POWER_MODE_DEEP_SLEEP_BEST_PDOWN,
    P11_POWER_MODE_DEEP_SLEEP_BEST_POFF,
};

struct low_power_target {
    char *name;
    enum LOW_POWER_LEVEL(*level)();
};

extern const struct low_power_target low_power_target_begin[];
extern const struct low_power_target low_power_target_end[];

#define list_for_each_low_power_target(p) \
    for (p = low_power_target_begin; p < low_power_target_end; p++)

#define REGISTER_LOWPOWER_TARGET(target) \
        static const struct low_power_target target SEC_USED(.lp_target)

enum LOW_POWER_LEVEL p11_low_power_level(void);

u8 is_p11_low_power_mode(enum LOW_POWER_LEVEL level);

//******************************************************************************************
/*p11低功耗回调
 */

struct low_power_callback {
    void (*low_power_enter)(enum LOW_POWER_LEVEL level);
    void (*low_power_exit)(enum LOW_POWER_LEVEL level);
};

extern const struct low_power_callback low_power_callback_begin[];
extern const struct low_power_callback low_power_callback_end[];

#define list_for_each_low_power_callback(p) \
    for (p = low_power_callback_begin; p < low_power_callback_end; p++)

#define REGISTER_LOWPOWER_CALLBACK(callback) \
        static const struct low_power_callback target SEC_USED(.lp_target)

//******************************************************************************************
/*
 * deepsleep register
 */
struct deepsleep_target {
    char *name;
    u8(*enter)(void);
    u8(*exit)(void);
};

#define DEEPSLEEP_TARGET_REGISTER(target) \
        const struct deepsleep_target target SEC_USED(.deepsleep_target)

extern const struct deepsleep_target deepsleep_target_begin[];
extern const struct deepsleep_target deepsleep_target_end[];

#define list_for_each_deepsleep_target(p) \
    for (p = deepsleep_target_begin; p < deepsleep_target_end; p++)




//******************************************************************************************
struct _phw_dev {
};

struct phw_dev_ops {
    void *(*early_init)(u32 arg);
    u32(*init)(struct _phw_dev *dev, u32 arg);
    u32(*ioctl)(struct _phw_dev *dev, u32 cmd, u32 arg);

    u32(*sleep_already)(struct _phw_dev *dev, u32 arg);
    u32(*sleep_prepare)(struct _phw_dev *dev, u32 arg);
    u32(*sleep_enter)(struct _phw_dev *dev, u32 arg);
    u32(*sleep_exit)(struct _phw_dev *dev, u32 arg);
    u32(*sleep_post)(struct _phw_dev *dev, u32 arg);

    u32(*soff_prepare)(struct _phw_dev *dev, u32 arg);
    u32(*soff_enter)(struct _phw_dev *dev, u32 arg);
    u32(*soff_exit)(struct _phw_dev *dev, u32 arg);

    u32(*deepsleep_enter)(struct _phw_dev *dev, u32 arg);
    u32(*deepsleep_exit)(struct _phw_dev *dev, u32 arg);
};

#define REGISTER_PHW_DEV_PMU_OPS(ops) \
		const struct phw_dev_ops *phw_pmu_ops = &ops

extern const struct phw_dev_ops *phw_pmu_ops;


#endif
