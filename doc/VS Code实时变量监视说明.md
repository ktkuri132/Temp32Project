# VS Code 实时变量监视使用说明

## 概述

现在你的 VS Code 调试配置已经启用了实时变量监视功能，可以像 Keil 一样在程序运行过程中实时查看变量变化。

## 已启用的功能

### 1. SWO (Serial Wire Output) 实时追踪
- **启用状态**: ✅ 已启用
- **CPU 频率**: 72MHz (STM32F103C8)
- **SWO 频率**: 2MHz
- **功能**: 实时数据输出和变量追踪

### 2. Live Watch (实时监视)
- **启用状态**: ✅ 已启用
- **采样频率**: 4次/秒
- **功能**: 实时更新变量值

### 3. RTT (Real-Time Transfer) 支持
- **启用状态**: ✅ 已启用
- **功能**: 高速双向通信，用于 printf 输出和命令输入

### 4. DWT (Data Watchpoint and Trace) 单元
- **启用状态**: ✅ 已启用
- **功能**: 硬件级别的变量监视和追踪

## 使用方法

### 1. 启动调试
1. 在 VS Code 中按 `Ctrl+Shift+D` 打开调试视图
2. 选择 "Debug STM32 (J-Link)" 配置
3. 按 `F5` 开始调试

### 2. 添加变量到监视窗口
在调试过程中，有几种方式添加变量监视：

#### 方法一：通过变量窗口
1. 在调试视图中找到 "VARIABLES" 面板
2. 展开 "Locals" 查看局部变量
3. 右键点击变量 → "Add to Watch"

#### 方法二：手动添加
1. 在 "WATCH" 面板中点击 "+"
2. 输入变量名（如：`counter`, `temperature`, `sensor_value`）
3. 按回车添加

#### 方法三：选中代码添加
1. 在代码中选中变量名
2. 右键 → "Add to Watch"

### 3. 实时数据图表
配置中包含了图表功能，可以显示变量的实时波形：

1. 在调试时，如果变量被正确配置，会在调试面板看到图表视图
2. 可以同时监视多个变量的波形变化
3. 时间跨度：10秒，数值范围：-100 到 100

### 4. SWO 控制台输出
启用 SWO 后，你可以：

1. 在代码中使用 `printf` 进行调试输出
2. 输出会实时显示在 "ITM" 控制台中
3. 不需要暂停程序就能看到输出

## 代码配置

### 在代码中启用 SWO 输出
如果你想使用 `printf` 进行 SWO 输出，需要在代码中添加重定向：

```c
// 在 main.c 或专门的文件中添加
#include <stdio.h>

// 重定向 printf 到 SWO
int _write(int file, char *ptr, int len) {
    for (int i = 0; i < len; i++) {
        ITM_SendChar((*ptr++));
    }
    return len;
}

// 在 main 函数中初始化 SWO
int main(void) {
    // ... 其他初始化代码
    
    // 启用 SWO
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    ITM->LAR = 0xC5ACCE55;
    ITM->TER = 0x00000001;
    ITM->TCR = 0x0001000D;
    
    // 现在可以使用 printf 输出到 SWO
    printf("SWO Debug Output Enabled\\n");
    
    while(1) {
        // 你的主循环代码
        printf("Counter: %d\\n", counter);
        delay(1000);
    }
}
```

### 添加变量监视点
在你想监视的变量附近添加注释，方便调试：

```c
volatile int sensor_reading = 0;     // 传感器读数 - 监视此变量
volatile float temperature = 25.0;   // 温度值 - 实时监视
volatile int system_state = 0;       // 系统状态 - 添加到监视窗口
```

## 调试面板功能

### 1. Variables 面板
- **Locals**: 当前函数的局部变量
- **Globals**: 全局变量
- **Registers**: CPU 寄存器状态

### 2. Watch 面板
- 显示你手动添加的监视变量
- 实时更新数值（程序运行时）
- 支持表达式计算

### 3. Call Stack 面板
- 显示函数调用栈
- 可以点击切换到不同的栈帧

### 4. Breakpoints 面板
- 管理所有断点
- 可以临时禁用/启用断点

## 实时监视的优势

### 相比传统调试的优势：
1. **程序不中断**: 变量值实时更新，程序继续运行
2. **高频采样**: 每秒4次采样，捕获快速变化
3. **图形化显示**: 波形图直观显示数据变化趋势
4. **多变量监视**: 同时监视多个变量
5. **SWO 输出**: 类似串口输出但不占用 UART 资源

### 适用场景：
- **PID 控制器调试**: 实时查看 PID 参数和输出
- **传感器数据监测**: 实时波形显示
- **状态机调试**: 监视状态变量变化
- **算法优化**: 观察算法中间结果
- **性能分析**: 监视执行时间和资源使用

## 故障排除

### 1. 实时监视不工作
- 确保程序在运行（不是暂停状态）
- 检查变量是否在当前作用域内
- 确认变量没有被编译器优化掉（使用 `volatile` 关键字）

### 2. SWO 输出看不到
- 检查硬件连接，确保 SWO 引脚正确连接
- 确认时钟频率设置正确（72MHz）
- 检查代码中是否正确初始化了 SWO

### 3. 图表不显示
- 确保变量值在配置的范围内（-100 到 100）
- 检查变量类型是否为数值类型
- 尝试手动配置图表变量地址

### 4. 性能影响
- 如果影响程序性能，可以降低采样频率
- 减少监视的变量数量
- 使用条件断点代替持续监视

## 高级配置

### 自定义图表配置
你可以修改 `launch.json` 中的 `graphConfig` 来自定义图表：

```json
"graphConfig": [
    {
        "label": "Temperature Sensor",
        "timespan": 30,
        "maximum": 100,
        "minimum": 0,
        "variables": [
            {
                "name": "temperature",
                "address": "0x20000100"
            }
        ]
    }
]
```

### 多通道 SWO 配置
可以配置多个 ITM 端口用于不同类型的输出：

```c
// 通道 0: 普通调试信息
ITM_SendChar_Ch0('H');

// 通道 1: 错误信息  
ITM_SendChar_Ch1('E');

// 通道 2: 性能数据
ITM_SendChar_Ch2('P');
```

现在你的 VS Code 调试环境已经具备了像 Keil 一样的实时变量监视功能！
