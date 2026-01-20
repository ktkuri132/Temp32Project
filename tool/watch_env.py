#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
环境配置监视器
监视 env_config.json 的变化，自动重新验证配置
"""

import sys
import time
import json
from pathlib import Path
from datetime import datetime

try:
    from watchdog.observers import Observer
    from watchdog.events import FileSystemEventHandler
except ImportError:
    print("错误: 需要安装 watchdog 库")
    print("请运行: pip install watchdog")
    sys.exit(1)

from env_setup import EnvironmentDetector, EnvConfigManager, ColorPrint


class EnvConfigHandler(FileSystemEventHandler):
    """环境配置文件变化处理器"""

    def __init__(self, project_root: Path, debounce_seconds: float = 2.0):
        super().__init__()
        self.project_root = project_root
        self.debounce_seconds = debounce_seconds
        self.last_trigger_time = 0
        self.config_manager = EnvConfigManager(project_root)
        self.detector = EnvironmentDetector()

    def on_modified(self, event):
        """文件修改事件处理"""
        if event.is_directory:
            return

        file_name = Path(event.src_path).name
        if file_name != 'env_config.json':
            return

        # 防抖
        current_time = time.time()
        if current_time - self.last_trigger_time < self.debounce_seconds:
            return

        self.last_trigger_time = current_time
        self._verify_config()

    def _verify_config(self):
        """验证配置"""
        timestamp = datetime.now().strftime("%H:%M:%S")
        print(f"\n{'='*60}")
        print(f"[{timestamp}] 检测到配置文件变化，正在验证...")
        print(f"{'='*60}")

        # 重新加载配置
        self.config_manager.config = self.config_manager.load_config()

        # 验证配置
        valid, issues = self.config_manager.verify_config()

        if valid:
            ColorPrint.success("配置验证通过！")
            self._show_tool_status()
        else:
            ColorPrint.error("配置验证失败:")
            for issue in issues:
                print(f"  - {issue}")

            # 尝试重新检测用户配置的路径
            self._redetect_custom_paths()

        print(f"\n[监视中...] 等待配置变化 (Ctrl+C 退出)")

    def _show_tool_status(self):
        """显示工具状态"""
        config = self.config_manager.config
        tools = config.get('tools', {})

        print("\n工具状态:")
        for tool_name, tool_info in tools.items():
            status = "✓ 启用" if tool_info.get('enabled') else "✗ 禁用"
            path = tool_info.get('path', 'N/A')
            print(f"  {tool_name}: {status}")
            if tool_info.get('enabled') and path:
                print(f"    路径: {path}")

    def _redetect_custom_paths(self):
        """重新检测用户自定义的路径"""
        config = self.config_manager.config
        tools = config.get('tools', {})
        updated = False

        # 检查 ARM-GCC
        arm_gcc = tools.get('arm_gcc', {})
        if arm_gcc.get('enabled'):
            custom_path = arm_gcc.get('path', '')
            if custom_path:
                bin_path = Path(custom_path) / 'bin'
                gcc_exe = bin_path / ('arm-none-eabi-gcc.exe' if sys.platform == 'win32' else 'arm-none-eabi-gcc')
                if gcc_exe.exists():
                    arm_gcc['bin_path'] = str(bin_path)
                    # 获取版本
                    try:
                        import subprocess
                        result = subprocess.run([str(gcc_exe), '--version'], capture_output=True, text=True)
                        if result.returncode == 0:
                            arm_gcc['version'] = result.stdout.split('\n')[0]
                    except Exception:
                        pass
                    updated = True
                    ColorPrint.success(f"ARM-GCC 路径已更新: {custom_path}")
                else:
                    ColorPrint.error(f"ARM-GCC 可执行文件不存在: {gcc_exe}")

        # 检查 OpenOCD
        openocd = tools.get('openocd', {})
        if openocd.get('enabled'):
            custom_path = openocd.get('path', '')
            if custom_path:
                ocd_exe = Path(custom_path) / 'bin' / ('openocd.exe' if sys.platform == 'win32' else 'openocd')
                if ocd_exe.exists():
                    scripts_path = Path(custom_path) / 'share' / 'openocd' / 'scripts'
                    if scripts_path.exists():
                        openocd['scripts_path'] = str(scripts_path)
                    updated = True
                    ColorPrint.success(f"OpenOCD 路径已更新: {custom_path}")
                else:
                    ColorPrint.error(f"OpenOCD 可执行文件不存在: {ocd_exe}")

        if updated:
            self.config_manager.save_config(config)
            ColorPrint.info("配置已自动更新")


class EnvConfigWatcher:
    """环境配置监视器"""

    def __init__(self, project_root: str = "."):
        self.project_root = Path(project_root).resolve()
        self.tool_dir = self.project_root / "tool"
        self.observer = None

    def start(self, debounce: float = 2.0):
        """启动监视服务"""
        config_file = self.tool_dir / "env_config.json"

        print(f"""
╔══════════════════════════════════════════════════════════╗
║           环境配置监视服务 v1.0                          ║
╠══════════════════════════════════════════════════════════╣
║  项目根目录: {str(self.project_root)[:42]:<42} ║
║  配置文件:   tool/env_config.json                        ║
║  防抖时间:   {debounce:.1f} 秒{' '*36} ║
╠══════════════════════════════════════════════════════════╣
║  修改配置文件后将自动验证                                ║
║  按 Ctrl+C 停止服务                                      ║
╚══════════════════════════════════════════════════════════╝
""")

        if not config_file.exists():
            ColorPrint.warning(f"配置文件不存在: {config_file}")
            ColorPrint.info("请先运行 'python env_setup.py' 生成配置")
            return

        # 显示当前配置状态
        config_manager = EnvConfigManager(self.project_root)
        valid, issues = config_manager.verify_config()

        if valid:
            ColorPrint.success("当前配置有效")
        else:
            ColorPrint.warning("当前配置存在问题:")
            for issue in issues:
                print(f"  - {issue}")

        # 创建事件处理器
        handler = EnvConfigHandler(self.project_root, debounce_seconds=debounce)

        # 创建观察者
        self.observer = Observer()
        self.observer.schedule(handler, str(self.tool_dir), recursive=False)

        # 启动观察
        self.observer.start()
        print("\n[监视中...] 等待配置变化\n")

        try:
            while True:
                time.sleep(1)
        except KeyboardInterrupt:
            self.stop()

    def stop(self):
        """停止监视服务"""
        if self.observer:
            print("\n\n正在停止监视服务...")
            self.observer.stop()
            self.observer.join()
            print("监视服务已停止")


def main():
    import argparse

    parser = argparse.ArgumentParser(
        description='环境配置监视服务 - 自动验证配置变化',
        formatter_class=argparse.RawDescriptionHelpFormatter
    )

    parser.add_argument(
        '--dir', '-d',
        default='.',
        help='项目根目录 (默认: 当前目录)'
    )

    parser.add_argument(
        '--debounce',
        type=float,
        default=2.0,
        help='防抖时间，单位秒 (默认: 2.0)'
    )

    args = parser.parse_args()

    project_root = Path(args.dir).resolve()
    if project_root.name == 'tool':
        project_root = project_root.parent

    watcher = EnvConfigWatcher(project_root)
    watcher.start(debounce=args.debounce)


if __name__ == '__main__':
    main()
