/*
 * @file df_init_gcc.ld
 * @brief 驱动框架自动初始化链接脚本片段（GCC/Clang）
 * @details 在主链接脚本中包含此文件，或手动复制以下内容到 .rodata 段
 * @date 2026-01-02
 *
 * 使用方法：
 * 1. 在主链接脚本中 INCLUDE "df_init_gcc.ld"
 * 2. 或者将下面的内容复制到主链接脚本的 .rodata 段中
 */

/* ========================================
 * 在 .rodata 段中添加以下内容：
 * ======================================== */

/*
.rodata :
{
    . = ALIGN(4);
    *(.rodata)
    *(.rodata*)

    // ===== 驱动框架自动初始化段（RT-Thread风格）=====
    . = ALIGN(4);
    __df_init_fn_start = .;
    KEEP(*(.df_init_fn.0))    // 板级初始化（BOARD）
    KEEP(*(.df_init_fn.1))    // 前置初始化（PREV）
    KEEP(*(.df_init_fn.2))    // 设备初始化（DEVICE）
    KEEP(*(.df_init_fn.3))    // 组件初始化（COMPONENT）
    KEEP(*(.df_init_fn.4))    // 环境初始化（ENV）
    KEEP(*(.df_init_fn.5))    // 应用初始化（APP）
    __df_init_fn_end = .;
    . = ALIGN(4);

    . = ALIGN(4);
} > FLASH
*/

/* ========================================
 * 完整示例：STM32 链接脚本
 * ======================================== */

/* 注意：这只是示例，实际使用时需要根据你的芯片型号调整 */

/*
SECTIONS
{
    .text :
    {
        . = ALIGN(4);
        *(.text)
        *(.text*)
        *(.glue_7)
        *(.glue_7t)
        *(.eh_frame)
        KEEP (*(.init))
        KEEP (*(.fini))
        . = ALIGN(4);
    } > FLASH

    .rodata :
    {
        . = ALIGN(4);
        *(.rodata)
        *(.rodata*)

        // ===== 驱动框架自动初始化段 =====
        . = ALIGN(4);
        __df_init_fn_start = .;
        KEEP(*(.df_init_fn.0))
        KEEP(*(.df_init_fn.1))
        KEEP(*(.df_init_fn.2))
        KEEP(*(.df_init_fn.3))
        KEEP(*(.df_init_fn.4))
        KEEP(*(.df_init_fn.5))
        __df_init_fn_end = .;
        . = ALIGN(4);

    } > FLASH

    // ... 其他段 ...
}
*/

/* ========================================
 * 说明：
 * ======================================== */

/*
 * 段名说明：
 *   .df_init_fn.0 - 板级初始化（最早执行）
 *   .df_init_fn.1 - 前置初始化
 *   .df_init_fn.2 - 设备初始化
 *   .df_init_fn.3 - 组件初始化
 *   .df_init_fn.4 - 环境初始化
 *   .df_init_fn.5 - 应用初始化（最晚执行）
 *
 * 链接器会按字典序自动排序这些段，所以执行顺序是：
 *   0 -> 1 -> 2 -> 3 -> 4 -> 5
 *
 * KEEP() 防止链接器优化掉未直接引用的初始化函数
 *
 * __df_init_fn_start 和 __df_init_fn_end 是段边界符号，
 * 在 df_init.c 中会使用这两个符号来遍历所有初始化函数
 */
