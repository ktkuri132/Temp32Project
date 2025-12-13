#include "ssd1306.h"
#include <string.h>

// 显存缓冲区
// 格式: 8页 x 128列
static uint8_t SSD1306_GRAM[SSD1306_PAGES][SSD1306_WIDTH];

static SSD1306_IO_t *ssd1306_io = NULL;

// 注册底层IO接口
void SSD1306_Register_IO(SSD1306_IO_t *io_ptr)
{
    ssd1306_io = io_ptr;
}

// 内部函数：写命令
static void SSD1306_Write_Cmd(uint8_t cmd)
{
    if (ssd1306_io == NULL)
        return;

#if SSD1306_MODE_I2C
    if (ssd1306_io->Transmit)
    {
        uint8_t data[2] = {0x00, cmd}; // 0x00: Co=0, D/C#=0 (Command)
        ssd1306_io->Transmit(SSD1306_I2C_ADDR, data, 2);
    }
#elif SSD1306_MODE_SPI
    if (ssd1306_io->DC_Control)
        ssd1306_io->DC_Control(0); // Command
    if (ssd1306_io->CS_Control)
        ssd1306_io->CS_Control(0); // Select
    if (ssd1306_io->Transmit)
        ssd1306_io->Transmit(&cmd, 1);
    if (ssd1306_io->CS_Control)
        ssd1306_io->CS_Control(1); // Deselect
#endif
}

// 内部函数：写数据
static void SSD1306_Write_Data(uint8_t data)
{
    if (ssd1306_io == NULL)
        return;

#if SSD1306_MODE_I2C
    if (ssd1306_io->Transmit)
    {
        uint8_t buf[2] = {0x40, data}; // 0x40: Co=0, D/C#=1 (Data)
        ssd1306_io->Transmit(SSD1306_I2C_ADDR, buf, 2);
    }
#elif SSD1306_MODE_SPI
    if (ssd1306_io->DC_Control)
        ssd1306_io->DC_Control(1); // Data
    if (ssd1306_io->CS_Control)
        ssd1306_io->CS_Control(0); // Select
    if (ssd1306_io->Transmit)
        ssd1306_io->Transmit(&data, 1);
    if (ssd1306_io->CS_Control)
        ssd1306_io->CS_Control(1); // Deselect
#endif
}

void SSD1306_Init(void)
{
    if (ssd1306_io == NULL)
        return;

#if SSD1306_MODE_SPI
    // SPI 模式下复位序列
    if (ssd1306_io->RES_Control)
    {
        ssd1306_io->RES_Control(0); // Reset
        if (ssd1306_io->DelayMs)
            ssd1306_io->DelayMs(10);
        ssd1306_io->RES_Control(1); // Work
        if (ssd1306_io->DelayMs)
            ssd1306_io->DelayMs(10);
    }
#endif

    // 初始化序列
    SSD1306_Write_Cmd(0xAE); // Display OFF

    SSD1306_Write_Cmd(0x20); // Set Memory Addressing Mode
    SSD1306_Write_Cmd(0x02); // 00:Horizontal, 01:Vertical, 02:Page

    SSD1306_Write_Cmd(0xB0); // Set Page Start Address for Page Addressing Mode

    SSD1306_Write_Cmd(0xC8); // Set COM Output Scan Direction

    SSD1306_Write_Cmd(0x00); // Set Low Column Address
    SSD1306_Write_Cmd(0x10); // Set High Column Address

    SSD1306_Write_Cmd(0x40); // Set Start Line Address

    SSD1306_Write_Cmd(0x81); // Set Contrast Control
    SSD1306_Write_Cmd(0xFF);

    SSD1306_Write_Cmd(0xA1); // Set Segment Re-map

    SSD1306_Write_Cmd(0xA6); // Set Normal/Inverse Display

    SSD1306_Write_Cmd(0xA8); // Set Multiplex Ratio
    SSD1306_Write_Cmd(0x3F);

    SSD1306_Write_Cmd(0xA4); // Entire Display ON (Resume)

    SSD1306_Write_Cmd(0xD3); // Set Display Offset
    SSD1306_Write_Cmd(0x00);

    SSD1306_Write_Cmd(0xD5); // Set Display Clock Divide Ratio/Oscillator Frequency
    SSD1306_Write_Cmd(0xF0);

    SSD1306_Write_Cmd(0xD9); // Set Pre-charge Period
    SSD1306_Write_Cmd(0x22);

    SSD1306_Write_Cmd(0xDA); // Set COM Pins Hardware Configuration
    SSD1306_Write_Cmd(0x12);

    SSD1306_Write_Cmd(0xDB); // Set VCOMH Deselect Level
    SSD1306_Write_Cmd(0x20);

    SSD1306_Write_Cmd(0x8D); // Charge Pump Setting
    SSD1306_Write_Cmd(0x14); // Enable Charge Pump

    SSD1306_Write_Cmd(0xAF); // Display ON

    SSD1306_Clear_All();
}

void SSD1306_Clear_All(void)
{
    memset(SSD1306_GRAM, 0x00, sizeof(SSD1306_GRAM));
    SSD1306_Update();
}

void SSD1306_Clear_Part(uint8_t page, uint8_t start_col, uint8_t end_col)
{
    if (page >= SSD1306_PAGES || start_col >= SSD1306_WIDTH || end_col >= SSD1306_WIDTH)
        return;

    for (uint8_t i = start_col; i <= end_col; i++)
    {
        SSD1306_GRAM[page][i] = 0x00;
    }
}

void SSD1306_Update(void)
{
    uint8_t i, n;
    for (i = 0; i < SSD1306_PAGES; i++)
    {
        SSD1306_Write_Cmd(0xB0 + i); // Set Page Address
        SSD1306_Write_Cmd(0x00);     // Set Low Column Address
        SSD1306_Write_Cmd(0x10);     // Set High Column Address

        for (n = 0; n < SSD1306_WIDTH; n++)
        {
            SSD1306_Write_Data(SSD1306_GRAM[i][n]);
        }
    }
}

void SSD1306_SetPixel(uint8_t x, uint8_t y, uint8_t color)
{
    if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT)
        return;

    if (color)
        SSD1306_GRAM[y / 8][x] |= (1 << (y % 8));
    else
        SSD1306_GRAM[y / 8][x] &= ~(1 << (y % 8));
}
