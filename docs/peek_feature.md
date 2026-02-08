# 低功耗事件预览功能

BitsButton库引入了peek功能，允许应用程序在不消费事件的情况下预览缓冲区中的按键事件，特别适用于低功耗唤醒场景。

## 功能概述

### 什么是peek功能

peek功能允许应用程序查看缓冲区中的下一个事件，但不将其从缓冲区中移除。这对于需要预览事件类型以决定后续处理策略的场景非常有用。

### 应用场景

1. **低功耗唤醒**：系统被按键事件唤醒，但需要先预览事件类型再决定是否完全处理
2. **事件过滤**：根据事件类型决定是否处理特定事件
3. **事件优先级**：根据事件重要性决定处理顺序

## API接口

### peek函数

```c
uint8_t bits_button_peek_key_result(bits_btn_result_t *result);
```

预览按键事件但不从缓冲区移除。

**参数：**
- `result`: 存储按键事件结果的指针

**返回值：** 
- `true` (1) 表示有事件可预览
- `false` (0) 表示无事件

### 对比get函数

```c
// get函数 - 获取并移除事件
uint8_t bits_button_get_key_result(bits_btn_result_t *result);

// peek函数 - 预览但不移除事件
uint8_t bits_button_peek_key_result(bits_btn_result_t *result);
```

## 使用示例

### 低功耗唤醒场景

```c
// 系统被按键事件唤醒，先预览事件
bits_btn_result_t preview_result;
if (bits_button_peek_key_result(&preview_result)) {
    // 根据事件类型决定是否深度处理
    if (preview_result.event == BTN_EVENT_PRESSED) {
        // 特定事件才消费并处理
        bits_btn_result_t actual_result;
        if (bits_button_get_key_result(&actual_result)) {
            // 实际处理事件
            handle_single_click(&actual_result);
        }
    } else if (preview_result.event == BTN_EVENT_LONG_PRESS) {
        // 长按开始事件，可能需要不同处理
        bits_btn_result_t actual_result;
        if (bits_button_get_key_result(&actual_result)) {
            handle_long_press_start(&actual_result);
        }
    } else {
        // 其他事件可能不需要立即处理
        // 可以稍后再处理或忽略
    }
}
```

### 事件过滤示例

```c
// 只处理特定按键的事件
bits_btn_result_t preview;
if (bits_button_peek_key_result(&preview)) {
    if (preview.key_id == POWER_BUTTON_ID) {
        // 处理电源键事件
        bits_btn_result_t result;
        if (bits_button_get_key_result(&result)) {
            handle_power_button(&result);
        }
    } else if (preview.key_id == VOLUME_BUTTON_ID) {
        // 处理音量键事件
        bits_btn_result_t result;
        if (bits_button_get_key_result(&result)) {
            handle_volume_button(&result);
        }
    }
    // 其他按键事件暂时不处理
}
```

### 优先级处理示例

```c
// 高优先级事件优先处理
bits_btn_result_t preview;
if (bits_button_peek_key_result(&preview)) {
    if (is_high_priority_event(&preview)) {
        // 立即处理高优先级事件
        bits_btn_result_t result;
        if (bits_button_get_key_result(&result)) {
            handle_high_priority_event(&result);
        }
    } else {
        // 检查是否有更高优先级的事件等待处理
        // 如果有，则延迟处理当前事件
    }
}
```

## 缓冲区模式支持

### 支持情况

| 缓冲区模式 | peek支持 | 说明 |
|------------|----------|------|
| C11原子模式 | ✅ | 完全支持 |
| 用户自定义模式 | ⚠️ | 需要实现peek接口 |
| 禁用缓冲区模式 | ❌ | 不可用 |

### 用户自定义缓冲区的peek实现

当使用用户自定义缓冲区模式时，需要在缓冲区操作接口中实现peek功能：

```c
// 自定义缓冲区结构
typedef struct {
    bits_btn_result_t buffer[BUFFER_SIZE];
    size_t read_idx;
    size_t write_idx;
} custom_buffer_t;

static uint8_t custom_buffer_peek(bits_btn_result_t *result) {
    if (custom_buffer.read_idx != custom_buffer.write_idx) {
        *result = custom_buffer.buffer[custom_buffer.read_idx];
        return true;
    }
    return false; // 缓冲区空
}

// 在缓冲区操作结构中包含peek函数指针
typedef struct {
    // ... 其他函数指针
    uint8_t (*peek)(bits_btn_result_t *result);  // peek功能
} extended_bits_btn_buffer_ops_t;
```

## 注意事项

1. **性能考虑**：频繁使用peek可能影响性能，因为它会多次访问缓冲区
2. **事件一致性**：两次调用之间事件可能发生变化（如果有其他事件加入）
3. **缓冲区模式**：在禁用缓冲区模式下，peek功能不可用
4. **线程安全**：在多线程环境中使用时需注意线程安全

## 最佳实践

1. **低功耗应用**：在低功耗唤醒后首先使用peek预览事件类型
2. **事件过滤**：使用peek实现高效的事件过滤机制
3. **状态检查**：在执行关键操作前检查是否有待处理事件
4. **调试辅助**：使用peek进行非侵入式调试和状态监控

## 与其他功能的关系

- **与状态重置**：peek操作不影响按键状态重置功能
- **与缓冲区管理**：peek不改变缓冲区的读写索引
- **与回调模式**：peek功能在轮询模式下使用