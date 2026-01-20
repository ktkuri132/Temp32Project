/**
 * @file adc.c
 * @brief STM32F4 ADC 片上外设驱动
 * @note 使用 f407_adc 底层驱动实现 df_adc.h 接口
 * @note 片上外设通过 DF_BOARD_INIT 分散加载初始化
 */

#include "driver.h"
#include "df_adc.h"
#include "df_init.h"
#include "f407_adc.h"

/*============================ ADC 配置 ============================*/
/* 默认使用 PA0 (ADC1 通道0) */
#ifndef DEFAULT_ADC_CHANNEL
#define DEFAULT_ADC_CHANNEL F407_ADC_CH0
#endif

/*============================ 内部变量 ============================*/
static f407_adc_handle_t adc1_handle;

/*============================ 前向声明 ============================*/
int adc1_init(df_arg_t arg);
int adc1_deinit(df_arg_t arg);
int adc1_get_value(df_arg_t arg);

/*============================ ADC 设备实例 ============================*/
df_adc_t adc1 = {
    .init_flag = false,
    .num = 1,
    .name = ADC1_NAME,
    .init = adc1_init,
    .deinit = adc1_deinit,
    .get_value = adc1_get_value};

/*============================ 接口实现 ============================*/

/**
 * @brief 初始化 ADC1
 */
int adc1_init(df_arg_t arg)
{
    (void)arg;

    /* 使用 f407 驱动初始化 ADC1 */
    if (f407_adc_init_simple(&adc1_handle, F407_ADC1) != 0)
    {
        return -1;
    }

    /* 配置默认通道引脚 */
    f407_adc_pin_init(DEFAULT_ADC_CHANNEL);

    /* 配置默认通道 */
    f407_adc_config_channel(&adc1_handle, DEFAULT_ADC_CHANNEL, 1, F407_ADC_SAMPLETIME_480);

    /* 使能 ADC */
    f407_adc_enable(&adc1_handle);

    adc1.init_flag = true;
    return 0;
}

/**
 * @brief 关闭 ADC1
 */
int adc1_deinit(df_arg_t arg)
{
    (void)arg;

    /* 反初始化 ADC1 */
    f407_adc_deinit(&adc1_handle);
    adc1.init_flag = false;

    return 0;
}

/**
 * @brief 获取 ADC 值
 * @note arg.us32 用于指定通道号（可选），默认使用 CH0
 */
int adc1_get_value(df_arg_t arg)
{
    if (!adc1.init_flag)
    {
        return -1;
    }

    /* 可通过 arg.us32 指定通道，默认 CH0 */
    f407_adc_channel_t channel = DEFAULT_ADC_CHANNEL;
    if (arg.us32 < F407_ADC_CH_MAX)
    {
        channel = (f407_adc_channel_t)arg.us32;
    }

    /* 读取 ADC 值 */
    uint16_t value = f407_adc_read_channel(&adc1_handle, channel);

    return (int)value;
}

/*============================ 扩展接口 ============================*/

/**
 * @brief 读取指定通道的 ADC 电压值 (mV)
 * @param channel ADC 通道
 * @param vref 参考电压 (mV)，默认 3300
 * @return 电压值 (mV)
 */
uint32_t adc1_read_voltage(f407_adc_channel_t channel, uint32_t vref)
{
    if (!adc1.init_flag)
    {
        return 0;
    }

    uint16_t adc_value = f407_adc_read_channel(&adc1_handle, channel);
    return f407_adc_to_voltage(&adc1_handle, adc_value, (uint16_t)vref);
}

/**
 * @brief 读取内部温度传感器
 * @return 温度值 (摄氏度 * 10)
 */
int16_t adc1_read_temperature(void)
{
    if (!adc1.init_flag)
    {
        return 0;
    }

    f407_adc_enable_temp_vref();
    int16_t temp = f407_adc_read_temperature(&adc1_handle);
    return temp;
}

/*============================ 片上外设自动初始化 ============================*/

/**
 * @brief ADC1 自动初始化函数
 * @note 通过 DF_BOARD_INIT 宏在系统启动时自动调用
 */
static int adc1_auto_init(void)
{
    return adc1_init(arg_null);
}
DF_BOARD_INIT(adc1_auto_init);
