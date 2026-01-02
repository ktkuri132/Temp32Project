/**
 * @file    df_irq.c
 * @brief   驱动框架 - 中断处理模块
 *
 * @details 本模块实现了一个软件中断处理框架，用于将硬件中断与业务逻辑解耦�?
 *
 *          设计思想�?
 *          1. 中断服务程序(ISR)中只做最少的工作：标记中断状态、保存数�?
 *          2. 实际的业务处理在主循环中按优先级顺序执行
 *          3. 通过状态机机制避免中断处理冲突
 *
 *          工作流程�?
 *          ┌─────────────�?   ┌──────────────────�?   ┌─────────────────�?
 *          �? 硬件中断   │───>�?irq_handle_loader │───>�?标记PENDING状�?�?
 *          �? (ISR调用)  �?   �?  (加载中断数据)  �?   �?  保存参数argv  �?
 *          └─────────────�?   └──────────────────�?   └─────────────────�?
 *                                                              �?
 *          ┌─────────────�?   ┌──────────────────�?            �?
 *          �? 执行回调   �?───�?irq_handle_runner �?────────────�?
 *          �? 清除状�?  �?   �?(主循环中调用)    �?
 *          └─────────────�?   └──────────────────�?
 *
 *          状态转换：
 *          IRQ_STATE_DISABLE ──(loader)──> IRQ_STATE_PENDING ──(runner)──> IRQ_STATE_READY ──(执行�?──> IRQ_STATE_DISABLE
 *
 * @version 1.0
 * @date    2025-12-27
 *
 * @note    使用须知�?
 *          - 中断句柄数组必须�?irq_num = 0xFFFF 作为结束标记
 *          - 优先级数值越小，优先级越高（0为最高优先级�?
 *          - 在ISR中调用loader，在主循环中调用runner
 */

#include "df_irq.h"
#include "df_init.h"
#include "df_log.h"
#include <stdio.h>
#include <stdlib.h>

// 外部函数声明
extern uint32_t get_tick(void);

/*============================================================================*/
/*                              私有函数定义                                   */
/*============================================================================*/

/**
 * @brief   在中断句柄数组中查找指定中断号的句柄索引
 *
 * @details 遍历中断句柄数组，根据中断号查找对应的处理句柄�?
 *
 *          注意事项�?
 *          - 当前实现�?sizeof(irq_handles) 无法正确获取数组大小
 *            因为数组作为参数传递时会退化为指针
 *          - 建议改用结束标记(0xFFFF)来判断数组边�?
 *
 * @param[in]  irq_handles  中断处理句柄数组，需以irq_num=0xFFFF结尾
 * @param[in]  irq_num      要查找的中断号（如：USART1_IRQn, TIM2_IRQn等）
 *
 * @return  int8_t 查找结果
 * @retval  >=0    成功找到，返回句柄在数组中的索引位置
 * @retval  -1     未找到匹配的中断�?
 *
 * @par 示例代码�?
 * @code
 *     irq_handle_t handles[] = {
 *         {.irq_num = USART1_IRQn, ...},
 *         {.irq_num = TIM2_IRQn, ...},
 *         {.irq_num = 0xFFFF}  // 结束标记
 *     };
 *     int8_t idx = find_irq_handle(handles, USART1_IRQn);
 *     if (idx >= 0) {
 *         // 找到了，idx = 0
 *     }
 * @endcode
 */
int8_t find_irq_handle(irq_handle_t ih[], uint16_t irq_num)
{
    /* 遍历句柄数组，使�?xFFFF作为结束标记 */
    for (int i = 0; ih[i].irq_num != 0xFFFF; i++)
    {
        if (ih[i].irq_num == irq_num)
        {
            return i; /* 找到匹配项，返回索引 */
        }
    }
    return -1; /* 遍历完成，未找到匹配�?*/
}

/*============================================================================*/
/*                              公共函数定义                                   */
/*============================================================================*/

/**
 * @brief   中断处理加载�?- 在ISR中调用，用于加载中断数据
 *
 * @details 此函数应在硬件中断服务程�?ISR)中调用�?
 *          主要功能�?
 *          1. 根据中断号查找对应的处理句柄
 *          2. 检查上一次中断是否处理完成（防止数据覆盖�?
 *          3. 将中断状态标记为PENDING，等待runner处理
 *          4. 保存中断相关的参数数�?
 *
 *          防重入机制：
 *          如果上一次中断数据尚未处理完（状态仍为PENDING），
 *          则丢弃本次数据，通过读取寄存器清除中断标志�?
 *
 * @param[in,out] ih       中断处理句柄指针数组，需以irq_num=0xFFFF结尾
 * @param[in]     irq_num  触发的中断号
 * @param[in]     argv     中断参数数组，通常包含接收到的数据指针
 *                         - argv[0]: 通常为数据缓冲区指针
 *                         - argv[1]: 可选，数据长度等附加信�?
 *
 * @return  int 加载结果
 * @retval  0   成功加载，等待runner执行
 * @retval  -1  加载失败，上次中断未处理完，本次数据被丢�?
 * @retval  -2  未找到对应的中断处理句柄
 *
 * @warning 此函数运行在中断上下文中，应尽量简�?
 * @warning argv指向的数据在runner处理前不应被修改
 *
 * @par 使用示例（在USART中断中）�?
 * @code
 *     void USART1_IRQHandler(void) {
 *         if (USART_GetITStatus(USART1, USART_IT_RXNE)) {
 *             static uint8_t rx_data;
 *             rx_data = USART_ReceiveData(USART1);
 *             void *args[] = {&rx_data, NULL};
 *             irq_handle_loader(irq_handles, USART1_IRQn, args);
 *         }
 *     }
 * @endcode
 */
int irq_handle_loader(irq_handle_t ih[], uint16_t irq_num, void *argv[])
{
    /* 步骤1：根据中断号查找对应的处理句�?*/
    int8_t irq_handle_index = find_irq_handle(ih, irq_num);

    if (irq_handle_index >= 0)
    {
        /* 步骤2：检查中断状态，防止数据覆盖 */
        if (ih[irq_handle_index].irq_state == IRQ_STATE_PENDING)
        {
            return -1; /* 加载失败，数据被丢弃 */
        }
        else
        {
            /* 步骤3：标记中断状态为待处�?*/
            ih[irq_handle_index].irq_state = IRQ_STATE_PENDING;

            /* 步骤4：保存中断参数，供runner使用 */
            ih[irq_handle_index].argv = argv;
            return 0; /* 成功加载 */
        }
    }
    else
    {
        /* 未注册的中断号，可能是配置错�?*/
        return -2;
    }
}

/**
 * @brief   中断处理运行�?- 在主循环中调用，执行中断回调函数
 *
 * @details 此函数应在主循环(main loop)中周期性调用�?
 *          主要功能�?
 *          1. 扫描所有中断句柄，收集待处理的中断
 *          2. 按优先级排序（优先级数值小的先执行�?
 *          3. 执行最高优先级的中断回调函�?
 *          4. 清除中断状态，释放资源
 *
 *          优先级机制：
 *          - 使用优先级作为临时数组的索引
 *          - 优先�?最高，数值越大优先级越低
 *          - 每次调用只执行一个最高优先级的中�?
 *
 * @param[in,out] ih            中断处理句柄指针数组，需以irq_num=0xFFFF结尾
 * @param[in]     max_priority  最大优先级数量（用于分配临时数组大小）
 *
 * @return  int 执行结果
 * @retval  0   成功执行了一个中断回�?
 * @retval  -1  没有待处理的中断或内存分配失�?
 *
 * @note    此函数每次只处理一个中断，如需处理所有待处理中断�?
 *          应循环调用直到返�?1
 *
 * @par 使用示例�?
 * @code
 *     int main(void) {
 *         // 初始�?..
 *         while (1) {
 *             // 处理所有待处理的中�?
 *             while (irq_handle_runner(irq_handles) == 0) {
 *                 // 继续处理下一�?
 *             }
 *             // 其他主循环任�?..
 *         }
 *     }
 * @endcode
 */
int irq_handle_runner(irq_handle_t ih[])
{
    /* 分配临时数组用于按优先级排序，大小为最大优先级�?*/
    static irq_handle_t ih_temp[IRQ_HANDLE_MAX_NUM];
    int i = 0, j = 0;
    /*
     * 第一阶段：扫描所有句柄，收集PENDING状态的中断
     * 并按优先级放入临时数�?
     */
    for (; ih[i].irq_num != 0xFFFF; i++)
    {
        if (ih[i].irq_state == IRQ_STATE_PENDING) /* 只处理PENDING状�?*/
        {
            /* 以优先级为索引存入临时数组，存入结构体的地址 */
            ih_temp[ih[i].irq_priority] = ih[i];
            /* 将状态从PENDING改为READY，表示即将执�?*/
            ih_temp[ih[i].irq_priority].irq_state = IRQ_STATE_READY;
        }
        ih[i].irq_state = IRQ_STATE_DISABLE; /* 清除原句柄状�?*/
    }
    /*
     * 第二阶段：按优先级顺序（�?开始）查找并执�?
     * 优先�?最高，先执�?
     */
    for (; j < IRQ_HANDLE_MAX_NUM; j++)
    {
        /* 检查该优先级是否有待处理的中断 */
        if (ih_temp[j].irq_state == IRQ_STATE_READY)
        {
            /* 执行中断回调函数，传入参�?*/
            ih_temp[j].irq_thread(0, ih_temp[j].argv);
            /* 执行完毕，清除状�?*/
            ih_temp[j].irq_state = IRQ_STATE_DISABLE;
        }
    }
    return -1; /* 没有待处理的中断 */
}

#include <driver.h>

// ============ 自动初始�?============
/**
 * @brief 中断管理框架自动初始化函�?
 * @details 在框架初始化时自动调用，初始化中断管理框�?
 * @return 0表示成功
 */
static int df_irq_auto_init(void)
{
    Systick_Init(1); // 配置1ms系统节拍
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    NVIC_SetPriority(SysTick_IRQn, 0); // SysTick最高优先级
    NVIC_EnableIRQ(SysTick_IRQn);
    log_set_timestamp_func(get_tick);
    log_enable_timestamp(ENABLE);
    // 中断框架暂无需特殊初始化，此函数用于日志记�?
    LOG_I("IRQ", "Interrupt framework initialized\n");
    return 0;
}

// 将中断框架初始化注册到PREV级别
DF_INIT_EXPORT(df_irq_auto_init, DF_INIT_EXPORT_PREV);
