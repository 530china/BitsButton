# API 文档

## 函数列表

### 初始化函数

```c
int32_t bits_button_init(const bits_btn_config_t *config);
```

使用配置结构体初始化按键检测库。

**参数：**
- `config`: 指向配置结构体的指针，包含所有初始化参数

**返回值：** 
- `BITS_BTN_OK` (0): 成功
- `BITS_BTN_ERR_INVALID_COMBO_ID` (-1): 组合按键配置中存在无效的按键ID
- `BITS_BTN_ERR_INVALID_PARAM` (-2): 输入参数无效（config/btns/read_func 为 NULL 等）
- `BITS_BTN_ERR_TOO_MANY_COMBOS` (-3): 组合按键数量超过 BITS_BTN_MAX_COMBO_BUTTONS
- `BITS_BTN_ERR_BUFFER_OPS_NULL` (-4): 用户缓冲区模式需要先设置 buffer ops
- `BITS_BTN_ERR_TOO_MANY_BUTTONS` (-5): 按键数量超过 BITS_BTN_MAX_BUTTONS
- `BITS_BTN_ERR_BTN_PARAM_NULL` (-6): 单按键的 param 指针为 NULL
- `BITS_BTN_ERR_COMBO_PARAM_NULL` (-7): 组合按键的 param 指针为 NULL
- `BITS_BTN_ERR_COMBO_KEYS_INVALID` (-8): 组合按键 keys 配置无效（key_single_ids 为 NULL 或 key_count 为 0）

---

### Ticks函数

```c
void bits_button_ticks(void);
```

后台ticks函数，由定时器以5ms间隔重复调用。这是检测按键状态变化的核心函数。

---

### 获取结果函数

```c
uint8_t bits_button_get_key_result(bits_btn_result_t *result);
```

从缓冲区获取按键事件结果。

**参数：**
- `result`: 存储按键事件结果的指针

**返回值：** 1表示有事件，0表示无事件

---

### Peek功能

```c
uint8_t bits_button_peek_key_result(bits_btn_result_t *result);
```

预览按键事件但不从缓冲区移除。

**参数：**
- `result`: 存储按键事件结果的指针

**返回值：** 1表示有事件可预览，0表示无事件

---

### 状态重置函数

```c
void bits_button_reset_states(void);
```

重置所有按键状态，用于低功耗唤醒后的状态同步。

---

### 缓冲区操作函数

```c
size_t get_bits_btn_buffer_used_count(void);
```

获取缓冲区已使用数量。

```c
size_t get_bits_btn_buffer_overwrite_count(void);
```

获取缓冲区覆盖次数（当缓冲区满时）。


```c
uint8_t bits_btn_is_buffer_full(void);
```

检查环形缓冲区是否已满。

```c
uint8_t bits_btn_is_buffer_empty(void);
```

检查环形缓冲区是否为空。

```c
size_t get_bits_btn_buffer_capacity(void);
```

获取按钮结果缓冲区的总容量。

---

### 用户自定义缓冲区函数

```c
void bits_button_set_buffer_ops(const bits_btn_buffer_ops_t *user_buffer_ops);
```

设置用户自定义的缓冲区操作函数。

```c
void bits_btn_register_result_filter_callback(bits_btn_result_user_filter_callback cb);
```

注册按钮结果事件的自定义过滤回调函数。允许用户控制哪些按钮事件写入缓冲区。


## 数据结构

### 配置结构体

```c
typedef struct {
    button_obj_t *btns;                                 // 单按键对象数组
    uint16_t btns_cnt;                                  // 单按键对象数量
    button_obj_combo_t *btns_combo;                     // 组合按键对象数组
    uint16_t btns_combo_cnt;                            // 组合按键对象数量
    bits_btn_read_button_level read_button_level_func;  // 状态读取函数
    bits_btn_result_callback bits_btn_result_cb;        // 结果回调函数
    bits_btn_debug_printf_func bits_btn_debug_printf;   // 日志打印函数
} bits_btn_config_t;
```

### 按键结果结构

```c
typedef struct bits_btn_result
{
    uint8_t event;                                      // 按键事件类型
    uint16_t key_id;                                    // 触发按键ID
    uint16_t long_press_period_trigger_cnt;             // 长按周期计数
    state_bits_type_t key_value;                        // 按键值（序列位图）
} bits_btn_result_t;
```

### 按键对象结构

```c
typedef struct button_obj_t {
    uint8_t  active_level : 1;                         // 激活电平
    uint8_t current_state : 3;                         // 当前状态
    uint8_t last_state : 3;                            // 上一状态
    uint16_t  key_id;                                   // 按键ID
    uint16_t long_press_period_trigger_cnt;             // 长按周期触发计数
    uint32_t state_entry_time;                          // 状态进入时间
    state_bits_type_t state_bits;                       // 状态位图
    const bits_btn_obj_param_t *param;                  // 参数指针
} button_obj_t;

### 按键对象参数结构

```c
typedef struct bits_btn_obj_param
{
    uint16_t short_press_time_ms;                        // 短按时间(ms)
    uint16_t long_press_start_time_ms;                   // 长按开始时间(ms)
    uint16_t long_press_period_triger_ms;                // 长按周期触发时间(ms)
    uint16_t time_window_time_ms;                        // 时间窗口时间(ms)
} bits_btn_obj_param_t;
```

### 组合按键对象结构

```c
typedef struct button_obj_combo
{
    uint8_t suppress;                                   // 是否抑制成员按键事件
    uint8_t key_count;                                  // 组合中按键数量
    uint16_t *key_single_ids;                           // 成员按键ID指针
    button_mask_type_t combo_mask;                      // 组合掩码
    button_obj_t btn;                                   // 组合按键状态
} button_obj_combo_t;
```

## 事件类型

- `BTN_STATE_IDLE`: 空闲状态
- `BTN_STATE_PRESSED`: 按下
- `BTN_STATE_LONG_PRESS`: 长按
- `BTN_STATE_RELEASE`: 抬起
- `BTN_STATE_RELEASE_WINDOW`: 释放窗口
- `BTN_STATE_FINISH`: 完成

## 错误码枚举

```c
typedef enum {
    BITS_BTN_OK                       =  0,  // 成功
    BITS_BTN_ERR_INVALID_COMBO_ID     = -1,  // 组合按键中存在无效的按键ID
    BITS_BTN_ERR_INVALID_PARAM        = -2,  // 无效参数（config/btns/read_func为NULL等）
    BITS_BTN_ERR_TOO_MANY_COMBOS      = -3,  // 组合按键数量超限
    BITS_BTN_ERR_BUFFER_OPS_NULL      = -4,  // 用户缓冲区模式需要先设置buffer ops
    BITS_BTN_ERR_TOO_MANY_BUTTONS     = -5,  // 按键数量超限
    BITS_BTN_ERR_BTN_PARAM_NULL       = -6,  // 单按键param为NULL
    BITS_BTN_ERR_COMBO_PARAM_NULL     = -7,  // 组合按键param为NULL
    BITS_BTN_ERR_COMBO_KEYS_INVALID   = -8,  // 组合按键keys配置无效
} bits_btn_error_t;
```

## 使用示例

```c
// 初始化按键库
bits_btn_config_t config = {
    .btns = btns,
    .btns_cnt = BTN_CNT,
    .btns_combo = combo_btns,
    .btns_combo_cnt = COMBO_BTN_COUNT,
    .read_button_level_func = read_button_level_func,
    .bits_btn_result_cb = result_callback,
    .bits_btn_debug_printf = debug_printf_func
};
int32_t ret = bits_button_init(&config);
if(ret != 0) {
    printf("Init failed, code: %d\n", ret);
}

// 在定时器中定期调用ticks函数（通常每5ms）
bits_button_ticks();

// 处理按键事件
bits_btn_result_t result;
while(bits_button_get_key_result(&result)) {
    // 处理按键事件
    printf("Key ID: %d, Event: %d\n", result.key_id, result.event);
}

// 低功耗应用中预览事件
bits_btn_result_t preview;
if(bits_button_peek_key_result(&preview)) {
    // 根据预览的事件决定是否进一步处理
    if(preview.event == BTN_STATE_PRESSED) {
        // 消费事件
        bits_button_get_key_result(&result);
        // 实际处理
    }
}
```