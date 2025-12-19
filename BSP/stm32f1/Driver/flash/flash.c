/**
 * @file    flash.c
 * @author  STM32 Flash Driver
 * @version V1.0
 * @date    2025-08-21
 * @brief   STM32F103 Internal Flash Driver Implementation
 *          实现flash读写操作和用户程序空间管理功能
 */

#include "flash.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#ifndef __clang__
#include "sys/cdefs.h"

/* 外部链接脚本符号声明 */
extern uint32_t _etext;  // 链接脚本定义的代码段结束地址

/* Flash操作超时时间(毫秒) */
#define FLASH_TIMEOUT_MS 5000

/* Flash键值 */
#define FLASH_KEY1 0x45670123
#define FLASH_KEY2 0xCDEF89AB

/* Flash状态寄存器位定义 */
#define FLASH_SR_BSY 0x00000001       // Busy
#define FLASH_SR_PGERR 0x00000004     // Programming Error
#define FLASH_SR_WRPRTERR 0x00000010  // Write Protection Error
#define FLASH_SR_EOP 0x00000020       // End of Operation

/* Flash控制寄存器位定义 */
#define FLASH_CR_PG 0x00000001    // Programming
#define FLASH_CR_PER 0x00000002   // Page Erase
#define FLASH_CR_MER 0x00000004   // Mass Erase
#define FLASH_CR_STRT 0x00000040  // Start
#define FLASH_CR_LOCK 0x00000080  // Lock

/* 用户程序结束标记数据 */
const uint32_t program_end_mark[4]
    __attribute__((section(".program_end_flag"), used)) = {
        USER_PROGRAM_END_MARK, USER_PROGRAM_END_MARK, USER_PROGRAM_END_MARK,
        USER_PROGRAM_END_MARK};

/**
 * @brief  简单的内存比较函数
 * @param  ptr1: 内存块1指针
 * @param  ptr2: 内存块2指针
 * @param  size: 比较大小
 * @retval int 0:相等 非0:不等
 */
static int flash_memcmp(const void *ptr1, const void *ptr2, size_t size) {
    const uint8_t *p1 = (const uint8_t *)ptr1;
    const uint8_t *p2 = (const uint8_t *)ptr2;
    size_t i;

    for (i = 0; i < size; i++) {
        if (p1[i] != p2[i]) {
            return (p1[i] - p2[i]);
        }
    }
    return 0;
}

/**
 * @brief  获取系统时钟tick(简单实现)
 * @param  None
 * @retval uint32_t 当前tick值
 */
static uint32_t flash_get_tick(void) {
    // 这里需要根据实际的系统时钟实现
    // 可以使用SysTick计数器或者其他定时器
    static uint32_t tick = 0;
    return ++tick;
}

/**
 * @brief  延时函数(简单实现)
 * @param  ms: 延时毫秒数
 * @retval None
 */
static void flash_delay_ms(uint32_t ms) {
    volatile uint32_t i;
    volatile uint32_t j;
    for (i = 0; i < ms; i++) {
        for (j = 0; j < 8000; j++) {
            // 延时循环
        }
    }
}

/**
 * @brief  初始化Flash驱动
 * @param  None
 * @retval flash_status_t 操作结果
 */
flash_status_t flash_init(void) {
    // 使能FLASH时钟
    RCC->AHBENR |= RCC_AHBENR_FLITFEN;

    // 清除所有错误标志
    FLASH->SR = FLASH_SR_EOP | FLASH_SR_PGERR | FLASH_SR_WRPRTERR;

    return FLASH_OK;
}

/**
 * @brief  等待Flash操作完成
 * @param  timeout: 超时时间(毫秒)
 * @retval flash_status_t 操作结果
 */
flash_status_t flash_wait_for_operation(uint32_t timeout) {
    uint32_t start_tick = flash_get_tick();

    while (FLASH->SR & FLASH_SR_BSY) {
        if ((flash_get_tick() - start_tick) > timeout) {
            return FLASH_TIMEOUT;
        }
        flash_delay_ms(1);
    }

    // 检查错误标志
    if (FLASH->SR & FLASH_SR_PGERR) {
        FLASH->SR = FLASH_SR_PGERR;
        return FLASH_ERROR;
    }

    if (FLASH->SR & FLASH_SR_WRPRTERR) {
        FLASH->SR = FLASH_SR_WRPRTERR;
        return FLASH_WRITE_PROTECTED;
    }

    // 清除操作完成标志
    FLASH->SR = FLASH_SR_EOP;

    return FLASH_OK;
}

/**
 * @brief  解锁Flash
 * @param  None
 * @retval flash_status_t 操作结果
 */
flash_status_t flash_unlock(void) {
    if (FLASH->CR & FLASH_CR_LOCK) {
        FLASH->KEYR = FLASH_KEY1;
        FLASH->KEYR = FLASH_KEY2;

        if (FLASH->CR & FLASH_CR_LOCK) {
            return FLASH_ERROR;
        }
    }

    return FLASH_OK;
}

/**
 * @brief  锁定Flash
 * @param  None
 * @retval None
 */
void flash_lock(void) { FLASH->CR |= FLASH_CR_LOCK; }

/**
 * @brief  擦除Flash页
 * @param  page_addr: 页地址(必须是页边界对齐)
 * @retval flash_status_t 操作结果
 */
flash_status_t flash_erase_page(uint32_t page_addr) {
    flash_status_t status;

    // 检查地址有效性
    if (!flash_is_valid_addr(page_addr) || !flash_is_page_aligned(page_addr)) {
        return FLASH_ADDR_ERROR;
    }

    // 等待上一个操作完成
    status = flash_wait_for_operation(FLASH_TIMEOUT_MS);
    if (status != FLASH_OK) {
        return status;
    }

    // 解锁Flash
    status = flash_unlock();
    if (status != FLASH_OK) {
        return status;
    }

    // 设置页擦除模式
    FLASH->CR |= FLASH_CR_PER;

    // 设置页地址
    FLASH->AR = page_addr;

    // 启动擦除
    FLASH->CR |= FLASH_CR_STRT;

    // 等待操作完成
    status = flash_wait_for_operation(FLASH_TIMEOUT_MS);

    // 清除页擦除模式
    FLASH->CR &= ~FLASH_CR_PER;

    // 锁定Flash
    flash_lock();

    return status;
}

/**
 * @brief  擦除多个Flash页
 * @param  start_page: 起始页号(0-31)
 * @param  page_count: 擦除页数
 * @retval flash_status_t 操作结果
 */
flash_status_t flash_erase_pages(uint32_t start_page, uint32_t page_count) {
    flash_status_t status;
    uint32_t i;

    if (start_page >= FLASH_PAGE_COUNT ||
        (start_page + page_count) > FLASH_PAGE_COUNT) {
        return FLASH_ADDR_ERROR;
    }

    for (i = 0; i < page_count; i++) {
        uint32_t page_addr =
            FLASH_BASE_ADDR + ((start_page + i) * FLASH_PAGE_SIZE);
        status = flash_erase_page(page_addr);
        if (status != FLASH_OK) {
            return status;
        }
    }

    return FLASH_OK;
}

/**
 * @brief  写入半字到Flash
 * @param  addr: 写入地址(必须2字节对齐)
 * @param  data: 要写入的半字数据
 * @retval flash_status_t 操作结果
 */
flash_status_t flash_write_halfword(uint32_t addr, uint16_t data) {
    flash_status_t status;

    // 检查地址有效性和对齐
    if (!flash_is_valid_addr(addr) || (addr & 0x01)) {
        return FLASH_ADDR_ERROR;
    }

    // 等待上一个操作完成
    status = flash_wait_for_operation(FLASH_TIMEOUT_MS);
    if (status != FLASH_OK) {
        return status;
    }

    // 解锁Flash
    status = flash_unlock();
    if (status != FLASH_OK) {
        return status;
    }

    // 设置编程模式
    FLASH->CR |= FLASH_CR_PG;

    // 写入数据
    *(__IO uint16_t *)addr = data;

    // 等待操作完成
    status = flash_wait_for_operation(FLASH_TIMEOUT_MS);

    // 清除编程模式
    FLASH->CR &= ~FLASH_CR_PG;

    // 锁定Flash
    flash_lock();

    // 验证写入的数据
    if (status == FLASH_OK) {
        if (*(__IO uint16_t *)addr != data) {
            status = FLASH_ERROR;
        }
    }

    return status;
}

/**
 * @brief  写入字到Flash
 * @param  addr: 写入地址(必须4字节对齐)
 * @param  data: 要写入的字数据
 * @retval flash_status_t 操作结果
 */
flash_status_t flash_write_word(uint32_t addr, uint32_t data) {
    flash_status_t status;

    // 检查地址对齐
    if (addr & 0x03) {
        return FLASH_ALIGN_ERROR;
    }

    // 分两次写入半字
    status = flash_write_halfword(addr, (uint16_t)(data & 0xFFFF));
    if (status != FLASH_OK) {
        return status;
    }

    status = flash_write_halfword(addr + 2, (uint16_t)((data >> 16) & 0xFFFF));

    return status;
}

/**
 * @brief  写入数据块到Flash
 * @param  addr: 写入地址(必须2字节对齐)
 * @param  data: 数据缓冲区指针
 * @param  size: 数据大小(字节)
 * @retval flash_status_t 操作结果
 */
flash_status_t flash_write_data(uint32_t addr, const uint8_t *data,
                                uint32_t size) {
    flash_status_t status;
    uint32_t i;
    uint16_t halfword;

    if (data == NULL || size == 0) {
        return FLASH_ERROR;
    }

    // 检查地址对齐
    if (addr & 0x01) {
        return FLASH_ALIGN_ERROR;
    }

    // 按半字写入
    for (i = 0; i < size; i += 2) {
        if (i + 1 < size) {
            halfword = data[i] | (data[i + 1] << 8);
        } else {
            // 最后一个字节，填充0xFF
            halfword = data[i] | 0xFF00;
        }

        status = flash_write_halfword(addr + i, halfword);
        if (status != FLASH_OK) {
            return status;
        }
    }

    return FLASH_OK;
}

/**
 * @brief  从Flash读取数据
 * @param  addr: 读取地址
 * @param  data: 数据缓冲区指针
 * @param  size: 读取大小(字节)
 * @retval flash_status_t 操作结果
 */
flash_status_t flash_read_data(uint32_t addr, uint8_t *data, uint32_t size) {
    uint32_t i;

    if (data == NULL || size == 0) {
        return FLASH_ERROR;
    }

    if (!flash_is_valid_addr(addr) || !flash_is_valid_addr(addr + size - 1)) {
        return FLASH_ADDR_ERROR;
    }

    for (i = 0; i < size; i++) {
        data[i] = *(__IO uint8_t *)(addr + i);
    }

    return FLASH_OK;
}

/**
 * @brief  在用户程序结束位置添加标记
 * @param  None
 * @retval flash_status_t 操作结果
 */
flash_status_t flash_add_program_end_mark(void) {
    uint32_t mark_addr;
    flash_status_t status;

    // 计算标记地址(4字节对齐)
    mark_addr = ((uint32_t)&_etext + 3) & ~3;

    // 检查地址是否在有效范围内
    if (!flash_is_valid_addr(mark_addr + USER_PROGRAM_END_SIZE - 1)) {
        return FLASH_ADDR_ERROR;
    }

    // 写入标记序列
    status = flash_write_data(mark_addr, (const uint8_t *)program_end_mark,
                              USER_PROGRAM_END_SIZE);

    return status;
}

/**
 * @brief  搜索用户程序结束标记
 * @param  None
 * @retval uint32_t 程序结束地址(0表示未找到)
 */
uint32_t flash_find_program_end_mark(void) {
    // 首先检查链接时是否已经包含了标记
    uint32_t mark_addr = (uint32_t)program_end_mark;

    // 检查标记是否在有效的Flash地址范围内
    if (flash_is_valid_addr(mark_addr)) {
        uint32_t read_mark[4];
        if (flash_read_data(mark_addr, (uint8_t *)read_mark,
                            USER_PROGRAM_END_SIZE) == FLASH_OK) {
            if (flash_memcmp(read_mark, program_end_mark,
                             USER_PROGRAM_END_SIZE) == 0) {
                return mark_addr;
            }
        }
    }

    // 如果链接时的标记无效，则搜索整个Flash
    uint32_t addr;
    uint32_t read_mark[4];

    // 从代码段开始搜索
    for (addr = FLASH_BASE_ADDR; addr < FLASH_END_ADDR - USER_PROGRAM_END_SIZE;
         addr += 4) {
        if (flash_read_data(addr, (uint8_t *)read_mark,
                            USER_PROGRAM_END_SIZE) == FLASH_OK) {
            if (flash_memcmp(read_mark, program_end_mark,
                             USER_PROGRAM_END_SIZE) == 0) {
                return addr;
            }
        }
    }

    return 0;  // 未找到
}

/**
 * @brief  获取Flash信息
 * @param  info: Flash信息结构体指针
 * @retval flash_status_t 操作结果
 */
flash_status_t flash_get_info(flash_info_t *info) {
    uint32_t program_end_addr;

    if (info == NULL) {
        return FLASH_ERROR;
    }

    // 填充基本信息
    info->total_size = FLASH_SIZE;
    info->page_size = FLASH_PAGE_SIZE;
    info->page_count = FLASH_PAGE_COUNT;

    // 搜索程序结束标记
    program_end_addr = flash_find_program_end_mark();

    if (program_end_addr != 0) {
        info->program_end_found = 1;
        info->user_program_end = program_end_addr + USER_PROGRAM_END_SIZE;
        info->used_size = info->user_program_end - FLASH_BASE_ADDR;
        info->free_size = FLASH_SIZE - info->used_size;
    } else {
        info->program_end_found = 0;
        info->user_program_end = (uint32_t)&_etext;
        info->used_size = info->user_program_end - FLASH_BASE_ADDR;
        info->free_size = FLASH_SIZE - info->used_size;
    }

    return FLASH_OK;
}

/**
 * @brief  计算已使用的Flash空间
 * @param  None
 * @retval uint32_t 已使用空间大小(字节)
 */
uint32_t flash_get_used_size(void) {
    flash_info_t info;
    if (flash_get_info(&info) == FLASH_OK) {
        return info.used_size;
    }
    return 0;
}

/**
 * @brief  计算剩余的Flash空间
 * @param  None
 * @retval uint32_t 剩余空间大小(字节)
 */
uint32_t flash_get_free_size(void) {
    flash_info_t info;
    if (flash_get_info(&info) == FLASH_OK) {
        return info.free_size;
    }
    return 0;
}

/**
 * @brief  检查地址是否在Flash范围内
 * @param  addr: 地址
 * @retval uint8_t 1:在范围内 0:不在范围内
 */
uint8_t flash_is_valid_addr(uint32_t addr) {
    return (addr >= FLASH_BASE_ADDR && addr <= FLASH_END_ADDR);
}

/**
 * @brief  检查地址是否页对齐
 * @param  addr: 地址
 * @retval uint8_t 1:页对齐 0:未对齐
 */
uint8_t flash_is_page_aligned(uint32_t addr) {
    return ((addr & (FLASH_PAGE_SIZE - 1)) == 0);
}

/**
 * @brief  获取地址所在的页号
 * @param  addr: 地址
 * @retval uint32_t 页号(0-31)
 */
uint32_t flash_get_page_number(uint32_t addr) {
    if (!flash_is_valid_addr(addr)) {
        return 0xFFFFFFFF;  // 无效地址
    }

    return (addr - FLASH_BASE_ADDR) / FLASH_PAGE_SIZE;
}

/**
 * @brief 计算Flash空间使用情况
 * @param used_bytes: 已使用字节数
 * @param free_bytes: 剩余字节数
 * @param total_bytes: 总字节数
 * @return 1 成功，0 失败
 */
uint8_t flash_get_space_usage(uint32_t *used_bytes, uint32_t *free_bytes,
                              uint32_t *total_bytes) {
    if (!used_bytes || !free_bytes || !total_bytes) {
        return 0;
    }

    uint32_t end_mark_addr = flash_find_program_end_mark();
    if (end_mark_addr == 0) {
        return 0;
    }

    *total_bytes = FLASH_SIZE;
    *used_bytes = (end_mark_addr - FLASH_BASE) + sizeof(program_end_mark);
    *free_bytes = FLASH_SIZE - *used_bytes;

    return 1;
}
#endif