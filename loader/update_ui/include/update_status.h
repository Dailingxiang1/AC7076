#ifndef __UPDATE_STATUS_H__
#define __UPDATE_STATUS_H__

enum {
	UPDATE_ERR_NONE = 0,
	UPDATE_ERR_UFW_HEAD_CRC_ERR,
	UPDATE_ERR_NOT_FIND_LOADER_FILE,
	UPDATE_ERR_NOT_FIND_TARGET_LOADER,

	UPDATE_ERR_LOADER_HEAD_CRC_ERR = 4,
	UPDATE_ERR_MALLOC_ERR,
	UPDATE_ERR_LOADER_WRITE_ERR,
	UPDATE_ERR_FILE_HANDLE_ERR,

	UPDATE_ERR_LOADER_VERIFY_ERR = 8,
	UPDATE_ERR_NOT_FIND_FLASH_BIN,
	UPDATE_ERR_FLASH_HEAD_CRC_ERR,
	UPDATE_ERR_NOT_FIND_TARGET_FILE,

	UPDATE_ERR_KEY_ERR = 12,
	UPDATE_ERR_UBOOT_NOT_MATCH,
	UPDATE_ERR_READ_REMOTE_FILE_ERR,
	UPDATE_ERR_REMOTE_RES_FILE_CRC_ERR,

	UPDATE_ERR_CODE_VERIFY_ERR = 16,
	UPDATE_ERR_RES0_VERIFY_ERR,
	UPDATE_ERR_NOT_FIND_RESERVE_DIR_FILE,
	UPDATE_ERR_LOCAL_FILE_HEAD_CRC_ERR,

	UPDATE_ERR_LOCAL_FILE_DATA_CRC_ERR = 20,
	UPDATE_ERR_PARAM_ERR,
	UPDATE_ERR_BT_CFG_UPDATE_ERR,
	UPDATE_ERR_TONE_UPDATE_ERR,

	UPDATE_ERR_RESERVED_CONFIG_UPDATE_ERR = 24,
	UPDATE_ERR_PRODUCT_ID_NOT_MATCH,
	UPDATE_ERR_CONN_ERR,

};

typedef enum {
	UPDATE_START = 0,                       //Éý¼¶¿ªÊ¼  param 0:Éý¼¶¿ªÊ¼ 1:½øÈëÉý¼¶ 2:½øÈëÇ¿ÖÆÉý¼¶
	UPDATE_STOP,                            //Éý¼¶½áÊø  param update error code
	UPDATE_PROCESS,                         //Éý¼¶ÖÐ    param Éý¼¶½ø¶Èpercent = param/100
	EX_API_UPDATE_TIPS_WAIT_CONN,           //µÈ´ýÀ¶ÑÀÁ¬½Ó param struct _user_api_param_t
    EX_API_UPDATE_TIPS_WAIT_UPDATE,         //µÈ´ýÉý¼¶  param struct _user_api_param_t
    EX_API_UPDATE_TIPS_WAIT_START_UPDATE,   //µÈ´ýAPPÊÖ¶¯´¥·¢Éý¼¶ param struct _user_api_param_t
    EX_API_UPDATE_TIPS_UPDATEING,           //¹ã²¥Éý¼¶Ê¹ÓÃ
    EX_API_UPDATE_CUSTOM_INFO = 0xff,       //ÓÃÓÚ¿Í»§×Ô¶¨Òå²Ù×÷£¬ÀýÈçÍùuser_api.bin´«Èë²ÎÊý
} UPDATA_STATUS;


#endif
