#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
源文件扫描和解析模块
负责扫描项目源文件、头文件和子目录
"""

import os
import re
from pathlib import Path


class SourceScanner:
    """源文件扫描器"""

    def __init__(self, project_root, config):
        self.project_root = Path(project_root)
        self.config = config

        # 文件扩展名定义
        self.source_extensions = {'.c', '.cpp', '.s', '.S', '.asm'}
        self.header_extensions = {'.h', '.hpp'}

        # 排除目录
        self.exclude_dirs = {'build', 'MDK', '.git', '.vscode', '__pycache__', 'Debug', 'Release'}

        # 芯片架构映射
        self.chip_arch_map = {
            'STM32F0': 'cortex-m0', 'STM32F1': 'cortex-m3', 'STM32F103': 'cortex-m3',
            'STM32F2': 'cortex-m3', 'STM32F3': 'cortex-m4', 'STM32F4': 'cortex-m4',
            'STM32F7': 'cortex-m7', 'STM32G0': 'cortex-m0plus', 'STM32G4': 'cortex-m4',
            'STM32H7': 'cortex-m7', 'STM32L0': 'cortex-m0plus', 'STM32L1': 'cortex-m3',
            'STM32L4': 'cortex-m4', 'STM32L5': 'cortex-m33', 'STM32U5': 'cortex-m33',
            'STM32WB': 'cortex-m4', 'STM32WL': 'cortex-m4'
        }

    def detect_chip_arch(self, chip_name):
        """检测芯片架构"""
        chip_upper = chip_name.upper()
        for chip_prefix, arch in self.chip_arch_map.items():
            if chip_upper.startswith(chip_prefix):
                return arch
        return 'cortex-m3'  # 默认

    def get_bsp_chip_dir(self, chip_name):
        """根据芯片名称获取对应的BSP目录名"""
        chip_upper = chip_name.upper()
        match = re.match(r'(STM32[A-Z])\d', chip_upper)
        if match:
            series_match = re.match(r'STM32([A-Z])(\d)', chip_upper)
            if series_match:
                letter = series_match.group(1).lower()
                number = series_match.group(2)
                return f"stm32{letter}{number}"
        return None

    def get_all_bsp_chip_dirs(self):
        """获取BSP目录下所有芯片目录（排除CMSIS）"""
        bsp_dir = self.project_root / 'BSP'
        if not bsp_dir.exists():
            bsp_dir = self.project_root / 'bsp'

        if not bsp_dir.exists():
            return set()

        chip_dirs = set()
        for item in bsp_dir.iterdir():
            if item.is_dir() and item.name.lower() != 'cmsis':
                chip_dirs.add(item.name.lower())

        return chip_dirs

    def should_exclude_bsp_chip_dir(self, dir_path, dir_name):
        """判断是否应该排除BSP中的芯片目录"""
        dir_name_lower = dir_name.lower()
        path_parts = [p.lower() for p in dir_path.parts]

        # 检查是否在BSP/bsp目录下
        if 'bsp' not in path_parts:
            return False

        # 获取BSP目录的索引
        try:
            bsp_idx = path_parts.index('bsp')
        except ValueError:
            return False

        # 检查当前目录是否正好是BSP目录
        if len(path_parts) - 1 != bsp_idx:
            return False

        # CMSIS目录永远不排除
        if dir_name_lower == 'cmsis':
            return False

        # 获取当前配置的芯片
        current_chip = self.config.get('project.chip', '')
        target_bsp_dir = self.get_bsp_chip_dir(current_chip)

        if target_bsp_dir is None:
            return False

        # 检查当前子目录是否是芯片目录
        all_chip_dirs = self.get_all_bsp_chip_dirs()

        if dir_name_lower in all_chip_dirs:
            if dir_name_lower != target_bsp_dir:
                return True

        return False

    def should_exclude(self, path_str):
        """判断是否排除文件/目录"""
        path_lower = path_str.lower()

        # 排除CMSIS源文件
        if 'cmsis' in path_lower and any(ext in path_str for ext in self.source_extensions):
            return True

        # 排除架构无关文件
        arch_dirs = ['ca5', 'ca7', 'ca9', 'cm0', 'cm4', 'cm7', 'cm23', 'cm33', 'cm55', 'cm85']
        for arch_dir in arch_dirs:
            if arch_dir in path_lower:
                return True

        return False

    def scan_sources(self, target_arch):
        """扫描源文件（排除BSP和Device，它们有独立的CMakeLists.txt）"""
        sources = []
        headers = []
        subdirs_with_cmake = []

        for root, dirs, files in os.walk(self.project_root):
            root_path = Path(root)

            # 过滤目录
            dirs[:] = [d for d in dirs if d not in self.exclude_dirs and not d.startswith('.')]

            # 排除BSP和Device目录（它们有独立的CMakeLists.txt）
            if root_path == self.project_root:
                dirs[:] = [d for d in dirs if d not in ['BSP', 'Device', 'bsp', 'device']]

            # 过滤BSP中不匹配的芯片目录
            dirs[:] = [d for d in dirs if not self.should_exclude_bsp_chip_dir(root_path, d)]

            # 检查子目录CMakeLists.txt
            if root_path != self.project_root and (root_path / "CMakeLists.txt").exists():
                subdirs_with_cmake.append(root_path)
                dirs.clear()
                continue

            for file in files:
                file_path = root_path / file
                path_str = str(file_path)

                if self.should_exclude(path_str):
                    continue

                if file_path.suffix in self.source_extensions:
                    sources.append(file_path)
                elif file_path.suffix in self.header_extensions:
                    headers.append(file_path)

        return sources, subdirs_with_cmake

    def scan_include_dirs(self):
        """专门扫描头文件目录（排除BSP和Device）"""
        include_dirs = set()

        for root, dirs, files in os.walk(self.project_root):
            root_path = Path(root)

            # 过滤要排除的目录
            dirs[:] = [d for d in dirs if d not in self.exclude_dirs and not d.startswith('.')]

            # 排除BSP和Device目录（它们有独立的CMakeLists.txt）
            if root_path == self.project_root:
                dirs[:] = [d for d in dirs if d not in ['BSP', 'Device', 'bsp', 'device']]

            # 过滤BSP中不匹配的芯片目录
            dirs[:] = [d for d in dirs if not self.should_exclude_bsp_chip_dir(root_path, d)]

            # 检查当前目录是否包含头文件
            has_headers = any(f.endswith(tuple(self.header_extensions)) for f in files)

            if has_headers:
                # 特殊处理CMSIS目录 - 只添加Core/Include
                if 'cmsis' in str(root_path).lower():
                    if 'core' in str(root_path).lower() and 'include' in str(root_path).lower():
                        path_parts = [part.lower() for part in root_path.parts]
                        try:
                            cmsis_idx = path_parts.index('cmsis')
                            core_idx = path_parts.index('core', cmsis_idx)
                            include_idx = path_parts.index('include', core_idx)
                            if include_idx == core_idx + 1:
                                include_dirs.add(root_path)
                        except ValueError:
                            pass
                else:
                    # 非CMSIS目录的处理
                    include_dirs.add(root_path)

                    # 如果当前目录名包含include，停止向下搜索
                    if 'include' in root_path.name.lower():
                        dirs.clear()

        return include_dirs

    def scan_subdir_includes(self, subdirs_with_cmake):
        """扫描子目录的头文件路径"""
        subdir_includes = set()

        for subdir in subdirs_with_cmake:
            # 添加子目录根目录
            subdir_includes.add(subdir)

            # 扫描子目录中的include目录
            for root, dirs, files in os.walk(subdir):
                root_path = Path(root)

                # 过滤要排除的目录
                dirs[:] = [d for d in dirs if not d.startswith('.')]

                # 检查是否有头文件
                has_headers = any(f.endswith(tuple(self.header_extensions)) for f in files)

                if has_headers:
                    subdir_includes.add(root_path)

                    # 如果是include目录，停止向下搜索
                    if 'include' in root_path.name.lower():
                        dirs.clear()

        return subdir_includes

    def analyze_cmake(self, cmake_path):
        """分析子目录CMakeLists.txt"""
        try:
            content = cmake_path.read_text(encoding='utf-8', errors='ignore')

            # 查找DIR_NAME变量定义
            dir_name_var = None
            dir_name_matches = re.findall(r'get_filename_component\s*\(\s*(\w+)\s+\$\{CMAKE_CURRENT_SOURCE_DIR\}\s+NAME\s*\)', content, re.IGNORECASE)
            if dir_name_matches:
                dir_name_var = dir_name_matches[0]

            # 查找静态库定义
            static_libs = []

            # 匹配使用变量的库定义
            if dir_name_var:
                var_lib_matches = re.findall(rf'add_library\s*\(\s*\${{\s*{dir_name_var}\s*}}\s+STATIC', content, re.IGNORECASE)
                if var_lib_matches:
                    actual_dir_name = cmake_path.parent.name
                    static_libs.append(actual_dir_name)

                var_normal_matches = re.findall(rf'add_library\s*\(\s*\${{\s*{dir_name_var}\s*}}\s*(?!STATIC|SHARED)', content, re.IGNORECASE)
                if var_normal_matches:
                    actual_dir_name = cmake_path.parent.name
                    if actual_dir_name not in static_libs:
                        static_libs.append(actual_dir_name)

            # 匹配明确的静态库定义
            explicit_static_matches = re.findall(r'add_library\s*\(\s*([^\s\$\)]+)\s+STATIC', content, re.IGNORECASE)
            static_libs.extend(explicit_static_matches)

            # 匹配普通库定义
            normal_lib_matches = re.findall(r'add_library\s*\(\s*([^\s\$\)]+)\s*\)', content, re.IGNORECASE)
            for lib in normal_lib_matches:
                if lib not in static_libs and lib != dir_name_var:
                    static_libs.append(lib)

            # 查找源文件变量
            source_vars = []
            if dir_name_var:
                dir_source_matches = re.findall(rf'\${{\s*{dir_name_var}\s*}}_\w+', content, re.IGNORECASE)
                source_vars.extend(dir_source_matches)

            var_matches = re.findall(r'set\s*\(\s*([A-Z_]+_SOURCES?)\s', content, re.IGNORECASE)
            source_vars.extend(var_matches)

            other_vars = re.findall(r'set\s*\(\s*([A-Z_]+_FILES?)\s', content, re.IGNORECASE)
            source_vars.extend(other_vars)

            # 检查是否有target_sources指令
            target_sources = re.findall(r'target_sources\s*\(\s*([^\s\)]+)', content, re.IGNORECASE)

            return {
                'static_libraries': static_libs,
                'source_vars': source_vars,
                'target_sources': target_sources,
                'has_libraries': len(static_libs) > 0,
                'dir_name_var': dir_name_var
            }
        except Exception as e:
            print(f"警告: 分析 {cmake_path} 失败: {e}")
            return {
                'static_libraries': [],
                'source_vars': [],
                'target_sources': [],
                'has_libraries': False,
                'dir_name_var': None
            }

    def find_linker_script(self, chip_name):
        """查找链接脚本文件"""
        linker_scripts = []
        linker_extensions = {'.ld', '.lds'}

        for root, dirs, files in os.walk(self.project_root):
            root_path = Path(root)

            # 过滤要排除的目录
            dirs[:] = [d for d in dirs if d not in self.exclude_dirs and not d.startswith('.')]

            # 过滤BSP中不匹配的芯片目录
            dirs[:] = [d for d in dirs if not self.should_exclude_bsp_chip_dir(root_path, d)]

            for file in files:
                file_path = root_path / file

                if file_path.suffix.lower() in linker_extensions:
                    linker_scripts.append(file_path)

        if not linker_scripts:
            print(f"警告: 未找到链接脚本文件")
            return f"{chip_name}Tx_FLASH.ld"

        # 优先选择与芯片名称匹配的链接脚本
        chip_upper = chip_name.upper()
        for script in linker_scripts:
            script_name_upper = script.name.upper()
            if chip_upper in script_name_upper or any(part in script_name_upper for part in chip_upper.split('F')):
                rel_path = script.relative_to(self.project_root)
                print(f"找到匹配的链接脚本: {rel_path}")
                return rel_path.as_posix()

        # 如果没有找到匹配的，选择第一个链接脚本
        first_script = linker_scripts[0]
        rel_path = first_script.relative_to(self.project_root)
        print(f"使用找到的链接脚本: {rel_path}")
        return rel_path.as_posix()

    def scan_bsp_sources(self, bsp_chip_dir):
        """扫描BSP目录下的源文件"""
        bsp_sources = []
        bsp_include_dirs = set()

        bsp_root = self.project_root / 'BSP' / bsp_chip_dir
        if not bsp_root.exists():
            return bsp_sources, bsp_include_dirs

        # 扫描源文件
        for root, dirs, files in os.walk(bsp_root):
            root_path = Path(root)
            dirs[:] = [d for d in dirs if d not in self.exclude_dirs and not d.startswith('.')]

            for file in files:
                file_path = root_path / file
                if file_path.suffix in self.source_extensions:
                    bsp_sources.append(file_path)
                elif file_path.suffix in self.header_extensions:
                    bsp_include_dirs.add(root_path)

        # 扫描CMSIS头文件目录
        cmsis_root = self.project_root / 'BSP' / 'CMSIS'
        if cmsis_root.exists():
            for root, dirs, files in os.walk(cmsis_root):
                root_path = Path(root)
                dirs[:] = [d for d in dirs if d not in self.exclude_dirs and not d.startswith('.')]

                has_headers = any(f.endswith(tuple(self.header_extensions)) for f in files)
                if has_headers:
                    bsp_include_dirs.add(root_path)

        return sorted(bsp_sources), sorted(bsp_include_dirs)

    def scan_device_sources(self):
        """扫描Device目录下的源文件，按设备分组"""
        device_root = self.project_root / 'Device'
        if not device_root.exists():
            return {}

        devices = {}

        # 遍历Device下的所有子目录
        for item in device_root.iterdir():
            if not item.is_dir() or item.name.startswith('.') or item.name in self.exclude_dirs:
                continue

            device_name = item.name
            device_sources = []
            device_include_dirs = set()

            # 扫描设备源文件
            for root, dirs, files in os.walk(item):
                root_path = Path(root)
                dirs[:] = [d for d in dirs if d not in self.exclude_dirs and not d.startswith('.')]

                # 排除examples和tests目录
                dirs[:] = [d for d in dirs if d.lower() not in ['examples', 'tests', 'example', 'test']]

                for file in files:
                    file_path = root_path / file
                    if file_path.suffix in self.source_extensions:
                        device_sources.append(file_path)
                    elif file_path.suffix in self.header_extensions:
                        device_include_dirs.add(root_path)

            if device_sources:
                devices[device_name] = {
                    'sources': sorted(device_sources),
                    'include_dirs': sorted(device_include_dirs)
                }

        return devices
