#ifndef __DF_IRQ_H__
#define __DF_IRQ_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>


#define IRQ_HANDLE_END {0xFFFF,NULL,false} // 结束标志
#define IRQ_HANDLE_MAX_NUM 5          // 最大中断句柄数量
#define READY 0x01
#define PENDING 0x02
#define DISABLE 0x00

typedef struct irq_handle_t
{
    uint16_t irq_num;                   // 中断号
    uint8_t irq_priority;            // 中断优先级
    int (*irq_thread)(int,void *[]);// 中断处理线程
    uint8_t irq_state;                  // 中断就绪标志
    void **argv;                 // 中断处理线程参数
}irq_handle_t;

int8_t find_irq_handle(irq_handle_t ih[], uint16_t irq_num);
int irq_handle_loader(irq_handle_t ih[],uint16_t irq_num,void *argv[]);
int irq_handle_runner(irq_handle_t ih[]);
#endif /* __DF_IRQ_H__ */