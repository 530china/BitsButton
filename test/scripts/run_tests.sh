#!/bin/bash

# BitsButton 测试运行脚本 (Linux/macOS)
# 支持 GitHub Actions CI 环境

set -e  # 遇到错误立即退出

echo "========================================="
echo "    BitsButton 测试框架 - CI 模式"
echo "========================================="

# 检测运行环境
if [ -n "$GITHUB_ACTIONS" ]; then
    echo "🚀 检测到 GitHub Actions 环境"
    CI_MODE=true
else
    echo "🏠 本地开发环境"
    CI_MODE=false
fi

# 设置构建目录
BUILD_DIR="build"
TEST_LOG="test_output.log"

# 清理之前的构建
if [ -d "$BUILD_DIR" ]; then
    echo "🧹 清理之前的构建..."
    rm -rf "$BUILD_DIR"
fi

# 创建构建目录
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

echo "🔧 配置 CMake..."
if [ "$CI_MODE" = true ]; then
    # CI 环境配置
    cmake -DCMAKE_BUILD_TYPE=Release \
          -DCMAKE_C_COMPILER="${CC:-gcc}" \
          -DCMAKE_VERBOSE_MAKEFILE=ON \
          ..
else
    # 本地环境配置
    cmake -DCMAKE_BUILD_TYPE=Debug ..
fi

echo "🔨 编译测试程序..."
make -j$(nproc 2>/dev/null || echo 4)

echo "🧪 运行测试..."
if [ "$CI_MODE" = true ]; then
    # CI 环境：输出到文件和控制台
    ./run_tests_new 2>&1 | tee "../$TEST_LOG"
    TEST_RESULT=${PIPESTATUS[0]}
else
    # 本地环境：直接运行
    ./run_tests_new
    TEST_RESULT=$?
fi

echo "========================================="
if [ $TEST_RESULT -eq 0 ]; then
    echo "✅ 所有测试通过！"
    exit 0
else
    echo "❌ 测试失败！退出码: $TEST_RESULT"
    exit $TEST_RESULT
fi