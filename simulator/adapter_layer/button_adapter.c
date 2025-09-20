#include "button_adapter.h"
#include <stdarg.h>
#include "stdio.h"
#define BITS_BTN_DEBOUNCE_TIME_MS 60
#include "../../bits_button.h"
#define MAX_BUTTONS 10

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
    BITS_BUTTON_COMBO_INIT(USER_BUTTON_COMBO_1, 1, &defaul_param, ((uint16_t[]){USER_BUTTON_0, USER_BUTTON_1, USER_BUTTON_3}), 3, 1),
};

void bits_btn_result_cb(struct button_obj_t *btn, struct bits_btn_result result)
{
    printf("id:%d, event:%d, key_value:%d, long press period trigger cnt:%d \r\n", result.key_id, result.event, result.key_value, result.long_press_period_trigger_cnt);

    if(result.event == BTN_STATE_PRESSED)
    {
        printf("id:%d pressed \n", result.key_id);
    }

    if(result.event == BTN_STATE_RELEASE)
    {
        printf("id:%d release\n", result.key_id);
    }

    if(result.event == BTN_STATE_LONG_PRESS)
    {
        if(result.key_value == 0b11)
            printf("id:%d, long press start\n", result.key_id);
        else if(result.key_value == 0b111)
        {
            printf("id:%d, long press hold, cnt:%d\n", result.key_id, result.long_press_period_trigger_cnt);
        }
        else if(result.key_value == 0b1011)
            printf("id:%d, single click and long press start\n", result.key_id);
        else if(result.key_value == 0b101011)
            printf("id:%d, double click and long press start\n", result.key_id);
    }

    if(result.event == BTN_STATE_FINISH)
    {
        switch(result.key_value)
        {
        case BITS_BTN_SINGLE_CLICK_KV:
            printf("id:%d single click\n", result.key_id);
            break;
        case BITS_BTN_DOUBLE_CLICK_KV:
            printf("id:%d double click\n", result.key_id);
            break;
        case 0b10111010:
            printf("id:%d single click and long press then single click\n", result.key_id);
            break;
        }
    }

    // 通用的长按保持处理（不同的方式判别长按保持）
    if(result.event == BTN_STATE_LONG_PRESS && result.long_press_period_trigger_cnt > 0)
    {
        printf("[%d] long press hold, period:%d\r\n",
               result.key_id,
               result.long_press_period_trigger_cnt);
        // 长按保持处理（如连续调节音量）
    }
}

// 初始化适配器
__attribute__((constructor)) static void init_adapter() {
    // int32_t ret = bits_button_init(btns, ARRAY_SIZE(btns), btns_combo, ARRAY_SIZE(btns_combo), read_key_state, bits_btn_result_cb, my_log_printf);
    int32_t ret = bits_button_init(btns, ARRAY_SIZE(btns), btns_combo, ARRAY_SIZE(btns_combo), read_key_state, bits_btn_result_cb, NULL);
    if(ret)
    {
        printf("bits button init failed, ret:%d \r\n", ret);
    }
    else
        printf("bits button success!\n");

}

void get_bits_btn_result_from_buffer_test(void)
{
    bits_btn_result_t result = {0};
    int32_t res = bits_button_get_key_result(&result);

    if(res == true)
    {
        printf("[buffer_example]:id:%d, event:%d, key_value:%d, long press period trigger cnt:%d \r\n", result.key_id, result.event, result.key_value, result.long_press_period_trigger_cnt);
    }
}
// 导出函数实现
void BUTTON_API set_key_state(uint8_t button_id, uint8_t state) {
    if(button_id < MAX_BUTTONS) key_states[button_id] = state;
}

void BUTTON_API button_ticks_wrapper() {

    bits_button_ticks();
    get_bits_btn_result_from_buffer_test();
}
