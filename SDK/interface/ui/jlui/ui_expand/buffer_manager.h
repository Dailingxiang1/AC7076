#ifndef __BUFFER_MANAGER_H__
#define __BUFFER_MANAGER_H__



// 缓存buf的状态定义
typedef enum buffer_status_def {
    BUFFER_STATUS_INIT,		// 初始
    BUFFER_STATUS_IDLE,		// 空闲
    BUFFER_STATUS_LOCK,		// 锁定
    BUFFER_STATUS_PEND,		// 等待
    BUFFER_STATUS_USED,		// 使用
} BUFFER_STATUS;



// buffer管理原生API，不建议直接调用原生API，
// 防止后续库升级导致调用方法改变，使用时请调用重封装的快捷调用
int buffer_manager_set_default_handler(void *buffer_hdl);

void buffer_manager_set_memory_callback(void *(*malloc)(int size, u32 ram_type, u32 module_type), void (*free)(void *p, u32 ram_type, u32 module_type));

void *buffer_manager_init_handler(u32 buf_list[], int buf_num);

void buffer_manager_free_handler(void *buffer_hdl);

int buffer_manager_set_buf_status(void *buffer_hdl, void *buf, int status);

int buffer_manager_get_buf_status(void *buffer_hdl, void *buf);

void *buffer_manager_get_buf_by_status(void *buffer_hdl, int status);

void *buffer_manager_change_buf_status(void *buffer_hdl, int target, int result);

void *buffer_manager_status_dump(void *buffer_hdl);

/* buffer管理API快捷调用 */
// 注册buffer管理模块的回调
#define buffer_manager_callback(malloc, free) \
	buffer_manager_set_memory_callback(malloc, free)

// 初始化buffer管理，创建buffer管理句柄
#define buffer_manager_init(hdl, buf, size, block_num) \
{ \
	int BM_i = 0; \
	int BM_buf_size = (size) / (block_num); \
	u32 BM_buf_list[3] = {0}; \
	for (BM_i = 0; BM_i < (block_num); BM_i++) { \
		BM_buf_list[BM_i] = (u32)((buf) + (BM_buf_size * BM_i)); \
	} \
	(hdl) = buffer_manager_init_handler(BM_buf_list, (block_num)); \
}

// 释放buffer管理资源和句柄
#define buffer_manager_free(buffer_handler) \
	buffer_manager_free_handler(buffer_handler)

// 设置指定buffer的状态
#define set_buffer_status(buffer_handler, buf_addr, status) \
	buffer_manager_set_buf_status(buffer_handler, buf_addr, status)

// 获取指定buffer的状态
#define get_buffer_status(buffer_handler, buf_addr) \
	buffer_manager_get_buf_status(buffer_handler, buf_addr)

// 获取指定状态的buffer
#define get_buffer_by_status(buffer_handler, status) \
	buffer_manager_get_buf_by_status(buffer_handler, status)

// 获取目标状态的buffer，并修改为结果状态
#define change_buffer_status(buffer_handler, target, result) \
	buffer_manager_change_buf_status(buffer_handler, target, result)


// 设置默认句柄，可以设置一个默认句柄，该句柄将作为buffer管理的静态变量，外部无需每次操作都传入该句柄
// 需要注意的时，buffer_manager_free时，可以传入NULL来释放默认句柄，但大buffer还需外面主动释放
// 即：buffer管理只做状态管理，和自身资源的申请、释放，不管被管理的buffer申请和释放
#define set_default_buffer_handler(buffer_handler) \
	buffer_manager_set_default_handler(buffer_handler)




/* 快捷获取指定状态的buffer */
// 获取空闲状态的buffer
#define get_idle_buffer(buffer_handler) \
	get_buffer_by_status(buffer_handler, BUFFER_STATUS_IDLE)

// 获取锁定状态的buffer
#define get_lock_buffer(buffer_handler) \
	get_buffer_by_status(buffer_handler, BUFFER_STATUS_LOCK)

// 获取等待状态的buffer
#define get_pend_buffer(buffer_handler) \
	get_buffer_by_status(buffer_handler, BUFFER_STATUS_PEND)

// 获取使用状态的buffer
#define get_used_buffer(buffer_handler) \
	get_buffer_by_status(buffer_handler, BUFFER_STATUS_USED)




/* 快捷获取指定状态的buffer，并修改buffer状态 */
// 获取空闲状态的buffer，并修改为锁定状态
#define get_idle_buffer_to_lock(buffer_handler) \
	change_buffer_status(buffer_handler, BUFFER_STATUS_IDLE, BUFFER_STATUS_LOCK)

// 获取锁定状态的buffer，并修改为等待状态
#define get_lock_buffer_to_pend(buffer_handler) \
	change_buffer_status(buffer_handler, BUFFER_STATUS_LOCK, BUFFER_STATUS_PEND)

// 获取等待状态的buffer，并修改为使用状态
#define get_pend_buffer_to_used(buffer_handler) \
	change_buffer_status(buffer_handler, BUFFER_STATUS_PEND, BUFFER_STATUS_USED)

// 获取使用状态的buffer，并修改为空闲状态
#define get_used_buffer_to_idle(buffer_handler) \
	change_buffer_status(buffer_handler, BUFFER_STATUS_USED, BUFFER_STATUS_IDLE)




/* 快捷设置指定buffer的状态 */
// 设置buffer状态为空闲
#define set_buffer_idle(buffer_handler, buf) \
	set_buffer_status(buffer_handler, buf, BUFFER_STATUS_IDLE)

// 设置buffer状态为锁定
#define set_buffer_lock(buffer_handler, buf) \
	set_buffer_status(buffer_handler, buf, BUFFER_STATUS_LOCK)

// 设置buffer状态为等待
#define set_buffer_pend(buffer_handler, buf) \
	set_buffer_status(buffer_handler, buf, BUFFER_STATUS_PEND)

// 设置buffer状态为使用
#define set_buffer_used(buffer_handler, buf) \
	set_buffer_status(buffer_handler, buf, BUFFER_STATUS_USED)




#endif


