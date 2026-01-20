#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
STM32项目构建脚本 - 主入口
整合所有模块，负责项目配置、文件扫描、CMake生成等功能
"""

import argparse
from pathlib import Path

from project_config import ProjectConfig
from source_scanner import SourceScanner
from chip_detector import ChipDetector
from cmake_generator import CMakeGenerator
from vscode_config_generator import VscodeConfigGenerator, AdditionalConfigGenerator
from config_generator import ConfigGenerator


class ProjectBuilder:
    """项目构建器"""

    def __init__(self, project_root="."):
        self.project_root = Path(project_root)
        self.config = ProjectConfig(project_root)
        self.is_first_run = not self.config.config_file.exists()
        self.chip_detector = ChipDetector(project_root)
        self.scanner = SourceScanner(project_root, self.config)
        self.cmake_gen = CMakeGenerator(project_root, self.config)
        self.vscode_gen = VscodeConfigGenerator(project_root, self.config, self.cmake_gen)
        self.additional_gen = AdditionalConfigGenerator(project_root, self.config)

    def update_config_from_detection(self, chip_info, sources=None, include_dirs=None,
                                    subdirs_with_cmake=None, static_libraries=None):
        """根据检测结果更新配置（仅在初次运行时）"""
        if not self.is_first_run:
            return

        # 更新芯片和架构信息
        self.config.set('project.chip', chip_info.get('chip', ''))
        self.config.set('project.architecture', chip_info.get('architecture', 'cortex-m3'))
        self.config.set('project.float_type', chip_info.get('float_description', '软件浮点'))
        self.config.set('download.target', chip_info.get('download_target', 'stm32f1x'))

        # 查找并设置链接脚本
        linker_script = self.scanner.find_linker_script(chip_info.get('chip', ''))
        self.config.set('linker.script', linker_script)

        # 保存文件信息
        if sources is not None:
            self.config.set('files.sources', self._to_relative_paths(sources))
        if include_dirs is not None:
            self.config.set('files.include_dirs', self._to_relative_paths(include_dirs))
        if subdirs_with_cmake is not None:
            self.config.set('files.subdirs', self._to_relative_paths(subdirs_with_cmake))
        if static_libraries is not None:
            self.config.set('files.static_libraries', static_libraries)

    def _to_relative_paths(self, paths):
        """将路径列表转换为相对路径字符串列表"""
        return [str(p.relative_to(self.project_root).as_posix()) for p in paths]

    def _collect_static_libraries(self, subdirs_with_cmake):
        """分析子目录并收集静态库列表"""
        static_libraries = []
        for subdir in subdirs_with_cmake:
            analysis = self.scanner.analyze_cmake(subdir / "CMakeLists.txt")
            if analysis['has_libraries']:
                static_libraries.extend(analysis['static_libraries'])
            else:
                static_libraries.append(subdir.name)
        return static_libraries

    def _load_files_from_config(self):
        """从配置文件读取文件信息"""
        sources = [self.project_root / p for p in self.config.get('files.sources', [])]
        include_dirs = {self.project_root / p for p in self.config.get('files.include_dirs', [])}
        subdirs = [self.project_root / p for p in self.config.get('files.subdirs', [])]
        static_libs = self.config.get('files.static_libraries', [])
        return sources, include_dirs, subdirs, static_libs

    def save_bsp_device_config(self):
        """保存BSP和Device配置到project_config.json"""
        # 获取芯片信息
        chip_name = self.config.get('project.chip', '')
        chip_info = self.chip_detector.get_chip_info(chip_name)
        bsp_chip_dir = chip_info.get('bsp_dir') or ''
        chip_package = chip_info.get('chip_package') or ''
        chip_model_dir = chip_info.get('chip_model_dir') or ''

        # 扫描BSP源文件
        if bsp_chip_dir:
            bsp_sources, bsp_include_dirs = self.scanner.scan_bsp_sources(
                bsp_chip_dir, chip_package, chip_model_dir
            )
            if chip_model_dir:
                print(f"使用芯片型号目录: {chip_model_dir}")
            if chip_package:
                print(f"使用芯片封装目录: {chip_package}")

            bsp_config = {
                'chip_dir': bsp_chip_dir,
                'chip_model_dir': chip_model_dir,
                'chip_package': chip_package,
                'sources': [str(src.relative_to(self.project_root / 'BSP').as_posix()) for src in bsp_sources],
                'include_dirs': [str(inc.relative_to(self.project_root).as_posix()) for inc in bsp_include_dirs]
            }
            self.config.set('bsp', bsp_config)
        else:
            print("警告: 未检测到有效的BSP芯片目录")

        # 扫描Device源文件
        devices = self.scanner.scan_device_sources()
        devices_config = {}
        for device_name, device_info in devices.items():
            devices_config[device_name] = {
                'enabled': True,  # 默认启用
                'sources': [str(src.relative_to(self.project_root / 'Device').as_posix()) for src in device_info['sources']],
                'include_dirs': [str(inc.relative_to(self.project_root).as_posix()) for inc in device_info['include_dirs']]
            }
        self.config.set('devices', devices_config)

    def add_manual_refresh_option(self, interactive=True):
        """添加手动刷新选项"""
        if not self.is_first_run and interactive:
            refresh = input("\n是否重新扫描项目文件? (y/N): ").strip().lower()
            if refresh in ['y', 'yes']:
                print("重新扫描项目文件...")
                self.is_first_run = True
                return True
        return False

    def build(self, chip_name=None, board_name=None, interactive=True):
        """执行构建流程

        Args:
            chip_name: 芯片型号
            board_name: 开发板名称
            interactive: 是否为交互模式（默认True）
        """
        # 如果不是初次运行且为交互模式，询问是否重新扫描
        refresh_requested = self.add_manual_refresh_option(interactive)

        # 如果是重新扫描，保留现有配置中的芯片名称
        if refresh_requested and not chip_name:
            existing_chip = self.config.get('project.chip', '')
            if existing_chip and len(existing_chip) > 6:  # 确保是完整芯片名如STM32F407VGT6
                chip_name = existing_chip
                print(f"使用现有配置的芯片型号: {chip_name}")

        if self.is_first_run:
            # 初次运行：使用传入参数或默认值进行自动检测
            chip_info = self.chip_detector.get_chip_info(chip_name) if chip_name else self.chip_detector.get_chip_info()
            chip_name = chip_info.get('chip', '')
            arch = chip_info.get('architecture', '')
            board_name = board_name or self.config.get('project.board')

            print("=== 初次运行，自动检测配置 ===")

            # 扫描文件系统
            sources, subdirs_with_cmake = self.scanner.scan_sources(arch)
            include_dirs = self.scanner.scan_include_dirs()
            subdir_includes = self.scanner.scan_subdir_includes(subdirs_with_cmake)
            all_include_dirs = include_dirs.union(subdir_includes)

            # 分析子目录并获取静态库
            static_libraries = self._collect_static_libraries(subdirs_with_cmake)

            # 更新配置（包含文件信息）
            self.update_config_from_detection(
                chip_info, sources, all_include_dirs, subdirs_with_cmake, static_libraries
            )

            # 保存BSP和Device配置
            self.save_bsp_device_config()
        else:
            # 配置文件已存在：从配置文件读取
            sources, all_include_dirs, subdirs_with_cmake, static_libraries = self._load_files_from_config()
            print("=== 使用现有配置文件 ===")
            print(f"从配置读取: {len(sources)} 个源文件, {len(all_include_dirs)} 个头文件目录, {len(static_libraries)} 个静态库")

        # 生成CMakeLists.txt
        cmake_content = self.cmake_gen.generate_cmake(
            sources, all_include_dirs, subdirs_with_cmake, static_libraries
        )
        self.cmake_gen.write_cmake(cmake_content)

        # 生成BSP和Device的CMakeLists.txt
        self.generate_bsp_device_cmake()

        # 生成Device/config.h
        self.generate_device_config_header()

        # 生成其他配置文件
        self.additional_gen.generate_idea_cfg()
        self.additional_gen.generate_gitignore()
        self.vscode_gen.generate_vscode_config()

        # 只在初次运行或手动刷新时保存配置文件
        if self.is_first_run or refresh_requested:
            self.config.save_config()
            print(f"已生成: CMakeLists.txt, {self.config.CONFIG_FILE}, idea.cfg, .vscode配置")
        else:
            print(f"已更新: CMakeLists.txt, idea.cfg, .vscode配置 (使用现有配置)")

    def generate_bsp_device_cmake(self):
        """生成BSP和Device的CMakeLists.txt（从配置读取）"""
        chip_name = self.config.get('project.chip', '')
        chip_info = self.chip_detector.get_chip_info(chip_name)
        chip_package = chip_info.get('chip_package', '')

        # 生成BSP CMakeLists.txt
        self._generate_bsp_cmake(chip_package)

        # 生成Device CMakeLists.txt
        self._generate_device_cmake()

    def _generate_bsp_cmake(self, chip_package):
        """生成BSP的CMakeLists.txt"""
        bsp_config = self.config.get('bsp', {})
        if not bsp_config:
            print("警告: 未找到BSP配置")
            return

        bsp_chip_dir = bsp_config.get('chip_dir', '')
        bsp_sources = [self.project_root / 'BSP' / src for src in bsp_config.get('sources', [])]
        bsp_include_dirs = [self.project_root / inc for inc in bsp_config.get('include_dirs', [])]

        if not bsp_sources:
            print("警告: BSP配置为空")
            return

        print(f"使用配置: {len(bsp_sources)} 个BSP源文件" + (f" (封装: {chip_package})" if chip_package else ""))
        bsp_cmake_content = self.cmake_gen.generate_bsp_cmake(
            bsp_chip_dir, bsp_sources, bsp_include_dirs, chip_package
        )
        self.cmake_gen.write_bsp_cmake(bsp_cmake_content)

    def _generate_device_cmake(self):
        """生成Device的CMakeLists.txt"""
        devices_config = self.config.get('devices', {})
        if not devices_config:
            print("警告: 未找到Device配置")
            return

        # 收集启用的设备
        devices = {}
        enabled_names = []
        for name, cfg in devices_config.items():
            if cfg.get('enabled', True):
                devices[name] = {
                    'sources': [self.project_root / 'Device' / src for src in cfg.get('sources', [])],
                    'include_dirs': [self.project_root / inc for inc in cfg.get('include_dirs', [])]
                }
                enabled_names.append(name)

        if not devices:
            print("提示: 所有设备驱动均已禁用")
            return

        total_sources = sum(len(d['sources']) for d in devices.values())
        print(f"使用配置: {len(devices)} 个设备 ({', '.join(enabled_names)})，共 {total_sources} 个源文件")
        device_cmake_content = self.cmake_gen.generate_device_cmake(devices)
        self.cmake_gen.write_device_cmake(device_cmake_content)

    def generate_device_config_header(self):
        """生成Device/config.h配置文件"""
        try:
            config_gen = ConfigGenerator(self.project_root)
            config_gen.write_config_header()
        except Exception as e:
            print(f"警告: 生成Device/config.h失败: {e}")

def main():
    parser = argparse.ArgumentParser(description='STM32 CMakeLists.txt生成器')
    parser.add_argument('--chip', default='', help='芯片型号')
    parser.add_argument('--board', default='', help='开发板')
    parser.add_argument('--force', action='store_true', help='强制覆盖')
    parser.add_argument('--dir', default='.', help='项目目录')
    parser.add_argument('--no-interactive', '-n', action='store_true',
                        dest='no_interactive', help='非交互模式（跳过所有提示）')

    args = parser.parse_args()

    builder = ProjectBuilder(args.dir)
    builder.build(args.chip, args.board, interactive=not args.no_interactive)


if __name__ == '__main__':
    main()
