import ctypes
import os
import platform
from threading import Thread
import time

class ButtonController:
    def __init__(self):
        self.lib = self._load_platform_lib()
        self._init_api()
        self._start_tick_thread()

    def _load_platform_lib(self):
        lib_name = "button.dll" if platform.system() == "Windows" else "libbutton.so"
        lib_path = os.path.join(
            os.path.dirname(__file__),
            "../output/",
            lib_name
        )
        return ctypes.CDLL(lib_path)

    def _init_api(self):
        # 定义函数原型
        self.lib.button_ticks_wrapper.restype = None
        self.lib.set_key_state.argtypes = [ctypes.c_uint8, ctypes.c_uint8]

    def _start_tick_thread(self):
        def tick_loop():
            # 设置高精度定时器（仅Windows）
            if platform.system() == 'Windows':
                winmm = ctypes.WinDLL('winmm')
                winmm.timeBeginPeriod(1)

            try:
                interval = 0.005  # 5ms
                next_time = time.perf_counter()

                while True:
                    # 调用硬件tick函数
                    self.lib.button_ticks_wrapper()

                    # 计算需要等待的时间
                    wait_time = next_time - time.perf_counter()

                    # 等待策略
                    if wait_time > 0.001:  # 超过1ms使用sleep
                        time.sleep(wait_time * 0.95)  # 睡眠95%的时间

                    # 自旋等待剩余时间
                    while time.perf_counter() < next_time:
                        pass

                    # 设置下一个周期点
                    next_time += interval
            finally:
                # 恢复默认定时器（仅Windows）
                if platform.system() == 'Windows':
                    winmm.timeEndPeriod(1)

        # 启动守护线程
        thread = Thread(target=tick_loop, daemon=True)
        thread.start()
        return thread

    def simulate_press(self, button_id):
        """触发按键按下事件"""
        self.lib.set_key_state(button_id, 1)

    def simulate_release(self, button_id):
        """触发按键释放事件"""
        self.lib.set_key_state(button_id, 0)