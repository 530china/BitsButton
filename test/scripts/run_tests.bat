@echo off
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
echo.

:: 设置测试目标
set TEST_TARGET=run_tests_new
set TEST_DESCRIPTION=分层架构测试

echo 运行模式: %TEST_DESCRIPTION%
echo 测试目录: %TEST_DIR%
echo.

:: 切换到test目录
cd /d "%TEST_DIR%"

:: 创建构建目录
if not exist "build" (
    echo 创建构建目录...
    mkdir "build"
)
cd "build"

:: 清除CMake缓存（可选）
if "%1"=="clean" (
    echo 清理构建缓存...
    if exist "CMakeCache.txt" del "CMakeCache.txt"
    if exist "CMakeFiles" rmdir /s /q "CMakeFiles"
)

:: 配置CMake项目
echo 配置CMake项目...
cmake -G "MinGW Makefiles" -DCMAKE_C_COMPILER=gcc -DCMAKE_C_STANDARD=11 ..
if %errorlevel% neq 0 (
    echo CMake配置失败！
    echo 当前目录: %CD%
    echo 查找CMakeLists.txt: 
    if exist "..\CMakeLists.txt" (
        echo   ✓ 找到 ..\CMakeLists.txt
    ) else (
        echo   ✗ 未找到 ..\CMakeLists.txt
    )
    cd /d "%ORIGINAL_DIR%"
    pause
    exit /b 1
)

:: 编译项目
echo.
echo 编译测试程序...
cmake --build . --target %TEST_TARGET%
if %errorlevel% neq 0 (
    echo 编译失败！
    cd /d "%ORIGINAL_DIR%"
    pause
    exit /b 1
)

:: 运行测试
echo.
echo 运行测试...
echo ========================================
%TEST_TARGET%.exe
set test_result=%errorlevel%

echo.
echo ========================================
if %test_result% equ 0 (
    echo 所有测试通过！ ✓
) else (
    echo 测试失败！ ✗
)
echo ========================================

:: 显示使用帮助
echo.
echo 使用方法:
echo   run_tests.bat       - 运行分层架构测试
echo   run_tests.bat clean - 清理后重新构建

:: 返回原始目录
cd /d "%ORIGINAL_DIR%"

pause
exit /b %test_result%