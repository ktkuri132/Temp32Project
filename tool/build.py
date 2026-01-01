#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
STM32项目构建脚本 - 主入口
整合所有模块，负责项目配置、文件扫描、CMake生成等功能
"""

import sys
import argparse
from pathlib import Path

# 添加tool目录到Python路径
sys.path.insert(0, str(Path(__file__).parent / 'tool'))

from project_config import ProjectConfig
from source_scanner import SourceScanner
from cmake_generator import CMakeGenerator
from vscode_config_generator import VscodeConfigGenerator, AdditionalConfigGenerator
from chip_detector import ChipDetector
from config_generator import ConfigGenerator


class ProjectBuilder:
    """项目构建器"""

    def __init__(self, project_root="."):
        self.project_root = Path(project_root)

        # 初始化配置管理
        self.config = ProjectConfig(project_root)

        # 检查是否为初次运行
        self.is_first_run = not self.config.config_file.exists()

        # 初始化芯片检测器
        self.chip_detector = ChipDetector(project_root)

        # 初始化各个模块
        self.scanner = SourceScanner(project_root, self.config)
        self.cmake_gen = CMakeGenerator(project_root, self.config)
        self.vscode_gen = VscodeConfigGenerator(project_root, self.config, self.cmake_gen)
        self.additional_gen = AdditionalConfigGenerator(project_root, self.config)

    def get_project_name(self):
        """获取项目名称"""
        import re
        project_root_abs = self.project_root.resolve()
        project_name = project_root_abs.name

        if not project_name or project_name in ['.', '..']:
            project_name = "STM32_Project"

        # 处理特殊字符
        project_name = re.sub(r'[^a-zA-Z0-9_]', '_', project_name)
        return project_name

    def update_config_from_detection(self, chip_name, sources=None, include_dirs=None,
                                    subdirs_with_cmake=None, static_libraries=None):
        """根据检测结果更新配置（仅在初次运行时）"""
        if not self.is_first_run:
            return

        # 使用chip_detector获取芯片完整信息（使用已检测的chip_name）
        chip_info = self.chip_detector.get_chip_info(chip_name)

        # 更新芯片信息
        self.config.set('project.chip', chip_name)

        # 更新架构信息
        arch = chip_info.get('architecture', 'cortex-m3')
        self.config.set('project.architecture', arch)

        # 检测并更新浮点类型
        fpu_info = chip_info.get('float_description', '软件浮点')
        self.config.set('project.float_type', fpu_info)
        linker_script = self.scanner.find_linker_script(chip_name)
        self.config.set('linker.script', linker_script)

        # 使用chip_detector获取下载目标
        download_target = chip_info.get('download_target', 'stm32f1x')
        self.config.set('download.target', download_target)

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

    def save_bsp_device_config(self):
        """保存BSP和Device配置到project_config.json"""
        # 获取芯片信息
        chip_name = self.config.get('project.chip', 'STM32F407VE')
        chip_info = self.chip_detector.get_chip_info(chip_name)
        bsp_chip_dir = chip_info.get('bsp_dir', 'stm32f4')

        # 扫描BSP源文件
        bsp_sources, bsp_include_dirs = self.scanner.scan_bsp_sources(bsp_chip_dir)
        bsp_config = {
            'chip_dir': bsp_chip_dir,
            'sources': [str(src.relative_to(self.project_root / 'BSP').as_posix()) for src in bsp_sources],
            'include_dirs': [str(inc.relative_to(self.project_root).as_posix()) for inc in bsp_include_dirs]
        }
        self.config.set('bsp', bsp_config)

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

    def add_manual_refresh_option(self):
        """添加手动刷新选项"""
        if not self.is_first_run:
            refresh = input("\n是否重新扫描项目文件? (y/N): ").strip().lower()
            if refresh in ['y', 'yes']:
                print("重新扫描项目文件...")
                self.is_first_run = True
                return True
        return False

    def build(self, chip_name=None, board_name=None):
        """执行构建流程"""
        # 如果不是初次运行，询问是否重新扫描
        refresh_requested = self.add_manual_refresh_option()

        if self.is_first_run:
            # 初次运行：使用传入参数或默认值进行自动检测
            # 如果没有指定芯片名称，使用chip_detector自动检测
            if not chip_name:
                chip_info = self.chip_detector.get_chip_info()
                chip_name = chip_info.get('chip', 'STM32F407VE')
                arch = chip_info.get('architecture', 'cortex-m3')
            else:
                chip_info = self.chip_detector.get_chip_info(chip_name)
                arch = chip_info.get('architecture', 'cortex-m3')

            board_name = board_name or self.config.get('project.board')

            print("=== 初次运行，自动检测配置 ===")

            # 扫描文件系统
            sources, subdirs_with_cmake = self.scanner.scan_sources(arch)
            include_dirs = self.scanner.scan_include_dirs()
            subdir_includes = self.scanner.scan_subdir_includes(subdirs_with_cmake)
            all_include_dirs = include_dirs.union(subdir_includes)

            # 分析子目录并获取静态库
            static_libraries = []
            for subdir in subdirs_with_cmake:
                analysis = self.scanner.analyze_cmake(subdir / "CMakeLists.txt")
                if analysis['has_libraries']:
                    static_libraries.extend(analysis['static_libraries'])
                else:
                    static_libraries.append(subdir.name)

            # 更新配置（包含文件信息）
            self.update_config_from_detection(
                chip_name, sources, all_include_dirs, subdirs_with_cmake, static_libraries
            )

            # 保存BSP和Device配置
            self.save_bsp_device_config()
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
        # 从配置读取BSP信息
        bsp_config = self.config.get('bsp', {})
        if bsp_config:
            bsp_chip_dir = bsp_config.get('chip_dir', 'stm32f4')
            bsp_sources = [self.project_root / 'BSP' / src for src in bsp_config.get('sources', [])]
            bsp_include_dirs = [self.project_root / inc for inc in bsp_config.get('include_dirs', [])]

            if bsp_sources:
                print(f"使用配置: {len(bsp_sources)} 个BSP源文件")
                bsp_cmake_content = self.cmake_gen.generate_bsp_cmake(
                    bsp_chip_dir, bsp_sources, bsp_include_dirs
                )
                self.cmake_gen.write_bsp_cmake(bsp_cmake_content)
            else:
                print("警告: BSP配置为空")
        else:
            print("警告: 未找到BSP配置")

        # 从配置读取Device信息
        devices_config = self.config.get('devices', {})
        if devices_config:
            # 转换为扫描器格式
            devices = {}
            enabled_devices = []
            for device_name, device_cfg in devices_config.items():
                if device_cfg.get('enabled', True):
                    devices[device_name] = {
                        'sources': [self.project_root / 'Device' / src for src in device_cfg.get('sources', [])],
                        'include_dirs': [self.project_root / inc for inc in device_cfg.get('include_dirs', [])]
                    }
                    enabled_devices.append(device_name)

            if devices:
                total_sources = sum(len(d['sources']) for d in devices.values())
                print(f"使用配置: {len(devices)} 个设备 ({', '.join(enabled_devices)})，共 {total_sources} 个源文件")
                device_cmake_content = self.cmake_gen.generate_device_cmake(devices)
                self.cmake_gen.write_device_cmake(device_cmake_content)
            else:
                print("提示: 所有设备驱动均已禁用")
        else:
            print("警告: 未找到Device配置")

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

    args = parser.parse_args()

    builder = ProjectBuilder(args.dir)
    builder.build(args.chip, args.board)


if __name__ == '__main__':
    main()
