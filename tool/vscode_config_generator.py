#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
VS Code和其他配置文件生成模块
负责生成.vscode配置、.clangd配置、idea.cfg等文件
"""

import os
import re
import json
import glob
import platform
from pathlib import Path


class VscodeConfigGenerator:
    """VS Code配置文件生成器"""

    def __init__(self, project_root, config, cmake_generator):
        self.project_root = Path(project_root)
        self.config = config
        self.cmake_generator = cmake_generator

    def generate_vscode_config(self):
        """生成VS Code配置文件"""
        vscode_dir = self.project_root / '.vscode'
        vscode_dir.mkdir(exist_ok=True)

        self.generate_c_cpp_properties(vscode_dir)
        self.generate_launch_json(vscode_dir)
        self.generate_tasks_json(vscode_dir)
        self.generate_settings_json(vscode_dir)

        print("已生成 VS Code 配置文件")

    def generate_c_cpp_properties(self, vscode_dir):
        """生成c_cpp_properties.json"""
        toolchain_info = self.cmake_generator.read_cmake_cache()

        include_paths = self.config.get('files.include_dirs', [])
        absolute_includes = [f"${{workspaceFolder}}/{path}" for path in include_paths]

        absolute_includes.extend([
            "${workspaceFolder}/**",
            "C:/Program Files (x86)/Arm GNU Toolchain arm-none-eabi/*/arm-none-eabi/include/**",
            "C:/Program Files (x86)/Arm GNU Toolchain arm-none-eabi/*/lib/gcc/arm-none-eabi/*/include/**"
        ])

        defines = self.config.get('defines', [])
        arch = self.config.get('project.architecture', '')

        arch_defines = []
        if 'cortex-m0' in arch:
            arch_defines.extend(['__CORTEX_M=0', 'ARM_MATH_CM0'])
        elif 'cortex-m3' in arch:
            arch_defines.extend(['__CORTEX_M=3', 'ARM_MATH_CM3'])
        elif 'cortex-m4' in arch:
            arch_defines.extend(['__CORTEX_M=4', 'ARM_MATH_CM4'])
        elif 'cortex-m7' in arch:
            arch_defines.extend(['__CORTEX_M=7', 'ARM_MATH_CM7'])

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
                        self.cmake_generator.get_fpu_flags(arch),
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
        project_name = self.config.get('project.name', '')
        interface = self.config.get('download.interface', '')
        target = self.config.get('download.target', '')
        speed = self.config.get('download.speed', '4000')

        current_platform = platform.system().lower()

        if interface == 'jlink':
            server_type = 'jlink'
            device = self.config.get('project.chip', '')
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

        if current_platform != "windows":
            launch_config["configurations"][0]["gdbPath"] = "gdb-multiarch"

        if server_type == 'openocd':
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
        else:
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
        current_platform = platform.system().lower()
        command = "python3" if current_platform != "windows" else "python"

        tasks_config = {
            "version": "2.0.0",
            "tasks": [
                {
                    "label": "$(zap)烧录",
                    "type": "shell",
                    "command": command,
                    "args": [
                        "${workspaceFolder}/tool/download.py"
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
        toolchain_info = self.cmake_generator.read_cmake_cache()
        current_platform = platform.system().lower()

        if toolchain_info["toolchain_path"]:
            toolchain_path = toolchain_info["toolchain_path"]
            if current_platform == "windows":
                gdb_path = str(Path(toolchain_path) / "arm-none-eabi-gdb.exe")
            else:
                gdb_path = "gdb-multiarch"
        else:
            if current_platform == "windows":
                toolchain_path = "C:/Program Files (x86)/Arm GNU Toolchain arm-none-eabi/13.2 Rel1/bin"
                gdb_path = "C:/Program Files (x86)/Arm GNU Toolchain arm-none-eabi/13.2 Rel1/bin/arm-none-eabi-gdb.exe"
            else:
                toolchain_path = "/usr/bin"
                gdb_path = "gdb-multiarch"

        if current_platform == "windows":
            cmake_generator = "Ninja"
            openocd_path = "C:/openocd/bin/openocd.exe"
            terminal_shell = "cmd.exe"
        elif current_platform == "linux":
            cmake_generator = "Unix Makefiles"
            openocd_path = "/usr/bin/openocd"
            terminal_shell = "/bin/bash"
        elif current_platform == "darwin":
            cmake_generator = "Unix Makefiles"
            openocd_path = "/usr/bin/openocd"
            terminal_shell = "/bin/bash"
        else:
            cmake_generator = "Ninja"
            openocd_path = "/usr/bin/openocd"
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

        if current_platform == "windows":
            settings_config["terminal.integrated.shell.windows"] = terminal_shell

        settings_file = vscode_dir / 'settings.json'
        try:
            with open(settings_file, 'w', encoding='utf-8') as f:
                json.dump(settings_config, f, indent=2, ensure_ascii=False)
            print(f"使用工具链路径: {toolchain_path}")
        except Exception as e:
            print(f"生成 settings.json 失败: {e}")


class AdditionalConfigGenerator:
    """其他配置文件生成器"""

    def __init__(self, project_root, config):
        self.project_root = Path(project_root)
        self.config = config

    def generate_idea_cfg(self):
        """生成idea.cfg文件"""
        interface = self.config.get('download.interface', 'stlink')
        target = self.config.get('download.target', 'stm32f1x')
        speed = self.config.get('download.speed', '4000')

        idea_cfg_content = f'''source [find interface/{interface}.cfg]
source [find target/{target}.cfg]
adapter speed {speed}
'''

        tool_dir = self.project_root / 'tool'
        tool_dir.mkdir(exist_ok=True)
        idea_cfg_file = tool_dir / 'idea.cfg'
        try:
            idea_cfg_file.write_text(idea_cfg_content, encoding='utf-8')
            print(f"生成 idea.cfg 文件")
        except Exception as e:
            print(f"生成 idea.cfg 失败: {e}")

    def generate_gitignore(self):
        """生成.gitignore文件"""
        gitignore_file = self.project_root / '.gitignore'

        gitignore_content = '''# 构建输出
build/

# 缓存文件夹
.cache/

# 自动生成的配置文件
CMakeLists.txt
BSP/CMakeLists.txt
Device/CMakeLists.txt
Device/config.h
tool/project_config.json
idea.cfg

# VS Code配置
.vscode/

# Python缓存
tool/__pycache__/
*.pyc
*.pyo

# Keil MDK构建输出
MDK/Objects/
MDK/Listings/
MDK/JLinkLog.txt

# 其他临时文件
*.bak
*.tmp
*.log
'''

        if not gitignore_file.exists():
            try:
                gitignore_file.write_text(gitignore_content, encoding='utf-8')
                print("生成 .gitignore 文件")
            except Exception as e:
                print(f"生成 .gitignore 失败: {e}")
        else:
            print(".gitignore 文件已存在，跳过生成")
