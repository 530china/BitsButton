/* test_combo_buttons.c - 组合按键测试 */
#include "unity.h"
#include "core/test_framework.h"
#include "utils/mock_utils.h"
#include "utils/time_utils.h"
#include "utils/assert_utils.h"
#include "config/test_config.h"
#include "bits_button.h"

// ==================== 测试设置和清理 ====================

void combo_setUp(void) {
    test_framework_reset();
    mock_reset_all_buttons();
    time_reset();
}

void combo_tearDown(void) {
    // 测试清理
}

// ==================== 基本组合按键测试 ====================

void test_basic_combo_button(void) {
    printf("\n=== 测试基本组合按键 ===\n");
    
    // 创建单按键对象
    static const bits_btn_obj_param_t param = TEST_DEFAULT_PARAM();
    button_obj_t buttons[] = {
        BITS_BUTTON_INIT(1, 1, &param),
        BITS_BUTTON_INIT(2, 1, &param)
    };
    
    // 创建组合按键
    uint16_t combo_keys[] = {1, 2};
    button_obj_combo_t combo_button = BITS_BUTTON_COMBO_INIT(
        100,                // 组合键ID
        1,                  // 有效电平
        &param,             // 参数配置
        combo_keys,         // 组合按键成员
        2,                  // 组合键成员数量
        1                   // 抑制单键事件
    );
    
    bits_button_init(buttons, 2, &combo_button, 1, 
                     test_framework_mock_read_button, 
                     test_framework_event_callback, 
                     test_framework_log_printf);

    // 模拟组合按键按下
    mock_button_press(1);
    mock_button_press(2);
    time_simulate_debounce_delay();
    time_simulate_pass(200);  // 按住200ms
    mock_button_release(1);
    mock_button_release(2);
    time_simulate_debounce_delay();
    time_simulate_time_window_end();

    // 验证组合按键事件和单键抑制
    uint16_t suppressed_keys[] = {1, 2};
    ASSERT_COMBO_SUPPRESSION(100, suppressed_keys, 2);
    printf("组合按键测试通过\n");
}

// ==================== 三键组合测试 ====================

void test_three_key_combo(void) {
    printf("\n=== 测试三键组合 ===\n");
    
    // 创建单按键对象
    static const bits_btn_obj_param_t param = TEST_DEFAULT_PARAM();
    button_obj_t buttons[] = {
        BITS_BUTTON_INIT(1, 1, &param),
        BITS_BUTTON_INIT(2, 1, &param),
        BITS_BUTTON_INIT(3, 1, &param)
    };
    
    // 创建三键组合
    uint16_t combo_keys[] = {1, 2, 3};
    button_obj_combo_t combo_button = BITS_BUTTON_COMBO_INIT(
        200,                // 组合键ID
        1,                  // 有效电平
        &param,             // 参数配置
        combo_keys,         // 组合按键成员
        3,                  // 组合键成员数量
        1                   // 抑制单键事件
    );
    
    bits_button_init(buttons, 3, &combo_button, 1, 
                     test_framework_mock_read_button, 
                     test_framework_event_callback, 
                     test_framework_log_printf);

    // 模拟三键同时按下
    mock_button_press(1);
    mock_button_press(2);
    mock_button_press(3);
    time_simulate_debounce_delay();
    time_simulate_pass(300);  // 按住300ms
    mock_button_release(1);
    mock_button_release(2);
    mock_button_release(3);
    time_simulate_debounce_delay();
    time_simulate_time_window_end();

    // 验证三键组合事件
    ASSERT_EVENT_EXISTS(200, BTN_STATE_FINISH);
    uint16_t suppressed_keys[] = {1, 2, 3};
    ASSERT_COMBO_SUPPRESSION(200, suppressed_keys, 3);
    printf("三键组合测试通过\n");
}

// ==================== 组合按键长按测试 ====================

void test_combo_long_press(void) {
    printf("\n=== 测试组合按键长按 ===\n");
    
    // 创建单按键对象
    static const bits_btn_obj_param_t param = TEST_DEFAULT_PARAM();
    button_obj_t buttons[] = {
        BITS_BUTTON_INIT(1, 1, &param),
        BITS_BUTTON_INIT(2, 1, &param)
    };
    
    // 创建组合按键
    uint16_t combo_keys[] = {1, 2};
    button_obj_combo_t combo_button = BITS_BUTTON_COMBO_INIT(
        300,                // 组合键ID
        1,                  // 有效电平
        &param,             // 参数配置
        combo_keys,         // 组合按键成员
        2,                  // 组合键成员数量
        1                   // 抑制单键事件
    );
    
    bits_button_init(buttons, 2, &combo_button, 1, 
                     test_framework_mock_read_button, 
                     test_framework_event_callback, 
                     test_framework_log_printf);

    // 模拟组合按键长按
    mock_button_press(1);
    mock_button_press(2);
    time_simulate_debounce_delay();
    time_simulate_long_press_threshold();  // 超过长按阈值
    mock_button_release(1);
    mock_button_release(2);
    time_simulate_debounce_delay();
    time_simulate_time_window_end();

    // 验证组合按键长按事件
    ASSERT_EVENT_EXISTS(300, BTN_STATE_LONG_PRESS);
    printf("组合按键长按测试通过\n");
}

// ==================== 部分组合按键测试 ====================

void test_partial_combo_press(void) {
    printf("\n=== 测试部分组合按键 ===\n");
    
    // 创建单按键对象
    static const bits_btn_obj_param_t param = TEST_DEFAULT_PARAM();
    button_obj_t buttons[] = {
        BITS_BUTTON_INIT(1, 1, &param),
        BITS_BUTTON_INIT(2, 1, &param)
    };
    
    // 创建组合按键（不抑制单键）
    uint16_t combo_keys[] = {1, 2};
    button_obj_combo_t combo_button = BITS_BUTTON_COMBO_INIT(
        400,                // 组合键ID
        1,                  // 有效电平
        &param,             // 参数配置
        combo_keys,         // 组合按键成员
        2,                  // 组合键成员数量
        0                   // 不抑制单键事件
    );
    
    bits_button_init(buttons, 2, &combo_button, 1, 
                     test_framework_mock_read_button, 
                     test_framework_event_callback, 
                     test_framework_log_printf);

    // 只按下一个键
    mock_button_click(1, STANDARD_CLICK_TIME_MS);
    time_simulate_time_window_end();

    // 验证只有单键事件，没有组合键事件
    ASSERT_EVENT_EXISTS(1, BTN_STATE_FINISH);
    ASSERT_EVENT_NOT_EXISTS(400, BTN_STATE_FINISH);
    printf("部分组合按键测试通过\n");
}