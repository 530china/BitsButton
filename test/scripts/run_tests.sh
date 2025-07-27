#!/bin/bash

# 设置UTF-8编码
export LANG=zh_CN.UTF-8

# 获取脚本所在目录的绝对路径
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# 获取test目录路径（脚本目录的上级目录）
TEST_DIR="$(dirname "$SCRIPT_DIR")"
# 保存当前目录
ORIGINAL_DIR="$(pwd)"

echo "========================================"
echo "     BitsButton 测试框架 v3.0"
echo "     分层架构 - 模块化设计"
echo "========================================"
echo

# 设置测试目标
TEST_TARGET="run_tests_new"
TEST_DESCRIPTION="分层架构测试"

echo "运行模式: $TEST_DESCRIPTION"
echo "测试目录: $TEST_DIR"
echo

# 切换到test目录
cd "$TEST_DIR"

# 创建构建目录
if [ ! -d "build" ]; then
    echo "创建构建目录..."
    mkdir build
fi
cd build

# 清除CMake缓存（可选）
if [ "$1" = "clean" ]; then
    echo "清理构建缓存..."
    rm -f CMakeCache.txt
    rm -rf CMakeFiles
fi

# 配置CMake项目
echo "配置CMake项目..."
cmake -DCMAKE_C_STANDARD=11 ..
if [ $? -ne 0 ]; then
    echo "CMake配置失败！"
    echo "当前目录: $(pwd)"
    echo "查找CMakeLists.txt:"
    if [ -f "../CMakeLists.txt" ]; then
        echo "  ✓ 找到 ../CMakeLists.txt"
    else
        echo "  ✗ 未找到 ../CMakeLists.txt"
    fi
    cd "$ORIGINAL_DIR"
    exit 1
fi

# 编译项目
echo
echo "编译测试程序..."
cmake --build . --target $TEST_TARGET
if [ $? -ne 0 ]; then
    echo "编译失败！"
    cd "$ORIGINAL_DIR"
    exit 1
fi

# 运行测试
echo
echo "运行测试..."
echo "========================================"
./$TEST_TARGET
test_result=$?

echo
echo "========================================"
if [ $test_result -eq 0 ]; then
    echo "所有测试通过！ ✓"
else
    echo "测试失败！ ✗"
fi
echo "========================================"

# 显示使用帮助
echo
echo "使用方法:"
echo "  ./run_tests.sh       - 运行分层架构测试"
echo "  ./run_tests.sh clean - 清理后重新构建"

# 返回原始目录
cd "$ORIGINAL_DIR"

exit $test_result