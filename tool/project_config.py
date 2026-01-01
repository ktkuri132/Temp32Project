#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
项目配置管理模块
负责加载、保存和管理项目配置文件(project_config.json)
"""

import json
from pathlib import Path


class ProjectConfig:
    """项目配置管理类"""

    CONFIG_FILE = "tool/project_config.json"

    DEFAULT_CONFIG = {
        "project": {
            "name": "",  # 自动从文件夹名获取
            "chip": "",
            "board": "",
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
            ""
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
        # 确保 tool 目录存在
        tool_dir = self.project_root / "tool"
        tool_dir.mkdir(exist_ok=True)
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
