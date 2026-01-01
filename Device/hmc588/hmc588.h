/**
 * @file    hmc588.h
 * @brief   HMC5883L三轴磁力计驱动头文件
 * @details 提供HMC5883L磁力计的初始化、数据读取、校准等功能
 * @note    支持软件I2C和硬件I2C两种通信方式
 */

#ifndef __HMC_H
#define __HMC_H

#include <config.h>
#include <device_hal.h>

#ifdef USE_DEVICE_HMC588

/*============================ HAL接口声明 ============================*/
extern device_i2c_hal_t *hmc5883l_i2c_hal; /**< HMC5883L I2C HAL接口实例 */

/*============================ 设备地址定义 ============================*/
#define HMC5883L_ADDRESS 0x3C /**< HMC5883L I2C地址 (7位地址左移1位) */

/*============================ 寄存器地址定义 ============================*/
#define HMC5883L_REG_CRA 0x00    /**< 配置寄存器A：采样平均、数据输出速率、测量模式 */
#define HMC5883L_REG_CRB 0x01    /**< 配置寄存器B：增益配置 */
#define HMC5883L_REG_MODE 0x02   /**< 模式寄存器：工作模式选择 */
#define HMC5883L_REG_OUTXM 0x03  /**< X轴数据高8位寄存器 */
#define HMC5883L_REG_OUTXL 0x04  /**< X轴数据低8位寄存器 */
#define HMC5883L_REG_OUTZM 0x05  /**< Z轴数据高8位寄存器 */
#define HMC5883L_REG_OUTZL 0x06  /**< Z轴数据低8位寄存器 */
#define HMC5883L_REG_OUTYM 0x07  /**< Y轴数据高8位寄存器 */
#define HMC5883L_REG_OUTYL 0x08  /**< Y轴数据低8位寄存器 */
#define HMC5883L_REG_STATUS 0x09 /**< 状态寄存器：数据就绪和锁定状态 */
#define HMC5883L_REG_IDA 0x0A    /**< 识别寄存器A：固定值 'H' */
#define HMC5883L_REG_IDB 0x0B    /**< 识别寄存器B：固定值 '4' */
#define HMC5883L_REG_IDC 0x0C    /**< 识别寄存器C：固定值 '3' */

/*============================ 配置寄存器A位定义 ============================*/
/* 采样平均数配置 MA[1:0] */
#define HMC5883L_SAMPLES_1 0x00 /**< 1次采样平均 */
#define HMC5883L_SAMPLES_2 0x20 /**< 2次采样平均 */
#define HMC5883L_SAMPLES_4 0x40 /**< 4次采样平均 */
#define HMC5883L_SAMPLES_8 0x60 /**< 8次采样平均 (默认) */

/* 数据输出速率配置 DO[2:0] */
#define HMC5883L_RATE_0_75 0x00 /**< 0.75 Hz */
#define HMC5883L_RATE_1_5 0x04  /**< 1.5 Hz */
#define HMC5883L_RATE_3 0x08    /**< 3 Hz */
#define HMC5883L_RATE_7_5 0x0C  /**< 7.5 Hz */
#define HMC5883L_RATE_15 0x10   /**< 15 Hz (默认) */
#define HMC5883L_RATE_30 0x14   /**< 30 Hz */
#define HMC5883L_RATE_75 0x18   /**< 75 Hz */

/* 测量模式配置 MS[1:0] */
#define HMC5883L_MEASURE_NORMAL 0x00   /**< 正常测量模式 */
#define HMC5883L_MEASURE_POSITIVE 0x01 /**< 正偏置测量模式 */
#define HMC5883L_MEASURE_NEGATIVE 0x02 /**< 负偏置测量模式 */

/*============================ 配置寄存器B位定义 ============================*/
/* 增益配置 GN[2:0] - 对应不同的磁场测量范围 */
#define HMC5883L_GAIN_1370 0x00 /**< ±0.88 Ga, 1370 LSB/Gauss */
#define HMC5883L_GAIN_1090 0x20 /**< ±1.3 Ga, 1090 LSB/Gauss (默认) */
#define HMC5883L_GAIN_820 0x40  /**< ±1.9 Ga, 820 LSB/Gauss */
#define HMC5883L_GAIN_660 0x60  /**< ±2.5 Ga, 660 LSB/Gauss */
#define HMC5883L_GAIN_440 0x80  /**< ±4.0 Ga, 440 LSB/Gauss */
#define HMC5883L_GAIN_390 0xA0  /**< ±4.7 Ga, 390 LSB/Gauss */
#define HMC5883L_GAIN_330 0xC0  /**< ±5.6 Ga, 330 LSB/Gauss */
#define HMC5883L_GAIN_230 0xE0  /**< ±8.1 Ga, 230 LSB/Gauss */

/*============================ 模式寄存器位定义 ============================*/
#define HMC5883L_MODE_CONTINUOUS 0x00 /**< 连续测量模式 */
#define HMC5883L_MODE_SINGLE 0x01     /**< 单次测量模式 */
#define HMC5883L_MODE_IDLE 0x02       /**< 空闲模式 */

/*============================ 状态寄存器位定义 ============================*/
#define HMC5883L_STATUS_RDY 0x01  /**< 数据就绪标志位 */
#define HMC5883L_STATUS_LOCK 0x02 /**< 数据锁定标志位 */

/*============================ 数据结构定义 ============================*/

/**
 * @brief HMC5883L原始数据结构体
 */
typedef struct
{
    int16_t x; /**< X轴原始数据 */
    int16_t y; /**< Y轴原始数据 */
    int16_t z; /**< Z轴原始数据 */
} HMC5883L_RawData_t;

/**
 * @brief HMC5883L磁场数据结构体（已转换为高斯）
 */
typedef struct
{
    float x; /**< X轴磁场强度 (Gauss) */
    float y; /**< Y轴磁场强度 (Gauss) */
    float z; /**< Z轴磁场强度 (Gauss) */
} HMC5883L_MagData_t;

/**
 * @brief HMC5883L校准参数结构体
 */
typedef struct
{
    int16_t offset_x; /**< X轴偏移量 */
    int16_t offset_y; /**< Y轴偏移量 */
    int16_t offset_z; /**< Z轴偏移量 */
    float scale_x;    /**< X轴比例因子 */
    float scale_y;    /**< Y轴比例因子 */
    float scale_z;    /**< Z轴比例因子 */
} HMC5883L_Calibration_t;

/**
 * @brief HMC5883L配置结构体
 */
typedef struct
{
    uint8_t samples;      /**< 采样平均数 */
    uint8_t rate;         /**< 数据输出速率 */
    uint8_t measure_mode; /**< 测量模式 */
    uint8_t gain;         /**< 增益配置 */
    uint8_t mode;         /**< 工作模式 */
} HMC5883L_Config_t;

/*============================ 基础读写函数 ============================*/

/**
 * @brief   绑定I2C HAL接口
 * @param   hal  I2C HAL接口指针
 * @note    必须在调用任何其他HMC5883L函数之前调用
 */
void HMC5883L_BindHAL(device_i2c_hal_t *hal);

/**
 * @brief   向HMC5883L写入单个寄存器
 * @param   RegAddress  寄存器地址
 * @param   Data        要写入的数据
 * @return  无
 */
void HMC_WriteReg(uint8_t RegAddress, uint8_t Data);

/**
 * @brief   从HMC5883L读取单个寄存器
 * @param   RegAddress  寄存器地址
 * @return  读取到的寄存器值
 */
uint8_t HMC_ReadReg(uint8_t RegAddress);

/**
 * @brief   从HMC5883L连续读取多个字节
 * @param   RegAddress  起始寄存器地址
 * @param   len         读取长度
 * @param   buf         数据缓冲区
 * @return  0-成功，其他-失败
 */
uint8_t HMC_ReadLen(uint8_t RegAddress, uint8_t len, uint8_t *buf);

/*============================ 初始化和配置函数 ============================*/

/**
 * @brief   HMC5883L初始化函数（使用默认配置）
 * @return  0-成功，-1-失败（设备未找到）
 * @note    默认配置：8次采样平均，15Hz输出，增益±4.7Ga，连续测量模式
 */
uint8_t HMC5883L_Init(void);

/**
 * @brief   使用指定配置初始化HMC5883L
 * @param   config  配置结构体指针
 * @return  0-成功，-1-失败
 */
uint8_t HMC5883L_Init_Config(HMC5883L_Config_t *config);

/**
 * @brief   设置采样平均数
 * @param   samples 采样平均数（使用HMC5883L_SAMPLES_x宏）
 * @return  无
 */
void HMC5883L_SetSamples(uint8_t samples);

/**
 * @brief   设置数据输出速率
 * @param   rate    输出速率（使用HMC5883L_RATE_x宏）
 * @return  无
 */
void HMC5883L_SetRate(uint8_t rate);

/**
 * @brief   设置增益
 * @param   gain    增益配置（使用HMC5883L_GAIN_x宏）
 * @return  无
 */
void HMC5883L_SetGain(uint8_t gain);

/**
 * @brief   设置工作模式
 * @param   mode    工作模式（使用HMC5883L_MODE_x宏）
 * @return  无
 */
void HMC5883L_SetMode(uint8_t mode);

/*============================ 数据读取函数 ============================*/

/**
 * @brief   获取设备ID
 * @return  设备ID（正常应返回0x48，即'H'）
 */
uint8_t HMC_GetID(void);

/**
 * @brief   检查数据是否就绪
 * @return  1-数据就绪，0-数据未就绪
 */
uint8_t HMC5883L_IsDataReady(void);

/**
 * @brief   获取原始磁力数据（分别获取X、Y、Z）
 * @param   MagX    X轴数据指针
 * @param   MagY    Y轴数据指针
 * @param   MagZ    Z轴数据指针
 * @return  无
 */
void HMC_GetData(int16_t *MagX, int16_t *MagY, int16_t *MagZ);

/**
 * @brief   获取原始磁力数据（结构体形式）
 * @param   raw_data    原始数据结构体指针
 * @return  0-成功，其他-失败
 */
uint8_t HMC5883L_GetRawData(HMC5883L_RawData_t *raw_data);

/**
 * @brief   获取磁场数据（转换为高斯单位）
 * @param   mag_data    磁场数据结构体指针
 * @return  0-成功，其他-失败
 */
uint8_t HMC5883L_GetMagData(HMC5883L_MagData_t *mag_data);

/**
 * @brief   获取校准后的磁场数据
 * @param   mag_data    磁场数据结构体指针
 * @param   calib       校准参数结构体指针
 * @return  0-成功，其他-失败
 */
uint8_t HMC5883L_GetCalibratedData(HMC5883L_MagData_t *mag_data, HMC5883L_Calibration_t *calib);

/*============================ 航向角计算函数 ============================*/

/**
 * @brief   计算航向角（方位角）
 * @return  航向角，范围0-360度，正北为0度，顺时针增加
 * @note    未进行倾斜补偿，需保持传感器水平放置
 */
float HMC5883L_GetHeading(void);

/**
 * @brief   使用校准数据计算航向角
 * @param   calib   校准参数结构体指针
 * @return  航向角，范围0-360度
 */
float HMC5883L_GetCalibratedHeading(HMC5883L_Calibration_t *calib);

/**
 * @brief   带倾斜补偿的航向角计算
 * @param   pitch   俯仰角（弧度）
 * @param   roll    横滚角（弧度）
 * @return  补偿后的航向角，范围0-360度
 */
float HMC5883L_GetTiltCompensatedHeading(float pitch, float roll);

/*============================ 校准函数 ============================*/

/**
 * @brief   自检测试
 * @return  0-自检通过，其他-自检失败
 * @note    使用内部偏置测试功能进行自检
 */
uint8_t HMC5883L_SelfTest(void);

/**
 * @brief   开始校准（需要将传感器旋转一周以上）
 * @param   calib       校准参数结构体指针（用于存储校准结果）
 * @param   samples     采样次数（建议>=100）
 * @return  0-校准成功，其他-校准失败
 * @note    调用此函数后，需要缓慢旋转传感器360度
 */
uint8_t HMC5883L_Calibrate(HMC5883L_Calibration_t *calib, uint16_t samples);

/**
 * @brief   应用校准参数
 * @param   calib   校准参数结构体指针
 * @return  无
 */
void HMC5883L_ApplyCalibration(HMC5883L_Calibration_t *calib);

/*============================ 状态和诊断函数 ============================*/

/**
 * @brief   获取状态寄存器值
 * @return  状态寄存器值
 */
uint8_t HMC5883L_GetStatus(void);

/**
 * @brief   检查设备是否连接
 * @return  1-设备已连接，0-设备未连接
 */
uint8_t HMC5883L_IsConnected(void);

/**
 * @brief   软复位（重新初始化）
 * @return  0-成功，其他-失败
 */
uint8_t HMC5883L_Reset(void);

#endif /* USE_DEVICE_HMC588 */

#endif /* __HMC_H */
