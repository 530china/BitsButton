# 测试框架技术实现文档

## 架构设计

BitsButton测试框架是一个分层架构的C语言测试系统，专门为BitsButton按键库设计。框架采用模块化设计，提供全面的测试覆盖和详细的测试报告。

### 分层结构
```
test/
├── Unity/                 # Unity测试框架
├── core/                  # 测试框架核心
│   ├── test_framework.h/c # 测试框架基础设施
│   └── test_runner.h/c    # 测试运行器和报告生成
├── utils/                 # 测试工具库
│   ├── mock_utils.h/c     # 模拟工具
│   ├── time_utils.h/c     # 时间工具
│   └── assert_utils.h/c   # 断言工具
├── cases/                 # 测试用例
│   ├── basic/             # 基础功能测试
│   ├── combo/             # 组合按键测试
│   ├── edge/              # 边界条件测试
│   └── performance/       # 性能测试
├── config/                # 测试配置
└── scripts/               # 构建脚本
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

### 基础功能测试 (7个)
1. **test_single_click_event** - 单击事件测试
2. **test_double_click_event** - 双击事件测试
3. **test_fast_double_click_event** - 快速双击测试
4. **test_long_press_event** - 长按事件测试
5. **test_long_press_hold_event** - 长按保持测试
6. **test_state_reset_functionality** - 按键状态重置功能测试
7. **test_combo_button_reset** - 组合按键状态重置测试

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


## 新增功能：低功耗状态重置

### 功能概述
为了解决低功耗模式下按键状态残留问题，新增了 `bits_button_reset_states()` 函数。该功能专门用于在系统从低功耗模式恢复时重置所有按键状态。

### 核心功能
- ✅ **完整状态重置**：重置所有单按键和组合按键的状态到空闲状态
- ✅ **物理状态同步**：自动读取当前物理按键状态并同步软件状态
- ✅ **防止虚假事件**：避免状态重置后产生意外的按键释放事件
- ✅ **缓冲区清理**：清空所有待处理的按键事件

### 使用场景
```c
void enter_low_power_mode(void) {
    stop_button_scan_timer();    // 停止按键扫描
    enter_sleep_mode();          // 进入低功耗模式
}

void exit_low_power_mode(void) {
    exit_sleep_mode();           // 退出低功耗模式
    bits_button_reset_states();  // 🔑 重置按键状态
    start_button_scan_timer();   // 恢复按键扫描
}
```

### 测试验证
- **test_state_reset_functionality**: 验证基础状态重置功能
- **test_combo_button_reset**: 专门验证组合按键状态重置功能

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
```
.\run_tests.bat clean        # Windows
# Linux/macOS不支持clean参数，直接重新运行即可
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
#define MAX_CAPTURED_EVENTS           100    // 最大捕获事件数
#define TEST_DEBOUNCE_TIME_MS         BITS_BTN_DEBOUNCE_TIME_MS       // 消抖时间 (默认40ms)
#define TEST_LONG_PRESS_TIME_MS       BITS_BTN_LONG_PRESS_START_TIME_MS // 长按开始时间 (默认1000ms)
#define TEST_TIME_WINDOW_MS           BITS_BTN_TIME_WINDOW_TIME_MS    // 多击时间窗口 (默认300ms)
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

## 用户使用参考

如需了解测试框架的基本使用方法和应用场景，请参阅主项目的用户文档：

- [测试框架使用指南](../docs/testing.md)
- [使用指南](../docs/usage_guide.md)

---
