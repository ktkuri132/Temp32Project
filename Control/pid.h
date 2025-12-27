/**
 * @file pid.h
 * @brief PID控制器库
 * @details 支持位置式PID、增量式PID，支持参数滤波对接
 */

#ifndef __PID_H
#define __PID_H

#include <stdint.h>
#include "filter.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*============================================================================
 *                              PID配置
 *============================================================================*/
#define PID_USE_DERIVATIVE_FILTER 1 // 是否对微分项使用滤波 (推荐开启)
#define PID_USE_SETPOINT_FILTER 0   // 是否对设定值使用滤波
#define PID_USE_FEEDBACK_FILTER 0   // 是否对反馈值使用滤波
#define PID_USE_OUTPUT_FILTER 0     // 是否对输出使用滤波

    /*============================================================================
     *                              PID类型定义
     *============================================================================*/

    /**
     * @brief PID参数滤波器配置
     */
    typedef struct
    {
        FilterInterface_t setpoint_filter;   // 设定值滤波器
        FilterInterface_t feedback_filter;   // 反馈值滤波器
        FilterInterface_t derivative_filter; // 微分项滤波器
        FilterInterface_t output_filter;     // 输出滤波器

        uint8_t enable_setpoint_filter;   // 使能设定值滤波
        uint8_t enable_feedback_filter;   // 使能反馈值滤波
        uint8_t enable_derivative_filter; // 使能微分项滤波
        uint8_t enable_output_filter;     // 使能输出滤波
    } PID_FilterConfig_t;

    /**
     * @brief PID控制器结构体 (位置式)
     */
    typedef struct
    {
        /* PID参数 */
        float Kp; // 比例系数
        float Ki; // 积分系数
        float Kd; // 微分系数

        /* 运行时变量 */
        float setpoint;   // 目标值
        float feedback;   // 反馈值
        float error;      // 当前误差
        float last_error; // 上次误差
        float prev_error; // 上上次误差 (用于微分平滑)
        float integral;   // 积分累加
        float derivative; // 微分值
        float output;     // 控制输出

        /* 限幅参数 */
        float output_max;   // 输出上限
        float output_min;   // 输出下限
        float integral_max; // 积分上限
        float integral_min; // 积分下限

        /* 抗积分饱和 */
        float deadband;      // 死区大小
        uint8_t anti_windup; // 抗积分饱和使能

        /* 微分先行 */
        uint8_t derivative_on_measurement; // 基于测量值微分而非误差
        float last_feedback;               // 上次反馈值 (用于微分先行)

        /* 滤波器配置 */
        PID_FilterConfig_t filters;

        /* 状态 */
        uint8_t initialized; // 初始化标志
        float dt;            // 采样周期 (秒)
    } PID_Controller_t;

    /**
     * @brief 增量式PID控制器结构体
     */
    typedef struct
    {
        /* PID参数 */
        float Kp; // 比例系数
        float Ki; // 积分系数
        float Kd; // 微分系数

        /* 运行时变量 */
        float setpoint;     // 目标值
        float feedback;     // 反馈值
        float error;        // 当前误差
        float last_error;   // 上次误差
        float prev_error;   // 上上次误差
        float delta_output; // 增量输出
        float output;       // 累计输出

        /* 限幅参数 */
        float output_max; // 输出上限
        float output_min; // 输出下限
        float delta_max;  // 单次增量上限

        /* 滤波器配置 */
        PID_FilterConfig_t filters;

        /* 状态 */
        uint8_t initialized; // 初始化标志
        float dt;            // 采样周期 (秒)
    } PID_Incremental_t;

    /**
     * @brief PID参数配置结构体 (用于初始化)
     */
    typedef struct
    {
        float Kp;
        float Ki;
        float Kd;
        float dt; // 采样周期 (秒)
        float output_max;
        float output_min;
        float integral_max;
        float integral_min;
        float deadband;
        uint8_t anti_windup;
        uint8_t derivative_on_measurement;
    } PID_Config_t;

    /*============================================================================
     *                              位置式PID函数
     *============================================================================*/

    /**
     * @brief 初始化位置式PID控制器
     */
    void PID_Init(PID_Controller_t *pid, PID_Config_t *config);

    /**
     * @brief 设置PID参数
     */
    void PID_SetParams(PID_Controller_t *pid, float Kp, float Ki, float Kd);

    /**
     * @brief 设置输出限幅
     */
    void PID_SetOutputLimits(PID_Controller_t *pid, float min, float max);

    /**
     * @brief 设置积分限幅
     */
    void PID_SetIntegralLimits(PID_Controller_t *pid, float min, float max);

    /**
     * @brief 设置目标值
     */
    void PID_SetSetpoint(PID_Controller_t *pid, float setpoint);

    /**
     * @brief 更新PID控制器
     * @param pid PID控制器指针
     * @param feedback 反馈值
     * @return 控制输出
     */
    float PID_Update(PID_Controller_t *pid, float feedback);

    /**
     * @brief 重置PID控制器
     */
    void PID_Reset(PID_Controller_t *pid);

    /**
     * @brief 获取各项分量
     */
    float PID_GetProportional(PID_Controller_t *pid);
    float PID_GetIntegral(PID_Controller_t *pid);
    float PID_GetDerivative(PID_Controller_t *pid);
    float PID_GetError(PID_Controller_t *pid);

    /*============================================================================
     *                              增量式PID函数
     *============================================================================*/

    /**
     * @brief 初始化增量式PID控制器
     */
    void PID_Inc_Init(PID_Incremental_t *pid, PID_Config_t *config);

    /**
     * @brief 设置增量式PID参数
     */
    void PID_Inc_SetParams(PID_Incremental_t *pid, float Kp, float Ki, float Kd);

    /**
     * @brief 设置增量式PID输出限幅
     */
    void PID_Inc_SetOutputLimits(PID_Incremental_t *pid, float min, float max);

    /**
     * @brief 设置单次增量限幅
     */
    void PID_Inc_SetDeltaLimit(PID_Incremental_t *pid, float delta_max);

    /**
     * @brief 设置目标值
     */
    void PID_Inc_SetSetpoint(PID_Incremental_t *pid, float setpoint);

    /**
     * @brief 更新增量式PID控制器
     * @param pid PID控制器指针
     * @param feedback 反馈值
     * @return 控制输出
     */
    float PID_Inc_Update(PID_Incremental_t *pid, float feedback);

    /**
     * @brief 重置增量式PID控制器
     */
    void PID_Inc_Reset(PID_Incremental_t *pid);

    /*============================================================================
     *                              滤波器对接函数
     *============================================================================*/

    /**
     * @brief 设置设定值滤波器
     */
    void PID_SetSetpointFilter(PID_Controller_t *pid, void *filter, FilterFunc_t func);

    /**
     * @brief 设置反馈值滤波器
     */
    void PID_SetFeedbackFilter(PID_Controller_t *pid, void *filter, FilterFunc_t func);

    /**
     * @brief 设置微分项滤波器
     */
    void PID_SetDerivativeFilter(PID_Controller_t *pid, void *filter, FilterFunc_t func);

    /**
     * @brief 设置输出滤波器
     */
    void PID_SetOutputFilter(PID_Controller_t *pid, void *filter, FilterFunc_t func);

    /**
     * @brief 使能/禁用滤波器
     */
    void PID_EnableSetpointFilter(PID_Controller_t *pid, uint8_t enable);
    void PID_EnableFeedbackFilter(PID_Controller_t *pid, uint8_t enable);
    void PID_EnableDerivativeFilter(PID_Controller_t *pid, uint8_t enable);
    void PID_EnableOutputFilter(PID_Controller_t *pid, uint8_t enable);

    /* 增量式PID滤波器设置 */
    void PID_Inc_SetSetpointFilter(PID_Incremental_t *pid, void *filter, FilterFunc_t func);
    void PID_Inc_SetFeedbackFilter(PID_Incremental_t *pid, void *filter, FilterFunc_t func);
    void PID_Inc_SetOutputFilter(PID_Incremental_t *pid, void *filter, FilterFunc_t func);

#ifdef __cplusplus
}
#endif

#endif /* __PID_H */
