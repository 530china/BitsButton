# BitsButton

嵌入式系统优化的按键检测框架 - 通过二进制位序列技术高效处理单键、组合键和复杂序列

[![CI Status](https://github.com/530china/BitsButton/workflows/BitsButton%20稳定CI/badge.svg)](https://github.com/530china/BitsButton/actions/workflows/stable-ci.yml)
[![License](https://img.shields.io/github/license/rahuldkjain/github-profile-readme-generator?style=flat-square)](https://github.com/530china/BitsButton/blob/master/LICENSE)

## 核心特性

- **全生命周期按键跟踪**：从按键按下到释放的完整状态跟踪，创新使用位序列记录按键状态，1代表按下，0代表松开：

  键值 | 说明
  --- | ---
  0b0 | 未按下
  0b010 | 单击
  0b01010 | 双击
  0b01010...n | n连击
  0b011 | 长按开始
  0b0111 | 长按保持
  0b01110 | 长按结束
  0b01011 | 短按然后长按
  0b0101011 | 双击然后长按
  0b01010..n11 | n连击然后长按

  直观的二进制表示让按键逻辑一目了然
- **基础按键事件**：单击、双击、三击、长按、长按保持等多种事件类型
- **高级序列检测**：可识别复杂的按键模式，如"单击→长按→双击"序列、"长按后单击切换设置项"、"双击后长按执行特殊功能"等高级操作
- **组合键支持**：支持多键组合，如同时按下"音量+"和"音量-"执行特殊功能
- **超低内存占用**：每个按键仅需少于20字节RAM，适合资源受限环境
- **灵活缓冲区模式**：支持C11原子缓冲（线程安全）、用户自定义缓冲和零内存占用模式
- **低功耗优化**：独创peek功能，可在不消费事件的情况下预览状态，完美适配休眠唤醒场景
- **双重处理模式**：支持回调模式（事件驱动）和轮询模式（精确控制）
- **C标准兼容**：支持C89/C99/C11，兼容老旧编译器

## 快速开始

```bash
# 克隆项目
git clone https://github.com/530china/BitsButton.git
cd BitsButton

# 运行自动化测试用例
./run_tests.bat  # Windows
```



## 项目结构

```
BitsButton/
├── bits_button.h/.c        # 核心库
├── examples/               # 示例代码
├── test/                   # 测试框架
├── simulator/              # 按键模拟器
├── docs/                   # 详细文档
└── README.md
```

## 应用场景

- **智能家居**：智能灯泡、智能插座、智能门锁等
- **可穿戴设备**：智能手表、健康监测设备等
- **嵌入式系统**：工业控制、医疗设备、汽车电子等

## 文档

详细文档请参见 [docs/](docs/) 目录，包括：

### 文档组织原则

- **项目根文档原则**：主要的、面向用户的文档放在docs目录下，作为统一的文档入口
- **模块内部文档原则**：特定模块的技术实现细节文档保留在模块目录内

遵循以上原则，避免文档内容重复，保持文档结构清晰。

- [使用指南](docs/usage_guide.md) - 如何在实际项目中使用本库
- [API文档](docs/api.md)
- [缓冲区模式配置](docs/buffer_modes.md)
- [低功耗事件预览功能](docs/peek_feature.md)
- [测试框架说明](docs/testing.md)
- [按键模拟器使用](docs/simulator.md)

## 许可证

Apache License Version 2.0