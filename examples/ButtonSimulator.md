# 按键模拟器

## 简介
为了脱离开发板进行程序的逻辑验证，我用python编写了一个按键模拟器，思路为将bits_button编译为动态库，由按键模拟器产生按键信号后调用动态库处理；
目前只测试了Windows


## 支持功能
- 配置按键映射，通过key_bindings.json
- 按键绑定动态更新；

## 快速开始

Windows:
```bat
cd bits_button

# 硬件模式(依赖编译的DLL/so)
.\simulator\build\build.bat(与下面的指令等价)
.\simulator\build\build.bat --mode hardware

# 软件模式(纯Python实现)
.\simulator\build\build.bat --mode software
```

效果图如下：
UI如下图：

![alt text](../docs/ButtonSimuUI.png)