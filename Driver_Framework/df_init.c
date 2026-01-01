/**
 * @file df_init.c
 * @brief 驱动框架自动初始化系统实现（RT-Thread风格）
 * @date 2026-01-02
 */

#include "df_init.h"
#include "df_log.h"
#include <stddef.h>

// ============ 初始化状态 ============
static volatile uint8_t df_initialized = 0;

// ============ 链接器段符号声明 ============
// 采用RT-Thread风格：段名自动按字典序排序
#if defined(__ARMCC_VERSION) /* Keil MDK */
extern const int DF_InitFnSection$$Base;
extern const int DF_InitFnSection$$Limit;
#elif defined(__ICCARM__) /* IAR */
#pragma section = "DF_InitFnSection"
#elif defined(__GNUC__) /* GCC */
extern df_init_fn_t __df_init_fn_start;
extern df_init_fn_t __df_init_fn_end;
#endif

// ============ 查询是否已初始化 ============
int df_is_initialized(void)
{
    return df_initialized;
}

// ============ 框架总初始化（RT-Thread风格） ============
int df_framework_init(void)
{
    if (df_initialized)
    {
        return 0;
    }

    const df_init_fn_t *fn_ptr;
    int result;
    int success = 0;
    int failed = 0;

    LOGI("╔════════════════════════════════════════╗\n");
    LOGI("║  Driver Framework Initialization      ║\n");
    LOGI("╚════════════════════════════════════════╝\n");

#if defined(__ARMCC_VERSION) /* Keil MDK */
    // Keil: 从$$Base到$$Limit遍历
    for (fn_ptr = (const df_init_fn_t *)&DF_InitFnSection$$Base;
         fn_ptr < (const df_init_fn_t *)&DF_InitFnSection$$Limit;
         fn_ptr++)
    {
        if (*fn_ptr != NULL)
        {
            result = (*fn_ptr)();
            if (result == 0)
            {
                success++;
            }
            else
            {
                failed++;
                LOGE("[DF_INIT] Function at %p failed (ret=%d)\n", *fn_ptr, result);
            }
        }
    }
#elif defined(__ICCARM__) /* IAR */
    // IAR: 使用pragma section
    fn_ptr = __section_begin("DF_InitFnSection");
    const df_init_fn_t *fn_end = __section_end("DF_InitFnSection");

    for (; fn_ptr < fn_end; fn_ptr++)
    {
        if (*fn_ptr != NULL)
        {
            result = (*fn_ptr)();
            if (result == 0)
            {
                success++;
            }
            else
            {
                failed++;
            }
        }
    }
#elif defined(__GNUC__)   /* GCC/Clang */
    // GCC: 从__df_init_fn_start到__df_init_fn_end遍历
    for (fn_ptr = &__df_init_fn_start; fn_ptr < &__df_init_fn_end; fn_ptr++)
    {
        if (*fn_ptr != NULL)
        {
            result = (*fn_ptr)();
            if (result == 0)
            {
                success++;
            }
            else
            {
                failed++;
                LOGE("[DF_INIT] Init function failed (ret=%d)\n", result);
            }
        }
    }
#else
#warning "DF_Framework_Init not implemented for this compiler"
#endif

    df_initialized = 1;

    LOGI("════════════════════════════════════════\n");
    LOGI("[DF_INIT] %d components initialized", success);
    if (failed > 0)
    {
        LOGE(", %d failed", failed);
    }
    LOGI("\n════════════════════════════════════════\n\n");

    return success;
}


