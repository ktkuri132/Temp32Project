# ARM架构 通用模板项目

实验室 Arm架构MCU 开发模板项目，支持多芯片、自动化构建和完整的工具链配置。

## ✨ 特性

- 🔧 **多芯片支持** - 支持 STM32F1/F4 系列，易于扩展
- 🚀 **自动化环境配置** - 一键检测和安装开发工具链
- 📦 **模块化设计** - BSP/Device/Driver 分层架构
- 🔄 **灵活的设备配置** - 通过 JSON 配置启用/禁用设备和协议
- 🛠️ **多调试器支持** - ST-Link、J-Link、CMSIS-DAP
- 📝 **VS Code 集成** - 完整的任务、调试配置

---

## 🚀 快速开始

### 1. 环境配置（首次使用）

```bash
# 运行环境配置工具
python tool/env_setup.py
```

工具会自动：
- ✅ 检测 ARM-GCC、OpenOCD、J-Link、CMake、Ninja
- ✅ 下载并安装缺失的工具
- ✅ 安装必要的 Python 包
- ✅ 生成 VS Code 任务配置
- ✅ 推荐必要的 VS Code 插件

### 2. 项目配置

```bash
# 运行构建配置（选择芯片、设备等）
python tool/build.py
```

### 3. 编译项目

```bash
# CMake 配置
cmake -B build -G Ninja

# 编译
cmake --build build -j
```

### 4. 烧录程序

```bash
python tool/download.py
```

---

## 📁 项目结构

```
├── app/                    # 应用层代码
│   ├── main.c             # 主程序入口
│   ├── init.c             # 系统初始化
│   └── control.c          # 控制逻辑
├── BSP/                    # 板级支持包
│   ├── CMSIS/             # ARM CMSIS 核心
│   ├── stm32f1/           # STM32F1 系列支持
│   └── stm32f4/           # STM32F4 系列支持
├── Device/                 # 外设设备驱动
│   ├── config.h           # 设备配置头文件（自动生成）
│   ├── mpu6050/           # MPU6050 驱动
│   ├── bmp280/            # BMP280 驱动
│   └── ...
├── Driver_Framework/       # 驱动框架层
│   ├── i2c/               # I2C 驱动抽象
│   ├── spi/               # SPI 驱动抽象
│   ├── display/           # 显示驱动
│   └── shell/             # 调试Shell
├── Control/                # 控制算法
│   ├── pid.c/h            # PID 控制器
│   └── filter.c/h         # 滤波器
├── Middleware/             # 中间件
├── tool/                   # 构建工具
│   ├── env_setup.py       # 环境配置工具
│   ├── build.py           # 构建配置生成
│   ├── download.py        # 烧录工具
│   ├── config_generator.py # 配置生成器
│   ├── project_config.json # 项目配置文件
│   └── watch_config.py    # 配置监视服务
├── doc/                    # 项目文档
└── .vscode/                # VS Code 配置
```

---

## ⚙️ 配置说明

### 项目配置文件 (`tool/project_config.json`)

```json
{
  "chip": {
    "series": "stm32f1",
    "model": "f103",
    "target": "STM32F103C8"
  },
  "devices": {
    "mpu6050": {
      "enabled": true,
      "protocol": "soft_i2c",
      "description": "6轴运动传感器"
    },
    "bmp280": {
      "enabled": false,
      "protocol": "hard_i2c",
      "description": "气压传感器"
    }
  }
}
```

### 协议选项

| 协议 | 宏定义 | 说明 |
|------|--------|------|
| `soft_i2c` | `__SOFTI2C_` | 软件模拟 I2C |
| `hard_i2c` | `__HARDI2C_` | 硬件 I2C |
| `soft_spi` | `__SOFTSPI_` | 软件模拟 SPI |
| `hard_spi` | `__HARDSPI_` | 硬件 SPI |

---

## 🛠️ 工具链要求

### 必需工具

| 工具 | 版本 | 说明 |
|------|------|------|
| ARM-GCC | 13.2+ | ARM 交叉编译器 |
| CMake | 3.20+ | 构建系统 |
| Python | 3.8+ | 构建脚本 |

### 可选工具

| 工具 | 说明 |
|------|------|
| OpenOCD | ST-Link/CMSIS-DAP 调试 |
| J-Link | SEGGER J-Link 调试 |
| Ninja | 快速构建（推荐） |

### VS Code 插件

运行 `python tool/env_setup.py` 后，VS Code 会自动推荐安装：

- **C/C++** - C/C++ 语言支持
- **Cortex-Debug** - ARM 调试
- **CMake Tools** - CMake 集成
- **Python** - Python 支持

---

## 📋 VS Code 任务

在 VS Code 中按 `Ctrl+Shift+P` → `Tasks: Run Task`：

| 任务 | 说明 |
|------|------|
| `$(tools)构建配置` | 生成构建文件 |
| `$(gear)CMake配置` | 运行 CMake 配置 |
| `$(package)编译项目` | 编译项目 |
| `$(zap)烧录` | 烧录程序 |
| `$(settings-gear)环境配置` | 运行环境配置 |
| `$(eye)监视配置` | 监视配置文件变化 |

---

## 🐛 调试

### J-Link 调试

1. 连接 J-Link 调试器
2. 在 VS Code 调试面板选择 "Debug STM32 (J-Link)"
3. 按 F5 开始调试

### ST-Link 调试

1. 连接 ST-Link 调试器
2. 选择 "Debug STM32 (ST-LINK)"
3. 按 F5 开始调试

### 实时变量监视

调试配置已启用实时变量监视功能：
- **Live Watch** - 实时更新变量值
- **SWO 输出** - printf 调试信息
- **RTT 支持** - 高速数据传输

---

## 📚 详细文档

- [构建系统文档](doc/build_system/) - 构建系统详细说明
- [驱动框架文档](doc/driver_framework/) - 驱动框架使用指南
- [调试配置文档](doc/debugging.md) - 调试器配置说明
- [设备驱动文档](doc/device_hal.md) - 设备驱动 HAL 架构
- [链接脚本文档](doc/linker/) - 链接脚本配置

---

## 🔧 常见问题

### Q: 找不到 ARM-GCC？

运行 `python tool/env_setup.py`，工具会自动下载安装。

### Q: 编译报错找不到头文件？

确保运行过 `python tool/build.py` 生成构建配置。

### Q: 烧录失败？

1. 检查调试器连接
2. 运行 `python tool/download.py` 重新配置接口

---

## 📄 许可证

MIT License

---

## 🤝 贡献

欢迎提交 Issue 和 Pull Request！

## 提示
当前版本仅支持SMT32F1，F4部分芯片型号，一些组件的功能也在优化和调试