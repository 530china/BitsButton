/* assert_utils.c - 断言增强工具实现 */
#include "assert_utils.h"
#include "../core/test_framework.h"
#include <stdio.h>
#include <string.h>

// ==================== 事件查找和验证函数实现 ====================

bits_btn_result_t* assert_find_event(uint16_t key_id, uint8_t event_type) {
    bits_btn_result_t* events = test_framework_get_events();
    int event_count = test_framework_get_event_count();
    
    for (int i = 0; i < event_count; i++) {
        if (events[i].key_id == key_id && events[i].event == event_type) {
            return &events[i];
        }
    }
    return NULL;
}

bits_btn_result_t* assert_find_last_event(uint16_t key_id, uint8_t event_type) {
    bits_btn_result_t* events = test_framework_get_events();
    int event_count = test_framework_get_event_count();
    
    for (int i = event_count - 1; i >= 0; i--) {
        if (events[i].key_id == key_id && events[i].event == event_type) {
            return &events[i];
        }
    }
    return NULL;
}

int assert_count_events(uint16_t key_id, uint8_t event_type) {
    bits_btn_result_t* events = test_framework_get_events();
    int event_count = test_framework_get_event_count();
    int count = 0;
    
    for (int i = 0; i < event_count; i++) {
        if (events[i].key_id == key_id && events[i].event == event_type) {
            count++;
        }
    }
    return count;
}

// ==================== 增强断言函数实现 ====================

void assert_event_exists_impl(uint16_t key_id, uint8_t event_type, int line) {
    bits_btn_result_t* event = assert_find_event(key_id, event_type);
    if (event == NULL) {
        printf("断言失败 (行 %d): 未找到事件 - 按键ID=%d, 事件类型=%d\n", 
               line, key_id, event_type);
        assert_print_all_events();
        TEST_FAIL_MESSAGE("事件不存在");
    } else {
        printf("✓ 事件验证通过: 按键ID=%d, 事件=%d, 按键值=0x%X\n", 
               event->key_id, event->event, event->key_value);
    }
}

void assert_event_with_value_impl(uint16_t key_id, uint8_t event_type, 
                                  uint32_t expected_key_value, int line) {
    bits_btn_result_t* event = assert_find_event(key_id, event_type);
    if (event == NULL) {
        printf("断言失败 (行 %d): 未找到事件 - 按键ID=%d, 事件类型=%d\n", 
               line, key_id, event_type);
        assert_print_all_events();
        TEST_FAIL_MESSAGE("事件不存在");
        return;
    }
    
    if (event->key_value != expected_key_value) {
        printf("断言失败 (行 %d): 按键值不匹配\n", line);
        printf("  期望值: 0x%X (", expected_key_value);
        assert_print_key_value_binary(expected_key_value);
        printf(")\n");
        printf("  实际值: 0x%X (", event->key_value);
        assert_print_key_value_binary(event->key_value);
        printf(")\n");
        assert_print_all_events();
        TEST_FAIL_MESSAGE("按键值不匹配");
    } else {
        printf("✓ 事件验证通过: 按键ID=%d, 事件=%d, 按键值=0x%X\n", 
               event->key_id, event->event, event->key_value);
    }
}

void assert_long_press_count_impl(uint16_t key_id, uint16_t expected_count, int line) {
    bits_btn_result_t* events = test_framework_get_events();
    int event_count = test_framework_get_event_count();
    int hold_events = 0;
    
    for (int i = 0; i < event_count; i++) {
        if (events[i].key_id == key_id && 
            events[i].event == BTN_EVENT_LONG_PRESS && 
            events[i].long_press_period_trigger_cnt > 0) {
            hold_events++;
        }
    }
    
    if (hold_events != expected_count) {
        printf("断言失败 (行 %d): 长按保持次数不匹配\n", line);
        printf("  期望次数: %d\n", expected_count);
        printf("  实际次数: %d\n", hold_events);
        assert_print_key_events(key_id);
        TEST_FAIL_MESSAGE("长按保持次数不匹配");
    } else {
        printf("✓ 长按保持验证通过: 按键ID=%d, 保持次数=%d\n", key_id, hold_events);
    }
}

void assert_combo_suppression_impl(uint16_t combo_key_id, uint16_t* suppressed_key_ids, 
                                   int count, int line) {
    // 验证组合按键事件存在
    bits_btn_result_t* combo_event = assert_find_event(combo_key_id, BTN_EVENT_FINISH);
    if (combo_event == NULL) {
        printf("断言失败 (行 %d): 组合按键事件不存在 - ID=%d\n", line, combo_key_id);
        TEST_FAIL_MESSAGE("组合按键事件不存在");
        return;
    }
    
    // 验证单键事件被抑制
    for (int i = 0; i < count; i++) {
        bits_btn_result_t* single_event = assert_find_event(suppressed_key_ids[i], BTN_EVENT_FINISH);
        if (single_event != NULL) {
            printf("断言失败 (行 %d): 单键事件未被抑制 - ID=%d\n", line, suppressed_key_ids[i]);
            TEST_FAIL_MESSAGE("单键事件未被抑制");
            return;
        }
    }
    
    printf("✓ 组合按键抑制验证通过: 组合键ID=%d, 抑制了%d个单键\n", combo_key_id, count);
}

void assert_event_not_exists_impl(uint16_t key_id, uint8_t event_type, int line) {
    bits_btn_result_t* event = assert_find_event(key_id, event_type);
    if (event != NULL) {
        printf("断言失败 (行 %d): 事件不应该存在 - 按键ID=%d, 事件类型=%d\n", 
               line, key_id, event_type);
        assert_print_all_events();
        TEST_FAIL_MESSAGE("事件不应该存在");
    } else {
        printf("✓ 事件不存在验证通过: 按键ID=%d, 事件类型=%d\n", key_id, event_type);
    }
}

void assert_event_count_impl(uint16_t key_id, uint8_t event_type, 
                            int expected_count, int line) {
    int actual_count = assert_count_events(key_id, event_type);
    if (actual_count != expected_count) {
        printf("断言失败 (行 %d): 事件数量不匹配\n", line);
        printf("  按键ID: %d, 事件类型: %d\n", key_id, event_type);
        printf("  期望数量: %d\n", expected_count);
        printf("  实际数量: %d\n", actual_count);
        assert_print_key_events(key_id);
        TEST_FAIL_MESSAGE("事件数量不匹配");
    } else {
        printf("✓ 事件数量验证通过: 按键ID=%d, 事件类型=%d, 数量=%d\n", 
               key_id, event_type, actual_count);
    }
}

// ==================== 调试和日志函数实现 ====================

void assert_print_all_events(void) {
    bits_btn_result_t* events = test_framework_get_events();
    int event_count = test_framework_get_event_count();
    
    printf("\n=== 所有捕获的事件 (%d个) ===\n", event_count);
    for (int i = 0; i < event_count; i++) {
        printf("[%d] 按键ID:%d, 事件:%d, 按键值:0x%X, 长按计数:%d\n", 
               i, events[i].key_id, events[i].event, 
               events[i].key_value, events[i].long_press_period_trigger_cnt);
    }
    printf("========================\n");
}

void assert_print_key_events(uint16_t key_id) {
    bits_btn_result_t* events = test_framework_get_events();
    int event_count = test_framework_get_event_count();
    int key_event_count = 0;
    
    printf("\n=== 按键ID %d 的事件 ===\n", key_id);
    for (int i = 0; i < event_count; i++) {
        if (events[i].key_id == key_id) {
            printf("[%d] 事件:%d, 按键值:0x%X, 长按计数:%d\n", 
                   key_event_count++, events[i].event, 
                   events[i].key_value, events[i].long_press_period_trigger_cnt);
        }
    }
    if (key_event_count == 0) {
        printf("(无事件)\n");
    }
    printf("==================\n");
}

void assert_print_key_value_binary(uint32_t key_value) {
    printf("0b");
    for (int i = 31; i >= 0; i--) {
        if (key_value & (1U << i)) {
            printf("1");
        } else if (i < 10) {  // 只显示低10位
            printf("0");
        }
    }
    printf(" (0x%X)", key_value);
}