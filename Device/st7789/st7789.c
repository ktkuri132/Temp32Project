#include "st7789.h"
#include <stddef.h>

static ST7789_IO_t *st7789_io = NULL;

// 注册底层IO接口
void ST7789_Register_IO(ST7789_IO_t *io_ptr)
{
    st7789_io = io_ptr;
}

// 内部函数：写命令
static void ST7789_Write_Cmd(uint8_t cmd)
{
    if (st7789_io == NULL)
        return;

    if (st7789_io->DC_Control)
        st7789_io->DC_Control(0); // Command
    if (st7789_io->CS_Control)
        st7789_io->CS_Control(0); // Select
    if (st7789_io->Transmit)
        st7789_io->Transmit(&cmd, 1);
    if (st7789_io->CS_Control)
        st7789_io->CS_Control(1); // Deselect
}

// 内部函数：写数据 (8位)
static void ST7789_Write_Data(uint8_t data)
{
    if (st7789_io == NULL)
        return;

    if (st7789_io->DC_Control)
        st7789_io->DC_Control(1); // Data
    if (st7789_io->CS_Control)
        st7789_io->CS_Control(0); // Select
    if (st7789_io->Transmit)
        st7789_io->Transmit(&data, 1);
    if (st7789_io->CS_Control)
        st7789_io->CS_Control(1); // Deselect
}

// 内部函数：写数据 (16位)
static void ST7789_Write_Data16(uint16_t data)
{
    if (st7789_io == NULL)
        return;

    uint8_t buf[2];
    buf[0] = data >> 8;
    buf[1] = data & 0xFF;

    if (st7789_io->DC_Control)
        st7789_io->DC_Control(1); // Data
    if (st7789_io->CS_Control)
        st7789_io->CS_Control(0); // Select
    if (st7789_io->Transmit)
        st7789_io->Transmit(buf, 2);
    if (st7789_io->CS_Control)
        st7789_io->CS_Control(1); // Deselect
}

void ST7789_Init(void)
{
    if (st7789_io == NULL)
        return;

    // 复位
    if (st7789_io->RES_Control)
    {
        st7789_io->RES_Control(0);
        if (st7789_io->DelayMs)
            st7789_io->DelayMs(100);
        st7789_io->RES_Control(1);
        if (st7789_io->DelayMs)
            st7789_io->DelayMs(100);
    }

    ST7789_Write_Cmd(0x11); // Sleep Out
    if (st7789_io->DelayMs)
        st7789_io->DelayMs(120);

    ST7789_Write_Cmd(0x36);  // Memory Data Access Control
    ST7789_Write_Data(0x00); // RGB

    ST7789_Write_Cmd(0x3A);  // Interface Pixel Format
    ST7789_Write_Data(0x05); // 16bit/pixel

    ST7789_Write_Cmd(0xB2); // Porch Setting
    ST7789_Write_Data(0x0C);
    ST7789_Write_Data(0x0C);
    ST7789_Write_Data(0x00);
    ST7789_Write_Data(0x33);
    ST7789_Write_Data(0x33);

    ST7789_Write_Cmd(0xB7); // Gate Control
    ST7789_Write_Data(0x35);

    ST7789_Write_Cmd(0xBB); // VCOM Setting
    ST7789_Write_Data(0x19);

    ST7789_Write_Cmd(0xC0); // LCM Control
    ST7789_Write_Data(0x2C);

    ST7789_Write_Cmd(0xC2); // VDV and VRH Command Enable
    ST7789_Write_Data(0x01);

    ST7789_Write_Cmd(0xC3); // VRH Set
    ST7789_Write_Data(0x12);

    ST7789_Write_Cmd(0xC4); // VDV Set
    ST7789_Write_Data(0x20);

    ST7789_Write_Cmd(0xC6); // Frame Rate Control
    ST7789_Write_Data(0x0F);

    ST7789_Write_Cmd(0xD0); // Power Control 1
    ST7789_Write_Data(0xA4);
    ST7789_Write_Data(0xA1);

    ST7789_Write_Cmd(0xE0); // Positive Voltage Gamma Control
    ST7789_Write_Data(0xD0);
    ST7789_Write_Data(0x04);
    ST7789_Write_Data(0x0D);
    ST7789_Write_Data(0x11);
    ST7789_Write_Data(0x13);
    ST7789_Write_Data(0x2B);
    ST7789_Write_Data(0x3F);
    ST7789_Write_Data(0x54);
    ST7789_Write_Data(0x4C);
    ST7789_Write_Data(0x18);
    ST7789_Write_Data(0x0D);
    ST7789_Write_Data(0x0B);
    ST7789_Write_Data(0x1F);
    ST7789_Write_Data(0x23);

    ST7789_Write_Cmd(0xE1); // Negative Voltage Gamma Control
    ST7789_Write_Data(0xD0);
    ST7789_Write_Data(0x04);
    ST7789_Write_Data(0x0C);
    ST7789_Write_Data(0x11);
    ST7789_Write_Data(0x13);
    ST7789_Write_Data(0x2C);
    ST7789_Write_Data(0x3F);
    ST7789_Write_Data(0x44);
    ST7789_Write_Data(0x51);
    ST7789_Write_Data(0x2F);
    ST7789_Write_Data(0x1F);
    ST7789_Write_Data(0x1F);
    ST7789_Write_Data(0x20);
    ST7789_Write_Data(0x23);

    ST7789_Write_Cmd(0x21); // Display Inversion On

    ST7789_Write_Cmd(0x29); // Display On

    if (st7789_io->BLK_Control)
        st7789_io->BLK_Control(1); // Turn on backlight

    ST7789_Clear_All(ST7789_BLACK);
}

static void ST7789_SetWindow(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    ST7789_Write_Cmd(0x2A); // Column Address Set
    ST7789_Write_Data(x1 >> 8);
    ST7789_Write_Data(x1 & 0xFF);
    ST7789_Write_Data(x2 >> 8);
    ST7789_Write_Data(x2 & 0xFF);

    ST7789_Write_Cmd(0x2B); // Row Address Set
    ST7789_Write_Data(y1 >> 8);
    ST7789_Write_Data(y1 & 0xFF);
    ST7789_Write_Data(y2 >> 8);
    ST7789_Write_Data(y2 & 0xFF);

    ST7789_Write_Cmd(0x2C); // Memory Write
}

void ST7789_Clear_All(uint16_t color)
{
    ST7789_Fill_Rect(0, 0, ST7789_WIDTH, ST7789_HEIGHT, color);
}

void ST7789_Fill_Rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    if (x >= ST7789_WIDTH || y >= ST7789_HEIGHT)
        return;
    if ((x + w) > ST7789_WIDTH)
        w = ST7789_WIDTH - x;
    if ((y + h) > ST7789_HEIGHT)
        h = ST7789_HEIGHT - y;

    ST7789_SetWindow(x, y, x + w - 1, y + h - 1);

    if (st7789_io == NULL)
        return;

    // 准备数据
    uint8_t buf[2] = {color >> 8, color & 0xFF};

    if (st7789_io->DC_Control)
        st7789_io->DC_Control(1); // Data
    if (st7789_io->CS_Control)
        st7789_io->CS_Control(0); // Select

    // 批量发送
    // 注意：这里为了简单，逐个像素发送。优化时应使用大缓冲区或DMA
    for (uint32_t i = 0; i < (uint32_t)w * h; i++)
    {
        if (st7789_io->Transmit)
            st7789_io->Transmit(buf, 2);
    }

    if (st7789_io->CS_Control)
        st7789_io->CS_Control(1); // Deselect
}

void ST7789_SetPixel(uint16_t x, uint16_t y, uint16_t color)
{
    if (x >= ST7789_WIDTH || y >= ST7789_HEIGHT)
        return;

    ST7789_SetWindow(x, y, x, y);
    ST7789_Write_Data16(color);
}

void ST7789_DrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t *data)
{
    if (x >= ST7789_WIDTH || y >= ST7789_HEIGHT)
        return;
    if ((x + w) > ST7789_WIDTH)
        w = ST7789_WIDTH - x;
    if ((y + h) > ST7789_HEIGHT)
        h = ST7789_HEIGHT - y;

    ST7789_SetWindow(x, y, x + w - 1, y + h - 1);

    if (st7789_io == NULL)
        return;

    if (st7789_io->DC_Control)
        st7789_io->DC_Control(1); // Data
    if (st7789_io->CS_Control)
        st7789_io->CS_Control(0); // Select

    // 批量发送
    // 假设 data 是连续的 RGB565 数据
    // 注意字节序，这里假设 data 已经是大端模式或者 MCU 是小端模式需要转换
    // 通常图片取模软件生成的数组是高位在前
    if (st7789_io->Transmit)
        st7789_io->Transmit((uint8_t *)data, w * h * 2);

    if (st7789_io->CS_Control)
        st7789_io->CS_Control(1); // Deselect
}
