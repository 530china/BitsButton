/* test_single_operations.c - 单按键基础操作测试 */
#include "unity.h"
#include "core/test_framework.h"
#include "utils/mock_utils.h"
#include "utils/time_utils.h"
#include "utils/assert_utils.h"
#include "config/test_config.h"
#include "bits_button.h"

// ==================== 测试设置和清理 ====================

void basic_setUp(void) {
    test_framework_reset();
    mock_reset_all_buttons();
    time_reset();
}

void basic_tearDown(void) {
    // 测试清理
}

// ==================== 单击测试 ====================

void test_single_click_event(void) {
    printf("\n=== 测试单击事件 ===\n");
    
    // 创建按键对象
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

    // 模拟单击
    mock_button_click(1, STANDARD_CLICK_TIME_MS);
    time_simulate_time_window_end();

    // 验证结果
    ASSERT_EVENT_WITH_VALUE(1, BTN_EVENT_FINISH, BITS_BTN_SINGLE_CLICK_KV);
    printf("单击测试通过\n");
}

// ==================== 双击测试 ====================

void test_double_click_event(void) {
    printf("\n=== 测试双击事件 ===\n");
    
    // 创建按键对象
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

    // 模拟双击
    mock_multiple_clicks(1, 2, STANDARD_CLICK_TIME_MS, 200);
    time_simulate_time_window_end();

    // 验证结果
    ASSERT_EVENT_WITH_VALUE(1, BTN_EVENT_FINISH, BITS_BTN_DOUBLE_CLICK_KV);
    printf("双击测试通过\n");
}

// ==================== 三连击测试 ====================

void test_triple_click_event(void) {
    printf("\n=== 测试双击事件(快速) ===\n");
    
    // 创建按键对象
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

    // 模拟快速双击（库不支持三连击，所以测试双击）
    mock_multiple_clicks(1, 2, STANDARD_CLICK_TIME_MS, 150);
    time_simulate_time_window_end();

    // 验证结果 - 使用库定义的双击常量
    ASSERT_EVENT_WITH_VALUE(1, BTN_EVENT_FINISH, BITS_BTN_DOUBLE_CLICK_KV);
    printf("快速双击测试通过\n");
}

// ==================== 长按测试 ====================

void test_long_press_event(void) {
    printf("\n=== 测试长按事件 ===\n");
    
    // 创建按键对象
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

    // 模拟长按
    mock_button_press(1);
    time_simulate_debounce_delay();
    time_simulate_long_press_threshold();
    mock_button_release(1);
    time_simulate_debounce_delay();
    time_simulate_time_window_end();

    // 验证长按开始事件
    ASSERT_EVENT_WITH_VALUE(1, BTN_EVENT_LONG_PRESS, BITS_BTN_LONG_PRESEE_START_KV);
    printf("长按测试通过\n");
}

// ==================== 长按保持测试 ====================

void test_long_press_hold_event(void) {
    printf("\n=== 测试长按保持事件 ===\n");
    
    // 创建按键对象，设置较短的长按周期
    static const bits_btn_obj_param_t param = {
        .long_press_period_triger_ms = 500,
        .long_press_start_time_ms = BITS_BTN_LONG_PRESS_START_TIME_MS,
        .short_press_time_ms = BITS_BTN_SHORT_TIME_MS,
        .time_window_time_ms = BITS_BTN_TIME_WINDOW_TIME_MS
    };
    
    button_obj_t button = BITS_BUTTON_INIT(1, 1, &param);
    
    bits_btn_config_t config = {
        .btns = &button,
        .btns_cnt = 1,
        .read_button_level_func = test_framework_mock_read_button,
        .bits_btn_result_cb = test_framework_event_callback,
        .bits_btn_debug_printf = test_framework_log_printf
    };
    bits_button_init(&config);

    // 模拟长按并保持
    mock_button_press(1);
    time_simulate_debounce_delay();
    time_simulate_long_press_threshold();  // 触发长按开始
    time_simulate_pass(600);               // 再等待600ms，触发长按保持
    mock_button_release(1);
    time_simulate_debounce_delay();

    // 验证长按保持事件
    ASSERT_LONG_PRESS_COUNT(1, 1);
    printf("长按保持测试通过\n");
}