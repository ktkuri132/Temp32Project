/**
 * @file filter.c
 * @brief 常用滤波算法库实现
 */

#include "filter.h"
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

/*============================================================================
 *                              一阶低通滤波器
 *============================================================================*/

/**
 * @brief 初始化一阶低通滤波器
 * @param filter 滤波器指针
 * @param alpha 滤波系数 (0~1), 越小滤波效果越强, 推荐0.1~0.3
 */
void LowPass_Init(LowPassFilter_t *filter, float alpha)
{
    if (filter == NULL)
        return;

    filter->alpha = alpha;
    filter->last_output = 0.0f;
    filter->initialized = 0;
}

/**
 * @brief 一阶低通滤波更新
 * @param filter 滤波器指针
 * @param input 输入值
 * @return 滤波后的值
 * @note 公式: y[n] = alpha * x[n] + (1 - alpha) * y[n-1]
 */
float LowPass_Update(void *filter, float input)
{
    LowPassFilter_t *lpf = (LowPassFilter_t *)filter;
    if (lpf == NULL)
        return input;

    if (!lpf->initialized)
    {
        lpf->last_output = input;
        lpf->initialized = 1;
        return input;
    }

    lpf->last_output = lpf->alpha * input + (1.0f - lpf->alpha) * lpf->last_output;
    return lpf->last_output;
}

/**
 * @brief 重置一阶低通滤波器
 */
void LowPass_Reset(LowPassFilter_t *filter)
{
    if (filter == NULL)
        return;
    filter->last_output = 0.0f;
    filter->initialized = 0;
}

/*============================================================================
 *                              滑动平均滤波器
 *============================================================================*/

/**
 * @brief 初始化滑动平均滤波器
 */
void MovingAvg_Init(MovingAvgFilter_t *filter)
{
    if (filter == NULL)
        return;

    memset(filter->buffer, 0, sizeof(filter->buffer));
    filter->index = 0;
    filter->count = 0;
    filter->sum = 0.0f;
}

/**
 * @brief 滑动平均滤波更新
 * @param filter 滤波器指针
 * @param input 输入值
 * @return 滤波后的值
 */
float MovingAvg_Update(void *filter, float input)
{
    MovingAvgFilter_t *maf = (MovingAvgFilter_t *)filter;
    if (maf == NULL)
        return input;

    // 减去将被替换的旧值
    maf->sum -= maf->buffer[maf->index];
    // 添加新值
    maf->buffer[maf->index] = input;
    maf->sum += input;

    // 更新索引
    maf->index = (maf->index + 1) % FILTER_MOVING_AVG_SIZE;

    // 更新有效计数
    if (maf->count < FILTER_MOVING_AVG_SIZE)
    {
        maf->count++;
    }

    return maf->sum / (float)maf->count;
}

/**
 * @brief 重置滑动平均滤波器
 */
void MovingAvg_Reset(MovingAvgFilter_t *filter)
{
    MovingAvg_Init(filter);
}

/*============================================================================
 *                              中值滤波器
 *============================================================================*/

/**
 * @brief 初始化中值滤波器
 */
void Median_Init(MedianFilter_t *filter)
{
    if (filter == NULL)
        return;

    memset(filter->buffer, 0, sizeof(filter->buffer));
    filter->index = 0;
    filter->count = 0;
}

/**
 * @brief 中值滤波更新
 * @param filter 滤波器指针
 * @param input 输入值
 * @return 滤波后的值
 */
float Median_Update(void *filter, float input)
{
    MedianFilter_t *mf = (MedianFilter_t *)filter;
    if (mf == NULL)
        return input;

    // 添加新值
    mf->buffer[mf->index] = input;
    mf->index = (mf->index + 1) % FILTER_MEDIAN_SIZE;

    if (mf->count < FILTER_MEDIAN_SIZE)
    {
        mf->count++;
    }

    // 复制并排序
    float sorted[FILTER_MEDIAN_SIZE];
    memcpy(sorted, mf->buffer, sizeof(float) * mf->count);

    // 简单冒泡排序
    for (uint8_t i = 0; i < mf->count - 1; i++)
    {
        for (uint8_t j = 0; j < mf->count - 1 - i; j++)
        {
            if (sorted[j] > sorted[j + 1])
            {
                float temp = sorted[j];
                sorted[j] = sorted[j + 1];
                sorted[j + 1] = temp;
            }
        }
    }

    // 返回中值
    return sorted[mf->count / 2];
}

/**
 * @brief 重置中值滤波器
 */
void Median_Reset(MedianFilter_t *filter)
{
    Median_Init(filter);
}

/*============================================================================
 *                              卡尔曼滤波器
 *============================================================================*/

/**
 * @brief 初始化卡尔曼滤波器
 * @param filter 滤波器指针
 * @param Q 过程噪声协方差 (越小越信任模型预测, 推荐0.001~0.1)
 * @param R 测量噪声协方差 (越小越信任测量值, 推荐0.1~1.0)
 * @param initial_value 初始估计值
 */
void Kalman_Init(KalmanFilter_t *filter, float Q, float R, float initial_value)
{
    if (filter == NULL)
        return;

    filter->Q = Q;
    filter->R = R;
    filter->P = 1.0f; // 初始估计误差协方差
    filter->K = 0.0f;
    filter->X = initial_value;
    filter->initialized = 1;
}

/**
 * @brief 卡尔曼滤波更新
 * @param filter 滤波器指针
 * @param input 测量值
 * @return 滤波后的值 (最优估计)
 */
float Kalman_Update(void *filter, float input)
{
    KalmanFilter_t *kf = (KalmanFilter_t *)filter;
    if (kf == NULL)
        return input;

    if (!kf->initialized)
    {
        kf->X = input;
        kf->initialized = 1;
        return input;
    }

    // 预测步骤
    // X_pred = X (假设系统为静态模型)
    // P_pred = P + Q
    kf->P = kf->P + kf->Q;

    // 更新步骤
    // K = P_pred / (P_pred + R)
    kf->K = kf->P / (kf->P + kf->R);

    // X = X_pred + K * (Z - X_pred)
    kf->X = kf->X + kf->K * (input - kf->X);

    // P = (1 - K) * P_pred
    kf->P = (1.0f - kf->K) * kf->P;

    return kf->X;
}

/**
 * @brief 重置卡尔曼滤波器
 */
void Kalman_Reset(KalmanFilter_t *filter)
{
    if (filter == NULL)
        return;
    filter->P = 1.0f;
    filter->K = 0.0f;
    filter->X = 0.0f;
    filter->initialized = 0;
}

/**
 * @brief 动态调整卡尔曼滤波参数
 */
void Kalman_SetParams(KalmanFilter_t *filter, float Q, float R)
{
    if (filter == NULL)
        return;
    filter->Q = Q;
    filter->R = R;
}

/*============================================================================
 *                              二阶巴特沃斯低通滤波器
 *============================================================================*/

/**
 * @brief 初始化二阶巴特沃斯低通滤波器
 * @param filter 滤波器指针
 * @param cutoff_freq 截止频率 (Hz)
 * @param sample_freq 采样频率 (Hz)
 */
void Butterworth2_Init(Butterworth2Filter_t *filter, float cutoff_freq, float sample_freq)
{
    if (filter == NULL)
        return;

    // 预畸变
    float omega = tanf(M_PI * cutoff_freq / sample_freq);
    float omega2 = omega * omega;
    float sqrt2 = 1.41421356237f;

    float denom = 1.0f + sqrt2 * omega + omega2;

    // 计算系数
    filter->b[0] = omega2 / denom;
    filter->b[1] = 2.0f * omega2 / denom;
    filter->b[2] = omega2 / denom;

    filter->a[0] = 1.0f;
    filter->a[1] = 2.0f * (omega2 - 1.0f) / denom;
    filter->a[2] = (1.0f - sqrt2 * omega + omega2) / denom;

    // 清零历史数据
    memset(filter->x, 0, sizeof(filter->x));
    memset(filter->y, 0, sizeof(filter->y));
    filter->initialized = 0;
}

/**
 * @brief 二阶巴特沃斯低通滤波更新
 */
float Butterworth2_Update(void *filter, float input)
{
    Butterworth2Filter_t *bf = (Butterworth2Filter_t *)filter;
    if (bf == NULL)
        return input;

    if (!bf->initialized)
    {
        bf->x[0] = bf->x[1] = bf->x[2] = input;
        bf->y[0] = bf->y[1] = bf->y[2] = input;
        bf->initialized = 1;
        return input;
    }

    // 移动历史数据
    bf->x[2] = bf->x[1];
    bf->x[1] = bf->x[0];
    bf->x[0] = input;

    bf->y[2] = bf->y[1];
    bf->y[1] = bf->y[0];

    // 计算输出
    bf->y[0] = bf->b[0] * bf->x[0] + bf->b[1] * bf->x[1] + bf->b[2] * bf->x[2] - bf->a[1] * bf->y[1] - bf->a[2] * bf->y[2];

    return bf->y[0];
}

/**
 * @brief 重置二阶巴特沃斯低通滤波器
 */
void Butterworth2_Reset(Butterworth2Filter_t *filter)
{
    if (filter == NULL)
        return;
    memset(filter->x, 0, sizeof(filter->x));
    memset(filter->y, 0, sizeof(filter->y));
    filter->initialized = 0;
}

/*============================================================================
 *                              限幅滤波器
 *============================================================================*/

/**
 * @brief 初始化限幅滤波器
 * @param filter 滤波器指针
 * @param max_delta 单次最大变化量
 */
void Limit_Init(LimitFilter_t *filter, float max_delta)
{
    if (filter == NULL)
        return;

    filter->max_delta = max_delta;
    filter->last_value = 0.0f;
    filter->initialized = 0;
}

/**
 * @brief 限幅滤波更新
 * @note 限制相邻两次采样值的变化幅度
 */
float Limit_Update(void *filter, float input)
{
    LimitFilter_t *lf = (LimitFilter_t *)filter;
    if (lf == NULL)
        return input;

    if (!lf->initialized)
    {
        lf->last_value = input;
        lf->initialized = 1;
        return input;
    }

    float delta = input - lf->last_value;

    if (delta > lf->max_delta)
    {
        lf->last_value += lf->max_delta;
    }
    else if (delta < -lf->max_delta)
    {
        lf->last_value -= lf->max_delta;
    }
    else
    {
        lf->last_value = input;
    }

    return lf->last_value;
}

/**
 * @brief 重置限幅滤波器
 */
void Limit_Reset(LimitFilter_t *filter)
{
    if (filter == NULL)
        return;
    filter->last_value = 0.0f;
    filter->initialized = 0;
}

/*============================================================================
 *                              限幅平均滤波器
 *============================================================================*/

/**
 * @brief 初始化限幅平均滤波器
 */
void LimitAvg_Init(LimitAvgFilter_t *filter, float max_delta)
{
    if (filter == NULL)
        return;

    Limit_Init(&filter->limit, max_delta);
    MovingAvg_Init(&filter->moving_avg);
}

/**
 * @brief 限幅平均滤波更新 (先限幅后平均)
 */
float LimitAvg_Update(void *filter, float input)
{
    LimitAvgFilter_t *laf = (LimitAvgFilter_t *)filter;
    if (laf == NULL)
        return input;

    // 先限幅
    float limited = Limit_Update(&laf->limit, input);
    // 再滑动平均
    return MovingAvg_Update(&laf->moving_avg, limited);
}

/**
 * @brief 重置限幅平均滤波器
 */
void LimitAvg_Reset(LimitAvgFilter_t *filter)
{
    if (filter == NULL)
        return;
    Limit_Reset(&filter->limit);
    MovingAvg_Reset(&filter->moving_avg);
}

/*============================================================================
 *                              通用滤波器接口
 *============================================================================*/

/**
 * @brief 设置滤波器接口
 * @param interface 接口指针
 * @param filter 滤波器实例
 * @param update_func 更新函数
 */
void Filter_SetInterface(FilterInterface_t *interface, void *filter, FilterFunc_t update_func)
{
    if (interface == NULL)
        return;
    interface->filter = filter;
    interface->update = update_func;
}

/**
 * @brief 应用滤波
 */
float Filter_Apply(FilterInterface_t *interface, float input)
{
    if (interface == NULL || interface->update == NULL)
    {
        return input;
    }
    return interface->update(interface->filter, input);
}
