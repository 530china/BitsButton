/* test_edge_cases.c - 边界条件测试 */
#include "unity.h"
#include "core/test_framework.h"
#include "utils/mock_utils.h"
#include "utils/time_utils.h"
#include "utils/assert_utils.h"
#include "config/test_config.h"
#include "bits_button.h"

// ==================== 测试设置和清理 ====================

void edge_setUp(void) {
    test_framework_reset();
    mock_reset_all_buttons();
    time_reset();
}

void edge_tearDown(void) {
    // 测试清理
}

// ==================== 超时双击测试 ====================

void test_slow_double_click_timeout(void) {
    printf("\n=== 测试超时双击 ===\n");
    
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

    // 模拟过慢的双击
    mock_button_click(1, STANDARD_CLICK_TIME_MS);  // 第一次点击
    time_simulate_pass(BITS_BTN_TIME_WINDOW_TIME_MS + 50);  // 超过时间窗口
    
    // 第一次点击应该已经完成，产生单击事件
    ASSERT_EVENT_WITH_VALUE(1, BTN_EVENT_FINISH, BITS_BTN_SINGLE_CLICK_KV);
    
    // 重置事件计数器，测试第二次点击
    test_framework_clear_events();
    mock_button_click(1, STANDARD_CLICK_TIME_MS);  // 第二次点击
    time_simulate_time_window_end();
    
    // 第二次点击也应该是单击，而不是双击
    ASSERT_EVENT_WITH_VALUE(1, BTN_EVENT_FINISH, BITS_BTN_SINGLE_CLICK_KV);
    printf("超时双击测试通过: 两次独立的单击事件\n");
}

// ==================== 消抖功能测试 ====================

void test_debounce_functionality(void) {
    printf("\n=== 测试消抖功能 ===\n");
    
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

    // 模拟按键抖动 - 快速按下释放多次
    for (int i = 0; i < 5; i++) {
        mock_button_press(1);
        time_simulate_pass(5);
        mock_button_release(1);
        time_simulate_pass(5);
    }
    
    // 然后正常按下
    mock_button_click(1, STANDARD_CLICK_TIME_MS);
    time_simulate_time_window_end();

    // 应该只有一个有效的按键事件
    ASSERT_EVENT_WITH_VALUE(1, BTN_EVENT_FINISH, BITS_BTN_SINGLE_CLICK_KV);
    printf("消抖测试通过: 抖动被过滤，只有1个有效事件\n");
}

// ==================== 极短按键测试 ====================

void test_very_short_press(void) {
    printf("\n=== 测试极短按键 ===\n");
    
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

    // 模拟极短按键（小于消抖时间）
    mock_button_press(1);
    time_simulate_pass(10);  // 只按10ms
    mock_button_release(1);
    time_simulate_time_window_end();

    // 极短按键应该被消抖过滤掉
    ASSERT_EVENT_COUNT(1, BTN_EVENT_PRESSED, 0);
    ASSERT_EVENT_NOT_EXISTS(1, BTN_EVENT_FINISH);
    printf("极短按键测试通过: 被消抖过滤\n");
}

// ==================== 长按边界测试 ====================

void test_long_press_boundary(void) {
    printf("\n=== 测试长按边界 ===\n");
    
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

    // 测试刚好达到长按阈值
    mock_button_press(1);
    time_simulate_debounce_delay();
    time_simulate_pass(BITS_BTN_LONG_PRESS_START_TIME_MS);  // 刚好达到阈值
    mock_button_release(1);
    time_simulate_debounce_delay();
    time_simulate_time_window_end();

    // 应该触发长按事件
    ASSERT_EVENT_EXISTS(1, BTN_EVENT_LONG_PRESS);
    printf("长按边界测试通过: 刚好达到阈值触发长按\n");
}

// ==================== 快速连击边界测试 ====================

void test_rapid_clicks_boundary(void) {
    printf("\n=== 测试快速连击边界 ===\n");
    
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

    // 模拟在时间窗口边界的快速连击
    mock_button_click(1, 50);   // 第一次点击
    time_simulate_pass(100);    // 间隔100ms
    mock_button_click(1, 50);   // 第二次点击
    time_simulate_pass(BITS_BTN_TIME_WINDOW_TIME_MS - 200);  // 刚好在时间窗口内
    mock_button_click(1, 50);   // 第三次点击
    time_simulate_time_window_end();

    // 应该识别为三连击
    ASSERT_EVENT_WITH_VALUE(1, BTN_EVENT_FINISH, 0b101010);  // 三连击: 010 + 010 + 10 = 101010
    printf("快速连击边界测试通过: 时间窗口边界三连击\n");
}