#include "media_memory.h"
#include "malloc.h"
#include "app_config.h"

#if 0

#if (defined(TCFG_PSRAM_DEV_ENABLE) && TCFG_PSRAM_DEV_ENABLE)
#include "asm/psram_api.h"
#endif

//#define MMM_LOG_DEBUG
#ifdef MMM_LOG_DEBUG
#define MMM_LOG	y_printf
#else
#define MMM_LOG(...)
#endif

#pragma   code_seg(".audio.text.cache.L1")

extern int PSRAM_BEGIN;

/*将要使用psram的模块对应得枚举添加到以下列表，不在列表中的模块默认使用sram*/
const enum audio_module media_psram_module[] =  {
    /*codec module list*/
    AUD_MODULE_MP3_DEC,
    AUD_MODULE_MP3_DEC_TMP,

    /*CVP module list*/
    AUD_MODULE_CVP_1MIC_RUN,
    AUD_MODULE_CVP_1MIC_TMP,

    /*CVP module list*/
    AUD_MODULE_KWS
};

void *media_malloc(enum audio_module module, size_t size)
{
#if (defined(TCFG_PSRAM_DEV_ENABLE) && TCFG_PSRAM_DEV_ENABLE)
    for (int i = 0; i < ARRAY_SIZE(media_psram_module); i++) {
        MMM_LOG("media_psram_module[%d]:%d,module:%d\n", i, media_psram_module[i], module);
        if (media_psram_module[i] == module) {
            void *ptr = malloc_psram(size);
            if (ptr) {
                memset(ptr, 0, size);
            }
            MMM_LOG("media_malloc psram:0x%x\n", (int)ptr);
            return ptr;
        }
    }
#endif
    return zalloc(size);
}

void media_free(void *pv)
{
#if (defined(TCFG_PSRAM_DEV_ENABLE) && TCFG_PSRAM_DEV_ENABLE)
    if ((int)pv >= (int)(&PSRAM_BEGIN)) {
        free_psram(pv);
    } else {
        free(pv);
    }
#else
    free(pv);
#endif
}

#endif
