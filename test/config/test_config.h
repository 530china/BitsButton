/* test_config.h - 测试配置文件 */
#ifndef TEST_CONFIG_H
#define TEST_CONFIG_H

#include "bits_button.h"

// ==================== 测试框架配置 ====================
#define MAX_CAPTURED_EVENTS         100
#define MAX_TEST_BUTTONS            16
#define MAX_TEST_SUITES             10
#define MAX_TESTS_PER_SUITE         20

// ==================== 测试按键ID定义 ====================
#define TEST_BUTTON_1               1
#define TEST_BUTTON_2               2
#define TEST_BUTTON_3               3
#define TEST_BUTTON_4               4

// ==================== 测试组合键ID定义 ====================
#define TEST_COMBO_BUTTON_1         100
#define TEST_COMBO_BUTTON_2         200
#define TEST_COMBO_BUTTON_3         300

// ==================== 测试时间常量 ====================
#define TEST_DEBOUNCE_TIME_MS       BITS_BTN_DEBOUNCE_TIME_MS
#define TEST_LONG_PRESS_TIME_MS     BITS_BTN_LONG_PRESS_START_TIME_MS
#define TEST_TIME_WINDOW_MS         BITS_BTN_TIME_WINDOW_TIME_MS
#define TEST_STANDARD_CLICK_MS      100
#define TEST_QUICK_CLICK_MS         50
#define TEST_LONG_CLICK_MS          200

// ==================== 测试参数宏 ====================
#define TEST_DEFAULT_PARAM() { \
    .long_press_period_triger_ms = BITS_BTN_LONG_PRESS_PERIOD_TRIGER_MS, \
    .long_press_start_time_ms = BITS_BTN_LONG_PRESS_START_TIME_MS, \
    .short_press_time_ms = BITS_BTN_SHORT_TIME_MS, \
    .time_window_time_ms = BITS_BTN_TIME_WINDOW_TIME_MS \
}

#define TEST_FAST_LONG_PRESS_PARAM() { \
    .long_press_period_triger_ms = 500, \
    .long_press_start_time_ms = BITS_BTN_LONG_PRESS_START_TIME_MS, \
    .short_press_time_ms = BITS_BTN_SHORT_TIME_MS, \
    .time_window_time_ms = BITS_BTN_TIME_WINDOW_TIME_MS \
}

// ==================== 测试按键创建宏 ====================
#define CREATE_TEST_BUTTON(id, level, param) \
    BITS_BUTTON_INIT(id, level, param)

#define CREATE_TEST_COMBO_BUTTON(id, level, param, keys, count, suppress) \
    BITS_BUTTON_COMBO_INIT(id, level, param, keys, count, suppress)

// ==================== 测试验证宏 ====================
#define VERIFY_SINGLE_CLICK(key_id) \
    ASSERT_EVENT_WITH_VALUE(key_id, BTN_EVENT_FINISH, BITS_BTN_SINGLE_CLICK_KV)

#define VERIFY_DOUBLE_CLICK(key_id) \
    ASSERT_EVENT_WITH_VALUE(key_id, BTN_EVENT_FINISH, BITS_BTN_DOUBLE_CLICK_KV)

#define VERIFY_TRIPLE_CLICK(key_id) \
    ASSERT_EVENT_WITH_VALUE(key_id, BTN_EVENT_FINISH, 0b101010)  // 三连击: 010 + 010 + 10

#define VERIFY_LONG_PRESS_START(key_id) \
    ASSERT_EVENT_WITH_VALUE(key_id, BTN_EVENT_LONG_PRESS, BITS_BTN_LONG_PRESEE_START_KV)

#define VERIFY_SINGLE_THEN_LONG_PRESS(key_id) \
    ASSERT_EVENT_WITH_VALUE(key_id, BTN_EVENT_LONG_PRESS, BITS_BTN_SINGLE_CLICK_THEN_LONG_PRESS_KV)

// ==================== 调试配置 ====================
#define TEST_ENABLE_DEBUG_PRINT     1
#define TEST_ENABLE_EVENT_LOG       1
#define TEST_ENABLE_BINARY_DISPLAY  1

// ==================== 测试模式配置 ====================
#define TEST_MODE_VERBOSE           1
#define TEST_MODE_QUIET             0

#endif /* TEST_CONFIG_H */