/*
 $License:
    Copyright (C) 2011-2012 InvenSense Corporation, All Rights Reserved.
    See included License.txt for License information.
 $
 */
/**
 * @file        inv_mpu_dmp_motion_driver.h
 * @brief       DMP（数字运动处理器）驱动头文件
 * @details     提供DMP固件加载、特性配置、数据读取等功能
 *              所有函数以dmp_前缀命名，以区分MPL和通用驱动函数
 * @note        移植自InvenSense官方MotionDriver 6.12
 */

#ifndef _INV_MPU_DMP_MOTION_DRIVER_H_
#define _INV_MPU_DMP_MOTION_DRIVER_H_

/*============================ 敲击检测轴掩码 ============================*/

#define TAP_X (0x01)   /**< X轴敲击检测 */
#define TAP_Y (0x02)   /**< Y轴敲击检测 */
#define TAP_Z (0x04)   /**< Z轴敲击检测 */
#define TAP_XYZ (0x07) /**< 三轴敲击检测 */

/*============================ 敲击方向定义 ============================*/

#define TAP_X_UP (0x01)   /**< X轴正向敲击 */
#define TAP_X_DOWN (0x02) /**< X轴负向敲击 */
#define TAP_Y_UP (0x03)   /**< Y轴正向敲击 */
#define TAP_Y_DOWN (0x04) /**< Y轴负向敲击 */
#define TAP_Z_UP (0x05)   /**< Z轴正向敲击 */
#define TAP_Z_DOWN (0x06) /**< Z轴负向敲击 */

/*============================ Android方向定义 ============================*/

#define ANDROID_ORIENT_PORTRAIT (0x00)          /**< 竖屏正向 */
#define ANDROID_ORIENT_LANDSCAPE (0x01)         /**< 横屏正向 */
#define ANDROID_ORIENT_REVERSE_PORTRAIT (0x02)  /**< 竖屏反向 */
#define ANDROID_ORIENT_REVERSE_LANDSCAPE (0x03) /**< 横屏反向 */

/*============================ DMP中断模式 ============================*/

#define DMP_INT_GESTURE (0x01)    /**< 手势中断模式 */
#define DMP_INT_CONTINUOUS (0x02) /**< 连续中断模式 */

/*============================ DMP功能特性掩码 ============================*/

#define DMP_FEATURE_TAP (0x001)            /**< 敲击检测功能 */
#define DMP_FEATURE_ANDROID_ORIENT (0x002) /**< Android方向检测功能 */
#define DMP_FEATURE_LP_QUAT (0x004)        /**< 低功耗四元数（仅陀螺仪） */
#define DMP_FEATURE_PEDOMETER (0x008)      /**< 计步器功能 */
#define DMP_FEATURE_6X_LP_QUAT (0x010)     /**< 六轴低功耗四元数（陀螺仪+加速度计） */
#define DMP_FEATURE_GYRO_CAL (0x020)       /**< 陀螺仪自动校准功能 */
#define DMP_FEATURE_SEND_RAW_ACCEL (0x040) /**< 发送原始加速度计数据 */
#define DMP_FEATURE_SEND_RAW_GYRO (0x080)  /**< 发送原始陀螺仪数据 */
#define DMP_FEATURE_SEND_CAL_GYRO (0x100)  /**< 发送校准后陀螺仪数据 */

/*============================ 四元数输出格式 ============================*/

#define INV_WXYZ_QUAT (0x100) /**< WXYZ格式四元数 */

/*============================ 初始化与配置函数 ============================*/

/**
 * @brief   加载DMP固件
 * @return  0-成功，其他-失败
 */
int dmp_load_motion_driver_firmware(void);

/**
 * @brief   设置FIFO输出速率
 * @param   rate    速率 (Hz)，范围1-200
 * @return  0-成功，其他-失败
 */
int dmp_set_fifo_rate(unsigned short rate);

/**
 * @brief   获取FIFO输出速率
 * @param   rate    返回速率指针
 * @return  0-成功，其他-失败
 */
int dmp_get_fifo_rate(unsigned short *rate);

/**
 * @brief   使能DMP功能特性
 * @param   mask    功能掩码 (DMP_FEATURE_xxx的组合)
 * @return  0-成功，其他-失败
 * @note    可同时使能多个功能，如：
 *          DMP_FEATURE_6X_LP_QUAT | DMP_FEATURE_GYRO_CAL
 */
int dmp_enable_feature(unsigned short mask);

/**
 * @brief   获取已使能的DMP功能
 * @param   mask    返回功能掩码指针
 * @return  0-成功，其他-失败
 */
int dmp_get_enabled_features(unsigned short *mask);

/**
 * @brief   设置DMP中断模式
 * @param   mode    中断模式 (DMP_INT_GESTURE或DMP_INT_CONTINUOUS)
 * @return  0-成功，其他-失败
 */
int dmp_set_interrupt_mode(unsigned char mode);

/**
 * @brief   设置传感器安装方向
 * @param   orient  方向标量值
 * @return  0-成功，其他-失败
 * @note    使用inv_orientation_matrix_to_scalar()将方向矩阵转换为标量
 */
int dmp_set_orientation(unsigned short orient);

/**
 * @brief   设置陀螺仪偏置
 * @param   bias    偏置数组[3] (Q16格式，单位：dps)
 * @return  0-成功，其他-失败
 */
int dmp_set_gyro_bias(long *bias);

/**
 * @brief   设置加速度计偏置
 * @param   bias    偏置数组[3] (Q16格式，单位：g)
 * @return  0-成功，其他-失败
 */
int dmp_set_accel_bias(long *bias);

/*============================ 敲击检测函数 ============================*/

/**
 * @brief   注册敲击检测回调函数
 * @param   func    回调函数指针，参数为(方向, 敲击次数)
 * @return  0-成功，其他-失败
 */
int dmp_register_tap_cb(void (*func)(unsigned char, unsigned char));

/**
 * @brief   设置敲击检测阈值
 * @param   axis    轴选择 (TAP_X/TAP_Y/TAP_Z)
 * @param   thresh  阈值 (mg)
 * @return  0-成功，其他-失败
 */
int dmp_set_tap_thresh(unsigned char axis, unsigned short thresh);

/**
 * @brief   设置敲击检测轴
 * @param   axis    轴掩码 (TAP_X | TAP_Y | TAP_Z)
 * @return  0-成功，其他-失败
 */
int dmp_set_tap_axes(unsigned char axis);

/**
 * @brief   设置最小敲击次数
 * @param   min_taps    最小敲击次数 (1-4)
 * @return  0-成功，其他-失败
 */
int dmp_set_tap_count(unsigned char min_taps);

/**
 * @brief   设置敲击间隔时间
 * @param   time    时间 (ms)
 * @return  0-成功，其他-失败
 */
int dmp_set_tap_time(unsigned short time);

/**
 * @brief   设置多次敲击间隔时间
 * @param   time    时间 (ms)
 * @return  0-成功，其他-失败
 */
int dmp_set_tap_time_multi(unsigned short time);

/**
 * @brief   设置抖动抑制阈值
 * @param   sf      比例因子
 * @param   thresh  阈值
 * @return  0-成功，其他-失败
 */
int dmp_set_shake_reject_thresh(long sf, unsigned short thresh);

/**
 * @brief   设置抖动抑制时间
 * @param   time    时间 (ms)
 * @return  0-成功，其他-失败
 */
int dmp_set_shake_reject_time(unsigned short time);

/**
 * @brief   设置抖动抑制超时时间
 * @param   time    时间 (ms)
 * @return  0-成功，其他-失败
 */
int dmp_set_shake_reject_timeout(unsigned short time);

/*============================ Android方向检测函数 ============================*/

/**
 * @brief   注册Android方向检测回调函数
 * @param   func    回调函数指针，参数为方向值
 * @return  0-成功，其他-失败
 */
int dmp_register_android_orient_cb(void (*func)(unsigned char));

/*============================ 四元数功能函数 ============================*/

/**
 * @brief   使能/禁用低功耗四元数（仅陀螺仪）
 * @param   enable  1-使能，0-禁用
 * @return  0-成功，其他-失败
 */
int dmp_enable_lp_quat(unsigned char enable);

/**
 * @brief   使能/禁用六轴低功耗四元数（陀螺仪+加速度计）
 * @param   enable  1-使能，0-禁用
 * @return  0-成功，其他-失败
 */
int dmp_enable_6x_lp_quat(unsigned char enable);

/*============================ 计步器函数 ============================*/

/**
 * @brief   获取计步器步数
 * @param   count   返回步数指针
 * @return  0-成功，其他-失败
 */
int dmp_get_pedometer_step_count(unsigned long *count);

/**
 * @brief   设置计步器步数
 * @param   count   步数值
 * @return  0-成功，其他-失败
 */
int dmp_set_pedometer_step_count(unsigned long count);

/**
 * @brief   获取行走时间
 * @param   time    返回时间指针 (ms)
 * @return  0-成功，其他-失败
 */
int dmp_get_pedometer_walk_time(unsigned long *time);

/**
 * @brief   设置行走时间
 * @param   time    时间值 (ms)
 * @return  0-成功，其他-失败
 */
int dmp_set_pedometer_walk_time(unsigned long time);

/*============================ 陀螺仪校准函数 ============================*/

/**
 * @brief   使能/禁用陀螺仪自动校准
 * @param   enable  1-使能，0-禁用
 * @return  0-成功，其他-失败
 * @note    使能后，DMP会在静止时自动校准陀螺仪偏置
 */
int dmp_enable_gyro_cal(unsigned char enable);

/*============================ 数据读取函数 ============================*/

/**
 * @brief   从DMP FIFO读取数据
 * @param   gyro        陀螺仪数据数组[3]
 * @param   accel       加速度计数据数组[3]
 * @param   quat        四元数数据数组[4] (Q30格式)
 * @param   timestamp   时间戳指针
 * @param   sensors     返回有效传感器掩码
 * @param   more        返回FIFO中剩余数据包数
 * @return  0-成功，其他-失败
 * @note    应在检测到MPU中断时调用此函数
 */
int dmp_read_fifo(short *gyro, short *accel, long *quat,
                  unsigned long *timestamp, short *sensors, unsigned char *more);

#endif /* _INV_MPU_DMP_MOTION_DRIVER_H_ */
