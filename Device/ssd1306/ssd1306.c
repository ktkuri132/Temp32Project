/**
 * @file ssd1306.c
 * @brief SSD1306 OLED显示屏驱动实现
 * @note 使用 device_hal.h 统一 HAL 接口，支持 I2C/SPI 两种通信方式
 */

#include "ssd1306.h"

#ifdef USE_DEVICE_SSD1306

/*============================ HAL接口实例 ============================*/
device_i2c_hal_t *ssd1306_i2c_hal = NULL;
device_spi_hal_t *ssd1306_spi_hal = NULL;

/* 通信模式 */
typedef enum
{
    SSD1306_MODE_NONE = 0,
    SSD1306_MODE_I2C,
    SSD1306_MODE_SPI
} ssd1306_mode_t;

static ssd1306_mode_t ssd1306_mode = SSD1306_MODE_NONE;

/* 显存缓冲区 */
uint8_t SSD1306_DisplayBuf[8][128];

/*============================ 底层通信函数 ============================*/

/**
 * @brief 写入命令到SSD1306 (I2C模式)
 */
static int SSD1306_I2C_WriteCommand(uint8_t command)
{
    if (!ssd1306_i2c_hal || !ssd1306_i2c_hal->initialized)
        return -1;
    return ssd1306_i2c_hal->write_byte(SSD1306_ADDRESS, SSD1306_Command_Mode, command);
}

/**
 * @brief 写入数据到SSD1306 (I2C模式)
 */
static int SSD1306_I2C_WriteData(uint8_t *data, uint8_t count)
{
    if (!ssd1306_i2c_hal || !ssd1306_i2c_hal->initialized)
        return -1;
    return ssd1306_i2c_hal->write_bytes(SSD1306_ADDRESS, SSD1306_Data_Mode, count, data);
}

/**
 * @brief 写入命令到SSD1306 (SPI模式)
 */
static int SSD1306_SPI_WriteCommand(uint8_t command)
{
    if (!ssd1306_spi_hal || !ssd1306_spi_hal->initialized)
        return -1;

    ssd1306_spi_hal->cs_control(true);
    /* DC=0表示命令 */
    ssd1306_spi_hal->transfer_byte(command);
    ssd1306_spi_hal->cs_control(false);
    return 0;
}

/**
 * @brief 写入数据到SSD1306 (SPI模式)
 */
static int SSD1306_SPI_WriteData(uint8_t *data, uint8_t count)
{
    if (!ssd1306_spi_hal || !ssd1306_spi_hal->initialized)
        return -1;

    ssd1306_spi_hal->cs_control(true);
    /* DC=1表示数据 */
    ssd1306_spi_hal->transfer_bytes(data, NULL, count);
    ssd1306_spi_hal->cs_control(false);
    return 0;
}

/**
 * @brief 写入命令到SSD1306 (统一接口)
 */
static int SSD1306_WriteCommand(uint8_t command)
{
    if (ssd1306_mode == SSD1306_MODE_I2C)
        return SSD1306_I2C_WriteCommand(command);
    else if (ssd1306_mode == SSD1306_MODE_SPI)
        return SSD1306_SPI_WriteCommand(command);
    return -1;
}

/**
 * @brief 写入数据到SSD1306 (统一接口)
 */
static int SSD1306_WriteData(uint8_t *data, uint8_t count)
{
    if (ssd1306_mode == SSD1306_MODE_I2C)
        return SSD1306_I2C_WriteData(data, count);
    else if (ssd1306_mode == SSD1306_MODE_SPI)
        return SSD1306_SPI_WriteData(data, count);
    return -1;
}

/**
 * @brief 检查SSD1306设备应答 (仅I2C模式)
 */
static int SSD1306_Device_AckCheck(void)
{
    if (ssd1306_mode != SSD1306_MODE_I2C)
        return 0;

    if (!ssd1306_i2c_hal || !ssd1306_i2c_hal->initialized)
        return -1;

    uint8_t dummy;
    return ssd1306_i2c_hal->read_byte(SSD1306_ADDRESS, 0x00, &dummy);
}

/*============================ HAL初始化函数 ============================*/

/**
 * @brief 初始化SSD1306并绑定I2C HAL接口
 * @param hal I2C HAL接口指针
 * @return 0-成功，非0-失败
 */
int SSD1306_Init_HAL_I2C(device_i2c_hal_t *hal)
{
    if (!hal || !hal->initialized)
        return -1;

    ssd1306_i2c_hal = hal;
    ssd1306_mode = SSD1306_MODE_I2C;
    return 0;
}

/**
 * @brief 初始化SSD1306并绑定SPI HAL接口
 * @param hal SPI HAL接口指针
 * @return 0-成功，非0-失败
 */
int SSD1306_Init_HAL_SPI(device_spi_hal_t *hal)
{
    if (!hal || !hal->initialized)
        return -1;

    ssd1306_spi_hal = hal;
    ssd1306_mode = SSD1306_MODE_SPI;
    return 0;
}

/**
 * @brief 兼容旧接口
 */
int SSD1306_Init_HAL(device_i2c_hal_t *hal)
{
    return SSD1306_Init_HAL_I2C(hal);
}

/**
 * @brief 初始化SSD1306
 * @return 0-成功，非0-失败
 */
uint8_t SSD1306_Init(void)
{
    if (ssd1306_mode == SSD1306_MODE_NONE)
        return 1;

    if (SSD1306_WriteCommand(0xAE)) // 关闭显示
        return 1;

    SSD1306_WriteCommand(0xD5); // 设置显示时钟分频比/振荡器频率
    SSD1306_WriteCommand(0x80); // 0x00~0xFF

    SSD1306_WriteCommand(0xA8); // 设置多路复用率
    SSD1306_WriteCommand(0x3F); // 0x0E~0x3F

    SSD1306_WriteCommand(0xD3); // 设置显示偏移
    SSD1306_WriteCommand(0x00); // 0x00~0x7F

    SSD1306_WriteCommand(0x40); // 设置显示开始行，0x40~0x7F

    SSD1306_WriteCommand(0xA1); // 设置左右方向，0xA1正常，0xA0左右反置

    SSD1306_WriteCommand(0xC8); // 设置上下方向，0xC8正常，0xC0上下反置

    SSD1306_WriteCommand(0xDA); // 设置COM引脚硬件配置
    SSD1306_WriteCommand(0x12);

    SSD1306_WriteCommand(0x81); // 设置对比度
    SSD1306_WriteCommand(0xCF); // 0x00~0xFF

    SSD1306_WriteCommand(0xD9); // 设置预充电周期
    SSD1306_WriteCommand(0xF1);

    SSD1306_WriteCommand(0xDB); // 设置VCOMH取消选择级别
    SSD1306_WriteCommand(0x30);

    SSD1306_WriteCommand(0xA4); // 设置整个显示打开/关闭

    SSD1306_WriteCommand(0xA6); // 设置正常/反色显示，0xA6正常，0xA7反色

    SSD1306_WriteCommand(0x8D); // 设置充电泵
    SSD1306_WriteCommand(0x14);

    SSD1306_WriteCommand(0xAF); // 开启显示

    SSD1306_Clear();  // 清空显存数组
    SSD1306_Update(); // 更新显示
    return 0;
}

/**
 * @brief 检测SSD1306设备是否存在
 * @return 0-设备存在，非0-设备不存在
 */
uint8_t SSD1306_CheckDevice(void)
{
    static uint8_t initialized = 0;
    if (!initialized)
    {
        if (SSD1306_Init())
        {
            return 1; // 设备不存在
        }
        initialized = 1;
    }
    else
    {
        if (SSD1306_Device_AckCheck())
        {
            initialized = 0;
            return 1; // 设备不存在
        }
    }
    return 0; // 设备存在
}

/*============================ 光标设置 ============================*/

/**
 * @brief SSD1306设置显示光标位置
 * @param Page 指定光标所在的页，范围：0~7
 * @param X 指定光标所在的X轴坐标，范围：0~127
 */
static void SSD1306_SetCursor(uint8_t Page, uint8_t X)
{
    SSD1306_WriteCommand(0xB0 | Page);              // 设置页位置
    SSD1306_WriteCommand(0x10 | ((X & 0xF0) >> 4)); // 设置X位置高4位
    SSD1306_WriteCommand(0x00 | (X & 0x0F));        // 设置X位置低4位
}

/*============================ 更新函数 ============================*/

/**
 * @brief 将SSD1306显存数组更新到SSD1306屏幕
 */
void SSD1306_Update(void)
{
    uint8_t j;
    for (j = 0; j < 8; j++)
    {
        SSD1306_SetCursor(j, 0);
        SSD1306_WriteData(SSD1306_DisplayBuf[j], 128);
    }
}

/**
 * @brief 将SSD1306显存数组部分更新到SSD1306屏幕
 */
void SSD1306_UpdateArea(int16_t X, int16_t Y, uint8_t Width, uint8_t Height)
{
    int16_t j;
    int16_t Page, Page1;

    Page = Y / 8;
    Page1 = (Y + Height - 1) / 8 + 1;
    if (Y < 0)
    {
        Page -= 1;
        Page1 -= 1;
    }

    for (j = Page; j < Page1; j++)
    {
        if (X >= 0 && X <= 127 && j >= 0 && j <= 7)
        {
            SSD1306_SetCursor(j, X);
            SSD1306_WriteData(&SSD1306_DisplayBuf[j][X], Width);
        }
    }
}

/*============================ 显存控制函数 ============================*/

/**
 * @brief 将SSD1306显存数组全部清零
 */
void SSD1306_Clear(void)
{
    uint8_t i, j;
    for (j = 0; j < 8; j++)
    {
        for (i = 0; i < 128; i++)
        {
            SSD1306_DisplayBuf[j][i] = 0x00;
        }
    }
}

/**
 * @brief 将SSD1306显存数组部分清零
 */
void SSD1306_ClearArea(int16_t X, int16_t Y, uint8_t Width, uint8_t Height)
{
    int16_t i, j;

    for (j = Y; j < Y + Height; j++)
    {
        for (i = X; i < X + Width; i++)
        {
            if (i >= 0 && i <= 127 && j >= 0 && j <= 63)
            {
                SSD1306_DisplayBuf[j / 8][i] &= ~(0x01 << (j % 8));
            }
        }
    }
}

/**
 * @brief 将SSD1306显存数组全部取反
 */
void SSD1306_Reverse(void)
{
    uint8_t i, j;
    for (j = 0; j < 8; j++)
    {
        for (i = 0; i < 128; i++)
        {
            SSD1306_DisplayBuf[j][i] ^= 0xFF;
        }
    }
}

/**
 * @brief 将SSD1306显存数组部分取反
 */
void SSD1306_ReverseArea(int16_t X, int16_t Y, uint8_t Width, uint8_t Height)
{
    int16_t i, j;

    for (j = Y; j < Y + Height; j++)
    {
        for (i = X; i < X + Width; i++)
        {
            if (i >= 0 && i <= 127 && j >= 0 && j <= 63)
            {
                SSD1306_DisplayBuf[j / 8][i] ^= 0x01 << (j % 8);
            }
        }
    }
}

/*============================ 绘图函数 ============================*/

/**
 * @brief SSD1306在指定位置画一个点
 */
void SSD1306_DrawPoint(int16_t X, int16_t Y)
{
    if (X >= 0 && X <= 127 && Y >= 0 && Y <= 63)
    {
        SSD1306_DisplayBuf[Y / 8][X] |= 0x01 << (Y % 8);
    }
}

/**
 * @brief SSD1306获取指定位置点的值
 */
uint32_t SSD1306_GetPoint(uint16_t X, uint16_t Y)
{
    if (X <= 127 && Y <= 63)
    {
        if (SSD1306_DisplayBuf[Y / 8][X] & (0x01 << (Y % 8)))
        {
            return 1;
        }
    }
    return 0;
}

/**
 * @brief 设置像素点（兼容统一绘图接口）
 */
void SSD1306_SetPixel(uint16_t x, uint16_t y, uint32_t color)
{
    if (color)
        SSD1306_DrawPoint(x, y);
    else
        SSD1306_ClearArea(x, y, 1, 1);
}

/**
 * @brief 填充矩形区域（兼容统一绘图接口）
 */
void SSD1306_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t color)
{
    (void)color;
    if (x == 0 && y == 0 && w >= SSD1306_WIDTH && h >= SSD1306_HEIGHT)
    {
        SSD1306_Clear();
        return;
    }
    SSD1306_ClearArea(x, y, w, h);
}

/**
 * @brief SSD1306显示图像
 */
void SSD1306_ShowImage(int16_t X, int16_t Y, uint8_t Width, uint8_t Height, const uint8_t *Image)
{
    uint8_t i = 0, j = 0;
    int16_t Page, Shift;

    SSD1306_ClearArea(X, Y, Width, Height);

    for (j = 0; j < (Height - 1) / 8 + 1; j++)
    {
        for (i = 0; i < Width; i++)
        {
            if (X + i >= 0 && X + i <= 127)
            {
                Page = Y / 8;
                Shift = Y % 8;
                if (Y < 0)
                {
                    Page -= 1;
                    Shift += 8;
                }

                if (Page + j >= 0 && Page + j <= 7)
                {
                    SSD1306_DisplayBuf[Page + j][X + i] |= Image[j * Width + i] << (Shift);
                }

                if (Page + j + 1 >= 0 && Page + j + 1 <= 7)
                {
                    SSD1306_DisplayBuf[Page + j + 1][X + i] |= Image[j * Width + i] >> (8 - Shift);
                }
            }
        }
    }
}

#endif /* USE_DEVICE_SSD1306 */
