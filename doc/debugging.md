# 调试配置指南

本文档介绍如何配置 J-Link、ST-Link 等调试器，以及 VS Code 实时变量监视功能。

---

## 目录

1. [调试器概述](#调试器概述)
2. [J-Link 配置](#j-link-配置)
3. [ST-Link 配置](#st-link-配置)
4. [实时变量监视](#实时变量监视)
5. [SWO 调试输出](#swo-调试输出)
6. [故障排除](#故障排除)

---

## 调试器概述

### 支持的调试器

| 调试器 | 优点 | 适用场景 |
|--------|------|----------|
| J-Link | 速度快、功能全 | 专业开发 |
| ST-Link | 成本低、集成好 | 入门学习 |
| CMSIS-DAP | 开源、兼容好 | 通用开发 |

### 自动配置

运行 `python tool/download.py` 后，脚本会：
1. 检测配置的调试接口类型
2. 自动生成对应的调试配置
3. 更新 `.vscode/launch.json`

---

## J-Link 配置

### 安装 J-Link 驱动

从 SEGGER 官网下载：https://www.segger.com/downloads/jlink/

### 自动检测路径

脚本会自动检测以下位置：
- `C:\Program Files\SEGGER\JLink`
- `C:\Program Files (x86)\SEGGER\JLink`
- `C:\Program Files\SEGGER\JLink_V*`

### VS Code 调试配置

```json
{
  "name": "Debug STM32 (J-Link)",
  "cwd": "${workspaceFolder}",
  "executable": "./build/General_template_Project.elf",
  "request": "launch",
  "type": "cortex-debug",
  "runToEntryPoint": "main",
  "servertype": "jlink",
  "device": "STM32F103C8",
  "interface": "swd",
  "armToolchainPath": "C:/Program Files (x86)/Arm GNU Toolchain arm-none-eabi/13.2 Rel1/bin",
  "jlinkGDBServerPath": "C:/Program Files/SEGGER/JLink/JLinkGDBServerCL.exe",
  "swoConfig": {
    "enabled": true,
    "cpuFrequency": 72000000,
    "swoFrequency": 2000000
  }
}
```

### J-Link 下载脚本

自动生成的 `build/flash_simple.jlink`：

```
connect
device STM32F103C8
si SWD
speed 20000
halt
erase
loadfile build\General_template_Project.hex
r
g
qc
```

---

## ST-Link 配置

### OpenOCD 配置

自动生成的 `build/idea.cfg`：

```
source [find interface/stlink.cfg]
source [find target/stm32f1x.cfg]
adapter speed 4000
```

### VS Code 调试配置

```json
{
  "name": "Debug STM32 (ST-LINK)",
  "cwd": "${workspaceFolder}",
  "executable": "./build/General_template_Project.elf",
  "request": "launch",
  "type": "cortex-debug",
  "runToEntryPoint": "main",
  "servertype": "openocd",
  "configFiles": [
    "interface/stlink.cfg",
    "target/stm32f1x.cfg"
  ],
  "searchDir": ["C:/openocd/share/openocd/scripts"]
}
```

---

## 实时变量监视

### 功能说明

VS Code + Cortex-Debug 支持实时变量监视，类似 Keil 的 Watch 功能：

| 功能 | 说明 |
|------|------|
| Live Watch | 程序运行时实时更新变量值 |
| SWO 输出 | printf 调试信息实时显示 |
| RTT 支持 | 高速双向通信 |
| DWT 追踪 | 硬件级变量监视 |

### 使用方法

1. **启动调试**：按 F5
2. **添加监视变量**：
   - 在代码中选中变量 → 右键 → "Add to Watch"
   - 或在 Watch 面板点击 "+" 手动添加
3. **查看实时变化**：程序运行时变量值自动更新

### 调试配置中的监视设置

```json
{
  "liveWatch": {
    "enabled": true,
    "samplesPerSecond": 4
  },
  "rttConfig": {
    "enabled": true,
    "address": "auto",
    "decoders": [
      {"port": 0, "type": "console"}
    ]
  }
}
```

---

## SWO 调试输出

### 代码配置

在代码中添加 SWO 重定向：

```c
#include <stdio.h>

// 重定向 printf 到 SWO
int _write(int file, char *ptr, int len) {
    for (int i = 0; i < len; i++) {
        ITM_SendChar((*ptr++));
    }
    return len;
}

// 初始化 SWO
void SWO_Init(void) {
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    ITM->LAR = 0xC5ACCE55;
    ITM->TER = 0x00000001;
    ITM->TCR = 0x0001000D;
}

int main(void) {
    SWO_Init();
    printf("SWO Debug Ready\n");

    while (1) {
        printf("Counter: %d\n", counter++);
        HAL_Delay(1000);
    }
}
```

### 配置参数

| 参数 | STM32F103 | STM32F407 |
|------|-----------|-----------|
| CPU 频率 | 72 MHz | 168 MHz |
| SWO 频率 | 2 MHz | 4 MHz |

---

## 故障排除

### 问题：找不到调试器

**解决方案：**
1. 检查 USB 连接
2. 安装/更新驱动
3. 运行 `python tool/env_setup.py --verify`

### 问题：下载失败

**解决方案：**
1. 检查目标芯片电源
2. 确认 SWD 连接正确（SWDIO、SWCLK、GND）
3. 降低调试速度（修改 `adapter speed`）

### 问题：变量监视不更新

**解决方案：**
1. 确保变量声明为 `volatile`
2. 检查优化级别（建议 `-O0` 或 `-Og`）
3. 确认 Live Watch 已启用

### 问题：SWO 无输出

**解决方案：**
1. 确认 SWO 引脚连接（通常是 PB3）
2. 检查 CPU 和 SWO 频率配置
3. 确保 ITM 已正确初始化

---

## 参考链接

- [SEGGER J-Link](https://www.segger.com/products/debug-probes/j-link/)
- [OpenOCD 官网](https://openocd.org/)
- [Cortex-Debug 扩展](https://marketplace.visualstudio.com/items?itemName=marus25.cortex-debug)
