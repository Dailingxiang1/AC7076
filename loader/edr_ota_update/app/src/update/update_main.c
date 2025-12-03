#ifdef SUPPORT_MS_EXTENSIONS_APP
#pragma bss_seg(".update_main.data.bss")
#pragma data_seg(".update_main.data")
#pragma const_seg(".update_main.text.const")
#pragma code_seg(".update_main.text")
#endif
#include "common.h"
#include "crc.h"
#include "update_main.h"
#include "lib_include.h"
#include "power/p33.h"
#include "dec.h"
#include "jlfs.h"

#include "gpio.h"
#include "power/power_reset.h"


#define LOG_TAG_CONST       UPDATE_MAIN
#define LOG_TAG             "[UPDATE_MAIN]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_DUMP_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"

extern update_mode_info_t update_mode_info;
extern void update_get_record_form_flash(u8 *ram);
extern void update_get_record_form_flash_v2(u8 **p);
extern u8 *update_param_ext_get(UPDATA_PARM *p, u8 ext_type);
extern int update_loop(update_op_api_t *update_op);
extern void user_api_special_handle_info_set(UPDATA_PARM *p);

void ram_protect_close(void);
void bredr_bd_close();
void ll_hci_destory(void);
void update_enter_jump_maskrom(void);
void latch_reset(void);
static u8 latch_flag = 0;
void jump_mode_check(u8 *p);
extern void latch_unlock(void);
extern void reserved_area_update_file_disable(void);

#define __this  (&update_mode_info)

// 从扩展升级参数中取出信息，判断是否不升级升级预留区域
static u8 reserved_update_check(u8 *p)
{
    u8 *reserved_zone_update_flag = update_param_ext_get((UPDATA_PARM *)p, EXT_RESERVED_UPDATE);
    if (reserved_zone_update_flag && reserved_zone_update_flag[0]) {
        reserved_area_update_file_disable();
    }
    return 0;
}

u8 new_sdk_update_again_flag_get(u8 *p)
{
    u8 *update_again_flag = update_param_ext_get((UPDATA_PARM *)p, EXT_NEW_SDK_UPD_AGAIN);
    if (update_again_flag && update_again_flag[0]) {
        return update_again_flag[0];
    }
    return 0;
}

//检查ram里的update信息
bool update_check(void)
{
    u16 crc_cal;
    //check_updata_parm is valid
    UPDATA_PARM *p  = (UPDATA_PARM *)UPDATE_FLAG_ADDR; //定位到ram1最后地址
    log_info("update param:");
    log_info_hexdump((u8 *)p, sizeof(UPDATA_PARM));
    log_info("update param priv:");
    log_info_hexdump(p->parm_priv, 32);
    crc_cal = chip_crc16(((u8 *)p) + 2, sizeof(UPDATA_PARM) - 2);
    if (crc_cal && crc_cal == p->parm_crc) {
        if ((NON_DEV_UPDATA != p->parm_type) && (UPDATA_READY == p->parm_result)) {
            log_info("update_check ture\n");
            return TRUE;
        }
    }
    return FALSE;
}

#define SDK_JUMP_FLAG                  "SDKJUMP"
#define OTA_JUMP_FLAG                  "OTAJUMP"
UPDATE_MODE uart_update_judge_startup_mode(void)
{
#if defined(UART_USE_SEC_MODE) && UART_USE_SEC_MODE
// 由一级loader跳转
    if (strcmp((const char *)UART_UPDATE_FLAG_ADDR, OTA_JUMP_FLAG) == 0)
#else
// 由SDK跳转
    if (strcmp((const char *)UART_UPDATE_FLAG_ADDR, SDK_JUMP_FLAG) == 0)
#endif
    {
        memset((u8 *)UART_UPDATE_FLAG_ADDR, 0, 4);
        return UPDATE_JUMP;
    }

    return UPDATE_POWERON;  // 复位或者断电重启
}

//写升级标志到ram to SDK, 通知SDK升级结果
void set_updata_result(u16 type, u16 result)
{
    UPDATA_PARM *p;

    log_info("set update result :0x%x 0x%x\n", type, result);
    p = (UPDATA_PARM *)UPDATE_FLAG_ADDR;
    memset(p, 0x0, sizeof(UPDATA_PARM));
    p->parm_result = result;
    p->magic = type;
    p->parm_crc = chip_crc16(((u8 *)p) + 2, sizeof(UPDATA_PARM) - 2);
}

#define LATCH_IO_NUM  10
/* static u32 latch_io[LATCH_IO_NUM] = {0}; */
static u8 val[LATCH_IO_NUM * 7 + 1] = {0};
//格式为PB01&0_PB02&1  &0\1表示输出低或者高

#if UPDATE_COMPATIBILITY_EN
static void mutual_get_update_latch_io(u8 *pos)
{
    if (!pos) {
        return;
    }
    for (int i = 0; i < 4; i++) {
        u32 keep_io = get_gpio(&pos[i * 5]);
        printf(" latch_io[%d] = %d, %s\n", i, keep_io, &pos[i * 5]);
        if (keep_io < IO_PORT_MAX) {
            gpio_set_direction(keep_io, 0);
            gpio_set_output_value(keep_io, 1);
        }
    }
}
#endif

static u32 update_set_latch_io(void)
{
    u8 *ptr = jlfs_get_isd_cfg_ptr();
#if UPDATE_COMPATIBILITY_EN
    if (dec_isd_cfg_ini("SD_LATCH_IO", (void *)val, ptr)) {
        u8 *pos = &val[0];
        mutual_get_update_latch_io(pos);
        return 0;
    }
#endif
    if (dec_isd_cfg_ini("LATCH_IO", (void *)val, ptr)) {
        put_buf(val, sizeof(val));
        int len = strlen((const char *)val);

        int i = 0;
        u8 *val_s = &val[0];
        char *pos = (char *)&val[0];
        u8 io_arg[10];
        while (1) {
            if (len < i * 7) {
                break;
            }
            //获取‘&’的位置,获取高低电平设置
            char *level = strchr((const char *)pos, '&');
#if UPDATE_COMPATIBILITY_EN
            if (level == NULL) {
                mutual_get_update_latch_io(pos);
                break;
            }
#endif
            u8 value = (u8)(level[1] - '0');

            //获取IO的字符串，传给gpio();
            u32 io_len = (u32)(level - pos);
            memset(io_arg, 0, sizeof(io_arg));
            memcpy(io_arg, pos, io_len);
            u32 keep_io = get_gpio((const char *)io_arg);

            //设置IO状态
            printf(" latch_io[%d] = %d, %s,  value = %d\n", i, keep_io, io_arg, value);
#ifdef IO_PORT_USB_MASK
            if (keep_io == IO_PORT_DP || keep_io == IO_PORT_DM) {
                gpio_set_direction(keep_io, 1);
                gpio_set_pull_up(keep_io, value);
                gpio_set_pull_down(keep_io, !value);
            } else
#endif
            {
                gpio_set_direction(keep_io, 0);
                gpio_set_output_value(keep_io, value);
            }

            //基指针偏移
            pos = strchr((const char *)pos, '_');
            if (pos == NULL) {
                break;
            }
            pos ++;
            i++;
        }
        return i;
    } else {
        return -1;
    }
}


static void update_set_latch()
{
    if (update_set_latch_io() != -1) {
        latch_unlock();
    }
}

#if defined(CONFIG_CPU_BD49)
struct boot_soft_flag_t boot_flag = {0};
#endif

void update_main()
{
    UPDATA_PARM *p  = (UPDATA_PARM *)UPDATE_FLAG_ADDR;
    UPDATE_MODE mode = UPDATE_JUMP;
    UPDATA_RESULT result = UPDATA_RESULT_FAIL;

#if !UART_UPDATE_ONLY_TEST_MODE
    //update_check
    if (update_check() == FALSE) {			                   //RAM中的结构体不存在， 判断为断电重新上电
        log_info("Ota Start PowerOn\n");
        mode = UPDATE_POWERON;
    } else {
#if (UART_UPDATA_MODULE_CONTROL || UART_UPDATA_USER_MODULE_CONTROL)
        mode = uart_update_judge_startup_mode();
#endif
    }
    log_info("mode : %d\n", mode);

#if UPDIFF_FLASH_UPDATE_SUPPORT_EN
    update_get_record_form_flash_v2(&p);       //从flash里把参数读出来，遍历整个flash
#else

    update_get_record_form_flash((u8 *)&p);       //从flash里把参数读出来
#endif

#if 0
    /* TODO */
    u8 *data = update_param_ext_get(p, EXT_RF_PA_INFO);
    put_buf(data, 3);
#endif

#endif

    jump_mode_check((u8 *)p);

    if (latch_flag) {
#if defined(CONFIG_CPU_BD49)
        memset(&boot_flag, 0, sizeof(struct boot_soft_flag_t));
        u8 *romio_info = update_param_ext_get((UPDATA_PARM *)p, EXT_KEEP_ROMIO_INFO);
        if (romio_info) {
            memcpy(&boot_flag, romio_info, sizeof(struct boot_soft_flag_t));
        }
#endif
    }

    reserved_update_check((u8 *)p);

    user_api_special_handle_info_set(p);

#if defined(CONFIG_CPU_BR35)
    u16 type = p->parm_type;
    u8 update_again_flag = new_sdk_update_again_flag_get(p);
#endif

#if (defined(CONFIG_CPU_BR35) && (USER_LC_FLASH_UPDATA_MODULE_CONTROL || EX_FLASH_UPDATE_SUPPORT_EN))
    extern void ota_get_sys_clk(u8 * param);
    u8 *tmp_buf = update_param_ext_get((UPDATA_PARM *)p, EXT_SYS_CLK_PARAM);
    if (tmp_buf) {
        ota_get_sys_clk(tmp_buf);
    }
#endif

    update_set_latch(); //升级前解除latch,处理SD卡和外置flash这种需要动IO的升级方式

    /* mode = UPDATE_POWERON; */
    //给各升级模块传递参数
    log_info(">>>[test]:UPDATE_STEP: SET UPDATE_PARM\n");
    __this->state_cbk(UPDATE_PARM, p);

    log_info(">>>[test]:UPDATE_STEP: UPDATE_START, mode = 0x%x\n", mode);
    __this->state_cbk(UPDATE_START, &mode);

#if !UART_UPDATE_ONLY_TEST_MODE
    //loop
    log_info(">>>[test]:UPDATE_STEP: update_loop\n");
    result = update_loop((update_op_api_t *)__this->file_op);

    log_info(">>>[test]:UPDATE_STEP: UPDATE_END,  result = 0x%x\n", result);
    //end
#if (defined(CONFIG_CPU_BR35) && (defined(EDR_UPDATA_SUPPORT_CONNECT) || defined(BLE_UPDATA_SUPPORT_CONNECT)))
    if (update_again_flag) {
        set_updata_result(type, UPDATA_DEV_ERR);
        update_reset();
    }
#endif
    __this->state_cbk(UPDATE_END,  &result);                    //不同升级完成之后可能需要执行不同操作
#endif
}

__attribute__((weak))
u8 *get_isd_cfg_ptr(void)
{
    return jlfs_get_isd_cfg_ptr();
}

u32 get_update_jump_flag()
{
    u8 *ptr = jlfs_get_isd_cfg_ptr();
    u32 u32Val;
    u8 val[16] = {0};
    printf("\n >>>[test]:func = %s,line= %d\n", __FUNCTION__, __LINE__);
    if (dec_isd_cfg_ini("UPDATE_JUMP", (void *)val, ptr)) {
        put_buf(val, 16);
        /* u32Val = (u32) (((u8 *)val)[0] - '0'); */
        u32Val = (u32)val[0];
        printf("update_jump = %d, vol[0] = %d\n", u32Val, val[0]);
        return u32Val;
    } else {
        return 0;
    }
}

void jump_mode_check(u8 *p)
{
    u8 *p_latch_flag = NULL;
    if (latch_flag == 0) {
        p_latch_flag = update_param_ext_get((UPDATA_PARM *)p, EXT_JUMP_FLAG);
        if (p_latch_flag) {
            latch_flag = p_latch_flag[0];
        }
    }
    printf(">>>[test]:latch_flag = %d\n", latch_flag);
}

u32 update_get_lrc_hz(void)
{
    u32 lrc_hz = 0;
    UPDATA_PARM *p  = (UPDATA_PARM *)UPDATE_FLAG_ADDR;
    memcpy(&lrc_hz, &p->parm_priv[sizeof(p->parm_priv) - sizeof(lrc_hz)], sizeof(lrc_hz));
    return lrc_hz;
}

__attribute__((weak))
void system_reset(enum RESET_FLAG flag)
{
    cpu_reset();
}

void update_reset(void)
{
    printf("\n >>>[test]:func = %s,line= %d\n", __FUNCTION__, __LINE__);
    /* 寄存器不会复位 */
#if (0 == UART_UPDATE_ONLY_TEST_MODE)
    /* u8 *p = NULL; */
    /* UPDATA_PARM *p  = (UPDATA_PARM *)UPDATE_FLAG_ADDR; */
    /* update_get_record_form_flash((u8 *)&p);       //从flash里把参数读出来 */
    /* jump_mode_check((u8 *)p); */
    if (latch_flag) {
        puts("latch reset...\n");
        void latch_reset(void);
        latch_reset();
    } else
#endif
    {
        printf(">>>[test]:cpu_reset\n");
        system_reset(UPDATE_FLAG);
    }
}

#if defined(CONFIG_CPU_BD49)
struct boot_soft_flag_t *latch_info_get(void)
{
    int k = 0;
    int len = sizeof(struct boot_soft_flag_t);
    u8 *romio_info = NULL;
    u8 *p = &boot_flag;
    for (int i = 0; i < len; i++) {
        if (p[i] != 0) {
            k = 1;
            break;
        }
    }
    if (k) {
        romio_info = &boot_flag;
    }
    return (struct boot_soft_flag_t *)romio_info;
}
#endif
