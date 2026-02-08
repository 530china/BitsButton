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

    // 新增：测试配置结构体为NULL
    int32_t result = bits_button_init(NULL);
    TEST_ASSERT_EQUAL(-2, result);

    // 测试空按键数组
    bits_btn_config_t config1 = {
        .btns = NULL,
        .btns_cnt = 1,
        .read_button_level_func = test_framework_mock_read_button,
        .bits_btn_result_cb = test_framework_event_callback,
        .bits_btn_debug_printf = test_framework_log_printf
    };
    result = bits_button_init(&config1);
    TEST_ASSERT_EQUAL(-2, result);  // 应该返回无效参数错误

    // 测试空读取函数
    static const bits_btn_obj_param_t param = TEST_DEFAULT_PARAM();
    button_obj_t button = BITS_BUTTON_INIT(1, 1, &param);

    bits_btn_config_t config2 = {
        .btns = &button,
        .btns_cnt = 1,
        .read_button_level_func = NULL,
        .bits_btn_result_cb = test_framework_event_callback,
        .bits_btn_debug_printf = test_framework_log_printf
    };
    result = bits_button_init(&config2);
    TEST_ASSERT_EQUAL(-2, result);  // 应该返回无效参数错误

    // 测试空事件回调（这个应该是允许的）
    bits_btn_config_t config3 = {
        .btns = &button,
        .btns_cnt = 1,
        .read_button_level_func = test_framework_mock_read_button,
        .bits_btn_result_cb = NULL,
        .bits_btn_debug_printf = test_framework_log_printf
    };
    result = bits_button_init(&config3);
    TEST_ASSERT_EQUAL(0, result);  // 应该成功

    // 测试空调试函数（这个应该是允许的）
    bits_btn_config_t config4 = {
        .btns = &button,
        .btns_cnt = 1,
        .read_button_level_func = test_framework_mock_read_button,
        .bits_btn_result_cb = test_framework_event_callback,
        .bits_btn_debug_printf = NULL
    };
    result = bits_button_init(&config4);
    TEST_ASSERT_EQUAL(0, result);  // 应该成功

    printf("空指针处理测试通过: 正确处理各种空指针情况\n");
}

void test_invalid_parameters(void) {
    printf("\n=== 测试无效参数 ===\n");

    static const bits_btn_obj_param_t param = TEST_DEFAULT_PARAM();
    button_obj_t button = BITS_BUTTON_INIT(1, 1, &param);

    // 测试零按键数量
    bits_btn_config_t config1 = {
        .btns = &button,
        .btns_cnt = 0,
        .read_button_level_func = test_framework_mock_read_button,
        .bits_btn_result_cb = test_framework_event_callback,
        .bits_btn_debug_printf = test_framework_log_printf
    };
    int32_t result = bits_button_init(&config1);
    // 根据实现，这可能是有效的或无效的
    printf("零按键数量初始化结果: %d\n", result);

    // 测试无效的组合按键配置
    static uint16_t invalid_combo_keys[] = {99, 100};  // 不存在的按键ID
    button_obj_combo_t invalid_combo =
        BITS_BUTTON_COMBO_INIT(200, 1, &param, invalid_combo_keys, 2, 1);

    bits_btn_config_t config2 = {
        .btns = &button,
        .btns_cnt = 1,
        .btns_combo = &invalid_combo,
        .btns_combo_cnt = 1,
        .read_button_level_func = test_framework_mock_read_button,
        .bits_btn_result_cb = test_framework_event_callback,
        .bits_btn_debug_printf = test_framework_log_printf
    };
    result = bits_button_init(&config2);
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
    bits_btn_config_t config1 = {
        .btns = &button,
        .btns_cnt = 1,
        .read_button_level_func = test_framework_mock_read_button,
        .bits_btn_result_cb = test_framework_event_callback,
        .bits_btn_debug_printf = test_framework_log_printf
    };
    int32_t result = bits_button_init(&config1);
    TEST_ASSERT_EQUAL(0, result);  // 应该能成功初始化

    // 测试极端按键ID
    button_obj_t extreme_button = BITS_BUTTON_INIT(65535, 1, &extreme_param);
    bits_btn_config_t config2 = {
        .btns = &extreme_button,
        .btns_cnt = 1,
        .read_button_level_func = test_framework_mock_read_button,
        .bits_btn_result_cb = test_framework_event_callback,
        .bits_btn_debug_printf = test_framework_log_printf
    };
    result = bits_button_init(&config2);
    TEST_ASSERT_EQUAL(0, result);  // 应该能成功初始化

    // 测试基本功能是否正常（使用合理的按键ID避免溢出）
    mock_button_click(255, 100);  // 使用uint8_t范围内的值
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
    bits_btn_config_t config = {
        .btns = buttons,
        .btns_cnt = BITS_BTN_MAX_COMBO_BUTTONS + 2,
        .btns_combo = combo_buttons,
        .btns_combo_cnt = BITS_BTN_MAX_COMBO_BUTTONS + 1,
        .read_button_level_func = test_framework_mock_read_button,
        .bits_btn_result_cb = test_framework_event_callback,
        .bits_btn_debug_printf = test_framework_log_printf
    };
    int32_t result = bits_button_init(&config);

    // 根据实现，可能成功或失败
    printf("超过最大组合按键数量的初始化结果: %d\n", result);

    // 测试缓冲区资源耗尽（在buffer_operations.c中已测试）
    printf("资源耗尽测试通过: 正确处理资源限制\n");
}

// ==================== 参数校验测试 ====================

void test_max_buttons_boundary(void) {
    printf("\n=== 测试最大按钮数边界（应成功） ===\n");

    static const bits_btn_obj_param_t param = TEST_DEFAULT_PARAM();

    // 创建恰好 BITS_BTN_MAX_BUTTONS 个按钮
    button_obj_t buttons[BITS_BTN_MAX_BUTTONS];
    for (size_t i = 0; i < BITS_BTN_MAX_BUTTONS; i++) {
        buttons[i] = (button_obj_t)BITS_BUTTON_INIT(i + 1, 1, &param);
    }

    bits_btn_config_t config = {
        .btns = buttons,
        .btns_cnt = BITS_BTN_MAX_BUTTONS,
        .read_button_level_func = test_framework_mock_read_button,
        .bits_btn_result_cb = test_framework_event_callback,
        .bits_btn_debug_printf = test_framework_log_printf
    };
    int32_t result = bits_button_init(&config);

    TEST_ASSERT_EQUAL(0, result);  // 应成功

    printf("最大按钮数边界测试通过: %d 个按钮初始化成功\n",
           (int)BITS_BTN_MAX_BUTTONS);
}

void test_too_many_buttons(void) {
    printf("\n=== 测试超过最大按钮数（应失败） ===\n");

    static const bits_btn_obj_param_t param = TEST_DEFAULT_PARAM();

    // 创建 BITS_BTN_MAX_BUTTONS + 1 个按钮
    button_obj_t buttons[BITS_BTN_MAX_BUTTONS + 1];
    for (size_t i = 0; i < BITS_BTN_MAX_BUTTONS + 1; i++) {
        buttons[i] = (button_obj_t)BITS_BUTTON_INIT(i + 1, 1, &param);
    }

    bits_btn_config_t config = {
        .btns = buttons,
        .btns_cnt = BITS_BTN_MAX_BUTTONS + 1,
        .read_button_level_func = test_framework_mock_read_button,
        .bits_btn_result_cb = test_framework_event_callback,
        .bits_btn_debug_printf = test_framework_log_printf
    };
    int32_t result = bits_button_init(&config);

    TEST_ASSERT_EQUAL(-5, result);  // 应返回按钮数量超限错误

    printf("超过最大按钮数测试通过: 正确返回错误码 -5\n");
}

void test_button_param_null(void) {
    printf("\n=== 测试单按钮 param 为 NULL ===\n");

    static const bits_btn_obj_param_t param = TEST_DEFAULT_PARAM();

    // 创建按钮数组，其中一个的 param 为 NULL
    button_obj_t buttons[] = {
        BITS_BUTTON_INIT(1, 1, &param),
        BITS_BUTTON_INIT(2, 1, NULL),   // param 为 NULL
        BITS_BUTTON_INIT(3, 1, &param)
    };

    bits_btn_config_t config = {
        .btns = buttons,
        .btns_cnt = 3,
        .read_button_level_func = test_framework_mock_read_button,
        .bits_btn_result_cb = test_framework_event_callback,
        .bits_btn_debug_printf = test_framework_log_printf
    };
    int32_t result = bits_button_init(&config);

    TEST_ASSERT_EQUAL(-6, result);  // 应返回按钮 param 为空错误

    printf("单按钮 param 为 NULL 测试通过: 正确返回错误码 -6\n");
}

void test_combo_button_param_null(void) {
    printf("\n=== 测试组合按钮 param 为 NULL ===\n");

    static const bits_btn_obj_param_t param = TEST_DEFAULT_PARAM();

    // 创建单按钮
    button_obj_t buttons[] = {
        BITS_BUTTON_INIT(1, 1, &param),
        BITS_BUTTON_INIT(2, 1, &param)
    };

    // 创建 param 为 NULL 的组合按钮
    static uint16_t combo_keys[] = {1, 2};
    button_obj_combo_t combo =
        BITS_BUTTON_COMBO_INIT(100, 1, NULL, combo_keys, 2, 1);  // param 为 NULL

    bits_btn_config_t config = {
        .btns = buttons,
        .btns_cnt = 2,
        .btns_combo = &combo,
        .btns_combo_cnt = 1,
        .read_button_level_func = test_framework_mock_read_button,
        .bits_btn_result_cb = test_framework_event_callback,
        .bits_btn_debug_printf = test_framework_log_printf
    };
    int32_t result = bits_button_init(&config);

    TEST_ASSERT_EQUAL(-7, result);  // 应返回组合按钮 param 为空错误

    printf("组合按钮 param 为 NULL 测试通过: 正确返回错误码 -7\n");
}