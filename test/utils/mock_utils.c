/* mock_utils.c - 模拟工具实现 */
#include "mock_utils.h"
#include "time_utils.h"
#include "../core/test_framework.h"
#include <string.h>

// ==================== 按键模拟函数实现 ====================

void mock_set_button_state(uint8_t button_id, uint8_t state) {
    if (button_id < MAX_TEST_BUTTONS) {
        g_test_framework.gpio_states[button_id] = state ? 1 : 0;
    }
}

uint8_t mock_get_button_state(uint8_t button_id) {
    if (button_id < MAX_TEST_BUTTONS) {
        return g_test_framework.gpio_states[button_id];
    }
    return 0;
}

void mock_button_press(uint8_t button_id) {
    mock_set_button_state(button_id, 1);
}

void mock_button_release(uint8_t button_id) {
    mock_set_button_state(button_id, 0);
}

void mock_button_click(uint8_t button_id, uint32_t hold_time_ms) {
    mock_button_press(button_id);
    time_simulate_debounce_delay();
    time_simulate_pass(hold_time_ms);
    mock_button_release(button_id);
    time_simulate_debounce_delay();
}

void mock_multiple_clicks(uint8_t button_id, int click_count, 
                         uint32_t hold_time_ms, uint32_t interval_ms) {
    for (int i = 0; i < click_count; i++) {
        mock_button_click(button_id, hold_time_ms);
        if (i < click_count - 1) {  // 最后一次点击后不需要间隔
            time_simulate_pass(interval_ms);
        }
    }
}

void mock_button_bounce(uint8_t button_id, int bounce_count, uint32_t bounce_interval_ms) {
    for (int i = 0; i < bounce_count; i++) {
        mock_button_press(button_id);
        time_simulate_pass(bounce_interval_ms);
        mock_button_release(button_id);
        time_simulate_pass(bounce_interval_ms);
    }
}

void mock_reset_all_buttons(void) {
    memset(g_test_framework.gpio_states, 0, sizeof(g_test_framework.gpio_states));
}