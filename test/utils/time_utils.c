/* time_utils.c - 时间模拟工具实现 */
#include "time_utils.h"
#include "../core/test_framework.h"
#include "bits_button.h"

// ==================== 时间模拟函数实现 ====================

void time_simulate_pass(uint32_t ms) {
    uint32_t ticks = time_ms_to_ticks(ms);
    time_simulate_ticks(ticks);
    g_test_framework.simulated_time += ms;
}

uint32_t time_get_current(void) {
    return g_test_framework.simulated_time;
}

void time_reset(void) {
    g_test_framework.simulated_time = 0;
}

void time_simulate_debounce_delay(void) {
    time_simulate_pass(DEBOUNCE_DELAY_MS);
}

void time_simulate_standard_click(void) {
    time_simulate_pass(STANDARD_CLICK_TIME_MS);
}

void time_simulate_long_press_threshold(void) {
    time_simulate_pass(LONG_PRESS_THRESHOLD_MS);
}

void time_simulate_time_window_end(void) {
    time_simulate_pass(TIME_WINDOW_DEFAULT_MS + 10);
}

void time_simulate_ticks(uint32_t ticks) {
    for (uint32_t i = 0; i < ticks; i++) {
        bits_button_ticks();
    }
}

uint32_t time_ms_to_ticks(uint32_t ms) {
    return ms / BITS_BTN_TICKS_INTERVAL;
}

uint32_t time_ticks_to_ms(uint32_t ticks) {
    return ticks * BITS_BTN_TICKS_INTERVAL;
}