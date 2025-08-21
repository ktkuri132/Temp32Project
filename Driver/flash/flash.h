/**
 * @file    flash.h
 * @author  STM32 Flash Driver
 * @version V1.0
 * @date    2025-08-21
 * @brief   STM32F103 Internal Flash Driver
 *          支持flash读写操作和用户程序空间管理
 */

#ifndef __FLASH_H
#define __FLASH_H

#include "stm32f10x.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Flash配置参数 */
#define FLASH_BASE_ADDR 0x08000000  // Flash起始地址
#define FLASH_SIZE (32 * 1024)      // 32KB Flash
#define FLASH_PAGE_SIZE 1024        // 页大小1KB
#define FLASH_PAGE_COUNT 32         // 页数量
#define FLASH_END_ADDR (FLASH_BASE_ADDR + FLASH_SIZE - 1)

/* 用户程序结束标记 */
#define USER_PROGRAM_END_MARK 0x55AA55AA  // 用户程序结束标记
#define USER_PROGRAM_END_SIZE 16          // 标记区域大小(字节)

/* Flash操作结果 */
typedef enum {
    FLASH_OK = 0,          // 操作成功
    FLASH_ERROR,           // 操作失败
    FLASH_BUSY,            // Flash忙
    FLASH_TIMEOUT,         // 超时
    FLASH_ADDR_ERROR,      // 地址错误
    FLASH_ALIGN_ERROR,     // 对齐错误
    FLASH_WRITE_PROTECTED  // 写保护
} flash_status_t;

/* Flash信息结构体 */
typedef struct {
    uint32_t total_size;        // 总容量
    uint32_t page_size;         // 页大小
    uint32_t page_count;        // 页数量
    uint32_t used_size;         // 已使用空间
    uint32_t free_size;         // 剩余空间
    uint32_t user_program_end;  // 用户程序结束地址
    uint8_t program_end_found;  // 是否找到程序结束标记
} flash_info_t;

/* 外部变量声明 */
extern uint32_t _etext;                     // 链接脚本定义的代码段结束地址
extern const uint32_t program_end_mark[4];  // 程序结束标记数组

/* 函数声明 */

/**
 * @brief  初始化Flash驱动
 * @param  None
 * @retval flash_status_t 操作结果
 */
flash_status_t flash_init(void);

/**
 * @brief  解锁Flash
 * @param  None
 * @retval flash_status_t 操作结果
 */
flash_status_t flash_unlock(void);

/**
 * @brief  锁定Flash
 * @param  None
 * @retval None
 */
void flash_lock(void);

/**
 * @brief  擦除Flash页
 * @param  page_addr: 页地址(必须是页边界对齐)
 * @retval flash_status_t 操作结果
 */
flash_status_t flash_erase_page(uint32_t page_addr);

/**
 * @brief  擦除多个Flash页
 * @param  start_page: 起始页号(0-31)
 * @param  page_count: 擦除页数
 * @retval flash_status_t 操作结果
 */
flash_status_t flash_erase_pages(uint32_t start_page, uint32_t page_count);

/**
 * @brief  写入半字到Flash
 * @param  addr: 写入地址(必须2字节对齐)
 * @param  data: 要写入的半字数据
 * @retval flash_status_t 操作结果
 */
flash_status_t flash_write_halfword(uint32_t addr, uint16_t data);

/**
 * @brief  写入字到Flash
 * @param  addr: 写入地址(必须4字节对齐)
 * @param  data: 要写入的字数据
 * @retval flash_status_t 操作结果
 */
flash_status_t flash_write_word(uint32_t addr, uint32_t data);

/**
 * @brief  写入数据块到Flash
 * @param  addr: 写入地址(必须2字节对齐)
 * @param  data: 数据缓冲区指针
 * @param  size: 数据大小(字节)
 * @retval flash_status_t 操作结果
 */
flash_status_t flash_write_data(uint32_t addr, const uint8_t *data,
                                uint32_t size);

/**
 * @brief  从Flash读取数据
 * @param  addr: 读取地址
 * @param  data: 数据缓冲区指针
 * @param  size: 读取大小(字节)
 * @retval flash_status_t 操作结果
 */
flash_status_t flash_read_data(uint32_t addr, uint8_t *data, uint32_t size);

/**
 * @brief  在用户程序结束位置添加标记
 * @param  None
 * @retval flash_status_t 操作结果
 */
flash_status_t flash_add_program_end_mark(void);

/**
 * @brief  搜索用户程序结束标记
 * @param  None
 * @retval uint32_t 程序结束地址(0表示未找到)
 */
uint32_t flash_find_program_end_mark(void);

/**
 * @brief  获取Flash信息
 * @param  info: Flash信息结构体指针
 * @retval flash_status_t 操作结果
 */
flash_status_t flash_get_info(flash_info_t *info);

/**
 * @brief  计算已使用的Flash空间
 * @param  None
 * @retval uint32_t 已使用空间大小(字节)
 */
uint32_t flash_get_used_size(void);

/**
 * @brief  计算剩余的Flash空间
 * @param  None
 * @retval uint32_t 剩余空间大小(字节)
 */
uint32_t flash_get_free_size(void);

/**
 * @brief  检查地址是否在Flash范围内
 * @param  addr: 地址
 * @retval uint8_t 1:在范围内 0:不在范围内
 */
uint8_t flash_is_valid_addr(uint32_t addr);

/**
 * @brief  检查地址是否页对齐
 * @param  addr: 地址
 * @retval uint8_t 1:页对齐 0:未对齐
 */
uint8_t flash_is_page_aligned(uint32_t addr);

/**
 * @brief  获取地址所在的页号
 * @param  addr: 地址
 * @retval uint32_t 页号(0-31)
 */
uint32_t flash_get_page_number(uint32_t addr);

/**
 * @brief 计算Flash空间使用情况
 * @param used_bytes: 已使用字节数
 * @param free_bytes: 剩余字节数
 * @param total_bytes: 总字节数
 * @return 1 成功，0 失败
 */
uint8_t flash_get_space_usage(uint32_t *used_bytes, uint32_t *free_bytes,
                              uint32_t *total_bytes);

/**
 * @brief  测试Flash空间计算和程序结束标记功能
 * @param  None
 * @retval uint8_t 测试结果 (1:成功, 0:失败)
 */
uint8_t flash_space_test(void);

/**
 * @brief  等待Flash操作完成
 *
 *
 * @param  timeout: 超时时间(毫秒)
 * @retval flash_status_t 操作结果
 */
flash_status_t flash_wait_for_operation(uint32_t timeout);

#ifdef __cplusplus
}
#endif

#endif /* __FLASH_H */
