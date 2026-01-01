/**
 * @file df_init.h
 * @brief 驱动框架自动初始化系统（借鉴RT-Thread实现）
 * @details 提供在main函数执行前自动初始化框架的机制
 *          - GCC/Clang: 使用段+constructor属性
 *          - Keil MDK: 使用$Sub$$main机制
 *          - IAR: 需手动调用
 * @date 2026-01-02
 */

#ifndef DF_INIT_H
#define DF_INIT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // ============ 编译器识别 ============
#if defined(__ARMCC_VERSION) /* ARM Compiler 4/5 */
#define DF_SECTION(x) __attribute__((section(x)))
#define DF_USED __attribute__((used))
#elif defined(__IAR_SYSTEMS_ICC__) /* IAR Compiler */
#define DF_SECTION(x) @x
#define DF_USED __root
#elif defined(__GNUC__) /* GCC */
#define DF_SECTION(x) __attribute__((section(x)))
#define DF_USED __attribute__((used))
#else
#define DF_SECTION(x)
#define DF_USED
#endif

    // ============ 初始化函数类型 ============
    typedef int (*df_init_fn_t)(void);

    // ============ 初始化优先级定义 ============
    /**
     * 初始化优先级（采用RT-Thread命名风格）
     * 使用字符串级别，链接器会自动按字典序排序
     */
#define DF_INIT_EXPORT_BOARD "0"     // 板级初始化（最早）
#define DF_INIT_EXPORT_PREV "1"      // 前置初始化
#define DF_INIT_EXPORT_DEVICE "2"    // 设备初始化
#define DF_INIT_EXPORT_COMPONENT "3" // 组件初始化
#define DF_INIT_EXPORT_ENV "4"       // 环境初始化
#define DF_INIT_EXPORT_APP "5"       // 应用初始化（最晚）

    // 兼容旧的枚举定义
    typedef enum
    {
        DF_INIT_LEVEL_CORE = 0,        // 核心层 -> 对应 BOARD
        DF_INIT_LEVEL_PLATFORM = 1,    // 平台层 -> 对应 PREV
        DF_INIT_LEVEL_DEVICE = 2,      // 设备层 -> 对应 DEVICE
        DF_INIT_LEVEL_APPLICATION = 3, // 应用层 -> 对应 APP
        DF_INIT_LEVEL_MAX
    } df_init_level_t;

    // ============ 框架初始化函数 ============
    /**
     * @brief 框架总初始化入口
     * @details 执行所有注册的初始化函数
     * @return 成功初始化的函数数量，负数表示错误
     */
    int df_framework_init(void);

    /**
     * @brief 查询框架是否已初始化
     * @return 1=已初始化, 0=未初始化
     */
    int df_is_initialized(void);

// ============ RT-Thread风格自动初始化宏 ============
/**
 * @brief 自动初始化宏（RT-Thread风格）
 * @param fn 初始化函数
 * @param level 初始化级别字符串 (DF_INIT_EXPORT_xxx)
 *
 * @example
 * static int log_init(void) { return 0; }
 * DF_INIT_EXPORT(log_init, DF_INIT_EXPORT_BOARD);
 */
#define DF_INIT_EXPORT(fn, level)             \
    DF_USED const df_init_fn_t __df_init_##fn \
        DF_SECTION(".df_init_fn." level) = fn

// ============ 分级自动初始化宏（便捷使用） ============
#define DF_BOARD_INIT(fn) DF_INIT_EXPORT(fn, DF_INIT_EXPORT_BOARD)
#define DF_PREV_INIT(fn) DF_INIT_EXPORT(fn, DF_INIT_EXPORT_PREV)
#define DF_DEVICE_INIT(fn) DF_INIT_EXPORT(fn, DF_INIT_EXPORT_DEVICE)
#define DF_COMPONENT_INIT(fn) DF_INIT_EXPORT(fn, DF_INIT_EXPORT_COMPONENT)
#define DF_ENV_INIT(fn) DF_INIT_EXPORT(fn, DF_INIT_EXPORT_ENV)
#define DF_APP_INIT(fn) DF_INIT_EXPORT(fn, DF_INIT_EXPORT_APP)

// 兼容旧接口的快捷宏
#define DF_INIT_CORE(fn) DF_BOARD_INIT(fn)
#define DF_INIT_PLATFORM(fn) DF_PREV_INIT(fn)
#define DF_INIT_DEVICE(fn) DF_DEVICE_INIT(fn)
#define DF_INIT_APPLICATION(fn) DF_APP_INIT(fn)

// ============ 自动初始化使能宏 ============
#if defined(__GNUC__) && !defined(__ARMCC_VERSION)
// GCC/Clang: 使用constructor属性
#define DF_INIT_AUTO_ENABLE()                                          \
    __attribute__((constructor(101))) static void __df_auto_init(void) \
    {                                                                  \
        df_framework_init();                                           \
    }
#elif defined(__CC_ARM) || defined(__ARMCC_VERSION)
// Keil MDK: 使用$Sub$$main机制
// 在链接时会自动调用$Sub$$main，然后跳转到真正的main
extern int $Super$$main(void);
int $Sub$$main(void)
{
    df_framework_init();
    return $Super$$main();
}
#define DF_INIT_AUTO_ENABLE() /* Keil自动处理 */
#else
// 其他编译器：需要手动调用
#define DF_INIT_AUTO_ENABLE() /* 请在main函数开头手动调用df_framework_init() */
#endif

#ifdef __cplusplus
}
#endif

#endif // DF_INIT_H
