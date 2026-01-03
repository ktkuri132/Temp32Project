#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
config_generator.py - 设备配置文件生成器
自动生成Device/config.h文件，管理设备驱动的条件编译宏
"""

import json
from typing import Dict, Any
from pathlib import Path


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

    def generate_device_config_header(self) -> str:
        """
        生成Device/config.h头文件内容

        Returns:
            生成的头文件内容
        """
        config = self.load_config()
        devices = config.get('devices', {})

        # 过滤出启用的设备
        enabled_devices = {
            name: info for name, info in devices.items()
            if info.get('enabled', False)
        }

        # 生成头文件
        header = self._generate_header_comment()
        header += self._generate_header_guard_start()
        header += self._generate_includes()
        header += self._generate_device_macros(enabled_devices)
        header += self._generate_device_configs(enabled_devices)
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

    def _generate_device_macros(self, enabled_devices: Dict[str, Any]) -> str:
        """
        生成设备宏定义（已废弃，不再生成旧的设备名宏）

        Args:
            enabled_devices: 启用的设备字典

        Returns:
            空字符串（旧宏已移除，统一使用USE_DEVICE_XXX）
        """
        # 不再生成旧的设备名宏（如BMP280, SH1106等）
        # 统一使用CMakeLists.txt中定义的USE_DEVICE_XXX宏
        return ""

    def _generate_device_configs(self, enabled_devices: Dict[str, Any]) -> str:
        """
        生成设备特定配置

        Args:
            enabled_devices: 启用的设备字典

        Returns:
            配置字符串
        """
        output = """/*============================ 设备驱动配置 ============================*/

"""

        # 通信总线配置
        output += self._generate_i2c_config(enabled_devices)

        # 各设备特定配置
        output += self._generate_sh1106_config(enabled_devices)
        output += self._generate_ssd1306_config(enabled_devices)
        output += self._generate_mpu6050_config(enabled_devices)
        output += self._generate_hmc588_config(enabled_devices)
        output += self._generate_bmp280_config(enabled_devices)

        return output

    def _generate_i2c_config(self, enabled_devices: Dict[str, Any]) -> str:
        """生成I2C通信配置"""
        # 检查是否有设备需要I2C
        i2c_devices = ['bmp280', 'hmc588', 'mpu6050', 'sh1106', 'ssd1306']
        needs_i2c = any(dev in enabled_devices for dev in i2c_devices)

        if not needs_i2c:
            return ""

        return """/* I2C通信总线选择 */
#define __SOFTI2C_

#ifdef __SOFTI2C_
#include <i2c/df_iic.h>
extern df_iic_t i2c1_bus;   /* 声明外部I2C总线 */
#define i2c_Dev (*i2c1_bus.soft_iic)
#endif

"""

    def _generate_sh1106_config(self, enabled_devices: Dict[str, Any]) -> str:
        """生成SH1106配置"""
        if 'sh1106' not in enabled_devices:
            return ""

        return """/* SH1106 OLED显示屏配置 */
#ifdef USE_DEVICE_SH1106
#endif /* USE_DEVICE_SH1106 */

"""

    def _generate_ssd1306_config(self, enabled_devices: Dict[str, Any]) -> str:
        """生成SSD1306配置"""
        if 'ssd1306' not in enabled_devices:
            return ""

        return """/* SSD1306 OLED显示屏配置 */
#ifdef USE_DEVICE_SSD1306
#endif /* USE_DEVICE_SSD1306 */

"""

    def _generate_mpu6050_config(self, enabled_devices: Dict[str, Any]) -> str:
        """生成MPU6050配置"""
        if 'mpu6050' not in enabled_devices:
            return ""

        return """/* MPU6050 六轴IMU配置 */
#ifdef USE_DEVICE_MPU6050
#endif /* USE_DEVICE_MPU6050 */

"""

    def _generate_hmc588_config(self, enabled_devices: Dict[str, Any]) -> str:
        """生成HMC5883L配置"""
        if 'hmc588' not in enabled_devices:
            return ""

        return """/* HMC5883L 三轴磁力计配置 */
#ifdef USE_DEVICE_HMC588
#endif /* USE_DEVICE_HMC588 */

"""

    def _generate_bmp280_config(self, enabled_devices: Dict[str, Any]) -> str:
        """生成BMP280配置"""
        if 'bmp280' not in enabled_devices:
            return ""

        return """/* BMP280 气压温度传感器配置 */
#ifdef USE_DEVICE_BMP280
#endif /* USE_DEVICE_BMP280 */

"""
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

        enabled = [name for name, info in devices.items() if info.get('enabled', False)]
        disabled = [name for name, info in devices.items() if not info.get('enabled', False)]

        info = f"  启用设备 ({len(enabled)}): {', '.join(enabled) if enabled else '无'}\n"
        if disabled:
            info += f"  禁用设备 ({len(disabled)}): {', '.join(disabled)}\n"

        return info


def main():
    """主函数 - 用于测试"""
    import sys
    from pathlib import Path

    # 获取项目根目录（向上两级）
    script_dir = Path(__file__).parent
    project_root = script_dir.parent

    generator = ConfigGenerator(project_root)

    print("=" * 60)
    print("Device/config.h 生成器")
    print("=" * 60)
    print(generator.get_enabled_devices_info())

    generator.write_config_header()

    print("=" * 60)


if __name__ == '__main__':
    main()
