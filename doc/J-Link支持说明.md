# J-Link 下载和调试支持说明

## 概述

`download.py` 脚本现在支持 J-Link 和 OpenOCD 两种下载方式，并会根据配置文件中的接口类型自动选择合适的下载方法。

## 主要特性

### 1. 智能接口检测
- 自动检测配置文件中的下载接口类型
- J-Link 接口：使用 J-Link 原生下载工具，效率更高
- 其他接口（ST-Link、CMSIS-DAP 等）：使用 OpenOCD

### 2. J-Link 支持
- 自动生成 J-Link 下载脚本 (`build/flash_simple.jlink`)
- 支持多个 J-Link 安装路径的自动检测
- 使用 J-Link 原生命令行工具，避免 OpenOCD 的效率问题

### 3. VS Code 调试配置
- 自动生成或更新 `.vscode/launch.json` 调试配置
- J-Link 接口：生成专门的 J-Link 调试配置
- 其他接口：生成 OpenOCD 调试配置

## 使用方法

### 1. 运行下载脚本
```bash
python download.py
```

### 2. 配置选择
脚本会询问是否修改下载配置，可以选择：
- 1. ST-Link
- 2. J-Link
- 3. CMSIS-DAP
- 4. XDS110

### 3. 自动配置
脚本会根据选择的接口：
- 生成相应的配置文件
- 更新 VS Code 调试配置
- 执行下载操作

## 生成的文件

### 1. J-Link 脚本 (`build/flash_simple.jlink`)
```
connect
device STM32F103C8
si SWD
speed 20000
halt
erase
loadfile build\Temp32Project.hex
r
g
qc
```

### 2. OpenOCD 配置 (`build/idea.cfg`)
```
source [find interface/stlink.cfg]
source [find target/stm32f1x.cfg]
adapter speed 4000
```

### 3. VS Code 调试配置 (`.vscode/launch.json`)

#### J-Link 配置：
```json
{
  "name": "Debug STM32 (J-Link)",
  "cwd": "${workspaceFolder}",
  "executable": "./build/Temp32Project.elf",
  "request": "launch",
  "type": "cortex-debug",
  "runToEntryPoint": "main",
  "showDevDebugOutput": "raw",
  "servertype": "jlink",
  "device": "STM32F103C8",
  "interface": "swd",
  "jlinkscript": "./build/flash_simple.jlink"
}
```

#### OpenOCD 配置：
```json
{
  "name": "Debug STM32 (ST-LINK)",
  "cwd": "${workspaceFolder}",
  "executable": "./build/Temp32Project.elf",
  "request": "launch",
  "type": "cortex-debug",
  "runToEntryPoint": "main",
  "showDevDebugOutput": "raw",
  "servertype": "openocd",
  "configFiles": [
    "interface/stlink.cfg",
    "target/stm32f1x.cfg"
  ],
  "openOCDLaunchCommands": [
    "adapter speed 4000"
  ]
}
```

## J-Link 安装路径检测

脚本会自动检测以下路径中的 J-Link 安装：
- `C:\Program Files\SEGGER\JLink\JLink.exe`
- `C:\Program Files (x86)\SEGGER\JLink\JLink.exe`
- `C:\Program Files\SEGGER\JLink_V862\JLink.exe`
- `C:\Program Files (x86)\SEGGER\JLink_V862\JLink.exe`

## 优势

### J-Link 相比 OpenOCD 的优势：
1. **更高效率**：J-Link 原生工具下载速度更快
2. **更稳定**：减少中间层，直接与硬件通信
3. **更好的调试支持**：J-Link 调试服务器功能更完善
4. **官方支持**：SEGGER 官方工具，兼容性更好

### 脚本特性：
1. **自动检测**：根据配置自动选择下载方式
2. **配置管理**：统一的配置文件管理
3. **调试集成**：自动生成 VS Code 调试配置
4. **错误处理**：完善的错误提示和处理

## 注意事项

1. 使用 J-Link 前请确保已安装 J-Link 软件包
2. 确保目标设备连接正确
3. 如果 J-Link 检测失败，脚本会提示可能的安装路径
4. 配置文件保存在 `build/project_config.json` 中

## 故障排除

### J-Link 未找到
- 检查 J-Link 软件是否已安装
- 确认安装路径是否在检测列表中
- 手动指定 J-Link 可执行文件路径

### 下载失败
- 检查设备连接
- 确认设备型号配置正确
- 查看错误输出信息

### 调试配置不生效
- 确保安装了 Cortex-Debug 扩展
- 检查配置文件语法
- 重启 VS Code
