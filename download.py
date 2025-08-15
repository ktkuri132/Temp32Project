import os
import glob
import subprocess
import json
from pathlib import Path

class ProjectConfig:
    """项目配置管理类"""

    CONFIG_FILE = "project_config.json"

    def __init__(self, project_root="."):
        self.project_root = Path(project_root)
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
        with open('idea.cfg', 'w', encoding='utf-8') as f:
            f.write(idea_cfg_content)
        print(f"已更新 idea.cfg 文件")
    except Exception as e:
        print(f"更新 idea.cfg 失败: {e}")

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

    # 总是更新idea.cfg文件（基于当前配置）
    update_idea_cfg(config)

    # 查找ELF文件
    elf_file = find_elf()
    if not elf_file:
        input("按回车键退出...")
        return

    elf_file = elf_file.replace("\\", "/")

    # 构建下载命令
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

    print("\n执行下载命令:")
    print(" ".join(cmd))

    try:
        result = subprocess.run(cmd, check=True, capture_output=True, text=True)
        print("下载成功!")
        if result.stdout:
            print("输出:", result.stdout)
    except subprocess.CalledProcessError as e:
        print(f"下载失败: {e}")
        if e.stderr:
            print("错误:", e.stderr)
    except Exception as e:
        print(f"执行失败: {e}")

if __name__ == "__main__":
    main()