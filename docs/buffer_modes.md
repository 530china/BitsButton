# 缓冲区模式配置

BitsButton库支持三种不同的缓冲区模式，以满足不同应用场景的需求。

## 模式概述

### 1. C11原子缓冲区模式（默认）

这是默认模式，适用于多线程环境，提供高性能和线程安全的特性。

**启用方式：**
```c
// 默认情况下启用，无需特殊定义
// 示例编译命令：
gcc -c -std=c11 bits_button.c
```

**特性：**
- 使用C11原子操作实现SPSC（单生产者单消费者）环形缓冲区
- 线程安全，无锁设计
- 高性能，适用于多线程环境
- 需要C11编译器支持

### 2. 禁用缓冲区模式

此模式禁用内部缓冲区，所有事件直接通过回调函数处理。

**启用方式：**
```c
#define BITS_BTN_DISABLE_BUFFER
// 示例编译命令：
gcc -c -DBITS_BTN_DISABLE_BUFFER -std=c99 bits_button.c
```

**特性：**
- 最小内存占用
- C89/C99兼容
- 不支持轮询模式（`bits_button_get_key_result`不可用）
- 事件不能缓存，必须立即处理
- 适用于资源极度受限的环境

### 3. 用户自定义缓冲区模式

此模式允许用户实现自己的缓冲区操作函数。

**启用方式：**
```c
#define BITS_BTN_USE_USER_BUFFER
// 示例编译命令：
gcc -c -DBITS_BTN_USE_USER_BUFFER -std=c99 bits_button.c
```

**特性：**
- 完全控制缓冲区实现
- C89/C99兼容
- 需要实现完整的缓冲区接口
- 适用于有特殊缓冲区需求的应用

## 缓冲区操作接口

当使用用户自定义缓冲区模式时，需要实现以下接口：

```c
typedef struct {
    void (*init)(void);                             // 初始化
    uint8_t (*write)(bits_btn_result_t *result);    // 写入数据
    uint8_t (*read)(bits_btn_result_t *result);     // 读取数据
    uint8_t (*is_empty)(void);                     // 检查是否为空
    uint8_t (*is_full)(void);                      // 检查是否已满
    size_t (*get_buffer_used_count)(void);         // 获取已使用数量
    void (*clear)(void);                           // 清空缓冲区
    size_t (*get_buffer_overwrite_count)(void);    // 获取覆盖次数
    size_t (*get_buffer_capacity)(void);           // 获取容量
    uint8_t (*peek)(bits_btn_result_t *result);    // 预览数据（不移除）
} bits_btn_buffer_ops_t;
```

## 设置自定义缓冲区

```c
// 实现自定义缓冲区
static bits_btn_result_t my_buffer[MY_BUFFER_SIZE];
static size_t my_read_idx = 0, my_write_idx = 0;

static uint8_t my_buffer_write(bits_btn_result_t *result) {
    // 实现缓冲区写入逻辑
    if ((my_write_idx + 1) % MY_BUFFER_SIZE != my_read_idx) {
        my_buffer[my_write_idx] = *result;
        my_write_idx = (my_write_idx + 1) % MY_BUFFER_SIZE;
        return true;
    }
    return false; // 缓冲区满
}

static uint8_t my_buffer_read(bits_btn_result_t *result) {
    // 实现缓冲区读取逻辑
    if (my_read_idx != my_write_idx) {
        *result = my_buffer[my_read_idx];
        my_read_idx = (my_read_idx + 1) % MY_BUFFER_SIZE;
        return true;
    }
    return false; // 缓冲区空
}

// 定义缓冲区操作接口
static const bits_btn_buffer_ops_t my_buffer_ops = {
    .init = my_buffer_init,
    .write = my_buffer_write,
    .read = my_buffer_read,
    .is_empty = my_buffer_is_empty,
    .is_full = my_buffer_is_full,
    .get_buffer_used_count = my_buffer_get_used_count,
    .clear = my_buffer_clear,
    .get_buffer_overwrite_count = my_buffer_get_overwrite_count,
    .get_buffer_capacity = my_buffer_get_capacity,
};

// 设置自定义缓冲区
void setup_custom_buffer(void) {
    bits_button_set_buffer_ops(&my_buffer_ops);
}
```

## 如何选择模式

**选择流程图：**

1. **支持C11且有多线程？** → 选择C11原子缓冲区模式（默认）
2. **资源极度受限？** → 选择禁用缓冲区模式
3. **需要特殊缓冲区实现？** → 选择用户自定义缓冲区模式

## 模式对比

| 特性 | C11原子模式 | 禁用缓冲区模式 | 用户自定义模式 |
|------|-------------|----------------|----------------|
| 内存占用 | 中等 | 最小 | 可控 |
| 性能 | 高 | 高 | 取决于实现 |
| 线程安全 | 是 | N/A | 取决于实现 |
| 轮询支持 | 是 | 否 | 是 |
| 回调支持 | 是 | 是 | 是 |
| 编译器要求 | C11 | C89+ | C89+ |
| 复杂度 | 低 | 低 | 高 |

## 注意事项

1. 禁用缓冲区模式下，`bits_button_get_key_result()` 函数将不可用
2. 用户自定义模式需要谨慎实现线程安全性
3. C11模式提供最佳的性能和安全性平衡
4. 在资源受限环境中，禁用缓冲区模式可能是最佳选择