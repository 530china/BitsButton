@echo off
setlocal enabledelayedexpansion
chcp 65001 >nul

:: 获取脚本所在目录的绝对路径
set SCRIPT_DIR=%~dp0
:: 获取test目录路径（脚本目录的上级目录）
set TEST_DIR=%SCRIPT_DIR%..
:: 保存当前目录
set ORIGINAL_DIR=%CD%

echo ========================================
echo     BitsButton 测试框架 v3.0
echo     分层架构 - 模块化设计
echo ========================================

REM 检测是否在 GitHub Actions 环境中
if defined GITHUB_ACTIONS (
    echo 🚀 检测到 GitHub Actions 环境
    set CI_MODE=true
) else (
    echo 🏠 本地开发环境
    set CI_MODE=false
)

echo.

:: 设置测试目标（与CMakeLists.txt中的目标名称一致）
set TEST_TARGET=run_tests_new
set TEST_DESCRIPTION=分层架构测试

echo 运行模式: %TEST_DESCRIPTION%
echo 测试目录: %TEST_DIR%
echo.

:: 切换到test目录
cd /d "%TEST_DIR%"

:: 创建构建目录
if not exist "build" (
    echo 🔧 创建构建目录...
    mkdir "build"
)
cd "build"

:: 清除CMake缓存（可选）
if "%1"=="clean" (
    echo 🧹 清理构建缓存...
    if exist "CMakeCache.txt" del "CMakeCache.txt"
    if exist "CMakeFiles" rmdir /s /q "CMakeFiles"
)

:: 配置CMake项目
echo 🔧 配置CMake项目...
if "%CI_MODE%"=="true" (
    REM CI 环境配置
    cmake -G "MinGW Makefiles" -DCMAKE_C_COMPILER=gcc -DCMAKE_C_STANDARD=11 -DCMAKE_BUILD_TYPE=Release -DCMAKE_VERBOSE_MAKEFILE=ON ..
) else (
    REM 本地环境配置
    cmake -G "MinGW Makefiles" -DCMAKE_C_COMPILER=gcc -DCMAKE_C_STANDARD=11 ..
)

if %errorlevel% neq 0 (
    echo ❌ CMake配置失败！
    echo 当前目录: %CD%
    echo 查找CMakeLists.txt:
    if exist "..\CMakeLists.txt" (
        echo   ✓ 找到 ..\CMakeLists.txt
    ) else (
        echo   ✗ 未找到 ..\CMakeLists.txt
    )
    cd /d "%ORIGINAL_DIR%"
    if "%CI_MODE%"=="false" pause
    exit /b 1
)

:: 编译项目
echo.
echo 🔨 编译测试程序...
if "%CI_MODE%"=="true" (
    REM CI 环境使用 cmake --build
    cmake --build . --target %TEST_TARGET% --verbose
) else (
    REM 本地环境使用 cmake --build
    cmake --build . --target %TEST_TARGET%
)

if %errorlevel% neq 0 (
    echo ❌ 编译失败！
    cd /d "%ORIGINAL_DIR%"
    if "%CI_MODE%"=="false" pause
    exit /b 1
)

:: 运行测试
echo.
echo 🧪 运行测试...
echo ========================================

REM 运行编译生成的可执行文件（目标名称.exe）
if "%CI_MODE%"=="true" (
    REM CI 环境：尝试输出到文件和控制台
    %TEST_TARGET%.exe > "..\test_output.log" 2>&1
    set test_result=!errorlevel!
    REM 显示日志内容
    type "..\test_output.log"
) else (
    REM 本地环境：直接运行
    %TEST_TARGET%.exe
    set test_result=!errorlevel!
)

echo.
echo ========================================
if !test_result! equ 0 (
    echo ✅ 所有测试通过！

    REM 额外执行编译配置验证
    echo.
    echo 🔧 【额外验证】编译配置兼容性
    echo ----------------------------------------

    REM 切换到项目根目录进行编译测试
    cd /d "%TEST_DIR%\.."

    echo 测试编译指令1: gcc -c -DBITS_BTN_DISABLE_BUFFER -std=c99 bits_button.c
    gcc -c -DBITS_BTN_DISABLE_BUFFER -std=c99 bits_button.c -o bits_button_disable.o 2>compile_error1.log
    if !errorlevel! equ 0 (
        echo ✅ 禁用缓冲区模式 C99 编译成功
        if exist "bits_button_disable.o" del "bits_button_disable.o" 2>nul
        if exist "compile_error1.log" del "compile_error1.log" 2>nul
    ) else (
        echo ❌ 禁用缓冲区模式 C99 编译失败
        echo 错误信息：
        if exist "compile_error1.log" (
            type "compile_error1.log"
            del "compile_error1.log" 2>nul
        )
    )

    echo 测试编译指令2: gcc -c -DBITS_BTN_USE_USER_BUFFER -std=c99 bits_button.c
    gcc -c -DBITS_BTN_USE_USER_BUFFER -std=c99 bits_button.c -o bits_button_user.o 2>compile_error2.log
    if !errorlevel! equ 0 (
        echo ✅ 用户缓冲区模式 C99 编译成功
        if exist "bits_button_user.o" del "bits_button_user.o" 2>nul
        if exist "compile_error2.log" del "compile_error2.log" 2>nul
    ) else (
        echo ❌ 用户缓冲区模式 C99 编译失败
        echo 错误信息：
        if exist "compile_error2.log" (
            type "compile_error2.log"
            del "compile_error2.log" 2>nul
        )
    )

    echo 测试编译指令3: gcc -c -std=c11 bits_button.c
    gcc -c -std=c11 bits_button.c -o bits_button_default.o 2>compile_error3.log
    if !errorlevel! equ 0 (
        echo ✅ 默认模式 C11 编译成功
        if exist "bits_button_default.o" del "bits_button_default.o" 2>nul
        if exist "compile_error3.log" del "compile_error3.log" 2>nul
    ) else (
        echo ❌ 默认模式 C11 编译失败
        echo 错误信息：
        if exist "compile_error3.log" (
            type "compile_error3.log"
            del "compile_error3.log" 2>nul
        )
    )

    echo ✅ 编译配置兼容性验证完成！

    REM 返回到构建目录
    cd /d "%TEST_DIR%\build"
) else (
    echo ❌ 测试失败！退出码: !test_result!
)
echo ========================================

:: 显示使用帮助
if "%CI_MODE%"=="false" (
    echo.
    echo 使用方法:
    echo   run_tests.bat       - 运行分层架构测试
    echo   run_tests.bat clean - 清理后重新构建
)

:: 返回原始目录
cd /d "%ORIGINAL_DIR%"

:: 如果不是CI环境，暂停等待用户输入
if "%CI_MODE%"=="false" (
    pause
)

exit /b !test_result!