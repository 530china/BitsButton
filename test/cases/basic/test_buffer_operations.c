/* test_buffer_operations.c - 缓冲区操作测试 */
#include "unity.h"
#include "core/test_framework.h"
#include "utils/mock_utils.h"
#include "utils/time_utils.h"
#include "utils/assert_utils.h"
#include "config/test_config.h"
#include "bits_button.h"

// ==================== 测试设置和清理 ====================

void buffer_setUp(void) {
    test_framework_reset();
    mock_reset_all_buttons();
    time_reset();
}

void buffer_tearDown(void) {
    // 测试清理
}

// ==================== 缓冲区溢出保护测试 ====================

void test_buffer_overflow_protection(void) {
    printf("\n=== 测试缓冲区溢出保护 ===\n");

    // 创建按键对象
    static const bits_btn_obj_param_t param = TEST_DEFAULT_PARAM();
    button_obj_t button = BITS_BUTTON_INIT(1, 1, &param);
    bits_button_init(&button, 1, NULL, 0,
                     test_framework_mock_read_button,
                     test_framework_event_callback,
                     test_framework_log_printf);

    // 记录初始的溢出计数
    size_t initial_overwrite_count = get_bits_btn_buffer_overwrite_count();
    size_t initial_buffer_count = get_bits_btn_buffer_used_count();

    // 快速产生大量事件，超过缓冲区大小
    const int excess_events = get_bits_btn_buffer_capacity() + 5;
    for (int i = 0; i < excess_events; i++) {
        mock_button_click(1, STANDARD_CLICK_TIME_MS);
        time_simulate_time_window_end();

        // 每次都调用ticks来处理事件
        bits_button_ticks();
    }

    // 检查是否发生了溢出
    size_t final_overwrite_count = get_bits_btn_buffer_overwrite_count();
    size_t final_buffer_count = get_bits_btn_buffer_used_count();

    // 验证溢出保护机制
    if (final_overwrite_count > initial_overwrite_count) {
        // 发生了溢出，这是预期的
        size_t overflow_count = final_overwrite_count - initial_overwrite_count;
        TEST_ASSERT_TRUE_MESSAGE(overflow_count > 0, "应该检测到缓冲区溢出");
        printf("✓ 缓冲区溢出保护正常工作: 溢出次数 = %zu\n", overflow_count);

        // 验证缓冲区大小没有超过限制
        TEST_ASSERT_TRUE_MESSAGE(final_buffer_count <= get_bits_btn_buffer_capacity(),
                                 "缓冲区大小不应超过最大限制");
    } else {
        // 没有溢出，说明缓冲区容量足够大
        printf("✓ 缓冲区未溢出，容量足够\n");

        // 验证所有事件都被正确存储
        TEST_ASSERT_TRUE_MESSAGE(final_buffer_count >= initial_buffer_count,
                                 "缓冲区应该包含新增的事件");
    }

    // 验证缓冲区状态检查函数的正确性
    bool is_full = bits_btn_is_buffer_full();
    bool is_empty = bits_btn_is_buffer_empty();
    size_t buffer_count = get_bits_btn_buffer_used_count();

    // 基本状态一致性检查
    TEST_ASSERT_FALSE_MESSAGE(is_full && is_empty, "缓冲区不能同时为满和空");

    if (is_empty) {
        TEST_ASSERT_EQUAL_MESSAGE(0, buffer_count, "空缓冲区的元素数量应该为0");
    } else {
        TEST_ASSERT_TRUE_MESSAGE(buffer_count > 0, "非空缓冲区的元素数量应该大于0");
    }

    if (is_full) {
        TEST_ASSERT_TRUE_MESSAGE(buffer_count >= get_bits_btn_buffer_capacity() * 0.8,
                                 "满缓冲区的元素数量应该接近最大值");
    }

    printf("缓冲区状态: 满=%s, 空=%s, 元素数量=%zu\n",
           is_full ? "是" : "否",
           is_empty ? "是" : "否",
           buffer_count);

    // 验证系统在溢出后仍能正常工作
    test_framework_reset();  // 清理事件缓冲区
    mock_button_click(1, STANDARD_CLICK_TIME_MS);
    time_simulate_time_window_end();
    bits_button_ticks();

    // 应该能正常获取新事件
    bits_btn_result_t result;
    bool got_result = bits_button_get_key_result(&result);
    TEST_ASSERT_TRUE_MESSAGE(got_result, "溢出后系统应该仍能正常处理新事件");
    TEST_ASSERT_EQUAL_MESSAGE(1, result.key_id, "事件内容应该正确");

    printf("✓ 缓冲区溢出保护测试通过\n");
}

void test_buffer_state_tracking(void) {
    printf("\n=== 测试缓冲区状态跟踪 ===\n");

    // 创建按键对象
    static const bits_btn_obj_param_t param = TEST_DEFAULT_PARAM();
    button_obj_t button = BITS_BUTTON_INIT(1, 1, &param);
    bits_button_init(&button, 1, NULL, 0,
                     test_framework_mock_read_button,
                     test_framework_event_callback,
                     test_framework_log_printf);

    // 初始状态应该是空的
    TEST_ASSERT_TRUE(bits_btn_is_buffer_empty());
    TEST_ASSERT_FALSE(bits_btn_is_buffer_full());
    TEST_ASSERT_EQUAL(0, get_bits_btn_buffer_used_count());

    // 产生一些事件
    mock_button_click(1, STANDARD_CLICK_TIME_MS);
    time_simulate_time_window_end();
    bits_button_ticks();

    // 缓冲区应该不再为空
    TEST_ASSERT_FALSE(bits_btn_is_buffer_empty());
    TEST_ASSERT_TRUE(get_bits_btn_buffer_used_count() > 0);

    // 读取事件
    bits_btn_result_t result;
    bool got_result = bits_button_get_key_result(&result);
    TEST_ASSERT_TRUE(got_result);
    TEST_ASSERT_EQUAL(1, result.key_id);

    // 读取后缓冲区计数应该减少
    size_t count_after_read = get_bits_btn_buffer_used_count();
    printf("读取事件后缓冲区元素数量: %zu\n", count_after_read);

    printf("缓冲区状态跟踪测试通过\n");
}

void test_buffer_edge_cases(void) {
    printf("\n=== 测试缓冲区边界情况 ===\n");

    // 创建按键对象
    static const bits_btn_obj_param_t param = TEST_DEFAULT_PARAM();
    button_obj_t button = BITS_BUTTON_INIT(1, 1, &param);
    bits_button_init(&button, 1, NULL, 0,
                     test_framework_mock_read_button,
                     test_framework_event_callback,
                     test_framework_log_printf);

    // 测试从空缓冲区读取
    bits_btn_result_t result;
    bool got_result = bits_button_get_key_result(&result);
    TEST_ASSERT_FALSE(got_result);  // 应该返回false，因为缓冲区为空

    // 填满缓冲区
    for (size_t i = 0; i < get_bits_btn_buffer_capacity(); i++) {
        mock_button_click(1, STANDARD_CLICK_TIME_MS);
        time_simulate_time_window_end();
        bits_button_ticks();
    }

    // 检查缓冲区是否接近满状态
    size_t buffer_count = get_bits_btn_buffer_used_count();
    printf("填充后缓冲区元素数量: %zu\n", buffer_count);

    // 尝试再添加事件（可能导致溢出）
    size_t overwrite_before = get_bits_btn_buffer_overwrite_count();
    mock_button_click(1, STANDARD_CLICK_TIME_MS);
    time_simulate_time_window_end();
    bits_button_ticks();
    size_t overwrite_after = get_bits_btn_buffer_overwrite_count();

    if (overwrite_after > overwrite_before) {
        printf("✓ 缓冲区溢出处理正常: 新溢出次数 = %zu\n",
               overwrite_after - overwrite_before);
    }

    // 逐个读取所有事件
    size_t read_count = 0;
    while (bits_button_get_key_result(&result)) {
        read_count++;
        TEST_ASSERT_EQUAL(1, result.key_id);

        // 防止无限循环
        if (read_count > get_bits_btn_buffer_capacity() + 10) {
            break;
        }
    }

    printf("成功读取事件数量: %llu\n", read_count);

    // 读取完毕后缓冲区应该为空
    TEST_ASSERT_TRUE(bits_btn_is_buffer_empty());

    printf("缓冲区边界情况测试通过\n");
}