/* test_peek_functionality.c - 测试peek功能 */
#include "unity.h"
#include "core/test_framework.h"
#include "utils/mock_utils.h"
#include "utils/time_utils.h"
#include "utils/assert_utils.h"
#include "config/test_config.h"
#include "bits_button.h"
#include <stdio.h>

// ==================== 测试设置和清理 ====================

void peek_setUp(void) {
    test_framework_reset();
    mock_reset_all_buttons();
    time_reset();
}

void peek_tearDown(void) {
    // 测试清理
}

// ==================== 测试用例：Peek功能验证 ====================

void test_peek_functionality(void) {
    printf("\n=== 测试Peek功能 ===\n");
    
    // 创建按键对象
    static const bits_btn_obj_param_t param = TEST_DEFAULT_PARAM();
    button_obj_t button = CREATE_TEST_BUTTON(1, 1, &param);
    
    // 初始化按键系统
    bits_btn_config_t config = {
        .btns = &button,
        .btns_cnt = 1,
        .read_button_level_func = test_framework_mock_read_button,
        .bits_btn_result_cb = test_framework_event_callback,
        .bits_btn_debug_printf = test_framework_log_printf
    };
    int32_t ret = bits_button_init(&config);
    TEST_ASSERT_EQUAL_MESSAGE(0, ret, "按键初始化应成功");
    
    // 验证初始状态下peek应该返回false（缓冲区为空）
    bits_btn_result_t result;
    uint8_t peek_result = bits_button_peek_key_result(&result);
    TEST_ASSERT_FALSE_MESSAGE(peek_result, "初始状态下peek应返回false");
    
    // 模拟一个按键点击事件
    mock_button_click(1, STANDARD_CLICK_TIME_MS);
    
    // 等待时间窗口结束以确保事件被写入缓冲区
    time_simulate_time_window_end();
    
    // 此时应该有事件在缓冲区中，peek应该成功
    peek_result = bits_button_peek_key_result(&result);
    TEST_ASSERT_TRUE_MESSAGE(peek_result, "有事件时peek应返回true");
    
    // 验证peek获取到的事件信息
    TEST_ASSERT_EQUAL_MESSAGE(1, result.key_id, "应为按键1的事件");
    TEST_ASSERT_EQUAL_MESSAGE(BTN_STATE_FINISH, result.event, "应为完成事件");
    TEST_ASSERT_EQUAL_MESSAGE(BITS_BTN_SINGLE_CLICK_KV, result.key_value, "应为单击按键值");
    
    // 再次peek，应该仍然能获取到相同的事件（因为peek不会移除事件）
    bits_btn_result_t result2;
    uint8_t peek_result2 = bits_button_peek_key_result(&result2);
    TEST_ASSERT_TRUE_MESSAGE(peek_result2, "第二次peek应仍返回true");
    TEST_ASSERT_EQUAL_MESSAGE(result.key_id, result2.key_id, "两次peek的按键ID应相同");
    TEST_ASSERT_EQUAL_MESSAGE(result.event, result2.event, "两次peek的事件类型应相同");
    TEST_ASSERT_EQUAL_MESSAGE(result.key_value, result2.key_value, "两次peek的按键值应相同");
    
    // 使用正常的get函数读取事件，应该能获取到相同的事件
    bits_btn_result_t result3;
    uint8_t get_result = bits_button_get_key_result(&result3);
    TEST_ASSERT_TRUE_MESSAGE(get_result, "get应能获取到事件");
    TEST_ASSERT_EQUAL_MESSAGE(result.key_id, result3.key_id, "get和peek的按键ID应相同");
    TEST_ASSERT_EQUAL_MESSAGE(result.event, result3.event, "get和peek的事件类型应相同");
    TEST_ASSERT_EQUAL_MESSAGE(result.key_value, result3.key_value, "get和peek的按键值应相同");
    
    // 再次peek，现在应该返回false（因为事件已被get函数移除）
    bits_btn_result_t result4;
    uint8_t peek_result4 = bits_button_peek_key_result(&result4);
    TEST_ASSERT_FALSE_MESSAGE(peek_result4, "事件被移除后peek应返回false");
    
    printf("Peek功能测试通过\n");
}

// ==================== 测试用例：Peek与Get对比 ====================

void test_peek_vs_get_behavior(void) {
    printf("\n=== 测试Peek与Get行为对比 ===\n");
    
    // 创建按键对象
    static const bits_btn_obj_param_t param = TEST_DEFAULT_PARAM();
    button_obj_t button = CREATE_TEST_BUTTON(2, 1, &param);
    
    // 初始化按键系统
    bits_btn_config_t config = {
        .btns = &button,
        .btns_cnt = 1,
        .read_button_level_func = test_framework_mock_read_button,
        .bits_btn_result_cb = test_framework_event_callback,
        .bits_btn_debug_printf = test_framework_log_printf
    };
    int32_t ret = bits_button_init(&config);
    TEST_ASSERT_EQUAL_MESSAGE(0, ret, "按键初始化应成功");
    
    // 模拟一个按键事件
    mock_button_click(2, STANDARD_CLICK_TIME_MS);
    time_simulate_time_window_end();
    
    // 使用peek查看事件但不移除
    bits_btn_result_t peek_result;
    TEST_ASSERT_TRUE_MESSAGE(bits_button_peek_key_result(&peek_result), "peek应成功");
    TEST_ASSERT_EQUAL_MESSAGE(2, peek_result.key_id, "应为按键2的事件");
    
    // 再次使用peek，应该仍能获取到事件
    bits_btn_result_t peek_result2;
    TEST_ASSERT_TRUE_MESSAGE(bits_button_peek_key_result(&peek_result2), "第二次peek应成功");
    TEST_ASSERT_EQUAL_MESSAGE(peek_result.key_id, peek_result2.key_id, "两次peek应获取相同事件");
    
    // 使用get获取事件，应该获取到相同的事件
    bits_btn_result_t get_result;
    TEST_ASSERT_TRUE_MESSAGE(bits_button_get_key_result(&get_result), "get应成功");
    TEST_ASSERT_EQUAL_MESSAGE(peek_result.key_id, get_result.key_id, "peek和get应获取相同事件ID");
    
    // 再次peek，应该返回false（事件已被get移除）
    bits_btn_result_t final_peek;
    TEST_ASSERT_FALSE_MESSAGE(bits_button_peek_key_result(&final_peek), "事件被移除后peek应失败");
    
    printf("Peek与Get行为对比测试通过\n");
}

// ==================== 测试用例：禁用缓冲区模式下的Peek ====================

void test_peek_disabled_buffer_mode(void) {
    printf("\n=== 测试禁用缓冲区模式下的Peek ===\n");
    
#ifdef BITS_BTN_DISABLE_BUFFER
    // 在禁用缓冲区模式下，peek应该始终返回false
    bits_btn_result_t result;
    uint8_t peek_result = bits_button_peek_key_result(&result);
    TEST_ASSERT_FALSE_MESSAGE(peek_result, "禁用缓冲区模式下peek应始终返回false");
    
    printf("禁用缓冲区模式下Peek测试通过\n");
#else
    printf("跳过：当前未启用BITS_BTN_DISABLE_BUFFER模式\n");
#endif
}