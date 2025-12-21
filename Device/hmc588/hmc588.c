/**
 * @file    hmc588.c
 * @brief   HMC5883L三轴磁力计驱动源文件
 * @details 实现HMC5883L磁力计的初始化、数据读取、校准等功能
 * @note    支持软件I2C和硬件I2C两种通信方式
 */

#include "hmc588.h"
#include <math.h>

#ifdef HMC5883L

/*============================ 私有变量定义 ============================*/

/** @brief 当前增益对应的LSB/Gauss转换系数 */
static float hmc5883l_gain_factor = 390.0f; // 默认增益对应的系数

/** @brief 存储当前配置寄存器A的值 */
static uint8_t hmc5883l_cra_value = 0x70;

/** @brief 存储当前校准参数 */
static HMC5883L_Calibration_t hmc5883l_calib = {0, 0, 0, 1.0f, 1.0f, 1.0f};


/*============================ 基础读写函数实现 ============================*/

/**
 * @brief   向HMC5883L写入单个寄存器
 * @param   RegAddress  寄存器地址
 * @param   Data        要写入的数据
 * @return  无
 */
void HMC_WriteReg(uint8_t RegAddress, uint8_t Data)
{
#ifdef __SOFTI2C_
	/* 使用软件I2C写入单字节 */
	Soft_IIC_Write_Byte(&i2c_dev, HMC5883L_ADDRESS, RegAddress, Data);
#else
	/* 使用硬件I2C写入 */
	I2C_GenerateSTART(I2C1, ENABLE);				   // 生成起始条件
	HMC_WaitEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT); // 等待EV5

	I2C_Send7bitAddress(I2C1, HMC5883L_ADDRESS, I2C_Direction_Transmitter); // 发送从机地址
	HMC_WaitEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED);		// 等待EV6

	I2C_SendData(I2C1, RegAddress);							// 发送寄存器地址
	HMC_WaitEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED); // 等待EV8_2

	I2C_SendData(I2C1, Data);								// 发送数据
	HMC_WaitEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED); // 等待EV8_2

	I2C_GenerateSTOP(I2C1, ENABLE); // 生成停止条件
#endif
}

/**
 * @brief   从HMC5883L读取单个寄存器
 * @param   RegAddress  寄存器地址
 * @return  读取到的寄存器值
 */
uint8_t HMC_ReadReg(uint8_t RegAddress)
{
	uint8_t Data = 0;

#ifdef __SOFTI2C_
	/* 使用软件I2C读取单字节 */
	return Soft_IIC_Read_Byte(&i2c_dev, HMC5883L_ADDRESS, RegAddress);
#else
	/* 使用硬件I2C读取 */
	I2C_GenerateSTART(I2C1, ENABLE);				   // 生成起始条件
	HMC_WaitEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT); // 等待EV5

	I2C_Send7bitAddress(I2C1, HMC5883L_ADDRESS, I2C_Direction_Transmitter); // 发送从机地址
	HMC_WaitEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED);		// 等待EV6

	I2C_SendData(I2C1, RegAddress);							// 发送寄存器地址
	HMC_WaitEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED); // 等待EV8_2

	I2C_GenerateSTART(I2C1, ENABLE);				   // 生成重复起始条件
	HMC_WaitEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT); // 等待EV5

	I2C_Send7bitAddress(I2C1, HMC5883L_ADDRESS, I2C_Direction_Receiver); // 发送从机地址，接收方向
	HMC_WaitEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED);		 // 等待EV6

	I2C_AcknowledgeConfig(I2C1, DISABLE); // 提前禁用应答
	I2C_GenerateSTOP(I2C1, ENABLE);		  // 提前申请停止条件

	HMC_WaitEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED); // 等待EV7
	Data = I2C_ReceiveData(I2C1);						 // 接收数据

	I2C_AcknowledgeConfig(I2C1, ENABLE); // 恢复应答使能
	return Data;
#endif
}

/**
 * @brief   从HMC5883L连续读取多个字节
 * @param   RegAddress  起始寄存器地址
 * @param   len         读取长度
 * @param   buf         数据缓冲区
 * @return  0-成功，其他-失败
 */
uint8_t HMC_ReadLen(uint8_t RegAddress, uint8_t len, uint8_t *buf)
{
#ifdef __SOFTI2C_
	/* 使用软件I2C连续读取 */
	return Soft_IIC_Read_Len(&i2c_dev, HMC5883L_ADDRESS, RegAddress, len, buf);
#else
	/* 硬件I2C连续读取需要单独实现 */
	for (uint8_t i = 0; i < len; i++)
	{
		buf[i] = HMC_ReadReg(RegAddress + i);
	}
	return 0;
#endif
}

/*============================ 初始化和配置函数实现 ============================*/

/**
 * @brief   获取设备ID
 * @return  设备ID（正常应返回0x48，即'H'）
 */
uint8_t HMC_GetID(void)
{
	return HMC_ReadReg(HMC5883L_REG_IDA);
}

/**
 * @brief   检查设备是否连接
 * @return  1-设备已连接，0-设备未连接
 */
uint8_t HMC5883L_IsConnected(void)
{
	/* 检查三个ID寄存器的值是否正确 */
	if (HMC_ReadReg(HMC5883L_REG_IDA) != 'H') // 0x48
		return 0;
	if (HMC_ReadReg(HMC5883L_REG_IDB) != '4') // 0x34
		return 0;
	if (HMC_ReadReg(HMC5883L_REG_IDC) != '3') // 0x33
		return 0;
	return 1;
}

/**
 * @brief   HMC5883L初始化函数（使用默认配置）
 * @return  0-成功，-1-失败（设备未找到）
 * @note    默认配置：8次采样平均，15Hz输出，增益±4.7Ga，连续测量模式
 */
uint8_t HMC5883L_Init(void)
{
	Soft_IIC_Init(&i2c_dev); // 初始化软件I2C
	/* 检查设备是否存在 */
	if (!HMC5883L_IsConnected())
	{
		return (uint8_t)-1;
	}

	/* 配置寄存器A：8次采样平均 + 15Hz输出速率 + 正常测量模式 */
	hmc5883l_cra_value = HMC5883L_SAMPLES_8 | HMC5883L_RATE_15 | HMC5883L_MEASURE_NORMAL;
	HMC_WriteReg(HMC5883L_REG_CRA, hmc5883l_cra_value);

	/* 配置寄存器B：增益设置为±4.7Ga，390 LSB/Gauss */
	HMC_WriteReg(HMC5883L_REG_CRB, HMC5883L_GAIN_390);
	hmc5883l_gain_factor = 390.0f;

	/* 模式寄存器：连续测量模式 */
	HMC_WriteReg(HMC5883L_REG_MODE, HMC5883L_MODE_CONTINUOUS);

	/* 等待首次测量完成 */
	// delay(100);

	return 0;
}

/**
 * @brief   使用指定配置初始化HMC5883L
 * @param   config  配置结构体指针
 * @return  0-成功，-1-失败
 */
uint8_t HMC5883L_Init_Config(HMC5883L_Config_t *config)
{
	if (config == NULL)
	{
		return (uint8_t)-1;
	}

	/* 检查设备是否存在 */
	if (!HMC5883L_IsConnected())
	{
		return (uint8_t)-1;
	}

	/* 配置寄存器A */
	hmc5883l_cra_value = config->samples | config->rate | config->measure_mode;
	HMC_WriteReg(HMC5883L_REG_CRA, hmc5883l_cra_value);

	/* 配置寄存器B：增益设置 */
	HMC_WriteReg(HMC5883L_REG_CRB, config->gain);
	HMC5883L_SetGain(config->gain); // 更新增益转换系数

	/* 模式寄存器 */
	HMC_WriteReg(HMC5883L_REG_MODE, config->mode);

	/* 等待首次测量完成 */
	delay(100);

	return 0;
}

/**
 * @brief   设置采样平均数
 * @param   samples 采样平均数（使用HMC5883L_SAMPLES_x宏）
 * @return  无
 */
void HMC5883L_SetSamples(uint8_t samples)
{
	/* 更新配置寄存器A，只修改采样平均位 */
	hmc5883l_cra_value = (hmc5883l_cra_value & 0x1F) | samples;
	HMC_WriteReg(HMC5883L_REG_CRA, hmc5883l_cra_value);
}

/**
 * @brief   设置数据输出速率
 * @param   rate    输出速率（使用HMC5883L_RATE_x宏）
 * @return  无
 */
void HMC5883L_SetRate(uint8_t rate)
{
	/* 更新配置寄存器A，只修改输出速率位 */
	hmc5883l_cra_value = (hmc5883l_cra_value & 0x63) | rate;
	HMC_WriteReg(HMC5883L_REG_CRA, hmc5883l_cra_value);
}

/**
 * @brief   设置增益
 * @param   gain    增益配置（使用HMC5883L_GAIN_x宏）
 * @return  无
 * @note    同时更新内部的增益转换系数
 */
void HMC5883L_SetGain(uint8_t gain)
{
	HMC_WriteReg(HMC5883L_REG_CRB, gain);

	/* 根据增益设置更新转换系数 */
	switch (gain)
	{
	case HMC5883L_GAIN_1370:
		hmc5883l_gain_factor = 1370.0f;
		break;
	case HMC5883L_GAIN_1090:
		hmc5883l_gain_factor = 1090.0f;
		break;
	case HMC5883L_GAIN_820:
		hmc5883l_gain_factor = 820.0f;
		break;
	case HMC5883L_GAIN_660:
		hmc5883l_gain_factor = 660.0f;
		break;
	case HMC5883L_GAIN_440:
		hmc5883l_gain_factor = 440.0f;
		break;
	case HMC5883L_GAIN_390:
		hmc5883l_gain_factor = 390.0f;
		break;
	case HMC5883L_GAIN_330:
		hmc5883l_gain_factor = 330.0f;
		break;
	case HMC5883L_GAIN_230:
		hmc5883l_gain_factor = 230.0f;
		break;
	default:
		hmc5883l_gain_factor = 390.0f;
		break;
	}
}

/**
 * @brief   设置工作模式
 * @param   mode    工作模式（使用HMC5883L_MODE_x宏）
 * @return  无
 */
void HMC5883L_SetMode(uint8_t mode)
{
	HMC_WriteReg(HMC5883L_REG_MODE, mode);
}

/*============================ 数据读取函数实现 ============================*/

/**
 * @brief   检查数据是否就绪
 * @return  1-数据就绪，0-数据未就绪
 */
uint8_t HMC5883L_IsDataReady(void)
{
	uint8_t status = HMC_ReadReg(HMC5883L_REG_STATUS);
	return (status & HMC5883L_STATUS_RDY) ? 1 : 0;
}

/**
 * @brief   获取状态寄存器值
 * @return  状态寄存器值
 */
uint8_t HMC5883L_GetStatus(void)
{
	return HMC_ReadReg(HMC5883L_REG_STATUS);
}

/**
 * @brief   获取原始磁力数据（分别获取X、Y、Z）
 * @param   MagX    X轴数据指针
 * @param   MagY    Y轴数据指针
 * @param   MagZ    Z轴数据指针
 * @return  无
 * @note    HMC5883L的数据寄存器顺序为：X_H, X_L, Z_H, Z_L, Y_H, Y_L
 */
void HMC_GetData(int16_t *MagX, int16_t *MagY, int16_t *MagZ)
{
	uint8_t buf[6];

	/* 从数据寄存器起始地址连续读取6个字节 */
	HMC_ReadLen(HMC5883L_REG_OUTXM, 6, buf);

	/* 组合高低字节，注意HMC5883L的寄存器顺序 */
	/* 寄存器顺序：X_H(03), X_L(04), Z_H(05), Z_L(06), Y_H(07), Y_L(08) */
	*MagX = (int16_t)((buf[0] << 8) | buf[1]); // X轴数据
	*MagZ = (int16_t)((buf[2] << 8) | buf[3]); // Z轴数据
	*MagY = (int16_t)((buf[4] << 8) | buf[5]); // Y轴数据
}

/**
 * @brief   获取原始磁力数据（结构体形式）
 * @param   raw_data    原始数据结构体指针
 * @return  0-成功，其他-失败
 */
uint8_t HMC5883L_GetRawData(HMC5883L_RawData_t *raw_data)
{
	if (raw_data == NULL)
	{
		return 1;
	}

	HMC_GetData(&raw_data->x, &raw_data->y, &raw_data->z);
	return 0;
}

/**
 * @brief   获取磁场数据（转换为高斯单位）
 * @param   mag_data    磁场数据结构体指针
 * @return  0-成功，其他-失败
 */
uint8_t HMC5883L_GetMagData(HMC5883L_MagData_t *mag_data)
{
	int16_t raw_x, raw_y, raw_z;

	if (mag_data == NULL)
	{
		return 1;
	}

	/* 获取原始数据 */
	HMC_GetData(&raw_x, &raw_y, &raw_z);

	/* 转换为高斯单位 */
	mag_data->x = (float)raw_x / hmc5883l_gain_factor;
	mag_data->y = (float)raw_y / hmc5883l_gain_factor;
	mag_data->z = (float)raw_z / hmc5883l_gain_factor;

	return 0;
}

/**
 * @brief   获取校准后的磁场数据
 * @param   mag_data    磁场数据结构体指针
 * @param   calib       校准参数结构体指针
 * @return  0-成功，其他-失败
 */
uint8_t HMC5883L_GetCalibratedData(HMC5883L_MagData_t *mag_data, HMC5883L_Calibration_t *calib)
{
	int16_t raw_x, raw_y, raw_z;

	if (mag_data == NULL || calib == NULL)
	{
		return 1;
	}

	/* 获取原始数据 */
	HMC_GetData(&raw_x, &raw_y, &raw_z);

	/* 应用校准偏移和比例因子，然后转换为高斯单位 */
	mag_data->x = ((float)(raw_x - calib->offset_x) * calib->scale_x) / hmc5883l_gain_factor;
	mag_data->y = ((float)(raw_y - calib->offset_y) * calib->scale_y) / hmc5883l_gain_factor;
	mag_data->z = ((float)(raw_z - calib->offset_z) * calib->scale_z) / hmc5883l_gain_factor;

	return 0;
}

/*============================ 航向角计算函数实现 ============================*/

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

/**
 * @brief   计算航向角（方位角）
 * @return  航向角，范围0-360度，正北为0度，顺时针增加
 * @note    未进行倾斜补偿，需保持传感器水平放置
 */
float HMC5883L_GetHeading(void)
{
	int16_t raw_x, raw_y, raw_z;
	float heading;

	/* 获取原始数据 */
	HMC_GetData(&raw_x, &raw_y, &raw_z);

	/* 计算航向角（弧度） */
	heading = atan2f((float)raw_y, (float)raw_x);

	/* 转换为度数 */
	heading = heading * 180.0f / M_PI;

	/* 转换为0-360度范围 */
	if (heading < 0)
	{
		heading += 360.0f;
	}

	return heading;
}

/**
 * @brief   使用校准数据计算航向角
 * @param   calib   校准参数结构体指针
 * @return  航向角，范围0-360度
 */
float HMC5883L_GetCalibratedHeading(HMC5883L_Calibration_t *calib)
{
	int16_t raw_x, raw_y, raw_z;
	float cal_x, cal_y;
	float heading;

	if (calib == NULL)
	{
		return HMC5883L_GetHeading();
	}

	/* 获取原始数据 */
	HMC_GetData(&raw_x, &raw_y, &raw_z);

	/* 应用校准 */
	cal_x = (float)(raw_x - calib->offset_x) * calib->scale_x;
	cal_y = (float)(raw_y - calib->offset_y) * calib->scale_y;

	/* 计算航向角 */
	heading = atan2f(cal_y, cal_x) * 180.0f / M_PI;

	/* 转换为0-360度范围 */
	if (heading < 0)
	{
		heading += 360.0f;
	}

	return heading;
}

/**
 * @brief   带倾斜补偿的航向角计算
 * @param   pitch   俯仰角（弧度）
 * @param   roll    横滚角（弧度）
 * @return  补偿后的航向角，范围0-360度
 * @note    需要从加速度计获取俯仰角和横滚角
 */
float HMC5883L_GetTiltCompensatedHeading(float pitch, float roll)
{
	int16_t raw_x, raw_y, raw_z;
	float cos_pitch, sin_pitch;
	float cos_roll, sin_roll;
	float xh, yh;
	float heading;

	/* 获取原始数据 */
	HMC_GetData(&raw_x, &raw_y, &raw_z);

	/* 预计算三角函数值 */
	cos_pitch = cosf(pitch);
	sin_pitch = sinf(pitch);
	cos_roll = cosf(roll);
	sin_roll = sinf(roll);

	/* 倾斜补偿计算 */
	/* 将磁力计读数投影到水平面上 */
	xh = (float)raw_x * cos_pitch + (float)raw_y * sin_roll * sin_pitch + (float)raw_z * cos_roll * sin_pitch;
	yh = (float)raw_y * cos_roll - (float)raw_z * sin_roll;

	/* 计算航向角 */
	heading = atan2f(yh, xh) * 180.0f / M_PI;

	/* 转换为0-360度范围 */
	if (heading < 0)
	{
		heading += 360.0f;
	}

	return heading;
}

/*============================ 校准函数实现 ============================*/

/**
 * @brief   自检测试
 * @return  0-自检通过，其他-自检失败
 * @note    使用内部正/负偏置测试功能进行自检
 */
uint8_t HMC5883L_SelfTest(void)
{
	int16_t x_pos, y_pos, z_pos;
	int16_t x_neg, y_neg, z_neg;
	int16_t x_diff, y_diff, z_diff;
	uint8_t result = 0;

	/* 保存当前配置 */
	uint8_t old_cra = HMC_ReadReg(HMC5883L_REG_CRA);
	uint8_t old_crb = HMC_ReadReg(HMC5883L_REG_CRB);
	uint8_t old_mode = HMC_ReadReg(HMC5883L_REG_MODE);

	/* 设置为正偏置自检模式 */
	HMC_WriteReg(HMC5883L_REG_CRA, HMC5883L_SAMPLES_8 | HMC5883L_RATE_15 | HMC5883L_MEASURE_POSITIVE);
	HMC_WriteReg(HMC5883L_REG_CRB, HMC5883L_GAIN_390);
	HMC_WriteReg(HMC5883L_REG_MODE, HMC5883L_MODE_SINGLE);

	delay(70); // 等待测量完成

	/* 读取正偏置数据 */
	HMC_GetData(&x_pos, &y_pos, &z_pos);

	/* 设置为负偏置自检模式 */
	HMC_WriteReg(HMC5883L_REG_CRA, HMC5883L_SAMPLES_8 | HMC5883L_RATE_15 | HMC5883L_MEASURE_NEGATIVE);
	HMC_WriteReg(HMC5883L_REG_MODE, HMC5883L_MODE_SINGLE);

	delay(70); // 等待测量完成

	/* 读取负偏置数据 */
	HMC_GetData(&x_neg, &y_neg, &z_neg);

	/* 计算差值 */
	x_diff = x_pos - x_neg;
	y_diff = y_pos - y_neg;
	z_diff = z_pos - z_neg;

	/* 检查差值是否在合理范围内 */
	/* 对于390 LSB/Gauss增益，正常差值大约在 ±1.1Gauss * 390 = ±430 LSB左右 */
	/* 这里使用宽松的范围进行检查 */
	if (x_diff < 100 || x_diff > 1000)
		result |= 0x01;
	if (y_diff < 100 || y_diff > 1000)
		result |= 0x02;
	if (z_diff < 100 || z_diff > 1000)
		result |= 0x04;

	/* 恢复原始配置 */
	HMC_WriteReg(HMC5883L_REG_CRA, old_cra);
	HMC_WriteReg(HMC5883L_REG_CRB, old_crb);
	HMC_WriteReg(HMC5883L_REG_MODE, old_mode);

	delay(70); // 等待恢复

	return result;
}

/**
 * @brief   开始校准（需要将传感器旋转一周以上）
 * @param   calib       校准参数结构体指针（用于存储校准结果）
 * @param   samples     采样次数（建议>=100）
 * @return  0-校准成功，其他-校准失败
 * @note    调用此函数后，需要缓慢旋转传感器360度
 */
uint8_t HMC5883L_Calibrate(HMC5883L_Calibration_t *calib, uint16_t samples)
{
	int16_t raw_x, raw_y, raw_z;
	int16_t x_min = 32767, x_max = -32768;
	int16_t y_min = 32767, y_max = -32768;
	int16_t z_min = 32767, z_max = -32768;
	float avg_delta;

	if (calib == NULL || samples < 10)
	{
		return 1;
	}

	/* 采集数据，记录各轴的最大最小值 */
	for (uint16_t i = 0; i < samples; i++)
	{
		/* 等待数据就绪 */
		while (!HMC5883L_IsDataReady())
		{
			delay(5);
		}

		/* 读取原始数据 */
		HMC_GetData(&raw_x, &raw_y, &raw_z);

		/* 更新最大最小值 */
		if (raw_x < x_min)
			x_min = raw_x;
		if (raw_x > x_max)
			x_max = raw_x;
		if (raw_y < y_min)
			y_min = raw_y;
		if (raw_y > y_max)
			y_max = raw_y;
		if (raw_z < z_min)
			z_min = raw_z;
		if (raw_z > z_max)
			z_max = raw_z;

		/* 采样间隔 */
		delay(50);
	}

	/* 计算硬铁偏移（椭圆中心） */
	calib->offset_x = (x_max + x_min) / 2;
	calib->offset_y = (y_max + y_min) / 2;
	calib->offset_z = (z_max + z_min) / 2;

	/* 计算软铁补偿（缩放因子） */
	/* 将各轴的范围归一化 */
	float delta_x = (float)(x_max - x_min) / 2.0f;
	float delta_y = (float)(y_max - y_min) / 2.0f;
	float delta_z = (float)(z_max - z_min) / 2.0f;

	/* 计算平均半径 */
	avg_delta = (delta_x + delta_y + delta_z) / 3.0f;

	/* 计算各轴的缩放因子 */
	if (delta_x != 0)
	{
		calib->scale_x = avg_delta / delta_x;
	}
	else
	{
		calib->scale_x = 1.0f;
	}

	if (delta_y != 0)
	{
		calib->scale_y = avg_delta / delta_y;
	}
	else
	{
		calib->scale_y = 1.0f;
	}

	if (delta_z != 0)
	{
		calib->scale_z = avg_delta / delta_z;
	}
	else
	{
		calib->scale_z = 1.0f;
	}

	return 0;
}

/**
 * @brief   应用校准参数
 * @param   calib   校准参数结构体指针
 * @return  无
 */
void HMC5883L_ApplyCalibration(HMC5883L_Calibration_t *calib)
{
	if (calib == NULL)
	{
		return;
	}

	/* 将校准参数保存到全局变量 */
	hmc5883l_calib.offset_x = calib->offset_x;
	hmc5883l_calib.offset_y = calib->offset_y;
	hmc5883l_calib.offset_z = calib->offset_z;
	hmc5883l_calib.scale_x = calib->scale_x;
	hmc5883l_calib.scale_y = calib->scale_y;
	hmc5883l_calib.scale_z = calib->scale_z;
}

/**
 * @brief   软复位（重新初始化）
 * @return  0-成功，其他-失败
 */
uint8_t HMC5883L_Reset(void)
{
	return HMC5883L_Init();
}

#endif /* HMC5883L */
