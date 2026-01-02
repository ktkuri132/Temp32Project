/**
 * @file f407_tim.h
 * @brief STM32F407 系列定时器板级驱动头文件
 * @details 提供TIM1-TIM14的初始化和操作API
 *          适用于STM32F407系列所有封装型号（VET6, ZGT6, ZET6, VGT6等）
 *          - TIM1, TIM8: 高级定时器 (16位, APB2)
 *          - TIM2, TIM5: 通用定时器 (32位, APB1)
 *          - TIM3, TIM4: 通用定时器 (16位, APB1)
 *          - TIM9-TIM11: 通用定时器 (16位, APB2)
 *          - TIM12-TIM14: 通用定时器 (16位, APB1)
 *          - TIM6, TIM7: 基本定时器 (16位, APB1)
 */

#ifndef __F407_TIM_H
#define __F407_TIM_H

#include <stm32f4xx.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#ifdef __cplusplus
extern "C"
{
#endif

    /*===========================================================================*/
    /*                              类型定义                                      */
    /*===========================================================================*/

    /**
     * @brief 定时器编号枚举
     */
    typedef enum
    {
        F407_TIM1 = 0,
        F407_TIM2,
        F407_TIM3,
        F407_TIM4,
        F407_TIM5,
        F407_TIM6,
        F407_TIM7,
        F407_TIM8,
        F407_TIM9,
        F407_TIM10,
        F407_TIM11,
        F407_TIM12,
        F407_TIM13,
        F407_TIM14,
        F407_TIM_MAX
    } f407_tim_t;

    /**
     * @brief 定时器通道
     */
    typedef enum
    {
        F407_TIM_CH1 = 0,
        F407_TIM_CH2,
        F407_TIM_CH3,
        F407_TIM_CH4,
        F407_TIM_CH_MAX
    } f407_tim_ch_t;

    /**
     * @brief 定时器模式
     */
    typedef enum
    {
        F407_TIM_MODE_BASIC = 0,  // 基本定时模式
        F407_TIM_MODE_PWM = 1,    // PWM输出模式
        F407_TIM_MODE_IC = 2,     // 输入捕获模式
        F407_TIM_MODE_OC = 3,     // 输出比较模式
        F407_TIM_MODE_ENCODER = 4 // 编码器模式
    } f407_tim_mode_t;

    /**
     * @brief PWM模式
     */
    typedef enum
    {
        F407_PWM_MODE1 = 0, // 向上计数时, CNT<CCR为有效电平
        F407_PWM_MODE2 = 1  // 向上计数时, CNT<CCR为无效电平
    } f407_pwm_mode_t;

    /**
     * @brief 计数方向
     */
    typedef enum
    {
        F407_TIM_DIR_UP = 0,  // 向上计数
        F407_TIM_DIR_DOWN = 1 // 向下计数
    } f407_tim_dir_t;

    /**
     * @brief 定时器基本配置
     */
    typedef struct
    {
        f407_tim_t tim;     // 定时器编号
        uint32_t prescaler; // 预分频值 (0-65535)
        uint32_t period;    // 自动重载值
        f407_tim_dir_t dir; // 计数方向
        bool auto_reload;   // 是否自动重载
    } f407_tim_base_config_t;

    /**
     * @brief PWM配置结构体
     */
    typedef struct
    {
        f407_tim_t tim;        // 定时器编号
        f407_tim_ch_t channel; // 定时器通道
        uint32_t frequency;    // PWM频率 (Hz)
        uint16_t duty;         // 占空比 (0-1000, 表示0.0%-100.0%)
        f407_pwm_mode_t mode;  // PWM模式
        bool polarity;         // 极性 (true=高电平有效)
    } f407_pwm_config_t;

    /**
     * @brief 编码器配置结构体
     */
    typedef struct
    {
        f407_tim_t tim;  // 定时器编号 (仅TIM1-5, TIM8支持)
        uint16_t filter; // 输入滤波 (0-15)
        bool reverse;    // 反向计数
    } f407_encoder_config_t;

/*===========================================================================*/
/*                              宏定义                                        */
/*===========================================================================*/

/* PWM占空比范围 */
#define F407_PWM_DUTY_MIN 0    // 0%
#define F407_PWM_DUTY_MAX 1000 // 100%

/* 定时器时钟频率 (假设APB1=42MHz, APB2=84MHz, 定时器时钟x2) */
#define F407_TIM_APB1_CLK 84000000UL  // APB1定时器时钟
#define F407_TIM_APB2_CLK 168000000UL // APB2定时器时钟

    /*===========================================================================*/
    /*                              基本定时API                                   */
    /*===========================================================================*/

    /**
     * @brief 使能定时器时钟
     * @param tim 定时器编号
     */
    void f407_tim_clk_enable(f407_tim_t tim);

    /**
     * @brief 禁用定时器时钟
     * @param tim 定时器编号
     */
    void f407_tim_clk_disable(f407_tim_t tim);

    /**
     * @brief 获取定时器外设基地址
     * @param tim 定时器编号
     * @return 定时器外设指针
     */
    TIM_TypeDef *f407_tim_get_base(f407_tim_t tim);

    /**
     * @brief 初始化定时器基本配置
     * @param config 配置结构体指针
     * @return 0:成功, -1:失败
     */
    int f407_tim_base_init(const f407_tim_base_config_t *config);

    /**
     * @brief 快速初始化定时器为微秒级定时
     * @param tim 定时器编号
     * @param us 定时周期(微秒)
     * @return 0:成功, -1:失败
     */
    int f407_tim_init_us(f407_tim_t tim, uint32_t us);

    /**
     * @brief 快速初始化定时器为毫秒级定时
     * @param tim 定时器编号
     * @param ms 定时周期(毫秒)
     * @return 0:成功, -1:失败
     */
    int f407_tim_init_ms(f407_tim_t tim, uint32_t ms);

    /**
     * @brief 启动定时器
     * @param tim 定时器编号
     */
    void f407_tim_start(f407_tim_t tim);

    /**
     * @brief 停止定时器
     * @param tim 定时器编号
     */
    void f407_tim_stop(f407_tim_t tim);

    /**
     * @brief 获取定时器计数值
     * @param tim 定时器编号
     * @return 当前计数值
     */
    uint32_t f407_tim_get_counter(f407_tim_t tim);

    /**
     * @brief 设置定时器计数值
     * @param tim 定时器编号
     * @param value 计数值
     */
    void f407_tim_set_counter(f407_tim_t tim, uint32_t value);

    /**
     * @brief 设置自动重载值
     * @param tim 定时器编号
     * @param value 自动重载值
     */
    void f407_tim_set_period(f407_tim_t tim, uint32_t value);

    /**
     * @brief 使能定时器更新中断
     * @param tim 定时器编号
     * @param priority 中断优先级 (0-15)
     */
    void f407_tim_enable_update_irq(f407_tim_t tim, uint8_t priority);

    /**
     * @brief 禁用定时器更新中断
     * @param tim 定时器编号
     */
    void f407_tim_disable_update_irq(f407_tim_t tim);

    /**
     * @brief 清除更新中断标志
     * @param tim 定时器编号
     */
    void f407_tim_clear_update_flag(f407_tim_t tim);

    /**
     * @brief 检查更新中断标志
     * @param tim 定时器编号
     * @return true:已产生中断, false:未产生
     */
    bool f407_tim_check_update_flag(f407_tim_t tim);

    /*===========================================================================*/
    /*                              PWM输出API                                    */
    /*===========================================================================*/

    /**
     * @brief 初始化PWM输出
     * @param config PWM配置结构体指针
     * @return 0:成功, -1:失败
     */
    int f407_pwm_init(const f407_pwm_config_t *config);

    /**
     * @brief 快速初始化PWM (默认高电平有效)
     * @param tim 定时器编号
     * @param channel 通道
     * @param frequency 频率(Hz)
     * @param duty 占空比(0-1000)
     * @return 0:成功, -1:失败
     */
    int f407_pwm_init_quick(f407_tim_t tim, f407_tim_ch_t channel,
                            uint32_t frequency, uint16_t duty);

    /**
     * @brief 启动PWM输出
     * @param tim 定时器编号
     * @param channel 通道
     */
    void f407_pwm_start(f407_tim_t tim, f407_tim_ch_t channel);

    /**
     * @brief 停止PWM输出
     * @param tim 定时器编号
     * @param channel 通道
     */
    void f407_pwm_stop(f407_tim_t tim, f407_tim_ch_t channel);

    /**
     * @brief 设置PWM占空比
     * @param tim 定时器编号
     * @param channel 通道
     * @param duty 占空比(0-1000)
     */
    void f407_pwm_set_duty(f407_tim_t tim, f407_tim_ch_t channel, uint16_t duty);

    /**
     * @brief 设置PWM频率
     * @param tim 定时器编号
     * @param frequency 频率(Hz)
     */
    void f407_pwm_set_frequency(f407_tim_t tim, uint32_t frequency);

    /**
     * @brief 设置PWM比较值
     * @param tim 定时器编号
     * @param channel 通道
     * @param value 比较值
     */
    void f407_pwm_set_compare(f407_tim_t tim, f407_tim_ch_t channel, uint32_t value);

    /**
     * @brief 获取PWM比较值
     * @param tim 定时器编号
     * @param channel 通道
     * @return 比较值
     */
    uint32_t f407_pwm_get_compare(f407_tim_t tim, f407_tim_ch_t channel);

    /*===========================================================================*/
    /*                              编码器接口API                                 */
    /*===========================================================================*/

    /**
     * @brief 初始化编码器接口
     * @param config 编码器配置结构体指针
     * @return 0:成功, -1:失败
     */
    int f407_encoder_init(const f407_encoder_config_t *config);

    /**
     * @brief 获取编码器计数值
     * @param tim 定时器编号
     * @return 计数值 (有符号)
     */
    int32_t f407_encoder_get_count(f407_tim_t tim);

    /**
     * @brief 重置编码器计数
     * @param tim 定时器编号
     */
    void f407_encoder_reset(f407_tim_t tim);

    /**
     * @brief 获取编码器方向
     * @param tim 定时器编号
     * @return 0:正转, 1:反转
     */
    uint8_t f407_encoder_get_direction(f407_tim_t tim);

    /*===========================================================================*/
    /*                              高级定时器API                                 */
    /*===========================================================================*/

    /**
     * @brief 使能高级定时器主输出
     * @param tim 定时器编号 (仅TIM1, TIM8)
     */
    void f407_tim_enable_moe(f407_tim_t tim);

    /**
     * @brief 禁用高级定时器主输出
     * @param tim 定时器编号 (仅TIM1, TIM8)
     */
    void f407_tim_disable_moe(f407_tim_t tim);

    /**
     * @brief 配置死区时间
     * @param tim 定时器编号 (仅TIM1, TIM8)
     * @param deadtime 死区时间值 (0-255)
     */
    void f407_tim_set_deadtime(f407_tim_t tim, uint8_t deadtime);

    /**
     * @brief 配置刹车功能
     * @param tim 定时器编号 (仅TIM1, TIM8)
     * @param enable 使能/禁用
     * @param polarity 刹车输入极性 (true=高电平有效)
     */
    void f407_tim_config_break(f407_tim_t tim, bool enable, bool polarity);

    /*===========================================================================*/
    /*                              辅助功能                                      */
    /*===========================================================================*/

    /**
     * @brief 获取定时器时钟频率
     * @param tim 定时器编号
     * @return 时钟频率(Hz)
     */
    uint32_t f407_tim_get_clk(f407_tim_t tim);

    /**
     * @brief 检查定时器是否支持32位计数
     * @param tim 定时器编号
     * @return true:32位, false:16位
     */
    bool f407_tim_is_32bit(f407_tim_t tim);

    /**
     * @brief 检查定时器是否为高级定时器
     * @param tim 定时器编号
     * @return true:高级定时器, false:通用/基本定时器
     */
    bool f407_tim_is_advanced(f407_tim_t tim);

    /**
     * @brief 软件触发更新事件
     * @param tim 定时器编号
     */
    void f407_tim_generate_update(f407_tim_t tim);

#ifdef __cplusplus
}
#endif

#endif /* __F407_TIM_H */
