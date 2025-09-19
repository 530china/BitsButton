import tkinter as tk
from tkinter import ttk, scrolledtext, messagebox
import json
import logging
import queue
import threading
import os
import sys
import time
from pynput import keyboard

class LogWindowHandler(logging.Handler):
    """自定义线程安全的日志显示器"""
    def __init__(self, text_widget):
        super().__init__()
        self.text_widget = text_widget
        self.log_queue = queue.Queue()
        self.root = None  # 稍后设置
        
        # 启动队列处理线程
        self.running = True
        self.thread = threading.Thread(target=self.process_queue, daemon=True)
        self.thread.start()

    def set_root(self, root):
        """设置Tk根窗口"""
        self.root = root

    def emit(self, record):
        """将日志记录放入队列"""
        try:
            msg = self.format(record)
            self.log_queue.put(msg)
        except Exception:
            self.handleError(record)

    def process_queue(self):
        """处理日志队列"""
        while self.running:
            try:
                msg = self.log_queue.get(timeout=0.1)
                if self.root:
                    # 在主线程中安全地更新UI
                    self.root.after(0, self._update_text_widget, msg)
            except queue.Empty:
                continue
            except Exception:
                self.handleError(None)

    def _update_text_widget(self, msg):
        """更新文本组件（在主线程中执行）"""
        try:
            self.text_widget.configure(state="normal")
            self.text_widget.insert(tk.END, msg + "\n")
            self.text_widget.see(tk.END)
            self.text_widget.configure(state="disabled")
        except Exception as e:
            print(f"更新文本组件失败: {str(e)}")

    def close(self):
        """关闭处理器"""
        self.running = False
        self.thread.join(timeout=1)
        super().close()

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
        self.root.title("高级按键模拟器 v3.4")
        self._create_ui()
        
        # 创建日志处理器
        self.log_window_handler = LogWindowHandler(self.log_area)
        self.log_window_handler.setLevel(logging.INFO)
        
        # 配置日志系统
        self._setup_logging()
        
        # 设置日志处理器的根窗口
        self.log_window_handler.set_root(self.root)

        self._init_hardware_mode()

        # 事件监听器
        self._start_keyboard_listener()
        self._start_event_processor()

        # 关闭事件绑定
        self.root.protocol("WM_DELETE_WINDOW", self._safe_shutdown)

    def _init_hardware_mode(self):
        """硬件模式初始化"""
        if self.use_hardware:
            try:
                # 获取当前脚本路径
                script_dir = os.path.dirname(os.path.abspath(__file__))
                
                # 添加库搜索路径
                sys.path.insert(0, os.path.join(script_dir, ".."))
                
                # 初始化日志信息
                logging.info("尝试导入硬件模块...")
                logging.info(f"系统路径: {sys.path}")
                
                # 导入硬件控制模块
                from button_ctrl import ButtonController
                # 创建独立的日志记录器
                self.controller = ButtonController()
                logging.info("硬件控制器初始化成功")
            except ImportError as e:
                self.use_hardware = False
                self.current_mode = "软件模式（硬件不可用）"
                self.root.title(f"高级按键模拟器 v3.4 - {self.current_mode}")
                logging.error(f"硬件模块加载失败: {str(e)}")
                
                # 显示错误通知
                messagebox.showerror("硬件加载失败", 
                                    f"无法加载硬件控制模块:\n{str(e)}\n\n"
                                    "已自动切换到软件模式。")
            except Exception as e:
                import traceback
                error_details = traceback.format_exc()
                self.use_hardware = False
                self.current_mode = "软件模式（硬件异常）"
                self.root.title(f"高级按键模拟器 v3.4 - {self.current_mode}")
                logging.error(f"硬件控制器初始化异常: {str(e)}\n{error_details}")
                
                # 显示详细的错误信息
                messagebox.showerror(
                    "硬件初始化失败",
                    f"硬件控制器初始化失败:\n\n{str(e)}\n\n"
                    f"详细信息:\n{error_details}"
                )

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
        """启动非阻塞式键盘监听，使用通用的错误处理策略"""
        def _on_event(event_type, key):
            try:
                self.event_queue.put((event_type, key))
            except Exception as e:
                logging.debug(f"事件队列异常: {str(e)}")
        
        def _safe_event_handler(handler_name):
            """创建安全的事件处理器包装器"""
            def wrapper(key):
                try:
                    _on_event(handler_name, key)
                except Exception as e:
                    # 忽略所有事件处理异常，防止程序崩溃
                    logging.debug(f"{handler_name}事件处理异常: {str(e)}")
            return wrapper
        
        max_retries = 3
        for attempt in range(max_retries):
            try:
                self.listener = keyboard.Listener(
                    on_press=_safe_event_handler('press'),
                    on_release=_safe_event_handler('release')
                )
                self.listener.daemon = True
                self.listener.start()
                logging.info("键盘监听器启动成功")
                return  # 成功返回
                
            except Exception as e:
                error_msg = str(e).lower()
                if attempt < max_retries - 1:
                    logging.warning(f"键盘监听器启动失败，重试 ({attempt + 1}/{max_retries}): {str(e)}")
                    time.sleep(1)  # 稍等片刻再重试
                else:
                    # 最后一次尝试失败，显示用户友好的错误信息
                    if "trusted" in error_msg or "accessibility" in error_msg:
                        self._show_permission_error()
                    else:
                        self._show_generic_error(str(e))
    
    def _show_permission_error(self):
        """显示权限错误对话框"""
        error_msg = """🔒 需要辅助功能权限

为了监听键盘事件，需要给应用程序授予辅助功能权限。

请按以下步骤操作：
1. 打开 系统设置
2. 进入 隐私与安全 > 辅助功能
3. 点击左下角的锁图标解锁
4. 添加以下应用：
   - Terminal.app
   - Python.app (如果存在)
   - 或者你正在使用的终端应用

设置完成后，请重新启动模拟器。

点击“确定”打开系统设置页面。"""
        
        result = messagebox.askokcancel("权限请求", error_msg)
        if result:
            try:
                import subprocess
                subprocess.run([
                    "open", 
                    "x-apple.systempreferences:com.apple.preference.security?Privacy_Accessibility"
                ])
            except Exception as e:
                logging.error(f"无法打开设置页面: {str(e)}")
    
    def _show_generic_error(self, error_msg):
        """显示通用错误对话框"""
        messagebox.showerror(
            "键盘监听器错误",
            f"键盘监听器启动失败：\n\n{error_msg}\n\n"
            f"请尝试重新启动模拟器或检查系统权限。"
        )

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
                    # 使用通用的异常处理，不依赖特定错误信息
                    error_msg = str(e).lower()
                    if any(keyword in error_msg for keyword in [
                        'not callable', 'threadhandle', 'mach_port', 
                        'compatibility', 'version'
                    ]):
                        # 这些都是兼容性问题，记录但不中断程序
                        logging.debug(f"检测到兼容性问题，忽略: {str(e)}")
                    else:
                        logging.warning(f"事件处理异常: {str(e)}")
                    # 继续循环，不中断程序

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
            if not self.use_hardware or not hasattr(self, 'controller'):
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
            config_path = "key_bindings.json"
            if os.path.exists(config_path):
                with open(config_path, 'r') as f:
                    config = json.load(f)
                    return {item['id']: item for item in config.get('mappings', [])}
            else:
                logging.warning("配置文件不存在，使用默认配置")
        except Exception as e:
            logging.error(f"配置加载失败: {str(e)}")
        
        # 默认配置
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
        existing = [k for k,v in self.key_bindings.items() if v['key'] == new_key and k != btn_id]
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
        config_path = "key_bindings.json"
        with open(config_path, 'w') as f:
            json.dump(config, f, indent=4)

    def _setup_logging(self):
        """配置日志系统"""
        # 首先清除任何已有的日志处理器
        for handler in logging.root.handlers[:]:
            logging.root.removeHandler(handler)
        
        # 配置日志格式
        formatter = logging.Formatter("%(asctime)s - %(message)s")
        
        # 文件处理器
        log_dir = "output"
        os.makedirs(log_dir, exist_ok=True)
        file_handler = logging.FileHandler(os.path.join(log_dir, "key_events.log"))
        file_handler.setFormatter(formatter)
        
        # 将UI日志处理器添加到根日志记录器
        self.log_window_handler.setFormatter(formatter)
        logging.getLogger().addHandler(self.log_window_handler)
        logging.getLogger().addHandler(file_handler)
        logging.getLogger().setLevel(logging.INFO)
        
        logging.info(f"启动成功 - 运行模式: {self.current_mode}")

    def _safe_shutdown(self):
        """安全关闭程序"""
        self.processing_flag.clear()
        
        # 关闭日志处理器
        if hasattr(self, 'log_window_handler'):
            self.log_window_handler.close()
        
        # 停止键盘监听器
        if hasattr(self, 'listener') and self.listener.is_alive():
            self.listener._suppress = True
            self.listener.stop()
        
        # 停止事件处理线程
        if hasattr(self, 'process_thread') and self.process_thread.is_alive():
            self.process_thread.join(timeout=1)
        
        # 保存配置
        self._save_config()
        
        # 清理硬件资源
        if hasattr(self, 'controller'):
            try:
                self.controller.cleanup()
            except:
                pass
        
        # 关闭窗口
        self.root.destroy()
        
        # 检查线程状态
        if hasattr(self, 'process_thread') and self.process_thread.is_alive():
            logging.error("事件处理线程未正常退出")

if __name__ == "__main__":
    import argparse
    import os
    import sys

    parser = argparse.ArgumentParser(description="嵌入式按键模拟器")
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