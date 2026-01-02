#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
CMakeLists.txt生成模块
负责生成CMake构建文件及相关配置文件
"""

import os
import re
import json
import glob
import platform
from pathlib import Path


class CMakeGenerator:
    """CMakeLists.txt生成器"""

    def __init__(self, project_root, config):
        self.project_root = Path(project_root)
        self.cmake_file = self.project_root / "CMakeLists.txt"
        self.config = config

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

    def get_fpu_flags(self, architecture):
        """根据架构获取浮点单元配置"""
        config = self.fpu_config.get(architecture, self.fpu_config['cortex-m3'])

        if config['fpu'] == 'soft':
            return '-mfloat-abi=soft'
        else:
            return f'-mfpu={config["fpu"]} -mfloat-abi={config["float_abi"]}'

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

            # 查找C编译器路径
            c_compiler_path = None
            c_compiler_match = re.search(r'CMAKE_C_COMPILER:STRING=(.+)', content)
            if c_compiler_match:
                c_compiler_path = c_compiler_match.group(1).strip()
            else:
                c_compiler_match = re.search(r'CMAKE_C_COMPILER:FILEPATH=(.+)', content)
                if c_compiler_match:
                    c_compiler_path = c_compiler_match.group(1).strip()
                else:
                    cxx_compiler_match = re.search(r'CMAKE_CXX_COMPILER:FILEPATH=(.+)', content)
                    if cxx_compiler_match:
                        cxx_compiler_path = cxx_compiler_match.group(1).strip()
                        if 'g++' in cxx_compiler_path or 'c++' in cxx_compiler_path:
                            c_compiler_path = cxx_compiler_path.replace('g++', 'gcc').replace('c++', 'gcc')

            if c_compiler_path:
                toolchain_info["c_compiler"] = c_compiler_path
                c_compiler_path_obj = Path(c_compiler_path)
                if c_compiler_path_obj.exists():
                    toolchain_info["toolchain_path"] = str(c_compiler_path_obj.parent)
                else:
                    toolchain_info["toolchain_path"] = str(c_compiler_path_obj.parent)
                    print("警告: 编译器文件不存在，但已推导出工具链路径")

            # 查找C++编译器路径
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
                    break

            if toolchain_info["toolchain_path"]:
                print(f"从CMake缓存读取到工具链路径: {toolchain_info['toolchain_path']}")
            else:
                print("警告: 未能从CMake缓存中获取工具链路径")

            return toolchain_info

        except Exception as e:
            print(f"读取CMake缓存失败: {e}")
            return toolchain_info

    def generate_cmake(self, sources, all_include_dirs, subdirs_with_cmake, static_libraries):
        """生成模块化的CMakeLists.txt内容"""
        # 导入芯片检测器
        from chip_detector import ChipDetector

        # 从配置获取各种参数
        chip_name = self.config.get('project.chip')
        board_name = self.config.get('project.board')
        arch = self.config.get('project.architecture')
        project_name = self.config.get('project.name')
        fpu_flags = self.get_fpu_flags(arch)
        float_desc = self.config.get('project.float_type')
        linker_script = self.config.get('linker.script')

        # 使用芯片检测器获取BSP目录
        detector = ChipDetector(self.project_root)
        chip_info = detector.get_chip_info(chip_name)  # 使用配置中的芯片名称
        bsp_chip_dir = chip_info.get('bsp_dir', '')
        chip_package = chip_info.get('chip_package', '')  # 获取芯片封装型号 (如vet6)

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

        # 构建额外的链接器脚本参数
        additional_scripts = self.config.get('linker.additional_scripts', [])
        additional_script_flags = ' '.join([f'-T${{CMAKE_SOURCE_DIR}}/{script}' for script in additional_scripts])

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
        print(f"芯片封装: {chip_package}")
        print(f"芯片架构: {arch}")
        print(f"浮点配置: {float_desc}")
        print(f"主链接脚本: {linker_script}")
        if additional_scripts:
            print(f"额外链接脚本: {additional_scripts}")
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
set(CHIP_PACKAGE "{chip_package}")  # 芯片封装型号 (如vet6, vgt6等)

# BSP芯片目录
set(BSP_CHIP_DIR "{bsp_chip_dir}")
set(BSP_PACKAGE_DIR "{chip_package}")  # 封装特定驱动目录

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
set(CMAKE_EXE_LINKER_FLAGS "-mcpu={arch} -mthumb {fpu_flags} {specs_flags} -T${{CMAKE_SOURCE_DIR}}/{linker_script} {additional_script_flags} {linker_options_str}")

# 宏定义
'''

        # 添加宏定义
        for define in self.config.get('defines', []):
            content += f'add_definitions(-D{define})\n'

        # 添加芯片封装型号宏定义
        if chip_package:
            package_macro = f'CHIP_PACKAGE_{chip_package.upper()}'
            content += f'add_definitions(-D{package_macro})  # 芯片封装型号: {chip_package}\n'
            content += f'add_definitions(-DCHIP_PACKAGE="{chip_package}")  # 芯片封装字符串\n'

        # 添加设备驱动开关宏定义
        devices_config = self.config.get('devices', {})
        for device_name, device_cfg in sorted(devices_config.items()):
            if device_cfg.get('enabled', True):
                var_name = device_name.upper().replace('-', '_').replace('.', '_')
                macro_name = f'USE_DEVICE_{var_name}'
                content += f'add_definitions(-D{macro_name})  # 启用{device_name}设备驱动\n'
        content += '\n'

        # 添加BSP子目录
        content += "# BSP层(板级支持包)\n"
        content += "add_subdirectory(BSP)\n\n"

        # 添加Device子目录
        content += "# Device层(外部设备驱动)\n"
        content += "add_subdirectory(Device)\n\n"

        # 添加包含目录(应用层和框架层)
        if all_include_dirs:
            content += "# 应用层和框架层头文件目录\ninclude_directories(\n"
            sorted_dirs = sorted(all_include_dirs, key=lambda x: (len(x.parts), str(x)))
            for inc_dir in sorted_dirs:
                if isinstance(inc_dir, str):
                    content += f"    {inc_dir}\n"
                else:
                    rel_path = inc_dir.relative_to(self.project_root)
                    content += f"    {rel_path.as_posix()}\n"
            content += ")\n"
            content += "# BSP和Device层头文件目录由子目录CMakeLists.txt导出\n"
            content += "include_directories(Device)  # 添加Device根目录用于<config.h>\n"
            content += "include_directories(${BSP_INCLUDE_DIRS})\n"
            content += "include_directories(${DEVICE_INCLUDE_DIRS})\n\n"
        else:
            content += "# BSP和Device层头文件目录由子目录CMakeLists.txt导出\n"
            content += "include_directories(Device)  # 添加Device根目录用于<config.h>\n"
            content += "include_directories(${BSP_INCLUDE_DIRS})\n"
            content += "include_directories(${DEVICE_INCLUDE_DIRS})\n\n"

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
        content += "# 创建可执行文件\nadd_executable(${PROJECT_NAME}.elf \n"
        content += "    ${SOURCE_FILES}\n"
        content += "    ${BSP_SOURCES}      # BSP层源文件\n"
        content += "    ${DEVICE_SOURCES}   # Device层源文件\n"
        content += ")\n\n"

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

    def write_cmake(self, content):
        """写入CMakeLists.txt文件"""
        try:
            self.cmake_file.write_text(content, encoding='utf-8')
            print(f"已生成: {self.cmake_file}")
        except Exception as e:
            print(f"写入CMakeLists.txt失败: {e}")

    def generate_bsp_cmake(self, bsp_chip_dir, bsp_sources, bsp_include_dirs, chip_package=None):
        """生成BSP/CMakeLists.txt内容"""
        content = '''# BSP (Board Support Package) CMakeLists
# 本文件由构建脚本自动生成，请勿手动修改
# 管理特定芯片平台的编译配置和源文件

'''

        content += f'# bsp chip catalog\n'
        content += f'set(BSP_CHIP_DIR "{bsp_chip_dir}")\n'
        content += f'message(STATUS "bsp chip catalog: ${{BSP_CHIP_DIR}}")\n\n'

        # 添加芯片封装信息
        if chip_package:
            content += f'# 芯片封装型号 (如 vet6, vgt6 等)\n'
            content += f'set(CHIP_PACKAGE "{chip_package}")\n'
            content += f'set(BSP_PACKAGE_DIR "{chip_package}")\n'
            content += f'message(STATUS "chip package: ${{CHIP_PACKAGE}}")\n\n'

            # 检查封装目录是否存在
            content += f'# 检查封装特定驱动目录\n'
            content += f'set(PACKAGE_DRIVER_DIR "${{CMAKE_CURRENT_SOURCE_DIR}}/${{BSP_CHIP_DIR}}/Driver/${{CHIP_PACKAGE}}")\n'
            content += f'if(EXISTS "${{PACKAGE_DRIVER_DIR}}")\n'
            content += f'    message(STATUS "found package driver dir: ${{PACKAGE_DRIVER_DIR}}")\n'
            content += f'    set(HAS_PACKAGE_DRIVER TRUE)\n'
            content += f'else()\n'
            content += f'    message(STATUS "no package driver dir found")\n'
            content += f'    set(HAS_PACKAGE_DRIVER FALSE)\n'
            content += f'endif()\n\n'

        # BSP源文件
        content += '# BSP源文件\n'
        content += 'set(BSP_SOURCES\n'
        for src in bsp_sources:
            rel_path = src.relative_to(self.project_root / 'BSP')
            content += f'    {rel_path.as_posix()}\n'
        content += ')\n\n'

        # 输出源文件列表
        content += '''# 输出BSP源文件列表
message(STATUS "collect bsp source files:")
foreach(src ${BSP_SOURCES})
    message(STATUS "  + BSP/${src}")
endforeach()
list(LENGTH BSP_SOURCES BSP_SOURCE_COUNT)
message(STATUS "total bsp source files: ${BSP_SOURCE_COUNT}")

'''

        # BSP头文件目录
        content += '# BSP头文件目录\n'
        content += 'set(BSP_INCLUDE_DIRS\n'
        for inc_dir in bsp_include_dirs:
            rel_path = inc_dir.relative_to(self.project_root)
            content += f'    ${{CMAKE_SOURCE_DIR}}/{rel_path.as_posix()}\n'
        content += ')\n\n'

        # 导出到父作用域
        content += '''# 将源文件路径转换为绝对路径
set(BSP_SOURCES_ABS "")
foreach(src ${BSP_SOURCES})
    list(APPEND BSP_SOURCES_ABS ${CMAKE_CURRENT_SOURCE_DIR}/${src})
endforeach()

# 导出到父作用域
set(BSP_SOURCES ${BSP_SOURCES_ABS} PARENT_SCOPE)
set(BSP_INCLUDE_DIRS ${BSP_INCLUDE_DIRS} PARENT_SCOPE)

# 可选：创建BSP库（用于独立编译测试）
if(BUILD_BSP_ONLY)
    add_library(bsp STATIC ${BSP_SOURCES_ABS})
    target_include_directories(bsp PUBLIC ${BSP_INCLUDE_DIRS})
    target_compile_options(bsp PRIVATE
        -Wall
        -fdata-sections
        -ffunction-sections
    )
    message(STATUS "Building BSP only mode")
endif()
'''

        return content

    def generate_device_cmake(self, devices):
        """生成Device/CMakeLists.txt内容"""
        content = '''# Device (外部设备驱动) CMakeLists
# 本文件由构建脚本自动生成，请勿手动修改
# 管理外部设备驱动的编译配置和源文件
#
# 设备驱动开关通过宏定义控制：
# - USE_DEVICE_XXX: 启用某个设备驱动
# 在 project_config.json 的 devices 配置中设置 "enabled": true/false 来控制

message(STATUS "configure external device drivers...")

'''

        # 自动扫描Device目录下的核心接口文件
        device_dir = self.project_root / 'Device'
        core_files = []
        core_file_patterns = ['device_hal.c', 'device_init.c', 'device_hal.h', 'device_init.h']

        for pattern in core_file_patterns:
            file_path = device_dir / pattern
            if file_path.exists() and pattern.endswith('.c'):
                core_files.append(pattern)

        if core_files:
            content += '# Device HAL层和初始化模块（自动扫描）\n'
            content += 'set(DEVICE_CORE_SOURCES\n'
            for core_file in sorted(core_files):
                content += f'    {core_file}\n'
            content += ')\n'
            content += 'message(STATUS "collect device core files:")\n'
            content += 'foreach(src ${DEVICE_CORE_SOURCES})\n'
            content += '    message(STATUS "  + Device/${src}")\n'
            content += 'endforeach()\n\n'

        # 为每个设备生成源文件列表
        all_device_sources = []
        all_device_includes = []

        for device_name, device_info in sorted(devices.items()):
            sources = device_info['sources']
            include_dirs = device_info['include_dirs']

            var_name = device_name.upper().replace('-', '_').replace('.', '_')
            macro_name = f'USE_DEVICE_{var_name}'

            content += f'# 设备: {device_name} (宏: {macro_name})\n'
            content += f'set({var_name}_SOURCES\n'
            for src in sources:
                rel_path = src.relative_to(self.project_root / 'Device')
                content += f'    {rel_path.as_posix()}\n'
                all_device_sources.append(f'    ${{{var_name}_SOURCES}}')
            content += ')\n'

            # 输出该设备的文件列表
            content += f'message(STATUS "collect device source files [{device_name}]:")\n'
            content += f'foreach(src ${{{var_name}_SOURCES}})\n'
            content += f'    message(STATUS "  + Device/${{src}}")\n'
            content += f'endforeach()\n\n'

            # 添加头文件目录
            for inc_dir in include_dirs:
                rel_path = inc_dir.relative_to(self.project_root)
                all_device_includes.append(f'    ${{CMAKE_SOURCE_DIR}}/{rel_path.as_posix()}')

        # 汇总所有设备源文件（包含核心文件）
        content += '# 所有设备源文件（包含核心接口层）\n'
        content += 'set(DEVICE_SOURCES\n'
        if core_files:
            content += '    ${DEVICE_CORE_SOURCES}\n'
        for src_var in sorted(set(all_device_sources)):
            content += f'{src_var}\n'
        content += ')\n\n'

        # 汇总所有头文件目录
        content += '# 所有设备头文件目录\n'
        content += 'set(DEVICE_INCLUDE_DIRS\n'
        for inc_dir in sorted(set(all_device_includes)):
            content += f'{inc_dir}\n'
        content += ')\n\n'

        # 输出统计信息
        content += '''list(LENGTH DEVICE_SOURCES DEVICE_SOURCE_COUNT)
message(STATUS "total device source files: ${DEVICE_SOURCE_COUNT}")

# 将源文件路径转换为绝对路径
set(DEVICE_SOURCES_ABS "")
foreach(src ${DEVICE_SOURCES})
    list(APPEND DEVICE_SOURCES_ABS ${CMAKE_CURRENT_SOURCE_DIR}/${src})
endforeach()

# 导出到父作用域
set(DEVICE_SOURCES ${DEVICE_SOURCES_ABS} PARENT_SCOPE)
set(DEVICE_INCLUDE_DIRS ${DEVICE_INCLUDE_DIRS} PARENT_SCOPE)

# 可选：仅编译Device层
if(BUILD_DEVICE_ONLY)
    add_library(device_all STATIC ${DEVICE_SOURCES_ABS})
    target_include_directories(device_all PUBLIC ${DEVICE_INCLUDE_DIRS})
    target_compile_options(device_all PRIVATE
        -Wall
        -fdata-sections
        -ffunction-sections
    )
    message(STATUS "Building Device layer only")
endif()
'''

        return content

    def write_bsp_cmake(self, content):
        """写入BSP/CMakeLists.txt文件"""
        try:
            bsp_cmake_file = self.project_root / 'BSP' / 'CMakeLists.txt'
            bsp_cmake_file.parent.mkdir(parents=True, exist_ok=True)
            bsp_cmake_file.write_text(content, encoding='utf-8')
            print(f"已生成: {bsp_cmake_file}")
        except Exception as e:
            print(f"写入BSP/CMakeLists.txt失败: {e}")

    def write_device_cmake(self, content):
        """写入Device/CMakeLists.txt文件"""
        try:
            device_cmake_file = self.project_root / 'Device' / 'CMakeLists.txt'
            device_cmake_file.parent.mkdir(parents=True, exist_ok=True)
            device_cmake_file.write_text(content, encoding='utf-8')
            print(f"已生成: {device_cmake_file}")
        except Exception as e:
            print(f"写入Device/CMakeLists.txt失败: {e}")
