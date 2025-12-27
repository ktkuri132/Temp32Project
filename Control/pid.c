/**
 * @file pid.c
 * @brief PID控制器库实现
 */

#include "pid.h"
#include <string.h>

/*============================================================================
 *                              辅助宏
 *============================================================================*/
#define LIMIT(val, min, max) ((val) < (min) ? (min) : ((val) > (max) ? (max) : (val)))
#define ABS(x) ((x) >= 0 ? (x) : -(x))

/*============================================================================
 *                              位置式PID实现
 *============================================================================*/

/**
 * @brief 初始化位置式PID控制器
 */
void PID_Init(PID_Controller_t *pid, PID_Config_t *config)
{
    if (pid == NULL)
        return;

    memset(pid, 0, sizeof(PID_Controller_t));

    if (config != NULL)
    {
        pid->Kp = config->Kp;
        pid->Ki = config->Ki;
        pid->Kd = config->Kd;
        pid->dt = config->dt;
        pid->output_max = config->output_max;
        pid->output_min = config->output_min;
        pid->integral_max = config->integral_max;
        pid->integral_min = config->integral_min;
        pid->deadband = config->deadband;
        pid->anti_windup = config->anti_windup;
        pid->derivative_on_measurement = config->derivative_on_measurement;
    }
    else
    {
        // 默认配置
        pid->Kp = 1.0f;
        pid->Ki = 0.0f;
        pid->Kd = 0.0f;
        pid->dt = 0.01f; // 10ms
        pid->output_max = 1000.0f;
        pid->output_min = -1000.0f;
        pid->integral_max = 500.0f;
        pid->integral_min = -500.0f;
        pid->deadband = 0.0f;
        pid->anti_windup = 1;
        pid->derivative_on_measurement = 0;
    }

    pid->initialized = 1;
}

/**
 * @brief 设置PID参数
 */
void PID_SetParams(PID_Controller_t *pid, float Kp, float Ki, float Kd)
{
    if (pid == NULL)
        return;
    pid->Kp = Kp;
    pid->Ki = Ki;
    pid->Kd = Kd;
}

/**
 * @brief 设置输出限幅
 */
void PID_SetOutputLimits(PID_Controller_t *pid, float min, float max)
{
    if (pid == NULL)
        return;
    pid->output_min = min;
    pid->output_max = max;
}

/**
 * @brief 设置积分限幅
 */
void PID_SetIntegralLimits(PID_Controller_t *pid, float min, float max)
{
    if (pid == NULL)
        return;
    pid->integral_min = min;
    pid->integral_max = max;
}

/**
 * @brief 设置目标值
 */
void PID_SetSetpoint(PID_Controller_t *pid, float setpoint)
{
    if (pid == NULL)
        return;

    // 应用设定值滤波
    if (pid->filters.enable_setpoint_filter && pid->filters.setpoint_filter.update != NULL)
    {
        pid->setpoint = Filter_Apply(&pid->filters.setpoint_filter, setpoint);
    }
    else
    {
        pid->setpoint = setpoint;
    }
}

/**
 * @brief 更新位置式PID控制器
 */
float PID_Update(PID_Controller_t *pid, float feedback)
{
    if (pid == NULL || !pid->initialized)
        return 0.0f;

    float P_term, I_term, D_term;

    // 应用反馈值滤波
    if (pid->filters.enable_feedback_filter && pid->filters.feedback_filter.update != NULL)
    {
        pid->feedback = Filter_Apply(&pid->filters.feedback_filter, feedback);
    }
    else
    {
        pid->feedback = feedback;
    }

    // 计算误差
    pid->error = pid->setpoint - pid->feedback;

    // 死区处理
    if (ABS(pid->error) < pid->deadband)
    {
        pid->error = 0.0f;
    }

    // 比例项
    P_term = pid->Kp * pid->error;

    // 积分项
    if (pid->Ki != 0.0f)
    {
        pid->integral += pid->error * pid->dt;

        // 积分限幅
        pid->integral = LIMIT(pid->integral, pid->integral_min, pid->integral_max);

        I_term = pid->Ki * pid->integral;
    }
    else
    {
        I_term = 0.0f;
    }

    // 微分项
    if (pid->Kd != 0.0f)
    {
        if (pid->derivative_on_measurement)
        {
            // 微分先行：基于测量值变化计算微分，避免设定值突变引起的微分冲击
            pid->derivative = -(pid->feedback - pid->last_feedback) / pid->dt;
        }
        else
        {
            // 标准微分：基于误差变化计算
            pid->derivative = (pid->error - pid->last_error) / pid->dt;
        }

        // 应用微分项滤波 (对噪声很敏感，建议开启)
        if (pid->filters.enable_derivative_filter && pid->filters.derivative_filter.update != NULL)
        {
            pid->derivative = Filter_Apply(&pid->filters.derivative_filter, pid->derivative);
        }

        D_term = pid->Kd * pid->derivative;
    }
    else
    {
        D_term = 0.0f;
    }

    // 计算输出
    pid->output = P_term + I_term + D_term;

    // 抗积分饱和 (Back-calculation)
    if (pid->anti_windup && pid->Ki != 0.0f)
    {
        float output_saturated = LIMIT(pid->output, pid->output_min, pid->output_max);
        if (pid->output != output_saturated)
        {
            // 输出饱和时，修正积分项
            pid->integral -= (pid->output - output_saturated) / pid->Ki;
            pid->integral = LIMIT(pid->integral, pid->integral_min, pid->integral_max);
            pid->output = output_saturated;
        }
    }

    // 输出限幅
    pid->output = LIMIT(pid->output, pid->output_min, pid->output_max);

    // 应用输出滤波
    if (pid->filters.enable_output_filter && pid->filters.output_filter.update != NULL)
    {
        pid->output = Filter_Apply(&pid->filters.output_filter, pid->output);
    }

    // 保存历史值
    pid->prev_error = pid->last_error;
    pid->last_error = pid->error;
    pid->last_feedback = pid->feedback;

    return pid->output;
}

/**
 * @brief 重置PID控制器
 */
void PID_Reset(PID_Controller_t *pid)
{
    if (pid == NULL)
        return;

    pid->setpoint = 0.0f;
    pid->feedback = 0.0f;
    pid->error = 0.0f;
    pid->last_error = 0.0f;
    pid->prev_error = 0.0f;
    pid->integral = 0.0f;
    pid->derivative = 0.0f;
    pid->output = 0.0f;
    pid->last_feedback = 0.0f;
}

/**
 * @brief 获取比例项
 */
float PID_GetProportional(PID_Controller_t *pid)
{
    if (pid == NULL)
        return 0.0f;
    return pid->Kp * pid->error;
}

/**
 * @brief 获取积分项
 */
float PID_GetIntegral(PID_Controller_t *pid)
{
    if (pid == NULL)
        return 0.0f;
    return pid->Ki * pid->integral;
}

/**
 * @brief 获取微分项
 */
float PID_GetDerivative(PID_Controller_t *pid)
{
    if (pid == NULL)
        return 0.0f;
    return pid->Kd * pid->derivative;
}

/**
 * @brief 获取当前误差
 */
float PID_GetError(PID_Controller_t *pid)
{
    if (pid == NULL)
        return 0.0f;
    return pid->error;
}

/*============================================================================
 *                              增量式PID实现
 *============================================================================*/

/**
 * @brief 初始化增量式PID控制器
 */
void PID_Inc_Init(PID_Incremental_t *pid, PID_Config_t *config)
{
    if (pid == NULL)
        return;

    memset(pid, 0, sizeof(PID_Incremental_t));

    if (config != NULL)
    {
        pid->Kp = config->Kp;
        pid->Ki = config->Ki;
        pid->Kd = config->Kd;
        pid->dt = config->dt;
        pid->output_max = config->output_max;
        pid->output_min = config->output_min;
        pid->delta_max = (config->output_max - config->output_min) * 0.1f; // 默认10%
    }
    else
    {
        pid->Kp = 1.0f;
        pid->Ki = 0.0f;
        pid->Kd = 0.0f;
        pid->dt = 0.01f;
        pid->output_max = 1000.0f;
        pid->output_min = -1000.0f;
        pid->delta_max = 100.0f;
    }

    pid->initialized = 1;
}

/**
 * @brief 设置增量式PID参数
 */
void PID_Inc_SetParams(PID_Incremental_t *pid, float Kp, float Ki, float Kd)
{
    if (pid == NULL)
        return;
    pid->Kp = Kp;
    pid->Ki = Ki;
    pid->Kd = Kd;
}

/**
 * @brief 设置增量式PID输出限幅
 */
void PID_Inc_SetOutputLimits(PID_Incremental_t *pid, float min, float max)
{
    if (pid == NULL)
        return;
    pid->output_min = min;
    pid->output_max = max;
}

/**
 * @brief 设置单次增量限幅
 */
void PID_Inc_SetDeltaLimit(PID_Incremental_t *pid, float delta_max)
{
    if (pid == NULL)
        return;
    pid->delta_max = delta_max;
}

/**
 * @brief 设置目标值
 */
void PID_Inc_SetSetpoint(PID_Incremental_t *pid, float setpoint)
{
    if (pid == NULL)
        return;

    if (pid->filters.enable_setpoint_filter && pid->filters.setpoint_filter.update != NULL)
    {
        pid->setpoint = Filter_Apply(&pid->filters.setpoint_filter, setpoint);
    }
    else
    {
        pid->setpoint = setpoint;
    }
}

/**
 * @brief 更新增量式PID控制器
 * @note 增量式PID: Δu = Kp*(e[k]-e[k-1]) + Ki*e[k] + Kd*(e[k]-2*e[k-1]+e[k-2])
 */
float PID_Inc_Update(PID_Incremental_t *pid, float feedback)
{
    if (pid == NULL || !pid->initialized)
        return 0.0f;

    // 应用反馈值滤波
    if (pid->filters.enable_feedback_filter && pid->filters.feedback_filter.update != NULL)
    {
        pid->feedback = Filter_Apply(&pid->filters.feedback_filter, feedback);
    }
    else
    {
        pid->feedback = feedback;
    }

    // 计算误差
    pid->error = pid->setpoint - pid->feedback;

    // 计算增量
    // Δu = Kp*(e[k]-e[k-1]) + Ki*e[k]*dt + Kd*(e[k]-2*e[k-1]+e[k-2])/dt
    float delta_P = pid->Kp * (pid->error - pid->last_error);
    float delta_I = pid->Ki * pid->error * pid->dt;
    float delta_D = pid->Kd * (pid->error - 2.0f * pid->last_error + pid->prev_error) / pid->dt;

    pid->delta_output = delta_P + delta_I + delta_D;

    // 增量限幅
    pid->delta_output = LIMIT(pid->delta_output, -pid->delta_max, pid->delta_max);

    // 累加输出
    pid->output += pid->delta_output;

    // 输出限幅
    pid->output = LIMIT(pid->output, pid->output_min, pid->output_max);

    // 应用输出滤波
    if (pid->filters.enable_output_filter && pid->filters.output_filter.update != NULL)
    {
        pid->output = Filter_Apply(&pid->filters.output_filter, pid->output);
    }

    // 保存历史误差
    pid->prev_error = pid->last_error;
    pid->last_error = pid->error;

    return pid->output;
}

/**
 * @brief 重置增量式PID控制器
 */
void PID_Inc_Reset(PID_Incremental_t *pid)
{
    if (pid == NULL)
        return;

    pid->setpoint = 0.0f;
    pid->feedback = 0.0f;
    pid->error = 0.0f;
    pid->last_error = 0.0f;
    pid->prev_error = 0.0f;
    pid->delta_output = 0.0f;
    pid->output = 0.0f;
}

/*============================================================================
 *                              位置式PID滤波器设置
 *============================================================================*/

/**
 * @brief 设置设定值滤波器
 */
void PID_SetSetpointFilter(PID_Controller_t *pid, void *filter, FilterFunc_t func)
{
    if (pid == NULL)
        return;
    Filter_SetInterface(&pid->filters.setpoint_filter, filter, func);
    pid->filters.enable_setpoint_filter = 1;
}

/**
 * @brief 设置反馈值滤波器
 */
void PID_SetFeedbackFilter(PID_Controller_t *pid, void *filter, FilterFunc_t func)
{
    if (pid == NULL)
        return;
    Filter_SetInterface(&pid->filters.feedback_filter, filter, func);
    pid->filters.enable_feedback_filter = 1;
}

/**
 * @brief 设置微分项滤波器
 */
void PID_SetDerivativeFilter(PID_Controller_t *pid, void *filter, FilterFunc_t func)
{
    if (pid == NULL)
        return;
    Filter_SetInterface(&pid->filters.derivative_filter, filter, func);
    pid->filters.enable_derivative_filter = 1;
}

/**
 * @brief 设置输出滤波器
 */
void PID_SetOutputFilter(PID_Controller_t *pid, void *filter, FilterFunc_t func)
{
    if (pid == NULL)
        return;
    Filter_SetInterface(&pid->filters.output_filter, filter, func);
    pid->filters.enable_output_filter = 1;
}

/**
 * @brief 使能/禁用设定值滤波器
 */
void PID_EnableSetpointFilter(PID_Controller_t *pid, uint8_t enable)
{
    if (pid == NULL)
        return;
    pid->filters.enable_setpoint_filter = enable;
}

/**
 * @brief 使能/禁用反馈值滤波器
 */
void PID_EnableFeedbackFilter(PID_Controller_t *pid, uint8_t enable)
{
    if (pid == NULL)
        return;
    pid->filters.enable_feedback_filter = enable;
}

/**
 * @brief 使能/禁用微分项滤波器
 */
void PID_EnableDerivativeFilter(PID_Controller_t *pid, uint8_t enable)
{
    if (pid == NULL)
        return;
    pid->filters.enable_derivative_filter = enable;
}

/**
 * @brief 使能/禁用输出滤波器
 */
void PID_EnableOutputFilter(PID_Controller_t *pid, uint8_t enable)
{
    if (pid == NULL)
        return;
    pid->filters.enable_output_filter = enable;
}

/*============================================================================
 *                              增量式PID滤波器设置
 *============================================================================*/

/**
 * @brief 设置增量式PID设定值滤波器
 */
void PID_Inc_SetSetpointFilter(PID_Incremental_t *pid, void *filter, FilterFunc_t func)
{
    if (pid == NULL)
        return;
    Filter_SetInterface(&pid->filters.setpoint_filter, filter, func);
    pid->filters.enable_setpoint_filter = 1;
}

/**
 * @brief 设置增量式PID反馈值滤波器
 */
void PID_Inc_SetFeedbackFilter(PID_Incremental_t *pid, void *filter, FilterFunc_t func)
{
    if (pid == NULL)
        return;
    Filter_SetInterface(&pid->filters.feedback_filter, filter, func);
    pid->filters.enable_feedback_filter = 1;
}

/**
 * @brief 设置增量式PID输出滤波器
 */
void PID_Inc_SetOutputFilter(PID_Incremental_t *pid, void *filter, FilterFunc_t func)
{
    if (pid == NULL)
        return;
    Filter_SetInterface(&pid->filters.output_filter, filter, func);
    pid->filters.enable_output_filter = 1;
}
