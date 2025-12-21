/*
 $License:
    Copyright (C) 2011-2012 InvenSense Corporation, All Rights Reserved.
    See included License.txt for License information.
 $
 */
/**
 * @file        inv_mpu.h
 * @brief       MPU6050/MPU6500/MPU9150/MPU9250 陀螺仪驱动头文件
 * @details     基于I2C通信的InvenSense陀螺仪驱动程序
 *              支持的设备型号：
 *              - MPU6050
 *              - MPU6500
 *              - MPU9150 (MPU6050 + AK8975磁力计)
 *              - MPU9250 (MPU6500 + AK8963磁力计)
 * @note        移植自InvenSense官方MotionDriver 6.12
 */

#ifndef _INV_MPU_H_
#define _INV_MPU_H_

/*============================ 类型定义 ============================*/
typedef unsigned char u8;

/*============================ 配置宏定义 ============================*/

/** @brief 默认DMP输出速率 (Hz) */
#define DEFAULT_MPU_HZ (200) /**< 200Hz，对应5ms周期 */

/*============================ 传感器轴选择掩码 ============================*/

#define INV_X_GYRO (0x40)                                   /**< X轴陀螺仪 */
#define INV_Y_GYRO (0x20)                                   /**< Y轴陀螺仪 */
#define INV_Z_GYRO (0x10)                                   /**< Z轴陀螺仪 */
#define INV_XYZ_GYRO (INV_X_GYRO | INV_Y_GYRO | INV_Z_GYRO) /**< 三轴陀螺仪 */
#define INV_XYZ_ACCEL (0x08)                                /**< 三轴加速度计 */
#define INV_XYZ_COMPASS (0x01)                              /**< 三轴磁力计 */

/*============================ 中断参数结构体 ============================*/

/**
 * @brief 中断配置参数结构体
 * @note  移植自官方MSP430 DMP驱动
 */
struct int_param_s
{
    void (*cb)(void);         /**< 中断回调函数指针 */
    unsigned short pin;       /**< 中断引脚编号 */
    unsigned char lp_exit;    /**< 低功耗退出标志 */
    unsigned char active_low; /**< 低电平有效标志 */
};

/*============================ 中断状态位定义 ============================*/

#define MPU_INT_STATUS_DATA_READY (0x0001)    /**< 数据就绪中断 */
#define MPU_INT_STATUS_DMP (0x0002)           /**< DMP中断 */
#define MPU_INT_STATUS_PLL_READY (0x0004)     /**< PLL就绪中断 */
#define MPU_INT_STATUS_I2C_MST (0x0008)       /**< I2C主机中断 */
#define MPU_INT_STATUS_FIFO_OVERFLOW (0x0010) /**< FIFO溢出中断 */
#define MPU_INT_STATUS_ZMOT (0x0020)          /**< 零运动检测中断 */
#define MPU_INT_STATUS_MOT (0x0040)           /**< 运动检测中断 */
#define MPU_INT_STATUS_FREE_FALL (0x0080)     /**< 自由落体检测中断 */
#define MPU_INT_STATUS_DMP_0 (0x0100)         /**< DMP中断0 */
#define MPU_INT_STATUS_DMP_1 (0x0200)         /**< DMP中断1 */
#define MPU_INT_STATUS_DMP_2 (0x0400)         /**< DMP中断2 */
#define MPU_INT_STATUS_DMP_3 (0x0800)         /**< DMP中断3 */
#define MPU_INT_STATUS_DMP_4 (0x1000)         /**< DMP中断4 */
#define MPU_INT_STATUS_DMP_5 (0x2000)         /**< DMP中断5 */

/*============================ 初始化与设置函数 ============================*/

/**
 * @brief   MPU传感器初始化
 * @return  0-成功，其他-失败
 */
int mpu_init(void);

/**
 * @brief   初始化从机设备（如磁力计）
 * @return  0-成功，其他-失败
 */
int mpu_init_slave(void);

/**
 * @brief   设置I2C旁路模式
 * @param   bypass_on   1-开启旁路，0-关闭旁路
 * @return  0-成功，其他-失败
 * @note    开启旁路后可直接访问辅助I2C总线上的设备
 */
int mpu_set_bypass(unsigned char bypass_on);

/*============================ 配置函数 ============================*/

/**
 * @brief   设置低功耗加速度计模式
 * @param   rate    采样率
 * @return  0-成功，其他-失败
 */
int mpu_lp_accel_mode(unsigned char rate);

/**
 * @brief   配置低功耗运动检测中断
 * @param   thresh      运动阈值
 * @param   time        检测时间
 * @param   lpa_freq    低功耗加速度计频率
 * @return  0-成功，其他-失败
 */
int mpu_lp_motion_interrupt(unsigned short thresh, unsigned char time,
                            unsigned char lpa_freq);

/**
 * @brief   设置中断电平极性
 * @param   active_low  1-低电平有效，0-高电平有效
 * @return  0-成功，其他-失败
 */
int mpu_set_int_level(unsigned char active_low);

/**
 * @brief   设置中断锁存模式
 * @param   enable  1-锁存模式，0-脉冲模式（50us）
 * @return  0-成功，其他-失败
 */
int mpu_set_int_latched(unsigned char enable);

/**
 * @brief   设置DMP使能状态
 * @param   enable  1-使能DMP，0-禁用DMP
 * @return  0-成功，其他-失败
 */
int mpu_set_dmp_state(unsigned char enable);

/**
 * @brief   获取DMP使能状态
 * @param   enabled 返回DMP状态指针
 * @return  0-成功，其他-失败
 */
int mpu_get_dmp_state(unsigned char *enabled);

/**
 * @brief   获取数字低通滤波器配置
 * @param   lpf     返回LPF值指针
 * @return  0-成功，其他-失败
 */
int mpu_get_lpf(unsigned short *lpf);

/**
 * @brief   设置数字低通滤波器
 * @param   lpf     滤波器截止频率 (Hz)
 * @return  0-成功，其他-失败
 */
int mpu_set_lpf(unsigned short lpf);

/**
 * @brief   获取陀螺仪量程
 * @param   fsr     返回量程值指针 (dps)
 * @return  0-成功，其他-失败
 */
int mpu_get_gyro_fsr(unsigned short *fsr);

/**
 * @brief   设置陀螺仪量程
 * @param   fsr     量程值：250/500/1000/2000 (dps)
 * @return  0-成功，其他-失败
 */
int mpu_set_gyro_fsr(unsigned short fsr);

/**
 * @brief   获取加速度计量程
 * @param   fsr     返回量程值指针 (g)
 * @return  0-成功，其他-失败
 */
int mpu_get_accel_fsr(unsigned char *fsr);

/**
 * @brief   设置加速度计量程
 * @param   fsr     量程值：2/4/8/16 (g)
 * @return  0-成功，其他-失败
 */
int mpu_set_accel_fsr(unsigned char fsr);

/**
 * @brief   获取磁力计量程
 * @param   fsr     返回量程值指针
 * @return  0-成功，其他-失败
 */
int mpu_get_compass_fsr(unsigned short *fsr);

/**
 * @brief   获取陀螺仪灵敏度
 * @param   sens    返回灵敏度指针 (LSB/dps)
 * @return  0-成功，其他-失败
 */
int mpu_get_gyro_sens(float *sens);

/**
 * @brief   获取加速度计灵敏度
 * @param   sens    返回灵敏度指针 (LSB/g)
 * @return  0-成功，其他-失败
 */
int mpu_get_accel_sens(unsigned short *sens);

/**
 * @brief   获取采样率
 * @param   rate    返回采样率指针 (Hz)
 * @return  0-成功，其他-失败
 */
int mpu_get_sample_rate(unsigned short *rate);

/**
 * @brief   设置采样率
 * @param   rate    采样率 (Hz)，范围4-1000
 * @return  0-成功，其他-失败
 */
int mpu_set_sample_rate(unsigned short rate);

/**
 * @brief   获取磁力计采样率
 * @param   rate    返回采样率指针 (Hz)
 * @return  0-成功，其他-失败
 */
int mpu_get_compass_sample_rate(unsigned short *rate);

/**
 * @brief   设置磁力计采样率
 * @param   rate    采样率 (Hz)，最大100Hz
 * @return  0-成功，其他-失败
 */
int mpu_set_compass_sample_rate(unsigned short rate);

/**
 * @brief   获取FIFO配置
 * @param   sensors 返回已启用传感器掩码
 * @return  0-成功，其他-失败
 */
int mpu_get_fifo_config(unsigned char *sensors);

/**
 * @brief   配置FIFO
 * @param   sensors 传感器掩码 (INV_XYZ_GYRO | INV_XYZ_ACCEL等)
 * @return  0-成功，其他-失败
 */
int mpu_configure_fifo(unsigned char sensors);

/**
 * @brief   获取电源状态
 * @param   power_on    返回电源状态指针
 * @return  0-成功，其他-失败
 */
int mpu_get_power_state(unsigned char *power_on);

/**
 * @brief   设置使能的传感器
 * @param   sensors 传感器掩码
 * @return  0-成功，其他-失败
 */
int mpu_set_sensors(unsigned char sensors);

/**
 * @brief   设置加速度计偏置
 * @param   accel_bias  偏置数组指针
 * @return  0-成功，其他-失败
 */
int mpu_set_accel_bias(const long *accel_bias);

/*============================ 数据读取函数 ============================*/

/**
 * @brief   获取陀螺仪寄存器原始数据
 * @param   data        数据数组[3]
 * @param   timestamp   时间戳指针
 * @return  0-成功，其他-失败
 */
int mpu_get_gyro_reg(short *data, unsigned long *timestamp);

/**
 * @brief   获取加速度计寄存器原始数据
 * @param   data        数据数组[3]
 * @param   timestamp   时间戳指针
 * @return  0-成功，其他-失败
 */
int mpu_get_accel_reg(short *data, unsigned long *timestamp);

/**
 * @brief   获取磁力计寄存器原始数据
 * @param   data        数据数组[3]
 * @param   timestamp   时间戳指针
 * @return  0-成功，其他-失败
 */
int mpu_get_compass_reg(short *data, unsigned long *timestamp);

/**
 * @brief   获取温度数据
 * @param   data        温度数据指针（Q16格式，单位：摄氏度）
 * @param   timestamp   时间戳指针
 * @return  0-成功，其他-失败
 */
int mpu_get_temperature(long *data, unsigned long *timestamp);

/**
 * @brief   获取中断状态
 * @param   status  状态值指针
 * @return  0-成功，其他-失败
 */
int mpu_get_int_status(short *status);

/**
 * @brief   从FIFO读取数据
 * @param   gyro        陀螺仪数据数组[3]
 * @param   accel       加速度计数据数组[3]
 * @param   timestamp   时间戳指针
 * @param   sensors     返回有效传感器掩码
 * @param   more        返回FIFO中剩余数据包数
 * @return  0-成功，其他-失败
 */
int mpu_read_fifo(short *gyro, short *accel, unsigned long *timestamp,
                  unsigned char *sensors, unsigned char *more);

/**
 * @brief   以流方式读取FIFO
 * @param   length  读取长度
 * @param   data    数据缓冲区
 * @param   more    返回FIFO中剩余数据包数
 * @return  0-成功，其他-失败
 */
int mpu_read_fifo_stream(unsigned short length, unsigned char *data,
                         unsigned char *more);

/**
 * @brief   复位FIFO
 * @return  0-成功，其他-失败
 */
int mpu_reset_fifo(void);

/*============================ DMP内存操作函数 ============================*/

/**
 * @brief   写入DMP内存
 * @param   mem_addr    内存地址
 * @param   length      数据长度
 * @param   data        数据指针
 * @return  0-成功，其他-失败
 */
int mpu_write_mem(unsigned short mem_addr, unsigned short length,
                  unsigned char *data);

/**
 * @brief   读取DMP内存
 * @param   mem_addr    内存地址
 * @param   length      数据长度
 * @param   data        数据缓冲区
 * @return  0-成功，其他-失败
 */
int mpu_read_mem(unsigned short mem_addr, unsigned short length,
                 unsigned char *data);

/**
 * @brief   加载DMP固件
 * @param   length      固件长度
 * @param   firmware    固件数据指针
 * @param   start_addr  程序起始地址
 * @param   sample_rate 采样率
 * @return  0-成功，其他-失败
 */
int mpu_load_firmware(unsigned short length, const unsigned char *firmware,
                      unsigned short start_addr, unsigned short sample_rate);

/*============================ 调试与自检函数 ============================*/

/**
 * @brief   寄存器值转储（调试用）
 * @return  0-成功，其他-失败
 */
int mpu_reg_dump(void);

/**
 * @brief   读取指定寄存器
 * @param   reg     寄存器地址
 * @param   data    数据指针
 * @return  0-成功，其他-失败
 */
int mpu_read_reg(unsigned char reg, unsigned char *data);

/**
 * @brief   运行自检测试
 * @param   gyro    返回陀螺仪偏差数组[3]
 * @param   accel   返回加速度计偏差数组[3]
 * @return  位掩码：bit0=陀螺仪通过，bit1=加速度计通过，bit2=磁力计通过
 */
int mpu_run_self_test(long *gyro, long *accel);

/**
 * @brief   注册敲击回调函数
 * @param   func    回调函数指针
 * @return  0-成功，其他-失败
 */
int mpu_register_tap_cb(void (*func)(unsigned char, unsigned char));

/*============================ 用户扩展函数 ============================*/

/**
 * @brief   获取系统时间（毫秒）
 * @param   time    返回时间值指针
 * @note    需要用户根据平台实现
 */
void mget_ms(unsigned long *time);

/**
 * @brief   将方向矩阵行转换为标量
 * @param   row     方向矩阵行数组[3]
 * @return  转换后的标量值
 */
unsigned short inv_row_2_scale(const signed char *row);

/**
 * @brief   将方向矩阵转换为标量
 * @param   mtx     方向矩阵数组[9]
 * @return  转换后的标量值
 * @note    用于设置DMP的传感器安装方向
 */
unsigned short inv_orientation_matrix_to_scalar(const signed char *mtx);

/**
 * @brief   运行自检并设置偏置
 * @return  0-自检通过，1-自检失败
 */
u8 run_self_test(void);

/**
 * @brief   MPU DMP初始化
 * @return  0-成功
 *          1-传感器使能失败
 *          2-FIFO配置失败
 *          3-采样率设置失败
 *          4-DMP固件加载失败
 *          5-方向设置失败
 *          6-特性使能失败
 *          7-FIFO速率设置失败
 *          9-DMP启动失败
 *          10-MPU初始化失败
 */
u8 mpu_dmp_init(void);

/**
 * @brief   从DMP获取姿态角数据
 * @param   pitch   俯仰角指针（度）
 * @param   roll    横滚角指针（度）
 * @param   yaw     偏航角指针（度）
 * @return  0-成功，1-FIFO读取失败，2-无四元数数据
 */
u8 mpu_dmp_get_data(float *pitch, float *roll, float *yaw);

#endif /* _INV_MPU_H_ */
