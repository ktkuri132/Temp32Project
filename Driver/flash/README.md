# STM32F103 Flash驱动程序

这是一个为STM32F103微控制器设计的内部Flash驱动程序，支持Flash读写操作和用户程序空间管理。

## 功能特性

- ✅ Flash页擦除和批量擦除
- ✅ 半字、字和数据块写入
- ✅ 任意长度数据读取
- ✅ 用户程序结束标记管理
- ✅ Flash空间使用情况统计
- ✅ 地址有效性和对齐检查
- ✅ 操作超时保护
- ✅ 错误状态报告

## 文件结构

```
Driver/flash/
├── flash.h           # Flash驱动头文件
├── flash.c           # Flash驱动实现
├── flash_test.h      # 测试函数声明
├── flash_test.c      # 简单测试实现
├── flash_example.c   # 完整使用示例
└── README.md         # 说明文档
```

## 硬件支持

- **目标芯片**: STM32F103C8T6 (32KB Flash)
- **Flash容量**: 32KB (32768字节)
- **页大小**: 1KB (1024字节)
- **页数量**: 32页
- **地址范围**: 0x08000000 - 0x08007FFF

## 快速开始

### 1. 包含头文件

```c
#include "flash.h"
```

### 2. 初始化Flash驱动

```c
flash_status_t status = flash_init();
if(status != FLASH_OK) {
    // 处理初始化错误
}
```

### 3. 基本读写操作

```c
// 写入半字
uint16_t data = 0x1234;
uint32_t addr = 0x08007C00;  // 最后一页的地址

// 先擦除页
flash_erase_page(addr);

// 写入数据
status = flash_write_halfword(addr, data);

// 读取数据
uint16_t read_data;
status = flash_read_data(addr, (uint8_t*)&read_data, 2);
```

### 4. 用户程序结束标记

```c
// 添加程序结束标记
status = flash_add_program_end_mark();

// 查找程序结束标记
uint32_t end_addr = flash_find_program_end_mark();
if(end_addr != 0) {
    // 找到标记
}
```

### 5. 空间信息查询

```c
flash_info_t info;
status = flash_get_info(&info);
if(status == FLASH_OK) {
    printf("总容量: %lu KB\\n", info.total_size / 1024);
    printf("已使用: %lu 字节\\n", info.used_size);
    printf("剩余: %lu 字节\\n", info.free_size);
}
```

## API参考

### 初始化函数

- `flash_init()` - 初始化Flash驱动

### 擦除操作

- `flash_erase_page(uint32_t page_addr)` - 擦除单页
- `flash_erase_pages(uint32_t start_page, uint32_t page_count)` - 擦除多页

### 写入操作

- `flash_write_halfword(uint32_t addr, uint16_t data)` - 写入半字
- `flash_write_word(uint32_t addr, uint32_t data)` - 写入字
- `flash_write_data(uint32_t addr, const uint8_t *data, uint32_t size)` - 写入数据块

### 读取操作

- `flash_read_data(uint32_t addr, uint8_t *data, uint32_t size)` - 读取数据

### 程序标记管理

- `flash_add_program_end_mark()` - 添加程序结束标记
- `flash_find_program_end_mark()` - 查找程序结束标记

### 信息查询

- `flash_get_info(flash_info_t *info)` - 获取Flash信息
- `flash_get_used_size()` - 获取已使用空间
- `flash_get_free_size()` - 获取剩余空间

### 工具函数

- `flash_is_valid_addr(uint32_t addr)` - 检查地址有效性
- `flash_is_page_aligned(uint32_t addr)` - 检查页对齐
- `flash_get_page_number(uint32_t addr)` - 获取页号

## 用户程序结束标记

驱动程序支持在用户程序结束位置添加特殊标记序列，用于：

1. **精确计算程序大小**: 通过标记位置计算实际使用的Flash空间
2. **空间管理**: 准确了解剩余可用空间
3. **OTA更新**: 为固件更新提供准确的边界信息

### 标记格式

- **标记值**: 0x55AA55AA (4字节)
- **标记长度**: 16字节 (4个标记值)
- **位置**: 程序代码段结束后的4字节对齐地址

### 使用方法

```c
// 在程序编译完成后调用
flash_add_program_end_mark();

// 查询程序使用情况
flash_info_t info;
flash_get_info(&info);
if(info.program_end_found) {
    printf("程序大小: %lu 字节\\n", info.used_size);
    printf("剩余空间: %lu 字节\\n", info.free_size);
}
```

## 错误代码

| 代码 | 含义 |
|------|------|
| FLASH_OK | 操作成功 |
| FLASH_ERROR | 操作失败 |
| FLASH_BUSY | Flash忙 |
| FLASH_TIMEOUT | 操作超时 |
| FLASH_ADDR_ERROR | 地址错误 |
| FLASH_ALIGN_ERROR | 对齐错误 |
| FLASH_WRITE_PROTECTED | 写保护 |

## 注意事项

1. **地址对齐**:
   - 半字写入需要2字节对齐
   - 字写入需要4字节对齐
   - 页擦除需要页边界对齐

2. **写入前擦除**: Flash写入前必须先擦除对应页面

3. **代码段保护**: 不要擦除包含当前运行代码的Flash页面

4. **中断处理**: Flash操作期间可能影响中断响应时间

5. **电源稳定**: Flash操作需要稳定的电源供应

## 测试代码

项目提供了简单的测试函数：

```c
#include "flash_test.h"

int main(void)
{
    // 系统初始化...

    // Flash测试
    simple_flash_test();
    flash_space_info();
    flash_program_mark_test();

    while(1) {
        // 主循环
    }
}
```

## 编译配置

确保项目配置文件中包含了flash驱动：

```json
{
  "files": {
    "sources": [
      "Driver/flash/flash.c",
      // 其他源文件...
    ],
    "include_dirs": [
      "Driver/flash",
      // 其他包含目录...
    ]
  }
}
```

## 许可证

本驱动程序遵循项目许可证。
