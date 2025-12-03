#include "update_main.h"
/* #include "common.h" */
#include "norflash.h"

#define LOG_TAG_CONST       USER_LC_UPDATE
#define LOG_TAG             "[USER_LC_UPDATE]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

//============================================================//
/* APP LOCAL_Flash 升级模型(APP_LOCAL_FLASH_UPDATA)
  ________               ________              ________
 |        |             |        |            |        |
 |        |    ufw      |        |    ufw     |        |
 |   APP  | --------->  |  CHIP  | -------->  |LC_Flash|
 |        |    ops      |        |    ops     |        |
 |________|             |________|            |________|
                            ^                      |
                            |______________________|
							         ota.bin
 */
//============================================================//

u32 user_lc_flash_update_get_base_addr(void);


//需要获取信息:
//src addr, len
//dest addr, len
typedef struct _lc_flash_update_info {
    u32 src_file_addr;
    u32 src_file_len;
} lc_flash_update_info_parm;

//==========================================================//
//全局变量
//==========================================================//
static lc_flash_update_info_parm lc_flash_update_info = {0};

static u8 lc_flash_init = 0;
static u32 lc_flash_file_offset = 0;

static volatile u32 mutil_ufw_offset = 0;

///////////////////lc_flash base api///////////////////////////////
int user_lc_flash_f_seek(void *fp, u8 type, u32 offset)
{
    if (type == SEEK_SET) {
        offset += mutil_ufw_offset;
        lc_flash_file_offset = offset;
    } else if (type == SEEK_CUR) {
        lc_flash_file_offset += offset;
    }

    return 0;//FR_OK;
}

//ufw嵌套ufw格式处理
void mutil_cpu_set_offset(u32 offset)
{
    mutil_ufw_offset = offset;
    lc_flash_file_offset = mutil_ufw_offset; //预先设置好偏移
}

u16 user_lc_flash_f_read(void *fp, u8 *buff, u16 len)
{
    if (lc_flash_init == 0) {
        return (u16) - 1;
    }

    norflash_read(buff, user_lc_flash_update_get_base_addr() + lc_flash_file_offset, len);

    return len;
}


int user_lc_flash_f_open(void)
{
    lc_flash_init = 1;
    return 0;
}

void user_lc_flash_close(void)
{
    if (lc_flash_init) {
        lc_flash_init = 0;
    }
}
//////////////////////lc_flash update loop/////////////////////////////////////

u32 user_lc_flash_update_get_base_addr(void)
{
    return lc_flash_update_info.src_file_addr;
}

u32 user_lc_flash_update_get_src_file_len(void)
{
    return lc_flash_update_info.src_file_len;
}


void user_lc_flash_parm_set(void *priv)
{
    UPDATA_PARM *p  = (UPDATA_PARM *)priv;
    memcpy((u8 *)(&lc_flash_update_info), p->parm_priv, sizeof(lc_flash_update_info_parm));
    put_buf(&lc_flash_update_info, sizeof(lc_flash_update_info_parm));
    printf("%s, src_file_addr = 0x%x\n", __func__, lc_flash_update_info.src_file_addr);
}

void user_lc_flash_update_state_cbk(u32 status, void *priv)
{
    switch (status) {
    case UPDATE_PARM:       //升级需要的参数
        log_info("LC_FLASH_UPDATE_SPARM_SET...\n");
        user_lc_flash_parm_set(priv);
        break;
    case UPDATE_START:      //测试盒edr升级需要根据priv判断是否恢复基带
        log_info("LC_FLASH_UPDATE_START...\n");
        /* noflash_init(); */
        break;
    case UPDATE_END:		//升级结束需要保存结果到Ram给SDK获取，并回复主机升级结果
        if (*((u8 *)priv) == UPDATE_ERR_NONE) {
            set_updata_result(USER_LC_FLASH_UFW_UPDATA, UPDATA_SUCCESSFULLY);
        } else {
            set_updata_result(USER_LC_FLASH_UFW_UPDATA, UPDATA_DEV_ERR);
        }
        user_lc_flash_close();
        /* cpu_reset(); */
        update_reset();
        break;
    }
}

update_op_api_t lc_flash_op_api = {
    .f_open = user_lc_flash_f_open,
    .f_read = user_lc_flash_f_read,
    .f_seek = user_lc_flash_f_seek,
    .notify_update_content_size = NULL,
};

update_mode_info_t update_mode_info = {
    .type      = USER_LC_FLASH_UFW_UPDATA,
    .state_cbk = user_lc_flash_update_state_cbk,
    .file_op   = &lc_flash_op_api,
};
