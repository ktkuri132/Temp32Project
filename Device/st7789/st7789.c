/**
 * @file st7789.c
 * @brief ST7789 LCD显示屏驱动实现
 * @note 使用 device_hal.h 统一 HAL 接口，支持 SPI 通信方式
 */

#include "st7789.h"

#ifdef USE_DEVICE_ST7789

/*============================ HAL接口实例 ============================*/
device_spi_hal_t *st7789_spi_hal = NULL;
static st7789_gpio_t *st7789_gpio = NULL;

/* 默认GPIO控制（无操作） */
static st7789_gpio_t st7789_gpio_default = {
    .dc_control = NULL,
    .res_control = NULL,
    .blk_control = NULL};

/*============================ 底层通信函数 ============================*/

/**
 * @brief DC引脚控制
 */
static void ST7789_DC(bool level)
{
    if (st7789_gpio && st7789_gpio->dc_control)
    {
        st7789_gpio->dc_control(level);
    }
}

/**
 * @brief RES引脚控制
 */
static void ST7789_RES(bool level)
{
    if (st7789_gpio && st7789_gpio->res_control)
    {
        st7789_gpio->res_control(level);
    }
}

/**
 * @brief 延时函数
 */
static void ST7789_DelayMs(uint32_t ms)
{
    if (st7789_spi_hal && st7789_spi_hal->delay_ms)
    {
        st7789_spi_hal->delay_ms(ms);
    }
    else
    {
        /* 简单忙等待延时 */
        volatile uint32_t count = ms * 10000;
        while (count--)
            ;
    }
}

/**
 * @brief 写入命令到ST7789
 */
static int ST7789_WriteCommand(uint8_t command)
{
    if (!st7789_spi_hal || !st7789_spi_hal->initialized)
        return -1;

    ST7789_DC(false); /* DC=0 表示命令 */
    st7789_spi_hal->cs_control(true);
    st7789_spi_hal->transfer_byte(command);
    st7789_spi_hal->cs_control(false);
    return 0;
}

/**
 * @brief 写入单字节数据到ST7789
 */
static int ST7789_WriteData(uint8_t data)
{
    if (!st7789_spi_hal || !st7789_spi_hal->initialized)
        return -1;

    ST7789_DC(true); /* DC=1 表示数据 */
    st7789_spi_hal->cs_control(true);
    st7789_spi_hal->transfer_byte(data);
    st7789_spi_hal->cs_control(false);
    return 0;
}

/**
 * @brief 写入16位数据到ST7789
 */
static int ST7789_WriteData16(uint16_t data)
{
    if (!st7789_spi_hal || !st7789_spi_hal->initialized)
        return -1;

    uint8_t buf[2] = {data >> 8, data & 0xFF};

    ST7789_DC(true); /* DC=1 表示数据 */
    st7789_spi_hal->cs_control(true);
    st7789_spi_hal->transfer_bytes(buf, NULL, 2);
    st7789_spi_hal->cs_control(false);
    return 0;
}

/**
 * @brief 批量写入数据到ST7789
 */
static int ST7789_WriteDataBulk(const uint8_t *data, uint32_t len)
{
    if (!st7789_spi_hal || !st7789_spi_hal->initialized)
        return -1;

    ST7789_DC(true); /* DC=1 表示数据 */
    st7789_spi_hal->cs_control(true);
    st7789_spi_hal->transfer_bytes(data, NULL, len);
    st7789_spi_hal->cs_control(false);
    return 0;
}

/*============================ HAL初始化函数 ============================*/

/**
 * @brief 初始化ST7789并绑定SPI HAL接口
 * @param hal SPI HAL接口指针
 * @param gpio GPIO控制结构体指针
 * @return 0-成功，非0-失败
 */
int ST7789_Init_HAL_SPI(device_spi_hal_t *hal, st7789_gpio_t *gpio)
{
    if (!hal || !hal->initialized)
        return -1;

    st7789_spi_hal = hal;
    st7789_gpio = gpio ? gpio : &st7789_gpio_default;
    return 0;
}

/**
 * @brief 兼容旧接口
 */
int ST7789_Init_HAL(device_spi_hal_t *hal)
{
    return ST7789_Init_HAL_SPI(hal, NULL);
}

/**
 * @brief 初始化ST7789
 * @return 0-成功，非0-失败
 */
uint8_t ST7789_Init(void)
{
    if (!st7789_spi_hal || !st7789_spi_hal->initialized)
        return 1;

    /* 硬件复位 */
    ST7789_RES(false);
    ST7789_DelayMs(100);
    ST7789_RES(true);
    ST7789_DelayMs(100);

    /* Sleep Out */
    ST7789_WriteCommand(0x11);
    ST7789_DelayMs(120);

    /* Memory Data Access Control */
    ST7789_WriteCommand(0x36);
    ST7789_WriteData(0x00); /* RGB */

    /* Interface Pixel Format */
    ST7789_WriteCommand(0x3A);
    ST7789_WriteData(0x05); /* 16bit/pixel */

    /* Porch Setting */
    ST7789_WriteCommand(0xB2);
    ST7789_WriteData(0x0C);
    ST7789_WriteData(0x0C);
    ST7789_WriteData(0x00);
    ST7789_WriteData(0x33);
    ST7789_WriteData(0x33);

    /* Gate Control */
    ST7789_WriteCommand(0xB7);
    ST7789_WriteData(0x35);

    /* VCOM Setting */
    ST7789_WriteCommand(0xBB);
    ST7789_WriteData(0x19);

    /* LCM Control */
    ST7789_WriteCommand(0xC0);
    ST7789_WriteData(0x2C);

    /* VDV and VRH Command Enable */
    ST7789_WriteCommand(0xC2);
    ST7789_WriteData(0x01);

    /* VRH Set */
    ST7789_WriteCommand(0xC3);
    ST7789_WriteData(0x12);

    /* VDV Set */
    ST7789_WriteCommand(0xC4);
    ST7789_WriteData(0x20);

    /* Frame Rate Control */
    ST7789_WriteCommand(0xC6);
    ST7789_WriteData(0x0F);

    /* Power Control 1 */
    ST7789_WriteCommand(0xD0);
    ST7789_WriteData(0xA4);
    ST7789_WriteData(0xA1);

    /* Positive Voltage Gamma Control */
    ST7789_WriteCommand(0xE0);
    ST7789_WriteData(0xD0);
    ST7789_WriteData(0x04);
    ST7789_WriteData(0x0D);
    ST7789_WriteData(0x11);
    ST7789_WriteData(0x13);
    ST7789_WriteData(0x2B);
    ST7789_WriteData(0x3F);
    ST7789_WriteData(0x54);
    ST7789_WriteData(0x4C);
    ST7789_WriteData(0x18);
    ST7789_WriteData(0x0D);
    ST7789_WriteData(0x0B);
    ST7789_WriteData(0x1F);
    ST7789_WriteData(0x23);

    /* Negative Voltage Gamma Control */
    ST7789_WriteCommand(0xE1);
    ST7789_WriteData(0xD0);
    ST7789_WriteData(0x04);
    ST7789_WriteData(0x0C);
    ST7789_WriteData(0x11);
    ST7789_WriteData(0x13);
    ST7789_WriteData(0x2C);
    ST7789_WriteData(0x3F);
    ST7789_WriteData(0x44);
    ST7789_WriteData(0x51);
    ST7789_WriteData(0x2F);
    ST7789_WriteData(0x1F);
    ST7789_WriteData(0x1F);
    ST7789_WriteData(0x20);
    ST7789_WriteData(0x23);

    /* Display Inversion On */
    ST7789_WriteCommand(0x21);

    /* Display On */
    ST7789_WriteCommand(0x29);

    /* 开启背光 */
    ST7789_Backlight(true);

    /* 清屏 */
    ST7789_Clear(ST7789_BLACK);

    return 0;
}

/**
 * @brief 检测ST7789设备是否存在
 * @return 0-设备存在，非0-设备不存在
 */
uint8_t ST7789_CheckDevice(void)
{
    if (!st7789_spi_hal || !st7789_spi_hal->initialized)
        return 1;

    /* ST7789 无法通过SPI读取ID，只能假设存在 */
    return 0;
}

/*============================ 显示控制函数 ============================*/

/**
 * @brief 设置显示窗口
 */
void ST7789_SetWindow(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    /* Column Address Set */
    ST7789_WriteCommand(0x2A);
    ST7789_WriteData(x1 >> 8);
    ST7789_WriteData(x1 & 0xFF);
    ST7789_WriteData(x2 >> 8);
    ST7789_WriteData(x2 & 0xFF);

    /* Row Address Set */
    ST7789_WriteCommand(0x2B);
    ST7789_WriteData(y1 >> 8);
    ST7789_WriteData(y1 & 0xFF);
    ST7789_WriteData(y2 >> 8);
    ST7789_WriteData(y2 & 0xFF);

    /* Memory Write */
    ST7789_WriteCommand(0x2C);
}

/**
 * @brief 全屏填充颜色
 */
void ST7789_Clear(uint16_t color)
{
    ST7789_FillRect(0, 0, ST7789_WIDTH, ST7789_HEIGHT, color);
}

/**
 * @brief 填充矩形区域
 */
void ST7789_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    if (x >= ST7789_WIDTH || y >= ST7789_HEIGHT)
        return;
    if ((x + w) > ST7789_WIDTH)
        w = ST7789_WIDTH - x;
    if ((y + h) > ST7789_HEIGHT)
        h = ST7789_HEIGHT - y;

    ST7789_SetWindow(x, y, x + w - 1, y + h - 1);

    if (!st7789_spi_hal || !st7789_spi_hal->initialized)
        return;

    uint8_t buf[2] = {color >> 8, color & 0xFF};
    uint32_t total = (uint32_t)w * h;

    ST7789_DC(true);
    st7789_spi_hal->cs_control(true);

    // for (uint32_t i = 0; i < total; i++)
    // {
    //     st7789_spi_hal->transfer_bytes(buf, NULL, 2);
    // }
    st7789_spi_hal->transfer_bytes(buf, NULL, total * 2);

    st7789_spi_hal->cs_control(false);
}

/**
 * @brief 设置像素点
 */
void ST7789_SetPixel(uint16_t x, uint16_t y, uint16_t color)
{
    if (x >= ST7789_WIDTH || y >= ST7789_HEIGHT)
        return;

    ST7789_SetWindow(x, y, x, y);
    ST7789_WriteData16(color);
}

/**
 * @brief 显示图像
 */
void ST7789_DrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t *data)
{
    if (x >= ST7789_WIDTH || y >= ST7789_HEIGHT)
        return;
    if ((x + w) > ST7789_WIDTH)
        w = ST7789_WIDTH - x;
    if ((y + h) > ST7789_HEIGHT)
        h = ST7789_HEIGHT - y;

    ST7789_SetWindow(x, y, x + w - 1, y + h - 1);

    if (!st7789_spi_hal || !st7789_spi_hal->initialized)
        return;

    ST7789_DC(true);
    st7789_spi_hal->cs_control(true);
    st7789_spi_hal->transfer_bytes((const uint8_t *)data, NULL, w * h * 2);
    st7789_spi_hal->cs_control(false);
}

/**
 * @brief 设置显示方向
 * @param rotation 旋转方向 (0-3)
 */
void ST7789_SetRotation(uint8_t rotation)
{
    uint8_t madctl = 0;

    switch (rotation % 4)
    {
    case 0:
        madctl = 0x00; /* Portrait */
        break;
    case 1:
        madctl = 0x60; /* Landscape */
        break;
    case 2:
        madctl = 0xC0; /* Portrait inverted */
        break;
    case 3:
        madctl = 0xA0; /* Landscape inverted */
        break;
    }

    ST7789_WriteCommand(0x36);
    ST7789_WriteData(madctl);
}

/**
 * @brief 背光控制
 */
void ST7789_Backlight(bool on)
{
    if (st7789_gpio && st7789_gpio->blk_control)
    {
        st7789_gpio->blk_control(on);
    }
}

#endif /* USE_DEVICE_ST7789 */
