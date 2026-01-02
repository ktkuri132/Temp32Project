/**
 * @file    bmp280.h
 * @brief   BMP280气压温度传感器驱动头文件
 * @details 提供BMP280传感器的初始化、温度/气压读取、海拔计算等功能
 * @note    支持软件I2C通信方式，适配df_iic接口框架
 *          风格与HMC5883L驱动保持一致，使用静态全局变量存储状态
 */

#ifndef __BMP280_H
#define __BMP280_H

#include <stdint.h>
#include <config.h>
#include <device_hal.h>

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef USE_DEVICE_BMP280

    /*============================ HAL接口声明 ============================*/
    extern device_i2c_hal_t *bmp280_i2c_hal; /**< BMP280 I2C HAL接口实例 */

/*============================ 设备地址定义 ============================*/
#define BMP280_I2C_ADDR_LOW 0xEC            /**< SDO接GND时的I2C地址 (0x76 << 1) */
#define BMP280_I2C_ADDR_HIGH 0xEE           /**< SDO接VDD时的I2C地址 (0x77 << 1) */
#define BMP280_I2C_ADDR BMP280_I2C_ADDR_LOW /**< 默认使用低地址 */

/*============================ 芯片ID定义 ============================*/
#define BMP280_CHIP_ID 0x58 /**< BMP280芯片ID */
#define BME280_CHIP_ID 0x60 /**< BME280芯片ID (兼容) */

/*============================ 寄存器地址定义 ============================*/
/* 校准数据寄存器 (只读) */
#define BMP280_REG_CALIB00 0x88 /**< 校准数据起始地址 */
#define BMP280_REG_CALIB25 0xA1 /**< 校准数据结束地址 */

/* 芯片ID寄存器 */
#define BMP280_REG_ID 0xD0 /**< 芯片ID寄存器 */

/* 控制寄存器 */
#define BMP280_REG_RESET 0xE0     /**< 软复位寄存器 */
#define BMP280_REG_STATUS 0xF3    /**< 状态寄存器 */
#define BMP280_REG_CTRL_MEAS 0xF4 /**< 测量控制寄存器 */
#define BMP280_REG_CONFIG 0xF5    /**< 配置寄存器 */

/* 数据寄存器 */
#define BMP280_REG_PRESS_MSB 0xF7  /**< 气压数据MSB */
#define BMP280_REG_PRESS_LSB 0xF8  /**< 气压数据LSB */
#define BMP280_REG_PRESS_XLSB 0xF9 /**< 气压数据XLSB */
#define BMP280_REG_TEMP_MSB 0xFA   /**< 温度数据MSB */
#define BMP280_REG_TEMP_LSB 0xFB   /**< 温度数据LSB */
#define BMP280_REG_TEMP_XLSB 0xFC  /**< 温度数据XLSB */

/*============================ 复位命令 ============================*/
#define BMP280_RESET_VALUE 0xB6 /**< 软复位命令值 */

/*============================ 状态寄存器位定义 ============================*/
#define BMP280_STATUS_MEASURING 0x08 /**< 正在测量标志 */
#define BMP280_STATUS_IM_UPDATE 0x01 /**< NVM数据拷贝中标志 */

/*============================ 过采样配置 (CTRL_MEAS) ============================*/
/* 温度过采样 osrs_t[2:0] - bit7:5 */
#define BMP280_OSRS_T_SKIP 0x00 /**< 跳过温度测量 */
#define BMP280_OSRS_T_X1 0x20   /**< 温度过采样 ×1 */
#define BMP280_OSRS_T_X2 0x40   /**< 温度过采样 ×2 */
#define BMP280_OSRS_T_X4 0x60   /**< 温度过采样 ×4 */
#define BMP280_OSRS_T_X8 0x80   /**< 温度过采样 ×8 */
#define BMP280_OSRS_T_X16 0xE0  /**< 温度过采样 ×16 */

/* 气压过采样 osrs_p[2:0] - bit4:2 */
#define BMP280_OSRS_P_SKIP 0x00 /**< 跳过气压测量 */
#define BMP280_OSRS_P_X1 0x04   /**< 气压过采样 ×1 */
#define BMP280_OSRS_P_X2 0x08   /**< 气压过采样 ×2 */
#define BMP280_OSRS_P_X4 0x0C   /**< 气压过采样 ×4 */
#define BMP280_OSRS_P_X8 0x10   /**< 气压过采样 ×8 */
#define BMP280_OSRS_P_X16 0x14  /**< 气压过采样 ×16 */

/* 电源模式 mode[1:0] - bit1:0 */
#define BMP280_MODE_SLEEP 0x00  /**< 睡眠模式 */
#define BMP280_MODE_FORCED 0x01 /**< 强制模式 (单次测量) */
#define BMP280_MODE_NORMAL 0x03 /**< 正常模式 (连续测量) */

/*============================ 配置寄存器 (CONFIG) ============================*/
/* 待机时间 t_sb[2:0] - bit7:5 */
#define BMP280_TSB_0_5 0x00  /**< 待机时间 0.5ms */
#define BMP280_TSB_62_5 0x20 /**< 待机时间 62.5ms */
#define BMP280_TSB_125 0x40  /**< 待机时间 125ms */
#define BMP280_TSB_250 0x60  /**< 待机时间 250ms */
#define BMP280_TSB_500 0x80  /**< 待机时间 500ms */
#define BMP280_TSB_1000 0xA0 /**< 待机时间 1000ms */
#define BMP280_TSB_2000 0xC0 /**< 待机时间 2000ms */
#define BMP280_TSB_4000 0xE0 /**< 待机时间 4000ms */

/* IIR滤波器系数 filter[2:0] - bit4:2 */
#define BMP280_FILTER_OFF 0x00 /**< 滤波器关闭 */
#define BMP280_FILTER_2 0x04   /**< 滤波器系数 2 */
#define BMP280_FILTER_4 0x08   /**< 滤波器系数 4 */
#define BMP280_FILTER_8 0x0C   /**< 滤波器系数 8 */
#define BMP280_FILTER_16 0x10  /**< 滤波器系数 16 */

/* SPI 3线模式 spi3w_en[0] - bit0 */
#define BMP280_SPI3W_DISABLE 0x00 /**< 禁用SPI 3线模式 */
#define BMP280_SPI3W_ENABLE 0x01  /**< 启用SPI 3线模式 */

    /*============================ 数据结构定义 ============================*/

    /**
     * @brief BMP280校准参数结构体
     * @note  从芯片NVM读取的工厂校准数据
     */
    typedef struct
    {
        uint16_t dig_T1; /**< 温度校准参数T1 (无符号) */
        int16_t dig_T2;  /**< 温度校准参数T2 (有符号) */
        int16_t dig_T3;  /**< 温度校准参数T3 (有符号) */
        uint16_t dig_P1; /**< 气压校准参数P1 (无符号) */
        int16_t dig_P2;  /**< 气压校准参数P2 (有符号) */
        int16_t dig_P3;  /**< 气压校准参数P3 (有符号) */
        int16_t dig_P4;  /**< 气压校准参数P4 (有符号) */
        int16_t dig_P5;  /**< 气压校准参数P5 (有符号) */
        int16_t dig_P6;  /**< 气压校准参数P6 (有符号) */
        int16_t dig_P7;  /**< 气压校准参数P7 (有符号) */
        int16_t dig_P8;  /**< 气压校准参数P8 (有符号) */
        int16_t dig_P9;  /**< 气压校准参数P9 (有符号) */
    } BMP280_Calib_t;

    /**
     * @brief BMP280配置结构体
     */
    typedef struct
    {
        uint8_t osrs_t; /**< 温度过采样设置 */
        uint8_t osrs_p; /**< 气压过采样设置 */
        uint8_t mode;   /**< 电源模式 */
        uint8_t t_sb;   /**< 待机时间 */
        uint8_t filter; /**< IIR滤波器系数 */
    } BMP280_Config_t;

    /**
     * @brief BMP280原始数据结构体
     */
    typedef struct
    {
        int32_t temperature; /**< 温度原始ADC值 (20位) */
        int32_t pressure;    /**< 气压原始ADC值 (20位) */
    } BMP280_RawData_t;

    /**
     * @brief BMP280测量数据结构体
     */
    typedef struct
    {
        float temperature; /**< 温度 (°C) */
        float pressure;    /**< 气压 (Pa) */
        float altitude;    /**< 海拔高度 (m) */
    } BMP280_Data_t;

/*============================ 错误码定义 ============================*/
#define BMP280_OK 0            /**< 操作成功 */
#define BMP280_ERR_I2C -1      /**< I2C通信错误 */
#define BMP280_ERR_ID -2       /**< 芯片ID错误 */
#define BMP280_ERR_CALIB -3    /**< 校准数据读取错误 */
#define BMP280_ERR_NOT_INIT -4 /**< 设备未初始化 */
#define BMP280_ERR_PARAM -5    /**< 参数错误 */

    /*============================ 基础读写函数 ============================*/

    /**
     * @brief   向BMP280写入单个寄存器
     * @param   reg     寄存器地址
     * @param   data    写入数据
     * @return  无
     */
    void BMP280_WriteReg(uint8_t reg, uint8_t data);

    /**
     * @brief   从BMP280读取单个寄存器
     * @param   reg     寄存器地址
     * @return  寄存器值
     */
    uint8_t BMP280_ReadReg(uint8_t reg);

    /**
     * @brief   从BMP280连续读取多个寄存器
     * @param   reg     起始寄存器地址
     * @param   buf     数据缓冲区指针
     * @param   len     读取长度
     * @return  0-成功, 其他-失败
     */
    uint8_t BMP280_ReadRegs(uint8_t reg, uint8_t *buf, uint8_t len);

    /*============================ 初始化与配置函数 ============================*/

    /**
     * @brief   绑定I2C HAL接口
     * @param   hal  I2C HAL接口指针
     * @note    必须在调用任何其他BMP280函数之前调用
     */
    void BMP280_BindHAL(device_i2c_hal_t *hal);

    /**
     * @brief   BMP280初始化函数（使用默认配置）
     * @return  0-成功, 负值-错误码
     * @note    默认配置：温度×1, 气压×4, 正常模式, 待机125ms, IIR滤波×4
     */
    int8_t BMP280_Init(void);

    /**
     * @brief   使用指定配置初始化BMP280
     * @param   config  配置结构体指针
     * @return  0-成功, 负值-错误码
     */
    int8_t BMP280_InitWithConfig(BMP280_Config_t *config);

    /**
     * @brief   软复位BMP280
     * @return  0-成功, 负值-错误码
     */
    int8_t BMP280_SoftReset(void);

    /**
     * @brief   设置BMP280工作模式
     * @param   mode    工作模式 (BMP280_MODE_SLEEP/FORCED/NORMAL)
     * @return  0-成功, 负值-错误码
     */
    int8_t BMP280_SetMode(uint8_t mode);

    /**
     * @brief   配置过采样参数
     * @param   osrs_t  温度过采样设置
     * @param   osrs_p  气压过采样设置
     * @return  0-成功, 负值-错误码
     */
    int8_t BMP280_SetOversampling(uint8_t osrs_t, uint8_t osrs_p);

    /**
     * @brief   配置IIR滤波器
     * @param   filter  滤波器系数
     * @return  0-成功, 负值-错误码
     */
    int8_t BMP280_SetFilter(uint8_t filter);

    /**
     * @brief   配置待机时间
     * @param   t_sb    待机时间设置
     * @return  0-成功, 负值-错误码
     */
    int8_t BMP280_SetStandbyTime(uint8_t t_sb);

    /**
     * @brief   设置海平面参考气压
     * @param   sea_level_pa    海平面气压值 (Pa), 默认101325 Pa
     */
    void BMP280_SetSeaLevelPressure(float sea_level_pa);

    /*============================ 数据读取函数 ============================*/

    /**
     * @brief   触发一次强制测量 (FORCED模式下使用)
     * @return  0-成功, 负值-错误码
     */
    int8_t BMP280_TriggerMeasurement(void);

    /**
     * @brief   检查测量是否完成
     * @return  1-测量中, 0-测量完成, 负值-错误码
     */
    int8_t BMP280_IsMeasuring(void);

    /**
     * @brief   读取温度值
     * @return  温度值 (°C), 读取失败返回-999.0
     */
    float BMP280_ReadTemperature(void);

    /**
     * @brief   读取气压值
     * @return  气压值 (Pa), 读取失败返回-1.0
     * @note    该函数会先更新温度以获取t_fine值
     */
    float BMP280_ReadPressure(void);

    /**
     * @brief   计算海拔高度
     * @param   pressure 当前气压值 (Pa), 若为0则自动读取
     * @return  海拔高度 (m)
     */
    float BMP280_CalculateAltitude(float pressure);

    /**
     * @brief   一次性读取所有测量数据
     * @param   data    数据结构体指针
     * @return  0-成功, 负值-错误码
     */
    int8_t BMP280_ReadAllData(BMP280_Data_t *data);

    /**
     * @brief   读取原始ADC数据
     * @param   raw     原始数据结构体指针
     * @return  0-成功, 负值-错误码
     */
    int8_t BMP280_ReadRawData(BMP280_RawData_t *raw);

    /*============================ 状态与诊断函数 ============================*/

    /**
     * @brief   读取芯片ID
     * @return  芯片ID值, 失败返回0
     */
    uint8_t BMP280_ReadChipID(void);

    /**
     * @brief   读取状态寄存器
     * @return  状态寄存器值
     */
    uint8_t BMP280_ReadStatus(void);

    /**
     * @brief   检查设备是否连接
     * @return  1-设备已连接, 0-设备未连接
     */
    uint8_t BMP280_IsConnected(void);

    /**
     * @brief   获取校准参数 (调试用)
     * @param   calib   校准参数结构体指针
     * @return  0-成功, 负值-错误码
     */
    int8_t BMP280_GetCalibData(BMP280_Calib_t *calib);

    /**
     * @brief   获取当前配置
     * @param   config  配置结构体指针
     * @return  0-成功, 负值-错误码
     */
    int8_t BMP280_GetConfig(BMP280_Config_t *config);

#endif /* USE_DEVICE_BMP280 */

#ifdef __cplusplus
}
#endif

#endif /* __BMP280_H */
