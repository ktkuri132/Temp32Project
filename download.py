import os
import glob
import subprocess
import json
from pathlib import Path

class ProjectConfig:
    """项目配置管理类"""

    CONFIG_FILE = "build/project_config.json"

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
                    return json.load(f)
            except Exception as e:
                print(f"加载配置文件失败: {e}")
                return {}
        return {}

    def save_config(self):
        """保存配置文件"""
        try:
            with open(self.config_file, 'w', encoding='utf-8') as f:
                json.dump(self.config, f, indent=2, ensure_ascii=False)
        except Exception as e:
            print(f"保存配置文件失败: {e}")

    def get(self, path, default=None):
        """获取配置值"""
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

def choose_config(config):
    """选择下载配置"""
    interfaces = {
        "1": "stlink",
        "2": "jlink",
        "3": "cmsis-dap",
        "4": "xds110"
    }

    # 显示当前芯片信息
    chip = config.get('project.chip', 'Unknown')
    current_target = config.get('download.target', 'stm32f1x')
    print(f"\n当前芯片: {chip}")
    print(f"检测到的目标: {current_target}")

    print("\n请选择下载接口:")
    for k, v in interfaces.items():
        print(f"{k}. {v}")

    current_interface = config.get('download.interface', 'stlink')
    choice = input(f"输入数字选择(当前: {current_interface}): ").strip()
    if choice in interfaces:
        config.set('download.interface', interfaces[choice])

    current_speed = config.get('download.speed', '4000')
    speed = input(f"请输入下载速度(当前: {current_speed}): ").strip()
    if speed:
        config.set('download.speed', speed)

    return True

def find_elf():
    """查找ELF文件"""
    files = glob.glob(os.path.join("build", "*.elf"))
    if not files:
        print("未找到 build 目录下的 elf 文件!")
        return None
    return files[0]

def update_idea_cfg(config):
    """更新idea.cfg文件"""
    interface = config.get('download.interface', 'stlink')
    target = config.get('download.target', 'stm32f1x')
    speed = config.get('download.speed', '4000')

    idea_cfg_content = f'''source [find interface/{interface}.cfg]
source [find target/{target}.cfg]
adapter speed {speed}
'''

    try:
        # 确保 build 目录存在
        Path('build').mkdir(exist_ok=True)
        with open('build/idea.cfg', 'w', encoding='utf-8') as f:
            f.write(idea_cfg_content)
        print(f"已更新 idea.cfg 文件")
    except Exception as e:
        print(f"更新 idea.cfg 失败: {e}")

def create_jlink_script(config, elf_file):
    """创建 J-Link 下载脚本"""
    chip = config.get('project.chip', 'STM32F103C8')
    speed = config.get('download.speed', '4000')

    # 将 elf 文件转换为 hex 文件路径
    hex_file = elf_file.replace('.elf', '.hex')
    hex_file = hex_file.replace('/', '\\')  # Windows路径格式

    jlink_script_content = f'''connect
device {chip}
si SWD
speed {speed}
halt
erase
loadfile {hex_file}
r
g
qc
'''

    script_path = Path('build/flash_simple.jlink')
    try:
        with open(script_path, 'w', encoding='utf-8') as f:
            f.write(jlink_script_content)
        print(f"已创建 J-Link 脚本: {script_path}")
        return str(script_path)
    except Exception as e:
        print(f"创建 J-Link 脚本失败: {e}")
        return None

def download_with_jlink(config, elf_file):
    """使用 J-Link 进行下载"""
    print("使用 J-Link 进行下载...")

    # 创建 J-Link 脚本
    script_path = create_jlink_script(config, elf_file)
    if not script_path:
        return False

    # 查找 J-Link 可执行文件
    jlink_paths = [
        r"C:\Program Files\SEGGER\JLink\JLink.exe",
        r"C:\Program Files (x86)\SEGGER\JLink\JLink.exe",
        r"C:\Program Files\SEGGER\JLink_V862\JLink.exe",
        r"C:\Program Files (x86)\SEGGER\JLink_V862\JLink.exe"
    ]

    jlink_exe = None
    for path in jlink_paths:
        if Path(path).exists():
            jlink_exe = path
            break

    if not jlink_exe:
        print("未找到 J-Link 可执行文件，请确保已安装 J-Link 软件")
        print("尝试的路径:")
        for path in jlink_paths:
            print(f"  {path}")
        return False

    chip = config.get('project.chip', 'STM32F103C8')
    speed = config.get('download.speed', '4000')

    # 构建 J-Link 命令
    cmd = [
        jlink_exe,
        "-AutoConnect", "1",
        "-If", "SWD",
        "-Speed", speed,
        "-Device", chip,
        "-CommanderScript", script_path
    ]

    print("\n执行 J-Link 下载命令:")
    print(" ".join(cmd))

    try:
        result = subprocess.run(cmd, check=True, capture_output=True, text=True, cwd=Path.cwd())
        print("J-Link 下载成功!")
        if result.stdout:
            print("输出:", result.stdout)
        return True
    except subprocess.CalledProcessError as e:
        print(f"J-Link 下载失败: {e}")
        if e.stderr:
            print("错误:", e.stderr)
        if e.stdout:
            print("输出:", e.stdout)
        return False
    except Exception as e:
        print(f"执行 J-Link 失败: {e}")
        return False

def download_with_openocd(config, elf_file):
    """使用 OpenOCD 进行下载"""
    print("使用 OpenOCD 进行下载...")

    interface = config.get('download.interface', 'stlink')
    target = config.get('download.target', 'stm32f1x')
    speed = config.get('download.speed', '4000')

    cmd = [
        "openocd",
        "-f", f"interface/{interface}.cfg",
        "-f", f"target/{target}.cfg",
        "-c", f"adapter speed {speed}",
        "-c", f"program {elf_file} 0x00000000 verify reset exit"
    ]

    print("\n执行 OpenOCD 下载命令:")
    print(" ".join(cmd))

    try:
        result = subprocess.run(cmd, check=True, capture_output=True, text=True)
        print("OpenOCD 下载成功!")
        if result.stdout:
            print("输出:", result.stdout)
        return True
    except subprocess.CalledProcessError as e:
        print(f"OpenOCD 下载失败: {e}")
        if e.stderr:
            print("错误:", e.stderr)
        return False
    except Exception as e:
        print(f"执行 OpenOCD 失败: {e}")
        return False

def find_arm_toolchain():
    """查找 ARM GCC 工具链路径"""
    # 常见的 ARM GCC 工具链安装路径
    possible_paths = [
        r"C:\Program Files (x86)\Arm GNU Toolchain arm-none-eabi\13.2 Rel1\bin",
        r"C:\Program Files\Arm GNU Toolchain arm-none-eabi\13.2 Rel1\bin",
        r"C:\Program Files (x86)\GNU Arm Embedded Toolchain\10 2021.10\bin",
        r"C:\Program Files\GNU Arm Embedded Toolchain\10 2021.10\bin",
        r"C:\Program Files (x86)\GNU Tools ARM Embedded\9 2020-q2-update\bin",
        r"C:\Program Files\GNU Tools ARM Embedded\9 2020-q2-update\bin",
        r"C:\Tools\arm-none-eabi\bin",
        r"C:\arm-none-eabi\bin"
    ]

    # 检查 PATH 环境变量中的 arm-none-eabi-gcc
    try:
        result = subprocess.run(['where', 'arm-none-eabi-gcc'],
                              capture_output=True, text=True, shell=True)
        if result.returncode == 0:
            gcc_path = result.stdout.strip().split('\n')[0]
            toolchain_path = Path(gcc_path).parent
            return str(toolchain_path)
    except:
        pass

    # 检查预定义路径
    for path in possible_paths:
        gcc_path = Path(path) / "arm-none-eabi-gcc.exe"
        if gcc_path.exists():
            return path

    return None

def find_jlink_path():
    """查找 J-Link 安装路径"""
    jlink_paths = [
        r"C:\Program Files\SEGGER\JLink",
        r"C:\Program Files (x86)\SEGGER\JLink",
        r"C:\Program Files\SEGGER\JLink_V862",
        r"C:\Program Files (x86)\SEGGER\JLink_V862"
    ]

    for path in jlink_paths:
        jlink_gdb_server = Path(path) / "JLinkGDBServerCL.exe"
        if jlink_gdb_server.exists():
            return path

    return None

def create_vscode_debug_config(config):
    """创建或更新 VS Code 调试配置"""
    vscode_dir = Path('.vscode')
    vscode_dir.mkdir(exist_ok=True)
    launch_json_path = vscode_dir / 'launch.json'

    interface = config.get('download.interface', 'stlink')
    chip = config.get('project.chip', 'STM32F103C8')

    # 查找 ARM 工具链路径
    toolchain_path = find_arm_toolchain()
    if not toolchain_path:
        print("警告: 未找到 ARM GCC 工具链，调试可能无法正常工作")
        print("请确保已安装 ARM GCC 工具链并添加到 PATH 环境变量中")

    # 查找 J-Link 路径
    jlink_path = find_jlink_path()

    if interface == 'jlink':
        # J-Link 调试配置
        debug_config = {
            "name": "Debug STM32 (J-Link)",
            "cwd": "${workspaceFolder}",
            "executable": "./build/Temp32Project.elf",
            "request": "launch",
            "type": "cortex-debug",
            "runToEntryPoint": "main",
            "showDevDebugOutput": "raw",
            "servertype": "jlink",
            "device": chip,
            "interface": "swd",
            "swoConfig": {
                "enabled": True,
                "cpuFrequency": 72000000,
                "swoFrequency": 2000000,
                "source": "probe",
                "decoders": [
                    {
                        "type": "console",
                        "label": "ITM",
                        "port": 0
                    },
                    {
                        "type": "advanced",
                        "decoder": "dwt",
                        "label": "DWT",
                        "ports": [
                            {
                                "number": 1,
                                "type": "variable",
                                "name": "Variable Watch",
                                "enabled": True
                            }
                        ]
                    }
                ]
            },
            "rttConfig": {
                "enabled": True,
                "address": "auto",
                "decoders": [
                    {
                        "port": 0,
                        "type": "console",
                        "label": "RTT Terminal"
                    }
                ]
            },
            "graphConfig": [
                {
                    "label": "Real-time Variables",
                    "timespan": 10,
                    "maximum": 100,
                    "minimum": -100,
                    "variables": [
                        {
                            "name": "variable_name",
                            "address": "0x20000000"
                        }
                    ]
                }
            ],
            "preLaunchCommands": [
                "set mem inaccessible-by-default off",
                "enable breakpoint",
                "monitor SWO EnableTarget 72000000 2000000 1 0"
            ],
            "postLaunchCommands": [
                "monitor reset",
                "monitor halt"
            ],
            "liveWatch": {
                "enabled": True,
                "samplesPerSecond": 4
            }
        }

        # 如果找到了工具链路径，添加具体的路径配置
        if toolchain_path:
            toolchain_path_unix = toolchain_path.replace('\\', '/')
            debug_config["armToolchainPath"] = toolchain_path_unix
            debug_config["gdbPath"] = f"{toolchain_path_unix}/arm-none-eabi-gdb.exe"

        # 如果找到了 J-Link 路径，添加 J-Link GDB Server 路径
        if jlink_path:
            jlink_path_unix = jlink_path.replace('\\', '/')
            debug_config["jlinkGDBServerPath"] = f"{jlink_path_unix}/JLinkGDBServerCL.exe"
    else:
        # OpenOCD 调试配置
        target = config.get('download.target', 'stm32f1x')
        speed = config.get('download.speed', '4000')

        debug_config = {
            "name": f"Debug STM32 ({interface.upper()})",
            "cwd": "${workspaceFolder}",
            "executable": "./build/Temp32Project.elf",
            "request": "launch",
            "type": "cortex-debug",
            "runToEntryPoint": "main",
            "showDevDebugOutput": "raw",
            "servertype": "openocd",
            "configFiles": [
                f"interface/{interface}.cfg",
                f"target/{target}.cfg"
            ],
            "searchDir": [],
            "openOCDLaunchCommands": [
                f"adapter speed {speed}"
            ],
            "svdFile": "",
            "swoConfig": {
                "enabled": True,
                "cpuFrequency": 72000000,
                "swoFrequency": 2000000,
                "source": "probe",
                "decoders": [
                    {
                        "type": "console",
                        "label": "ITM",
                        "port": 0
                    },
                    {
                        "type": "advanced",
                        "decoder": "dwt",
                        "label": "DWT",
                        "ports": [
                            {
                                "number": 1,
                                "type": "variable",
                                "name": "Variable Watch",
                                "enabled": True
                            }
                        ]
                    }
                ]
            },
            "liveWatch": {
                "enabled": True,
                "samplesPerSecond": 4
            }
        }

        # 如果找到了工具链路径，添加具体的路径配置
        if toolchain_path:
            toolchain_path_unix = toolchain_path.replace('\\', '/')
            debug_config["armToolchainPath"] = toolchain_path_unix
            debug_config["gdbPath"] = f"{toolchain_path_unix}/arm-none-eabi-gdb.exe"

    # 读取现有配置或创建新配置
    launch_config = {
        "version": "0.2.0",
        "configurations": [debug_config]
    }

    if launch_json_path.exists():
        try:
            with open(launch_json_path, 'r', encoding='utf-8') as f:
                existing_config = json.load(f)

            # 检查是否已存在相同名称的配置
            existing_configs = existing_config.get('configurations', [])
            config_updated = False

            for i, cfg in enumerate(existing_configs):
                if cfg.get('name') == debug_config['name']:
                    existing_configs[i] = debug_config
                    config_updated = True
                    break

            if not config_updated:
                existing_configs.append(debug_config)

            launch_config = existing_config

        except Exception as e:
            print(f"读取现有 launch.json 失败: {e}")

    try:
        with open(launch_json_path, 'w', encoding='utf-8') as f:
            json.dump(launch_config, f, indent=2, ensure_ascii=False)
        print(f"已更新 VS Code 调试配置: {launch_json_path}")
        print(f"配置名称: {debug_config['name']}")
    except Exception as e:
        print(f"更新 VS Code 调试配置失败: {e}")

def main():
    # 加载项目配置
    config = ProjectConfig()

    if not config.config:
        print("未找到项目配置文件，请先运行 build.py 生成配置")
        return

    # 显示当前配置
    print("=== 当前下载配置 ===")
    print(f"芯片型号: {config.get('project.chip', 'Unknown')}")
    print(f"下载接口: {config.get('download.interface', 'stlink')}")
    print(f"目标配置: {config.get('download.target', 'stm32f1x')}")
    print(f"下载速度: {config.get('download.speed', '4000')}")

    # 询问是否修改配置
    # modify = input("\n是否修改下载配置? (y/N): ").strip().lower()
    # if modify in ['y', 'yes']:
    #     choose_config(config)
    #     config.save_config()
    #     print("下载配置已更新")

    # 获取下载接口
    interface = config.get('download.interface', 'stlink')

    # 根据接口类型更新配置文件
    if interface == 'jlink':
        # 对于 J-Link，不需要 idea.cfg，直接创建 J-Link 脚本
        print("检测到 J-Link 接口，将使用 J-Link 原生下载")
    else:
        # 对于其他接口，使用 OpenOCD，更新 idea.cfg
        update_idea_cfg(config)

    # 查找ELF文件
    elf_file = find_elf()
    if not elf_file:
        input("按回车键退出...")
        return

    elf_file = elf_file.replace("\\", "/")

    # 根据接口选择下载方法
    success = False
    if interface == 'jlink':
        success = download_with_jlink(config, elf_file)
    else:
        success = download_with_openocd(config, elf_file)

    # 创建或更新 VS Code 调试配置
    print("\n正在更新 VS Code 调试配置...")
    create_vscode_debug_config(config)

    print(f"\n下载{'成功' if success else '失败'}!")

    if not success:
        input("按回车键退出...")

if __name__ == "__main__":
    main()