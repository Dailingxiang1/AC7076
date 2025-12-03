#ifndef __PSRAM_API_H__
#define __PSRAM_API_H__


enum PSRAM_PORT_SEL_TABLE {
    PSRAM_PORT_SEL_PORTA = 0,
};

enum PSRAM_MODE_SEL_TABLE {
    PSRAM_MODE_1_WIRE = 0,
    PSRAM_MODE_4_WIRE_CMD1_ADR4_DAT4,
    PSRAM_MODE_4_WIRE_CMD4_ADR4_DAT4,
};


enum psram_power_status {
    PSRAM_STATE_POWER_OFF,
    PSRAM_STATE_POWER_ON,
    PSRAM_STATE_POWER_STANDBY,
};

struct psram_platform_data {
    u8 power_port;
    enum PSRAM_PORT_SEL_TABLE port;
    enum PSRAM_MODE_SEL_TABLE mode;
    u32 init_clk;
};

#define PSRAM_PLATFORM_DATA_BEGIN(data) \
		static const struct psram_platform_data data = {

#define PSRAM_PLATFORM_DATA_END()  \
};

/* ---------------------------------------------------------------------------- */
/**
 * @brief 获取psram在使用的buf数量
 *
 * @return psram在使用的buf数量
 */
/* ---------------------------------------------------------------------------- */
extern u32 psram_get_used_block(void);

/* ---------------------------------------------------------------------------- */
/**
 * @brief psram heap reset
 */
/* ---------------------------------------------------------------------------- */
extern void psram_heap_reset(void);
/* ------------------------------------------------------------------------------------*/
/**
 * @brief psram_flush_cache
 *
 * @param begin
 * @param len
 */
/* ------------------------------------------------------------------------------------*/
void psram_flush_cache(void *begin, u32 len);
/* ------------------------------------------------------------------------------------*/
/**
 * @brief psram_flush_invaild_cache
 *
 * @param begin
 * @param len
 */
/* ------------------------------------------------------------------------------------*/
void psram_flush_invaild_cache(void *begin, u32 len);
/* ------------------------------------------------------------------------------------*/
/**
 * @brief psram_init psram初始化
 *
 * @param config
 */
/* ------------------------------------------------------------------------------------*/
void psram_init(const struct psram_platform_data *config);

/* --------------------------------------------------------------------------*/
/**
 * @brief :检查mem是否psram cache地址
 *
 * @param mem :检查对象
 *
 * @return :true - psram地址，false - 非psram地址
 */
/* ----------------------------------------------------------------------------*/
int psram_cache_check(void *pv);
/* --------------------------------------------------------------------------*/
/**
 * @brief :检查mem是否psram no cache地址
 *
 * @param mem :检查对象
 *
 * @return :true - psram地址，false - 非psram地址
 */
/* ----------------------------------------------------------------------------*/
int psram_no_cache_check(void *pv);

/* --------------------------------------------------------------------------*/
/**
 * @brief :psram no cache地址转换为cache地址
 *
 * @param mem :检查对象
 *
 * @return :非0- psram cahce地址，NULL - 非psram地址
 */
/* ----------------------------------------------------------------------------*/
void *psram_no_cache_2_cache(void *pv);
/* --------------------------------------------------------------------------*/
/**
 * @brief :psram cache地址转换为no cache地址
 *
 * @param mem :检查对象
 *
 * @return :非0- psram no cahce地址，NULL - 非psram地址
 */
/* ----------------------------------------------------------------------------*/
void *psram_cache_2_no_cache(void *pv);

#endif
