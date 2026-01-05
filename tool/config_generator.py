#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
config_generator.py - 设备配置文件生成器 v3.0
自动生成Device/config.h文件，管理设备驱动的条件编译宏

重构特性：
- 数据驱动：所有设备配置完全从 project_config.json 读取
- 易扩展：新增设备只需在 project_config.json 中添加配置
- 通信总线自动检测：根据设备的 protocol 属性自动生成总线配置
"""

import json
from typing import Dict, Any, List, Set
from pathlib import Path


# ============================================================================
#                          传输协议定义
# ============================================================================

# 协议字符串到宏定义的映射
# project_config.json 中使用的字符串值 -> 对应的 C 宏定义
PROTOCOL_MACROS = {
    "soft_i2c": "__SOFTI2C_",   # 软件 I2C
    "hard_i2c": "__HARDI2C_",   # 硬件 I2C
    "soft_spi": "__SOFTSPI_",   # 软件 SPI
    "hard_spi": "__HARDSPI_",   # 硬件 SPI
}

# 协议的描述信息
PROTOCOL_DESCRIPTIONS = {
    "soft_i2c": "软件模拟 I2C",
    "hard_i2c": "硬件 I2C",
    "soft_spi": "软件模拟 SPI",
    "hard_spi": "硬件 SPI",
}

# 协议对应的总线类型
PROTOCOL_BUS_TYPE = {
    "soft_i2c": "i2c",
    "hard_i2c": "i2c",
    "soft_spi": "spi",
    "hard_spi": "spi",
}


class ConfigGenerator:
    """设备配置文件生成器"""

    def __init__(self, project_root: Path):
        """
        初始化生成器

        Args:
            project_root: 项目根目录
        """
        self.project_root = Path(project_root)
        self.config_file = self.project_root / "tool" / "project_config.json"
        self.output_file = self.project_root / "Device" / "config.h"

    def load_config(self) -> Dict[str, Any]:
        """加载项目配置"""
        with open(self.config_file, 'r', encoding='utf-8') as f:
            return json.load(f)

    def get_enabled_devices(self) -> Dict[str, Any]:
        """获取启用的设备列表"""
        config = self.load_config()
        devices = config.get('devices', {})
        return {
            name: info for name, info in devices.items()
            if info.get('enabled', False) and name != '_root'
        }

    def detect_required_buses(self, enabled_devices: Dict[str, Any]) -> Set[str]:
        """
        检测需要的通信总线类型

        Args:
            enabled_devices: 启用的设备字典

        Returns:
            需要的总线类型集合 (如 {'i2c', 'spi'})
        """
        buses = set()
        for device_name, device_info in enabled_devices.items():
            # 从设备配置中获取协议，通过协议推断总线类型
            protocol = device_info.get('protocol', '')
            if protocol and protocol in PROTOCOL_BUS_TYPE:
                buses.add(PROTOCOL_BUS_TYPE[protocol])
        return buses

    def detect_required_protocols(self, enabled_devices: Dict[str, Any]) -> Set[str]:
        """
        检测需要的传输协议

        Args:
            enabled_devices: 启用的设备字典

        Returns:
            需要的协议集合 (如 {'soft_i2c', 'hard_spi'})
        """
        protocols = set()
        for device_name, device_info in enabled_devices.items():
            protocol = device_info.get('protocol', '')
            if protocol and protocol in PROTOCOL_MACROS:
                protocols.add(protocol)
        return protocols

    def generate_device_config_header(self) -> str:
        """
        生成Device/config.h头文件内容

        Returns:
            生成的头文件内容
        """
        enabled_devices = self.get_enabled_devices()
        required_buses = self.detect_required_buses(enabled_devices)
        required_protocols = self.detect_required_protocols(enabled_devices)

        # 生成头文件
        header = self._generate_header_comment()
        header += self._generate_header_guard_start()
        header += self._generate_includes()
        header += self._generate_bus_config(required_buses, required_protocols, enabled_devices)
        header += self._generate_device_sections(enabled_devices)
        header += self._generate_header_guard_end()

        return header

    def _generate_header_comment(self) -> str:
        """生成文件头注释"""
        return """/**
 * @file    config.h
 * @brief   设备驱动配置文件 (自动生成)
 * @details 本文件由构建系统自动生成，请勿手动编辑
 *          通过修改 tool/project_config.json 来配置设备驱动
 * @warning 此文件会在每次构建时重新生成，手动修改将会丢失
 */

"""

    def _generate_header_guard_start(self) -> str:
        """生成头文件保护宏开始"""
        return """#ifndef __CONFIG_H_
#define __CONFIG_H_

"""

    def _generate_includes(self) -> str:
        """生成通用包含文件"""
        return """#include <stdint.h>
#include <driver.h>

"""

    def _generate_bus_config(self, required_buses: Set[str], required_protocols: Set[str],
                               enabled_devices: Dict[str, Any]) -> str:
        """
        生成通信总线配置

        Args:
            required_buses: 需要的总线类型集合
            required_protocols: 需要的协议集合
            enabled_devices: 启用的设备字典

        Returns:
            总线配置字符串
        """
        output = """/*============================ 通信总线配置 ============================*/

"""
        # 检测 I2C 使用的协议
        i2c_protocols = {p for p in required_protocols if PROTOCOL_BUS_TYPE.get(p) == 'i2c'}
        # 检测 SPI 使用的协议
        spi_protocols = {p for p in required_protocols if PROTOCOL_BUS_TYPE.get(p) == 'spi'}

        # I2C 配置
        if 'i2c' in required_buses:
            # 确定使用哪种 I2C 协议
            use_soft_i2c = 'soft_i2c' in i2c_protocols
            use_hard_i2c = 'hard_i2c' in i2c_protocols

            # 如果没有指定协议，默认使用软件 I2C
            if not use_soft_i2c and not use_hard_i2c:
                use_soft_i2c = True

            output += "/* I2C通信总线选择 */\n"
            if use_soft_i2c:
                output += f"#define {PROTOCOL_MACROS['soft_i2c']}\n"
            else:
                output += f"// #define {PROTOCOL_MACROS['soft_i2c']}\n"

            if use_hard_i2c:
                output += f"#define {PROTOCOL_MACROS['hard_i2c']}\n"
            else:
                output += f"// #define {PROTOCOL_MACROS['hard_i2c']}\n"

            output += "\n"
            output += f"#ifdef {PROTOCOL_MACROS['soft_i2c']}\n"
            output += "#include <i2c/df_iic.h>\n"
            output += "extern df_iic_t i2c1_bus;   /* 声明外部I2C总线 */\n"
            output += "#define i2c_Dev (*i2c1_bus.soft_iic)\n"
            output += "#endif\n\n"

            output += f"#ifdef {PROTOCOL_MACROS['hard_i2c']}\n"
            output += "/* TODO: 硬件I2C配置 */\n"
            output += "#endif\n\n"

        # SPI 配置
        if 'spi' in required_buses:
            # 确定使用哪种 SPI 协议
            use_soft_spi = 'soft_spi' in spi_protocols
            use_hard_spi = 'hard_spi' in spi_protocols

            # 如果没有指定协议，默认使用软件 SPI
            if not use_soft_spi and not use_hard_spi:
                use_soft_spi = True

            output += "/* SPI通信总线选择 */\n"
            if use_soft_spi:
                output += f"#define {PROTOCOL_MACROS['soft_spi']}\n"
            else:
                output += f"// #define {PROTOCOL_MACROS['soft_spi']}\n"

            if use_hard_spi:
                output += f"#define {PROTOCOL_MACROS['hard_spi']}\n"
            else:
                output += f"// #define {PROTOCOL_MACROS['hard_spi']}\n"

            output += "\n"
            output += f"#ifdef {PROTOCOL_MACROS['soft_spi']}\n"
            output += "#include <spi/df_spi.h>\n"
            output += "extern df_spi_t spi1_bus;   /* 声明外部SPI总线 */\n"
            output += "#define spi_Dev (*spi1_bus.soft_spi)\n"
            output += "#endif\n\n"

            output += f"#ifdef {PROTOCOL_MACROS['hard_spi']}\n"
            output += "/* TODO: 硬件SPI配置 */\n"
            output += "#endif\n\n"

        return output

    def _generate_device_sections(self, enabled_devices: Dict[str, Any]) -> str:
        """
        生成所有设备的配置区段

        Args:
            enabled_devices: 启用的设备字典

        Returns:
            设备配置字符串
        """
        if not enabled_devices:
            return """/*============================ 设备驱动配置 ============================*/

/* 无启用的设备 */

"""

        output = """/*============================ 设备驱动配置 ============================*/

"""
        for device_name, device_info in enabled_devices.items():
            output += self._generate_single_device_config(device_name, device_info)

        return output

    def _generate_single_device_config(self, device_name: str, device_info: Dict[str, Any]) -> str:
        """
        生成单个设备的配置区段

        Args:
            device_name: 设备名称
            device_info: 设备信息（来自 project_config.json）

        Returns:
            设备配置字符串
        """
        # 从 device_info 中获取设备元数据（所有配置都在 project_config.json 中）
        description = device_info.get('description', f'{device_name.upper()} 设备')
        protocol = device_info.get('protocol', '')
        bus = PROTOCOL_BUS_TYPE.get(protocol, 'unknown')
        default_addr = device_info.get('default_addr', '')

        # 生成宏名称
        macro_name = f"USE_DEVICE_{device_name.upper()}"

        # 生成配置块
        output = f"/* {description} ({bus.upper()}) */\n"
        output += f"#ifdef {macro_name}\n"

        # 如果有默认地址，生成地址宏
        if default_addr:
            addr_macro = f"{device_name.upper()}_I2C_ADDR"
            output += f"#ifndef {addr_macro}\n"
            output += f"#define {addr_macro} {default_addr}\n"
            output += f"#endif\n"

        # 从 device_info 中读取额外配置（如果有）
        extra_config = device_info.get('config', {})
        for key, value in extra_config.items():
            macro = f"{device_name.upper()}_{key.upper()}"
            if isinstance(value, bool):
                if value:
                    output += f"#define {macro}\n"
            elif isinstance(value, str):
                output += f"#define {macro} {value}\n"
            else:
                output += f"#define {macro} {value}\n"

        output += f"#endif /* {macro_name} */\n\n"

        return output

    def _generate_header_guard_end(self) -> str:
        """生成头文件保护宏结束"""
        return """#endif /* __CONFIG_H_ */
"""

    def write_config_header(self) -> None:
        """写入配置头文件"""
        content = self.generate_device_config_header()

        # 确保目录存在
        self.output_file.parent.mkdir(parents=True, exist_ok=True)

        # 写入文件
        with open(self.output_file, 'w', encoding='utf-8', newline='\n') as f:
            f.write(content)

        print(f"✓ 生成设备配置文件: Device/config.h")

    def get_enabled_devices_info(self) -> str:
        """
        获取启用设备的信息摘要

        Returns:
            设备信息字符串
        """
        config = self.load_config()
        devices = config.get('devices', {})

        enabled = []
        disabled = []
        for name, info in devices.items():
            if name == '_root':
                continue
            if info.get('enabled', False):
                protocol = info.get('protocol', 'auto')
                bus = PROTOCOL_BUS_TYPE.get(protocol, '?')
                protocol_desc = PROTOCOL_DESCRIPTIONS.get(protocol, protocol)
                enabled.append(f"{name}({bus}/{protocol_desc})")
            else:
                disabled.append(name)

        info = f"  启用设备 ({len(enabled)}): {', '.join(enabled) if enabled else '无'}\n"

        # 检测需要的总线和协议
        enabled_devices = self.get_enabled_devices()
        buses = self.detect_required_buses(enabled_devices)
        protocols = self.detect_required_protocols(enabled_devices)

        if buses:
            info += f"  所需总线: {', '.join(sorted(buses))}\n"

        if protocols:
            protocol_macros = [PROTOCOL_MACROS.get(p, p) for p in sorted(protocols)]
            info += f"  使用协议: {', '.join(protocol_macros)}\n"

        if disabled:
            info += f"  禁用设备 ({len(disabled)}): {', '.join(disabled)}\n"

        return info

    def list_available_devices(self) -> str:
        """
        列出所有可用设备（从 project_config.json 读取）

        Returns:
            设备列表字符串
        """
        config = self.load_config()
        devices = config.get('devices', {})

        output = "可用设备列表:\n"
        output += "-" * 50 + "\n"

        # 按总线类型分组
        by_bus: Dict[str, List[str]] = {}
        for name, info in devices.items():
            if name == '_root':
                continue
            protocol = info.get('protocol', '')
            bus = PROTOCOL_BUS_TYPE.get(protocol, 'other')
            description = info.get('description', '')
            status = "✓" if info.get('enabled', False) else "✗"
            if bus not in by_bus:
                by_bus[bus] = []
            by_bus[bus].append(f"  [{status}] {name}: {description}")

        for bus in sorted(by_bus.keys()):
            output += f"\n[{bus.upper()} 设备]\n"
            output += "\n".join(by_bus[bus]) + "\n"

        return output


def main():
    """主函数 - 用于测试"""
    import sys
    from pathlib import Path

    # 获取项目根目录（向上两级）
    script_dir = Path(__file__).parent
    project_root = script_dir.parent

    generator = ConfigGenerator(project_root)

    print("=" * 60)
    print("Device/config.h 生成器 v3.0")
    print("=" * 60)

    # 显示可用设备（可选）
    if '--list' in sys.argv:
        print(generator.list_available_devices())
        return

    print(generator.get_enabled_devices_info())

    generator.write_config_header()

    print("=" * 60)


if __name__ == '__main__':
    main()
