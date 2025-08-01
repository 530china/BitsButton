# BitsButton 测试框架 v3.0

## 概述

BitsButton测试框架是一个分层架构的C语言测试系统，专门为BitsButton按键库设计。框架采用模块化设计，提供全面的测试覆盖和详细的测试报告。

## 测试结果

✅ **所有测试通过！** (35/35)
- 测试成功率：100%
- 覆盖范围：完整的功能、边界、性能和错误处理测试

## 架构设计

### 分层结构
```
test/
├── core/           # 核心测试框架
├── utils/          # 测试工具库
├── cases/          # 测试用例
│   ├── basic/      # 基础功能测试
│   ├── combo/      # 组合按键测试
│   ├── edge/       # 边界条件测试
│   └── performance/# 性能测试
├── config/         # 测试配置
└── scripts/        # 构建脚本
```

### 核心组件

#### 1. 测试框架核心 (core/)
- **test_framework.h/c**: 测试框架基础设施
- **test_runner.h/c**: 测试运行器和报告生成

#### 2. 测试工具库 (utils/)
- **mock_utils.h/c**: 模拟工具，提供按键状态模拟
- **time_utils.h/c**: 时间工具，处理时序测试
- **assert_utils.h/c**: 断言工具，提供丰富的验证功能

#### 3. 测试用例 (cases/)
- **basic/**: 基础功能测试（单击、双击、长按等）
- **combo/**: 组合按键测试（多键组合、冲突处理）
- **edge/**: 边界条件测试（超时、消抖、状态机边界）
- **performance/**: 性能测试（高频处理、并发、稳定性）

## 测试用例详情

### 基础功能测试 (5个)
1. **test_single_click_event** - 单击事件测试
2. **test_double_click_event** - 双击事件测试
3. **test_triple_click_event** - 快速双击测试
4. **test_long_press_event** - 长按事件测试
5. **test_long_press_hold_event** - 长按保持测试

### 组合按键测试 (2个)
1. **test_basic_combo_button** - 基本组合按键测试
2. **test_combo_long_press** - 组合按键长按测试

### 边界条件测试 (5个)
1. **test_slow_double_click_timeout** - 超时双击测试
2. **test_debounce_functionality** - 消抖功能测试
3. **test_very_short_press** - 极短按键测试
4. **test_long_press_boundary** - 长按边界测试
5. **test_rapid_clicks_boundary** - 快速连击边界测试

### 性能测试 (4个)
1. **test_high_frequency_button_presses** - 高频按键处理测试
2. **test_multiple_buttons_concurrent** - 多按键并发处理测试
3. **test_long_running_stability** - 长时间运行稳定性测试
4. **test_memory_usage** - 内存使用测试

### 缓冲区测试 (3个)
1. **test_buffer_overflow_protection** - 缓冲区溢出保护测试
2. **test_buffer_state_tracking** - 缓冲区状态跟踪测试
3. **test_buffer_edge_cases** - 缓冲区边界情况测试

### 高级组合测试 (3个)
1. **test_advanced_three_key_combo** - 三键组合测试
2. **test_combo_with_different_timing** - 不同时序组合测试
3. **test_multiple_combos_conflict** - 多组合键冲突测试

### 状态机测试 (4个)
1. **test_state_transition_timing** - 状态转换时序测试
2. **test_time_window_boundary** - 时间窗口边界测试
3. **test_long_press_period_boundary** - 长按周期边界测试
4. **test_rapid_state_changes** - 快速状态变化测试

### 错误处理测试 (4个)
1. **test_null_pointer_handling** - 空指针处理测试
2. **test_invalid_parameters** - 无效参数测试
3. **test_boundary_values** - 边界值测试
4. **test_resource_exhaustion** - 资源耗尽测试

### 初始化测试 (5个)
1. **test_successful_initialization** - 成功初始化测试
2. **test_different_active_levels** - 不同激活电平测试
3. **test_custom_parameters** - 自定义参数测试
4. **test_multiple_button_initialization** - 多按键初始化测试
5. **test_callback_functions** - 回调函数测试

## 按键值编码规律

BitsButton库使用位模式编码按键值：
- **单击**: `0b010` (0x2)
- **双击**: `0b01010` (0xA) 
- **三连击**: `0b101010` (0x2A)
- **四连击**: `0b10101010` (0xAA)
- **五连击**: `0b1010101010` (0x2AA)
- **长按**: `0b011` (0x3)

编码规律：N连击 = (N-1) × `010` + `10`

## 快速开始

### 运行所有测试
```bash
# Windows
.\run_tests.bat

# Linux/macOS  
./test/scripts/run_tests.sh
```

### 清理重建
```bash
.\run_tests.bat clean
```

### 手动构建
```bash
cd test
mkdir build
cd build
cmake ..
make
./run_tests_new
```

## 配置说明

### 测试配置 (test/config/test_config.h)
```c
#define MAX_TEST_EVENTS 100        // 最大测试事件数
#define TEST_TIMEOUT_MS 5000       // 测试超时时间
#define DEBOUNCE_TIME_MS 20        // 消抖时间
#define LONG_PRESS_TIME_MS 1000    // 长按时间
#define DOUBLE_CLICK_TIME_MS 300   // 双击时间窗口
```

## 测试工具

### 模拟工具
- `simulate_key_press(id, state)` - 模拟按键状态
- `reset_test_state()` - 重置测试状态
- `bits_btn_ticks(count)` - 模拟时间流逝

### 断言工具
- `ASSERT_EVENT_WITH_VALUE(id, event, value)` - 验证事件和值
- `ASSERT_NO_EVENT(id, event)` - 验证事件不存在
- `ASSERT_EVENT_COUNT(id, event, count)` - 验证事件数量

## 持续集成

测试框架支持多种CI环境：
- GitHub Actions
- 本地开发环境
- Windows/Linux/macOS跨平台

## 贡献指南

1. 添加新测试用例时，请遵循现有的命名规范
2. 确保测试用例有清晰的注释和预期结果
3. 运行完整测试套件确保无回归
4. 更新相关文档

## 故障排除

### 常见问题
1. **编译错误**: 检查CMake版本和编译器配置
2. **测试失败**: 查看详细日志，检查期望值是否正确
3. **性能问题**: 调整测试配置中的超时参数

### 调试技巧
- 使用 `printf` 调试输出查看中间状态
- 检查事件缓冲区状态
- 验证时序参数设置

---
