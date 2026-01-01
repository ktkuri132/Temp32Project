#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
芯片检测模块
负责自动检测项目使用的芯片型号和架构
"""

import re
from pathlib import Path


class ChipDetector:
    """芯片检测器"""

    def __init__(self, project_root):
        self.project_root = Path(project_root)

        # 芯片架构映射
        self.chip_arch_map = {
            'STM32F0': 'cortex-m0',
            'STM32F1': 'cortex-m3',
            'STM32F103': 'cortex-m3',
            'STM32F2': 'cortex-m3',
            'STM32F3': 'cortex-m4',
            'STM32F4': 'cortex-m4',
            'STM32F7': 'cortex-m7',
            'STM32G0': 'cortex-m0plus',
            'STM32G4': 'cortex-m4',
            'STM32H7': 'cortex-m7',
            'STM32L0': 'cortex-m0plus',
            'STM32L1': 'cortex-m3',
            'STM32L4': 'cortex-m4',
            'STM32L5': 'cortex-m33',
            'STM32U5': 'cortex-m33',
            'STM32WB': 'cortex-m4',
            'STM32WL': 'cortex-m4'
        }

        # FPU配置
        self.fpu_config = {
            'cortex-m0': {'fpu': 'soft', 'float_abi': 'soft', 'description': '软件浮点'},
            'cortex-m0plus': {'fpu': 'soft', 'float_abi': 'soft', 'description': '软件浮点'},
            'cortex-m3': {'fpu': 'soft', 'float_abi': 'soft', 'description': '软件浮点'},
            'cortex-m4': {'fpu': 'fpv4-sp-d16', 'float_abi': 'hard', 'description': '硬件浮点 (FPU: fpv4-sp-d16)'},
            'cortex-m7': {'fpu': 'fpv5-d16', 'float_abi': 'hard', 'description': '硬件浮点 (FPU: fpv5-d16)'},
            'cortex-m33': {'fpu': 'fpv5-sp-d16', 'float_abi': 'hard', 'description': '硬件浮点 (FPU: fpv5-sp-d16)'}
        }

        # 下载目标映射
        self.download_target_map = {
            'STM32F0': 'stm32f0x', 'STM32F1': 'stm32f1x', 'STM32F2': 'stm32f2x',
            'STM32F3': 'stm32f3x', 'STM32F4': 'stm32f4x', 'STM32F7': 'stm32f7x',
            'STM32G0': 'stm32g0x', 'STM32G4': 'stm32g4x', 'STM32H7': 'stm32h7x',
            'STM32L0': 'stm32l0x', 'STM32L1': 'stm32l1x', 'STM32L4': 'stm32l4x',
            'STM32L5': 'stm32l5x', 'STM32U5': 'stm32u5x', 'STM32WB': 'stm32wbx',
            'STM32WL': 'stm32wlx'
        }

    def get_bsp_chip_dir(self, chip_name):
        """根据芯片名称获取对应的BSP目录名（如 stm32f4）"""
        chip_upper = chip_name.upper()
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

    def detect_chip_from_bsp(self):
        """从BSP目录结构检测芯片型号"""
        bsp_dirs = self.get_all_bsp_chip_dirs()

        # 常见芯片系列优先级
        priority_series = ['stm32f4', 'stm32f1', 'stm32f7', 'stm32h7', 'stm32l4']

        for series in priority_series:
            if series in bsp_dirs:
                # 将目录名转换为芯片名
                match = re.match(r'stm32([a-z])(\d)', series)
                if match:
                    letter = match.group(1).upper()
                    number = match.group(2)
                    return f"STM32{letter}{number}"

        # 如果没有找到优先级芯片，返回第一个
        if bsp_dirs:
            first_dir = sorted(bsp_dirs)[0]
            match = re.match(r'stm32([a-z])(\d)', first_dir)
            if match:
                letter = match.group(1).upper()
                number = match.group(2)
                return f"STM32{letter}{number}"

        return None

    def detect_chip_from_defines(self):
        """从代码定义中检测芯片型号"""
        # 常见的定义位置
        search_files = [
            'app/main.c',
            'app/main.h',
            'Device/config.h',
            'BSP/*/CORE/system_*.c'
        ]

        chip_patterns = [
            r'STM32F\d{3}[A-Z]{2}',
            r'STM32[FGLHUW][0-9][0-9A-Z]{2,4}'
        ]

        for file_pattern in search_files:
            for file_path in self.project_root.rglob(file_pattern):
                if file_path.is_file():
                    try:
                        with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
                            content = f.read()
                            for pattern in chip_patterns:
                                matches = re.findall(pattern, content, re.IGNORECASE)
                                if matches:
                                    return matches[0].upper()
                    except Exception:
                        continue

        return None

    def detect_chip(self):
        """检测芯片型号（综合多种方法）"""
        # 方法1：从BSP目录结构检测
        chip = self.detect_chip_from_bsp()
        if chip:
            return chip

        # 方法2：从代码定义检测
        chip = self.detect_chip_from_defines()
        if chip:
            return chip

        # 默认返回
        return "STM32F407VE"

    def detect_chip_arch(self, chip_name):
        """检测芯片架构"""
        chip_upper = chip_name.upper()
        for chip_prefix, arch in self.chip_arch_map.items():
            if chip_upper.startswith(chip_prefix):
                return arch
        return 'cortex-m3'  # 默认

    def get_fpu_info(self, arch):
        """获取FPU配置信息"""
        return self.fpu_config.get(arch, self.fpu_config['cortex-m3'])

    def get_download_target(self, chip_name):
        """根据芯片名称获取下载目标"""
        chip_upper = chip_name.upper()
        for prefix, target in self.download_target_map.items():
            if chip_upper.startswith(prefix):
                return target
        return 'stm32f1x'  # 默认

    def get_chip_info(self, chip_name=None):
        """获取完整的芯片信息"""
        if chip_name is None:
            chip_name = self.detect_chip()

        arch = self.detect_chip_arch(chip_name)
        fpu_info = self.get_fpu_info(arch)
        download_target = self.get_download_target(chip_name)
        bsp_dir = self.get_bsp_chip_dir(chip_name)

        return {
            'chip': chip_name,
            'architecture': arch,
            'fpu': fpu_info['fpu'],
            'float_abi': fpu_info['float_abi'],
            'float_description': fpu_info['description'],
            'download_target': download_target,
            'bsp_dir': bsp_dir
        }


if __name__ == '__main__':
    # 测试代码
    import sys

    project_root = sys.argv[1] if len(sys.argv) > 1 else '.'
    detector = ChipDetector(project_root)

    chip_info = detector.get_chip_info()
    print("检测到的芯片信息:")
    for key, value in chip_info.items():
        print(f"  {key}: {value}")
