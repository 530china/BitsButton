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
    
    bits_btn_config_t config = {
        .btns = buttons,
        .btns_cnt = 2,
        .btns_combo = &combo_button,
        .btns_combo_cnt = 1,
        .read_button_level_func = test_framework_mock_read_button,
        .bits_btn_result_cb = test_framework_event_callback,
        .bits_btn_debug_printf = test_framework_log_printf
    };
    bits_button_init(&config);

    // 模拟组合按键按下
    mock_button_press(1);
    mock_button_press(2);
    time_simulate_debounce_delay();
    time_simulate_pass(200);  // 按住200ms
    mock_button_release(1);
    mock_button_release(2);
    time_simulate_debounce_delay();
    time_simulate_time_window_end();

    // 验证组合按键事件
    ASSERT_EVENT_EXISTS(100, BTN_STATE_FINISH);
    
    // 验证单键被抑制（不应该有单键事件）
    ASSERT_EVENT_NOT_EXISTS(1, BTN_STATE_FINISH);
    ASSERT_EVENT_NOT_EXISTS(2, BTN_STATE_FINISH);
    printf("组合按键测试通过\n");
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
    
    bits_btn_config_t config = {
        .btns = buttons,
        .btns_cnt = 2,
        .btns_combo = &combo_button,
        .btns_combo_cnt = 1,
        .read_button_level_func = test_framework_mock_read_button,
        .bits_btn_result_cb = test_framework_event_callback,
        .bits_btn_debug_printf = test_framework_log_printf
    };
    bits_button_init(&config);

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