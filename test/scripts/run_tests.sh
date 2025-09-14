# BitsButton 测试运行脚本 (Linux/macOS)
# 支持 GitHub Actions CI 环境

set -e  # 遇到错误立即退出

# 设置路径变量
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TEST_DIR="$(dirname "$SCRIPT_DIR")"
PROJECT_ROOT="$(dirname "$TEST_DIR")"

echo "========================================="
echo "    BitsButton 测试框架 - CI 模式"
echo "========================================="

# 检测运行环境
if [ -n "$GITHUB_ACTIONS" ]; then
    echo "🚀 检测到 GitHub Actions 环境"
    CI_MODE=true
    echo "环境信息:"
    echo "  - OS: $RUNNER_OS"
    echo "  - CC: ${CC:-未设置}"
    echo "  - CXX: ${CXX:-未设置}"
    echo "  - PATH: $PATH"
else
    echo "🏠 本地开发环境"
    CI_MODE=false
fi

# 设置构建目录
BUILD_DIR="build"
TEST_LOG="test_output.log"

# 切换到test目录
cd "$TEST_DIR"

# 检查当前目录
 echo "当前工作目录: $(pwd)"
 echo "目录内容:"
 ls -la

# 清理之前的构建
if [ -d "$BUILD_DIR" ]; then
    echo "🧹 清理之前的构建..."
    rm -rf "$BUILD_DIR"
fi

# 创建构建目录
echo "📁 创建构建目录..."
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"
echo "构建目录: $(pwd)"

echo "🔧 配置 CMake..."
if [ "$CI_MODE" = true ]; then
    # CI 环境配置
    echo "使用编译器: CC=${CC:-gcc}, CXX=${CXX:-g++}"
    cmake -DCMAKE_BUILD_TYPE=Release \
          -DCMAKE_C_COMPILER="${CC:-gcc}" \
          -DCMAKE_CXX_COMPILER="${CXX:-g++}" \
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

    # 额外执行编译配置验证
    echo ""
    echo "🔧 【额外验证】编译配置兼容性"
    echo "----------------------------------------"

    # 切换到项目根目录进行编译测试
    cd "$PROJECT_ROOT"

    compilation_failed=false

    # 使用CI环境中设置的编译器
    C_COMPILER="${CC:-gcc}"
    CXX_COMPILER="${CXX:-g++}"

    echo "使用编译器: C=$C_COMPILER, C++=$CXX_COMPILER"

    # 检查编译器是否可用
    if ! command -v "$C_COMPILER" >/dev/null 2>&1; then
        echo "❌ 错误: C编译器 '$C_COMPILER' 不可用"
        echo "可用的编译器:"
        which gcc 2>/dev/null && echo "  - gcc: $(which gcc)"
        which clang 2>/dev/null && echo "  - clang: $(which clang)"
        exit 1
    fi

    if ! command -v "$CXX_COMPILER" >/dev/null 2>&1; then
        echo "❌ 错误: C++编译器 '$CXX_COMPILER' 不可用"
        echo "可用的编译器:"
        which g++ 2>/dev/null && echo "  - g++: $(which g++)"
        which clang++ 2>/dev/null && echo "  - clang++: $(which clang++)"
        exit 1
    fi

    echo ""

    echo "测试编译指令1: $C_COMPILER -c -DBITS_BTN_DISABLE_BUFFER -std=c99 bits_button.c"
    if $C_COMPILER -c -DBITS_BTN_DISABLE_BUFFER -std=c99 bits_button.c -o bits_button_disable.o 2>compile_error1.log; then
        echo "✅ 禁用缓冲区模式 C99 编译成功"
        [ -f "bits_button_disable.o" ] && rm -f "bits_button_disable.o"
        [ -f "compile_error1.log" ] && rm -f "compile_error1.log"
    else
        echo "❌ 禁用缓冲区模式 C99 编译失败"
        echo "错误信息："
        if [ -f "compile_error1.log" ]; then
            cat "compile_error1.log"
            rm -f "compile_error1.log"
        fi
        compilation_failed=true
    fi

    echo "测试编译指令2: $C_COMPILER -c -DBITS_BTN_USE_USER_BUFFER -std=c99 bits_button.c"
    if $C_COMPILER -c -DBITS_BTN_USE_USER_BUFFER -std=c99 bits_button.c -o bits_button_user.o 2>compile_error2.log; then
        echo "✅ 用户缓冲区模式 C99 编译成功"
        [ -f "bits_button_user.o" ] && rm -f "bits_button_user.o"
        [ -f "compile_error2.log" ] && rm -f "compile_error2.log"
    else
        echo "❌ 用户缓冲区模式 C99 编译失败"
        echo "错误信息："
        if [ -f "compile_error2.log" ]; then
            cat "compile_error2.log"
            rm -f "compile_error2.log"
        fi
        compilation_failed=true
    fi

    echo "测试编译指令3: $C_COMPILER -c -std=c11 bits_button.c"
    if $C_COMPILER -c -std=c11 bits_button.c -o bits_button_default.o 2>compile_error3.log; then
        echo "✅ 默认模式 C11 编译成功"
        [ -f "bits_button_default.o" ] && rm -f "bits_button_default.o"
        [ -f "compile_error3.log" ] && rm -f "compile_error3.log"
    else
        echo "❌ 默认模式 C11 编译失败"
        echo "错误信息："
        if [ -f "compile_error3.log" ]; then
            cat "compile_error3.log"
            rm -f "compile_error3.log"
        fi
        compilation_failed=true
    fi

    echo "测试编译指令4: $CXX_COMPILER -std=c++11 -Wall -Wextra -I. test/cases/compat/test_cpp_compat.cpp"
    if $CXX_COMPILER -std=c++11 -Wall -Wextra -I. test/cases/compat/test_cpp_compat.cpp -o test_cpp_compat 2>compile_error4.log; then
        echo "✅ C++兼容性 C++11 编译成功"
        [ -f "test_cpp_compat" ] && rm -f "test_cpp_compat"
        [ -f "compile_error4.log" ] && rm -f "compile_error4.log"
    else
        echo "❌ C++兼容性 C++11 编译失败"
        echo "错误信息："
        if [ -f "compile_error4.log" ]; then
            cat "compile_error4.log"
            rm -f "compile_error4.log"
        fi
        compilation_failed=true
    fi

    if [ "$compilation_failed" = true ]; then
        echo "❌ 编译配置兼容性验证失败！"
        exit 1
    else
        echo "✅ 编译配置兼容性验证完成！"
    fi

    exit 0
else
    echo "❌ 测试失败！退出码: $TEST_RESULT"
    exit $TEST_RESULT
fi