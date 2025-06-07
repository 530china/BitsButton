import ctypes
import os
import platform
import sys
from threading import Thread
import time
import logging

class ButtonController:
    def __init__(self, ui_logger=None):
        # 创建独立的日志记录器，不连接到 UI
        self.logger = logging.getLogger('ButtonController')
        self.logger.propagate = False  # 防止传播到根日志器
        
        # 如果没有处理器，添加控制台处理器
        if not self.logger.handlers:
            console_handler = logging.StreamHandler()
            formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
            console_handler.setFormatter(formatter)
            self.logger.addHandler(console_handler)
            self.logger.setLevel(logging.INFO)
        
        self.logger.info("初始化 ButtonController...")
        
        # macOS 特殊处理
        self.is_mac = platform.system() == 'Darwin'
        self.logger.info(f"当前系统: {platform.system()} (macOS? {self.is_mac})")
        
        # 加载库
        self.lib = self._load_platform_lib()
        
        # 初始化API
        self._init_api()
        
        # 启动线程
        self._start_tick_thread()
        self.logger.info("ButtonController 初始化完成")

    def _load_platform_lib(self):
        """加载平台特定的动态库"""
        # 确定库文件名
        system = platform.system()
        lib_name = {
            'Windows': 'button.dll',
            'Darwin': 'libbutton.dylib',  # macOS
            'Linux': 'libbutton.so'
        }.get(system, 'libbutton.so')  # 默认为.so
        
        self.logger.info(f"平台检测: {system}, 库名: {lib_name}")
        
        # 尝试查找库路径
        possible_paths = [
            # 1. 项目相对路径
            os.path.join(os.path.dirname(__file__), "../output/", lib_name),
            # 2. 绝对路径备用位置
            os.path.join("/usr/local/lib", lib_name),
            # 3. 系统库路径
            lib_name  # 默认名称，让系统尝试解析
        ]
        
        # 尝试每种路径
        for lib_path in possible_paths:
            try:
                absolute_path = os.path.abspath(lib_path)
                self.logger.info(f"尝试加载库: {absolute_path}")
                
                # macOS 需要特殊处理加载
                if system == 'Darwin':
                    # 使用默认加载机制
                    return ctypes.CDLL(lib_path)
                
                # 常规加载
                return ctypes.CDLL(lib_path)
            except Exception as e:
                self.logger.warning(f"加载失败: {str(e)}")
        
        # 所有尝试都失败
        raise OSError(f"无法加载平台库: {lib_name}。请确保库在搜索路径中。")

    def _init_api(self):
        """初始化API函数"""
        if not self.lib:
            raise RuntimeError("无法初始化API - 库未加载")
            
        self.logger.info("初始化API函数...")
        
        # 基本函数
        try:
            self.lib.button_ticks_wrapper.restype = None
            self.lib.set_key_state.argtypes = [ctypes.c_uint8, ctypes.c_uint8]
            self.logger.info("基本API函数初始化成功")
        except AttributeError as e:
            self.logger.error(f"缺少基本API函数: {str(e)}")
            raise
            
        # 可选函数
        try:
            if hasattr(self.lib, 'button_init'):
                self.lib.button_init.restype = ctypes.c_int
                if self.lib.button_init() == 0:
                    self.logger.info("硬件初始化成功")
                else:
                    self.logger.warning("硬件初始化返回错误")
            else:
                self.logger.info("未找到button_init函数，跳过初始化")
        except Exception as e:
            self.logger.error(f"初始化失败: {str(e)}")

    def _start_tick_thread(self):
        """启动tick线程"""
        def tick_loop():
            # 使用独立的线程日志记录器
            thread_logger = logging.getLogger('ButtonController.TickThread')
            thread_logger.propagate = False
            
            if not thread_logger.handlers:
                console_handler = logging.StreamHandler()
                formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
                console_handler.setFormatter(formatter)
                thread_logger.addHandler(console_handler)
                thread_logger.setLevel(logging.INFO)
            
            thread_logger.info("硬件tick线程启动")
            
            # Windows高精度定时器
            winmm = None
            if platform.system() == 'Windows':
                try:
                    winmm = ctypes.WinDLL('winmm')
                    winmm.timeBeginPeriod(1)
                    thread_logger.info("启用Windows高精度定时器")
                except Exception as e:
                    thread_logger.error(f"Windows定时器设置失败: {str(e)}")
            
            try:
                interval = 0.005  # 5ms
                next_time = time.perf_counter()
                
                while True:
                    try:
                        # 调用硬件tick函数
                        self.lib.button_ticks_wrapper()
                    except Exception as e:
                        thread_logger.error(f"调用tick失败: {str(e)}")
                        break
                    
                    # 计算等待时间
                    current_time = time.perf_counter()
                    wait_time = next_time - current_time
                    
                    # 等待策略
                    if wait_time > 0.001:  # 超过1ms使用sleep
                        time.sleep(wait_time * 0.95)
                    
                    # 自旋等待
                    while time.perf_counter() < next_time:
                        pass
                    
                    # 设置下一个tick时间
                    next_time += interval
            finally:
                # 清理资源
                if winmm:
                    try:
                        winmm.timeEndPeriod(1)
                        thread_logger.info("恢复Windows定时器设置")
                    except Exception as e:
                        thread_logger.error(f"恢复定时器失败: {str(e)}")
                
                thread_logger.info("硬件tick线程退出")
        
        # 启动线程
        thread = Thread(target=tick_loop, daemon=True)
        thread.name = "HardwareTickThread"
        thread.start()
        return thread

    def simulate_press(self, button_id):
        """触发按键按下事件"""
        try:
            self.logger.debug(f"模拟按键按下: ID={button_id}")
            self.lib.set_key_state(button_id, 1)
            return True
        except Exception as e:
            self.logger.error(f"按键按下失败: {str(e)}")
            return False

    def simulate_release(self, button_id):
        """触发按键释放事件"""
        try:
            self.logger.debug(f"模拟按键释放: ID={button_id}")
            self.lib.set_key_state(button_id, 0)
            return True
        except Exception as e:
            self.logger.error(f"按键释放失败: {str(e)}")
            return False
    
    def cleanup(self):
        """清理资源"""
        try:
            if hasattr(self.lib, 'button_cleanup'):
                self.lib.button_cleanup()
                self.logger.info("硬件资源清理完成")
            else:
                self.logger.info("未找到button_cleanup函数，跳过清理")
        except Exception as e:
            self.logger.error(f"硬件清理失败: {str(e)}")