/**
 * @file f103_tim.h
 * @brief STM32F103 系列定时器驱动头文件
 */

#ifndef __F103_TIM_H
#define __F103_TIM_H

#include "stm32f10x.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief 定时器端口枚举
     */
    typedef enum
    {
        F103_TIM1 = 0,
        F103_TIM2,
        F103_TIM3,
        F103_TIM4,
        F103_TIM_MAX
    } f103_tim_port_t;

    /**
     * @brief 定时器模式枚举
     */
    typedef enum
    {
        F103_TIM_MODE_TIMER = 0, /* 定时器模式 */
        F103_TIM_MODE_PWM,       /* PWM模式 */
        F103_TIM_MODE_ENCODER    /* 编码器模式 */
    } f103_tim_mode_t;

    /**
     * @brief PWM通道枚举
     */
    typedef enum
    {
        F103_TIM_CH1 = 0,
        F103_TIM_CH2,
        F103_TIM_CH3,
        F103_TIM_CH4,
        F103_TIM_CH_MAX
    } f103_tim_channel_t;

    /**
     * @brief 计数方向枚举
     */
    typedef enum
    {
        F103_TIM_COUNT_UP = 0, /* 向上计数 */
        F103_TIM_COUNT_DOWN    /* 向下计数 */
    } f103_tim_dir_t;

    /**
     * @brief 定时器配置结构体
     */
    typedef struct
    {
        f103_tim_port_t port; /* 定时器端口 */
        f103_tim_mode_t mode; /* 工作模式 */
        uint32_t prescaler;   /* 预分频值 */
        uint32_t period;      /* 自动重载值 */
        f103_tim_dir_t dir;   /* 计数方向 */
        bool enable_irq;      /* 是否使能中断 */
    } f103_tim_config_t;

    /**
     * @brief PWM配置结构体
     */
    typedef struct
    {
        f103_tim_port_t port;       /* 定时器端口 */
        f103_tim_channel_t channel; /* PWM通道 */
        uint32_t prescaler;         /* 预分频值 */
        uint32_t period;            /* PWM周期 */
        uint32_t pulse;             /* 脉冲宽度 */
        bool output_polarity;       /* 输出极性: true=高电平有效 */
    } f103_pwm_config_t;

    /**
     * @brief 初始化定时器
     * @param config 定时器配置
     * @return 0:成功 -1:失败
     */
    int f103_tim_init(const f103_tim_config_t *config);

    /**
     * @brief 快速初始化定时器（1ms周期）
     * @param port 定时器端口
     * @param enable_irq 是否使能中断
     * @return 0:成功 -1:失败
     */
    int f103_tim_init_1ms(f103_tim_port_t port, bool enable_irq);

    /**
     * @brief 启动定时器
     * @param port 定时器端口
     */
    void f103_tim_start(f103_tim_port_t port);

    /**
     * @brief 停止定时器
     * @param port 定时器端口
     */
    void f103_tim_stop(f103_tim_port_t port);

    /**
     * @brief 获取定时器计数值
     * @param port 定时器端口
     * @return 计数值
     */
    uint32_t f103_tim_get_count(f103_tim_port_t port);

    /**
     * @brief 设置定时器计数值
     * @param port 定时器端口
     * @param count 计数值
     */
    void f103_tim_set_count(f103_tim_port_t port, uint32_t count);

    /**
     * @brief 清除定时器中断标志
     * @param port 定时器端口
     */
    void f103_tim_clear_irq(f103_tim_port_t port);

    /**
     * @brief 初始化PWM
     * @param config PWM配置
     * @return 0:成功 -1:失败
     */
    int f103_pwm_init(const f103_pwm_config_t *config);

    /**
     * @brief 快速初始化PWM
     * @param port 定时器端口
     * @param channel PWM通道
     * @param freq PWM频率(Hz)
     * @param duty 占空比(0-100)
     * @return 0:成功 -1:失败
     */
    int f103_pwm_init_quick(f103_tim_port_t port, f103_tim_channel_t channel,
                            uint32_t freq, uint8_t duty);

    /**
     * @brief 设置PWM占空比
     * @param port 定时器端口
     * @param channel PWM通道
     * @param duty 占空比(0-100)
     */
    void f103_pwm_set_duty(f103_tim_port_t port, f103_tim_channel_t channel, uint8_t duty);

    /**
     * @brief 设置PWM脉冲宽度
     * @param port 定时器端口
     * @param channel PWM通道
     * @param pulse 脉冲宽度
     */
    void f103_pwm_set_pulse(f103_tim_port_t port, f103_tim_channel_t channel, uint32_t pulse);

    /**
     * @brief 启动PWM输出
     * @param port 定时器端口
     * @param channel PWM通道
     */
    void f103_pwm_start(f103_tim_port_t port, f103_tim_channel_t channel);

    /**
     * @brief 停止PWM输出
     * @param port 定时器端口
     * @param channel PWM通道
     */
    void f103_pwm_stop(f103_tim_port_t port, f103_tim_channel_t channel);

    /**
     * @brief 编码器模式初始化
     * @param port 定时器端口 (TIM2/3/4)
     * @return 0:成功 -1:失败
     */
    int f103_encoder_init(f103_tim_port_t port);

    /**
     * @brief 获取编码器计数值
     * @param port 定时器端口
     * @return 编码器计数值(带符号)
     */
    int32_t f103_encoder_get_count(f103_tim_port_t port);

    /**
     * @brief 复位编码器计数
     * @param port 定时器端口
     */
    void f103_encoder_reset(f103_tim_port_t port);

#ifdef __cplusplus
}
#endif

#endif /* __F103_TIM_H */
