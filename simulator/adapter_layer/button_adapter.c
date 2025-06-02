#include "button_adapter.h"
#include <stdarg.h>
#include "stdio.h"
#include "../../bits_button.h"
#define MAX_BUTTONS 10
// button_adapter.c
struct button_obj_t button1;
struct button_obj_t button2;

// 全局按键状态存储
static uint8_t key_states[MAX_BUTTONS] = {0};

// 替换原始GPIO读取函数
uint8_t read_key_state(struct button_obj_t *btn) {
    uint8_t button_id = btn->key_id;
    return (button_id < MAX_BUTTONS) ? key_states[button_id] : 0;
}

typedef enum
{
    USER_BUTTON_0 = 0,
    USER_BUTTON_1,
    USER_BUTTON_2,
    USER_BUTTON_3,
    USER_BUTTON_4,
    USER_BUTTON_5,
    USER_BUTTON_6,
    USER_BUTTON_7,
    USER_BUTTON_8,
    USER_BUTTON_9,
    USER_BUTTON_INVALID,
    USER_BUTTON_MAX,

    USER_BUTTON_COMBO_0 = 0x100,
    USER_BUTTON_COMBO_1,
    USER_BUTTON_COMBO_2,
    USER_BUTTON_COMBO_3,
    USER_BUTTON_COMBO_MAX,
} user_button_t;

int my_log_printf(const char* format, ...) {

    va_list args;
    va_start(args, format);
    int result = vprintf(format, args);  // 使用标准库的vprintf
    va_end(args);

    return result;
}

static const bits_btn_obj_param_t defaul_param = {.long_press_period_triger_ms = BITS_BTN_LONG_PRESS_PERIOD_TRIGER_MS,
    .long_press_start_time_ms = BITS_BTN_LONG_PRESS_START_TIME_MS,
    .short_press_time_ms = BITS_BTN_SHORT_TIME_MS,
    .time_window_time_ms = BITS_BTN_TIME_WINDOW_TIME_MS};
button_obj_t btns[] =
{
    BITS_BUTTON_INIT(USER_BUTTON_0, 1, &defaul_param),
    BITS_BUTTON_INIT(USER_BUTTON_1, 1, &defaul_param),
    BITS_BUTTON_INIT(USER_BUTTON_2, 1, &defaul_param),
    BITS_BUTTON_INIT(USER_BUTTON_3, 1, &defaul_param),
    // BITS_BUTTON_INIT(USER_BUTTON_2, 1, &defaul_param),
};

button_obj_combo_t btns_combo[] =
{
    BITS_BUTTON_COMBO_INIT(USER_BUTTON_COMBO_0, 1, &defaul_param, ((uint16_t[]){USER_BUTTON_0, USER_BUTTON_1}), 2, 1),
    BITS_BUTTON_COMBO_INIT(USER_BUTTON_COMBO_1, 1, &defaul_param, ((uint16_t[]){USER_BUTTON_2, USER_BUTTON_3}), 2, 1),
};

// 初始化适配器
__attribute__((constructor)) static void init_adapter() {
    // 替换所有button实例的读取函数
    // struct button_obj_t* btn = button_list_head;
    // while(btn) {
    //     btn->_read_button_func_ptr = read_key_state;
    //     btn = btn->next;
    // }

    bits_button_init(btns, ARRAY_SIZE(btns), btns_combo, ARRAY_SIZE(btns_combo), read_key_state, NULL, my_log_printf);
#if 0
    button_init(&button1, read_key_state, 1, 0, NULL, 0);
    button_start(&button1);

    button_init(&button2, read_key_state, 1, 1, NULL, 0);
    button_start(&button2);
#endif

}

// 导出函数实现
void BUTTON_API set_key_state(uint8_t button_id, uint8_t state) {
    if(button_id < MAX_BUTTONS) key_states[button_id] = state;
}

void BUTTON_API button_ticks_wrapper() {

    bits_button_ticks();
}
