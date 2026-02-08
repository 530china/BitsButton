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

// 5. 按键结果回调定义
void bits_btn_result_cb(struct button_obj_t *btn, struct bits_btn_result result)
{
    printf("id:%d, event:%d, key_value:%d, long press period trigger cnt:%d \r\n", result.key_id, result.event, result.key_value, result.long_press_period_trigger_cnt);

    if(result.event == BTN_EVENT_PRESSED)
    {
        printf("id:%d pressed \n", result.key_id);
    }

    if(result.event == BTN_EVENT_RELEASE)
    {
        printf("id:%d release\n", result.key_id);
    }

    if(result.event == BTN_EVENT_LONG_PRESS)
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

    if(result.event == BTN_EVENT_FINISH)
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
    if(result.event == BTN_EVENT_LONG_PRESS && result.long_press_period_trigger_cnt > 0)
    {
        printf("[%d] 长按保持 周期:%d\r\n",
               result.key_id,
               result.long_press_period_trigger_cnt);
        // 长按保持处理（如连续调节音量）
    }
}

int main()
{
    // 6. 按键初始化；
    bits_btn_config_t config = {
        .btns = btns,
        .btns_cnt = ARRAY_SIZE(btns),
        .btns_combo = btns_combo,
        .btns_combo_cnt = ARRAY_SIZE(btns_combo),
        .read_button_level_func = read_key_state,
        .bits_btn_result_cb = bits_btn_result_cb,
        .bits_btn_debug_printf = my_log_printf
    };
    int32_t ret = bits_button_init(&config);
    if(ret)
    {
        printf("bits button init failed, ret:%d \r\n", ret);
    }

    // 使定时器以5ms为间隔调用button_ticks()函数。
    // 此函数需要用户自己实现定时器调用逻辑。
    // 7. 5ms周期性调用bits_button_ticks() - 这是按键检测的核心，确保能够及时响应按键事件
    __timer_start(bits_button_ticks, 0, 5);

    while(1)
    {}
}
