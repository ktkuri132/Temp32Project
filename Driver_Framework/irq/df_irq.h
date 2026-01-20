#ifndef __DF_IRQ_H__
#define __DF_IRQ_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <dev_frame.h>

#define DF_IRQ_END {0xFFFF, 0, NULL, DF_IRQ_STATE_DISABLE, {0}} // 结束标志
#define DF_IRQ_MAX_NUM 5                                        // 最大中断句柄数量
#define DF_IRQ_STATE_READY 0x01
#define DF_IRQ_STATE_PENDING 0x02
#define DF_IRQ_STATE_DISABLE 0x00

/**
 * @brief 中断句柄结构体
 * @note 中断处理线程统一使用 int (*)(df_arg_t) 类型
 */
typedef struct df_irq_struct
{
    uint16_t irq_num;         // 中断号
    uint8_t priority;         // 中断优先级
    int (*handler)(df_arg_t); // 中断处理线程（统一接口）
    uint8_t state;            // 中断就绪标志
    df_arg_t argv;            // 中断处理线程参数
} df_irq_t;

int8_t df_irq_find(df_irq_t ih[], uint16_t irq_num);
int df_irq_load(df_irq_t ih[], uint16_t irq_num, df_arg_t argv);
int df_irq_run(df_irq_t ih[]);
#endif /* __DF_IRQ_H__ */