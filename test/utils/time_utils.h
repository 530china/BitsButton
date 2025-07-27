/* time_utils.h - 时间模拟工具 */
#ifndef TIME_UTILS_H
#define TIME_UTILS_H

#include <stdint.h>

// ==================== 时间常量定义 ====================
#define DEBOUNCE_DELAY_MS           50
#define STANDARD_CLICK_TIME_MS      100
#define LONG_PRESS_THRESHOLD_MS     1000
#define TIME_WINDOW_DEFAULT_MS      500

// ==================== 时间模拟函数 ====================

/**
 * @brief 模拟时间流逝
 * @param ms 毫秒数
 */
void time_simulate_pass(uint32_t ms);

/**
 * @brief 获取当前模拟时间
 * @return 当前时间（毫秒）
 */
uint32_t time_get_current(void);

/**
 * @brief 重置模拟时间
 */
void time_reset(void);

/**
 * @brief 模拟消抖延时
 */
void time_simulate_debounce_delay(void);

/**
 * @brief 模拟标准点击时间
 */
void time_simulate_standard_click(void);

/**
 * @brief 模拟长按阈值时间
 */
void time_simulate_long_press_threshold(void);

/**
 * @brief 模拟时间窗口结束
 */
void time_simulate_time_window_end(void);

/**
 * @brief 等待指定的ticks数量
 * @param ticks tick数量
 */
void time_simulate_ticks(uint32_t ticks);

/**
 * @brief 将毫秒转换为ticks
 * @param ms 毫秒数
 * @return tick数量
 */
uint32_t time_ms_to_ticks(uint32_t ms);

/**
 * @brief 将ticks转换为毫秒
 * @param ticks tick数量
 * @return 毫秒数
 */
uint32_t time_ticks_to_ms(uint32_t ticks);

#endif /* TIME_UTILS_H */