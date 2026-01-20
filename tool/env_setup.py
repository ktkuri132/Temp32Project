#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
STM32 开发环境配置工具 v1.0
自动检测、下载和配置开发工具链

功能：
1. 检测 ARM-GCC 工具链、OpenOCD、J-Link 驱动
2. 自动下载缺失的工具
3. 配置 Python 环境和必要的包
4. 创建项目目录结构
5. 生成环境配置表
"""

import os
import sys
import json
import shutil
import platform
import subprocess
import urllib.request
import zipfile
import tarfile
from pathlib import Path
from datetime import datetime
from typing import Dict, List, Optional, Tuple


# ============================================================================
#                          配置常量
# ============================================================================

# 默认安装路径
if platform.system() == 'Windows':
    DEFAULT_INSTALL_DIR = Path("C:/Program Files (x86)")
    OPENOCD_INSTALL_DIR = Path("C:/openocd")
else:
    DEFAULT_INSTALL_DIR = Path.home() / ".local"
    OPENOCD_INSTALL_DIR = Path.home() / ".local" / "openocd"

# 工具下载链接 (根据平台选择)
TOOL_DOWNLOADS = {
    'Windows': {
        'arm-gcc': {
            'url': 'https://developer.arm.com/-/media/Files/downloads/gnu/13.2.rel1/binrel/arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-arm-none-eabi.zip',
            'filename': 'arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-arm-none-eabi.zip',
            'extract_name': 'arm-gnu-toolchain-13.2.Rel1-mingw-w64-i686-arm-none-eabi',
            'install_name': 'Arm GNU Toolchain arm-none-eabi/13.2 Rel1',
        },
        'openocd': {
            'url': 'https://github.com/openocd-org/openocd/releases/download/v0.12.0/openocd-v0.12.0-i686-w64-mingw32.tar.gz',
            'filename': 'openocd-v0.12.0-i686-w64-mingw32.tar.gz',
            'extract_name': 'openocd-v0.12.0-i686-w64-mingw32',
            'install_name': 'openocd-0.12.0',
        },
    },
    'Linux': {
        'arm-gcc': {
            'url': 'https://developer.arm.com/-/media/Files/downloads/gnu/13.2.rel1/binrel/arm-gnu-toolchain-13.2.rel1-x86_64-arm-none-eabi.tar.xz',
            'filename': 'arm-gnu-toolchain-13.2.rel1-x86_64-arm-none-eabi.tar.xz',
            'extract_name': 'arm-gnu-toolchain-13.2.Rel1-x86_64-arm-none-eabi',
            'install_name': 'arm-gnu-toolchain-13.2',
        },
        'openocd': {
            'url': 'https://github.com/openocd-org/openocd/releases/download/v0.12.0/openocd-v0.12.0-x86_64-linux.tar.gz',
            'filename': 'openocd-v0.12.0-x86_64-linux.tar.gz',
            'extract_name': 'openocd-v0.12.0-x86_64-linux',
            'install_name': 'openocd-0.12.0',
        },
    }
}

# 必需的 Python 包
REQUIRED_PYTHON_PACKAGES = [
    'watchdog',      # 文件监视
    'pyocd',         # 调试/烧录工具 (可选)
]

# 项目必需的目录结构
REQUIRED_DIRECTORIES = [
    'app',
    'BSP',
    'BSP/CMSIS',
    'BSP/CMSIS/Core',
    'BSP/CMSIS/Core/Include',
    'build',
    'Control',
    'Device',
    'Driver_Framework',
    'Driver_Framework/display',
    'Driver_Framework/i2c',
    'Driver_Framework/irq',
    'Driver_Framework/key',
    'Driver_Framework/lcd',
    'Driver_Framework/linker',
    'Driver_Framework/shell',
    'Driver_Framework/spi',
    'Middleware',
    'Middleware/trans',
    'tool',
    'doc',
    '.vscode',
]

# 环境配置文件路径
ENV_CONFIG_FILE = "tool/env_config.json"

# VS Code 推荐插件
RECOMMENDED_EXTENSIONS = [
    {
        'id': 'ms-vscode.cpptools',
        'name': 'C/C++',
        'description': 'C/C++ 语言支持、IntelliSense、调试',
        'required': True,
    },
    {
        'id': 'marus25.cortex-debug',
        'name': 'Cortex-Debug',
        'description': 'ARM Cortex-M 调试支持',
        'required': True,
    },
    {
        'id': 'ms-vscode.cmake-tools',
        'name': 'CMake Tools',
        'description': 'CMake 构建支持',
        'required': True,
    },
    {
        'id': 'twxs.cmake',
        'name': 'CMake',
        'description': 'CMake 语法高亮',
        'required': False,
    },
    {
        'id': 'ms-python.python',
        'name': 'Python',
        'description': 'Python 语言支持',
        'required': True,
    },
    {
        'id': 'dan-c-underwood.arm',
        'name': 'ARM Assembly',
        'description': 'ARM 汇编语法高亮',
        'required': False,
    },
    {
        'id': 'zixuanwang.linkerscript',
        'name': 'Linker Script',
        'description': '链接脚本语法高亮',
        'required': False,
    },
]


# ============================================================================
#                          工具类
# ============================================================================

class ColorPrint:
    """彩色输出工具"""

    COLORS = {
        'red': '\033[91m',
        'green': '\033[92m',
        'yellow': '\033[93m',
        'blue': '\033[94m',
        'magenta': '\033[95m',
        'cyan': '\033[96m',
        'reset': '\033[0m',
    }

    @classmethod
    def _supports_color(cls) -> bool:
        """检查终端是否支持颜色"""
        if platform.system() == 'Windows':
            # Windows 10+ 支持 ANSI
            return os.environ.get('TERM') or os.environ.get('WT_SESSION')
        return hasattr(sys.stdout, 'isatty') and sys.stdout.isatty()

    @classmethod
    def print(cls, text: str, color: str = 'reset'):
        if cls._supports_color():
            print(f"{cls.COLORS.get(color, '')}{text}{cls.COLORS['reset']}")
        else:
            print(text)

    @classmethod
    def success(cls, text: str):
        cls.print(f"✓ {text}", 'green')

    @classmethod
    def error(cls, text: str):
        cls.print(f"✗ {text}", 'red')

    @classmethod
    def warning(cls, text: str):
        cls.print(f"⚠ {text}", 'yellow')

    @classmethod
    def info(cls, text: str):
        cls.print(f"ℹ {text}", 'cyan')


class DownloadProgress:
    """下载进度显示"""

    def __init__(self, filename: str):
        self.filename = filename
        self.last_percent = -1

    def __call__(self, block_num, block_size, total_size):
        if total_size > 0:
            percent = int(block_num * block_size * 100 / total_size)
            percent = min(percent, 100)
            if percent != self.last_percent:
                bar_length = 40
                filled = int(bar_length * percent / 100)
                bar = '█' * filled + '░' * (bar_length - filled)
                size_mb = total_size / (1024 * 1024)
                print(f"\r  下载 {self.filename}: [{bar}] {percent}% ({size_mb:.1f}MB)", end='', flush=True)
                self.last_percent = percent
                if percent == 100:
                    print()


# ============================================================================
#                          环境检测器
# ============================================================================

class EnvironmentDetector:
    """环境检测器"""

    def __init__(self):
        self.system = platform.system()
        self.arch = platform.machine()
        self.env_config = {}

    def detect_arm_gcc(self) -> Dict:
        """检测 ARM-GCC 工具链"""
        result = {
            'found': False,
            'path': None,
            'version': None,
            'bin_path': None,
        }

        # 搜索路径列表
        search_paths = []

        if self.system == 'Windows':
            search_paths = [
                Path("C:/Program Files (x86)/Arm GNU Toolchain arm-none-eabi"),
                Path("C:/Program Files/Arm GNU Toolchain arm-none-eabi"),
                Path("C:/Program Files (x86)/GNU Arm Embedded Toolchain"),
                Path("C:/Program Files/GNU Arm Embedded Toolchain"),
            ]
        else:
            search_paths = [
                Path.home() / ".local" / "arm-gnu-toolchain",
                Path("/opt/arm-gnu-toolchain"),
                Path("/usr/local/arm-gnu-toolchain"),
            ]

        # 先检查 PATH 中是否有
        gcc_name = "arm-none-eabi-gcc.exe" if self.system == 'Windows' else "arm-none-eabi-gcc"

        try:
            which_result = subprocess.run(
                ['where' if self.system == 'Windows' else 'which', 'arm-none-eabi-gcc'],
                capture_output=True, text=True
            )
            if which_result.returncode == 0:
                gcc_path = Path(which_result.stdout.strip().split('\n')[0])
                bin_path = gcc_path.parent
                result['bin_path'] = str(bin_path)
                result['path'] = str(bin_path.parent)
                result['found'] = True
        except Exception:
            pass

        # 搜索常见安装路径
        if not result['found']:
            for base_path in search_paths:
                if base_path.exists():
                    # 搜索子目录中的版本
                    for version_dir in sorted(base_path.iterdir(), reverse=True):
                        if version_dir.is_dir():
                            bin_path = version_dir / "bin"
                            if (bin_path / gcc_name).exists():
                                result['found'] = True
                                result['path'] = str(version_dir)
                                result['bin_path'] = str(bin_path)
                                break
                    if result['found']:
                        break

        # 获取版本信息
        if result['found']:
            try:
                gcc_exe = Path(result['bin_path']) / gcc_name
                version_result = subprocess.run(
                    [str(gcc_exe), '--version'],
                    capture_output=True, text=True
                )
                if version_result.returncode == 0:
                    version_line = version_result.stdout.split('\n')[0]
                    result['version'] = version_line
            except Exception:
                pass

        return result

    def detect_openocd(self) -> Dict:
        """检测 OpenOCD"""
        result = {
            'found': False,
            'path': None,
            'version': None,
            'scripts_path': None,
        }

        search_paths = []

        if self.system == 'Windows':
            search_paths = [
                Path("C:/openocd"),
                Path("C:/Program Files/OpenOCD"),
                Path("C:/Program Files (x86)/OpenOCD"),
            ]
            exe_name = "openocd.exe"
        else:
            search_paths = [
                Path.home() / ".local" / "openocd",
                Path("/opt/openocd"),
                Path("/usr/local"),
            ]
            exe_name = "openocd"

        # 先检查 PATH
        try:
            which_result = subprocess.run(
                ['where' if self.system == 'Windows' else 'which', 'openocd'],
                capture_output=True, text=True
            )
            if which_result.returncode == 0:
                openocd_path = Path(which_result.stdout.strip().split('\n')[0])
                result['found'] = True
                result['path'] = str(openocd_path.parent.parent)
        except Exception:
            pass

        # 搜索常见路径
        if not result['found']:
            for base_path in search_paths:
                if base_path.exists():
                    # 检查直接路径
                    bin_path = base_path / "bin" / exe_name
                    if bin_path.exists():
                        result['found'] = True
                        result['path'] = str(base_path)
                        break

                    # 检查子目录
                    for sub_dir in base_path.iterdir():
                        if sub_dir.is_dir():
                            bin_path = sub_dir / "bin" / exe_name
                            if bin_path.exists():
                                result['found'] = True
                                result['path'] = str(sub_dir)
                                break
                    if result['found']:
                        break

        # 获取版本和脚本路径
        if result['found']:
            scripts_path = Path(result['path']) / "share" / "openocd" / "scripts"
            if scripts_path.exists():
                result['scripts_path'] = str(scripts_path)

            try:
                openocd_exe = Path(result['path']) / "bin" / exe_name
                version_result = subprocess.run(
                    [str(openocd_exe), '--version'],
                    capture_output=True, text=True
                )
                # OpenOCD 输出版本到 stderr
                version_output = version_result.stderr or version_result.stdout
                if version_output:
                    result['version'] = version_output.split('\n')[0]
            except Exception:
                pass

        return result

    def detect_jlink(self) -> Dict:
        """检测 J-Link 驱动"""
        result = {
            'found': False,
            'path': None,
            'version': None,
        }

        if self.system == 'Windows':
            search_paths = [
                Path("C:/Program Files/SEGGER"),
                Path("C:/Program Files (x86)/SEGGER"),
            ]
            exe_name = "JLink.exe"
        else:
            search_paths = [
                Path("/opt/SEGGER"),
                Path.home() / "SEGGER",
            ]
            exe_name = "JLinkExe"

        for base_path in search_paths:
            if base_path.exists():
                for jlink_dir in sorted(base_path.iterdir(), reverse=True):
                    if jlink_dir.is_dir() and 'jlink' in jlink_dir.name.lower():
                        exe_path = jlink_dir / exe_name
                        if exe_path.exists():
                            result['found'] = True
                            result['path'] = str(jlink_dir)
                            result['version'] = jlink_dir.name
                            break
                if result['found']:
                    break

        return result

    def detect_cmake(self) -> Dict:
        """检测 CMake"""
        result = {
            'found': False,
            'path': None,
            'version': None,
        }

        try:
            cmake_result = subprocess.run(
                ['cmake', '--version'],
                capture_output=True, text=True
            )
            if cmake_result.returncode == 0:
                result['found'] = True
                version_line = cmake_result.stdout.split('\n')[0]
                result['version'] = version_line

                # 获取路径
                which_result = subprocess.run(
                    ['where' if self.system == 'Windows' else 'which', 'cmake'],
                    capture_output=True, text=True
                )
                if which_result.returncode == 0:
                    result['path'] = which_result.stdout.strip().split('\n')[0]
        except Exception:
            pass

        return result

    def detect_ninja(self) -> Dict:
        """检测 Ninja"""
        result = {
            'found': False,
            'path': None,
            'version': None,
        }

        try:
            ninja_result = subprocess.run(
                ['ninja', '--version'],
                capture_output=True, text=True
            )
            if ninja_result.returncode == 0:
                result['found'] = True
                result['version'] = ninja_result.stdout.strip()
        except Exception:
            pass

        return result

    def detect_python_packages(self) -> Dict[str, bool]:
        """检测 Python 包"""
        packages = {}
        for pkg in REQUIRED_PYTHON_PACKAGES:
            try:
                __import__(pkg.replace('-', '_'))
                packages[pkg] = True
            except ImportError:
                packages[pkg] = False
        return packages

    def run_full_detection(self) -> Dict:
        """运行完整检测"""
        return {
            'system': self.system,
            'architecture': self.arch,
            'arm_gcc': self.detect_arm_gcc(),
            'openocd': self.detect_openocd(),
            'jlink': self.detect_jlink(),
            'cmake': self.detect_cmake(),
            'ninja': self.detect_ninja(),
            'python_packages': self.detect_python_packages(),
            'detection_time': datetime.now().isoformat(),
        }


# ============================================================================
#                          工具安装器
# ============================================================================

class ToolInstaller:
    """工具安装器"""

    def __init__(self, install_dir: Path = None):
        self.system = platform.system()
        self.install_dir = install_dir or DEFAULT_INSTALL_DIR
        self.temp_dir = Path.home() / ".stm32_setup_temp"

    def _ensure_temp_dir(self):
        """确保临时目录存在"""
        self.temp_dir.mkdir(parents=True, exist_ok=True)

    def _cleanup_temp(self):
        """清理临时目录"""
        if self.temp_dir.exists():
            shutil.rmtree(self.temp_dir, ignore_errors=True)

    def download_file(self, url: str, filename: str) -> Path:
        """下载文件"""
        self._ensure_temp_dir()
        local_path = self.temp_dir / filename

        if local_path.exists():
            ColorPrint.info(f"使用已下载的文件: {filename}")
            return local_path

        ColorPrint.info(f"正在下载: {filename}")
        try:
            urllib.request.urlretrieve(url, local_path, DownloadProgress(filename))
            return local_path
        except Exception as e:
            ColorPrint.error(f"下载失败: {e}")
            raise

    def extract_archive(self, archive_path: Path, extract_to: Path) -> Path:
        """解压文件"""
        ColorPrint.info(f"正在解压到: {extract_to}")

        extract_to.mkdir(parents=True, exist_ok=True)

        if archive_path.suffix == '.zip':
            with zipfile.ZipFile(archive_path, 'r') as zf:
                zf.extractall(extract_to)
        elif archive_path.suffix in ['.gz', '.xz']:
            with tarfile.open(archive_path, 'r:*') as tf:
                tf.extractall(extract_to)
        else:
            raise ValueError(f"不支持的压缩格式: {archive_path.suffix}")

        return extract_to

    def install_arm_gcc(self) -> Optional[str]:
        """安装 ARM-GCC 工具链"""
        if self.system not in TOOL_DOWNLOADS:
            ColorPrint.error(f"不支持的操作系统: {self.system}")
            return None

        tool_info = TOOL_DOWNLOADS[self.system]['arm-gcc']

        try:
            # 下载
            archive_path = self.download_file(tool_info['url'], tool_info['filename'])

            # 解压
            extract_dir = self.temp_dir / "arm-gcc-extract"
            self.extract_archive(archive_path, extract_dir)

            # 移动到安装目录
            source_dir = extract_dir / tool_info['extract_name']
            if not source_dir.exists():
                # 尝试查找解压后的目录
                for item in extract_dir.iterdir():
                    if item.is_dir():
                        source_dir = item
                        break

            target_dir = self.install_dir / tool_info['install_name']
            target_dir.parent.mkdir(parents=True, exist_ok=True)

            if target_dir.exists():
                shutil.rmtree(target_dir)

            shutil.move(str(source_dir), str(target_dir))

            ColorPrint.success(f"ARM-GCC 已安装到: {target_dir}")
            return str(target_dir)

        except Exception as e:
            ColorPrint.error(f"安装 ARM-GCC 失败: {e}")
            return None

    def install_openocd(self) -> Optional[str]:
        """安装 OpenOCD"""
        if self.system not in TOOL_DOWNLOADS:
            ColorPrint.error(f"不支持的操作系统: {self.system}")
            return None

        tool_info = TOOL_DOWNLOADS[self.system]['openocd']

        try:
            # 下载
            archive_path = self.download_file(tool_info['url'], tool_info['filename'])

            # 解压
            extract_dir = self.temp_dir / "openocd-extract"
            self.extract_archive(archive_path, extract_dir)

            # 移动到安装目录
            source_dir = extract_dir / tool_info['extract_name']
            if not source_dir.exists():
                for item in extract_dir.iterdir():
                    if item.is_dir():
                        source_dir = item
                        break

            target_dir = OPENOCD_INSTALL_DIR
            target_dir.parent.mkdir(parents=True, exist_ok=True)

            if target_dir.exists():
                shutil.rmtree(target_dir)

            shutil.move(str(source_dir), str(target_dir))

            ColorPrint.success(f"OpenOCD 已安装到: {target_dir}")
            return str(target_dir)

        except Exception as e:
            ColorPrint.error(f"安装 OpenOCD 失败: {e}")
            return None

    def install_python_packages(self) -> bool:
        """安装 Python 包"""
        ColorPrint.info("正在安装 Python 依赖包...")

        success = True
        for pkg in REQUIRED_PYTHON_PACKAGES:
            try:
                subprocess.run(
                    [sys.executable, '-m', 'pip', 'install', pkg, '-q'],
                    check=True,
                    capture_output=True
                )
                ColorPrint.success(f"已安装: {pkg}")
            except subprocess.CalledProcessError:
                ColorPrint.warning(f"安装失败 (可选): {pkg}")

        return success


# ============================================================================
#                          项目初始化器
# ============================================================================

class ProjectInitializer:
    """项目初始化器"""

    def __init__(self, project_root: Path):
        self.project_root = Path(project_root)

    def create_directory_structure(self) -> List[str]:
        """创建目录结构"""
        created = []

        for dir_path in REQUIRED_DIRECTORIES:
            full_path = self.project_root / dir_path
            if not full_path.exists():
                full_path.mkdir(parents=True, exist_ok=True)
                created.append(dir_path)

        return created

    def create_placeholder_files(self):
        """创建占位文件"""
        placeholders = {
            'app/main.c': '''/**
 * @file    main.c
 * @brief   主程序入口
 */

#include "main.h"

int main(void)
{
    /* 初始化 */

    while (1)
    {
        /* 主循环 */
    }
}
''',
            'app/main.h': '''/**
 * @file    main.h
 * @brief   主程序头文件
 */

#ifndef __MAIN_H
#define __MAIN_H

#include <stdint.h>

#endif /* __MAIN_H */
''',
        }

        for file_path, content in placeholders.items():
            full_path = self.project_root / file_path
            if not full_path.exists():
                full_path.parent.mkdir(parents=True, exist_ok=True)
                full_path.write_text(content, encoding='utf-8')

    def generate_tasks_json(self, detection_result: Dict) -> bool:
        """根据检测结果生成 tasks.json"""
        vscode_dir = self.project_root / ".vscode"
        vscode_dir.mkdir(parents=True, exist_ok=True)
        tasks_file = vscode_dir / "tasks.json"

        tasks = []

        # 基础任务：构建配置
        tasks.append({
            "label": "$(tools)构建配置",
            "type": "shell",
            "command": "python",
            "args": ["${workspaceFolder}/tool/build.py", "--no-interactive"],
            "problemMatcher": [],
            "group": {"kind": "build", "isDefault": False}
        })

        # 监视配置任务
        tasks.append({
            "label": "$(eye)监视配置",
            "type": "shell",
            "command": "python",
            "args": ["${workspaceFolder}/tool/watch_config.py"],
            "problemMatcher": [],
            "group": "none",
            "isBackground": True,
            "presentation": {"reveal": "always", "panel": "dedicated", "clear": True}
        })

        # 根据工具检测结果添加下载任务
        openocd_found = detection_result.get('openocd', {}).get('found', False)
        jlink_found = detection_result.get('jlink', {}).get('found', False)

        if openocd_found or jlink_found:
            tasks.append({
                "label": "$(zap)烧录",
                "type": "shell",
                "command": "python",
                "args": ["${workspaceFolder}/tool/download.py"],
                "problemMatcher": [],
                "group": {"kind": "build", "isDefault": False}
            })

        # CMake 构建任务
        cmake_found = detection_result.get('cmake', {}).get('found', False)
        ninja_found = detection_result.get('ninja', {}).get('found', False)

        if cmake_found:
            generator = "Ninja" if ninja_found else "Unix Makefiles"
            tasks.append({
                "label": "$(gear)CMake配置",
                "type": "shell",
                "command": "cmake",
                "args": ["-B", "build", "-G", generator],
                "problemMatcher": "$gcc",
                "group": "build"
            })
            tasks.append({
                "label": "$(package)编译项目",
                "type": "shell",
                "command": "cmake",
                "args": ["--build", "build", "-j"],
                "problemMatcher": "$gcc",
                "group": {"kind": "build", "isDefault": True}
            })
            tasks.append({
                "label": "$(trash)清理构建",
                "type": "shell",
                "command": "cmake",
                "args": ["--build", "build", "--target", "clean"],
                "problemMatcher": [],
                "group": "build"
            })

        # 环境配置任务
        tasks.append({
            "label": "$(settings-gear)环境配置",
            "type": "shell",
            "command": "python",
            "args": ["${workspaceFolder}/tool/env_setup.py"],
            "problemMatcher": [],
            "group": "none",
            "presentation": {"reveal": "always", "panel": "shared"}
        })

        tasks.append({
            "label": "$(check)验证环境",
            "type": "shell",
            "command": "python",
            "args": ["${workspaceFolder}/tool/env_setup.py", "--verify"],
            "problemMatcher": [],
            "group": "none"
        })

        tasks_config = {
            "version": "2.0.0",
            "tasks": tasks
        }

        try:
            with open(tasks_file, 'w', encoding='utf-8') as f:
                json.dump(tasks_config, f, indent=2, ensure_ascii=False)
            return True
        except Exception as e:
            ColorPrint.error(f"生成 tasks.json 失败: {e}")
            return False

    def generate_extensions_json(self) -> bool:
        """生成推荐插件配置"""
        vscode_dir = self.project_root / ".vscode"
        vscode_dir.mkdir(parents=True, exist_ok=True)
        extensions_file = vscode_dir / "extensions.json"

        recommendations = [ext['id'] for ext in RECOMMENDED_EXTENSIONS]

        config = {
            "recommendations": recommendations
        }

        try:
            with open(extensions_file, 'w', encoding='utf-8') as f:
                json.dump(config, f, indent=2, ensure_ascii=False)
            return True
        except Exception as e:
            ColorPrint.error(f"生成 extensions.json 失败: {e}")
            return False

    def check_extensions_installed(self) -> List[Dict]:
        """检查推荐插件是否已安装"""
        missing = []
        try:
            result = subprocess.run(
                ['code', '--list-extensions'],
                capture_output=True, text=True
            )
            if result.returncode == 0:
                installed = result.stdout.lower().split('\n')
                for ext in RECOMMENDED_EXTENSIONS:
                    if ext['id'].lower() not in installed:
                        missing.append(ext)
        except Exception:
            # 无法检测，返回所有必需插件
            missing = [ext for ext in RECOMMENDED_EXTENSIONS if ext['required']]
        return missing

    def print_extension_recommendations(self):
        """打印插件推荐信息"""
        missing = self.check_extensions_installed()

        if not missing:
            ColorPrint.success("所有推荐插件已安装")
            return

        print("\n" + "="*60)
        print("                 VS Code 插件推荐")
        print("="*60)

        required_missing = [ext for ext in missing if ext['required']]
        optional_missing = [ext for ext in missing if not ext['required']]

        if required_missing:
            ColorPrint.warning("必需插件 (建议立即安装):")
            for ext in required_missing:
                print(f"  • {ext['name']}: {ext['description']}")
                print(f"    安装: code --install-extension {ext['id']}")

        if optional_missing:
            print("\n可选插件:")
            for ext in optional_missing:
                print(f"  • {ext['name']}: {ext['description']}")

        # 一键安装命令
        if required_missing:
            print("\n一键安装所有必需插件:")
            install_cmd = "code " + " ".join([f"--install-extension {ext['id']}" for ext in required_missing])
            ColorPrint.info(install_cmd)

        print("="*60)


# ============================================================================
#                          环境配置管理器
# ============================================================================

class EnvConfigManager:
    """环境配置管理器"""

    def __init__(self, project_root: Path):
        self.project_root = Path(project_root)
        self.config_file = self.project_root / ENV_CONFIG_FILE
        self.config = self.load_config()

    def load_config(self) -> Dict:
        """加载配置"""
        if self.config_file.exists():
            try:
                with open(self.config_file, 'r', encoding='utf-8') as f:
                    return json.load(f)
            except Exception:
                pass
        return {}

    def save_config(self, config: Dict):
        """保存配置"""
        self.config_file.parent.mkdir(parents=True, exist_ok=True)
        with open(self.config_file, 'w', encoding='utf-8') as f:
            json.dump(config, f, indent=2, ensure_ascii=False)
        self.config = config

    def generate_config(self, detection_result: Dict) -> Dict:
        """生成环境配置"""
        config = {
            '_comment': '环境配置文件 - 可手动修改工具路径',
            '_last_check': datetime.now().isoformat(),
            'system': {
                'os': detection_result['system'],
                'architecture': detection_result['architecture'],
            },
            'tools': {
                'arm_gcc': {
                    'enabled': detection_result['arm_gcc']['found'],
                    'path': detection_result['arm_gcc'].get('path', ''),
                    'bin_path': detection_result['arm_gcc'].get('bin_path', ''),
                    'version': detection_result['arm_gcc'].get('version', ''),
                },
                'openocd': {
                    'enabled': detection_result['openocd']['found'],
                    'path': detection_result['openocd'].get('path', ''),
                    'scripts_path': detection_result['openocd'].get('scripts_path', ''),
                    'version': detection_result['openocd'].get('version', ''),
                },
                'jlink': {
                    'enabled': detection_result['jlink']['found'],
                    'path': detection_result['jlink'].get('path', ''),
                    'version': detection_result['jlink'].get('version', ''),
                },
                'cmake': {
                    'enabled': detection_result['cmake']['found'],
                    'path': detection_result['cmake'].get('path', ''),
                    'version': detection_result['cmake'].get('version', ''),
                },
                'ninja': {
                    'enabled': detection_result['ninja']['found'],
                    'version': detection_result['ninja'].get('version', ''),
                },
            },
            'python': {
                'executable': sys.executable,
                'version': platform.python_version(),
                'packages': detection_result['python_packages'],
            },
            'project': {
                'directories_created': [],
            }
        }
        return config

    def verify_config(self) -> Tuple[bool, List[str]]:
        """验证配置是否有效"""
        issues = []

        if not self.config:
            return False, ['配置文件不存在或为空']

        tools = self.config.get('tools', {})

        # 检查 ARM-GCC
        arm_gcc = tools.get('arm_gcc', {})
        if arm_gcc.get('enabled'):
            bin_path = arm_gcc.get('bin_path', '')
            if bin_path:
                gcc_exe = Path(bin_path) / ('arm-none-eabi-gcc.exe' if platform.system() == 'Windows' else 'arm-none-eabi-gcc')
                if not gcc_exe.exists():
                    issues.append(f"ARM-GCC 不存在: {gcc_exe}")

        # 检查 OpenOCD
        openocd = tools.get('openocd', {})
        if openocd.get('enabled'):
            ocd_path = openocd.get('path', '')
            if ocd_path:
                ocd_exe = Path(ocd_path) / 'bin' / ('openocd.exe' if platform.system() == 'Windows' else 'openocd')
                if not ocd_exe.exists():
                    issues.append(f"OpenOCD 不存在: {ocd_exe}")

        return len(issues) == 0, issues


# ============================================================================
#                          主程序
# ============================================================================

class EnvironmentSetup:
    """环境配置主类"""

    def __init__(self, project_root: str = "."):
        self.project_root = Path(project_root).resolve()
        self.detector = EnvironmentDetector()
        self.installer = ToolInstaller()
        self.initializer = ProjectInitializer(self.project_root)
        self.config_manager = EnvConfigManager(self.project_root)

    def print_banner(self):
        """打印横幅"""
        print("""
╔══════════════════════════════════════════════════════════════════╗
║         STM32 开发环境配置工具 v1.0                              ║
║         自动检测、下载和配置开发工具链                           ║
╚══════════════════════════════════════════════════════════════════╝
        """)

    def print_detection_result(self, result: Dict):
        """打印检测结果"""
        print("\n" + "="*60)
        print("                    环境检测结果")
        print("="*60)

        # ARM-GCC
        arm_gcc = result['arm_gcc']
        if arm_gcc['found']:
            ColorPrint.success(f"ARM-GCC: {arm_gcc.get('version', '已安装')}")
            print(f"         路径: {arm_gcc.get('path', 'N/A')}")
        else:
            ColorPrint.error("ARM-GCC: 未安装")

        # OpenOCD
        openocd = result['openocd']
        if openocd['found']:
            ColorPrint.success(f"OpenOCD: {openocd.get('version', '已安装')}")
            print(f"         路径: {openocd.get('path', 'N/A')}")
        else:
            ColorPrint.error("OpenOCD: 未安装")

        # J-Link
        jlink = result['jlink']
        if jlink['found']:
            ColorPrint.success(f"J-Link:  {jlink.get('version', '已安装')}")
            print(f"         路径: {jlink.get('path', 'N/A')}")
        else:
            ColorPrint.warning("J-Link:  未安装 (可选)")

        # CMake
        cmake = result['cmake']
        if cmake['found']:
            ColorPrint.success(f"CMake:   {cmake.get('version', '已安装')}")
        else:
            ColorPrint.error("CMake:   未安装")

        # Ninja
        ninja = result['ninja']
        if ninja['found']:
            ColorPrint.success(f"Ninja:   v{ninja.get('version', '已安装')}")
        else:
            ColorPrint.warning("Ninja:   未安装 (可选)")

        # Python 包
        print("\nPython 依赖包:")
        for pkg, installed in result['python_packages'].items():
            if installed:
                ColorPrint.success(f"  {pkg}")
            else:
                ColorPrint.warning(f"  {pkg} (未安装)")

        print("="*60)

    def run_interactive_setup(self):
        """运行交互式安装"""
        self.print_banner()

        # 1. 检测环境
        print("\n[步骤 1/6] 检测当前环境...")
        detection_result = self.detector.run_full_detection()
        self.print_detection_result(detection_result)

        # 2. 安装缺失的工具
        print("\n[步骤 2/6] 安装缺失的工具...")

        if not detection_result['arm_gcc']['found']:
            response = input("是否下载并安装 ARM-GCC 工具链? (Y/n): ").strip().lower()
            if response != 'n':
                path = self.installer.install_arm_gcc()
                if path:
                    detection_result['arm_gcc'] = self.detector.detect_arm_gcc()

        if not detection_result['openocd']['found']:
            response = input("是否下载并安装 OpenOCD? (Y/n): ").strip().lower()
            if response != 'n':
                path = self.installer.install_openocd()
                if path:
                    detection_result['openocd'] = self.detector.detect_openocd()

        if not detection_result['jlink']['found']:
            ColorPrint.info("J-Link 需要从 SEGGER 官网手动下载安装")
            print("  下载地址: https://www.segger.com/downloads/jlink/")

        # 3. 安装 Python 包
        print("\n[步骤 3/6] 安装 Python 依赖包...")
        missing_packages = [pkg for pkg, installed in detection_result['python_packages'].items() if not installed]
        if missing_packages:
            self.installer.install_python_packages()
            detection_result['python_packages'] = self.detector.detect_python_packages()
        else:
            ColorPrint.success("所有 Python 包已安装")

        # 4. 创建目录结构
        print("\n[步骤 4/6] 创建项目目录结构...")
        created_dirs = self.initializer.create_directory_structure()
        if created_dirs:
            for d in created_dirs:
                ColorPrint.success(f"创建目录: {d}")
        else:
            ColorPrint.success("目录结构已完整")

        # 5. 生成 VS Code 配置
        print("\n[步骤 5/6] 生成 VS Code 配置...")

        # 生成 tasks.json
        if self.initializer.generate_tasks_json(detection_result):
            ColorPrint.success("已生成 .vscode/tasks.json")
        else:
            ColorPrint.warning("生成 tasks.json 失败")

        # 生成 extensions.json
        if self.initializer.generate_extensions_json():
            ColorPrint.success("已生成 .vscode/extensions.json")
        else:
            ColorPrint.warning("生成 extensions.json 失败")

        # 检查并推荐插件
        self.initializer.print_extension_recommendations()

        # 6. 生成环境配置文件
        print("\n[步骤 6/6] 生成环境配置表...")
        config = self.config_manager.generate_config(detection_result)
        config['project']['directories_created'] = created_dirs
        self.config_manager.save_config(config)
        ColorPrint.success(f"配置已保存到: {self.config_manager.config_file}")

        # 清理临时文件
        self.installer._cleanup_temp()

        # 最终检查
        print("\n" + "="*60)
        print("                    配置完成")
        print("="*60)

        valid, issues = self.config_manager.verify_config()
        if valid:
            ColorPrint.success("所有工具配置正确，环境准备就绪！")
        else:
            ColorPrint.warning("存在以下问题:")
            for issue in issues:
                print(f"  - {issue}")

        print("\n提示:")
        print(f"  - 环境配置文件: {self.config_manager.config_file}")
        print("  - 可手动编辑配置文件更改工具路径")
        print("  - 运行 'python env_setup.py --verify' 重新验证配置")

    def verify_only(self):
        """仅验证配置"""
        self.print_banner()

        print("验证环境配置...")

        # 加载并验证配置
        valid, issues = self.config_manager.verify_config()

        if valid:
            ColorPrint.success("环境配置验证通过！")

            # 显示当前配置
            config = self.config_manager.config
            tools = config.get('tools', {})

            print("\n当前配置:")
            for tool_name, tool_info in tools.items():
                if tool_info.get('enabled'):
                    print(f"  {tool_name}: {tool_info.get('path', 'N/A')}")
        else:
            ColorPrint.error("配置验证失败:")
            for issue in issues:
                print(f"  - {issue}")

            # 询问是否重新检测
            response = input("\n是否重新运行环境检测? (Y/n): ").strip().lower()
            if response != 'n':
                self.run_interactive_setup()


def main():
    import argparse

    parser = argparse.ArgumentParser(
        description='STM32 开发环境配置工具',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
示例:
  python env_setup.py                # 运行完整配置
  python env_setup.py --verify       # 仅验证配置
  python env_setup.py --dir /path    # 指定项目目录
        """
    )

    parser.add_argument(
        '--dir', '-d',
        default='.',
        help='项目根目录 (默认: 当前目录)'
    )

    parser.add_argument(
        '--verify', '-v',
        action='store_true',
        help='仅验证现有配置'
    )

    parser.add_argument(
        '--no-interactive', '-n',
        action='store_true',
        help='非交互模式（自动安装所有缺失工具）'
    )

    args = parser.parse_args()

    # 解析项目目录
    project_root = Path(args.dir).resolve()
    if project_root.name == 'tool':
        project_root = project_root.parent

    setup = EnvironmentSetup(project_root)

    if args.verify:
        setup.verify_only()
    else:
        setup.run_interactive_setup()


if __name__ == '__main__':
    main()
