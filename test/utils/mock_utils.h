/* mock_utils.h - 模拟工具 */
#ifndef MOCK_UTILS_H
#define MOCK_UTILS_H

#include "bits_button.h"
#include <stdint.h>
#include <stdbool.h>

// ==================== 按键模拟函数 ====================

/**
 * @brief 设置按键GPIO状态
 * @param button_id 按键ID
 * @param state GPIO状态 (0或1)
 */
void mock_set_button_state(uint8_t button_id, uint8_t state);

/**
 * @brief 获取按键GPIO状态
 * @param button_id 按键ID
 * @return GPIO状态
 */
uint8_t mock_get_button_state(uint8_t button_id);

/**
 * @brief 模拟按键按下
 * @param button_id 按键ID
 */
void mock_button_press(uint8_t button_id);

/**
 * @brief 模拟按键释放
 * @param button_id 按键ID
 */
void mock_button_release(uint8_t button_id);

/**
 * @brief 模拟完整的按键点击
 * @param button_id 按键ID
 * @param hold_time_ms 按住时间（毫秒）
 */
void mock_button_click(uint8_t button_id, uint32_t hold_time_ms);

/**
 * @brief 模拟多次连续点击
 * @param button_id 按键ID
 * @param click_count 点击次数
 * @param hold_time_ms 每次按住时间
 * @param interval_ms 点击间隔时间
 */
void mock_multiple_clicks(uint8_t button_id, int click_count, 
                         uint32_t hold_time_ms, uint32_t interval_ms);

/**
 * @brief 模拟按键抖动
 * @param button_id 按键ID
 * @param bounce_count 抖动次数
 * @param bounce_interval_ms 抖动间隔
 */
void mock_button_bounce(uint8_t button_id, int bounce_count, uint32_t bounce_interval_ms);

/**
 * @brief 重置所有按键状态
 */
void mock_reset_all_buttons(void);

#endif /* MOCK_UTILS_H */