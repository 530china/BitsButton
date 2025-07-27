/* test_framework.c - 测试框架核心实现 */
#include "test_framework.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

// ==================== 全局测试状态 ====================
test_framework_state_t g_test_framework = {0};

// ==================== 框架初始化函数 ====================

bool test_framework_init(void) {
    memset(&g_test_framework, 0, sizeof(test_framework_state_t));
    g_test_framework.framework_initialized = true;
    printf("测试框架初始化完成\n");
    return true;
}

void test_framework_cleanup(void) {
    memset(&g_test_framework, 0, sizeof(test_framework_state_t));
    printf("测试框架清理完成\n");
}

void test_framework_reset(void) {
    g_test_framework.event_count = 0;
    g_test_framework.simulated_time = 0;
    memset(g_test_framework.gpio_states, 0, sizeof(g_test_framework.gpio_states));
    memset(g_test_framework.events, 0, sizeof(g_test_framework.events));
    printf("测试状态重置完成\n");
}

// ==================== 事件管理函数 ====================

void test_framework_event_callback(struct button_obj_t *btn, bits_btn_result_t result) {
    (void)btn;
    if (g_test_framework.event_count < MAX_TEST_EVENTS) {
        g_test_framework.events[g_test_framework.event_count] = result;
        g_test_framework.event_count++;
    }
}

int test_framework_get_event_count(void) {
    return g_test_framework.event_count;
}

bits_btn_result_t* test_framework_get_events(void) {
    return g_test_framework.events;
}

void test_framework_clear_events(void) {
    g_test_framework.event_count = 0;
    memset(g_test_framework.events, 0, sizeof(g_test_framework.events));
}

// ==================== 模拟函数 ====================

uint8_t test_framework_mock_read_button(struct button_obj_t *btn) {
    if (btn->key_id < MAX_TEST_BUTTONS) {
        return g_test_framework.gpio_states[btn->key_id];
    }
    return 0;
}

int test_framework_log_printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    int result = vprintf(format, args);
    va_end(args);
    return result;
}