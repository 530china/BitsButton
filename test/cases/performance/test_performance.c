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
    
    bits_btn_config_t config = {
        .btns = &button,
        .btns_cnt = 1,
        .read_button_level_func = test_framework_mock_read_button,
        .bits_btn_result_cb = test_framework_event_callback,
        .bits_btn_debug_printf = test_framework_log_printf
    };
    bits_button_init(&config);

    // 模拟快速连续点击
    int expected_clicks = 5;
    mock_multiple_clicks(1, expected_clicks, 50, 100);
    time_simulate_time_window_end();

    // 验证最终的连击序列 - 5连击的位模式(5连击应该是 0b1010101010)
    uint32_t expected_pattern = 0b1010101010; // 5连击: 010+010+010+010+10 = 1010101010
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
    
    bits_btn_config_t config = {
        .btns = buttons,
        .btns_cnt = 4,
        .read_button_level_func = test_framework_mock_read_button,
        .bits_btn_result_cb = test_framework_event_callback,
        .bits_btn_debug_printf = test_framework_log_printf
    };
    bits_button_init(&config);

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
    ASSERT_EVENT_WITH_VALUE(3, BTN_STATE_FINISH, 0b101010);  // 三连击: 010 + 010 + 10 = 101010
    ASSERT_EVENT_EXISTS(4, BTN_STATE_LONG_PRESS);
    
    printf("多按键并发测试通过: 4个按键同时处理\n");
}

// ==================== 长时间运行稳定性测试 ====================

void test_long_running_stability(void) {
    printf("\n=== 测试长时间运行稳定性 ===\n");
    
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

    // 模拟连续的按键操作序列
    for (int cycle = 0; cycle < 5; cycle++) {
        // 每个周期包含不同类型的操作
        mock_button_click(1, STANDARD_CLICK_TIME_MS);  // 单击
        time_simulate_pass(300);  // 间隔300ms
        
        mock_button_click(1, STANDARD_CLICK_TIME_MS);  // 再次单击
        time_simulate_pass(300);  // 间隔300ms
    }
    
    time_simulate_time_window_end();
    
    // 验证系统在连续操作后仍然稳定 - 应该产生10连击模式
    ASSERT_EVENT_WITH_VALUE(1, BTN_STATE_FINISH, 0b10101010101010101010); // 10连击模式
    printf("长时间运行稳定性测试通过: 连续操作稳定\n");
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
    
    bits_btn_config_t config = {
        .btns = buttons,
        .btns_cnt = MAX_TEST_BUTTONS,
        .read_button_level_func = test_framework_mock_read_button,
        .bits_btn_result_cb = test_framework_event_callback,
        .bits_btn_debug_printf = test_framework_log_printf
    };
    bits_button_init(&config);

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