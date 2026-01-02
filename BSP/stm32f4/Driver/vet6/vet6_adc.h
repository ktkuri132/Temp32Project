/**
 * @file vet6_adc.h
 * @brief STM32F407VET6 ADC板级驱动头文件
 * @details 提供ADC1-ADC3的初始化和操作API
 *          - ADC1: 16通道, APB2 (最高36MHz ADC时钟)
 *          - ADC2: 16通道, APB2
 *          - ADC3: 16通道, APB2
 *          - 支持单次/连续转换, 扫描模式, DMA传输
 *          - 支持内部温度传感器和VREFINT
 *
 * ADC通道引脚:
 *   ADC123_IN0:  PA0    ADC123_IN8:  PB0
 *   ADC123_IN1:  PA1    ADC123_IN9:  PB1
 *   ADC123_IN2:  PA2    ADC123_IN10: PC0
 *   ADC123_IN3:  PA3    ADC123_IN11: PC1
 *   ADC12_IN4:   PA4    ADC123_IN12: PC2
 *   ADC12_IN5:   PA5    ADC123_IN13: PC3
 *   ADC12_IN6:   PA6    ADC12_IN14:  PC4
 *   ADC12_IN7:   PA7    ADC12_IN15:  PC5
 *   ADC1_IN16: 温度传感器  ADC1_IN17: VREFINT  ADC1_IN18: VBAT
 */

#ifndef __VET6_ADC_H
#define __VET6_ADC_H

#include <stm32f4xx.h>
#include <stdint.h>
#include <stdbool.h>
#include "vet6_gpio.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /*===========================================================================*/
    /*                              类型定义                                      */
    /*===========================================================================*/

    /**
     * @brief ADC编号枚举
     */
    typedef enum
    {
        VET6_ADC1 = 0,
        VET6_ADC2,
        VET6_ADC3,
        VET6_ADC_MAX
    } vet6_adc_t;

    /**
     * @brief ADC通道枚举
     */
    typedef enum
    {
        VET6_ADC_CH0 = 0,         // PA0
        VET6_ADC_CH1 = 1,         // PA1
        VET6_ADC_CH2 = 2,         // PA2
        VET6_ADC_CH3 = 3,         // PA3
        VET6_ADC_CH4 = 4,         // PA4
        VET6_ADC_CH5 = 5,         // PA5
        VET6_ADC_CH6 = 6,         // PA6
        VET6_ADC_CH7 = 7,         // PA7
        VET6_ADC_CH8 = 8,         // PB0
        VET6_ADC_CH9 = 9,         // PB1
        VET6_ADC_CH10 = 10,       // PC0
        VET6_ADC_CH11 = 11,       // PC1
        VET6_ADC_CH12 = 12,       // PC2
        VET6_ADC_CH13 = 13,       // PC3
        VET6_ADC_CH14 = 14,       // PC4
        VET6_ADC_CH15 = 15,       // PC5
        VET6_ADC_CH_TEMP = 16,    // 内部温度传感器 (仅ADC1)
        VET6_ADC_CH_VREFINT = 17, // 内部参考电压 (仅ADC1)
        VET6_ADC_CH_VBAT = 18,    // VBAT/2 (仅ADC1)
        VET6_ADC_CH_MAX
    } vet6_adc_channel_t;

    /**
     * @brief ADC分辨率
     */
    typedef enum
    {
        VET6_ADC_RES_12BIT = 0, // 12位分辨率
        VET6_ADC_RES_10BIT = 1, // 10位分辨率
        VET6_ADC_RES_8BIT = 2,  // 8位分辨率
        VET6_ADC_RES_6BIT = 3   // 6位分辨率
    } vet6_adc_resolution_t;

    /**
     * @brief ADC采样时间
     */
    typedef enum
    {
        VET6_ADC_SAMPLETIME_3 = 0,   // 3个周期
        VET6_ADC_SAMPLETIME_15 = 1,  // 15个周期
        VET6_ADC_SAMPLETIME_28 = 2,  // 28个周期
        VET6_ADC_SAMPLETIME_56 = 3,  // 56个周期
        VET6_ADC_SAMPLETIME_84 = 4,  // 84个周期
        VET6_ADC_SAMPLETIME_112 = 5, // 112个周期
        VET6_ADC_SAMPLETIME_144 = 6, // 144个周期
        VET6_ADC_SAMPLETIME_480 = 7  // 480个周期
    } vet6_adc_sampletime_t;

    /**
     * @brief ADC转换模式
     */
    typedef enum
    {
        VET6_ADC_MODE_SINGLE = 0,     // 单次转换
        VET6_ADC_MODE_CONTINUOUS = 1, // 连续转换
        VET6_ADC_MODE_SCAN = 2        // 扫描模式
    } vet6_adc_mode_t;

    /**
     * @brief ADC触发源
     */
    typedef enum
    {
        VET6_ADC_TRIG_SOFTWARE = 0,  // 软件触发
        VET6_ADC_TRIG_TIM1_CC1 = 1,  // TIM1 CC1
        VET6_ADC_TRIG_TIM1_CC2 = 2,  // TIM1 CC2
        VET6_ADC_TRIG_TIM1_CC3 = 3,  // TIM1 CC3
        VET6_ADC_TRIG_TIM2_CC2 = 4,  // TIM2 CC2
        VET6_ADC_TRIG_TIM2_CC3 = 5,  // TIM2 CC3
        VET6_ADC_TRIG_TIM2_CC4 = 6,  // TIM2 CC4
        VET6_ADC_TRIG_TIM2_TRGO = 7, // TIM2 TRGO
        VET6_ADC_TRIG_TIM3_CC1 = 8,  // TIM3 CC1
        VET6_ADC_TRIG_TIM3_TRGO = 9, // TIM3 TRGO
        VET6_ADC_TRIG_TIM4_CC4 = 10, // TIM4 CC4
        VET6_ADC_TRIG_TIM5_CC1 = 11, // TIM5 CC1
        VET6_ADC_TRIG_TIM5_CC2 = 12, // TIM5 CC2
        VET6_ADC_TRIG_TIM5_CC3 = 13, // TIM5 CC3
        VET6_ADC_TRIG_TIM8_CC1 = 14, // TIM8 CC1
        VET6_ADC_TRIG_TIM8_TRGO = 15 // TIM8 TRGO
    } vet6_adc_trigger_t;

    /**
     * @brief ADC配置结构体
     */
    typedef struct
    {
        vet6_adc_t adc;                   // ADC编号
        vet6_adc_resolution_t resolution; // 分辨率
        vet6_adc_mode_t mode;             // 转换模式
        bool align_left;                  // 左对齐 (false=右对齐)
        bool use_dma;                     // 使用DMA
    } vet6_adc_config_t;

    /**
     * @brief ADC句柄结构体
     */
    typedef struct
    {
        ADC_TypeDef *instance;    // ADC外设指针
        vet6_adc_config_t config; // 配置信息
        bool initialized;         // 初始化标志
    } vet6_adc_handle_t;

/*===========================================================================*/
/*                              宏定义                                        */
/*===========================================================================*/

/* ADC最大值 */
#define VET6_ADC_MAX_VALUE_12BIT 4095
#define VET6_ADC_MAX_VALUE_10BIT 1023
#define VET6_ADC_MAX_VALUE_8BIT 255
#define VET6_ADC_MAX_VALUE_6BIT 63

/* 内部参考电压 (mV) */
#define VET6_VREFINT_MV 1200

/* 温度传感器参数 */
#define VET6_TEMP_V25 760      // 25°C时电压 (mV)
#define VET6_TEMP_AVG_SLOPE 25 // 平均斜率 (mV/°C * 10)

/* ADC时钟预分频 */
#define VET6_ADC_PRESCALER_DIV2 0
#define VET6_ADC_PRESCALER_DIV4 1
#define VET6_ADC_PRESCALER_DIV6 2
#define VET6_ADC_PRESCALER_DIV8 3

    /*===========================================================================*/
    /*                              初始化API                                     */
    /*===========================================================================*/

    /**
     * @brief 使能ADC时钟
     * @param adc ADC编号
     */
    void vet6_adc_clk_enable(vet6_adc_t adc);

    /**
     * @brief 禁用ADC时钟
     * @param adc ADC编号
     */
    void vet6_adc_clk_disable(vet6_adc_t adc);

    /**
     * @brief 获取ADC外设基地址
     * @param adc ADC编号
     * @return ADC外设指针
     */
    ADC_TypeDef *vet6_adc_get_base(vet6_adc_t adc);

    /**
     * @brief 配置ADC通道引脚为模拟模式
     * @param channel ADC通道
     * @return 0:成功, -1:失败
     */
    int vet6_adc_pin_init(vet6_adc_channel_t channel);

    /**
     * @brief 初始化ADC
     * @param handle ADC句柄指针
     * @param config 配置结构体指针
     * @return 0:成功, -1:失败
     */
    int vet6_adc_init(vet6_adc_handle_t *handle, const vet6_adc_config_t *config);

    /**
     * @brief 快速初始化ADC (单次转换, 12位)
     * @param handle ADC句柄指针
     * @param adc ADC编号
     * @return 0:成功, -1:失败
     */
    int vet6_adc_init_simple(vet6_adc_handle_t *handle, vet6_adc_t adc);

    /**
     * @brief 反初始化ADC
     * @param handle ADC句柄指针
     */
    void vet6_adc_deinit(vet6_adc_handle_t *handle);

    /*===========================================================================*/
    /*                              基本操作API                                   */
    /*===========================================================================*/

    /**
     * @brief 使能ADC
     * @param handle ADC句柄指针
     */
    void vet6_adc_enable(vet6_adc_handle_t *handle);

    /**
     * @brief 禁用ADC
     * @param handle ADC句柄指针
     */
    void vet6_adc_disable(vet6_adc_handle_t *handle);

    /**
     * @brief 配置ADC通道
     * @param handle ADC句柄指针
     * @param channel 通道号
     * @param rank 序列位置 (1-16)
     * @param sampletime 采样时间
     * @return 0:成功, -1:失败
     */
    int vet6_adc_config_channel(vet6_adc_handle_t *handle, vet6_adc_channel_t channel,
                                uint8_t rank, vet6_adc_sampletime_t sampletime);

    /**
     * @brief 启动ADC转换
     * @param handle ADC句柄指针
     */
    void vet6_adc_start(vet6_adc_handle_t *handle);

    /**
     * @brief 停止ADC转换
     * @param handle ADC句柄指针
     */
    void vet6_adc_stop(vet6_adc_handle_t *handle);

    /**
     * @brief 等待转换完成
     * @param handle ADC句柄指针
     * @param timeout 超时时间
     * @return 0:成功, -1:超时
     */
    int vet6_adc_wait_eoc(vet6_adc_handle_t *handle, uint32_t timeout);

    /**
     * @brief 读取ADC转换结果
     * @param handle ADC句柄指针
     * @return 转换结果
     */
    uint16_t vet6_adc_read(vet6_adc_handle_t *handle);

    /**
     * @brief 单次转换并读取结果
     * @param handle ADC句柄指针
     * @param channel 通道号
     * @return 转换结果
     */
    uint16_t vet6_adc_read_channel(vet6_adc_handle_t *handle, vet6_adc_channel_t channel);

    /**
     * @brief 多次采样取平均值
     * @param handle ADC句柄指针
     * @param channel 通道号
     * @param samples 采样次数
     * @return 平均值
     */
    uint16_t vet6_adc_read_average(vet6_adc_handle_t *handle, vet6_adc_channel_t channel,
                                   uint8_t samples);

    /*===========================================================================*/
    /*                              特殊功能API                                   */
    /*===========================================================================*/

    /**
     * @brief 使能内部温度传感器和VREFINT
     */
    void vet6_adc_enable_temp_vref(void);

    /**
     * @brief 禁用内部温度传感器和VREFINT
     */
    void vet6_adc_disable_temp_vref(void);

    /**
     * @brief 使能VBAT测量
     */
    void vet6_adc_enable_vbat(void);

    /**
     * @brief 禁用VBAT测量
     */
    void vet6_adc_disable_vbat(void);

    /**
     * @brief 读取内部温度
     * @param handle ADC句柄指针 (必须是ADC1)
     * @return 温度值 (°C * 10, 如250表示25.0°C)
     */
    int16_t vet6_adc_read_temperature(vet6_adc_handle_t *handle);

    /**
     * @brief 读取内部参考电压
     * @param handle ADC句柄指针 (必须是ADC1)
     * @return VREFINT ADC值
     */
    uint16_t vet6_adc_read_vrefint(vet6_adc_handle_t *handle);

    /**
     * @brief 读取VBAT电压
     * @param handle ADC句柄指针 (必须是ADC1)
     * @return VBAT电压 (mV)
     */
    uint16_t vet6_adc_read_vbat(vet6_adc_handle_t *handle);

    /**
     * @brief ADC值转换为电压 (mV)
     * @param handle ADC句柄指针
     * @param adc_value ADC值
     * @param vref_mv 参考电压 (mV)
     * @return 电压值 (mV)
     */
    uint16_t vet6_adc_to_voltage(vet6_adc_handle_t *handle, uint16_t adc_value, uint16_t vref_mv);

    /*===========================================================================*/
    /*                              中断/DMA API                                  */
    /*===========================================================================*/

    /**
     * @brief 使能ADC转换完成中断
     * @param handle ADC句柄指针
     * @param priority 中断优先级
     */
    void vet6_adc_enable_eoc_irq(vet6_adc_handle_t *handle, uint8_t priority);

    /**
     * @brief 禁用ADC转换完成中断
     * @param handle ADC句柄指针
     */
    void vet6_adc_disable_eoc_irq(vet6_adc_handle_t *handle);

    /**
     * @brief 使能ADC DMA
     * @param handle ADC句柄指针
     */
    void vet6_adc_enable_dma(vet6_adc_handle_t *handle);

    /**
     * @brief 禁用ADC DMA
     * @param handle ADC句柄指针
     */
    void vet6_adc_disable_dma(vet6_adc_handle_t *handle);

#ifdef __cplusplus
}
#endif

#endif /* __VET6_ADC_H */
