/**
 * @file f407_dac.h
 * @brief STM32F407系列 DAC板级驱动头文件
 * @details 提供DAC1/DAC2通道的初始化和操作API
 *          适用于STM32F407所有封装型号 (VET6, ZGT6, ZET6, VGT6等)
 *
 * DAC通道引脚分配:
 * - DAC_CH1: PA4
 * - DAC_CH2: PA5
 */

#ifndef __F407_DAC_H
#define __F407_DAC_H

#include "f407_gpio.h"

/*===========================================================================*/
/*                              类型定义                                      */
/*===========================================================================*/

/**
 * @brief DAC通道枚举
 */
typedef enum
{
    F407_DAC_CH1 = 0, /**< DAC通道1 (PA4) */
    F407_DAC_CH2 = 1, /**< DAC通道2 (PA5) */
    F407_DAC_CH_MAX
} f407_dac_channel_t;

/**
 * @brief DAC数据对齐方式
 */
typedef enum
{
    F407_DAC_ALIGN_12B_R = 0, /**< 12位右对齐 */
    F407_DAC_ALIGN_12B_L = 1, /**< 12位左对齐 */
    F407_DAC_ALIGN_8B_R = 2   /**< 8位右对齐 */
} f407_dac_align_t;

/**
 * @brief DAC触发源
 */
typedef enum
{
    F407_DAC_TRIG_NONE = 0,      /**< 无触发 (软件触发) */
    F407_DAC_TRIG_TIM6 = 1,      /**< TIM6 TRGO事件 */
    F407_DAC_TRIG_TIM8 = 2,      /**< TIM8 TRGO事件 */
    F407_DAC_TRIG_TIM7 = 3,      /**< TIM7 TRGO事件 */
    F407_DAC_TRIG_TIM5 = 4,      /**< TIM5 TRGO事件 */
    F407_DAC_TRIG_TIM2 = 5,      /**< TIM2 TRGO事件 */
    F407_DAC_TRIG_TIM4 = 6,      /**< TIM4 TRGO事件 */
    F407_DAC_TRIG_EXT_LINE9 = 7, /**< 外部中断线9 */
    F407_DAC_TRIG_SOFTWARE = 8   /**< 软件触发 */
} f407_dac_trigger_t;

/**
 * @brief DAC波形生成类型
 */
typedef enum
{
    F407_DAC_WAVE_NONE = 0,    /**< 无波形 */
    F407_DAC_WAVE_NOISE = 1,   /**< 噪声波形 */
    F407_DAC_WAVE_TRIANGLE = 2 /**< 三角波形 */
} f407_dac_wave_t;

/**
 * @brief DAC三角波幅度
 */
typedef enum
{
    F407_DAC_TRIANGLE_AMP_1 = 0,     /**< 幅度1 */
    F407_DAC_TRIANGLE_AMP_3 = 1,     /**< 幅度3 */
    F407_DAC_TRIANGLE_AMP_7 = 2,     /**< 幅度7 */
    F407_DAC_TRIANGLE_AMP_15 = 3,    /**< 幅度15 */
    F407_DAC_TRIANGLE_AMP_31 = 4,    /**< 幅度31 */
    F407_DAC_TRIANGLE_AMP_63 = 5,    /**< 幅度63 */
    F407_DAC_TRIANGLE_AMP_127 = 6,   /**< 幅度127 */
    F407_DAC_TRIANGLE_AMP_255 = 7,   /**< 幅度255 */
    F407_DAC_TRIANGLE_AMP_511 = 8,   /**< 幅度511 */
    F407_DAC_TRIANGLE_AMP_1023 = 9,  /**< 幅度1023 */
    F407_DAC_TRIANGLE_AMP_2047 = 10, /**< 幅度2047 */
    F407_DAC_TRIANGLE_AMP_4095 = 11  /**< 幅度4095 */
} f407_dac_triangle_amp_t;

/**
 * @brief DAC配置结构体
 */
typedef struct
{
    f407_dac_channel_t channel;           /**< DAC通道 */
    f407_dac_trigger_t trigger;           /**< 触发源 */
    f407_dac_wave_t wave_gen;             /**< 波形生成类型 */
    f407_dac_triangle_amp_t triangle_amp; /**< 三角波幅度 */
    bool output_buffer;                   /**< 输出缓冲使能 */
    bool use_dma;                         /**< DMA使能 */
} f407_dac_config_t;

/**
 * @brief DAC句柄结构体
 */
typedef struct
{
    f407_dac_config_t config; /**< 配置信息 */
    bool initialized;         /**< 初始化标志 */
} f407_dac_handle_t;

/*===========================================================================*/
/*                              常量定义                                      */
/*===========================================================================*/

/* DAC最大值定义 */
#define F407_DAC_MAX_VALUE_12BIT 4095
#define F407_DAC_MAX_VALUE_8BIT 255

/* DAC参考电压 (mV) */
#define F407_DAC_VREF_MV 3300

/* DAC通道引脚 */
#define F407_DAC_CH1_PIN F407_PA4
#define F407_DAC_CH2_PIN F407_PA5

/*===========================================================================*/
/*                              时钟控制                                      */
/*===========================================================================*/

/**
 * @brief 使能DAC时钟
 */
void f407_dac_clk_enable(void);

/**
 * @brief 禁用DAC时钟
 */
void f407_dac_clk_disable(void);

/*===========================================================================*/
/*                              初始化函数                                    */
/*===========================================================================*/

/**
 * @brief 初始化DAC通道
 * @param handle DAC句柄
 * @param config 配置参数
 * @return 0成功, <0失败
 */
int f407_dac_init(f407_dac_handle_t *handle, const f407_dac_config_t *config);

/**
 * @brief 快速初始化DAC通道
 * @param handle DAC句柄
 * @param channel DAC通道
 * @return 0成功, <0失败
 */
int f407_dac_init_simple(f407_dac_handle_t *handle, f407_dac_channel_t channel);

/**
 * @brief 反初始化DAC通道
 * @param handle DAC句柄
 */
void f407_dac_deinit(f407_dac_handle_t *handle);

/*===========================================================================*/
/*                              基本操作                                      */
/*===========================================================================*/

/**
 * @brief 使能DAC通道
 * @param handle DAC句柄
 */
void f407_dac_enable(f407_dac_handle_t *handle);

/**
 * @brief 禁用DAC通道
 * @param handle DAC句柄
 */
void f407_dac_disable(f407_dac_handle_t *handle);

/**
 * @brief 设置DAC输出值 (12位右对齐)
 * @param handle DAC句柄
 * @param value 输出值 (0-4095)
 */
void f407_dac_set_value(f407_dac_handle_t *handle, uint16_t value);

/**
 * @brief 设置DAC输出值 (指定对齐方式)
 * @param handle DAC句柄
 * @param value 输出值
 * @param align 对齐方式
 */
void f407_dac_set_value_aligned(f407_dac_handle_t *handle, uint16_t value,
                                f407_dac_align_t align);

/**
 * @brief 获取DAC输出值
 * @param handle DAC句柄
 * @return 当前输出值
 */
uint16_t f407_dac_get_value(f407_dac_handle_t *handle);

/**
 * @brief 设置DAC输出电压 (mV)
 * @param handle DAC句柄
 * @param voltage_mv 目标电压(mV)
 */
void f407_dac_set_voltage(f407_dac_handle_t *handle, uint16_t voltage_mv);

/**
 * @brief 软件触发DAC转换
 * @param handle DAC句柄
 */
void f407_dac_software_trigger(f407_dac_handle_t *handle);

/*===========================================================================*/
/*                              波形生成                                      */
/*===========================================================================*/

/**
 * @brief 使能噪声波形生成
 * @param handle DAC句柄
 * @param lfsr_mask LFSR掩码选择 (0-11)
 */
void f407_dac_enable_noise(f407_dac_handle_t *handle, uint8_t lfsr_mask);

/**
 * @brief 使能三角波生成
 * @param handle DAC句柄
 * @param amplitude 三角波幅度
 */
void f407_dac_enable_triangle(f407_dac_handle_t *handle, f407_dac_triangle_amp_t amplitude);

/**
 * @brief 禁用波形生成
 * @param handle DAC句柄
 */
void f407_dac_disable_wave(f407_dac_handle_t *handle);

/*===========================================================================*/
/*                              DMA支持                                       */
/*===========================================================================*/

/**
 * @brief 使能DAC DMA
 * @param handle DAC句柄
 */
void f407_dac_enable_dma(f407_dac_handle_t *handle);

/**
 * @brief 禁用DAC DMA
 * @param handle DAC句柄
 */
void f407_dac_disable_dma(f407_dac_handle_t *handle);

/**
 * @brief 获取DAC数据寄存器地址 (用于DMA)
 * @param channel DAC通道
 * @param align 数据对齐方式
 * @return 数据寄存器地址
 */
volatile uint32_t *f407_dac_get_dhr_addr(f407_dac_channel_t channel, f407_dac_align_t align);

/*===========================================================================*/
/*                              双通道同时输出                                 */
/*===========================================================================*/

/**
 * @brief 初始化DAC双通道
 * @param handle1 通道1句柄
 * @param handle2 通道2句柄
 * @return 0成功, <0失败
 */
int f407_dac_dual_init(f407_dac_handle_t *handle1, f407_dac_handle_t *handle2);

/**
 * @brief 同时设置双通道输出值
 * @param value1 通道1输出值 (0-4095)
 * @param value2 通道2输出值 (0-4095)
 */
void f407_dac_dual_set_value(uint16_t value1, uint16_t value2);

/**
 * @brief 软件触发双通道同时转换
 */
void f407_dac_dual_software_trigger(void);

#endif /* __F407_DAC_H */
