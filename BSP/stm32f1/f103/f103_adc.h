/**
 * @file f103_adc.h
 * @brief STM32F103 系列 ADC 驱动头文件
 */

#ifndef __F103_ADC_H
#define __F103_ADC_H

#include "stm32f10x.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief ADC端口枚举
     */
    typedef enum
    {
        F103_ADC1 = 0,
        F103_ADC2,
        F103_ADC_MAX
    } f103_adc_port_t;

    /**
     * @brief ADC通道枚举
     */
    typedef enum
    {
        F103_ADC_CH0 = 0,
        F103_ADC_CH1,
        F103_ADC_CH2,
        F103_ADC_CH3,
        F103_ADC_CH4,
        F103_ADC_CH5,
        F103_ADC_CH6,
        F103_ADC_CH7,
        F103_ADC_CH8,
        F103_ADC_CH9,
        F103_ADC_CH10,
        F103_ADC_CH11,
        F103_ADC_CH12,
        F103_ADC_CH13,
        F103_ADC_CH14,
        F103_ADC_CH15,
        F103_ADC_CH16, /* 内部温度传感器 */
        F103_ADC_CH17, /* VREFINT */
        F103_ADC_CH_MAX
    } f103_adc_channel_t;

    /**
     * @brief ADC采样时间枚举
     */
    typedef enum
    {
        F103_ADC_SAMPLETIME_1_5 = 0,  /* 1.5 cycles */
        F103_ADC_SAMPLETIME_7_5 = 1,  /* 7.5 cycles */
        F103_ADC_SAMPLETIME_13_5 = 2, /* 13.5 cycles */
        F103_ADC_SAMPLETIME_28_5 = 3, /* 28.5 cycles */
        F103_ADC_SAMPLETIME_41_5 = 4, /* 41.5 cycles */
        F103_ADC_SAMPLETIME_55_5 = 5, /* 55.5 cycles */
        F103_ADC_SAMPLETIME_71_5 = 6, /* 71.5 cycles */
        F103_ADC_SAMPLETIME_239_5 = 7 /* 239.5 cycles */
    } f103_adc_sampletime_t;

    /**
     * @brief ADC配置结构体
     */
    typedef struct
    {
        f103_adc_port_t port;              /* ADC端口 */
        bool continuous;                   /* 连续转换模式 */
        bool scan;                         /* 扫描模式 */
        f103_adc_sampletime_t sample_time; /* 默认采样时间 */
    } f103_adc_config_t;

    /**
     * @brief 初始化ADC
     * @param config ADC配置
     * @return 0:成功 -1:失败
     */
    int f103_adc_init(const f103_adc_config_t *config);

    /**
     * @brief 快速初始化ADC（单次转换模式）
     * @param port ADC端口
     * @return 0:成功 -1:失败
     */
    int f103_adc_init_quick(f103_adc_port_t port);

    /**
     * @brief 配置ADC通道
     * @param port ADC端口
     * @param channel ADC通道
     * @param sample_time 采样时间
     * @return 0:成功 -1:失败
     */
    int f103_adc_config_channel(f103_adc_port_t port, f103_adc_channel_t channel,
                                f103_adc_sampletime_t sample_time);

    /**
     * @brief 读取单通道ADC值
     * @param port ADC端口
     * @param channel ADC通道
     * @return ADC转换值(12位)
     */
    uint16_t f103_adc_read(f103_adc_port_t port, f103_adc_channel_t channel);

    /**
     * @brief 读取ADC电压值(mV)
     * @param port ADC端口
     * @param channel ADC通道
     * @param vref 参考电压(mV)
     * @return 电压值(mV)
     */
    uint32_t f103_adc_read_voltage(f103_adc_port_t port, f103_adc_channel_t channel,
                                   uint32_t vref);

    /**
     * @brief 启动ADC软件触发转换
     * @param port ADC端口
     */
    void f103_adc_start(f103_adc_port_t port);

    /**
     * @brief 等待ADC转换完成
     * @param port ADC端口
     * @return 0:成功 -1:超时
     */
    int f103_adc_wait(f103_adc_port_t port);

    /**
     * @brief 读取ADC转换结果
     * @param port ADC端口
     * @return ADC值
     */
    uint16_t f103_adc_get_value(f103_adc_port_t port);

    /**
     * @brief 使能内部温度传感器
     */
    void f103_adc_enable_temp_sensor(void);

    /**
     * @brief 读取内部温度
     * @return 温度值(摄氏度*10)
     */
    int16_t f103_adc_read_temperature(void);

    /**
     * @brief ADC校准
     * @param port ADC端口
     */
    void f103_adc_calibrate(f103_adc_port_t port);

    /**
     * @brief 多通道DMA采集初始化
     * @param port ADC端口
     * @param channels 通道数组
     * @param channel_count 通道数量
     * @param buffer 数据缓冲区
     * @return 0:成功 -1:失败
     */
    int f103_adc_dma_init(f103_adc_port_t port, const f103_adc_channel_t *channels,
                          uint8_t channel_count, uint16_t *buffer);

    /**
     * @brief 启动DMA采集
     * @param port ADC端口
     */
    void f103_adc_dma_start(f103_adc_port_t port);

#ifdef __cplusplus
}
#endif

#endif /* __F103_ADC_H */
