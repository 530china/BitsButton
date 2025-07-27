/* assert_utils.h - 断言增强工具 */
#ifndef ASSERT_UTILS_H
#define ASSERT_UTILS_H

#include "unity.h"
#include "bits_button.h"
#include <stdint.h>
#include <stdbool.h>

// ==================== 事件查找和验证函数 ====================

/**
 * @brief 查找特定事件
 * @param key_id 按键ID
 * @param event_type 事件类型
 * @return 找到的事件指针，未找到返回NULL
 */
bits_btn_result_t* assert_find_event(uint16_t key_id, uint8_t event_type);

/**
 * @brief 查找最后一个特定类型的事件
 * @param key_id 按键ID
 * @param event_type 事件类型
 * @return 找到的事件指针，未找到返回NULL
 */
bits_btn_result_t* assert_find_last_event(uint16_t key_id, uint8_t event_type);

/**
 * @brief 统计特定类型事件的数量
 * @param key_id 按键ID
 * @param event_type 事件类型
 * @return 事件数量
 */
int assert_count_events(uint16_t key_id, uint8_t event_type);

// ==================== 增强断言函数 ====================

/**
 * @brief 断言事件存在
 * @param key_id 按键ID
 * @param event_type 事件类型
 * @param line 调用行号
 */
void assert_event_exists_impl(uint16_t key_id, uint8_t event_type, int line);

/**
 * @brief 断言事件存在并验证按键值
 * @param key_id 按键ID
 * @param event_type 事件类型
 * @param expected_key_value 期望的按键值
 * @param line 调用行号
 */
void assert_event_with_value_impl(uint16_t key_id, uint8_t event_type, 
                                  uint32_t expected_key_value, int line);

/**
 * @brief 断言长按事件的触发次数
 * @param key_id 按键ID
 * @param expected_count 期望的触发次数
 * @param line 调用行号
 */
void assert_long_press_count_impl(uint16_t key_id, uint16_t expected_count, int line);

/**
 * @brief 断言组合按键的抑制效果
 * @param combo_key_id 组合按键ID
 * @param suppressed_key_ids 被抑制的单键ID数组
 * @param count 被抑制的按键数量
 * @param line 调用行号
 */
void assert_combo_suppression_impl(uint16_t combo_key_id, uint16_t* suppressed_key_ids, 
                                   int count, int line);

/**
 * @brief 断言事件不存在
 * @param key_id 按键ID
 * @param event_type 事件类型
 * @param line 调用行号
 */
void assert_event_not_exists_impl(uint16_t key_id, uint8_t event_type, int line);

/**
 * @brief 断言事件数量
 * @param key_id 按键ID
 * @param event_type 事件类型
 * @param expected_count 期望的事件数量
 * @param line 调用行号
 */
void assert_event_count_impl(uint16_t key_id, uint8_t event_type, 
                            int expected_count, int line);

// ==================== 调试和日志函数 ====================

/**
 * @brief 打印所有捕获的事件
 */
void assert_print_all_events(void);

/**
 * @brief 打印特定按键的事件
 * @param key_id 按键ID
 */
void assert_print_key_events(uint16_t key_id);

/**
 * @brief 打印按键值的二进制表示
 * @param key_value 按键值
 */
void assert_print_key_value_binary(uint32_t key_value);

// ==================== 便利宏定义 ====================

#define ASSERT_EVENT_EXISTS(key_id, event_type) \
    assert_event_exists_impl(key_id, event_type, __LINE__)

#define ASSERT_EVENT_WITH_VALUE(key_id, event_type, expected_value) \
    assert_event_with_value_impl(key_id, event_type, expected_value, __LINE__)

#define ASSERT_LONG_PRESS_COUNT(key_id, expected_count) \
    assert_long_press_count_impl(key_id, expected_count, __LINE__)

#define ASSERT_COMBO_SUPPRESSION(combo_id, suppressed_ids, count) \
    assert_combo_suppression_impl(combo_id, suppressed_ids, count, __LINE__)

#define ASSERT_EVENT_NOT_EXISTS(key_id, event_type) \
    assert_event_not_exists_impl(key_id, event_type, __LINE__)

#define ASSERT_EVENT_COUNT(key_id, event_type, expected_count) \
    assert_event_count_impl(key_id, event_type, expected_count, __LINE__)

#define PRINT_ALL_EVENTS() \
    assert_print_all_events()

#define PRINT_KEY_EVENTS(key_id) \
    assert_print_key_events(key_id)

#define PRINT_KEY_VALUE_BINARY(key_value) \
    assert_print_key_value_binary(key_value)

#endif /* ASSERT_UTILS_H */