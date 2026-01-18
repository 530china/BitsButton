# 按键模拟器技术实现文档

## 技术架构
按键模拟器采用Python实现，通过C语言适配层与BitsButton库交互。该模块实现了跨平台的按键模拟功能，支持软件模式和硬件模式双模式运行。

## 目录结构

```
simulator/
├── adapter_layer/           # C语言适配层
│   ├── button_adapter.c     # 适配层实现（连接C库和Python模拟器）
│   ├── button_adapter.h     # 适配层接口定义
│   ├── button_types.h       # 按键类型定义
│   └── Makefile             # 编译配置
├── python_simulator/        # Python实现的模拟器
│   ├── advanced_v2_sim.py   # 高级模拟器实现
│   ├── button_ctrl.py       # 按键控制逻辑
│   ├── button_ui.py         # 用户界面实现
│   ├── event_logger.py      # 事件日志记录
│   └── config_manager.py    # 配置管理
├── key_bindings.json        # 按键绑定配置文件
├── run.py                   # 主运行脚本
├── output/                  # 编译输出目录
│   ├── libbutton.dll        # Windows动态库
│   ├── libbutton.so         # Linux动态库
│   └── libbutton.dylib      # macOS动态库
└── ButtonSimulator.md       # 本文档
```

## 核心组件

- **Python GUI界面**：使用tkinter实现的跨平台图形界面
- **C语言适配层**：通过adapter_layer实现与BitsButton库的交互
- **双模式支持**：
    - **软件模式**：纯Python模拟实现
    - **硬件模式**：通过动态库与底层交互
- **配置管理系统**：JSON格式的按键绑定配置
- **日志系统**：详细的操作事件记录


## 技术部署

### 环境要求

- Python 3.6+ 运行时环境
- tkinter GUI库（Python内置，但某些Linux发行版需单独安装）
- 编译工具链（硬件模式）：
  - Windows: MinGW
  - Linux: GCC
  - macOS: Xcode Command Line Tools

### 架构实现

```bash
# 项目结构
cd BitsButton/simulator

# 主要运行方式
python run.py                   # 软件模式运行
python run.py --mode hardware   # 硬件模式运行

# 环境配置选项
python run.py --fix-env         # 自动修复环境依赖
python run.py --install-compiler # 安装编译工具
python run.py --help            # 查看命令行选项
```

### 组件交互

软件模式下，Python模拟器在纯Python环境中运行，用于验证模拟器自身的界面和功能。硬件模式下，通过动态库与BitsButton库交互，用于验证库的功能。

## 常见问题解决

### 🔧 自动环境修复
如果遇到以下问题，可以使用自动修复功能：

```bash
# 修复所有环境问题
python3 run.py --fix-env
```

**支持自动修复的问题：**
- ✅ **tkinter 缺失**：自动安装适合的 tkinter 支持
- ✅ **依赖包兼容性**：智能选择兼容的 pynput 版本
- ✅ **虚拟环境损坏**：自动重建虚拟环境
- ✅ **权限问题**：自动检测并引导解决 macOS 辅助功能权限

### 📋 手动解决方案
#### 依赖包问题
```bash
# 卸载可能损坏的依赖
pip uninstall pynput -y

# 重新安装兼容版本
pip install "pynput>=1.7.6"

# 如果上面失败，尝试特定版本
pip install "pynput==1.7.6"
```

#### macOS tkinter 问题
```bash
# 查看Python版本
python3 --version

# 安装对应版本的tkinter（替换X.X为实际版本号）
brew install python-tk@X.X
# 例如：brew install python-tk@3.13

# 验证安装
python3 -c "import tkinter; print('tkinter可用')"
```

#### 虚拟环境问题
```bash
# 删除损坏的虚拟环境
rm -rf simulator/.venv

# 重新运行脚本（会自动创建新环境）
python3 simulator/run.py
```

## 硬件模式说明
### 编译器要求
平台 | 编译器 | 安装指南
--- | --- | ---
Windows |MinGW | 下载安装：https://sourceforge.net/projects/mingw/
Linux|gcc/build-essential|sudo apt install build-essential
macOS|Xcode 命令行工具|xcode-select --install

### 常见问题解决
- **硬件加载失败**：
```bash
# macOS特殊处理
xattr -d com.apple.quarantine output/libbutton.dylib
```
- **编译器问题**：
```bash
# 自动安装编译器
python run.py --install-compiler
```
- **环境问题**：
```bash
# 自动修复环境
python run.py --fix-env
```

## 配置文件
按键绑定保存在**key_bindings.json**中：
```json
{
  "mappings": [
    {
      "id": "btn1",
      "key": "a",
      "color": "red",
      "btn_number": 0
    },
    {
      "id": "btn2",
      "key": "b",
      "color": "orange",
      "btn_number": 1
    }
  ]
}
```

- id：按钮标识符
- key：绑定的键盘按键
- color：按下时显示的颜色
- btn_number：硬件按钮编号（硬件模式使用）

> 提示：修改配置文件后需重启应用生效

## 用户使用参考

如需了解模拟器的基本使用方法和应用场景，请参阅主项目的用户文档：

- [使用指南](../docs/usage_guide.md)
- [模拟器用户文档](../docs/simulator.md)

---

效果图如下：
UI如下图：

![alt text](../docs/ButtonSimuUI.png)