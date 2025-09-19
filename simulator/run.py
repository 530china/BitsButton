#!/usr/bin/env python3
"""
BitsButton 模拟器一体化构建和运行脚本 - 支持 Windows/Linux/macOS
"""

import sys
import os
import subprocess
import argparse
import platform
import venv
import shutil
import logging
import time

# 配置日志
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
    handlers=[logging.StreamHandler()]
)
logger = logging.getLogger('SimulatorBuilder')

class SimulatorBuilder:
    def __init__(self, mode='software'):
        # 确定根路径
        self.root_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
        self.simulator_dir = os.path.dirname(os.path.abspath(__file__))
        self.output_dir = os.path.join(self.simulator_dir, 'output')
        self.python_simulator_dir = os.path.join(self.simulator_dir, 'python_simulator')
        self.venv_dir = os.path.join(self.simulator_dir, '.venv')
        self.os_name = platform.system()

        # 确保输出目录存在
        os.makedirs(self.output_dir, exist_ok=True)

        # 设置模式
        self.mode = mode
        logger.info(f"运行模式: {self.mode}")

        # 平台特定的设置
        self.library_name = self.get_library_name()
        self.python_exec = self.get_python_executable()
        self.compiler = self.detect_compiler()

        if self.compiler:
            logger.info(f"使用编译器: {self.compiler}")
        else:
            logger.warning("未找到C编译器!")

        # 设置库搜索路径
        self.set_library_search_path()

    def set_library_search_path(self):
        """设置库搜索路径"""
        path_sep = ';' if self.os_name == 'Windows' else ':'

        # Windows
        if self.os_name == 'Windows':
            os.environ['PATH'] = f"{self.output_dir}{path_sep}{os.environ.get('PATH', '')}"

        # Linux
        elif self.os_name == 'Linux':
            os.environ['LD_LIBRARY_PATH'] = f"{self.output_dir}{path_sep}{os.environ.get('LD_LIBRARY_PATH', '')}"

        # macOS
        elif self.os_name == 'Darwin':
            # macOS 需要在构建时指定rpath
            os.environ['DYLD_LIBRARY_PATH'] = self.output_dir

        logger.info(f"库搜索路径已设置: {self.output_dir}")

    def get_library_name(self):
        """根据操作系统确定库名称"""
        if self.os_name == 'Windows':
            return "button.dll"
        elif self.os_name == 'Darwin':  # macOS
            return "libbutton.dylib"
        else:  # Linux and others
            return "libbutton.so"

    def get_python_executable(self):
        """获取Python可执行文件路径"""
        if self.os_name == 'Windows':
            return os.path.join(self.venv_dir, 'Scripts', 'python.exe')
        else:
            return os.path.join(self.venv_dir, 'bin', 'python')

    def detect_compiler(self):
        """检测可用的C编译器"""
        compilers = []

        if self.os_name == 'Windows':
            compilers = ['gcc', 'clang', 'cl']  # MinGW GCC, LLVM Clang, Microsoft CL
        elif self.os_name == 'Darwin':  # macOS
            compilers = ['clang', 'gcc']
        else:  # Linux
            compilers = ['gcc', 'clang']

        for compiler in compilers:
            try:
                subprocess.run([compiler, '--version'], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
                return compiler
            except (OSError, FileNotFoundError):
                continue

        return None

    def build_library(self):
        """构建共享库"""
        if self.mode == 'software':
            logger.info("软件模式 - 跳过库构建")
            return True

        if not self.compiler:
            logger.error("未找到可用的C编译器! 切换到软件模式")
            self.mode = 'software'
            return False

        logger.info("构建硬件模式共享库...")

        # 源文件路径
        adapter_src = os.path.join(self.simulator_dir, 'adapter_layer', 'button_adapter.c')
        core_src = os.path.join(self.root_dir, 'bits_button.c')
        output_lib = os.path.join(self.output_dir, self.library_name)

        # 检查源文件是否存在
        for src in [adapter_src, core_src]:
            if not os.path.exists(src):
                logger.error(f"源文件不存在: {src}")
                self.mode = 'software'
                return False

        # 确定编译器选项
        compile_options = []
        if self.compiler in ['gcc', 'clang']:
            compile_options = ['-fPIC', '-shared', '-o', output_lib, adapter_src, core_src]

            # macOS 需要额外的rpath设置
            if self.os_name == 'Darwin':
                compile_options.extend(['-Wl,-install_name,@rpath/' + self.library_name])

            if self.compiler == 'clang':
                compile_options.insert(0, '-Wno-everything')  # Clang 可能会报告更多警告

        elif self.compiler == 'cl' and self.os_name == 'Windows':  # MSVC
            compile_options = ['/LD', f'/Fe{output_lib}', adapter_src, core_src]

        # 执行编译
        try:
            logger.info(f"执行: {self.compiler} {' '.join(compile_options)}")
            result = subprocess.run(
                [self.compiler] + compile_options,
                stderr=subprocess.PIPE,
                stdout=subprocess.PIPE,
                text=True
            )

            if result.stdout:
                logger.debug(f"编译输出:\n{result.stdout}")
            if result.stderr:
                logger.debug(f"编译错误:\n{result.stderr}")

            if os.path.exists(output_lib):
                logger.info(f"构建成功: {output_lib}")
                return True
            else:
                logger.error("构建完成但未生成库文件! 切换到软件模式")
                self.mode = 'software'
                return False
        except Exception as e:
            logger.error(f"编译失败: {str(e)}")
            logger.info("切换到软件模式")
            self.mode = 'software'
            return False

    def check_tkinter_availability(self):
        """检查tkinter是否可用，如果不可用则尝试修复"""
        # 获取正确的Python命令
        python_cmd = 'python3' if self.os_name != 'Windows' else 'python'

        try:
            # 使用系统Python测试tkinter
            result = subprocess.run(
                [python_cmd, '-c', 'import tkinter; print("tkinter_ok")'],
                capture_output=True, text=True, timeout=10
            )
            if result.returncode == 0 and 'tkinter_ok' in result.stdout:
                logger.info("tkinter 可用")
                return True
            else:
                logger.warning("tkinter 不可用，尝试修复...")
                return self.fix_tkinter_issue()
        except Exception as e:
            logger.error(f"检查tkinter时出错: {str(e)}")
            return self.fix_tkinter_issue()

    def check_permissions(self):
        """检查macOS权限，使用通用方法不依赖特定版本"""
        if self.os_name != 'Darwin':
            return True

        logger.info("检查macOS辅助功能权限...")

        try:
            # 使用通用的检测方法，不依赖Python版本
            test_script = '''
import sys
try:
    from pynput import keyboard
    # 创建一个简单的监听器测试
    def dummy_handler(key):
        pass

    listener = keyboard.Listener(
        on_press=dummy_handler,
        on_release=dummy_handler
    )
    listener.start()
    listener.stop()
    print("permission_test_passed")
except Exception as e:
    if "trusted" in str(e).lower() or "accessibility" in str(e).lower():
        print("permission_denied")
    else:
        print(f"other_error:{str(e)}")
'''

            result = subprocess.run(
                [self.python_exec, '-c', test_script],
                capture_output=True, text=True, timeout=15
            )

            output = result.stdout.strip()
            if "permission_test_passed" in output:
                logger.info("✅ 权限检查通过")
                return True
            elif "permission_denied" in output:
                logger.warning("⚠️  需要辅助功能权限")
                self._show_permission_guidance()
                return False
            elif "other_error" in output:
                error_detail = output.replace("other_error:", "")
                logger.warning(f"检测过程中出现其他问题: {error_detail}")
                return True  # 非权限问题，继续运行
            else:
                logger.warning("权限检查结果不明确，继续运行")
                return True

        except subprocess.TimeoutExpired:
            logger.warning("权限检查超时，继续运行")
            return True
        except Exception as e:
            logger.warning(f"权限检查失败: {str(e)}，继续运行")
            return True

    def _show_permission_guidance(self):
        """显示权限指导"""
        logger.info("❌ macOS辅助功能权限不足")
        logger.info("🛠️  解决方案：")
        logger.info("   1. 打开 系统设置")
        logger.info("   2. 进入 隐私与安全 > 辅助功能")
        logger.info("   3. 点击左下角的锁图标解锁")
        logger.info("   4. 添加并勾选 Terminal.app 或当前终端应用")
        logger.info("   5. 重新运行此脚本")
        logger.info("")
        logger.info("🚀 自动打开设置页面...")

        try:
            subprocess.run([
                "open",
                "x-apple.systempreferences:com.apple.preference.security?Privacy_Accessibility"
            ])
        except Exception as e:
            logger.error(f"无法打开设置页面: {str(e)}")

    def fix_tkinter_issue(self):
        """尝试修复tkinter问题"""
        if self.os_name == 'Darwin':
            logger.info("检测到macOS tkinter问题，尝试自动修复...")
            return self._fix_macos_tkinter()
        elif self.os_name == 'Windows':
            logger.info("检测到Windows tkinter问题")
            return self._fix_windows_tkinter()
        else:
            logger.info("检测到Linux tkinter问题")
            return self._fix_linux_tkinter()

    def _fix_windows_tkinter(self):
        """修复Windows tkinter问题"""
        logger.info("📝 Windows tkinter 修复指南:")
        logger.info("   1. 确保使用完整的Python安装（从 python.org 下载）")
        logger.info("   2. 在安装时勾选 'tcl/tk and IDLE' 选项")
        logger.info("   3. 或者使用: pip install tk")
        logger.info("   4. 如果使用Anaconda: conda install tk")
        logger.info("")

        # 尝试使用pip安装tk
        try:
            logger.info("尝试使用pip安装tk...")
            subprocess.run(['pip', 'install', 'tk'], check=True,
                         capture_output=True, text=True)

            # 验证修复结果
            result = subprocess.run(
                ['python', '-c', 'import tkinter; print("修复成功")'],
                capture_output=True, text=True, timeout=10
            )

            if result.returncode == 0:
                logger.info("✅ tkinter 修复成功！")
                return True
            else:
                logger.warning("⚠️  pip安装无效，需要手动处理")
                return False

        except subprocess.CalledProcessError:
            logger.warning("⚠️  pip安装失败，需要重新安装Python并包含Tk组件")
            return False
        except Exception as e:
            logger.error(f"修复过程中出错: {str(e)}")
            return False

    def _fix_linux_tkinter(self):
        """修复Linux tkinter问题"""
        logger.info("📝 Linux tkinter 修复指南:")
        logger.info("   Ubuntu/Debian: sudo apt-get install python3-tk")
        logger.info("   CentOS/RHEL: sudo yum install tkinter")
        logger.info("   Fedora: sudo dnf install python3-tkinter")
        logger.info("   Arch: sudo pacman -S tk")
        logger.info("")

        # 尝试自动安装（Ubuntu/Debian）
        try:
            subprocess.run(['sudo', 'apt-get', 'install', '-y', 'python3-tk'],
                         check=True, capture_output=True, text=True)
            logger.info("✅ 安装成功！")
            return True
        except (subprocess.CalledProcessError, FileNotFoundError):
            logger.warning("⚠️  自动安装失败，请手动安装")
            return False

    def _fix_macos_tkinter(self):
        """修复macOS tkinter问题"""
        try:
            # 获取当前Python版本
            version_result = subprocess.run(
                ['python3', '--version'], capture_output=True, text=True
            )
            version_str = version_result.stdout.strip()

            # 提取主要版本号
            import re
            version_match = re.search(r'Python (\d+\.\d+)', version_str)
            if not version_match:
                logger.error("无法确定Python版本")
                return False

            python_version = version_match.group(1)
            tk_package = f"python-tk@{python_version}"

            logger.info(f"当前Python版本: {version_str}")
            logger.info(f"尝试安装: {tk_package}")

            # 检查是否已安装
            check_result = subprocess.run(
                ['brew', 'list', tk_package],
                capture_output=True, text=True
            )

            if check_result.returncode == 0:
                logger.info(f"{tk_package} 已安装，尝试重新安装...")
                subprocess.run(['brew', 'reinstall', tk_package], check=True)
            else:
                logger.info(f"安装 {tk_package}...")
                subprocess.run(['brew', 'install', tk_package], check=True)

            # 验证修复结果
            time.sleep(2)
            result = subprocess.run(
                ['python3', '-c', 'import tkinter; print("修复成功")'],
                capture_output=True, text=True, timeout=10
            )

            if result.returncode == 0:
                logger.info("✅ tkinter 修复成功！")
                return True
            else:
                logger.error("❌ tkinter 修复失败")
                return False

        except subprocess.CalledProcessError as e:
            logger.error(f"修复过程中命令执行失败: {str(e)}")
            logger.info("建议手动执行: brew install python-tk@<version>")
            return False
        except Exception as e:
            logger.error(f"修复tkinter时出现异常: {str(e)}")
            return False

    def setup_virtual_environment(self):
        """设置Python虚拟环境"""
        # 如果虚拟环境存在，检查是否需要重建
        if os.path.exists(self.venv_dir):
            # 检查虚拟环境是否健康
            try:
                result = subprocess.run(
                    [self.python_exec, '--version'],
                    capture_output=True, text=True, timeout=5
                )
                if result.returncode == 0:
                    logger.info("使用已有的虚拟环境")
                    return True
                else:
                    logger.warning("虚拟环境损坏，重新创建...")
                    shutil.rmtree(self.venv_dir)
            except Exception:
                logger.warning("虚拟环境检查失败，重新创建...")
                shutil.rmtree(self.venv_dir)

        logger.info("创建Python虚拟环境...")
        try:
            venv.create(self.venv_dir, with_pip=True)
            logger.info(f"虚拟环境已创建: {self.venv_dir}")
            return True
        except Exception as e:
            logger.error(f"创建虚拟环境失败: {str(e)}")
            return False

    def install_dependencies(self):
        """安装Python依赖"""
        # 检查pip是否存在
        pip_cmd = [self.python_exec, '-m', 'pip']

        # 使用通用的pynput安装策略，而不是版本特定的判断
        pynput_packages = [
            'pynput>=1.7.6',  # 首先尝试最新版本
            'pynput==1.7.6',  # 如果失败，尝试稳定版本
        ]

        # 尝试安装依赖，最多重试3次
        for attempt in range(3):
            try:
                # 升级pip
                subprocess.run(
                    pip_cmd + ['install', '--upgrade', 'pip'],
                    check=True,
                    stdout=subprocess.DEVNULL,
                    stderr=subprocess.DEVNULL
                )

                # 尝试安装pynput，使用版本降级策略
                pynput_installed = False
                for package in pynput_packages:
                    try:
                        logger.info(f"尝试安装 {package}...")
                        subprocess.run(
                            pip_cmd + ['install', package],
                            check=True,
                            stdout=subprocess.DEVNULL,
                            stderr=subprocess.DEVNULL
                        )
                        logger.info(f"成功安装 {package}")
                        pynput_installed = True
                        break
                    except subprocess.CalledProcessError:
                        logger.warning(f"{package} 安装失败，尝试下一个版本...")
                        continue

                if pynput_installed:
                    logger.info("依赖安装完成")
                    return True
                else:
                    raise Exception("所有pynput版本都安装失败")

            except subprocess.CalledProcessError as e:
                if attempt < 2:  # 重试最多3次
                    logger.warning(f"安装依赖失败，尝试重试 ({attempt+1}/3)...")
                    time.sleep(2)
                else:
                    logger.error(f"安装依赖失败: {str(e)}")
                    return False
            except Exception as e:
                if attempt < 2:
                    logger.warning(f"安装过程中出现问题，尝试重试 ({attempt+1}/3): {str(e)}")
                    time.sleep(2)
                else:
                    logger.error(f"安装依赖失败: {str(e)}")
                    return False
        return False

    def check_python_version(self):
        """检查Python版本是否符合要求"""
        try:
            result = subprocess.run(
                [self.python_exec, '--version'],
                capture_output=True, text=True
            )
            version_str = result.stdout.strip()
            version_parts = version_str.split()[1].split('.')
            major, minor = int(version_parts[0]), int(version_parts[1])

            if major == 3 and minor >= 6:
                logger.info(f"Python版本符合要求: {version_str}")
                return True
            else:
                logger.error(f"需要Python 3.6+，当前版本: {version_str}")
                return False
        except Exception as e:
            logger.error(f"检查Python版本失败: {str(e)}")
            return False

    def run_simulator(self):
        """运行模拟器"""
        # 确定模式
        sim_mode = 'hardware' if self.mode != 'software' else 'software'

        # 设置环境变量
        env = os.environ.copy()
        path_sep = ';' if self.os_name == 'Windows' else ':'

        # 设置库路径
        if self.os_name == 'Windows':
            env['PATH'] = f"{self.output_dir}{path_sep}{env.get('PATH', '')}"

        elif self.os_name == 'Linux':
            env['LD_LIBRARY_PATH'] = f"{self.output_dir}{path_sep}{env.get('LD_LIBRARY_PATH', '')}"

        elif self.os_name == 'Darwin':
            env['DYLD_LIBRARY_PATH'] = self.output_dir

        # 设置Python路径
        env['PYTHONPATH'] = f"{self.simulator_dir}{path_sep}{env.get('PYTHONPATH', '')}"

        # 设置项目根目录环境变量
        env['PROJECT_ROOT'] = self.root_dir

        # 运行模拟器脚本
        sim_script = os.path.join(self.python_simulator_dir, 'advanced_v2_sim.py')
        logger.info(f"启动模拟器: {'硬件' if sim_mode == 'hardware' else '软件'}模式")

        try:
            process = subprocess.Popen(
                [self.python_exec, sim_script, '--mode', sim_mode],
                env=env,
                cwd=self.simulator_dir
            )
            process.communicate()  # 等待进程结束
            logger.info("模拟器已退出")
            return True
        except KeyboardInterrupt:
            logger.info("用户中断模拟器运行")
            return True
        except Exception as e:
            logger.error(f"模拟器运行失败: {str(e)}")
            return False

    def run(self):
        """主运行方法"""
        logger.info(f"项目根目录: {self.root_dir}")
        logger.info(f"模拟器目录: {self.simulator_dir}")
        logger.info(f"输出目录: {self.output_dir}")

        # 步骤0: 检查tkinter环境
        if not self.check_tkinter_availability():
            logger.error("❌ tkinter 环境检查失败")
            logger.error("请确保安装了tkinter支持，或手动执行:")
            if self.os_name == 'Darwin':
                logger.error("  brew install python-tk@<version>")
            logger.error("然后重新运行此脚本")
            return False

        # 步骤0.5: 检查macOS权限
        if not self.check_permissions():
            logger.warning("⚠️  权限检查不通过，但继续运行")
            logger.warning("如果遇到键盘监听问题，请按照上面的指导设置权限")

        # 步骤1: 设置虚拟环境
        if not self.setup_virtual_environment():
            logger.error("虚拟环境设置失败，退出")
            return False

        # 步骤2: 安装依赖
        if not self.install_dependencies():
            logger.error("依赖安装失败，退出")
            return False

        # 步骤3: 检查Python版本
        if not self.check_python_version():
            logger.warning("Python版本检查失败，但继续运行")

        # 步骤4: 构建库（如果是硬件模式）
        if self.mode != 'software':
            if not self.build_library():
                logger.warning("库构建失败，回退到软件模式")
                self.mode = 'software'

        # 步骤5: 运行模拟器
        if not self.run_simulator():
            logger.error("模拟器运行失败")
            return False

        return True

def install_compiler():
    """尝试安装必要的编译器"""
    system = platform.system()
    logger.info(f"为 {system} 安装编译器...")

    if system == 'Windows':
        logger.warning("Windows平台请手动安装MinGW: https://sourceforge.net/projects/mingw/")
        return False

    elif system == 'Darwin':  # macOS
        try:
            # 检查命令行工具是否安装
            if not os.path.exists('/Library/Developer/CommandLineTools/usr/bin/clang'):
                logger.info("安装Xcode命令行工具...")
                subprocess.run(['xcode-select', '--install'], check=True)
                return True
            else:
                logger.info("Xcode命令行工具已安装")
                return True
        except Exception as e:
            logger.error(f"在macOS上安装编译器失败: {str(e)}")
            return False

    elif system == 'Linux':
        try:
            # 尝试使用apt安装
            if shutil.which('apt'):
                logger.info("使用APT安装构建工具...")
                subprocess.run(['sudo', 'apt', 'update'], check=True)
                subprocess.run(['sudo', 'apt', 'install', '-y', 'build-essential'], check=True)
                return True

            # 尝试使用dnf安装
            if shutil.which('dnf'):
                logger.info("使用DNF安装构建工具...")
                subprocess.run(['sudo', 'dnf', 'install', '-y', 'gcc', 'gcc-c++', 'make'], check=True)
                return True

            # 尝试使用yum安装
            if shutil.which('yum'):
                logger.info("使用YUM安装构建工具...")
                subprocess.run(['sudo', 'yum', 'install', '-y', 'gcc', 'gcc-c++', 'make'], check=True)
                return True

            logger.warning("无法确定Linux发行版，请手动安装GCC编译器")
            return False
        except Exception as e:
            logger.error(f"在Linux上安装编译器失败: {str(e)}")
            return False

    else:
        logger.warning(f"不支持的系统: {system}")
        return False

def main():
    """主函数"""
    parser = argparse.ArgumentParser(description='BitsButton 模拟器构建和运行脚本')
    parser.add_argument('--mode', choices=['hardware', 'software'], default='software',
                        help='运行模式: hardware(硬件)或software(软件)，默认是software')
    parser.add_argument('--install-compiler', action='store_true',
                        help='尝试安装必要的编译器')
    parser.add_argument('--fix-env', action='store_true',
                        help='尝试修复环境问题（如tkinter缺失）')
    args = parser.parse_args()

    # 如果需要安装编译器
    if args.install_compiler:
        if install_compiler():
            logger.info("编译器安装完成! 请重新运行脚本")
        else:
            logger.error("编译器安装失败，请手动安装")
        return

    # 如果需要修复环境
    if args.fix_env:
        builder = SimulatorBuilder(mode='software')  # 临时创建来使用修复方法
        if builder.check_tkinter_availability():
            logger.info("✅ 环境修复完成! 现在可以正常运行模拟器")
        else:
            logger.error("❌ 环境修复失败")
        return

    try:
        # 创建并运行构建器
        builder = SimulatorBuilder(mode=args.mode)
        success = builder.run()

        if success:
            logger.info("✅ 运行成功完成")
            sys.exit(0)
        else:
            logger.error("❌ 运行过程中出现错误")
            logger.info("💡 提示: 如果遇到环境问题，可以尝试运行: python3 simulator/run.py --fix-env")
            sys.exit(1)
    except KeyboardInterrupt:
        logger.info("用户中断程序运行")
        sys.exit(0)
    except Exception as e:
        logger.error(f"程序运行时出现异常: {str(e)}")
        logger.info("💡 提示: 如果遇到环境问题，可以尝试运行: python3 simulator/run.py --fix-env")
        sys.exit(1)

if __name__ == '__main__':
    main()