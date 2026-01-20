#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
配置文件监视服务
监视 project_config.json 的变化，自动触发构建

依赖安装: pip install watchdog
"""

import sys
import time
import subprocess
from pathlib import Path
from datetime import datetime

try:
    from watchdog.observers import Observer
    from watchdog.events import FileSystemEventHandler, FileModifiedEvent
except ImportError:
    print("错误: 需要安装 watchdog 库")
    print("请运行: pip install watchdog")
    sys.exit(1)


class ConfigFileHandler(FileSystemEventHandler):
    """配置文件变化处理器"""

    def __init__(self, project_root: Path, watch_files: list, debounce_seconds: float = 1.0):
        super().__init__()
        self.project_root = project_root
        self.watch_files = [Path(f).name for f in watch_files]  # 只保留文件名
        self.debounce_seconds = debounce_seconds
        self.last_trigger_time = 0
        self.build_script = project_root / "tool" / "build.py"

    def on_modified(self, event):
        """文件修改事件处理"""
        if event.is_directory:
            return

        # 检查是否是我们关注的文件
        file_name = Path(event.src_path).name
        if file_name not in self.watch_files:
            return

        # 防抖：避免短时间内多次触发
        current_time = time.time()
        if current_time - self.last_trigger_time < self.debounce_seconds:
            return

        self.last_trigger_time = current_time
        self._trigger_build(file_name)

    def _trigger_build(self, changed_file: str):
        """触发构建"""
        timestamp = datetime.now().strftime("%H:%M:%S")
        print(f"\n{'='*60}")
        print(f"[{timestamp}] 检测到文件变化: {changed_file}")
        print(f"{'='*60}")

        try:
            # 运行构建脚本，使用非交互模式
            result = subprocess.run(
                [
                    sys.executable,
                    str(self.build_script),
                    "--dir", str(self.project_root),
                    "--no-interactive"  # 非交互模式
                ],
                cwd=str(self.project_root),
                timeout=60
            )

            if result.returncode == 0:
                print(f"\n✓ 构建完成")
            else:
                print(f"\n✗ 构建失败 (返回码: {result.returncode})")

        except subprocess.TimeoutExpired:
            print("\n✗ 构建超时")
        except Exception as e:
            print(f"\n✗ 构建出错: {e}")

        print(f"\n[监视中...] 等待文件变化 (Ctrl+C 退出)")


class ConfigWatcher:
    """配置文件监视器"""

    def __init__(self, project_root: str = "."):
        self.project_root = Path(project_root).resolve()
        self.tool_dir = self.project_root / "tool"
        self.observer = None

        # 默认监视的文件
        self.watch_files = [
            "project_config.json",
        ]

    def start(self, watch_files: list = None, debounce: float = 1.0):
        """启动监视服务"""
        if watch_files:
            self.watch_files = watch_files

        # 验证文件存在
        config_file = self.tool_dir / "project_config.json"
        if not config_file.exists():
            print(f"警告: 配置文件不存在: {config_file}")
            print("将在文件创建后开始监视")

        print(f"""
╔══════════════════════════════════════════════════════════╗
║           配置文件监视服务 v1.0                          ║
╠══════════════════════════════════════════════════════════╣
║  项目根目录: {str(self.project_root)[:42]:<42} ║
║  监视目录:   {str(self.tool_dir)[:42]:<42} ║
║  监视文件:   {', '.join(self.watch_files)[:42]:<42} ║
║  防抖时间:   {debounce:.1f} 秒{' '*36} ║
╠══════════════════════════════════════════════════════════╣
║  按 Ctrl+C 停止服务                                      ║
╚══════════════════════════════════════════════════════════╝
""")

        # 创建事件处理器
        handler = ConfigFileHandler(
            self.project_root,
            self.watch_files,
            debounce_seconds=debounce
        )

        # 创建观察者
        self.observer = Observer()
        self.observer.schedule(handler, str(self.tool_dir), recursive=False)

        # 启动观察
        self.observer.start()
        print("[监视中...] 等待文件变化\n")

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
        description='配置文件监视服务 - 自动检测变化并重新构建',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
示例:
  python watch_config.py                    # 在当前目录启动监视
  python watch_config.py --dir ..           # 指定项目目录
  python watch_config.py --debounce 2.0     # 设置2秒防抖时间
  python watch_config.py --files project_config.json config.h
        """
    )

    parser.add_argument(
        '--dir', '-d',
        default='.',
        help='项目根目录 (默认: 当前目录)'
    )

    parser.add_argument(
        '--files', '-f',
        nargs='+',
        default=['project_config.json'],
        help='要监视的文件列表 (默认: project_config.json)'
    )

    parser.add_argument(
        '--debounce',
        type=float,
        default=1.0,
        help='防抖时间，单位秒 (默认: 1.0)'
    )

    args = parser.parse_args()

    # 解析项目目录
    project_root = Path(args.dir).resolve()

    # 如果在 tool 目录下运行，自动定位到项目根目录
    if project_root.name == 'tool':
        project_root = project_root.parent

    watcher = ConfigWatcher(project_root)
    watcher.start(watch_files=args.files, debounce=args.debounce)


if __name__ == '__main__':
    main()
