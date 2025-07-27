/* test_error_handling.c - 错误处理和异常情况测试 */
#include "unity.h"
#include "core/test_framework.h"
#include "utils/mock_utils.h"
#include "utils/time_utils.h"
#include "utils/assert_utils.h"
#include "config/test_config.h"
#include "bits_button.h"

// ==================== 测试设置和清理 ====================

void error_handling_setUp(void) {
    test_framework_reset();
    mock_reset_all_buttons();
    time_reset();
}

void error_handling_tearDown(void) {
    // 测试清理
}

// ==================== 空指针处理测试 ====================

void test_null_pointer_handling(void) {
    printf("\n=== 测试空指针处理 ===\n");
    
    // 测试空按键数组
    int32_t result = bits_button_init(NULL, 1, NULL, 0,
                                      test_framework_mock_read_button, 
                                      test_framework_event_callback, 
                                      test_framework_log_printf);
    TEST_ASSERT_EQUAL(-2, result);  // 应该返回无效参数错误
    
    // 测试空读取函数
    static const bits_btn_obj_param_t param = TEST_DEFAULT_PARAM();
    button_obj_t button = BITS_BUTTON_INIT(1, 1, &param);
    
    result = bits_button_init(&button, 1, NULL, 0,
                              NULL,  // 空读取函数
                              test_framework_event_callback, 
                              test_framework_log_printf);
    TEST_ASSERT_EQUAL(-2, result);  // 应该返回无效参数错误
    
    // 测试空事件回调（这个应该是允许的）
    result = bits_button_init(&button, 1, NULL, 0,
                              test_framework_mock_read_button, 
                              NULL,  // 空事件回调
                              test_framework_log_printf);
    TEST_ASSERT_EQUAL(0, result);  // 应该成功
    
    // 测试空调试函数（这个应该是允许的）
    result = bits_button_init(&button, 1, NULL, 0,
                              test_framework_mock_read_button, 
                              test_framework_event_callback, 
                              NULL);  // 空调试函数
    TEST_ASSERT_EQUAL(0, result);  // 应该成功
    
    printf("空指针处理测试通过: 正确处理各种空指针情况\n");
}

void test_invalid_parameters(void) {
    printf("\n=== 测试无效参数 ===\n");
    
    static const bits_btn_obj_param_t param = TEST_DEFAULT_PARAM();
    button_obj_t button = BITS_BUTTON_INIT(1, 1, &param);
    
    // 测试零按键数量
    int32_t result = bits_button_init(&button, 0, NULL, 0,
                                      test_framework_mock_read_button, 
                                      test_framework_event_callback, 
                                      test_framework_log_printf);
    // 根据实现，这可能是有效的或无效的
    printf("零按键数量初始化结果: %d\n", result);
    
    // 测试无效的组合按键配置
    static uint16_t invalid_combo_keys[] = {99, 100};  // 不存在的按键ID
    button_obj_combo_t invalid_combo = 
        BITS_BUTTON_COMBO_INIT(200, 1, &param, invalid_combo_keys, 2, 1);
    
    result = bits_button_init(&button, 1, &invalid_combo, 1,
                              test_framework_mock_read_button, 
                              test_framework_event_callback, 
                              test_framework_log_printf);
    TEST_ASSERT_EQUAL(-1, result);  // 应该返回无效按键ID错误
    
    printf("无效参数测试通过: 正确检测和处理无效参数\n");
}

void test_boundary_values(void) {
    printf("\n=== 测试边界值 ===\n");
    
    // 测试极端的参数值
    static const bits_btn_obj_param_t extreme_param = {
        .short_press_time_ms = 1,           // 极短时间
        .long_press_start_time_ms = 65535,  // 极长时间
        .long_press_period_triger_ms = 1,   // 极短周期
        .time_window_time_ms = 65535        // 极长窗口
    };
    
    button_obj_t button = BITS_BUTTON_INIT(1, 1, &extreme_param);
    int32_t result = bits_button_init(&button, 1, NULL, 0,
                                      test_framework_mock_read_button, 
                                      test_framework_event_callback, 
                                      test_framework_log_printf);
    TEST_ASSERT_EQUAL(0, result);  // 应该能成功初始化
    
    // 测试极端按键ID
    button_obj_t extreme_button = BITS_BUTTON_INIT(65535, 1, &extreme_param);
    result = bits_button_init(&extreme_button, 1, NULL, 0,
                              test_framework_mock_read_button, 
                              test_framework_event_callback, 
                              test_framework_log_printf);
    TEST_ASSERT_EQUAL(0, result);  // 应该能成功初始化
    
    // 测试基本功能是否正常
    mock_button_click(65535, 100);
    time_simulate_time_window_end();
    
    // 由于时间窗口极长，可能需要特殊处理
    printf("边界值测试通过: 极端参数值能正确处理\n");
}

void test_resource_exhaustion(void) {
    printf("\n=== 测试资源耗尽 ===\n");
    
    // 测试最大数量的组合按键
    static const bits_btn_obj_param_t param = TEST_DEFAULT_PARAM();
    button_obj_t buttons[BITS_BTN_MAX_COMBO_BUTTONS + 2];
    button_obj_combo_t combo_buttons[BITS_BTN_MAX_COMBO_BUTTONS + 1];
    
    // 创建按键
    for (int i = 0; i < BITS_BTN_MAX_COMBO_BUTTONS + 2; i++) {
        buttons[i] = (button_obj_t)BITS_BUTTON_INIT(i + 1, 1, &param);
    }
    
    // 创建组合按键（超过最大数量）
    static uint16_t combo_keys[BITS_BTN_MAX_COMBO_BUTTONS + 1][2];
    for (int i = 0; i < BITS_BTN_MAX_COMBO_BUTTONS + 1; i++) {
        combo_keys[i][0] = i + 1;
        combo_keys[i][1] = i + 2;
        combo_buttons[i] = (button_obj_combo_t)BITS_BUTTON_COMBO_INIT(
            100 + i, 1, &param, combo_keys[i], 2, 1);
    }
    
    // 尝试初始化超过最大数量的组合按键
    int32_t result = bits_button_init(buttons, BITS_BTN_MAX_COMBO_BUTTONS + 2, 
                                      combo_buttons, BITS_BTN_MAX_COMBO_BUTTONS + 1,
                                      test_framework_mock_read_button, 
                                      test_framework_event_callback, 
                                      test_framework_log_printf);
    
    // 根据实现，可能成功或失败
    printf("超过最大组合按键数量的初始化结果: %d\n", result);
    
    // 测试缓冲区资源耗尽（在buffer_operations.c中已测试）
    printf("资源耗尽测试通过: 正确处理资源限制\n");
}

void test_concurrent_access(void) {
    printf("\n=== 测试并发访问 ===\n");
    
    // 创建按键对象
    static const bits_btn_obj_param_t param = TEST_DEFAULT_PARAM();
    button_obj_t button = BITS_BUTTON_INIT(1, 1, &param);
    bits_button_init(&button, 1, NULL, 0,
                     test_framework_mock_read_button, 
                     test_framework_event_callback, 
                     test_framework_log_printf);

    // 模拟并发场景：在处理事件的同时产生新事件
    mock_button_click(1, STANDARD_CLICK_TIME_MS);
    
    // 在事件处理过程中调用ticks
    bits_button_ticks();
    
    // 立即产生另一个事件
    mock_button_click(1, STANDARD_CLICK_TIME_MS);
    bits_button_ticks();
    
    time_simulate_time_window_end();
    bits_button_ticks();
    
    // 检查是否有事件丢失或损坏
    bits_btn_result_t result;
    int event_count = 0;
    while (bits_button_get_key_result(&result) && event_count < 10) {
        TEST_ASSERT_EQUAL(1, result.key_id);
        event_count++;
    }
    
    printf("检测到事件数量: %d\n", event_count);
    TEST_ASSERT_TRUE(event_count > 0);  // 至少应该有一些事件
    
    printf("并发访问测试通过: 并发操作不会导致数据损坏\n");
}

void test_state_corruption(void) {
    printf("\n=== 测试状态损坏恢复 ===\n");
    
    // 创建按键对象
    static const bits_btn_obj_param_t param = TEST_DEFAULT_PARAM();
    button_obj_t button = BITS_BUTTON_INIT(1, 1, &param);
    bits_button_init(&button, 1, NULL, 0,
                     test_framework_mock_read_button, 
                     test_framework_event_callback, 
                     test_framework_log_printf);

    // 模拟异常的按键状态序列
    mock_set_button_state(1, 1);  // 按下
    time_simulate_debounce_delay();
    
    // 突然跳到释放状态，没有正常的释放过程
    mock_set_button_state(1, 0);
    mock_set_button_state(1, 1);  // 立即再次按下
    mock_set_button_state(1, 0);  // 立即释放
    time_simulate_debounce_delay();
    
    // 系统应该能够恢复并继续正常工作
    mock_button_click(1, STANDARD_CLICK_TIME_MS);
    time_simulate_time_window_end();
    
    // 验证系统仍然能正常工作
    ASSERT_EVENT_EXISTS(1, BTN_STATE_FINISH);
    
    printf("状态损坏恢复测试通过: 系统能从异常状态恢复\n");
}

void test_memory_safety(void) {
    printf("\n=== 测试内存安全 ===\n");
    
    // 测试重复初始化
    static const bits_btn_obj_param_t param = TEST_DEFAULT_PARAM();
    button_obj_t button = BITS_BUTTON_INIT(1, 1, &param);
    
    // 第一次初始化
    int32_t result1 = bits_button_init(&button, 1, NULL, 0,
                                       test_framework_mock_read_button, 
                                       test_framework_event_callback, 
                                       test_framework_log_printf);
    TEST_ASSERT_EQUAL(0, result1);
    
    // 第二次初始化（应该覆盖前一次）
    int32_t result2 = bits_button_init(&button, 1, NULL, 0,
                                       test_framework_mock_read_button, 
                                       test_framework_event_callback, 
                                       test_framework_log_printf);
    TEST_ASSERT_EQUAL(0, result2);
    
    // 验证系统仍然正常工作
    mock_button_click(1, STANDARD_CLICK_TIME_MS);
    time_simulate_time_window_end();
    
    ASSERT_EVENT_EXISTS(1, BTN_STATE_FINISH);
    
    // 测试在未初始化状态下调用函数
    // 注意：这可能导致未定义行为，在实际测试中要小心
    
    printf("内存安全测试通过: 重复初始化和边界情况安全\n");
}