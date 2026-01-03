#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
CMake生成模块
负责生成CMakeLists.txt文件
"""

import re
from pathlib import Path


class CMakeGenerator:
    """CMake生成器"""

    def __init__(self, project_root, config):
        self.project_root = Path(project_root)
        self.config = config

        # FPU配置
        self.fpu_config = {
            'cortex-m0': {'fpu': 'soft', 'float_abi': 'soft'},
            'cortex-m0plus': {'fpu': 'soft', 'float_abi': 'soft'},
            'cortex-m3': {'fpu': 'soft', 'float_abi': 'soft'},
            'cortex-m4': {'fpu': 'fpv4-sp-d16', 'float_abi': 'hard'},
            'cortex-m7': {'fpu': 'fpv5-d16', 'float_abi': 'hard'},
            'cortex-m33': {'fpu': 'fpv5-sp-d16', 'float_abi': 'hard'}
        }

    def get_fpu_flags(self, arch):
        """获取FPU编译标志"""
        fpu_info = self.fpu_config.get(arch, self.fpu_config['cortex-m3'])
        if fpu_info['fpu'] == 'soft':
            return "-mfloat-abi=soft"
        else:
            return f"-mfpu={fpu_info['fpu']} -mfloat-abi={fpu_info['float_abi']}"

    def read_cmake_cache(self):
        """读取CMakeCache.txt获取工具链信息"""
        cache_file = self.project_root / 'build' / 'CMakeCache.txt'
        result = {
            'c_compiler': '',
            'cxx_compiler': '',
            'toolchain_path': ''
        }

        if not cache_file.exists():
            return result

        try:
            content = cache_file.read_text(encoding='utf-8', errors='ignore')

            # 查找编译器路径
            c_match = re.search(r'CMAKE_C_COMPILER:FILEPATH=(.+)', content)
            if c_match:
                result['c_compiler'] = c_match.group(1).strip()
                # 提取工具链路径
                compiler_path = Path(result['c_compiler'])
                if compiler_path.parent.name == 'bin':
                    result['toolchain_path'] = str(compiler_path.parent)

            cxx_match = re.search(r'CMAKE_CXX_COMPILER:FILEPATH=(.+)', content)
            if cxx_match:
                result['cxx_compiler'] = cxx_match.group(1).strip()

        except Exception as e:
            print(f"警告: 读取CMakeCache.txt失败: {e}")

        return result

    def generate_cmake(self, sources, include_dirs, subdirs_with_cmake, static_libraries):
        """生成主CMakeLists.txt内容"""
        # 获取配置
        project_name = self.config.get('project.name', 'STM32_Project')
        chip = self.config.get('project.chip', 'STM32F407VGT6')
        board = self.config.get('project.board', 'BLUEPILL')
        arch = self.config.get('project.architecture', 'cortex-m4')
        float_type = self.config.get('project.float_type', '')

        # 工具链配置
        c_compiler = self.config.get('toolchain.c_compiler', 'arm-none-eabi-gcc')
        cxx_compiler = self.config.get('toolchain.cxx_compiler', 'arm-none-eabi-g++')
        asm_compiler = self.config.get('toolchain.asm_compiler', 'arm-none-eabi-gcc')
        ar = self.config.get('toolchain.ar', 'arm-none-eabi-ar')
        objcopy = self.config.get('toolchain.objcopy', 'arm-none-eabi-objcopy')
        size = self.config.get('toolchain.size', 'arm-none-eabi-size')

        # 构建配置
        optimization = self.config.get('build.optimization', 'O0')
        debug_info = self.config.get('build.debug_info', 'g3')
        stack_usage = self.config.get('build.stack_usage', True)

        # 链接配置
        linker_script = self.config.get('linker.script', '')
        additional_scripts = self.config.get('linker.additional_scripts', [])
        specs = self.config.get('linker.specs', ['nosys.specs', 'nano.specs'])
        gc_sections = self.config.get('linker.gc_sections', True)
        map_file = self.config.get('linker.map_file', True)
        printf_float = self.config.get('linker.printf_float', True)
        scanf_float = self.config.get('linker.scanf_float', True)

        # 宏定义
        defines = self.config.get('defines', [])

        # BSP配置
        bsp_config = self.config.get('bsp', {})
        bsp_chip_dir = bsp_config.get('chip_dir', '')
        chip_model_dir = bsp_config.get('chip_model_dir', '')
        chip_package = bsp_config.get('chip_package', '')

        # 获取FPU标志
        fpu_flags = self.get_fpu_flags(arch)

        # 生成编译标志
        common_flags = f"-mcpu={arch} -mthumb {fpu_flags} -fdata-sections -ffunction-sections"
        if stack_usage:
            common_flags += " -fstack-usage"
        common_flags += f" -{optimization} -{debug_info}"

        # 生成链接标志
        linker_flags = f"-mcpu={arch} -mthumb {fpu_flags}"
        for spec in specs:
            linker_flags += f" -specs={spec}"
        if linker_script:
            linker_flags += f" -T${{CMAKE_SOURCE_DIR}}/{linker_script}"
        for script in additional_scripts:
            linker_flags += f" -T${{CMAKE_SOURCE_DIR}}/{script}"
        if gc_sections:
            linker_flags += " -Wl,--gc-sections"
        if map_file:
            linker_flags += " -Wl,-Map=${PROJECT_NAME}.map,--cref"
        if printf_float:
            linker_flags += " -u _printf_float"
        if scanf_float:
            linker_flags += " -u _scanf_float"
        linker_flags += " -Wl,--print-memory-usage"

        # 开始生成CMakeLists.txt
        lines = []
        lines.append("#本文件由自动化脚本生成,改这个文件是没用的,要更改配置请修改项目根目录下的 project_config.json 文件,然后再次运行脚本,即可更改编译配置")
        lines.append("#未经作者允许,不可更改脚本内容,不可传播,脚本源码闭源,只可在实验室内部使用,否则本人线下追究责任,脚本版权归作者本人和实验室所有")
        lines.append("cmake_minimum_required(VERSION 3.16)")
        lines.append("")
        lines.append("# 项目配置")
        lines.append("set(CMAKE_SYSTEM_NAME Generic)")
        lines.append(f"set(CMAKE_SYSTEM_PROCESSOR {arch})")
        lines.append(f"set(TARGET_CHIP {chip})")
        lines.append(f"set(TARGET_BOARD {board})")
        if chip_package:
            lines.append(f'set(CHIP_PACKAGE "{chip_package}")  # 芯片封装型号 (如vet6, vgt6等)')
        lines.append("")
        lines.append("# BSP芯片目录")
        lines.append(f'set(BSP_CHIP_DIR "{bsp_chip_dir}")')
        if chip_model_dir:
            lines.append(f'set(BSP_MODEL_DIR "{chip_model_dir}")  # 芯片型号目录')
        if chip_package:
            lines.append(f'set(BSP_PACKAGE_DIR "{chip_package}")  # 封装特定驱动目录')
        lines.append("")
        lines.append("# 工具链")
        lines.append(f"set(CMAKE_C_COMPILER {c_compiler})")
        lines.append(f"set(CMAKE_CXX_COMPILER {cxx_compiler})")
        lines.append(f"set(CMAKE_ASM_COMPILER {asm_compiler})")
        lines.append(f"set(CMAKE_AR {ar})")
        lines.append(f"set(CMAKE_OBJCOPY {objcopy})")
        lines.append(f"set(SIZE {size})")
        lines.append("set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)")
        lines.append("")
        lines.append(f"project({project_name} C CXX ASM)")
        lines.append("")
        lines.append(f"# 编译标志 ({float_type})")
        lines.append(f'set(CMAKE_C_FLAGS "{common_flags}")')
        lines.append(f'set(CMAKE_CXX_FLAGS "{common_flags}")')
        lines.append(f'set(CMAKE_ASM_FLAGS "{common_flags}")')
        lines.append("")
        lines.append("# 链接标志")
        lines.append(f'set(CMAKE_EXE_LINKER_FLAGS "{linker_flags}")')
        lines.append("")
        lines.append("# 宏定义")
        for define in defines:
            lines.append(f"add_definitions(-D{define})")
        if chip_model_dir:
            lines.append(f"add_definitions(-DCHIP_MODEL_{chip_model_dir.upper()})     # 芯片型号: {chip_model_dir}")
        if chip_package:
            lines.append(f"add_definitions(-DCHIP_PACKAGE_{chip_package.upper()})   # 芯片封装型号: {chip_package}")
            lines.append(f'add_definitions(-DCHIP_PACKAGE="{chip_package}") # 芯片封装字符串')

        # 设备宏定义
        devices_config = self.config.get('devices', {})
        for device_name, device_cfg in devices_config.items():
            if device_cfg.get('enabled', True):
                lines.append(f"add_definitions(-DUSE_DEVICE_{device_name.upper()})  # 启用{device_name}设备驱动")

        lines.append("")
        lines.append("# BSP层(板级支持包)")
        lines.append("add_subdirectory(BSP)")
        lines.append("")
        lines.append("# Device层(外部设备驱动)")
        lines.append("add_subdirectory(Device)")
        lines.append("")

        # 生成头文件目录
        lines.append("# 应用层和框架层头文件目录")
        lines.append("include_directories(")
        for inc_dir in sorted(include_dirs):
            try:
                rel_path = inc_dir.relative_to(self.project_root)
                lines.append(f"    {rel_path.as_posix()}")
            except ValueError:
                lines.append(f"    {inc_dir.as_posix()}")
        lines.append(")")
        lines.append("# BSP和Device层头文件目录由子目录CMakeLists.txt导出")
        lines.append("include_directories(Device)  # 添加Device根目录用于<config.h>")
        lines.append("include_directories(${BSP_INCLUDE_DIRS})")
        lines.append("include_directories(${DEVICE_INCLUDE_DIRS})")
        lines.append("")

        # 按目录分组源文件
        source_groups = self._group_sources_by_dir(sources)
        for group_name, group_sources in sorted(source_groups.items()):
            var_name = group_name.upper().replace('/', '_').replace('-', '_') + "_SOURCES"
            lines.append(f"set({var_name}")
            for src in sorted(group_sources):
                try:
                    rel_path = src.relative_to(self.project_root)
                    lines.append(f"    {rel_path.as_posix()}")
                except ValueError:
                    lines.append(f"    {src.as_posix()}")
            lines.append(")")
            lines.append("")

        # 合并所有源文件
        lines.append("# 所有源文件")
        lines.append("set(SOURCE_FILES")
        for group_name in sorted(source_groups.keys()):
            var_name = group_name.upper().replace('/', '_').replace('-', '_') + "_SOURCES"
            lines.append(f"    ${{{var_name}}}")
        lines.append(")")
        lines.append("")

        # 创建可执行文件
        lines.append("# 创建可执行文件")
        lines.append("add_executable(${PROJECT_NAME}.elf")
        lines.append("    ${SOURCE_FILES}")
        lines.append("    ${BSP_SOURCES}      # BSP层源文件")
        lines.append("    ${DEVICE_SOURCES}   # Device层源文件")
        lines.append(")")
        lines.append("")

        # 后处理命令
        lines.append("# 生成输出文件")
        lines.append("add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD")
        lines.append("    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${PROJECT_NAME}.elf> ${PROJECT_NAME}.hex")
        lines.append("    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${PROJECT_NAME}.elf> ${PROJECT_NAME}.bin")
        lines.append('    COMMAND ${SIZE} --format=berkeley $<TARGET_FILE:${PROJECT_NAME}.elf>')
        lines.append('    COMMENT "Building output files"')
        lines.append(")")

        return '\n'.join(lines)

    def _group_sources_by_dir(self, sources):
        """按目录分组源文件"""
        groups = {}
        for src in sources:
            try:
                rel_path = src.relative_to(self.project_root)
                parts = rel_path.parts
                if len(parts) >= 2:
                    # 使用前两级目录作为分组名
                    group = '/'.join(parts[:2]) if len(parts) > 2 else parts[0]
                else:
                    group = parts[0] if parts else 'root'
            except ValueError:
                group = 'external'

            if group not in groups:
                groups[group] = []
            groups[group].append(src)

        return groups

    def write_cmake(self, content):
        """写入CMakeLists.txt"""
        cmake_file = self.project_root / 'CMakeLists.txt'
        try:
            cmake_file.write_text(content, encoding='utf-8')
            print(f"已生成: CMakeLists.txt")
        except Exception as e:
            print(f"写入CMakeLists.txt失败: {e}")

    def generate_bsp_cmake(self, bsp_chip_dir, bsp_sources, bsp_include_dirs, chip_package=None):
        """生成BSP/CMakeLists.txt"""
        bsp_config = self.config.get('bsp', {})
        chip_model_dir = bsp_config.get('chip_model_dir', '')

        lines = []
        lines.append("# BSP (Board Support Package) CMakeLists")
        lines.append("# 本文件由构建脚本自动生成，请勿手动修改")
        lines.append("# 管理特定芯片平台的编译配置和源文件")
        lines.append("")
        lines.append("# 芯片配置层级:")
        lines.append("# BSP_CHIP_DIR     - 芯片系列目录 (如 stm32f4)")
        lines.append("# BSP_MODEL_DIR    - 芯片型号目录 (如 f407)")
        lines.append("# BSP_PACKAGE_DIR  - 芯片封装目录 (如 vgt6)")
        lines.append(f'set(BSP_CHIP_DIR "{bsp_chip_dir}")')
        if chip_model_dir:
            lines.append(f'set(BSP_MODEL_DIR "{chip_model_dir}")')
        if chip_package:
            lines.append(f'set(BSP_PACKAGE_DIR "{chip_package}")')
        lines.append(f'message(STATUS "BSP chip config: ${{BSP_CHIP_DIR}}/${{BSP_MODEL_DIR}}/${{BSP_PACKAGE_DIR}}")')
        lines.append("")

        # 源文件
        lines.append("# BSP源文件")
        lines.append("set(BSP_SOURCES")
        for src in sorted(bsp_sources):
            try:
                rel_path = src.relative_to(self.project_root / 'BSP')
                lines.append(f"    {rel_path.as_posix()}")
            except ValueError:
                lines.append(f"    {src.as_posix()}")
        lines.append(")")
        lines.append("")

        # 输出信息
        lines.append("# 输出BSP源文件列表")
        lines.append('message(STATUS "collect bsp source files:")')
        lines.append("foreach(src ${BSP_SOURCES})")
        lines.append('    message(STATUS "  + BSP/${src}")')
        lines.append("endforeach()")
        lines.append("list(LENGTH BSP_SOURCES BSP_SOURCE_COUNT)")
        lines.append('message(STATUS "total bsp source files: ${BSP_SOURCE_COUNT}")')
        lines.append("")

        # 头文件目录
        lines.append("# BSP头文件目录")
        lines.append("set(BSP_INCLUDE_DIRS")
        for inc in sorted(bsp_include_dirs):
            lines.append(f"    ${{CMAKE_SOURCE_DIR}}/{inc.relative_to(self.project_root).as_posix()}")
        lines.append(")")
        lines.append("")

        # 转换为绝对路径
        lines.append("# 将源文件路径转换为绝对路径")
        lines.append('set(BSP_SOURCES_ABS "")')
        lines.append("foreach(src ${BSP_SOURCES})")
        lines.append("    list(APPEND BSP_SOURCES_ABS ${CMAKE_CURRENT_SOURCE_DIR}/${src})")
        lines.append("endforeach()")
        lines.append("")

        # 导出变量
        lines.append("# 导出到父作用域")
        lines.append("set(BSP_SOURCES ${BSP_SOURCES_ABS} PARENT_SCOPE)")
        lines.append("set(BSP_INCLUDE_DIRS ${BSP_INCLUDE_DIRS} PARENT_SCOPE)")

        return '\n'.join(lines)

    def write_bsp_cmake(self, content):
        """写入BSP/CMakeLists.txt"""
        bsp_cmake = self.project_root / 'BSP' / 'CMakeLists.txt'
        try:
            bsp_cmake.write_text(content, encoding='utf-8')
            print(f"已生成: BSP/CMakeLists.txt")
        except Exception as e:
            print(f"写入BSP/CMakeLists.txt失败: {e}")

    def generate_device_cmake(self, devices):
        """生成Device/CMakeLists.txt"""
        lines = []
        lines.append("# Device (外部设备驱动) CMakeLists")
        lines.append("# 本文件由构建脚本自动生成，请勿手动修改")
        lines.append("# 管理外部设备驱动的编译配置")
        lines.append("")
        lines.append("# 获取当前目录名作为模块名")
        lines.append("get_filename_component(MODULE_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)")
        lines.append('message(STATUS "Configure Device Layer: ${MODULE_NAME}")')
        lines.append("")

        # 设备源文件
        lines.append("# 设备源文件")
        lines.append("set(DEVICE_SOURCES")
        for device_name, device_info in sorted(devices.items()):
            # 为 _root 使用更友好的注释名称
            display_name = "device_common" if device_name == "_root" else device_name
            lines.append(f"    # {display_name}")
            for src in device_info['sources']:
                try:
                    rel_path = src.relative_to(self.project_root / 'Device')
                    lines.append(f"    {rel_path.as_posix()}")
                except ValueError:
                    lines.append(f"    {src.as_posix()}")
        lines.append(")")
        lines.append("")

        # 头文件目录
        all_includes = set()
        for device_info in devices.values():
            for inc in device_info['include_dirs']:
                all_includes.add(inc)

        lines.append("# 设备头文件目录")
        lines.append("set(DEVICE_INCLUDE_DIRS")
        for inc in sorted(all_includes):
            lines.append(f"    ${{CMAKE_SOURCE_DIR}}/{inc.relative_to(self.project_root).as_posix()}")
        lines.append(")")
        lines.append("")

        # 输出信息
        lines.append("# 输出信息")
        lines.append('message(STATUS "Enabled Device Drivers:")')
        for device_name in sorted(devices.keys()):
            # 为 _root 使用更友好的显示名称
            display_name = "device_common (root sources)" if device_name == "_root" else device_name
            lines.append(f'message(STATUS "  + {display_name}")')
        lines.append("list(LENGTH DEVICE_SOURCES DEVICE_SOURCE_COUNT)")
        lines.append('message(STATUS "Total Number Of Device Source Files: ${DEVICE_SOURCE_COUNT}")')
        lines.append("")

        # 转换为绝对路径
        lines.append("# 将源文件路径转换为绝对路径")
        lines.append('set(DEVICE_SOURCES_ABS "")')
        lines.append("foreach(src ${DEVICE_SOURCES})")
        lines.append("    list(APPEND DEVICE_SOURCES_ABS ${CMAKE_CURRENT_SOURCE_DIR}/${src})")
        lines.append("endforeach()")
        lines.append("")

        # 导出变量
        lines.append("# 导出到父作用域")
        lines.append("set(DEVICE_SOURCES ${DEVICE_SOURCES_ABS} PARENT_SCOPE)")
        lines.append("set(DEVICE_INCLUDE_DIRS ${DEVICE_INCLUDE_DIRS} PARENT_SCOPE)")

        return '\n'.join(lines)

    def write_device_cmake(self, content):
        """写入Device/CMakeLists.txt"""
        device_cmake = self.project_root / 'Device' / 'CMakeLists.txt'
        try:
            device_cmake.write_text(content, encoding='utf-8')
            print(f"已生成: Device/CMakeLists.txt")
        except Exception as e:
            print(f"写入Device/CMakeLists.txt失败: {e}")
