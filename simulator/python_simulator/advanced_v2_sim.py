import tkinter as tk
from tkinter import ttk, scrolledtext
import json
import logging
import queue
import threading
from pynput import keyboard

class DynamicKeySimulator:
    def __init__(self, use_hardware=False):
        # 核心状态初始化
        self.key_states = {}
        self.entries = {}  # 存储配置输入框
        self.event_queue = queue.Queue(maxsize=1000)
        self.processing_flag = threading.Event()
        self.processing_flag.set()

        # 硬件模式配置
        self.use_hardware = use_hardware
        self.current_mode = "硬件模式" if self.use_hardware else "软件模式"

        # GUI初始化
        self.root = tk.Tk()
        self.root.title("高级按键模拟器 v3.3")
        self._create_ui()
        self._setup_logging()

        self._init_hardware_mode()

        # 事件监听器
        self._start_keyboard_listener()
        self._start_event_processor()

        # 关闭事件绑定
        self.root.protocol("WM_DELETE_WINDOW", self._safe_shutdown)

    def _init_hardware_mode(self):
        """硬件模式初始化"""
        self.current_mode = "硬件模式" if self.use_hardware else "软件模式"
        if self.use_hardware:
            try:
                from button_ctrl import ButtonController
                self.controller = ButtonController()
                logging.info("硬件控制器初始化成功")
            except ImportError:
                self.use_hardware = False
                self.current_mode = "软件模式（硬件不可用）"
                logging.warning("硬件模块加载失败")

    def _create_ui(self):
        """动态生成界面组件"""
        main_frame = ttk.Frame(self.root)
        main_frame.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)

        # 模式状态栏
        self._build_mode_indicator(main_frame)

        # 配置面板
        config_frame = ttk.LabelFrame(main_frame, text="按键绑定配置")
        config_frame.pack(fill=tk.X, pady=5)
        self._build_config_entries(config_frame)

        # 状态面板
        status_frame = ttk.LabelFrame(main_frame, text="按键状态")
        status_frame.pack(fill=tk.X, pady=5)
        self._build_status_labels(status_frame)

        # 日志面板
        self._build_log_panel(main_frame)

    def _build_mode_indicator(self, parent):
        """构建模式指示器"""
        mode_frame = ttk.Frame(parent)
        mode_frame.pack(fill=tk.X, pady=5)
        self.mode_label = ttk.Label(
            mode_frame,
            text=f"当前模式: {self.current_mode}",
            foreground="red" if self.use_hardware else "green",
            font=('微软雅黑', 10, 'bold')
        )
        self.mode_label.pack(side=tk.RIGHT, padx=10)

    def _build_config_entries(self, parent):
        """构建配置输入框"""
        # 从配置加载键绑定（原 load_config 方法）
        self.key_bindings = self.load_config()

        for btn_id, config in self.key_bindings.items():
            row = ttk.Frame(parent)
            row.pack(fill=tk.X, pady=2)

            ttk.Label(row, text=f"{config['id']}:").pack(side=tk.LEFT)
            entry = ttk.Entry(row, width=5)
            entry.insert(0, config['key'])
            entry.pack(side=tk.LEFT, padx=5)

            ttk.Button(row, text="更新",
                    command=lambda id=btn_id: self._update_binding(id))\
                    .pack(side=tk.LEFT)
            self.entries[btn_id] = entry  # 正确存储到 entries

    def _build_status_labels(self, parent):
        """构建状态标签"""
        self.status_labels = {}
        for btn_id in self.key_bindings:
            lbl = ttk.Label(parent,
                          text=f"{btn_id}: 未按下",
                          foreground="gray",
                          padding=5)
            lbl.pack(side=tk.LEFT, padx=10)
            self.status_labels[btn_id] = lbl

    def _build_log_panel(self, parent):
        """构建日志面板"""
        log_frame = ttk.LabelFrame(parent, text="操作日志")
        log_frame.pack(fill=tk.BOTH, expand=True)
        self.log_area = scrolledtext.ScrolledText(log_frame, wrap=tk.WORD)
        self.log_area.pack(fill=tk.BOTH, expand=True)

    def _start_keyboard_listener(self):
        """启动非阻塞式键盘监听"""
        def _on_event(event_type, key):
            self.event_queue.put((event_type, key))

        self.listener = keyboard.Listener(
            on_press=lambda k: _on_event('press', k),
            on_release=lambda k: _on_event('release', k)
        )
        self.listener.daemon = True
        self.listener.start()

    def _start_event_processor(self):
        """启动事件处理线程"""
        def _processor():
            while self.processing_flag.is_set():
                try:
                    event_type, key = self.event_queue.get(timeout=0.1)
                    self.root.after(0, self._process_event, event_type, key)
                except queue.Empty:
                    continue
                except Exception as e:
                    logging.error(f"事件处理异常: {str(e)}")

        self.process_thread = threading.Thread(target=_processor)
        self.process_thread.daemon = True
        self.process_thread.start()

    def _process_event(self, event_type, key):
        """GUI线程安全的事件处理"""
        try:
            if hasattr(key, 'char') and key.char.isprintable():
                key_char = key.char.lower()
                is_press = (event_type == 'press')

                # 状态去重检查
                if self.key_states.get(key_char, False) != is_press:
                    self.key_states[key_char] = is_press
                    self._handle_key_action(key_char, is_press)
        except AttributeError:
            pass  # 忽略非字符按键

    def _handle_key_action(self, key_char, is_press):
        """处理有效按键事件"""
        for btn_id, config in self.key_bindings.items():
            if key_char == config['key']:
                self._update_ui(btn_id, is_press)
                if self.use_hardware:
                    self._control_hardware(btn_id, is_press)
                self._log_action(key_char, is_press)

    def _update_ui(self, btn_id, is_press):
        """更新界面状态"""
        config = self.key_bindings[btn_id]
        state = "已按下" if is_press else "未按下"
        color = config['color'] if is_press else "gray"
        self.status_labels[btn_id].config(
            text=f"{btn_id}: {state}",
            foreground=color
        )

    def _control_hardware(self, btn_id, is_press):
        """硬件控制接口 - 添加更多错误处理"""
        try:
            if not self.use_hardware:
                return

            config = self.key_bindings.get(btn_id, None)
            if not config:
                logging.error(f"未知按钮ID: {btn_id}")
                return

            num_id = config.get('btn_number')
            if num_id is None:
                logging.error(f"按钮 {btn_id} 缺少btn_number配置")
                return

            # 在日志中添加调试信息
            logging.info(f"硬件映射: {btn_id} => 编号{num_id} ({'按下' if is_press else '释放'})")

            if is_press:
                self.controller.simulate_press(num_id)
            else:
                self.controller.simulate_release(num_id)
        except Exception as e:
            logging.error(f"硬件控制失败: {str(e)}")

    def _log_action(self, key_char, is_press):
        """记录标准化日志"""
        action = "按下" if is_press else "释放"
        logging.info(f"{action} {key_char}")

    def load_config(self):
        """加载配置文件"""
        try:
            with open("key_bindings.json", 'r') as f:
                config = json.load(f)
                return {item['id']: item for item in config.get('mappings', [])}
        except Exception as e:
            logging.error(f"配置加载失败: {str(e)}")
            return {
                "btn1": {"id": "btn1", "key": "a", "color": "red", "btn_number": 0 },
                "btn2": {"id": "btn2", "key": "b", "color": "orange", "btn_number": 1}
            }

    def _update_binding(self, btn_id):
        """更新按键绑定"""
        if btn_id not in self.entries:
            logging.error("无效的按钮ID")
            return

        new_key = self.entries[btn_id].get().strip().lower()
        # 新增冲突检测
        existing = [k for k,v in self.key_bindings.items() if v['key'] == new_key]
        if existing:
            logging.error(f"按键 {new_key} 已被 {existing[0]} 占用")
            return

        if len(new_key) == 1 and new_key.isalpha():
            self.key_bindings[btn_id]['key'] = new_key
            self._save_config()
            logging.info(f"已更新 {btn_id} 绑定到：{new_key}")
        else:
            logging.error("无效按键，请输入单个字母")

    def _save_config(self):
        """保存配置文件"""
        config = {"mappings": list(self.key_bindings.values())}
        with open("key_bindings.json", 'w') as f:
            json.dump(config, f, indent=4)

    def _setup_logging(self):
        """配置日志系统"""
        logging.basicConfig(
            level=logging.INFO,
            format="%(asctime)s - %(message)s",
            handlers=[
                self.LogWindowHandler(self.log_area),
                logging.FileHandler("./output/key_events.log")
            ]
        )
        logging.info(f"启动成功 - 运行模式: {self.current_mode}")

    def _safe_shutdown(self):
        """安全关闭程序"""
        self.processing_flag.clear()
        if self.listener.is_alive():
            self.listener._suppress = True  # pynput特殊处理
            self.listener.stop()
        self.process_thread.join(timeout=1)
        self._save_config()
        self.root.destroy()
        if self.process_thread.is_alive():
            logging.error("事件处理线程未正常退出")

    class LogWindowHandler(logging.Handler):
        """自定义日志显示器"""
        def __init__(self, text_widget):
            super().__init__()
            self.text_widget = text_widget

        def emit(self, record):
            msg = self.format(record)
            self.text_widget.configure(state="normal")
            self.text_widget.insert(tk.END, msg + "\n")
            self.text_widget.see(tk.END)
            self.text_widget.configure(state="disabled")

if __name__ == "__main__":
    import argparse
    import os
    import sys

    parser = argparse.ArgumentParser(description="嵌入式按键模拟器")
    # parser.add_argument("--hardware", action="store_true", help="启用硬件模式")
    parser.add_argument("--mode", choices=["hardware", "software"], default="software",
                        help="运行模式: hardware(硬件仿真)或software(纯软件模拟), 默认是software")
    args = parser.parse_args()
    use_hardware = (args.mode == "hardware")

    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.abspath(os.path.join(script_dir, '..'))
    os.chdir(project_root)
    print("脚本路径:", script_dir)
    print("上级目录:", project_root)
    print(f"当前工作目录已固定为：{os.getcwd()}")

    app = DynamicKeySimulator(use_hardware=use_hardware)
    app.root.mainloop()