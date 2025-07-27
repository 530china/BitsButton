/* test_framework.h - 测试框架核心 */
#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include "unity.h"
#include "bits_button.h"
#include <stdint.h>
#include <stdbool.h>

// ==================== 测试框架配置 ====================
#define MAX_TEST_EVENTS 100
#define MAX_TEST_BUTTONS 16

// ==================== 测试状态管理 ====================
typedef struct {
    bits_btn_result_t events[MAX_TEST_EVENTS];
    int event_count;
    uint32_t simulated_time;
    uint8_t gpio_states[MAX_TEST_BUTTONS];
    bool framework_initialized;
} test_framework_state_t;

// ==================== 全局测试状态 ====================
extern test_framework_state_t g_test_framework;

// ==================== 框架初始化函数 ====================

/**
 * @brief 初始化测试框架
 * @return true 成功，false 失败
 */
bool test_framework_init(void);

/**
 * @brief 清理测试框架
 */
void test_framework_cleanup(void);

/**
 * @brief 重置测试状态
 */
void test_framework_reset(void);

// ==================== 事件管理函数 ====================

/**
 * @brief 事件捕获回调函数
 * @param btn 按键对象
 * @param result 按键结果
 */
void test_framework_event_callback(struct button_obj_t *btn, bits_btn_result_t result);

/**
 * @brief 获取事件数量
 * @return 当前捕获的事件数量
 */
int test_framework_get_event_count(void);

/**
 * @brief 获取事件列表
 * @return 事件数组指针
 */
bits_btn_result_t* test_framework_get_events(void);

/**
 * @brief 清空事件列表
 */
void test_framework_clear_events(void);

// ==================== 模拟函数 ====================

/**
 * @brief 模拟GPIO读取函数
 * @param btn 按键对象
 * @return GPIO状态
 */
uint8_t test_framework_mock_read_button(struct button_obj_t *btn);

/**
 * @brief 测试日志输出函数
 * @param format 格式字符串
 * @param ... 参数
 * @return 输出字符数
 */
int test_framework_log_printf(const char* format, ...);

#endif /* TEST_FRAMEWORK_H */