#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import re
import argparse
import glob
import json
import platform
from pathlib import Path

class ProjectConfig:
    """项目配置管理类"""

    CONFIG_FILE = "build/project_config.json"

    DEFAULT_CONFIG = {
        "project": {
            "name": "",  # 自动从文件夹名获取
            "chip": "STM32F103C8",
            "board": "BLUEPILL",
            "architecture": "",  # 自动从芯片名推导
            "instruction_set": "thumb",
            "float_type": "",  # 自动从架构推导
        },
        "toolchain": {
            "c_compiler": "arm-none-eabi-gcc",
            "cxx_compiler": "arm-none-eabi-g++",
            "asm_compiler": "arm-none-eabi-gcc",
            "ar": "arm-none-eabi-ar",
            "objcopy": "arm-none-eabi-objcopy",
            "size": "arm-none-eabi-size"
        },
        "build": {
            "optimization": "O0",
            "debug_info": "g3",
            "warnings": True,
            "wall": True,
            "werror": False,
            "stack_usage": True
        },
        "linker": {
            "specs": ["nosys.specs", "nano.specs"],
            "script": "",  # 自动查找
            "gc_sections": True,
            "map_file": True,
            "printf_float": True,  # 默认允许printf输出浮点型
            "scanf_float": True   # 默认允许scanf输入浮点型
        },
        "defines": [
            "USE_HAL_DRIVER"
        ],
        "download": {
            "interface": "stlink",
            "target": "stm32f1x",
            "speed": "4000"
        },
        "files": {
            "sources": [],  # 源文件列表
            "headers": [],  # 头文件列表
            "include_dirs": [],  # 头文件目录列表
            "subdirs": [],  # 子目录列表
            "static_libraries": []  # 静态库列表
        }
    }

    def __init__(self, project_root="."):
        self.project_root = Path(project_root)
        # 确保 build 目录存在
        build_dir = self.project_root / "build"
        build_dir.mkdir(exist_ok=True)
        self.config_file = self.project_root / self.CONFIG_FILE
        self.config = self.load_config()

    def load_config(self):
        """加载配置文件"""
        if self.config_file.exists():
            try:
                with open(self.config_file, 'r', encoding='utf-8') as f:
                    config = json.load(f)
                # 合并默认配置，确保所有字段都存在
                return self._merge_config(self.DEFAULT_CONFIG, config)
            except Exception as e:
                print(f"加载配置文件失败: {e}")
                return self.DEFAULT_CONFIG.copy()
        return self.DEFAULT_CONFIG.copy()

    def save_config(self):
        """保存配置文件"""
        try:
            with open(self.config_file, 'w', encoding='utf-8') as f:
                json.dump(self.config, f, indent=2, ensure_ascii=False)
            print(f"配置已保存到: {self.config_file}")
        except Exception as e:
            print(f"保存配置文件失败: {e}")

    def _merge_config(self, default, user):
        """递归合并配置"""
        result = default.copy()
        for key, value in user.items():
            if key in result and isinstance(result[key], dict) and isinstance(value, dict):
                result[key] = self._merge_config(result[key], value)
            else:
                result[key] = value
        return result

    def get(self, path, default=None):
        """获取配置值，支持点号路径如 'project.chip'"""
        keys = path.split('.')
        value = self.config
        for key in keys:
            if isinstance(value, dict) and key in value:
                value = value[key]
            else:
                return default
        return value

    def set(self, path, value):
        """设置配置值"""
        keys = path.split('.')
        config = self.config
        for key in keys[:-1]:
            if key not in config:
                config[key] = {}
            config = config[key]
        config[keys[-1]] = value

class CMakeGenerator:
    def __init__(self, project_root):
        self.project_root = Path(project_root)
        self.cmake_file = self.project_root / "CMakeLists.txt"
        self.source_extensions = {'.c', '.cpp', '.s', '.S', '.asm'}
        self.header_extensions = {'.h', '.hpp'}

        # 配置管理
        self.config = ProjectConfig(project_root)

        # 检查是否为初次运行
        self.is_first_run = not self.config.config_file.exists()

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

        # 浮点单元配置映射
        self.fpu_config = {
            'cortex-m0': {'fpu': 'soft', 'float_abi': 'soft', 'description': '软件浮点'},
            'cortex-m0plus': {'fpu': 'soft', 'float_abi': 'soft', 'description': '软件浮点'},
            'cortex-m3': {'fpu': 'soft', 'float_abi': 'soft', 'description': '软件浮点'},
            'cortex-m4': {'fpu': 'fpv4-sp-d16', 'float_abi': 'hard', 'description': '硬件浮点 (FPU: fpv4-sp-d16)'},
            'cortex-m7': {'fpu': 'fpv5-d16', 'float_abi': 'hard', 'description': '硬件浮点 (FPU: fpv5-d16)'},
            'cortex-m33': {'fpu': 'fpv5-sp-d16', 'float_abi': 'hard', 'description': '硬件浮点 (FPU: fpv5-sp-d16)'},
            'cortex-m55': {'fpu': 'fpv5-d16', 'float_abi': 'hard', 'description': '硬件浮点 (FPU: fpv5-d16)'},
            'cortex-m85': {'fpu': 'fpv5-d16', 'float_abi': 'hard', 'description': '硬件浮点 (FPU: fpv5-d16)'},
        }

    def read_cmake_cache(self):
        """从CMake缓存读取工具链路径"""
        build_dir = self.project_root / "build"
        cache_file = build_dir / "CMakeCache.txt"

        toolchain_info = {
            "c_compiler": None,
            "cxx_compiler": None,
            "toolchain_path": None
        }

        if not cache_file.exists():
            print("警告: CMakeCache.txt 不存在，使用默认工具链路径")
            return toolchain_info

        try:
            with open(cache_file, 'r', encoding='utf-8', errors='ignore') as f:
                content = f.read()

            # 查找C编译器路径 - 多种模式
            c_compiler_path = None

            # 模式1: CMAKE_C_COMPILER:STRING=
            c_compiler_match = re.search(r'CMAKE_C_COMPILER:STRING=(.+)', content)
            if c_compiler_match:
                c_compiler_path = c_compiler_match.group(1).strip()
                # print("找到C编译器路径 (STRING模式)")
            else:
                # 模式2: CMAKE_C_COMPILER:FILEPATH=
                c_compiler_match = re.search(r'CMAKE_C_COMPILER:FILEPATH=(.+)', content)
                if c_compiler_match:
                    c_compiler_path = c_compiler_match.group(1).strip()
                    # print("找到C编译器路径 (FILEPATH模式)")
                else:
                    # 模式3: 尝试查找C++编译器作为备选
                    cxx_compiler_match = re.search(r'CMAKE_CXX_COMPILER:FILEPATH=(.+)', content)
                    if cxx_compiler_match:
                        cxx_compiler_path = cxx_compiler_match.group(1).strip()
                        # 从C++编译器路径推导C编译器路径
                        if 'g++' in cxx_compiler_path or 'c++' in cxx_compiler_path:
                            c_compiler_path = cxx_compiler_path.replace('g++', 'gcc').replace('c++', 'gcc')
                            # print("从C++编译器路径推导C编译器路径")
                    else:
                        # 模式4: 尝试其他可能的模式
                        other_patterns = [
                            r'CMAKE_C_COMPILER:INTERNAL=(.+)',
                            r'CMAKE_C_COMPILER=(.+)',
                            r'CMAKE_CXX_COMPILER:STRING=(.+)',
                            r'CMAKE_CXX_COMPILER:INTERNAL=(.+)'
                        ]

                        for pattern in other_patterns:
                            match = re.search(pattern, content)
                            if match:
                                found_path = match.group(1).strip()
                                if 'g++' in found_path or 'c++' in found_path:
                                    c_compiler_path = found_path.replace('g++', 'gcc').replace('c++', 'gcc')
                                elif 'gcc' in found_path:
                                    c_compiler_path = found_path
                                if c_compiler_path:
                                    # print(f"找到编译器路径 (模式: {pattern})")
                                    break

            if c_compiler_path:
                toolchain_info["c_compiler"] = c_compiler_path

                # 从编译器路径推导工具链目录
                c_compiler_path_obj = Path(c_compiler_path)
                if c_compiler_path_obj.exists():
                    # 工具链路径通常是编译器所在的bin目录
                    toolchain_info["toolchain_path"] = str(c_compiler_path_obj.parent)
                else:
                    # 即使文件不存在，也尝试从路径推导目录
                    toolchain_info["toolchain_path"] = str(c_compiler_path_obj.parent)
                    print("警告: 编译器文件不存在，但已推导出工具链路径")

            # 查找C++编译器路径 - 多种模式
            cxx_patterns = [
                r'CMAKE_CXX_COMPILER:STRING=(.+)',
                r'CMAKE_CXX_COMPILER:FILEPATH=(.+)',
                r'CMAKE_CXX_COMPILER:INTERNAL=(.+)',
                r'CMAKE_CXX_COMPILER=(.+)'
            ]

            for pattern in cxx_patterns:
                cxx_compiler_match = re.search(pattern, content)
                if cxx_compiler_match:
                    toolchain_info["cxx_compiler"] = cxx_compiler_match.group(1).strip()
                    # print(f"找到C++编译器路径 (模式: {pattern})")
                    break

            if toolchain_info["toolchain_path"]:
                print(f"从CMake缓存读取到工具链路径: {toolchain_info['toolchain_path']}")
            else:
                print("警告: 未能从CMake缓存中获取工具链路径")

            return toolchain_info

        except Exception as e:
            print(f"读取CMake缓存失败: {e}")
            return toolchain_info

    def update_config_from_detection(self, chip_name, sources=None, include_dirs=None, subdirs_with_cmake=None, static_libraries=None):
        """根据检测结果更新配置（仅在初次运行时）"""
        if not self.is_first_run:
            # 如果配置文件已存在，不自动更新配置
            return

        # 更新芯片信息
        self.config.set('project.chip', chip_name)

        # 检测并更新架构
        arch = self.detect_chip_arch(chip_name)
        self.config.set('project.architecture', arch)

        # 检测并更新浮点类型
        fpu_info = self.fpu_config.get(arch, self.fpu_config['cortex-m3'])
        self.config.set('project.float_type', fpu_info['description'])

        # 更新项目名称
        project_name = self.get_project_name()
        self.config.set('project.name', project_name)

        # 更新链接脚本
        linker_script = self.find_linker_script(chip_name)
        self.config.set('linker.script', linker_script)

        # 更新下载配置
        download_target = self.get_download_target(chip_name)
        self.config.set('download.target', download_target)

        # 添加芯片特定的宏定义
        chip_define = f"{chip_name}__Chipinfo__"
        defines = self.config.get('defines', [])
        if chip_define not in defines:
            defines.append(chip_define)
            self.config.set('defines', defines)

        # 保存文件信息
        if sources is not None:
            source_paths = [str(src.relative_to(self.project_root).as_posix()) for src in sources]
            self.config.set('files.sources', source_paths)

        if include_dirs is not None:
            include_paths = [str(inc.relative_to(self.project_root).as_posix()) for inc in include_dirs]
            self.config.set('files.include_dirs', include_paths)

        if subdirs_with_cmake is not None:
            subdir_paths = [str(sub.relative_to(self.project_root).as_posix()) for sub in subdirs_with_cmake]
            self.config.set('files.subdirs', subdir_paths)

        if static_libraries is not None:
            self.config.set('files.static_libraries', static_libraries)

    def get_download_target(self, chip_name):
        """根据芯片名称获取下载目标"""
        chip_upper = chip_name.upper()
        if chip_upper.startswith('STM32F0'):
            return 'stm32f0x'
        elif chip_upper.startswith('STM32F1'):
            return 'stm32f1x'
        elif chip_upper.startswith('STM32F2'):
            return 'stm32f2x'
        elif chip_upper.startswith('STM32F3'):
            return 'stm32f3x'
        elif chip_upper.startswith('STM32F4'):
            return 'stm32f4x'
        elif chip_upper.startswith('STM32F7'):
            return 'stm32f7x'
        elif chip_upper.startswith('STM32G0'):
            return 'stm32g0x'
        elif chip_upper.startswith('STM32G4'):
            return 'stm32g4x'
        elif chip_upper.startswith('STM32H7'):
            return 'stm32h7x'
        elif chip_upper.startswith('STM32L0'):
            return 'stm32l0x'
        elif chip_upper.startswith('STM32L1'):
            return 'stm32l1x'
        elif chip_upper.startswith('STM32L4'):
            return 'stm32l4x'
        elif chip_upper.startswith('STM32L5'):
            return 'stm32l5x'
        elif chip_upper.startswith('STM32U5'):
            return 'stm32u5x'
        elif chip_upper.startswith('STM32WB'):
            return 'stm32wbx'
        elif chip_upper.startswith('STM32WL'):
            return 'stm32wlx'
        else:
            return 'stm32f1x'  # 默认

    def get_project_name(self):
        """获取项目名称"""
        project_root_abs = self.project_root.resolve()
        project_name = project_root_abs.name

        if not project_name or project_name in ['.', '..']:
            project_name = "STM32_Project"

        # 处理特殊字符
        project_name = re.sub(r'[^a-zA-Z0-9_]', '_', project_name)
        return project_name

    def detect_chip_arch(self, chip_name):
        """检测芯片架构"""
        chip_upper = chip_name.upper()
        for chip_prefix, arch in self.chip_arch_map.items():
            if chip_upper.startswith(chip_prefix):
                return arch
        return 'cortex-m3'  # 默认

    def get_fpu_flags(self, architecture):
        """根据架构获取浮点单元配置"""
        config = self.fpu_config.get(architecture, self.fpu_config['cortex-m3'])

        if config['fpu'] == 'soft':
            return '-mfloat-abi=soft'
        else:
            return f'-mfpu={config["fpu"]} -mfloat-abi={config["float_abi"]}'

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

    def scan_include_dirs(self):
        """专门扫描头文件目录"""
        include_dirs = set()

        for root, dirs, files in os.walk(self.project_root):
            root_path = Path(root)

            # 过滤要排除的目录
            dirs[:] = [d for d in dirs if d not in self.exclude_dirs and not d.startswith('.')]

            # 检查当前目录是否包含头文件
            has_headers = any(f.endswith(tuple(self.header_extensions)) for f in files)

            if has_headers:
                # 特殊处理CMSIS目录 - 只添加Core/Include
                if 'cmsis' in str(root_path).lower():
                    if 'core' in str(root_path).lower() and 'include' in str(root_path).lower():
                        # 确保是BSP/CMSIS/Core/Include路径
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
            # 添加子目录根目录（通常包含头文件）
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

    def scan_sources(self, target_arch):
        """扫描源文件"""
        sources = []
        headers = []
        subdirs_with_cmake = []

        for root, dirs, files in os.walk(self.project_root):
            root_path = Path(root)

            # 过滤目录
            dirs[:] = [d for d in dirs if d not in self.exclude_dirs and not d.startswith('.')]

            # 检查子目录CMakeLists.txt
            if root_path != self.project_root and (root_path / "CMakeLists.txt").exists():
                subdirs_with_cmake.append(root_path)
                dirs.clear()  # 不再遍历子目录
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

            # 匹配使用变量的库定义: add_library(${DIR_NAME} STATIC ...)
            if dir_name_var:
                var_lib_matches = re.findall(rf'add_library\s*\(\s*\${{\s*{dir_name_var}\s*}}\s+STATIC', content, re.IGNORECASE)
                if var_lib_matches:
                    # 使用实际的目录名作为库名
                    actual_dir_name = cmake_path.parent.name
                    static_libs.append(actual_dir_name)

                # 也匹配普通的变量库定义: add_library(${DIR_NAME} ...)
                var_normal_matches = re.findall(rf'add_library\s*\(\s*\${{\s*{dir_name_var}\s*}}\s*(?!STATIC|SHARED)', content, re.IGNORECASE)
                if var_normal_matches:
                    actual_dir_name = cmake_path.parent.name
                    if actual_dir_name not in static_libs:
                        static_libs.append(actual_dir_name)

            # 匹配明确的静态库定义: add_library(name STATIC ...)
            explicit_static_matches = re.findall(r'add_library\s*\(\s*([^\s\$\)]+)\s+STATIC', content, re.IGNORECASE)
            static_libs.extend(explicit_static_matches)

            # 匹配普通库定义: add_library(name ...)
            normal_lib_matches = re.findall(r'add_library\s*\(\s*([^\s\$\)]+)\s*\)', content, re.IGNORECASE)
            for lib in normal_lib_matches:
                if lib not in static_libs and lib != dir_name_var:  # 排除变量名本身
                    static_libs.append(lib)

            # 查找源文件变量
            source_vars = []
            if dir_name_var:
                # 匹配使用DIR_NAME的源文件变量: ${DIR_NAME}_Source
                dir_source_matches = re.findall(rf'\${{\s*{dir_name_var}\s*}}_\w+', content, re.IGNORECASE)
                source_vars.extend(dir_source_matches)

            # 匹配普通源文件变量
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

        # 常见的链接脚本文件扩展名
        linker_extensions = {'.ld', '.lds'}

        for root, dirs, files in os.walk(self.project_root):
            root_path = Path(root)

            # 过滤要排除的目录
            dirs[:] = [d for d in dirs if d not in self.exclude_dirs and not d.startswith('.')]

            for file in files:
                file_path = root_path / file

                # 检查是否是链接脚本文件
                if file_path.suffix.lower() in linker_extensions:
                    linker_scripts.append(file_path)

        if not linker_scripts:
            print(f"警告: 未找到链接脚本文件")
            return f"{chip_name}Tx_FLASH.ld"  # 返回默认名称

        # 优先选择与芯片名称匹配的链接脚本
        chip_upper = chip_name.upper()
        for script in linker_scripts:
            script_name_upper = script.name.upper()
            # 检查文件名是否包含芯片名称
            if chip_upper in script_name_upper or any(part in script_name_upper for part in chip_upper.split('F')):
                rel_path = script.relative_to(self.project_root)
                print(f"找到匹配的链接脚本: {rel_path}")
                return rel_path.as_posix()

        # 如果没有找到匹配的，选择第一个链接脚本
        first_script = linker_scripts[0]
        rel_path = first_script.relative_to(self.project_root)
        print(f"使用找到的链接脚本: {rel_path}")
        return rel_path.as_posix()

    def generate_cmake(self, chip_name=None, board_name=None):
        """生成CMakeLists.txt内容"""
        if self.is_first_run:
            # 初次运行：使用传入参数或默认值进行自动检测
            chip_name = chip_name or self.config.get('project.chip')
            board_name = board_name or self.config.get('project.board')

            print("=== 初次运行，自动检测配置 ===")

            # 扫描文件系统
            arch = self.detect_chip_arch(chip_name)
            sources, subdirs_with_cmake = self.scan_sources(arch)
            include_dirs = self.scan_include_dirs()
            subdir_includes = self.scan_subdir_includes(subdirs_with_cmake)
            all_include_dirs = include_dirs.union(subdir_includes)

            # 分析子目录并获取静态库
            static_libraries = []
            for subdir in subdirs_with_cmake:
                analysis = self.analyze_cmake(subdir / "CMakeLists.txt")
                if analysis['has_libraries']:
                    static_libraries.extend(analysis['static_libraries'])
                else:
                    static_libraries.append(subdir.name)

            # 更新配置（包含文件信息）
            self.update_config_from_detection(
                chip_name, sources, all_include_dirs, subdirs_with_cmake, static_libraries
            )

        else:
            # 配置文件已存在：完全使用配置文件中的值
            chip_name = self.config.get('project.chip')
            board_name = self.config.get('project.board')

            print("=== 使用现有配置文件 ===")

            # 从配置文件读取文件信息
            source_paths = self.config.get('files.sources', [])
            sources = [self.project_root / path for path in source_paths]

            include_paths = self.config.get('files.include_dirs', [])
            all_include_dirs = {self.project_root / path for path in include_paths}

            subdir_paths = self.config.get('files.subdirs', [])
            subdirs_with_cmake = [self.project_root / path for path in subdir_paths]

            static_libraries = self.config.get('files.static_libraries', [])

            print(f"从配置读取: {len(sources)} 个源文件")
            print(f"从配置读取: {len(all_include_dirs)} 个头文件目录")
            print(f"从配置读取: {len(subdirs_with_cmake)} 个子目录")
            print(f"从配置读取: {len(static_libraries)} 个静态库")

        # 从配置获取各种参数
        arch = self.config.get('project.architecture')
        project_name = self.config.get('project.name')

        # 如果项目名称为空，重新获取
        if not project_name:
            project_name = self.get_project_name()
            if self.is_first_run:
                self.config.set('project.name', project_name)

        fpu_flags = self.get_fpu_flags(arch)
        float_desc = self.config.get('project.float_type')
        linker_script = self.config.get('linker.script')

        # 如果链接脚本为空，重新查找
        if not linker_script:
            linker_script = self.find_linker_script(chip_name)
            if self.is_first_run:
                self.config.set('linker.script', linker_script)

        # 构建编译标志
        optimization = self.config.get('build.optimization')
        debug_info = self.config.get('build.debug_info')
        warning_flags = []
        if self.config.get('build.warnings'):
            if self.config.get('build.wall'):
                warning_flags.append('-Wall')
            if self.config.get('build.werror'):
                warning_flags.append('-Werror')
        if self.config.get('build.stack_usage'):
            warning_flags.append('-fstack-usage')

        warning_str = ' '.join(warning_flags)

        # 构建链接器specs参数
        specs_list = self.config.get('linker.specs', [])
        specs_flags = ' '.join([f'-specs={spec}' for spec in specs_list])

        # 构建其他链接器选项
        linker_options = []
        if self.config.get('linker.gc_sections', True):
            linker_options.append('-Wl,--gc-sections')
        if self.config.get('linker.map_file', True):
            linker_options.append('-Wl,-Map=${PROJECT_NAME}.map,--cref')

        # 添加printf/scanf浮点支持
        if self.config.get('linker.printf_float', True):
            linker_options.append('-u _printf_float')
        if self.config.get('linker.scanf_float', True):
            linker_options.append('-u _scanf_float')

        # 添加内存使用信息打印
        linker_options.append('-Wl,--print-memory-usage')

        linker_options_str = ' '.join(linker_options)

        print(f"项目名称: {project_name}")
        print(f"芯片型号: {chip_name}")
        print(f"芯片架构: {arch}")
        print(f"浮点配置: {float_desc}")
        print(f"链接脚本: {linker_script}")
        print(f"链接器specs: {specs_list}")

        # 生成CMakeLists.txt内容
        content = f'''#本文件由自动化脚本生成,改这个文件是没用的,要更改配置请修改项目根目录下的 project_config.json 文件,然后再次运行脚本,即可更改编译配置
#未经作者允许,不可更改脚本内容,不可传播,脚本源码闭源,只可在实验室内部使用,否则本人线下追究责任,脚本版权归作者本人和实验室所有
cmake_minimum_required(VERSION 3.16)

# 项目配置
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR {arch})
set(TARGET_CHIP {chip_name})
set(TARGET_BOARD {board_name})

# 工具链
set(CMAKE_C_COMPILER {self.config.get('toolchain.c_compiler')})
set(CMAKE_CXX_COMPILER {self.config.get('toolchain.cxx_compiler')})
set(CMAKE_ASM_COMPILER {self.config.get('toolchain.asm_compiler')})
set(CMAKE_AR {self.config.get('toolchain.ar')})
set(CMAKE_OBJCOPY {self.config.get('toolchain.objcopy')})
set(SIZE {self.config.get('toolchain.size')})
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

project({project_name} C CXX ASM)

# 编译标志 ({float_desc})
set(CMAKE_C_FLAGS "-mcpu={arch} -mthumb {fpu_flags} -fdata-sections -ffunction-sections {warning_str} -{optimization} -{debug_info}")
set(CMAKE_CXX_FLAGS "-mcpu={arch} -mthumb {fpu_flags} -fdata-sections -ffunction-sections {warning_str} -{optimization} -{debug_info}")
set(CMAKE_ASM_FLAGS "-mcpu={arch} -mthumb {fpu_flags} -fdata-sections -ffunction-sections {warning_str} -{optimization} -{debug_info}")

# 链接标志
set(CMAKE_EXE_LINKER_FLAGS "-mcpu={arch} -mthumb {fpu_flags} {specs_flags} -T${{CMAKE_SOURCE_DIR}}/{linker_script} {linker_options_str}")

# 宏定义
'''

        # 添加宏定义
        for define in self.config.get('defines', []):
            content += f'add_definitions(-D{define})\n'
        content += '\n'

        # 添加包含目录
        if all_include_dirs:
            content += "# 头文件目录\ninclude_directories(\n"
            sorted_dirs = sorted(all_include_dirs, key=lambda x: (len(x.parts), str(x)))
            for inc_dir in sorted_dirs:
                if isinstance(inc_dir, str):
                    content += f"    {inc_dir}\n"
                else:
                    rel_path = inc_dir.relative_to(self.project_root)
                    content += f"    {rel_path.as_posix()}\n"
            content += ")\n\n"

        # 源文件分组
        source_groups = {}
        for source in sources:
            if isinstance(source, str):
                source_path = Path(source)
            else:
                source_path = source.relative_to(self.project_root)

            dir_name = source_path.parent.name or "root"
            if dir_name not in source_groups:
                source_groups[dir_name] = []

            if isinstance(source, str):
                source_groups[dir_name].append(source_path)
            else:
                source_groups[dir_name].append(source_path)

        if source_groups:
            content += "# 源文件分组\n"
            all_vars = []
            for group_name, group_sources in sorted(source_groups.items()):
                if group_sources:
                    var_name = f"{group_name.upper()}_SOURCES"
                    all_vars.append(var_name)
                    content += f"set({var_name}\n"
                    for source in sorted(group_sources):
                        content += f"    {source.as_posix()}\n"
                    content += ")\n\n"

            content += "# 所有源文件\nset(SOURCE_FILES\n"
            for var in all_vars:
                content += f"    ${{{var}}}\n"
            content += ")\n\n"
        else:
            content += "# 所有源文件\nset(SOURCE_FILES)\n\n"

        # 子目录
        if subdirs_with_cmake:
            content += "# 子目录\n"
            for subdir in sorted(subdirs_with_cmake):
                if isinstance(subdir, str):
                    content += f"add_subdirectory({subdir})\n"
                else:
                    rel_path = subdir.relative_to(self.project_root)
                    content += f"add_subdirectory({rel_path.as_posix()})\n"
            content += "\n"

        # 可执行文件和链接
        content += f"# 创建可执行文件\nadd_executable(${{PROJECT_NAME}}.elf ${{SOURCE_FILES}})\n\n"

        if static_libraries:
            content += "# 链接静态库\ntarget_link_libraries(${PROJECT_NAME}.elf\n"
            unique_libs = sorted(set(static_libraries))
            for lib in unique_libs:
                content += f"    {lib}\n"
            content += ")\n\n"

        content += '''# 生成输出文件
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${PROJECT_NAME}.elf> ${PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${PROJECT_NAME}.elf> ${PROJECT_NAME}.bin
    COMMAND ${SIZE} --format=berkeley $<TARGET_FILE:${PROJECT_NAME}.elf>
    COMMENT "Building output files"
)
'''

        return content

    def generate_idea_cfg(self):
        """生成idea.cfg文件"""
        interface = self.config.get('download.interface', 'stlink')
        target = self.config.get('download.target', 'stm32f1x')
        speed = self.config.get('download.speed', '4000')

        idea_cfg_content = f'''source [find interface/{interface}.cfg]
source [find target/{target}.cfg]
adapter speed {speed}
'''

        # 确保 build 目录存在
        build_dir = self.project_root / 'build'
        build_dir.mkdir(exist_ok=True)
        idea_cfg_file = build_dir / 'idea.cfg'
        try:
            idea_cfg_file.write_text(idea_cfg_content, encoding='utf-8')
            print(f"生成 idea.cfg 文件")
        except Exception as e:
            print(f"生成 idea.cfg 失败: {e}")

    def generate_vscode_config(self):
        """生成VS Code配置文件"""
        vscode_dir = self.project_root / '.vscode'
        vscode_dir.mkdir(exist_ok=True)

        # 生成c_cpp_properties.json
        self.generate_c_cpp_properties(vscode_dir)

        # 生成launch.json
        self.generate_launch_json(vscode_dir)

        # 生成tasks.json
        self.generate_tasks_json(vscode_dir)

        # 生成settings.json
        self.generate_settings_json(vscode_dir)

        print("已生成 VS Code 配置文件")

    def generate_clangd_config(self):
        """生成clangd配置文件"""
        # 读取CMake缓存获取工具链路径
        toolchain_info = self.read_cmake_cache()

        # 尝试从compile_commands.json中获取实际的编译器路径
        actual_compiler_path = self.get_compiler_from_compile_commands()

        # 获取架构信息
        arch = self.config.get('project.architecture', 'cortex-m3')
        chip = self.config.get('project.chip', 'STM32F103C8')

        # 获取头文件路径
        include_paths = self.config.get('files.include_dirs', [])

        # 获取宏定义
        defines = self.config.get('defines', [])

        # 根据架构添加相应的宏
        arch_defines = []
        if 'cortex-m0' in arch:
            arch_defines.extend(['__CORTEX_M=0', 'ARM_MATH_CM0'])
        elif 'cortex-m3' in arch:
            arch_defines.extend(['__CORTEX_M=3', 'ARM_MATH_CM3'])
        elif 'cortex-m4' in arch:
            arch_defines.extend(['__CORTEX_M=4', 'ARM_MATH_CM4'])
        elif 'cortex-m7' in arch:
            arch_defines.extend(['__CORTEX_M=7', 'ARM_MATH_CM7'])

        # 使用实际的编译器路径
        compiler_path = actual_compiler_path or toolchain_info["c_compiler"]
        if not compiler_path:
            compiler_path = self.config.get('toolchain.c_compiler', 'arm-none-eabi-gcc')

        # 获取浮点单元配置
        fpu_flags = self.get_fpu_flags(arch)

        # 构建编译标志
        compile_flags = [
            f"-mcpu={arch}",
            "-mthumb",
            fpu_flags,
            "-fdata-sections",
            "-ffunction-sections",
            "-Wall",
            "-O0",
            "-g3",
            "--target=arm-none-eabi",
            "-nostdlib",
            "-ffreestanding"
        ]

        # 添加宏定义
        all_defines = defines + arch_defines + [
            "__GNUC__",
            "__ARM_ARCH",
            "ARM_MATH_MATRIX_CHECK",
            "ARM_MATH_ROUNDING"
        ]

        for define in all_defines:
            compile_flags.append(f"-D{define}")

        # 添加头文件路径
        for include_path in include_paths:
            compile_flags.append(f"-I{include_path}")

        # 添加工具链的系统头文件路径
        if compiler_path and '/' in compiler_path:
            import os
            import glob

            # 从编译器路径推导工具链目录
            compiler_dir = os.path.dirname(compiler_path)
            toolchain_base = os.path.dirname(compiler_dir)

            print(f"检测到工具链路径: {toolchain_base}")

            # ARM工具链的标准头文件路径
            possible_include_paths = [
                f"{toolchain_base}/arm-none-eabi/include",
                f"{toolchain_base}/arm-none-eabi/include/c++/*",
                f"{toolchain_base}/lib/gcc/arm-none-eabi/*/include",
                f"{toolchain_base}/lib/gcc/arm-none-eabi/*/include-fixed"
            ]

            for path_pattern in possible_include_paths:
                if '*' in path_pattern:
                    # 对于包含通配符的路径，尝试展开
                    expanded_paths = glob.glob(path_pattern)
                    for expanded_path in sorted(expanded_paths):
                        if os.path.exists(expanded_path):
                            compile_flags.append(f"-I{expanded_path}")
                            print(f"添加系统头文件路径: {expanded_path}")
                else:
                    if os.path.exists(path_pattern):
                        compile_flags.append(f"-I{path_pattern}")
                        print(f"添加系统头文件路径: {path_pattern}")

        # 生成.clangd配置文件内容
        clangd_config = {
            "CompileFlags": {
                "Add": compile_flags,
                "CompilationDatabase": "build"
            },
            "Index": {
                "Background": "Build",
                "StandardLibrary": False
            },
            "InlayHints": {
                "Enabled": True,
                "ParameterNames": True,
                "DeducedTypes": True
            },
            "Hover": {
                "ShowAKA": True
            },
            "Diagnostics": {
                "ClangTidy": {
                    "Add": [
                        "readability-*",
                        "bugprone-*",
                        "performance-*"
                    ],
                    "Remove": [
                        "readability-magic-numbers",
                        "readability-identifier-length"
                    ]
                },
                "Suppress": [
                    "pp_file_not_found",  # 抑制找不到文件的警告（系统头文件）
                    "unknown_warning_option"  # 抑制未知警告选项
                ]
            }
        }

        clangd_file = self.project_root / '.clangd'
        try:
            import yaml
            with open(clangd_file, 'w', encoding='utf-8') as f:
                yaml.dump(clangd_config, f, default_flow_style=False, allow_unicode=True, sort_keys=False)
            print("已生成 .clangd 配置文件 (YAML格式)")
        except ImportError:
            # 如果没有yaml库，使用JSON格式
            import json
            with open(clangd_file, 'w', encoding='utf-8') as f:
                json.dump(clangd_config, f, indent=2, ensure_ascii=False)
            print("已生成 .clangd 配置文件 (JSON格式)")
        except Exception as e:
            print(f"生成 .clangd 配置文件失败: {e}")

    def generate_compile_flags_txt(self):
        """生成compile_flags.txt文件作为clangd的备用配置"""
        # 获取架构信息
        arch = self.config.get('project.architecture', 'cortex-m3')

        # 获取头文件路径
        include_paths = self.config.get('files.include_dirs', [])

        # 获取宏定义
        defines = self.config.get('defines', [])

        # 根据架构添加相应的宏
        arch_defines = []
        if 'cortex-m0' in arch:
            arch_defines.extend(['__CORTEX_M=0', 'ARM_MATH_CM0'])
        elif 'cortex-m3' in arch:
            arch_defines.extend(['__CORTEX_M=3', 'ARM_MATH_CM3'])
        elif 'cortex-m4' in arch:
            arch_defines.extend(['__CORTEX_M=4', 'ARM_MATH_CM4'])
        elif 'cortex-m7' in arch:
            arch_defines.extend(['__CORTEX_M=7', 'ARM_MATH_CM7'])

        # 获取浮点单元配置
        fpu_flags = self.get_fpu_flags(arch)

        # 构建编译标志
        compile_flags = [
            f"-mcpu={arch}",
            "-mthumb",
            fpu_flags,
            "-fdata-sections",
            "-ffunction-sections",
            "-Wall",
            "-O0",
            "-g3",
            "--target=arm-none-eabi",
            "-nostdlib",
            "-ffreestanding"
        ]

        # 添加宏定义
        all_defines = defines + arch_defines + [
            "__GNUC__",
            "__ARM_ARCH",
            "ARM_MATH_MATRIX_CHECK",
            "ARM_MATH_ROUNDING"
        ]

        for define in all_defines:
            compile_flags.append(f"-D{define}")

        # 添加头文件路径
        for include_path in include_paths:
            compile_flags.append(f"-I{include_path}")

        # 尝试从compile_commands.json中获取实际的编译器路径并添加系统头文件
        actual_compiler_path = self.get_compiler_from_compile_commands()
        if actual_compiler_path and '/' in actual_compiler_path:
            import os
            import glob

            compiler_dir = os.path.dirname(actual_compiler_path)
            toolchain_base = os.path.dirname(compiler_dir)

            # ARM工具链的标准头文件路径
            possible_include_paths = [
                f"{toolchain_base}/arm-none-eabi/include",
                f"{toolchain_base}/lib/gcc/arm-none-eabi/*/include",
                f"{toolchain_base}/lib/gcc/arm-none-eabi/*/include-fixed"
            ]

            for path_pattern in possible_include_paths:
                if '*' in path_pattern:
                    expanded_paths = glob.glob(path_pattern)
                    for expanded_path in sorted(expanded_paths):
                        if os.path.exists(expanded_path):
                            compile_flags.append(f"-I{expanded_path}")
                else:
                    if os.path.exists(path_pattern):
                        compile_flags.append(f"-I{path_pattern}")

        # 生成compile_flags.txt文件
        compile_flags_file = self.project_root / 'compile_flags.txt'
        try:
            with open(compile_flags_file, 'w', encoding='utf-8') as f:
                for flag in compile_flags:
                    f.write(f"{flag}\n")
            print("已生成 compile_flags.txt 文件")
        except Exception as e:
            print(f"生成 compile_flags.txt 失败: {e}")

    def get_compiler_from_compile_commands(self):
        """从compile_commands.json中获取编译器路径"""
        build_dir = self.project_root / "build"
        compile_commands_file = build_dir / "compile_commands.json"

        if not compile_commands_file.exists():
            return None

        try:
            import json
            with open(compile_commands_file, 'r', encoding='utf-8') as f:
                commands = json.load(f)

            if commands and len(commands) > 0:
                # 取第一个编译命令中的编译器路径
                command = commands[0].get('command', '')
                if command:
                    # 提取编译器路径（命令的第一部分）
                    parts = command.split()
                    if parts:
                        compiler_path = parts[0]
                        print(f"从compile_commands.json检测到编译器: {compiler_path}")
                        return compiler_path
        except Exception as e:
            print(f"读取compile_commands.json失败: {e}")

        return None

    def generate_c_cpp_properties(self, vscode_dir):
        """生成c_cpp_properties.json"""
        # 读取CMake缓存获取工具链路径
        toolchain_info = self.read_cmake_cache()

        # 获取头文件路径
        include_paths = self.config.get('files.include_dirs', [])
        # 转换为绝对路径
        absolute_includes = [f"${{workspaceFolder}}/{path}" for path in include_paths]

        # 添加工具链路径
        absolute_includes.extend([
            "${workspaceFolder}/**",
            "C:/Program Files (x86)/Arm GNU Toolchain arm-none-eabi/*/arm-none-eabi/include/**",
            "C:/Program Files (x86)/Arm GNU Toolchain arm-none-eabi/*/lib/gcc/arm-none-eabi/*/include/**"
        ])

        # 获取宏定义
        defines = self.config.get('defines', [])

        # 获取架构信息
        arch = self.config.get('project.architecture', 'cortex-m3')
        chip = self.config.get('project.chip', 'STM32F103C8')

        # 根据架构添加相应的宏
        arch_defines = []
        if 'cortex-m0' in arch:
            arch_defines.extend(['__CORTEX_M=0', 'ARM_MATH_CM0'])
        elif 'cortex-m3' in arch:
            arch_defines.extend(['__CORTEX_M=3', 'ARM_MATH_CM3'])
        elif 'cortex-m4' in arch:
            arch_defines.extend(['__CORTEX_M=4', 'ARM_MATH_CM4'])
        elif 'cortex-m7' in arch:
            arch_defines.extend(['__CORTEX_M=7', 'ARM_MATH_CM7'])

        # 使用CMake缓存中的编译器路径，如果不存在则使用默认路径
        compiler_path = toolchain_info["c_compiler"]
        if not compiler_path:
            compiler_path = f"C:/Program Files (x86)/Arm GNU Toolchain arm-none-eabi/*/bin/{self.config.get('toolchain.c_compiler', 'arm-none-eabi-gcc')}.exe"

        c_cpp_properties = {
            "configurations": [
                {
                    "name": "STM32",
                    "includePath": absolute_includes,
                    "defines": defines + arch_defines + [
                        "__GNUC__",
                        "__ARM_ARCH",
                        "ARM_MATH_MATRIX_CHECK",
                        "ARM_MATH_ROUNDING"
                    ],
                    "compilerPath": compiler_path,
                    "cStandard": "c11",
                    "cppStandard": "c++17",
                    "intelliSenseMode": "gcc-arm",
                    "compilerArgs": [
                        f"-mcpu={arch}",
                        "-mthumb",
                        self.get_fpu_flags(arch),
                        "-fdata-sections",
                        "-ffunction-sections"
                    ]
                }
            ],
            "version": 4
        }

        c_cpp_file = vscode_dir / 'c_cpp_properties.json'
        try:
            with open(c_cpp_file, 'w', encoding='utf-8') as f:
                json.dump(c_cpp_properties, f, indent=2, ensure_ascii=False)
        except Exception as e:
            print(f"生成 c_cpp_properties.json 失败: {e}")

    def generate_launch_json(self, vscode_dir):
        """生成launch.json"""
        project_name = self.config.get('project.name', '32_temp_project')
        interface = self.config.get('download.interface', 'stlink')
        target = self.config.get('download.target', 'stm32f1x')
        speed = self.config.get('download.speed', '4000')

        # 检测当前操作系统平台
        current_platform = platform.system().lower()

        # 根据接口类型选择调试器
        if interface == 'jlink':
            server_type = 'jlink'
            device = self.config.get('project.chip', 'STM32F103C8')
        else:
            server_type = 'openocd'
            device = None

        launch_config = {
            "version": "0.2.0",
            "configurations": [
                {
                    "name": "Debug STM32",
                    "cwd": "${workspaceFolder}",
                    "executable": f"./build/{project_name}.elf",
                    "request": "launch",
                    "type": "cortex-debug",
                    "runToEntryPoint": "main",
                    "showDevDebugOutput": "raw",
                    "servertype": server_type
                }
            ]
        }

        # 在Linux上使用gdb-multiarch，Windows上使用工具链的GDB
        if current_platform != "windows":
            launch_config["configurations"][0]["gdbPath"] = "gdb-multiarch"

        if server_type == 'openocd':
            # 根据平台设置OpenOCD搜索路径
            if current_platform == "windows":
                search_dirs = [
                    "${workspaceFolder}",
                    "C:/openocd/share/openocd/scripts/"
                ]
            else:
                search_dirs = [
                    "${workspaceFolder}",
                    "/usr/local/share/openocd/scripts/"
                ]

            launch_config["configurations"][0].update({
                "configFiles": [
                    f"interface/{interface}.cfg",
                    f"target/{target}.cfg"
                ],
                "searchDir": search_dirs,
                "openOCDLaunchCommands": [
                    f"adapter speed {speed}"
                ]
            })
        else:  # jlink
            launch_config["configurations"][0].update({
                "device": device,
                "interface": "swd"
            })

        launch_file = vscode_dir / 'launch.json'
        try:
            with open(launch_file, 'w', encoding='utf-8') as f:
                json.dump(launch_config, f, indent=2, ensure_ascii=False)
        except Exception as e:
            print(f"生成 launch.json 失败: {e}")

    def generate_tasks_json(self, vscode_dir):
        """生成tasks.json"""
        project_name = self.config.get('project.name', 'project')
        current_platform = platform.system().lower()
        command = "python3"
        if current_platform == "windows":
            command = "python"
        tasks_config = {
            "version": "2.0.0",
            "tasks": [
                {
                    "label": "download",
                    "type": "shell",
                    "command": command,
                    "args": [
                        "${workspaceFolder}/download.py"
                    ],
                    "problemMatcher": [],
                    "group": {
                        "kind": "build",
                        "isDefault": False
                    }
                }
            ]
        }

        tasks_file = vscode_dir / 'tasks.json'
        try:
            with open(tasks_file, 'w', encoding='utf-8') as f:
                json.dump(tasks_config, f, indent=2, ensure_ascii=False)
        except Exception as e:
            print(f"生成 tasks.json 失败: {e}")

    def generate_settings_json(self, vscode_dir):
        """生成settings.json"""
        # 读取CMake缓存获取工具链路径
        toolchain_info = self.read_cmake_cache()

        # 检测当前操作系统平台
        current_platform = platform.system().lower()

        # 确定工具链路径和GDB路径
        if toolchain_info["toolchain_path"]:
            # 使用从CMake缓存读取的路径
            toolchain_path = toolchain_info["toolchain_path"]
            if current_platform == "windows":
                gdb_path = str(Path(toolchain_path) / "arm-none-eabi-gdb.exe")
            else:
                # 在Linux上优先使用系统的gdb-multiarch，它更兼容
                gdb_path = "gdb-multiarch"
        else:
            # 使用默认路径
            if current_platform == "windows":
                toolchain_path = "C:/Program Files (x86)/Arm GNU Toolchain arm-none-eabi/13.2 Rel1/bin"
                gdb_path = "C:/Program Files (x86)/Arm GNU Toolchain arm-none-eabi/13.2 Rel1/bin/arm-none-eabi-gdb.exe"
            else:
                toolchain_path = "/usr/bin"  # Linux/macOS 默认路径
                gdb_path = "gdb-multiarch"  # 使用系统的gdb-multiarch

        # 根据操作系统平台选择CMake生成器
        if current_platform == "windows":
            cmake_generator = "Ninja"
            print("检测到Windows平台，使用Ninja生成器")
        elif current_platform == "linux":
            cmake_generator = "Unix Makefiles"
            print("检测到Linux平台，使用Unix Makefiles生成器")
        elif current_platform == "darwin":  # macOS
            cmake_generator = "Unix Makefiles"
            print("检测到macOS平台，使用Unix Makefiles生成器")
        else:
            cmake_generator = "Ninja"  # 默认使用Ninja
            print(f"检测到未知平台 {current_platform}，使用默认Ninja生成器")

        # 根据平台设置特定的路径和配置
        if current_platform == "windows":
            openocd_path = "C:/openocd/bin/openocd.exe"
            terminal_shell = "cmd.exe"
        else:
            openocd_path = "/usr/bin/openocd"  # Linux/macOS 默认路径
            terminal_shell = "/bin/bash"

        settings_config = {
            "files.associations": {
                "*.h": "c",
                "*.c": "c",
                "*.hpp": "cpp",
                "*.cpp": "cpp",
                "*.ld": "text",
                "*.lds": "text"
            },
            "cortex-debug.openocdPath": openocd_path,
            "cortex-debug.armToolchainPath": toolchain_path,
            "cortex-debug.gdbPath": gdb_path,
            "cmake.configureOnOpen": True,
            "cmake.buildDirectory": "${workspaceFolder}/build",
            "cmake.generator": cmake_generator,
            "C_Cpp.default.configurationProvider": "ms-vscode.cmake-tools",
            "editor.formatOnSave": True,
            "editor.insertSpaces": True,
            "editor.tabSize": 4,
            "files.trimTrailingWhitespace": True,
            "files.encoding": "utf8"
        }

        # 只在Windows上设置terminal shell
        if current_platform == "windows":
            settings_config["terminal.integrated.shell.windows"] = terminal_shell

        settings_file = vscode_dir / 'settings.json'
        try:
            with open(settings_file, 'w', encoding='utf-8') as f:
                json.dump(settings_config, f, indent=2, ensure_ascii=False)
            print(f"使用工具链路径: {toolchain_path}")
        except Exception as e:
            print(f"生成 settings.json 失败: {e}")

    def generate_extensions_json(self, vscode_dir):
        """生成extensions.json推荐扩展列表"""
        extensions_config = {
            "recommendations": [
                "ms-vscode.cpptools",
                "ms-vscode.cmake-tools",
                "marus25.cortex-debug",
                "ms-vscode.vscode-serial-monitor",
                "webfreak.debug",
                "dan-c-underwood.arm",
                "zixuanwang.linkerscript",
                "jeff-hykin.better-cpp-syntax",
                "ms-vscode.hexeditor"
            ]
        }

        extensions_file = vscode_dir / 'extensions.json'
        try:
            with open(extensions_file, 'w', encoding='utf-8') as f:
                json.dump(extensions_config, f, indent=2, ensure_ascii=False)
        except Exception as e:
            print(f"生成 extensions.json 失败: {e}")

    def add_manual_refresh_option(self):
        """添加手动刷新选项"""
        if not self.is_first_run:
            refresh = input("\n是否重新扫描项目文件? (y/N): ").strip().lower()
            if refresh in ['y', 'yes']:
                print("重新扫描项目文件...")
                # 临时设置为初次运行模式以触发重新扫描
                self.is_first_run = True
                return True
        return False

    def generate(self, chip_name=None, board_name=None, force=True):
        """生成CMakeLists.txt和配置文件"""
        # if self.cmake_file.exists() and not force:
        #     print("CMakeLists.txt exists. Use --force to overwrite.")
        #     return

        # 如果不是初次运行，询问是否重新扫描
        refresh_requested = self.add_manual_refresh_option()

        content = self.generate_cmake(chip_name, board_name)
        self.cmake_file.write_text(content, encoding='utf-8')

        # 生成idea.cfg文件
        self.generate_idea_cfg()

        # 生成VS Code配置文件
        self.generate_vscode_config()

        # 生成clangd配置文件
        self.generate_clangd_config()

        # 生成compile_flags.txt作为备用配置
        self.generate_compile_flags_txt()

        # 只在初次运行或手动刷新时保存配置文件
        if self.is_first_run or refresh_requested:
            self.config.save_config()
            print(f"已生成: CMakeLists.txt, {self.config.CONFIG_FILE}, idea.cfg, .vscode配置, .clangd配置, compile_flags.txt")
        else:
            print(f"已更新: CMakeLists.txt, idea.cfg, .vscode配置, .clangd配置, compile_flags.txt (使用现有配置)")

def main():
    parser = argparse.ArgumentParser(description='STM32 CMakeLists.txt生成器')
    parser.add_argument('--chip', default='STM32F103C8', help='芯片型号')
    parser.add_argument('--board', default='BLUEPILL', help='开发板')
    parser.add_argument('--force', action='store_true', help='强制覆盖')
    parser.add_argument('--dir', default='.', help='项目目录')

    args = parser.parse_args()
    generator = CMakeGenerator(args.dir)
    generator.generate(args.chip, args.board, args.force)

if __name__ == '__main__':
    main()



