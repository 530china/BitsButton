# 测试框架使用指南

了解如何使用BitsButton的测试框架来验证您的项目是否正常工作，以及如何添加新的测试用例。

> 🔍 **了解更多**：如需深入了解测试框架的技术实现细节，请参阅 [测试框架技术实现文档](../test/README.md)。



## 快速开始

### 运行测试

您可以轻松运行测试来验证BitsButton库的功能：

```bash
# Windows - 运行自动化测试用例
./run_tests.bat

# Linux/macOS - 运行自动化测试用例
./test/scripts/run_tests.sh
```

### 清理重建

```bash
./run_tests.bat clean  # Windows
./test/scripts/run_tests.sh clean  # Linux/macOS
```

## 测试覆盖范围

BitsButton测试框架提供全面的测试覆盖：

- **基础功能测试**：验证单击、双击、长按等基本按键事件
- **组合按键测试**：验证多键组合功能
- **边界条件测试**：验证极端情况下的行为
- **性能测试**：验证高负载下的性能表现
- **缓冲区测试**：验证各种缓冲区模式下的功能
- **低功耗状态重置测试**：验证新增的`bits_button_reset_states()`函数

## 添加新测试

### 测试用例结构

当您添加新功能时，需要创建相应的测试用例。测试用例位于`test/cases/`目录下，按功能分类：

- `basic/`：基础功能测试
- `combo/`：组合按键测试
- `edge/`：边界条件测试
- `performance/`：性能测试

### 编写测试用例

测试用例基于Unity测试框架编写，一个典型的测试用例如下：

```c
#include "unity.h"
#include "bits_button.h"

void test_example_functionality(void) {
    // 测试准备
    setup_test_environment();

    // 执行测试动作
    simulate_key_press(KEY_ID_1, KEY_PRESSED);
    bits_btn_ticks(DEBOUNCE_TIME_MS + 1);
    simulate_key_press(KEY_ID_1, KEY_RELEASED);

    // 验证结果
    bits_btn_result_t result;
    TEST_ASSERT_TRUE(bits_button_get_key_result(&result));
    TEST_ASSERT_EQUAL(BTN_STATE_FINISH, result.event);
    TEST_ASSERT_EQUAL(KEY_ID_1, result.key_id);

    // 清理
    cleanup_test_environment();
}
```

### 注册测试函数

在`test/test_main_new.c`中注册您的测试函数：

```c
int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_example_functionality);
    // ... 其他测试

    return UNITY_END();
}
```

## 测试配置

### 主要参数

测试配置文件位于`test/config/test_config.h`，包含以下参数：

```c
#define MAX_TEST_EVENTS 100        // 最大测试事件数
#define TEST_TIMEOUT_MS 5000       // 测试超时时间
#define DEBOUNCE_TIME_MS 20        // 消抖时间
#define LONG_PRESS_TIME_MS 1000    // 长按时间
#define DOUBLE_CLICK_TIME_MS 300   // 双击时间窗口
```

## 测试工具

### 模拟工具

测试框架提供了多种模拟工具来帮助您创建测试场景：

```c
// 模拟按键状态
simulate_key_press(key_id, state);

// 重置测试状态
reset_test_state();

// 模拟时间流逝
bits_btn_ticks(count);
```

### 断言工具

使用断言工具验证测试结果：

```c
// 验证事件和值
ASSERT_EVENT_WITH_VALUE(id, event, value);

// 验证事件不存在
ASSERT_NO_EVENT(id, event);

// 验证事件数量
ASSERT_EVENT_COUNT(id, event, count);
```



## CI/CD集成

测试框架支持持续集成：

- GitHub Actions 自动测试
- 多平台支持 (Windows/Linux/macOS)
- 多编译器验证 (GCC/Clang)
- 测试结果报告

## 故障排除

### 常见问题

1. **编译错误**：检查CMake版本和编译器配置
2. **测试失败**：查看详细日志，检查期望值是否正确
3. **性能问题**：调整测试配置中的超时参数

### 调试技巧

- 使用 `printf` 调试输出查看中间状态
- 检查事件缓冲区状态
- 验证时序参数设置
- 单独运行失败的测试用例

## 测试覆盖率

测试框架设计目标是达到100%功能覆盖率，包括：

- 所有API函数调用
- 所有事件类型的处理
- 所有错误处理路径
- 所有配置模式的验证

## 贡献指南

1. 添加新功能时，必须包含相应测试
2. 确保测试用例有清晰的注释和预期结果
3. 运行完整测试套件确保无回归
4. 更新相关文档

