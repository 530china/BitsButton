/* test_performance.c - 性能测试 */
#include "unity.h"
#include "core/test_framework.h"
#include "utils/mock_utils.h"
#include "utils/time_utils.h"
#include "utils/assert_utils.h"
#include "config/test_config.h"
#include "bits_button.h"

// ==================== 测试设置和清理 ====================

void performance_setUp(void) {
    test_framework_reset();
    mock_reset_all_buttons();
    time_reset();
}

void performance_tearDown(void) {
    // 测试清理
}

// ==================== 高频按键处理测试 ====================

void test_high_frequency_button_presses(void) {
    printf("\n=== 测试高频按键处理 ===\n");
    
    // 创建按键对象
    static const bits_btn_obj_param_t param = TEST_DEFAULT_PARAM();
    button_obj_t button = BITS_BUTTON_INIT(1, 1, &param);
    bits_button_init(&button, 1, NULL, 0, 
                     test_framework_mock_read_button, 
                     test_framework_event_callback, 
                     test_framework_log_printf);

    // 模拟快速连续点击
    int expected_clicks = 5;
    mock_multiple_clicks(1, expected_clicks, 50, 100);
    time_simulate_time_window_end();

    // 验证最终的连击序列 (5连击应该是 0b1010101010)
    uint32_t expected_pattern = 0b1010101010;
    ASSERT_EVENT_WITH_VALUE(1, BTN_STATE_FINISH, expected_pattern);
    printf("高频按键测试通过: %d连击\n", expected_clicks);
}

// ==================== 多按键并发测试 ====================

void test_multiple_buttons_concurrent(void) {
    printf("\n=== 测试多按键并发处理 ===\n");
    
    // 创建多个按键对象
    static const bits_btn_obj_param_t param = TEST_DEFAULT_PARAM();
    button_obj_t buttons[] = {
        BITS_BUTTON_INIT(1, 1, &param),
        BITS_BUTTON_INIT(2, 1, &param),
        BITS_BUTTON_INIT(3, 1, &param),
        BITS_BUTTON_INIT(4, 1, &param)
    };
    
    bits_button_init(buttons, 4, NULL, 0, 
                     test_framework_mock_read_button, 
                     test_framework_event_callback, 
                     test_framework_log_printf);

    // 模拟多个按键同时操作
    mock_button_click(1, STANDARD_CLICK_TIME_MS);  // 按键1单击
    mock_multiple_clicks(2, 2, 80, 120);           // 按键2双击
    mock_multiple_clicks(3, 3, 60, 100);           // 按键3三连击
    
    // 按键4长按
    mock_button_press(4);
    time_simulate_debounce_delay();
    time_simulate_long_press_threshold();
    mock_button_release(4);
    time_simulate_debounce_delay();
    
    time_simulate_time_window_end();

    // 验证各个按键的事件
    ASSERT_EVENT_WITH_VALUE(1, BTN_STATE_FINISH, BITS_BTN_SINGLE_CLICK_KV);
    ASSERT_EVENT_WITH_VALUE(2, BTN_STATE_FINISH, BITS_BTN_DOUBLE_CLICK_KV);
    ASSERT_EVENT_WITH_VALUE(3, BTN_STATE_FINISH, 0b0101010);
    ASSERT_EVENT_EXISTS(4, BTN_STATE_LONG_PRESS);
    
    printf("多按键并发测试通过: 4个按键同时处理\n");
}

// ==================== 长时间运行稳定性测试 ====================

void test_long_running_stability(void) {
    printf("\n=== 测试长时间运行稳定性 ===\n");
    
    // 创建按键对象
    static const bits_btn_obj_param_t param = TEST_DEFAULT_PARAM();
    button_obj_t button = BITS_BUTTON_INIT(1, 1, &param);
    bits_button_init(&button, 1, NULL, 0, 
                     test_framework_mock_read_button, 
                     test_framework_event_callback, 
                     test_framework_log_printf);

    // 模拟长时间的按键操作序列
    for (int cycle = 0; cycle < 10; cycle++) {
        // 每个周期包含不同类型的操作
        mock_button_click(1, STANDARD_CLICK_TIME_MS);  // 单击
        time_simulate_pass(300);
        
        mock_multiple_clicks(1, 2, 80, 150);           // 双击
        time_simulate_pass(500);
        
        // 长按
        mock_button_press(1);
        time_simulate_debounce_delay();
        time_simulate_pass(800);
        mock_button_release(1);
        time_simulate_debounce_delay();
        time_simulate_pass(400);
        
        // 清空事件以避免溢出
        if (cycle % 3 == 2) {
            test_framework_clear_events();
        }
    }

    // 最后一次操作验证
    mock_button_click(1, STANDARD_CLICK_TIME_MS);
    time_simulate_time_window_end();
    
    ASSERT_EVENT_WITH_VALUE(1, BTN_STATE_FINISH, BITS_BTN_SINGLE_CLICK_KV);
    printf("长时间运行稳定性测试通过: 10个周期操作\n");
}

// ==================== 内存使用测试 ====================

void test_memory_usage(void) {
    printf("\n=== 测试内存使用 ===\n");
    
    // 创建最大数量的按键对象
    static const bits_btn_obj_param_t param = TEST_DEFAULT_PARAM();
    button_obj_t buttons[MAX_TEST_BUTTONS];
    
    // 初始化所有按键
    for (int i = 0; i < MAX_TEST_BUTTONS; i++) {
        buttons[i] = (button_obj_t)BITS_BUTTON_INIT(i, 1, &param);
    }
    
    bits_button_init(buttons, MAX_TEST_BUTTONS, NULL, 0, 
                     test_framework_mock_read_button, 
                     test_framework_event_callback, 
                     test_framework_log_printf);

    // 同时操作所有按键
    for (int i = 0; i < MAX_TEST_BUTTONS; i++) {
        mock_button_click(i, STANDARD_CLICK_TIME_MS);
    }
    time_simulate_time_window_end();

    // 验证所有按键都产生了事件
    for (int i = 0; i < MAX_TEST_BUTTONS; i++) {
        ASSERT_EVENT_EXISTS(i, BTN_STATE_FINISH);
    }
    
    printf("内存使用测试通过: %d个按键同时工作\n", MAX_TEST_BUTTONS);
}

// ==================== 极限频率测试 ====================

void test_extreme_frequency(void) {
    printf("\n=== 测试极限频率处理 ===\n");
    
    // 创建按键对象
    static const bits_btn_obj_param_t param = TEST_DEFAULT_PARAM();
    button_obj_t button = BITS_BUTTON_INIT(1, 1, &param);
    bits_button_init(&button, 1, NULL, 0, 
                     test_framework_mock_read_button, 
                     test_framework_event_callback, 
                     test_framework_log_printf);

    // 模拟极高频率的按键操作（每次间隔很短）
    int extreme_clicks = 8;
    for (int i = 0; i < extreme_clicks; i++) {
        mock_button_click(1, 30);   // 很短的按键时间
        time_simulate_pass(50);     // 很短的间隔
    }
    time_simulate_time_window_end();

    // 验证系统能够处理极限频率
    ASSERT_EVENT_EXISTS(1, BTN_STATE_FINISH);
    printf("极限频率测试通过: %d次极高频操作\n", extreme_clicks);
}