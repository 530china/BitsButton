# BitsButton 测试框架 🧪

这个目录包含了 BitsButton 项目的完整测试框架，专为嵌入式C项目设计的自研测试系统。

## 🏗️ 目录结构

```
test/
├── core/                    # 🎯 测试框架核心
│   ├── test_framework.h     # 测试框架头文件
│   ├── test_framework.c     # 测试框架实现
│   ├── test_runner.h        # 测试运行器头文件
│   └── test_runner.c        # 测试运行器实现
├── utils/                   # 🛠️ 测试工具
│   ├── assert_utils.h       # 断言工具
│   ├── assert_utils.c       # 断言工具实现
│   ├── mock_utils.h         # 模拟工具
│   ├── mock_utils.c         # 模拟工具实现
│   ├── time_utils.h         # 时间工具
│   └── time_utils.c         # 时间工具实现
├── cases/                   # 📋 测试用例
│   ├── basic/               # 基础功能测试
│   │   ├── test_initialization.c      # 初始化测试
│   │   ├── test_single_operations.c   # 单按钮操作测试
│   │   └── test_buffer_operations.c   # 缓冲区操作测试
│   ├── combo/               # 组合按键测试
│   │   ├── test_combo_buttons.c       # 基础组合按键测试
│   │   └── test_advanced_combo.c      # 高级组合按键测试
│   ├── edge/                # 边界条件测试
│   │   ├── test_edge_cases.c          # 一般边界测试
│   │   ├── test_state_machine_edge.c  # 状态机边界测试
│   │   └── test_error_handling.c      # 错误处理测试
│   └── performance/         # 性能测试
│       └── test_performance.c         # 性能基准测试
├── config/                  # ⚙️ 测试配置
│   └── test_config.h        # 测试配置头文件
├── scripts/                 # 🚀 测试脚本
│   ├── run_tests.sh         # Linux/Mac 运行脚本
│   └── run_tests.bat        # Windows 运行脚本
├── cmake/                   # 🔧 CMake 配置
│   └── CompilerFlags.cmake  # 编译器标志配置
├── Unity/                   # 📚 Unity 测试框架
│   └── src/                 # Unity 源码
├── CMakeLists.txt          # CMake 构建配置
├── test_main_new.c         # 测试主程序
└── README.md               # 本文件
```

## ✨ 测试框架特性

### 🎯 核心功能
- **自定义测试框架**: 轻量级的 C 测试框架，专为嵌入式设计
- **测试运行器**: 自动发现和运行测试用例
- **断言工具**: 丰富的断言宏和函数
- **模拟工具**: 硬件和系统调用模拟
- **时间工具**: 时间相关的测试辅助函数

### 📊 测试覆盖统计（实际运行数据）
测试类型 | 覆盖内容 | 测试数量
--- | --- | ---
**基础功能** | 单击、双击、长按、连击 | 5个测试
**组合按键** | 基本组合、高级组合、冲突处理 | 4个测试  
**边界条件** | 超时、消抖、状态转换、时间窗口 | 8个测试
**错误处理** | 空指针、无效参数、资源耗尽 | 4个测试
**初始化配置** | 成功初始化、激活电平、自定义参数 | 5个测试
**缓冲区操作** | 溢出保护、状态跟踪、边界情况 | 3个测试
**总计** | **全面的功能覆盖** | **29个测试**

### 🚀 CI/CD 集成
- **多平台支持**: Ubuntu, Windows, macOS 自动测试
- **多编译器**: GCC + Clang 交叉验证
- **自动化运行**: 每次提交自动触发测试
- **结果上传**: 失败时自动保存调试信息

## 🚀 运行测试

### 快速开始
```bash
# 项目根目录一键测试
./run_tests.bat     # Windows（项目根目录）
```

### Windows 环境
```batch
# 方法1: 使用项目根目录脚本
run_tests.bat

# 方法2: 进入 test 目录
cd test
.\scripts\run_tests.bat

# 方法3: 使用 CMake
cd test
mkdir build && cd build
cmake -G "MinGW Makefiles" ..
mingw32-make
.\run_tests_new.exe
```

### Linux/Mac 环境
```bash
# 方法1: 进入 test 目录
cd test
./scripts/run_tests.sh

# 方法2: 使用 CMake
cd test
mkdir build && cd build
cmake ..
make -j$(nproc)
./run_tests_new
```

### 测试输出示例
```
========================================
     BitsButton 测试框架 v3.0
     分层架构 - 模块化设计
========================================

【单按键基础功能测试】
✓ test_single_click_event: PASS
✓ test_double_click_event: PASS  
✓ test_triple_click_event: PASS
✓ test_long_press_event: PASS
✓ test_long_press_hold_event: PASS

【组合按键功能测试】
✓ test_basic_combo_button: PASS

【边界条件测试】
✓ test_slow_double_click_timeout: PASS
✓ test_debounce_functionality: PASS

【性能压力测试】
✓ test_high_frequency_button_presses: PASS

【缓冲区操作测试】
✓ test_buffer_overflow_protection: PASS
✓ test_buffer_state_tracking: PASS
✓ test_buffer_edge_cases: PASS

...

========================================
           测试完成
========================================
-----------------------
29 Tests 0 Failures 0 Ignored
OK
========================================
✅ 所有测试通过！
========================================
```

## 🔧 开发指南

### 添加新测试

1. **选择合适的目录**：
   - `basic/` - 基础功能测试
   - `combo/` - 组合按键测试
   - `edge/` - 边界条件测试
   - `performance/` - 性能测试

2. **创建测试文件**：
   ```c
   /* test_my_feature.c - 我的功能测试 */
   #include "unity.h"
   #include "core/test_framework.h"
   #include "utils/mock_utils.h"
   #include "utils/time_utils.h"
   #include "utils/assert_utils.h"
   #include "config/test_config.h"
   #include "bits_button.h"
   
   void test_my_new_feature(void) {
       printf("\n=== 测试我的新功能 ===\n");
       
       // 设置测试环境
       test_framework_reset();
       mock_reset_all_buttons();
       
       // 创建按键对象
       static const bits_btn_obj_param_t param = TEST_DEFAULT_PARAM();
       button_obj_t button = BITS_BUTTON_INIT(1, 1, &param);
       bits_button_init(&button, 1, NULL, 0, 
                        test_framework_mock_read_button, 
                        test_framework_event_callback, 
                        test_framework_log_printf);
       
       // 执行测试逻辑
       mock_button_click(1, STANDARD_CLICK_TIME_MS);
       
       // 验证结果
       ASSERT_EVENT_WITH_VALUE(1, BTN_STATE_FINISH, EXPECTED_VALUE);
       printf("我的新功能测试通过\n");
   }
   ```

3. **在主程序中注册**：
   在 `test_main_new.c` 中添加：
   ```c
   // 在外部函数声明区域添加
   extern void test_my_new_feature(void);
   
   // 在 main() 函数的相应测试组中添加
   int main(void) {
       // ... 其他代码
       
       printf("\n【我的功能测试】\n");
       RUN_TEST(test_my_new_feature);
       
       // ... 其他代码
   }
   ```

4. **更新CMakeLists.txt**：
   在 `test/CMakeLists.txt` 中将新的测试文件添加到源文件列表中。

### 测试最佳实践

#### ✅ 推荐做法
- **独立性**: 每个测试都应该独立，不依赖其他测试
- **清晰命名**: 使用描述性的测试函数名
- **模拟硬件**: 使用 mock_utils 隔离硬件依赖
- **边界测试**: 测试极端值和错误条件
- **性能验证**: 关键路径添加性能测试

#### ❌ 避免做法
- 测试之间相互依赖
- 硬编码的时间延迟
- 忽略错误处理路径
- 过于复杂的测试逻辑

### 调试测试

#### 启用详细输出
```c
// 在 test_config.h 中设置
#define TEST_VERBOSE_OUTPUT 1
#define TEST_DEBUG_ENABLED 1
```

#### 单独运行特定测试
```bash
# 使用环境变量过滤测试
export TEST_FILTER="test_single_click"
./run_tests_new
```

## ⚙️ 配置选项

### test_config.h 配置
```c
// 测试超时设置
#define TEST_TIMEOUT_MS 5000

// 性能基准参数
#define PERFORMANCE_TEST_ITERATIONS 1000
#define PERFORMANCE_THRESHOLD_MS 10

// 模拟硬件配置
#define MOCK_BUTTON_COUNT 8
#define MOCK_TIMER_RESOLUTION_MS 1

// 调试选项
#define TEST_VERBOSE_OUTPUT 0
#define TEST_DEBUG_ENABLED 0
```

### CMake 配置选项
```bash
# 调试构建
cmake -DCMAKE_BUILD_TYPE=Debug ..

# 发布构建
cmake -DCMAKE_BUILD_TYPE=Release ..

# 启用代码覆盖率
cmake -DENABLE_COVERAGE=ON ..
```

## 🎯 CI/CD 集成

### GitHub Actions 配置
项目使用 `.github/workflows/stable-ci.yml` 进行自动化测试：

```yaml
# 多平台测试矩阵
strategy:
  matrix:
    os: [ubuntu-latest, windows-latest, macos-latest]
    compiler: [gcc, clang]

# 测试步骤
- name: 运行测试
  run: |
    cd test
    mkdir build && cd build
    cmake ..
    make
    ./run_tests_new
```

### 本地 CI 模拟
```bash
# 模拟 Ubuntu + GCC
export CC=gcc && cd test && mkdir build && cd build && cmake .. && make && ./run_tests_new

# 模拟 Ubuntu + Clang  
export CC=clang && cd test && mkdir build && cd build && cmake .. && make && ./run_tests_new
```

## 📈 测试覆盖率

### 生成覆盖率报告
```bash
# 启用覆盖率构建
cd test/build
cmake -DENABLE_COVERAGE=ON ..
make

# 运行测试并生成报告
./run_tests_new
lcov --capture --directory . --output-file coverage.info
genhtml coverage.info --output-directory coverage_html
```

### 覆盖率目标
- **行覆盖率**: > 90%
- **函数覆盖率**: > 95%
- **分支覆盖率**: > 85%

## 🔍 故障排除

### 常见问题

#### 编译错误
```bash
# 确保 C11 支持
gcc --version  # 需要 GCC 4.9+
clang --version  # 需要 Clang 3.1+
```

#### 测试失败
```bash
# 启用详细输出
export TEST_VERBOSE=1
./run_tests_new

# 检查特定测试
export TEST_FILTER="failing_test_name"
./run_tests_new
```

#### Windows 构建问题
```batch
# 确保使用正确的 make 命令
mingw32-make  # 而不是 make

# 检查 MSYS2 环境
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake
```

### 获取帮助
- 查看 CI 日志了解自动化测试结果
- 检查 `test/build/` 目录下的构建日志
- 在项目 Issues 中报告问题

---

## 🎉 总结

BitsButton 测试框架提供了：
- ✅ **完整的测试覆盖** - 29个测试用例覆盖所有核心功能
- ✅ **多平台支持** - Windows, Linux, macOS 全覆盖
- ✅ **CI/CD 集成** - 自动化质量保证
- ✅ **开发友好** - 简单易用的测试 API
- ✅ **高质量代码** - 严格的测试标准

通过这个测试框架，BitsButton 项目确保了代码质量和跨平台兼容性！🚀
