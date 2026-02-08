/* test_advanced_combo.c - 高级组合按键测试 */
#include "unity.h"
#include "core/test_framework.h"
#include "utils/mock_utils.h"
#include "utils/time_utils.h"
#include "utils/assert_utils.h"
#include "config/test_config.h"
#include "bits_button.h"

// ==================== 测试设置和清理 ====================

void advanced_combo_setUp(void) {
    test_framework_reset();
    mock_reset_all_buttons();
    time_reset();
}

void advanced_combo_tearDown(void) {
    // 测试清理
}

// ==================== 高级组合按键测试 ====================

void test_advanced_three_key_combo(void) {
    printf("\n=== 测试三键组合 ===\n");
    
    // 创建三个单键
    static const bits_btn_obj_param_t param = TEST_DEFAULT_PARAM();
    button_obj_t buttons[] = {
        BITS_BUTTON_INIT(1, 1, &param),
        BITS_BUTTON_INIT(2, 1, &param),
        BITS_BUTTON_INIT(3, 1, &param)
    };
    
    // 创建三键组合
    static uint16_t combo_keys[] = {1, 2, 3};
    button_obj_combo_t combo = BITS_BUTTON_COMBO_INIT(100, 1, &param, combo_keys, 3, 1);
    
    bits_btn_config_t config = {
        .btns = buttons,
        .btns_cnt = 3,
        .btns_combo = &combo,
        .btns_combo_cnt = 1,
        .read_button_level_func = test_framework_mock_read_button,
        .bits_btn_result_cb = test_framework_event_callback,
        .bits_btn_debug_printf = test_framework_log_printf
    };
    bits_button_init(&config);

    // 同时按下三个键
    mock_set_button_state(1, 1);
    mock_set_button_state(2, 1);
    mock_set_button_state(3, 1);
    time_simulate_debounce_delay();
    
    // 释放所有键
    mock_set_button_state(1, 0);
    mock_set_button_state(2, 0);
    mock_set_button_state(3, 0);
    time_simulate_debounce_delay();
    time_simulate_time_window_end();
    
    // 验证组合键事件
    ASSERT_EVENT_EXISTS(100, BTN_EVENT_FINISH);
    
    // 验证单键被抑制
    ASSERT_EVENT_NOT_EXISTS(1, BTN_EVENT_FINISH);
    ASSERT_EVENT_NOT_EXISTS(2, BTN_EVENT_FINISH);
    ASSERT_EVENT_NOT_EXISTS(3, BTN_EVENT_FINISH);
    
    printf("三键组合测试通过: 组合键触发，单键被抑制\n");
}

void test_combo_with_different_timing(void) {
    printf("\n=== 测试不同时序的组合按键 ===\n");
    
    // 创建两个单键
    static const bits_btn_obj_param_t param = TEST_DEFAULT_PARAM();
    button_obj_t buttons[] = {
        BITS_BUTTON_INIT(1, 1, &param),
        BITS_BUTTON_INIT(2, 1, &param)
    };
    
    // 创建组合键
    static uint16_t combo_keys[] = {1, 2};
    button_obj_combo_t combo = BITS_BUTTON_COMBO_INIT(100, 1, &param, combo_keys, 2, 1);
    
    bits_btn_config_t config = {
        .btns = buttons,
        .btns_cnt = 2,
        .btns_combo = &combo,
        .btns_combo_cnt = 1,
        .read_button_level_func = test_framework_mock_read_button,
        .bits_btn_result_cb = test_framework_event_callback,
        .bits_btn_debug_printf = test_framework_log_printf
    };
    bits_button_init(&config);

    // 测试1: 先按键1，后按键2
    mock_set_button_state(1, 1);
    time_simulate_pass(50);
    mock_set_button_state(2, 1);
    time_simulate_debounce_delay();
    
    mock_set_button_state(1, 0);
    mock_set_button_state(2, 0);
    time_simulate_debounce_delay();
    time_simulate_time_window_end();
    
    // 验证组合键事件
    ASSERT_EVENT_EXISTS(100, BTN_EVENT_FINISH);
    
    // 清空事件
    test_framework_clear_events();
    
    // 测试2: 先按键2，后按键1
    mock_set_button_state(2, 1);
    time_simulate_pass(50);
    mock_set_button_state(1, 1);
    time_simulate_debounce_delay();
    
    mock_set_button_state(1, 0);
    mock_set_button_state(2, 0);
    time_simulate_debounce_delay();
    time_simulate_time_window_end();
    
    // 验证组合键事件
    ASSERT_EVENT_EXISTS(100, BTN_EVENT_FINISH);
    
    printf("不同时序组合按键测试通过: 按键顺序不影响组合识别\n");
}


void test_multiple_combos_conflict(void) {
    printf("\n=== 测试多组合键冲突 ===\n");
    
    // 创建三个单键
    static const bits_btn_obj_param_t param = TEST_DEFAULT_PARAM();
    button_obj_t buttons[] = {
        BITS_BUTTON_INIT(1, 1, &param),
        BITS_BUTTON_INIT(2, 1, &param),
        BITS_BUTTON_INIT(3, 1, &param)
    };
    
    // 创建两个有重叠的组合键
    static uint16_t combo1_keys[] = {1, 2};
    static uint16_t combo2_keys[] = {2, 3};
    button_obj_combo_t combos[] = {
        BITS_BUTTON_COMBO_INIT(100, 1, &param, combo1_keys, 2, 1),
        BITS_BUTTON_COMBO_INIT(101, 1, &param, combo2_keys, 2, 1)
    };
    
    bits_btn_config_t config = {
        .btns = buttons,
        .btns_cnt = 3,
        .btns_combo = combos,
        .btns_combo_cnt = 2,
        .read_button_level_func = test_framework_mock_read_button,
        .bits_btn_result_cb = test_framework_event_callback,
        .bits_btn_debug_printf = test_framework_log_printf
    };
    bits_button_init(&config);

    // 按下键1和键2（应该触发组合键100）
    mock_set_button_state(1, 1);
    mock_set_button_state(2, 1);
    time_simulate_debounce_delay();
    
    mock_set_button_state(1, 0);
    mock_set_button_state(2, 0);
    time_simulate_debounce_delay();
    time_simulate_time_window_end();
    
    // 验证组合键100触发
    ASSERT_EVENT_EXISTS(100, BTN_EVENT_FINISH);
    ASSERT_EVENT_NOT_EXISTS(101, BTN_EVENT_FINISH);
    
    // 清空事件
    test_framework_clear_events();
    
    // 按下键2和键3（应该触发组合键101）
    mock_set_button_state(2, 1);
    mock_set_button_state(3, 1);
    time_simulate_debounce_delay();
    
    mock_set_button_state(2, 0);
    mock_set_button_state(3, 0);
    time_simulate_debounce_delay();
    time_simulate_time_window_end();
    
    // 验证组合键101触发
    ASSERT_EVENT_EXISTS(101, BTN_EVENT_FINISH);
    ASSERT_EVENT_NOT_EXISTS(100, BTN_EVENT_FINISH);
    
    printf("多组合键冲突测试通过: 不同组合键正确识别\n");
}