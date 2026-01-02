/**
 * @file vet6_config.h
 * @brief STM32F407VET6 芯片配置
 *
 * 此文件定义 VET6 芯片的特有配置：
 *   - Flash 大小: 512KB
 *   - RAM 大小: 128KB (112KB SRAM + 16KB CCM)
 *   - 封装: LQFP100
 */

#ifndef __VET6_CONFIG_H
#define __VET6_CONFIG_H

#ifdef __cplusplus
extern "C"
{
#endif

    /*===========================================================================*/
    /*                          芯片规格定义                                      */
    /*===========================================================================*/

/**
 * @brief 芯片型号标识
 */
#define VET6_CHIP_NAME "STM32F407VET6"
#define VET6_CHIP_PACKAGE "LQFP100"

/**
 * @brief 存储器配置
 */
#define VET6_FLASH_SIZE (512 * 1024) /* 512KB Flash */
#define VET6_FLASH_BASE 0x08000000UL /* Flash 起始地址 */
#define VET6_FLASH_END (VET6_FLASH_BASE + VET6_FLASH_SIZE)

#define VET6_SRAM_SIZE (112 * 1024) /* 112KB SRAM */
#define VET6_SRAM_BASE 0x20000000UL /* SRAM 起始地址 */
#define VET6_SRAM_END (VET6_SRAM_BASE + VET6_SRAM_SIZE)

#define VET6_CCM_SIZE (64 * 1024)  /* 64KB CCM RAM */
#define VET6_CCM_BASE 0x10000000UL /* CCM 起始地址 */
#define VET6_CCM_END (VET6_CCM_BASE + VET6_CCM_SIZE)

/**
 * @brief Flash 扇区定义
 * VET6 有 8 个扇区:
 *   - Sector 0-3: 16KB each (0x08000000 - 0x0800FFFF)
 *   - Sector 4:   64KB      (0x08010000 - 0x0801FFFF)
 *   - Sector 5-7: 128KB each (0x08020000 - 0x0807FFFF)
 */
#define VET6_FLASH_SECTOR_COUNT 8

#define VET6_FLASH_SECTOR0_BASE 0x08000000UL
#define VET6_FLASH_SECTOR0_SIZE (16 * 1024)

#define VET6_FLASH_SECTOR1_BASE 0x08004000UL
#define VET6_FLASH_SECTOR1_SIZE (16 * 1024)

#define VET6_FLASH_SECTOR2_BASE 0x08008000UL
#define VET6_FLASH_SECTOR2_SIZE (16 * 1024)

#define VET6_FLASH_SECTOR3_BASE 0x0800C000UL
#define VET6_FLASH_SECTOR3_SIZE (16 * 1024)

#define VET6_FLASH_SECTOR4_BASE 0x08010000UL
#define VET6_FLASH_SECTOR4_SIZE (64 * 1024)

#define VET6_FLASH_SECTOR5_BASE 0x08020000UL
#define VET6_FLASH_SECTOR5_SIZE (128 * 1024)

#define VET6_FLASH_SECTOR6_BASE 0x08040000UL
#define VET6_FLASH_SECTOR6_SIZE (128 * 1024)

#define VET6_FLASH_SECTOR7_BASE 0x08060000UL
#define VET6_FLASH_SECTOR7_SIZE (128 * 1024)

/**
 * @brief GPIO 端口可用性
 * LQFP100 封装可用端口: A, B, C, D, E (部分引脚不可用)
 */
#define VET6_GPIOA_AVAILABLE 1
#define VET6_GPIOB_AVAILABLE 1
#define VET6_GPIOC_AVAILABLE 1
#define VET6_GPIOD_AVAILABLE 1
#define VET6_GPIOE_AVAILABLE 1
#define VET6_GPIOF_AVAILABLE 0 /* LQFP100 无 GPIOF */
#define VET6_GPIOG_AVAILABLE 0 /* LQFP100 无 GPIOG */
#define VET6_GPIOH_AVAILABLE 0 /* LQFP100 只有 PH0, PH1 (OSC) */
#define VET6_GPIOI_AVAILABLE 0 /* LQFP100 无 GPIOI */

/**
 * @brief 链接器脚本引用
 */
#define VET6_LINKER_SCRIPT "stm32f407vet6.ld"

#ifdef __cplusplus
}
#endif

#endif /* __VET6_CONFIG_H */
