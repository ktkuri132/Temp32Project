/*
 * @file df_init_sections.ld
 * @brief 驱动框架自动初始化段定义（独立链接脚本）
 * @details 定义独立的初始化段，存储在 FLASH 中
 * @date 2026-01-02
 *
 * 使用方法：
 *   在 CMake 或编译命令中添加：-T df_init_sections.ld
 *   必须在主链接脚本之后指定
 */

/* 定义初始化段 - 独立存储在 FLASH 中 */
SECTIONS
{
    .df_init : ALIGN(4)
    {
        PROVIDE(__df_init_fn_start = .);
        KEEP(*(.df_init_fn.0))    /* 板级初始化（BOARD） */
        KEEP(*(.df_init_fn.1))    /* 前置初始化（PREV） */
        KEEP(*(.df_init_fn.2))    /* 设备初始化（DEVICE） */
        KEEP(*(.df_init_fn.3))    /* 组件初始化（COMPONENT） */
        KEEP(*(.df_init_fn.4))    /* 环境初始化（ENV） */
        KEEP(*(.df_init_fn.5))    /* 应用初始化（APP） */
        PROVIDE(__df_init_fn_end = .);
    } >FLASH
}

/*
 * 说明：
 * 1. 创建独立的 .df_init 段存储在 FLASH 中
 * 2. PROVIDE 定义符号但不覆盖已存在的定义
 * 3. KEEP 防止链接器优化掉这些段
 * 4. >FLASH 指定存储位置（必须与主链接脚本中定义的 FLASH 内存区域匹配）
 * 5. 此段会在主链接脚本的所有段之后分配
 */
