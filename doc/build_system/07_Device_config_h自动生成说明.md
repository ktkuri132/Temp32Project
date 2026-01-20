# Device/config.h 自动生成说明

## 概述

`Device/config.h` 文件现在由构建系统自动生成，集中管理所有设备驱动的条件编译宏定义。

## 双重宏系统

为了兼容性，项目现在采用**双重宏系统**：

### 1. 新宏：USE_DEVICE_XXX

用于设备驱动源文件的条件编译保护：

```c
// Device/bmp280/bmp280.c
#ifdef USE_DEVICE_BMP280
#include "bmp280.h"
// ... 驱动实现代码
#endif // USE_DEVICE_BMP280
```

**特点：**
- ✅ 在 CMakeLists.txt 中通过 `add_definitions()` 定义
- ✅ 由 project_config.json 中的 `"enabled": true/false` 控制
- ✅ 用于驱动文件的条件编译（.c和.h文件）

### 2. 旧宏：设备名称宏

用于应用层代码的功能判断：

```c
// Device/config.h (自动生成)
#define BMP280               /* bmp280 驱动已启用 */
#define HMC5883L             /* hmc588 驱动已启用 */
#define MPU6050              /* mpu6050 驱动已启用 */
#define SH1106               /* sh1106 驱动已启用 */
#define SSD1306              /* ssd1306 驱动已启用 */
```

**特点：**
- ✅ 在 Device/config.h 中定义
- ✅ 用于应用层代码的功能判断
- ✅ 保持向后兼容性

## 宏定义映射关系

| 设备名称 | 新宏 (USE_DEVICE_XXX) | 旧宏 (设备名) | 说明 |
|---------|---------------------|-------------|------|
| bmp280 | USE_DEVICE_BMP280 | BMP280 | 气压温度传感器 |
| hmc588 | USE_DEVICE_HMC588 | HMC5883L | 三轴磁力计 |
| mpu6050 | USE_DEVICE_MPU6050 | MPU6050 | 六轴IMU |
| sh1106 | USE_DEVICE_SH1106 | SH1106 | OLED显示屏 |
| ssd1306 | USE_DEVICE_SSD1306 | SSD1306 | OLED显示屏 |
| st7789 | USE_DEVICE_ST7789 | ST7789 | TFT彩屏 |

## 工作流程

### 1. 配置阶段

编辑 `tool/project_config.json`：

```json
{
  "devices": {
    "bmp280": {
      "enabled": true,  // ← 启用设备
      "sources": [...],
      "include_dirs": [...]
    },
    "ssd1306": {
      "enabled": false,  // ← 禁用设备
      "sources": [...],
      "include_dirs": [...]
    }
  }
}
```

### 2. 生成阶段

运行 `python tool\build.py` 后：

**a) 生成 CMakeLists.txt**

```cmake
# 仅为启用的设备定义宏
add_definitions(-DUSE_DEVICE_BMP280)  # 启用bmp280设备驱动
add_definitions(-DUSE_DEVICE_HMC588)  # 启用hmc588设备驱动
add_definitions(-DUSE_DEVICE_MPU6050) # 启用mpu6050设备驱动
add_definitions(-DUSE_DEVICE_SH1106)  # 启用sh1106设备驱动
# USE_DEVICE_SSD1306 未定义 - ssd1306已禁用
```

**b) 生成 Device/config.h**

```c
/*============================ 设备驱动宏定义 ============================*/
#define BMP280               /* bmp280 驱动已启用 */
#define HMC5883L             /* hmc588 驱动已启用 */
#define MPU6050              /* mpu6050 驱动已启用 */
#define SH1106               /* sh1106 驱动已启用 */
// SSD1306 未定义 - 已禁用

/*============================ 设备驱动配置 ============================*/
/* SH1106 OLED显示屏配置 */
#ifdef SH1106
#include <lcd/df_lcd.h>
// ... SH1106特定配置
#endif

/* BMP280 气压温度传感器配置 */
#ifdef BMP280
#include <i2c/df_iic.h>
#define i2c_dev i2c1_bus
#endif
```

**c) 生成 Device/CMakeLists.txt**

```cmake
# 仅包含启用的设备源文件
set(BMP280_SOURCES bmp280/bmp280.c)
set(MPU6050_SOURCES mpu6050/inv_mpu.c ...)
# ssd1306 源文件不包含
```

### 3. 编译阶段

**编译器行为：**

```bash
# 驱动文件条件编译
gcc ... -DUSE_DEVICE_BMP280 -DUSE_DEVICE_MPU6050 ... Device/bmp280/bmp280.c
# ↑ USE_DEVICE_BMP280 已定义，bmp280.c 内部代码会被编译

gcc ... Device/ssd1306/ssd1306.c
# ↑ USE_DEVICE_SSD1306 未定义，ssd1306.c 内部代码会被跳过（但文件仍在编译列表中）
```

**注意：** 实际上禁用的设备文件不会出现在 Device/CMakeLists.txt 中，所以不会被编译。

## 使用场景

### 场景1：驱动文件内部保护（新宏）

```c
// Device/bmp280/bmp280.c
#ifdef USE_DEVICE_BMP280  // ← 新宏，由CMake定义

#include "bmp280.h"
#include <string.h>

// BMP280驱动实现
void BMP280_Init(void) {
    // ...
}

#endif // USE_DEVICE_BMP280
```

### 场景2：应用层功能判断（旧宏）

```c
// app/main.c
#include <config.h>

void sensors_init(void) {
#ifdef BMP280  // ← 旧宏，在config.h中定义
    BMP280_Init();
    printf("BMP280 initialized\n");
#endif

#ifdef SSD1306  // ← 若禁用，这个宏不会定义
    SSD1306_Init();
    printf("SSD1306 initialized\n");
#endif

#ifdef MPU6050
    MPU6050_Init();
    printf("MPU6050 initialized\n");
#endif
}
```

### 场景3：驱动头文件保护（新宏）

```c
// Device/bmp280/bmp280.h
#ifndef __BMP280_H
#define __BMP280_H

#ifdef USE_DEVICE_BMP280  // ← 新宏

#include <stdint.h>
#include <config.h>

// BMP280相关定义和声明
typedef struct {
    float temperature;
    float pressure;
} BMP280_Data_t;

void BMP280_Init(void);
float BMP280_ReadTemperature(void);

#endif // USE_DEVICE_BMP280
#endif // __BMP280_H
```

## 为什么需要两种宏？

### USE_DEVICE_XXX（新宏）的作用

1. **源文件级别的条件编译**
   - 保护整个驱动源文件不被编译
   - 即使文件存在，代码也不会被包含到最终固件中

2. **CMake层面的控制**
   - 在编译命令行通过 `-D` 参数传递
   - 构建系统直接控制，无需修改代码

3. **干净的依赖关系**
   - 禁用设备时，相关的源文件代码完全不参与编译
   - 减少编译时间和二进制体积

### 设备名宏（旧宏）的作用

1. **应用层的灵活性**
   - 应用代码可以通过 `#include <config.h>` 获取设备启用状态
   - 简洁的条件判断语法

2. **向后兼容性**
   - 保持原有代码风格
   - 不需要大规模修改现有应用代码

3. **运行时逻辑**
   - 适合在初始化函数中判断哪些设备需要初始化
   - 方便打印设备信息

## 自动生成机制

### config_generator.py

新增的配置生成器模块：

```python
class ConfigGenerator:
    def generate_device_config_header(self):
        """生成Device/config.h头文件内容"""
        config = self.load_config()
        devices = config.get('devices', {})

        # 过滤出启用的设备
        enabled_devices = {
            name: info for name, info in devices.items()
            if info.get('enabled', False)
        }

        # 生成宏定义
        for device_name in sorted(enabled_devices.keys()):
            macro_name = device_macro_map.get(device_name)
            output += f"#define {macro_name} /* {device_name} 驱动已启用 */\n"
```

### 集成到build.py

```python
from config_generator import ConfigGenerator

class ProjectBuilder:
    def build(self, chip_name=None, board_name=None):
        # ... 生成CMakeLists.txt

        # 生成Device/config.h
        self.generate_device_config_header()

    def generate_device_config_header(self):
        """生成Device/config.h配置文件"""
        config_gen = ConfigGenerator(self.project_root)
        config_gen.write_config_header()
```

## 完整示例

### 禁用ssd1306显示屏

**1. 修改配置**

```json
// tool/project_config.json
{
  "devices": {
    "ssd1306": {
      "enabled": false,  // ← 改为false
      "sources": ["ssd1306/fonts.c", "ssd1306/ssd1306.c"],
      "include_dirs": ["Device/ssd1306"]
    }
  }
}
```

**2. 重新生成**

```bash
python tool\build.py
```

输出：
```
使用配置: 4 个设备 (bmp280, hmc588, mpu6050, sh1106)，共 6 个源文件
# 注意：少了ssd1306，从5个设备减少到4个
✓ 生成设备配置文件: Device/config.h
```

**3. 检查生成结果**

**CMakeLists.txt:**
```cmake
add_definitions(-DUSE_DEVICE_BMP280)
add_definitions(-DUSE_DEVICE_HMC588)
add_definitions(-DUSE_DEVICE_MPU6050)
add_definitions(-DUSE_DEVICE_SH1106)
# 没有 USE_DEVICE_SSD1306
```

**Device/config.h:**
```c
#define BMP280
#define HMC5883L
#define MPU6050
#define SH1106
// 没有 #define SSD1306
```

**Device/CMakeLists.txt:**
```cmake
set(BMP280_SOURCES bmp280/bmp280.c)
set(MPU6050_SOURCES mpu6050/inv_mpu.c ...)
set(SH1106_SOURCES sh1106/fonts.c sh1106/sh1106.c)
# 没有 SSD1306_SOURCES
```

**4. 应用层代码行为**

```c
// app/main.c
void display_init(void) {
#ifdef SH1106
    SH1106_Init();  // ✅ 会执行
#endif

#ifdef SSD1306
    SSD1306_Init();  // ❌ 不会执行，因为SSD1306未定义
#endif
}
```

## 优势总结

### ✅ 配置集中化
- 所有设备配置在 project_config.json
- config.h 自动生成，无需手动维护

### ✅ 两层保护
- 驱动层：USE_DEVICE_XXX 保护源文件
- 应用层：设备名宏用于功能判断

### ✅ 完全自动化
- 修改JSON配置
- 运行build.py
- 自动更新所有相关文件

### ✅ 向后兼容
- 保留原有宏名称（BMP280、MPU6050等）
- 不影响现有应用代码

### ✅ 灵活裁剪
- 禁用设备立即生效
- 减少编译时间和固件体积

## 注意事项

### ⚠️ 不要手动编辑config.h

```c
// Device/config.h
/**
 * @warning 此文件会在每次构建时重新生成，手动修改将会丢失
 */
```

所有修改应在 `tool/project_config.json` 中进行。

### ⚠️ 宏名称对应关系

注意设备目录名和宏名称的对应关系：
- `hmc588` 目录 → `HMC5883L` 宏（历史原因）
- 其他设备：目录名的大写形式

### ⚠️ 依赖关系检查

禁用设备前，确保应用代码没有硬编码调用：

```c
// 不推荐：硬编码调用
void bad_init(void) {
    BMP280_Init();  // 如果BMP280被禁用，会编译失败
}

// 推荐：条件编译
void good_init(void) {
#ifdef BMP280
    BMP280_Init();
#endif
}
```

## 文件关系图

```
tool/project_config.json
        ↓
   [build.py]
        ↓
    ┌───┴───┐
    ↓       ↓
CMakeLists.txt   Device/config.h
(USE_DEVICE_XXX)  (BMP280, MPU6050等)
    ↓               ↓
Device/*.c      app/*.c
(驱动源码)      (应用代码)
```

## 总结

新的宏管理系统实现了：

1. **双重保护机制**
   - 驱动层：USE_DEVICE_XXX（CMake定义）
   - 应用层：设备名宏（config.h定义）

2. **完全自动化**
   - 一个JSON配置
   - 自动生成所有必需的宏定义

3. **灵活可控**
   - 启用/禁用设备立即生效
   - 无需手动修改多个文件

4. **向后兼容**
   - 保留原有宏名称
   - 应用代码无需大改

这套系统让设备驱动管理更加简单、安全、高效！
