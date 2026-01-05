# 设备驱动 HAL 架构

本文档介绍设备驱动的硬件抽象层（HAL）架构设计。

---

## 目录

1. [架构概述](#架构概述)
2. [HAL 接口设计](#hal-接口设计)
3. [设备初始化](#设备初始化)
4. [使用示例](#使用示例)
5. [扩展新设备](#扩展新设备)

---

## 架构概述

### 设计目标

1. ✅ 消除对底层硬件的直接依赖
2. ✅ 支持软件/硬件 I2C/SPI 的无缝切换
3. ✅ 提高代码的可移植性和可测试性
4. ✅ 统一设备驱动的接口规范

### 架构对比

#### 旧架构（宏函数）

```
Device驱动 -> 宏选择 (#ifdef __SOFTI2C__) -> 直接调用底层API
```

**缺点：**
- 编译时绑定，无法动态切换
- 宏条件分支遍布代码
- 不利于单元测试

#### 新架构（函数指针 HAL）

```
Device驱动 -> HAL接口（函数指针） -> HAL适配器 -> 底层API
```

**优点：**
- 运行时绑定，灵活切换
- 代码更清晰，无宏分支
- 支持依赖注入和模拟测试

---

## HAL 接口设计

### I2C HAL 接口

```c
// Device/device_hal.h

typedef struct {
    // 写单字节
    int (*write_byte)(uint8_t dev_addr, uint8_t reg_addr, uint8_t data);

    // 读单字节
    int (*read_byte)(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data);

    // 读多字节
    int (*read_bytes)(uint8_t dev_addr, uint8_t reg_addr,
                      uint8_t len, uint8_t *buf);

    // 写多字节
    int (*write_bytes)(uint8_t dev_addr, uint8_t reg_addr,
                       uint8_t len, const uint8_t *buf);

    // 延时函数
    void (*delay_ms)(uint32_t ms);
    void (*delay_us)(uint32_t us);

    // 状态
    bool initialized;
    void *user_data;
} device_i2c_hal_t;
```

### SPI HAL 接口

```c
typedef struct {
    // 读写操作
    int (*read_write)(uint8_t *tx_buf, uint8_t *rx_buf, uint16_t len);

    // 片选控制
    void (*cs_low)(void);
    void (*cs_high)(void);

    // 延时
    void (*delay_us)(uint32_t us);

    bool initialized;
    void *user_data;
} device_spi_hal_t;
```

---

## 设备初始化

### 统一初始化接口

```c
// Device/device_init.h

// 初始化 HAL 接口（根据 config.h 配置）
void Device_HAL_Init(void);

// 初始化所有启用的设备
void Device_Init_All(void);

// 单独设备初始化
int Device_BMP280_Init(void);
int Device_MPU6050_Init(void);
int Device_HMC5883L_Init(void);
```

### 配置驱动的协议选择

在 `tool/project_config.json` 中配置：

```json
{
  "devices": {
    "mpu6050": {
      "enabled": true,
      "protocol": "soft_i2c",
      "default_addr": "0x68"
    },
    "bmp280": {
      "enabled": true,
      "protocol": "hard_i2c",
      "default_addr": "0x76"
    }
  }
}
```

自动生成的 `Device/config.h`：

```c
#define USE_MPU6050     1
#define __SOFTI2C_          // MPU6050 使用软件 I2C

#define USE_BMP280      1
#define __HARDI2C_          // BMP280 使用硬件 I2C
```

---

## 使用示例

### 快速开始

```c
#include <device_init.h>

int main(void)
{
    // 一键初始化所有设备（包括 HAL 接口）
    Device_Init_All();

    // 使用设备
    BMP280_Data_t bmp_data;
    BMP280_ReadAllData(&bmp_data);
    printf("温度: %.2f°C\n", bmp_data.temperature);

    MPU6050_RawData_t mpu_data;
    MPU6050_ReadRawData(&mpu_data);
    printf("加速度 X: %d\n", mpu_data.accel_x);

    return 0;
}
```

### 手动绑定 HAL

```c
#include <device_hal.h>
#include <bmp280.h>

// 自定义 HAL 实现
static device_i2c_hal_t my_i2c_hal = {
    .write_byte = my_i2c_write_byte,
    .read_byte = my_i2c_read_byte,
    .read_bytes = my_i2c_read_bytes,
    .write_bytes = my_i2c_write_bytes,
    .delay_ms = HAL_Delay,
    .delay_us = delay_us,
    .initialized = true,
};

int main(void)
{
    // 绑定自定义 HAL
    BMP280_BindHAL(&my_i2c_hal);

    // 初始化设备
    BMP280_Init();

    // 使用设备...
}
```

---

## 扩展新设备

### 步骤 1：创建设备目录

```
Device/
└── new_sensor/
    ├── new_sensor.c
    └── new_sensor.h
```

### 步骤 2：定义设备接口

```c
// new_sensor.h
#ifndef __NEW_SENSOR_H
#define __NEW_SENSOR_H

#include <stdint.h>
#include <device_hal.h>

// 绑定 HAL
void NewSensor_BindHAL(device_i2c_hal_t *hal);

// 初始化
int NewSensor_Init(void);

// 读取数据
int NewSensor_ReadData(uint8_t *data);

#endif
```

### 步骤 3：实现设备驱动

```c
// new_sensor.c
#include "new_sensor.h"

#define NEW_SENSOR_ADDR  0x48

static device_i2c_hal_t *i2c_hal = NULL;

void NewSensor_BindHAL(device_i2c_hal_t *hal) {
    i2c_hal = hal;
}

int NewSensor_Init(void) {
    if (!i2c_hal || !i2c_hal->initialized) {
        return -1;
    }

    // 初始化代码...
    return 0;
}

int NewSensor_ReadData(uint8_t *data) {
    if (!i2c_hal) return -1;

    return i2c_hal->read_byte(NEW_SENSOR_ADDR, 0x00, data);
}
```

### 步骤 4：添加到配置

在 `project_config.json` 中添加：

```json
{
  "devices": {
    "new_sensor": {
      "enabled": true,
      "protocol": "soft_i2c",
      "description": "新传感器",
      "default_addr": "0x48",
      "sources": ["new_sensor/new_sensor.c"],
      "include_dirs": ["new_sensor"]
    }
  }
}
```

### 步骤 5：重新生成配置

```bash
python tool/build.py --no-interactive
```

---

## 支持的设备列表

| 设备 | 类型 | 支持协议 |
|------|------|----------|
| MPU6050 | 6轴传感器 | I2C |
| BMP280 | 气压传感器 | I2C/SPI |
| HMC5883L | 磁力计 | I2C |
| SSD1306 | OLED显示 | I2C/SPI |
| ST7789 | TFT显示 | SPI |

---

## 参考文档

- [驱动框架文档](driver_framework/)
- [构建系统文档](build_system/)
