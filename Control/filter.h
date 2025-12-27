/**
 * @file filter.h
 * @brief 常用滤波算法库
 * @details 包含一阶低通滤波、滑动平均滤波、中值滤波、卡尔曼滤波等常用滤波算法
 */

#ifndef __FILTER_H
#define __FILTER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/*============================================================================
 *                              滤波器配置
 *============================================================================*/
#define FILTER_MOVING_AVG_SIZE 10 // 滑动平均滤波窗口大小
#define FILTER_MEDIAN_SIZE 5      // 中值滤波窗口大小

    /*============================================================================
     *                              滤波器类型定义
     *============================================================================*/

    /**
     * @brief 一阶低通滤波器结构体
     */
    typedef struct
    {
        float alpha;         // 滤波系数 (0~1), 越小滤波效果越强
        float last_output;   // 上一次输出值
        uint8_t initialized; // 初始化标志
    } LowPassFilter_t;

    /**
     * @brief 滑动平均滤波器结构体
     */
    typedef struct
    {
        float buffer[FILTER_MOVING_AVG_SIZE]; // 数据缓冲区
        uint8_t index;                        // 当前索引
        uint8_t count;                        // 有效数据个数
        float sum;                            // 数据总和
    } MovingAvgFilter_t;

    /**
     * @brief 中值滤波器结构体
     */
    typedef struct
    {
        float buffer[FILTER_MEDIAN_SIZE]; // 数据缓冲区
        uint8_t index;                    // 当前索引
        uint8_t count;                    // 有效数据个数
    } MedianFilter_t;

    /**
     * @brief 一阶卡尔曼滤波器结构体
     */
    typedef struct
    {
        float Q;             // 过程噪声协方差
        float R;             // 测量噪声协方差
        float P;             // 估计误差协方差
        float K;             // 卡尔曼增益
        float X;             // 估计值
        uint8_t initialized; // 初始化标志
    } KalmanFilter_t;

    /**
     * @brief 二阶巴特沃斯低通滤波器结构体
     */
    typedef struct
    {
        float a[3];          // 分母系数 (a0, a1, a2)
        float b[3];          // 分子系数 (b0, b1, b2)
        float x[3];          // 输入历史 (x[n], x[n-1], x[n-2])
        float y[3];          // 输出历史 (y[n], y[n-1], y[n-2])
        uint8_t initialized; // 初始化标志
    } Butterworth2Filter_t;

    /**
     * @brief 限幅滤波器结构体
     */
    typedef struct
    {
        float max_delta;     // 最大变化量
        float last_value;    // 上一次值
        uint8_t initialized; // 初始化标志
    } LimitFilter_t;

    /**
     * @brief 限幅平均滤波器结构体 (限幅 + 滑动平均)
     */
    typedef struct
    {
        LimitFilter_t limit;          // 限幅滤波器
        MovingAvgFilter_t moving_avg; // 滑动平均滤波器
    } LimitAvgFilter_t;

    /**
     * @brief 通用滤波器函数指针类型
     * @param filter 滤波器实例指针
     * @param input 输入值
     * @return 滤波后的值
     */
    typedef float (*FilterFunc_t)(void *filter, float input);

    /**
     * @brief 通用滤波器接口结构体
     */
    typedef struct
    {
        void *filter;        // 滤波器实例
        FilterFunc_t update; // 滤波更新函数
    } FilterInterface_t;

    /*============================================================================
     *                              函数声明
     *============================================================================*/

    /* 一阶低通滤波器 */
    void LowPass_Init(LowPassFilter_t *filter, float alpha);
    float LowPass_Update(void *filter, float input);
    void LowPass_Reset(LowPassFilter_t *filter);

    /* 滑动平均滤波器 */
    void MovingAvg_Init(MovingAvgFilter_t *filter);
    float MovingAvg_Update(void *filter, float input);
    void MovingAvg_Reset(MovingAvgFilter_t *filter);

    /* 中值滤波器 */
    void Median_Init(MedianFilter_t *filter);
    float Median_Update(void *filter, float input);
    void Median_Reset(MedianFilter_t *filter);

    /* 卡尔曼滤波器 */
    void Kalman_Init(KalmanFilter_t *filter, float Q, float R, float initial_value);
    float Kalman_Update(void *filter, float input);
    void Kalman_Reset(KalmanFilter_t *filter);
    void Kalman_SetParams(KalmanFilter_t *filter, float Q, float R);

    /* 二阶巴特沃斯低通滤波器 */
    void Butterworth2_Init(Butterworth2Filter_t *filter, float cutoff_freq, float sample_freq);
    float Butterworth2_Update(void *filter, float input);
    void Butterworth2_Reset(Butterworth2Filter_t *filter);

    /* 限幅滤波器 */
    void Limit_Init(LimitFilter_t *filter, float max_delta);
    float Limit_Update(void *filter, float input);
    void Limit_Reset(LimitFilter_t *filter);

    /* 限幅平均滤波器 */
    void LimitAvg_Init(LimitAvgFilter_t *filter, float max_delta);
    float LimitAvg_Update(void *filter, float input);
    void LimitAvg_Reset(LimitAvgFilter_t *filter);

    /* 通用滤波器接口 */
    void Filter_SetInterface(FilterInterface_t *interface, void *filter, FilterFunc_t update_func);
    float Filter_Apply(FilterInterface_t *interface, float input);

#ifdef __cplusplus
}
#endif

#endif /* __FILTER_H */
