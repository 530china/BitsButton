# 使用指南

BitsButton是一个轻量级、高性能的按键检测库，支持单按键和组合按键检测，提供丰富的事件类型和灵活的配置选项。本指南将结合实际示例代码，帮助您快速上手使用该库。

## 库的主要特性

- **高级按键检测**：支持单击、双击、长按、连击及复杂序列
- **组合键支持**：多键组合检测
- **灵活缓冲区**：支持C11原子缓冲、用户自定义和禁用缓冲模式
- **低功耗预览**：peek功能，预览事件而不消费
- **C标准兼容**：支持C89/C99/C11
- **轻量化**：RAM占用少于20字节/按键

## 快速开始

### 1. 集成库文件

将以下文件复制到您的项目中：

- `bits_button.h`
- `bits_button.c`

### 2. 两种使用方式

本库提供两种主要的事件处理方式：

#### 方式一：回调模式（Callback Mode）

这种方式通过回调函数自动处理按键事件，适用于事件处理逻辑相对简单的场景。

**完整示例代码（参考 [example_callback.c](../examples/example_callback.c)）：**

```c
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
        printf("[%d] 长按保持 周期:%d\r\n",
               result.key_id,
               result.long_press_period_trigger_cnt);
        // 长按保持处理（如连续调节音量）
    }
}

int main()
{
    // 6. 按键初始化；
    int32_t ret = bits_button_init(btns, ARRAY_SIZE(btns), btns_combo, ARRAY_SIZE(btns_combo), read_key_state, bits_btn_result_cb, my_log_printf);
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
```

#### 方式二：轮询模式（Polling Mode）

这种方式通过主动查询获取按键事件，适用于需要精确控制事件处理时机的场景。

**完整示例代码（参考 [example_poll.c](../examples/example_poll.c)）：**

```c
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
    int32_t ret = bits_button_init(btns, ARRAY_SIZE(btns), btns_combo, ARRAY_SIZE(btns_combo), read_key_state, NULL, my_log_printf);
    if(ret)
    {
        printf("bits button init failed, ret:%d \r\n", ret);
    }

    // 使定时器以5ms为间隔调用button_ticks()函数。
    // 此函数需要用户自己实现定时器调用逻辑。
    // 6. 5ms周期性调用bits_button_ticks() - 这是按键检测的核心，确保能够及时响应按键事件
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
```

## 关键概念和API

### 按键初始化

使用`bits_button_init()`函数初始化按键系统，需要提供单按键数组、组合按键数组、状态读取函数、结果回调函数和日志函数。

### 事件类型

- `BTN_STATE_PRESSED`：按键按下
- `BTN_STATE_RELEASE`：按键释放
- `BTN_STATE_LONG_PRESS`：长按事件
- `BTN_STATE_FINISH`：按键动作完成

### 组合按键

可以通过`BITS_BUTTON_COMBO_INIT`宏定义组合按键，实现多个按键同时按下的功能。

## 低功耗应用

对于低功耗应用，本库提供了peek功能，允许您预览事件而不从缓冲区移除它们，这对于判断是否需要唤醒系统非常有用。

```c
void low_power_handler(void) {
    // 系统被按键中断唤醒
    // 先预览事件类型，决定是否完全处理
    bits_btn_result_t preview;
    if(bits_button_peek_key_result(&preview)) {
        if(preview.event == BTN_STATE_LONG_PRESS && preview.key_id == POWER_KEY_ID) {
            // 电源键长按，执行关机
            bits_btn_result_t actual;
            bits_button_get_key_result(&actual);  // 消费事件
            system_shutdown();
        } else if(preview.event == BTN_STATE_PRESSED && preview.key_id == WAKEUP_KEY_ID) {
            // 唤醒键单击，恢复正常模式
            bits_btn_result_t actual;
            bits_button_get_key_result(&actual);  // 消费事件
            exit_low_power_mode();
        } else {
            // 其他事件，可能需要进一步处理
            // 或者继续保持低功耗模式
        }
    }
}
```

## 配置选项

### 缓冲区模式

根据您的应用需求，可以选择不同的缓冲区模式：

- **C11原子模式（默认）**：线程安全，适用于多线程环境
- **禁用缓冲区模式**：资源受限环境
- **用户自定义缓冲区**：灵活定制

### 时间参数

可以通过`bits_btn_obj_param_t`结构体配置按键检测的时间参数，如消抖时间、短按时间、长按时间等。

## 故障排除

### 常见问题

1. **按键事件不触发**：
   - 检查`read_key_state`函数是否正确读取按键状态
   - 确认`active_level`配置是否与硬件匹配
   - 确保定时器正确调用`bits_button_ticks()`函数

2. **事件识别错误**：
   - 检查时间参数是否合适
   - 确认定时器调用频率是否稳定

3. **组合按键不工作**：
   - 检查组成按键的ID是否正确
   - 确认组合按键配置是否正确

## 最佳实践

1. **定时器设置**：建议使用5ms间隔调用`bits_button_ticks()`函数
2. **按键ID**：使用有意义的枚举值，避免冲突
3. **激活电平**：根据硬件电路确定（上拉电阻用1，下拉电阻用0）
4. **参数调优**：根据实际用户体验调整时间参数
5. **错误处理**：在初始化函数调用后检查返回值