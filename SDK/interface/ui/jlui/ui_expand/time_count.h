/* ------------------------------------------------------------------------------------*/
/**
 * @file time_count.h
 *
 * @brief 时间探针，用于粗略记录程序执行时间
 *
 * 使用方法：
 * 1. 在程序开始执行的位置调用 time_count_start；
 * 2. 在程序结束执行的位置调用 time_count_end；
 * 3. 保证 start 和 end 时传入的 id 一致、模块会自动在程序结束位置打印出执行时间，单位 us；
 * 4. 若打印出 Error 信息，说明 start 和 end 时传入的 id 不一致；
 * 5. 此方法仅用于粗略统计程序执行时间，非精确时间。可用于性能调试参考。
 * 注意：
 * 本模块接口仅应用层可以调用，库里调用会导致宏开关无法关闭，造成资源浪费。
 * 每start一个时间，会malloc 20byte空间占用，end时会释放
 *
 * @author
 *
 * @version V1.0.0
 *
 * @date 2025-03-18
 *
 * Copyright(C) 2010- , JIELI TECHNOLOGY, Inc.
 * All right reserved.
 */
/* ------------------------------------------------------------------------------------*/

#ifndef __TIME_COUNT_H__
#define __TIME_COUNT_H__

// 程序计数功能使能
#define TIME_COUNT_ENABLE	0


#if (defined TIME_COUNT_ENABLE && TIME_COUNT_ENABLE)

#define time_count_start(id) \
	time_count_pushing(id)

#define time_count_end(id) \
{ \
	printf("%s(), %d, time count id: %d, time: %llu us\n", __func__, __LINE__, id, time_count_popping(id)); \
}

#else

#define time_count_start(id)

#define time_count_end(id)

#endif



/* ------------------------------------------------------------------------------------*/
/**
 * @brief time_count_pushing 将当前时间压入计时栈中
 *
 * @Params id 为当前时间命名的id，用于计时出栈时计算运行时间
 *
 * @return 当前时间
 */
/* ------------------------------------------------------------------------------------*/
u64 time_count_pushing(u32 id);


/* ------------------------------------------------------------------------------------*/
/**
 * @brief time_count_popping 统计指定ID的程序段运行时间，并将该时间出栈
 *
 * @Params id 需要统计时间的ID号，与计时入栈时传入的ID一致
 *
 * @return 指定ID的程序段运行时间，如果指定ID没有入栈，则返回当前时间，并打印错误提示
 */
/* ------------------------------------------------------------------------------------*/
u64 time_count_popping(u32 id);


#endif // !__TIME_COUNT_H__


