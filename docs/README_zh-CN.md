<h1 align="center">BitsButton</h1>

<p align="center">
<a href="https://github.com/530china/EmbeddedButton/blob/master/LICENSE" target="blank">
<img src="https://img.shields.io/github/license/rahuldkjain/github-profile-readme-generator?style=flat-square" alt="github-profile-readme-generator license" />
</a>
<a href="https://github.com/530china/EmbeddedButton/stargazers" target="blank">
<img src="https://img.shields.io/github/stars/rahuldkjain/github-profile-readme-generator?style=flat-square" alt="github-profile-readme-generator stars"/>
</a>
<a href="https://github.com/530china/EmbeddedButton/issues" target="blank">
<img src="https://img.shields.io/github/issues/rahuldkjain/github-profile-readme-generator?style=flat-square" alt="github-profile-readme-generator issues"/>
</a>
<a href="https://github.com/530china/EmbeddedButton/pulls" target="blank">
<img src="https://img.shields.io/github/issues-pr/rahuldkjain/github-profile-readme-generator?style=flat-square" alt="github-profile-readme-generator pull-requests"/>
</a>
</p>

<h2>👋 简介</h2>
Bits Button是一个高效、灵活的按键事件检测库，专为嵌入式系统设计。它支持从简单按键检测到复杂按键序列识别的各种场景，特别适用于需要处理组合按键和复杂按键序列的应用；

## 🌱 特性


> 1.使用C语言实现，巧妙利用位运算来实现每个按键键值的二进制记录表示，1代表按下，0代表松开

键值 | 说明
--- | ---
0b0 | 未按下
0b010 | 单击
0b01010 | 双击
0b01010...n | n连击
0b011 | 长按开始
0b0111| 长按保持
0b01110|长按结束
0b01011|短按然后长按
0b0101011 | 双击然后长按
0b01010..n11 | n连击然后长按

> 2.​​组合按键支持
- 轻松定义任意按键组合（如 Ctrl+C）
- 智能处理组合键与单键的冲突检测
- 可配置组合键的抑制策略（阻止触发组合内的单键事件）

> 3.序列按键检测：
- 识别复杂按键序列（如 单击-长按-双击）
- 支持按键序列的二进制编码表示（如 0b1011101010）
- 状态机驱动的序列识别逻辑；

> 4.​​​完整按键事件生命周期（多种按键事件类型）​​
- 按键按下/抬起事件
- 单击、双击、多连击识别
- 长按开始/结束/周期性触发
- 状态完成事件

> 5.​​高性能事件队列
- 无锁环形缓冲区设计（FIFO）
- 线程安全的单生产者/单消费者(SPSC)模型​
- 可配置的缓冲区大小
- 缓冲区溢出检测与统计
![alt text](image-1.png)

> 6.​​模块化架构
- 位运算优化处理逻辑
- 状态机驱动的事件处理
- 原子操作支持多线程环境

> 7.​高度可配置
- 状态机驱动的核心逻辑
- 硬件抽象层设计（HAL）
- 可插拔的调试接口

## 核心数据结构

```c
// 单按键对象
typedef struct {
    uint16_t key_id;             // 唯一按键ID
    button_param_t *param;      // 按键参数
    btn_state_t current_state;   // 当前状态
    uint32_t state_bits;         // 状态序列位图
} button_obj_t;

// 组合按键对象
typedef struct {
    uint16_t key_count;          // 组合中按键数量
    uint16_t key_single_ids[BITS_BTN_MAX_COMBO_KEYS]; // 成员按键ID
    button_obj_t btn;            // 组合按键状态
    button_mask_type_t combo_mask; // 组合掩码
    bool suppress;               // 是否抑制成员按键事件
} button_obj_combo_t;

// 按键事件结果
typedef struct {
    uint16_t key_id;             // 触发按键ID
    btn_state_t event;           // 按键事件类型
    uint16_t long_press_period_trigger_cnt; // 长按周期计数
    key_value_type_t key_value;  // 按键值（序列位图）
} bits_btn_result_t;
```

## 📋 快速开始

### 1）使用
<details>
<summary>点击展开/折叠C代码<img src="https://media.giphy.com/media/WUlplcMpOCEmTGBtBW/giphy.gif" width="30"></summary>

- [使用callback方式](../examples/example_callback.c);
- [使用poll方式](../examples/example_poll.c);
<br></details>

### 2）调试

<details>
<summary>点击展开/折叠<img src="https://media.giphy.com/media/WUlplcMpOCEmTGBtBW/giphy.gif" width="30"></summary>

- bits_button_init时，注册定义你的打印函数：
```c
int my_log_printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    int result = vprintf(format, args);
    va_end(args);
    return result;
}

bits_button_init(
    btns,
    ARRAY_SIZE(btns),
    btns_combo,
    ARRAY_SIZE(btns_combo),
    read_key_state,
    bits_btn_result_cb,
    my_log_printf
);
```
<br></details>

## ⚡ 其他
- 本项目基于本人实际开发中遇到的一些按键驱动使用体验问题，在他人项目（见参考链接）的思想基础上，开发的此按键检测框架，感谢帮助思考我的小伙伴[shawnfeng0](https://github.com/shawnfeng0)以及正在使用此模块的小伙伴，欢迎一起开发改进！
- 更多高级用法见 [按键模拟器](../examples/ButtonSimulator.md)

TODO:
- [ ] 按键模拟器linux环境支持；
- [ ] 自动化测试用例支持；

## 💬 参考链接
- [MultiButton](https://github.com/0x1abin/MultiButton)
- [FlexibleButton](https://github.com/murphyzhao/FlexibleButton/tree/master)
- [安富莱按键FIFO思想](https://www.armbbs.cn/forum.php?mod=viewthread&tid=111527&highlight=%B0%B4%BC%FC)
- [easy_button](https://github.com/bobwenstudy/easy_button#)