#ifndef _MEM_HEAP_H_
#define _MEM_HEAP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "typedef.h"

typedef enum {
    P_MEMORY_TOTAL,       // ram0
    P_MEMORY_UNUSED,      // ram0
    P_MEMORY_USED,        // ram0

    P_VLT_MEMORY_TOTAL,   // ram1
    P_VLT_MEMORY_UNUSED,  // ram1
    P_VLT_MEMORY_USED,    // ram1

    P_HEAP_SIZE,          // ram0 + ram1
} MEMORY_TYPE;

void memory_init(void);

void *malloc(size_t size);
void *zalloc(size_t size);
void free(void *mem);

void mem_stats(void);
/* ---------------------------------------------------------------------------- */
/**
 * @brief :获取物理内存的大小
 *
 * @param type: 需要获取的内存类型;
 *
 * @return : 对应类型物理内存大小
 */
/* ---------------------------------------------------------------------------- */
size_t memory_get_size(MEMORY_TYPE type);

/* ---------------------------------------------------------------------------- */
/**
 * @brief :申请连续空间的物理内存
 *
 * @param size: 申请内存大小, 单位byte;
 *
 * @return : 物理内存地址
 */
/* ---------------------------------------------------------------------------- */
void *phy_malloc(size_t size);

/* ---------------------------------------------------------------------------- */
/**
 * @brief :释放连续空间的物理内存
 *
 * @param :物理内存地址
 */
/* ---------------------------------------------------------------------------- */
void phy_free(void *pv);


/* --------------------------------------------------------------------------*/
/**
 * @brief dma_malloc
 *
 * @param size: 申请内存大小, 单位byte;
 *
 * @return : dma内存地址
 */
/* ----------------------------------------------------------------------------*/
void *dma_malloc(size_t size);

/* ---------------------------------------------------------------------------- */
/**
 * @brief :释放连续空间的dma内存
 *
 * @param :dma内存地址
 */
/* ---------------------------------------------------------------------------- */
void dma_free(void *pv);

/* --------------------------------------------------------------------------*/
/**
 * @brief :检查mem是否物理内存地址
 *
 * @param mem :检查对象
 *
 * @return :true - 物理内存，false - 非物理内存
 */
/* ----------------------------------------------------------------------------*/
int memory_in_phy(const void *mem);

/* --------------------------------------------------------------------------*/
/**
 * @brief :检查mem是否虚拟内存地址
 *
 * @param mem :检查对象
 *
 * @return :true - 虚拟内存，false - 非虚拟内存
 */
/* ----------------------------------------------------------------------------*/
int memory_in_vtl(const void *mem);
/* --------------------------------------------------------------------------*/
/**
 * @brief :检查mem是否psram地址
 *
 * @param mem :检查对象
 *
 * @return :true - psram地址，false - 非psram地址
 */
/* ----------------------------------------------------------------------------*/
int memory_in_psram(const void *pv);
/* --------------------------------------------------------------------------*/
/**
 * @brief :检查mem是否flash cache地址
 *
 * @param mem :检查对象
 *
 * @return :true - flash cache地址，false - 非flash cache地址
 */
/* ----------------------------------------------------------------------------*/
int memory_in_flash_cache(const void *pv);
/* --------------------------------------------------------------------------*/
/**
 * @brief :检查mem是否flash no cache地址
 *
 * @param mem :检查对象
 *
 * @return :true - flash no cache地址，false - 非flash no cache地址
 */
/* ----------------------------------------------------------------------------*/
int memory_in_flash_nocache(const void *pv);
#define memory_in_flash(addr) 	(memory_in_flash_cache((const void *)(addr)) || memory_in_flash_nocache((const void *)(addr)))
/* --------------------------------------------------------------------------*/
/**
 * @brief :获取空闲可掉电内存
 *
 * @param mem :
 *
 * @return :
 */
/* ----------------------------------------------------------------------------*/
extern u16 addr_map_2_ram(u32 start);

/* --------------------------------------------------------------------------*/
/**
 * @brief :启动内存整理
 *
 * @param mem :
 *
 * @return :
 */
/* ----------------------------------------------------------------------------*/
extern u8 memory_defrag(void);

/* --------------------------------------------------------------------------*/
/**
 * @brief :获取空闲ram的起始地址
 *
 * @param mem :
 *
 * @return :
 */
/* ----------------------------------------------------------------------------*/
extern u32 pmalloc_get_unused_addr();
/* ------------------------------------------------------------------------------------*/
/**
 * @brief psram_heap_stats psram使用情况
 */
/* ------------------------------------------------------------------------------------*/
extern void psram_heap_stats(void);
/* ------------------------------------------------------------------------------------*/
/**
 * @brief malloc_psram 申请psram
 *
 * @param size	大小
 *
 * @return psram地址
 */
/* ------------------------------------------------------------------------------------*/
extern void *malloc_psram(size_t size);
/* ------------------------------------------------------------------------------------*/
/**
 * @brief realloc_psram 申请psram
 *
 * @param pv	原内存地址
 * @param size	大小
 *
 * @return psram地址
 */
/* ------------------------------------------------------------------------------------*/
extern void *realloc_psram(void *pv, size_t size);
/* ------------------------------------------------------------------------------------*/
/**
 * @brief free_psram 释放psram
 *
 * @param pv	地址
 */
/* ------------------------------------------------------------------------------------*/
extern void free_psram(void *pv);

//******************************************************************************************
/*内存整理的锁申请
 */
struct memory_defrag_lock_request {
    char *name;
    u8(*wait_is_idle)();   //查询能否整理,返回0即本次不整理
    void (*request_lock)();//注册内存整理的锁
    void (*request_unlock)();
};

#define REGISTER_MEMORY_DEFRAG_REQUEST(target) \
        const struct memory_defrag_lock_request target sec(.memory_defrag_lock)

extern const struct memory_defrag_lock_request memory_defrag_lock_begin[];
extern const struct memory_defrag_lock_request memory_defrag_lock_end[];

#define list_for_each_memory_defrag_request(p) \
    for (p = memory_defrag_lock_begin; p < memory_defrag_lock_end; p++)


#define list_for_each_reverse_memory_defrag_request(p) \
    for (p = memory_defrag_lock_end-1; p >= memory_defrag_lock_begin; p--)




#ifdef __cplusplus
}
#endif

#endif /* _MEM_HEAP_H_ */
