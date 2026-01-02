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
        """查找链接脚本文件

        按新目录结构查找：
        BSP/<bsp_dir>/<model_dir>/<package>/stm32<model><package>.ld
        例如: BSP/stm32f4/f407/vgt6/stm32f4vgt6.ld

        优先级:
        1. 精确匹配封装目录下的链接脚本
        2. 型号目录下的通用链接脚本
        3. Core目录下的链接脚本
        4. 全局搜索
        """
        chip_lower = chip_name.lower()

        # 从芯片名解析各层目录信息
        # STM32F407VGT6 -> bsp_dir=stm32f4, model_dir=f407, package=vgt6
        import re
        bsp_dir = self.get_bsp_chip_dir(chip_name)  # stm32f4
        model_match = re.match(r'stm32([a-z]\d{3})', chip_lower)
        model_dir = model_match.group(1) if model_match else None  # f407
        package_match = re.match(r'stm32[a-z]\d{3}([a-z]\d*[a-z]*\d*)', chip_lower)
        chip_package = package_match.group(1) if package_match else None  # vgt6

        # 优先级1: 按新目录结构查找封装特定的链接脚本
        if bsp_dir and model_dir and chip_package:
            # BSP/stm32f4/f407/vgt6/stm32f4vgt6.ld
            package_ld_path = self.project_root / 'BSP' / bsp_dir / model_dir / chip_package
            if package_ld_path.exists():
                for ld_file in package_ld_path.glob('*.ld'):
                    rel_path = ld_file.relative_to(self.project_root)
                    print(f"找到封装特定链接脚本: {rel_path}")
                    return rel_path.as_posix()

        # 优先级2: 型号目录下的通用链接脚本
        if bsp_dir and model_dir:
            model_ld_path = self.project_root / 'BSP' / bsp_dir / model_dir
            if model_ld_path.exists():
                for ld_file in model_ld_path.glob('*.ld'):
                    rel_path = ld_file.relative_to(self.project_root)
                    print(f"找到型号通用链接脚本: {rel_path}")
                    return rel_path.as_posix()

        # 优先级3: Core目录下的链接脚本
        if bsp_dir:
            core_ld_path = self.project_root / 'BSP' / bsp_dir / 'Core'
            if core_ld_path.exists():
                for ld_file in core_ld_path.glob('*.ld'):
                    rel_path = ld_file.relative_to(self.project_root)
                    print(f"找到Core链接脚本: {rel_path}")
                    return rel_path.as_posix()

        # 优先级4: 全局搜索（兼容旧结构）
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

    def scan_bsp_sources(self, bsp_chip_dir, chip_package=None, chip_model_dir=None):
        """扫描BSP目录下的源文件

        新目录结构:
        BSP/<bsp_chip_dir>/Core/         # 核心启动文件
        BSP/<bsp_chip_dir>/Driver/       # 通用驱动
        BSP/<bsp_chip_dir>/<model_dir>/  # 型号共享驱动 (如 f407)
        BSP/<bsp_chip_dir>/<model_dir>/<package>/  # 封装特定配置 (如 vgt6)

        Args:
            bsp_chip_dir: 芯片系列目录名 (如 stm32f4)
            chip_package: 芯片封装型号 (如 vet6, vgt6)
            chip_model_dir: 芯片型号目录 (如 f407, f103)
        """
        bsp_sources = []
        bsp_include_dirs = set()

        bsp_root = self.project_root / 'BSP' / bsp_chip_dir
        if not bsp_root.exists():
            return bsp_sources, bsp_include_dirs

        # 扫描 Core 目录
        core_dir = bsp_root / 'Core'
        if core_dir.exists():
            self._scan_dir_for_sources(core_dir, bsp_sources, bsp_include_dirs)

        # 扫描 Driver 目录 (通用外设驱动)
        driver_dir = bsp_root / 'Driver'
        if driver_dir.exists():
            self._scan_dir_for_sources(driver_dir, bsp_sources, bsp_include_dirs, recursive=False)

        # 扫描型号目录 (如 f407)
        if chip_model_dir:
            model_dir = bsp_root / chip_model_dir
            if model_dir.exists():
                # 扫描型号目录下的共享驱动文件
                self._scan_dir_for_sources(model_dir, bsp_sources, bsp_include_dirs, recursive=False)

                # 扫描封装特定目录 (如 vgt6)
                if chip_package:
                    package_dir = model_dir / chip_package
                    if package_dir.exists():
                        bsp_include_dirs.add(package_dir)
                        # 封装目录通常只有头文件和链接脚本，不扫描 .c 文件

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

    def _scan_dir_for_sources(self, directory, sources_list, includes_set, recursive=True):
        """扫描指定目录的源文件和头文件

        Args:
            directory: 要扫描的目录
            sources_list: 源文件列表（会被修改）
            includes_set: 头文件目录集合（会被修改）
            recursive: 是否递归扫描子目录
        """
        if recursive:
            for root, dirs, files in os.walk(directory):
                root_path = Path(root)
                dirs[:] = [d for d in dirs if d not in self.exclude_dirs and not d.startswith('.')]

                for file in files:
                    file_path = root_path / file
                    if file_path.suffix in self.source_extensions:
                        sources_list.append(file_path)
                    elif file_path.suffix in self.header_extensions:
                        includes_set.add(root_path)
        else:
            # 只扫描当前目录，不递归
            for file in directory.iterdir():
                if file.is_file():
                    if file.suffix in self.source_extensions:
                        sources_list.append(file)
                    elif file.suffix in self.header_extensions:
                        includes_set.add(directory)

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
