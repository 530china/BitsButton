/**
 * @file test_state_reset.c
 * @brief 测试按键状态重置功能
 */

#include "unity.h"
#include "core/test_framework.h"
#include "utils/mock_utils.h"
#include "utils/time_utils.h"
#include "utils/assert_utils.h"
#include "config/test_config.h"
#include "bits_button.h"

void test_state_reset_functionality(void) {
    printf("\n=== 测试状态重置功能 ===\n");
    
    // 1. 初始化按键系统
    static const bits_btn_obj_param_t param = TEST_DEFAULT_PARAM();
    button_obj_t button = BITS_BUTTON_INIT(1, 1, &param);
    
    bits_btn_config_t config = {
        .btns = &button,
        .btns_cnt = 1,
        .read_button_level_func = test_framework_mock_read_button,
        .bits_btn_result_cb = test_framework_event_callback,
        .bits_btn_debug_printf = test_framework_log_printf
    };
    bits_button_init(&config);
    printf("✓ 按键系统初始化完成\n");
    
    // 2. 模拟按键进入长按状态
    printf("模拟按键进入长按状态...\n");
    mock_button_press(1);
    time_simulate_debounce_delay();
    time_simulate_long_press_threshold();
    
    // 验证长按事件
    ASSERT_EVENT_WITH_VALUE(1, BTN_EVENT_LONG_PRESS, BITS_BTN_LONG_PRESEE_START_KV);
    printf("✓ 长按状态确认: 按键ID=1, 事件=2\n");
    
    // 3. 调用状态重置函数
    printf("调用状态重置函数...\n");
    bits_button_reset_states();
    printf("✓ 状态重置函数调用成功\n");
    
    // 4. 验证状态重置后按键能正常工作
    printf("验证按键重置后的正常功能...\n");
    
    // 清理之前的状态，重新开始
    mock_button_release(1);
    time_simulate_debounce_delay();
    test_framework_reset();  // 清理事件缓冲区
    
    // 模拟新的单击操作
    mock_button_click(1, STANDARD_CLICK_TIME_MS);
    time_simulate_time_window_end();
    
    // 验证单击事件正常
    ASSERT_EVENT_WITH_VALUE(1, BTN_EVENT_FINISH, BITS_BTN_SINGLE_CLICK_KV);
    printf("✓ 重置后按键功能正常\n");
    
    printf("✓ 状态重置功能测试完成\n");
}

void test_combo_button_reset(void) {
    printf("\n=== 测试组合按键状态重置 ===\n");
    
    // 1. 初始化单按键
    static const bits_btn_obj_param_t param = TEST_DEFAULT_PARAM();
    button_obj_t buttons[2] = {
        BITS_BUTTON_INIT(1, 1, &param),
        BITS_BUTTON_INIT(2, 1, &param)
    };
    
    // 2. 初始化组合按键（抑制单键事件）
    static uint16_t combo_keys[] = {1, 2};
    button_obj_combo_t combo_button = {
        .suppress = 1,  // 抑制单键，只触发组合键事件
        .key_count = 2,
        .key_single_ids = combo_keys,
        .btn = BITS_BUTTON_INIT(100, 1, &param)  // 组合按键ID=100
    };
    
    // 3. 初始化按键系统（包含组合按键）
    bits_btn_config_t config = {
        .btns = buttons,
        .btns_cnt = 2,
        .btns_combo = &combo_button,
        .btns_combo_cnt = 1,
        .read_button_level_func = test_framework_mock_read_button,
        .bits_btn_result_cb = test_framework_event_callback,
        .bits_btn_debug_printf = test_framework_log_printf
    };
    int32_t init_result = bits_button_init(&config);
    
    TEST_ASSERT_EQUAL_INT32(0, init_result);
    printf("✓ 按键系统（含组合按键）初始化完成\n");
    
    // 4. 通过正常按键操作让组合按键进入长按状态
    printf("模拟组合按键进入长按状态...\n");
    mock_button_press(1);
    mock_button_press(2);
    time_simulate_debounce_delay();
    time_simulate_long_press_threshold();
    
    // 验证组合按键长按事件
    ASSERT_EVENT_WITH_VALUE(100, BTN_EVENT_LONG_PRESS, BITS_BTN_LONG_PRESEE_START_KV);
    printf("✓ 组合按键长按状态确认: 按键ID=100, 事件=2\n");
    
    // 5. 调用状态重置函数
    printf("调用状态重置函数...\n");
    bits_button_reset_states();
    printf("✓ 状态重置函数调用成功\n");
    
    // 6. 清理并验证重置后组合按键能正常工作
    printf("验证组合按键重置后的正常功能...\n");
    
    // 释放所有按键并清理事件缓冲区
    mock_button_release(1);
    mock_button_release(2);
    time_simulate_debounce_delay();
    test_framework_reset();  // 清理事件缓冲区
    
    // 重新模拟组合按键单击操作
    mock_button_press(1);
    mock_button_press(2);
    time_simulate_debounce_delay();
    time_simulate_standard_click();
    mock_button_release(1);
    mock_button_release(2);
    time_simulate_debounce_delay();
    time_simulate_time_window_end();
    
    // 验证组合按键单击事件正常（说明状态已被正确重置）
    ASSERT_EVENT_WITH_VALUE(100, BTN_EVENT_FINISH, BITS_BTN_SINGLE_CLICK_KV);
    printf("✓ 重置后组合按键功能正常\n");
    
    // 验证单键事件被抑制（组合按键正常工作）
    ASSERT_EVENT_NOT_EXISTS(1, BTN_EVENT_FINISH);
    ASSERT_EVENT_NOT_EXISTS(2, BTN_EVENT_FINISH);
    printf("✓ 单键事件正确被抑制\n");
    
    printf("✓ 组合按键状态重置测试完成\n");
}