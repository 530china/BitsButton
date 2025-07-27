# BitsButton 测试框架

这个目录包含了 BitsButton 项目的完整测试框架。

## 目录结构

```
test/
├── core/                    # 测试框架核心
│   ├── test_framework.h     # 测试框架头文件
│   ├── test_framework.c     # 测试框架实现
│   ├── test_runner.h        # 测试运行器头文件
│   └── test_runner.c        # 测试运行器实现
├── utils/                   # 测试工具
│   ├── assert_utils.h       # 断言工具
│   ├── assert_utils.c       # 断言工具实现
│   ├── mock_utils.h         # 模拟工具
│   ├── mock_utils.c         # 模拟工具实现
│   ├── time_utils.h         # 时间工具
│   └── time_utils.c         # 时间工具实现
├── cases/                   # 测试用例
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
├── config/                  # 测试配置
│   └── test_config.h        # 测试配置头文件
├── scripts/                 # 测试脚本
│   ├── run_tests.sh         # Linux/Mac 运行脚本
│   └── run_tests.bat        # Windows 运行脚本
├── CMakeLists.txt          # CMake 构建配置
├── test_main_new.c         # 测试主程序
└── README.md               # 本文件
```

## 测试框架特性

### 核心功能
- **自定义测试框架**: 轻量级的 C 测试框架
- **测试运行器**: 自动发现和运行测试用例
- **断言工具**: 丰富的断言宏和函数
- **模拟工具**: 硬件和系统调用模拟
- **时间工具**: 时间相关的测试辅助函数

### 测试类型
- **基础功能测试**: 验证核心功能的正确性
- **组合按键测试**: 测试复杂的按键组合逻辑
- **边界条件测试**: 测试极端情况和错误处理
- **性能测试**: 验证系统性能指标

## 运行测试

### Windows 环境
```batch
# 运行所有测试
run_tests.bat

# 或者进入 test 目录
cd test
.\scripts\run_tests.bat
```

### Linux/Mac 环境
```bash
# 运行所有测试
./test/scripts/run_tests.sh

# 或者进入 test 目录
cd test
./scripts/run_tests.sh
```

### 使用 CMake
```bash
# 构建测试
mkdir build
cd build
cmake ..
make

# 运行测试
./test_main_new
```

## 添加新测试

1. 在相应的 `cases/` 子目录中创建测试文件
2. 包含必要的头文件：
   ```c
   #include "../core/test_framework.h"
   #include "../utils/assert_utils.h"
   #include "../../bits_button.h"
   ```
3. 实现测试函数并注册：
   ```c
   void test_my_function() {
       // 测试代码
       ASSERT_EQ(expected, actual);
   }
   
   void register_my_tests() {
       REGISTER_TEST(test_my_function);
   }
   ```
4. 在 `test_main_new.c` 中添加测试注册调用

## 测试配置

测试配置在 `config/test_config.h` 中定义，包括：
- 测试超时设置
- 性能基准参数
- 模拟硬件配置
- 调试选项

## 注意事项

- 所有测试都应该是独立的，不依赖于其他测试的执行顺序
- 使用模拟工具来隔离硬件依赖
- 性能测试应该在稳定的环境中运行
- 边界测试应该覆盖所有可能的错误情况