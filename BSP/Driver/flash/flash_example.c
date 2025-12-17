/**
 * @file    flash_example.c
 * @author  Flash Driver Example
 * @version V1.0
 * @date    2025-08-21
 * @brief   STM32F103 Flash驱动使用示例
 *          演示flash读写操作和空间管理功能
 */

#include <stdint.h>
#include <stdio.h>

#include "flash.h"
#ifndef __clang__
/**
 * @brief  Flash驱动使用示例
 * @param  None
 * @retval None
 */
void flash_example(void) {
    flash_status_t status;
    flash_info_t flash_info;
    uint8_t test_data[64];
    uint8_t read_data[64];
    uint32_t test_addr;
    uint32_t i;

    printf("=== STM32F103 Flash驱动示例 ===\r\n");

    // 1. 初始化Flash驱动
    status = flash_init();
    if (status != FLASH_OK) {
        printf("Flash初始化失败！错误代码: %d\r\n", status);
        return;
    }
    printf("Flash初始化成功\r\n");

    // 2. 获取Flash信息
    status = flash_get_info(&flash_info);
    if (status == FLASH_OK) {
        printf("\r\n=== Flash信息 ===\r\n");
        printf("总容量: %lu KB\r\n", flash_info.total_size / 1024);
        printf("页大小: %lu 字节\r\n", flash_info.page_size);
        printf("页数量: %lu 页\r\n", flash_info.page_count);
        printf("已使用空间: %lu 字节 (%.1f%%)\r\n", flash_info.used_size,
               (float)flash_info.used_size * 100.0f / flash_info.total_size);
        printf("剩余空间: %lu 字节 (%.1f%%)\r\n", flash_info.free_size,
               (float)flash_info.free_size * 100.0f / flash_info.total_size);

        if (flash_info.program_end_found) {
            printf("程序结束标记: 已找到 (地址: 0x%08lX)\r\n",
                   flash_info.user_program_end);
        } else {
            printf("程序结束标记: 未找到\r\n");
        }
    }

    // 3. 添加用户程序结束标记
    printf("\r\n=== 添加程序结束标记 ===\r\n");
    status = flash_add_program_end_mark();
    if (status == FLASH_OK) {
        printf("程序结束标记添加成功\r\n");

        // 重新获取Flash信息
        flash_get_info(&flash_info);
        if (flash_info.program_end_found) {
            printf("程序结束标记验证: 已找到 (地址: 0x%08lX)\r\n",
                   flash_info.user_program_end);
        }
    } else {
        printf("程序结束标记添加失败！错误代码: %d\r\n", status);
    }

    // 4. Flash读写测试 (使用最后一页进行测试)
    printf("\r\n=== Flash读写测试 ===\r\n");
    test_addr = FLASH_BASE_ADDR + (FLASH_PAGE_COUNT - 1) * FLASH_PAGE_SIZE;
    printf("测试地址: 0x%08lX (最后一页)\r\n", test_addr);

    // 准备测试数据
    for (i = 0; i < sizeof(test_data); i++) {
        test_data[i] = (uint8_t)(i + 0xA5);
    }

    // 擦除测试页
    printf("擦除测试页...\r\n");
    status = flash_erase_page(test_addr);
    if (status != FLASH_OK) {
        printf("页擦除失败！错误代码: %d\r\n", status);
        return;
    }
    printf("页擦除成功\r\n");

    // 写入测试数据
    printf("写入测试数据...\r\n");
    status = flash_write_data(test_addr, test_data, sizeof(test_data));
    if (status != FLASH_OK) {
        printf("数据写入失败！错误代码: %d\r\n", status);
        return;
    }
    printf("数据写入成功\r\n");

    // 读取并验证数据
    printf("读取并验证数据...\r\n");
    status = flash_read_data(test_addr, read_data, sizeof(read_data));
    if (status != FLASH_OK) {
        printf("数据读取失败！错误代码: %d\r\n", status);
        return;
    }

    // 数据验证
    uint8_t verify_ok = 1;
    for (i = 0; i < sizeof(test_data); i++) {
        if (test_data[i] != read_data[i]) {
            printf("数据验证失败！位置: %lu, 写入: 0x%02X, 读取: 0x%02X\r\n", i,
                   test_data[i], read_data[i]);
            verify_ok = 0;
            break;
        }
    }

    if (verify_ok) {
        printf("数据验证成功！\r\n");
    }

    // 5. 显示部分读取的数据
    printf("\r\n=== 读取的数据 (前16字节) ===\r\n");
    for (i = 0; i < 16; i++) {
        printf("0x%02X ", read_data[i]);
        if ((i + 1) % 8 == 0) printf("\r\n");
    }

    printf("\r\n=== Flash驱动示例完成 ===\r\n");
}

/**
 * @brief  打印Flash状态错误信息
 * @param  status: 状态代码
 * @retval None
 */
void flash_print_status(flash_status_t status) {
    switch (status) {
        case FLASH_OK:
            printf("FLASH_OK: 操作成功\r\n");
            break;
        case FLASH_ERROR:
            printf("FLASH_ERROR: 操作失败\r\n");
            break;
        case FLASH_BUSY:
            printf("FLASH_BUSY: Flash忙\r\n");
            break;
        case FLASH_TIMEOUT:
            printf("FLASH_TIMEOUT: 操作超时\r\n");
            break;
        case FLASH_ADDR_ERROR:
            printf("FLASH_ADDR_ERROR: 地址错误\r\n");
            break;
        case FLASH_ALIGN_ERROR:
            printf("FLASH_ALIGN_ERROR: 对齐错误\r\n");
            break;
        case FLASH_WRITE_PROTECTED:
            printf("FLASH_WRITE_PROTECTED: 写保护\r\n");
            break;
        default:
            printf("未知错误代码: %d\r\n", status);
            break;
    }
}

/**
 * @brief  Flash信息查询功能
 * @param  None
 * @retval None
 */
void flash_info_query(void) {
    flash_info_t info;
    flash_status_t status;

    printf("\r\n=== Flash信息查询 ===\r\n");

    status = flash_get_info(&info);
    if (status != FLASH_OK) {
        printf("获取Flash信息失败！");
        flash_print_status(status);
        return;
    }

    printf("Flash基地址: 0x%08X\r\n", FLASH_BASE_ADDR);
    printf("Flash结束地址: 0x%08X\r\n", FLASH_END_ADDR);
    printf("总容量: %lu 字节 (%lu KB)\r\n", info.total_size,
           info.total_size / 1024);
    printf("页大小: %lu 字节\r\n", info.page_size);
    printf("页数量: %lu 页\r\n", info.page_count);

    printf("\r\n--- 空间使用情况 ---\r\n");
    printf("已使用空间: %lu 字节\r\n", info.used_size);
    printf("剩余空间: %lu 字节\r\n", info.free_size);
    printf("使用率: %.1f%%\r\n",
           (float)info.used_size * 100.0f / info.total_size);

    printf("\r\n--- 程序结束标记 ---\r\n");
    if (info.program_end_found) {
        printf("状态: 已找到\r\n");
        printf("程序结束地址: 0x%08lX\r\n", info.user_program_end);
        printf("程序大小: %lu 字节\r\n",
               info.user_program_end - FLASH_BASE_ADDR);
    } else {
        printf("状态: 未找到\r\n");
        printf("估计程序结束地址: 0x%08lX\r\n", info.user_program_end);
    }
}
#endif
