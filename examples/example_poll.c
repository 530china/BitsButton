// 1. 包含头文件
#include "bits_button.h"

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

// 2. 定义按键参数、单按键实例、组合按键实例
static const bits_btn_obj_param_t defaul_param = {.long_press_period_triger_ms = BITS_BTN_LONG_PRESS_PERIOD_TRIGER_MS,
                                                  .long_press_start_time_ms = BITS_BTN_LONG_PRESS_START_TIME_MS,
                                                  .short_press_time_ms = BITS_BTN_SHORT_TIME_MS,
                                                  .time_window_time_ms = BITS_BTN_TIME_WINDOW_TIME_MS};
button_obj_t btns[] =
{
    BITS_BUTTON_INIT(USER_BUTTON_0, 1, &defaul_param),
    BITS_BUTTON_INIT(USER_BUTTON_1, 1, &defaul_param),
    // BITS_BUTTON_INIT(USER_BUTTON_2, 1, &defaul_param),
};

button_obj_combo_t btns_combo[] =
{
    BITS_BUTTON_COMBO_INIT(
        USER_BUTTON_COMBO_0,    // 组合键ID
        1,                      // 有效电平
        &defaul_param,          // 参数配置
        ((uint16_t[]){USER_BUTTON_0, USER_BUTTON_1}),   // 组合按键成员
        2,                      // 组合键成员数量
        1),                     // 抑制单键事件
};

// 3. 读取按键状态函数
uint8_t read_key_state(struct button_obj_t *btn)
{
    uint8_t _id = btn->key_id;
    // you can share the GPIO read function with multiple Buttons
    switch(_id)
    {
        case USER_BUTTON_0:
            return get_button1_value(); //Require self implementation
            break;
        case USER_BUTTON_1:
            return get_button2_value(); //Require self implementation
            break;
        default:
            return 0;
            break;
    }

    return 0;
}

// 4. 日志函数（可选）
int my_log_printf(const char* format, ...) {

    va_list args;
    va_start(args, format);
    int result = vprintf(format, args);
    va_end(args);

    return result;
}

int main()
{
    // 5. 按键初始化；
    bits_button_init(btns, ARRAY_SIZE(btns), btns_combo, ARRAY_SIZE(btns_combo), read_key_state, NULL, my_log_printf);

    //make the timer invoking the button_ticks() interval 5ms.
    //This function is implemented by yourself.
    // 6. 5ms周期性调用bits_button_ticks()
    __timer_start(bits_button_ticks, 0, 5);

    while(1)
    {
        bits_btn_result_t result = {0};
        int32_t res = bits_button_get_key_result(&result);

        if(res == true)
        {
            printf("id:%d, event:%d, key_value:%d, long press period trigger cnt:%d \r\n", result.key_id, result.event, result.key_value, result.long_press_period_trigger_cnt);
            // 可在此处进行按键结果处理，可参考example_callback.c中的bits_btn_result_cb()函数；
        }
    }
}
