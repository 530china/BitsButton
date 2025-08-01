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
    bits_button_init(&button, 1, NULL, 0, 
                     test_framework_mock_read_button, 
                     test_framework_event_callback, 
                     test_framework_log_printf);
    printf("✓ 按键系统初始化完成\n");
    
    // 2. 模拟按键进入长按状态
    printf("模拟按键进入长按状态...\n");
    mock_button_press(1);
    time_simulate_debounce_delay();
    time_simulate_long_press_threshold();
    
    // 验证长按事件
    ASSERT_EVENT_WITH_VALUE(1, BTN_STATE_LONG_PRESS, BITS_BTN_LONG_PRESEE_START_KV);
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
    ASSERT_EVENT_WITH_VALUE(1, BTN_STATE_FINISH, BITS_BTN_SINGLE_CLICK_KV);
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
    
    // 2. 初始化组合按键
    static uint16_t combo_keys[] = {1, 2};
    button_obj_combo_t combo_button = {
        .suppress = 0,
        .key_count = 2,
        .key_single_ids = combo_keys,
        .btn = BITS_BUTTON_INIT(100, 1, &param)  // 组合按键ID=100
    };
    
    // 3. 初始化按键系统（包含组合按键）
    int32_t init_result = bits_button_init(buttons, 2, &combo_button, 1,
                                          test_framework_mock_read_button, 
                                          test_framework_event_callback, 
                                          test_framework_log_printf);
    TEST_ASSERT_EQUAL_INT32(0, init_result);
    printf("✓ 按键系统（含组合按键）初始化完成\n");
    
    // 4. 直接设置组合按键状态（模拟非空闲状态）
    printf("设置组合按键为非空闲状态...\n");
    combo_button.btn.current_state = BTN_STATE_PRESSED;
    combo_button.btn.last_state = BTN_STATE_PRESSED;
    combo_button.btn.state_bits = 0x5;  // 设置一些状态位
    combo_button.btn.state_entry_time = 1000;
    combo_button.btn.long_press_period_trigger_cnt = 2;
    
    // 验证状态确实被设置了
    TEST_ASSERT_EQUAL_INT(BTN_STATE_PRESSED, combo_button.btn.current_state);
    TEST_ASSERT_EQUAL_INT(BTN_STATE_PRESSED, combo_button.btn.last_state);
    TEST_ASSERT_EQUAL_INT(0x5, combo_button.btn.state_bits);
    TEST_ASSERT_EQUAL_INT(1000, combo_button.btn.state_entry_time);
    TEST_ASSERT_EQUAL_INT(2, combo_button.btn.long_press_period_trigger_cnt);
    printf("✓ 组合按键状态设置确认\n");
    
    // 5. 调用状态重置函数
    printf("调用状态重置函数...\n");
    bits_button_reset_states();
    printf("✓ 状态重置函数调用成功\n");
    
    // 6. 验证组合按键状态是否被重置
    printf("验证组合按键状态重置结果...\n");
    
    // 这里是关键测试：如果组合按键重置代码被注释掉，这些断言会失败
    TEST_ASSERT_EQUAL_INT_MESSAGE(BTN_STATE_IDLE, combo_button.btn.current_state, 
                                  "组合按键 current_state 应该被重置为 BTN_STATE_IDLE");
    TEST_ASSERT_EQUAL_INT_MESSAGE(BTN_STATE_IDLE, combo_button.btn.last_state,
                                  "组合按键 last_state 应该被重置为 BTN_STATE_IDLE");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, combo_button.btn.state_bits,
                                  "组合按键 state_bits 应该被重置为 0");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, combo_button.btn.state_entry_time,
                                  "组合按键 state_entry_time 应该被重置为 0");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, combo_button.btn.long_press_period_trigger_cnt,
                                  "组合按键 long_press_period_trigger_cnt 应该被重置为 0");
    
    printf("✓ 组合按键状态重置验证通过\n");
    printf("✓ 组合按键状态重置测试完成\n");
}
