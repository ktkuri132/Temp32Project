/**
 * @file test_auto_init.c
 * @brief 测试框架分散自动初始化功能
 * @details 本文件用于验证各个框架模块的自动初始化是否正常执行
 * @date 2026-01-02
 */

#include "df_init.h"
#include "df_log.h"
#include <stdio.h>

// ============ 自定义测试初始化函数 ============

/**
 * @brief 测试BOARD级别初始化
 * @return 0表示成功
 */
static int test_board_init(void)
{
    printf("[TEST] Board level init executed\n");
    return 0;
}

/**
 * @brief 测试DEVICE级别初始化
 * @return 0表示成功
 */
static int test_device_init(void)
{
    // 使用日志系统（此时日志系统应该已经初始化）
    LOG_I("TEST", "Device level init executed\n");
    return 0;
}

/**
 * @brief 测试APP级别初始化
 * @return 0表示成功
 */
static int test_app_init(void)
{
    LOG_I("TEST", "App level init executed\n");
    return 0;
}

// 注册初始化函数到不同级别
DF_INIT_EXPORT(test_board_init, DF_INIT_EXPORT_BOARD);
DF_INIT_EXPORT(test_device_init, DF_INIT_EXPORT_DEVICE);
DF_INIT_EXPORT(test_app_init, DF_INIT_EXPORT_APP);

// ============ 测试函数（供main.c调用） ============

/**
 * @brief 测试框架自动初始化的函数
 * @details 这个函数可以在main.c的main函数中调用来测试自动初始化
 */
void test_framework_auto_init(void)
{
    printf("\n========== 框架自动初始化测试 ==========\n\n");

    // 框架已经自动初始化
    printf("框架初始化状态: %s\n", df_is_initialized() ? "已初始化" : "未初始化");

    printf("\n========== 测试完成 ==========\n");

    // 测试日志系统
    printf("\n========== 测试日志系统 ==========\n");
    LOG_I("MAIN", "This is INFO log\n");
    LOG_W("MAIN", "This is WARNING log\n");
    LOG_E("MAIN", "This is ERROR log\n");
    LOG_D("MAIN", "This is DEBUG log (may not show if level < DEBUG)\n");

    printf("\n========== 所有测试完成 ==========\n");
}
