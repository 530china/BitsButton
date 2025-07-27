@echo off
echo 正在启动BitsButton测试框架...
cd test
call scripts\run_tests.bat %*
cd ..