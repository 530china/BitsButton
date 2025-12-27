/* test_main_new.c - 新架构的主测试文件 */
#include "unity.h"
#include "core/test_framework.h"
#include "core/test_runner.h"
#include "config/test_config.h"

// ==================== 外部测试函数声明 ====================

// 基础功能测试
extern void test_single_click_event(void);
extern void test_double_click_event(void);
extern void test_triple_click_event(void);
extern void test_long_press_event(void);
extern void test_long_press_hold_event(void);
extern void test_state_reset_functionality(void);
extern void test_combo_button_reset(void);

// 组合按键测试
extern void test_basic_combo_button(void);
extern void test_combo_long_press(void);

// 边界测试
extern void test_slow_double_click_timeout(void);
extern void test_debounce_functionality(void);
extern void test_very_short_press(void);
extern void test_long_press_boundary(void);
extern void test_rapid_clicks_boundary(void);

// 性能测试
extern void test_high_frequency_button_presses(void);
extern void test_multiple_buttons_concurrent(void);
extern void test_long_running_stability(void);
extern void test_memory_usage(void);

// 新增测试函数
// 缓冲区操作测试
extern void test_buffer_overflow_protection(void);
extern void test_buffer_state_tracking(void);
extern void test_buffer_edge_cases(void);

// 高级组合按键测试
extern void test_advanced_three_key_combo(void);
extern void test_combo_with_different_timing(void);
extern void test_multiple_combos_conflict(void);

// 状态机边界测试
extern void test_state_transition_timing(void);
extern void test_time_window_boundary(void);
extern void test_long_press_period_boundary(void);
extern void test_rapid_state_changes(void);

// 错误处理测试
extern void test_null_pointer_handling(void);
extern void test_invalid_parameters(void);
extern void test_boundary_values(void);
extern void test_resource_exhaustion(void);

// 初始化测试
extern void test_successful_initialization(void);
extern void test_different_active_levels(void);
extern void test_custom_parameters(void);
extern void test_multiple_button_initialization(void);
extern void test_callback_functions(void);

// Peek功能测试
extern void test_peek_functionality(void);
extern void test_peek_vs_get_behavior(void);
extern void test_peek_disabled_buffer_mode(void);

// ==================== 测试套件设置函数 ====================

void basic_tests_setup(void) {
    test_framework_reset();
}

void basic_tests_teardown(void) {
    // 基础测试清理
}

void combo_tests_setup(void) {
    test_framework_reset();
}

void combo_tests_teardown(void) {
    // 组合测试清理
}

void edge_tests_setup(void) {
    test_framework_reset();
}

void edge_tests_teardown(void) {
    // 边界测试清理
}

void performance_tests_setup(void) {
    test_framework_reset();
}

void performance_tests_teardown(void) {
    // 性能测试清理
}

// ==================== Unity标准设置函数 ====================

void setUp(void) {
    // Unity会在每个测试前调用
    test_framework_reset();
}

void tearDown(void) {
    // Unity会在每个测试后调用
    // 清理工作
}

// ==================== 主函数 ====================

int main(void) {
    // 初始化Unity测试框架
    UNITY_BEGIN();

    printf("========================================\n");
    printf("     BitsButton 测试框架 v3.0\n");
    printf("     分层架构 - 模块化设计\n");
    printf("========================================\n\n");

    // 初始化测试框架
    test_framework_init();

    printf("【单按键基础功能测试】\n");
    RUN_TEST(test_single_click_event);
    RUN_TEST(test_double_click_event);
    RUN_TEST(test_triple_click_event);
    RUN_TEST(test_long_press_event);
    RUN_TEST(test_long_press_hold_event);
    RUN_TEST(test_state_reset_functionality);
    RUN_TEST(test_combo_button_reset);

    printf("\n【组合按键功能测试】\n");
    RUN_TEST(test_basic_combo_button);
    RUN_TEST(test_combo_long_press);

    printf("\n【边界条件测试】\n");
    RUN_TEST(test_slow_double_click_timeout);
    RUN_TEST(test_debounce_functionality);
    RUN_TEST(test_very_short_press);
    RUN_TEST(test_long_press_boundary);
    RUN_TEST(test_rapid_clicks_boundary);

    printf("\n【性能压力测试】\n");
    RUN_TEST(test_high_frequency_button_presses);
    RUN_TEST(test_multiple_buttons_concurrent);
    RUN_TEST(test_long_running_stability);
    RUN_TEST(test_memory_usage);

    printf("\n【缓冲区操作测试】\n");
    RUN_TEST(test_buffer_overflow_protection);
    RUN_TEST(test_buffer_state_tracking);
    RUN_TEST(test_buffer_edge_cases);

    printf("\n【高级组合按键测试】\n");
    RUN_TEST(test_advanced_three_key_combo);
    RUN_TEST(test_combo_with_different_timing);
    RUN_TEST(test_multiple_combos_conflict);

    printf("\n【状态机边界测试】\n");
    RUN_TEST(test_state_transition_timing);
    RUN_TEST(test_time_window_boundary);
    RUN_TEST(test_long_press_period_boundary);
    RUN_TEST(test_rapid_state_changes);

    printf("\n【错误处理测试】\n");
    RUN_TEST(test_null_pointer_handling);
    RUN_TEST(test_invalid_parameters);
    RUN_TEST(test_boundary_values);
    RUN_TEST(test_resource_exhaustion);

    printf("\n【初始化配置测试】\n");
    RUN_TEST(test_successful_initialization);
    RUN_TEST(test_different_active_levels);
    RUN_TEST(test_custom_parameters);
    RUN_TEST(test_multiple_button_initialization);
    RUN_TEST(test_callback_functions);

    printf("\n【Peek功能测试】\n");
    RUN_TEST(test_peek_functionality);
    RUN_TEST(test_peek_vs_get_behavior);
    RUN_TEST(test_peek_disabled_buffer_mode);

    printf("\n========================================\n");
    printf("           测试完成\n");
    printf("========================================\n");

    // 清理资源
    test_framework_cleanup();

    // 返回测试结果
    return UNITY_END();
}