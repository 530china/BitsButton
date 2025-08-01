cmake_minimum_required(VERSION 3.10)
project(BitsButtonTests)

# 设置C标准
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 包含编译器配置
include(cmake/CompilerFlags.cmake)

# 添加头文件路径
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../
    ${CMAKE_CURRENT_SOURCE_DIR}/
    ${CMAKE_CURRENT_SOURCE_DIR}/Unity/src
    ${CMAKE_CURRENT_SOURCE_DIR}/core
    ${CMAKE_CURRENT_SOURCE_DIR}/utils
    ${CMAKE_CURRENT_SOURCE_DIR}/config
)

# 收集所有源文件
set(TEST_SOURCES
    # 核心框架
    core/test_framework.c
    core/test_runner.c
    
    # 工具库
    utils/mock_utils.c
    utils/time_utils.c
    utils/assert_utils.c
    
    # 测试用例 - 基础功能
    cases/basic/test_single_operations.c
    cases/basic/test_buffer_operations.c
    cases/basic/test_initialization.c
    cases/basic/test_state_reset.c
    
    # 测试用例 - 组合按键
    cases/combo/test_combo_buttons.c
    cases/combo/test_advanced_combo.c
    
    # 测试用例 - 边界测试
    cases/edge/test_edge_cases.c
    cases/edge/test_state_machine_edge.c
    cases/edge/test_error_handling.c
    
    # 测试用例 - 性能测试
    cases/performance/test_performance.c
    
    # Unity测试框架
    Unity/src/unity.c
    
    # 被测试的源文件
    ${CMAKE_CURRENT_SOURCE_DIR}/../bits_button.c
)

# 创建新架构的测试可执行文件
add_executable(run_tests_new
    test_main_new.c
    ${TEST_SOURCES}
)

# 设置编译选项
target_compile_options(run_tests_new PRIVATE
    -Wall
    -Wextra
    -Wno-unused-parameter
    -DTEST_NEW_ARCHITECTURE=1
)

# 添加测试目标
enable_testing()

# 新架构测试
add_test(NAME BitsButtonTestsNew COMMAND run_tests_new)

# 设置测试属性
set_tests_properties(BitsButtonTestsNew PROPERTIES
    TIMEOUT 300
    LABELS "new_architecture;full_test"
)

# 显示构建信息
message(STATUS "BitsButton 测试框架 v3.0 - 分层架构")
message(STATUS "测试源文件: ${TEST_SOURCES}")
message(STATUS "构建目标: run_tests_new")