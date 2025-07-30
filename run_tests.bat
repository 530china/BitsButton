@echo off
setlocal enabledelayedexpansion
chcp 65001 >nul

echo ========================================
echo      BitsButton 快速测试启动器
echo ========================================

REM 检查test目录是否存在
if not exist "test" (
    echo ❌ 错误：找不到test目录！
    echo 请确保在BitsButton项目根目录下运行此脚本。
    pause
    exit /b 1
)

REM 检查测试脚本是否存在
if not exist "test\scripts\run_tests.bat" (
    echo ❌ 错误：找不到测试脚本 test\scripts\run_tests.bat
    pause
    exit /b 1
)

echo 🚀 启动测试框架...
echo 测试脚本位置: test\scripts\run_tests.bat
echo.

REM 调用test目录下的测试脚本
call "test\scripts\run_tests.bat" %*

REM 获取测试结果
set test_result=%errorlevel%

echo.
echo ========================================
if %test_result% equ 0 (
    echo ✅ 测试完成！所有测试通过
) else (
    echo ❌ 测试失败！退出码: %test_result%
)
echo ========================================

REM 如果不是在CI环境中，暂停等待用户输入
if not defined GITHUB_ACTIONS (
    pause
)

exit /b %test_result%