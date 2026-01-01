/**
 * @file    bmp280.c
 * @brief   BMP280气压温度传感器驱动实现
 * @details 基于博世官方补偿算法，适配df_iic软件I2C框架
 *          风格与HMC5883L驱动保持一致，使用静态全局变量存储状态
 * @note    参考文档: BST-BMP280-DS001
 *
 * @example 使用示例:
 *          // 1. 初始化BMP280 (使用默认配置)
 *          if (BMP280_Init() == BMP280_OK) {
 *              printf("BMP280初始化成功\n");
 *          }
 *
 *          // 2. 读取数据
 *          BMP280_Data_t data;
 *          BMP280_ReadAllData(&data);
 *          printf("温度: %.2f°C, 气压: %.2fPa, 海拔: %.2fm\n",
 *                 data.temperature, data.pressure, data.altitude);
 *
 *          // 3. 或者单独读取
 *          float temp = BMP280_ReadTemperature();
 *          float press = BMP280_ReadPressure();
 *          float alt = BMP280_CalculateAltitude(press);
 */

#include "bmp280.h"

#ifdef USE_DEVICE_BMP280

#include <string.h>
#include <df_delay.h>

extern Dt delay;
#include <math.h>

/*============================ HAL接口实例 ============================*/
device_i2c_hal_t *bmp280_i2c_hal = NULL; /**< BMP280 I2C HAL接口实例 */

/*============================ 私有宏定义 ============================*/
#define BMP280_CALIB_DATA_LEN 26 /**< 校准数据长度 (字节) */
#define BMP280_DATA_LEN 6        /**< 测量数据长度 (字节) */

/* 默认海平面气压 (Pa) */
#define BMP280_SEA_LEVEL_PA_DEFAULT 101325.0f

/*============================ 私有变量定义 ============================*/

/** @brief 校准参数缓存 */
static BMP280_Calib_t bmp280_calib = {0};

/** @brief 当前配置 */
static BMP280_Config_t bmp280_config = {
    .osrs_t = BMP280_OSRS_T_X1,
    .osrs_p = BMP280_OSRS_P_X4,
    .mode = BMP280_MODE_NORMAL,
    .t_sb = BMP280_TSB_125,
    .filter = BMP280_FILTER_4};

/** @brief 温度精细值 (用于气压补偿计算) */
static int32_t bmp280_t_fine = 0;

/** @brief 海平面参考气压 */
static float bmp280_sea_level_pa = BMP280_SEA_LEVEL_PA_DEFAULT;

/** @brief 芯片ID */
static uint8_t bmp280_chip_id = 0;

/** @brief 初始化标志 */
static uint8_t bmp280_initialized = 0;

/*============================ 私有函数声明 ============================*/
static int8_t BMP280_ReadCalibData(void);
static int32_t BMP280_CompensateTemp(int32_t adc_T);
static uint32_t BMP280_CompensatePressure(int32_t adc_P);

/*============================ 基础读写函数 ============================*/

/**
 * @brief   向BMP280写入单个寄存器
 */
void BMP280_WriteReg(uint8_t reg, uint8_t data)
{
    if (bmp280_i2c_hal && bmp280_i2c_hal->initialized)
    {
        bmp280_i2c_hal->write_byte(BMP280_I2C_ADDR, reg, data);
    }
}

/**
 * @brief   从BMP280读取单个寄存器
 */
uint8_t BMP280_ReadReg(uint8_t reg)
{
    uint8_t data = 0;
    if (bmp280_i2c_hal && bmp280_i2c_hal->initialized)
    {
        bmp280_i2c_hal->read_byte(BMP280_I2C_ADDR, reg, &data);
    }
    return data;
}

/**
 * @brief   从BMP280连续读取多个寄存器
 */
uint8_t BMP280_ReadRegs(uint8_t reg, uint8_t *buf, uint8_t len)
{
    if (bmp280_i2c_hal && bmp280_i2c_hal->initialized)
    {
        return bmp280_i2c_hal->read_bytes(BMP280_I2C_ADDR, reg, len, buf);
    }
    return -1;
}

/*============================ 私有函数实现 ============================*/

/**
 * @brief   读取校准参数
 */
static int8_t BMP280_ReadCalibData(void)
{
    uint8_t calib_data[BMP280_CALIB_DATA_LEN];

    /* 读取校准数据 (0x88 - 0xA1, 共26字节) */
    if (BMP280_ReadRegs(BMP280_REG_CALIB00, calib_data, BMP280_CALIB_DATA_LEN) != 0)
    {
        return BMP280_ERR_CALIB;
    }

    /* 解析温度校准参数 */
    bmp280_calib.dig_T1 = (uint16_t)(calib_data[1] << 8 | calib_data[0]);
    bmp280_calib.dig_T2 = (int16_t)(calib_data[3] << 8 | calib_data[2]);
    bmp280_calib.dig_T3 = (int16_t)(calib_data[5] << 8 | calib_data[4]);

    /* 解析气压校准参数 */
    bmp280_calib.dig_P1 = (uint16_t)(calib_data[7] << 8 | calib_data[6]);
    bmp280_calib.dig_P2 = (int16_t)(calib_data[9] << 8 | calib_data[8]);
    bmp280_calib.dig_P3 = (int16_t)(calib_data[11] << 8 | calib_data[10]);
    bmp280_calib.dig_P4 = (int16_t)(calib_data[13] << 8 | calib_data[12]);
    bmp280_calib.dig_P5 = (int16_t)(calib_data[15] << 8 | calib_data[14]);
    bmp280_calib.dig_P6 = (int16_t)(calib_data[17] << 8 | calib_data[16]);
    bmp280_calib.dig_P7 = (int16_t)(calib_data[19] << 8 | calib_data[18]);
    bmp280_calib.dig_P8 = (int16_t)(calib_data[21] << 8 | calib_data[20]);
    bmp280_calib.dig_P9 = (int16_t)(calib_data[23] << 8 | calib_data[22]);

    return BMP280_OK;
}

/**
 * @brief   温度补偿算法 (博世官方算法)
 * @param   adc_T   温度ADC原始值
 * @return  补偿后的温度值 (分辨率0.01°C)
 */
static int32_t BMP280_CompensateTemp(int32_t adc_T)
{
    int32_t var1, var2, T;

    var1 = ((((adc_T >> 3) - ((int32_t)bmp280_calib.dig_T1 << 1))) * ((int32_t)bmp280_calib.dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((int32_t)bmp280_calib.dig_T1)) * ((adc_T >> 4) - ((int32_t)bmp280_calib.dig_T1))) >> 12) * ((int32_t)bmp280_calib.dig_T3)) >> 14;

    bmp280_t_fine = var1 + var2;
    T = (bmp280_t_fine * 5 + 128) >> 8;

    return T;
}

/**
 * @brief   气压补偿算法 (博世官方64位算法)
 * @param   adc_P   气压ADC原始值
 * @return  补偿后的气压值 (单位: Pa, 24位整数, 8位小数)
 */
static uint32_t BMP280_CompensatePressure(int32_t adc_P)
{
    int64_t var1, var2, p;

    var1 = ((int64_t)bmp280_t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)bmp280_calib.dig_P6;
    var2 = var2 + ((var1 * (int64_t)bmp280_calib.dig_P5) << 17);
    var2 = var2 + (((int64_t)bmp280_calib.dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)bmp280_calib.dig_P3) >> 8) + ((var1 * (int64_t)bmp280_calib.dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)bmp280_calib.dig_P1) >> 33;

    if (var1 == 0)
    {
        return 0; /* 避免除零异常 */
    }

    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)bmp280_calib.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)bmp280_calib.dig_P8) * p) >> 19;

    p = ((p + var1 + var2) >> 8) + (((int64_t)bmp280_calib.dig_P7) << 4);

    return (uint32_t)p;
}

/*============================ 初始化与配置函数 ============================*/

/**
 * @brief   绑定I2C HAL接口
 * @param   hal  I2C HAL接口指针
 * @note    必须在调用任何其他BMP280函数之前调用
 */
void BMP280_BindHAL(device_i2c_hal_t *hal)
{
    bmp280_i2c_hal = hal;
}

/**
 * @brief   BMP280初始化函数（使用默认配置）
 * @return  0-成功, 负值-错误码
 * @note    默认配置：温度×1, 气压×4, 正常模式, 待机125ms, IIR滤波×4
 */
int8_t BMP280_Init(void)
{
    /* 检查HAL接口是否已绑定 */
    if (!bmp280_i2c_hal || !bmp280_i2c_hal->initialized)
    {
        return BMP280_ERR_NOT_INIT;
    }

    /* 检查设备是否连接 */
    if (!BMP280_IsConnected())
    {
        return BMP280_ERR_I2C;
    }

    /* 读取并验证芯片ID */
    // bmp280_chip_id = BMP280_ReadChipID();
    // if (bmp280_chip_id != BMP280_CHIP_ID && bmp280_chip_id != BME280_CHIP_ID)
    // {
    //     return BMP280_ERR_ID;
    // }

    /* 软复位 */
    BMP280_SoftReset();
    if (bmp280_i2c_hal->delay_ms)
    {
        bmp280_i2c_hal->delay_ms(10);
    }

    /* 读取校准参数 */
    if (BMP280_ReadCalibData() != BMP280_OK)
    {
        return BMP280_ERR_CALIB;
    }

    /* 使用默认配置 */
    bmp280_config.osrs_t = BMP280_OSRS_T_X1;
    bmp280_config.osrs_p = BMP280_OSRS_P_X4;
    bmp280_config.mode = BMP280_MODE_NORMAL;
    bmp280_config.t_sb = BMP280_TSB_125;
    bmp280_config.filter = BMP280_FILTER_4;

    /* 写入CONFIG寄存器 */
    uint8_t config_val = bmp280_config.t_sb | bmp280_config.filter;
    BMP280_WriteReg(BMP280_REG_CONFIG, config_val);

    /* 写入CTRL_MEAS寄存器 */
    uint8_t ctrl_meas_val = bmp280_config.osrs_t | bmp280_config.osrs_p | bmp280_config.mode;
    BMP280_WriteReg(BMP280_REG_CTRL_MEAS, ctrl_meas_val);

    bmp280_initialized = 1;

    return BMP280_OK;
}

/**
 * @brief   使用指定配置初始化BMP280
 */
int8_t BMP280_InitWithConfig(BMP280_Config_t *config)
{
    if (config == NULL)
    {
        return BMP280_ERR_PARAM;
    }

#ifdef __SOFTI2C_
    /* 初始化软件I2C */
    Soft_IIC_Init(&i2c_Dev);
#endif

    /* 检查设备是否连接 */
    if (!BMP280_IsConnected())
    {
        return BMP280_ERR_I2C;
    }

    /* 读取并验证芯片ID */
    bmp280_chip_id = BMP280_ReadChipID();
    if (bmp280_chip_id != BMP280_CHIP_ID && bmp280_chip_id != BME280_CHIP_ID)
    {
        return BMP280_ERR_ID;
    }

    /* 软复位 */
    BMP280_SoftReset();
    delay.ms(10);

    /* 读取校准参数 */
    if (BMP280_ReadCalibData() != BMP280_OK)
    {
        return BMP280_ERR_CALIB;
    }

    /* 使用传入的配置 */
    memcpy(&bmp280_config, config, sizeof(BMP280_Config_t));

    /* 写入CONFIG寄存器 */
    uint8_t config_val = bmp280_config.t_sb | bmp280_config.filter;
    BMP280_WriteReg(BMP280_REG_CONFIG, config_val);

    /* 写入CTRL_MEAS寄存器 */
    uint8_t ctrl_meas_val = bmp280_config.osrs_t | bmp280_config.osrs_p | bmp280_config.mode;
    BMP280_WriteReg(BMP280_REG_CTRL_MEAS, ctrl_meas_val);

    bmp280_initialized = 1;

    return BMP280_OK;
}

/**
 * @brief   软复位BMP280
 */
int8_t BMP280_SoftReset(void)
{
    BMP280_WriteReg(BMP280_REG_RESET, BMP280_RESET_VALUE);
    return BMP280_OK;
}

/**
 * @brief   设置BMP280工作模式
 */
int8_t BMP280_SetMode(uint8_t mode)
{
    uint8_t ctrl_meas;

    if (!bmp280_initialized)
    {
        return BMP280_ERR_NOT_INIT;
    }

    /* 读取当前配置 */
    ctrl_meas = BMP280_ReadReg(BMP280_REG_CTRL_MEAS);

    /* 清除模式位并设置新模式 */
    ctrl_meas = (ctrl_meas & 0xFC) | (mode & 0x03);
    bmp280_config.mode = mode;

    BMP280_WriteReg(BMP280_REG_CTRL_MEAS, ctrl_meas);

    return BMP280_OK;
}

/**
 * @brief   配置过采样参数
 */
int8_t BMP280_SetOversampling(uint8_t osrs_t, uint8_t osrs_p)
{
    uint8_t ctrl_meas;

    if (!bmp280_initialized)
    {
        return BMP280_ERR_NOT_INIT;
    }

    bmp280_config.osrs_t = osrs_t;
    bmp280_config.osrs_p = osrs_p;

    ctrl_meas = osrs_t | osrs_p | bmp280_config.mode;
    BMP280_WriteReg(BMP280_REG_CTRL_MEAS, ctrl_meas);

    return BMP280_OK;
}

/**
 * @brief   配置IIR滤波器
 */
int8_t BMP280_SetFilter(uint8_t filter)
{
    uint8_t config_val;

    if (!bmp280_initialized)
    {
        return BMP280_ERR_NOT_INIT;
    }

    bmp280_config.filter = filter;
    config_val = bmp280_config.t_sb | filter;

    BMP280_WriteReg(BMP280_REG_CONFIG, config_val);

    return BMP280_OK;
}

/**
 * @brief   配置待机时间
 */
int8_t BMP280_SetStandbyTime(uint8_t t_sb)
{
    uint8_t config_val;

    if (!bmp280_initialized)
    {
        return BMP280_ERR_NOT_INIT;
    }

    bmp280_config.t_sb = t_sb;
    config_val = t_sb | bmp280_config.filter;

    BMP280_WriteReg(BMP280_REG_CONFIG, config_val);

    return BMP280_OK;
}

/**
 * @brief   设置海平面参考气压
 */
void BMP280_SetSeaLevelPressure(float sea_level_pa)
{
    bmp280_sea_level_pa = sea_level_pa;
}

/*============================ 数据读取函数 ============================*/

/**
 * @brief   触发一次强制测量
 */
int8_t BMP280_TriggerMeasurement(void)
{
    return BMP280_SetMode(BMP280_MODE_FORCED);
}

/**
 * @brief   检查测量是否完成
 */
int8_t BMP280_IsMeasuring(void)
{
    uint8_t status;

    if (!bmp280_initialized)
    {
        return BMP280_ERR_NOT_INIT;
    }

    status = BMP280_ReadReg(BMP280_REG_STATUS);

    return (status & BMP280_STATUS_MEASURING) ? 1 : 0;
}

/**
 * @brief   读取原始ADC数据
 */
int8_t BMP280_ReadRawData(BMP280_RawData_t *raw)
{
    uint8_t data[BMP280_DATA_LEN];

    if (raw == NULL || !bmp280_initialized)
    {
        return BMP280_ERR_NOT_INIT;
    }

    /* 一次性读取全部6字节数据 (0xF7 - 0xFC) */
    if (BMP280_ReadRegs(BMP280_REG_PRESS_MSB, data, BMP280_DATA_LEN) != 0)
    {
        return BMP280_ERR_I2C;
    }

    /* 解析20位气压ADC值 */
    raw->pressure = ((int32_t)data[0] << 12) | ((int32_t)data[1] << 4) | ((int32_t)data[2] >> 4);

    /* 解析20位温度ADC值 */
    raw->temperature = ((int32_t)data[3] << 12) | ((int32_t)data[4] << 4) | ((int32_t)data[5] >> 4);

    return BMP280_OK;
}

/**
 * @brief   读取温度值
 */
float BMP280_ReadTemperature(void)
{
    BMP280_RawData_t raw;
    int32_t temp_raw;

    if (!bmp280_initialized)
    {
        return -999.0f;
    }

    if (BMP280_ReadRawData(&raw) != BMP280_OK)
    {
        return -999.0f;
    }

    temp_raw = BMP280_CompensateTemp(raw.temperature);

    return (float)temp_raw / 100.0f;
}

/**
 * @brief   读取气压值
 */
float BMP280_ReadPressure(void)
{
    BMP280_RawData_t raw;
    int32_t temp_raw;
    uint32_t press_raw;

    if (!bmp280_initialized)
    {
        return -1.0f;
    }

    if (BMP280_ReadRawData(&raw) != BMP280_OK)
    {
        return -1.0f;
    }

    /* 必须先计算温度以获取t_fine */
    temp_raw = BMP280_CompensateTemp(raw.temperature);
    (void)temp_raw; /* 消除未使用警告 */

    press_raw = BMP280_CompensatePressure(raw.pressure);

    return (float)press_raw / 256.0f;
}

/**
 * @brief   计算海拔高度
 */
float BMP280_CalculateAltitude(float pressure)
{
    float altitude;

    /* 如果未提供气压值，自动读取 */
    if (pressure <= 0)
    {
        pressure = BMP280_ReadPressure();
    }

    if (pressure <= 0)
    {
        return 0.0f;
    }

    /* 国际气压高度公式 */
    /* h = 44330 * (1 - (P/P0)^(1/5.255)) */
    altitude = 44330.0f * (1.0f - powf(pressure / bmp280_sea_level_pa, 0.190295f));

    return altitude;
}

/**
 * @brief   一次性读取所有测量数据
 */
int8_t BMP280_ReadAllData(BMP280_Data_t *data)
{
    BMP280_RawData_t raw;
    int32_t temp_raw;
    uint32_t press_raw;

    if (data == NULL || !bmp280_initialized)
    {
        return BMP280_ERR_NOT_INIT;
    }

    /* 读取原始数据 */
    if (BMP280_ReadRawData(&raw) != BMP280_OK)
    {
        return BMP280_ERR_I2C;
    }

    /* 温度补偿 */
    temp_raw = BMP280_CompensateTemp(raw.temperature);
    data->temperature = (float)temp_raw / 100.0f;

    /* 气压补偿 */
    press_raw = BMP280_CompensatePressure(raw.pressure);
    data->pressure = (float)press_raw / 256.0f;

    /* 计算海拔 */
    data->altitude = 44330.0f * (1.0f - powf(data->pressure / bmp280_sea_level_pa, 0.190295f));

    return BMP280_OK;
}

/*============================ 状态与诊断函数 ============================*/

/**
 * @brief   读取芯片ID
 */
uint8_t BMP280_ReadChipID(void)
{
    return BMP280_ReadReg(BMP280_REG_ID);
}

/**
 * @brief   读取状态寄存器
 */
uint8_t BMP280_ReadStatus(void)
{
    return BMP280_ReadReg(BMP280_REG_STATUS);
}

/**
 * @brief   检查设备是否连接
 * @return  1-已连接, 0-未连接
 */
uint8_t BMP280_IsConnected(void)
{

    if (Soft_IIC_Cheak(&i2c_Dev, BMP280_I2C_ADDR))
    {
        return 0;
    }

    return 1;
}

/**
 * @brief   获取校准参数
 */
int8_t BMP280_GetCalibData(BMP280_Calib_t *calib)
{
    if (calib == NULL || !bmp280_initialized)
    {
        return BMP280_ERR_NOT_INIT;
    }

    memcpy(calib, &bmp280_calib, sizeof(BMP280_Calib_t));

    return BMP280_OK;
}

/**
 * @brief   获取当前配置
 */
int8_t BMP280_GetConfig(BMP280_Config_t *config)
{
    if (config == NULL || !bmp280_initialized)
    {
        return BMP280_ERR_NOT_INIT;
    }

    memcpy(config, &bmp280_config, sizeof(BMP280_Config_t));

    return BMP280_OK;
}

#endif /* USE_DEVICE_BMP280 */
