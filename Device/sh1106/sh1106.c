#include "sh1106.h"
#include <string.h>

// 显存缓冲区
// 格式: 8页 x 128列
static uint8_t SH1106_GRAM[SH1106_PAGES][SH1106_WIDTH];

// 列地址偏移量，SH1106驱动128宽屏幕时通常为2
#define SH1106_COL_OFFSET 2

static SH1106_IO_t *sh1106_io = NULL;

// 注册底层IO接口
void SH1106_Register_IO(SH1106_IO_t *io_ptr)
{
    sh1106_io = io_ptr;
}

// 内部函数：写命令
static void SH1106_Write_Cmd(uint8_t cmd)
{
    if (sh1106_io == NULL)
        return;

#if SH1106_MODE_I2C
    if (sh1106_io->Transmit)
    {
        uint8_t data[2] = {0x00, cmd}; // 0x00: Co=0, D/C#=0 (Command)
        sh1106_io->Transmit(SH1106_I2C_ADDR, data, 2);
    }
#elif SH1106_MODE_SPI
    if (sh1106_io->DC_Control)
        sh1106_io->DC_Control(0); // Command
    if (sh1106_io->CS_Control)
        sh1106_io->CS_Control(0); // Select
    if (sh1106_io->Transmit)
        sh1106_io->Transmit(&cmd, 1);
    if (sh1106_io->CS_Control)
        sh1106_io->CS_Control(1); // Deselect
#endif
}

// 内部函数：写数据
static void SH1106_Write_Data(uint8_t data)
{
    if (sh1106_io == NULL)
        return;

#if SH1106_MODE_I2C
    if (sh1106_io->Transmit)
    {
        uint8_t buf[2] = {0x40, data}; // 0x40: Co=0, D/C#=1 (Data)
        sh1106_io->Transmit(SH1106_I2C_ADDR, buf, 2);
    }
#elif SH1106_MODE_SPI
    if (sh1106_io->DC_Control)
        sh1106_io->DC_Control(1); // Data
    if (sh1106_io->CS_Control)
        sh1106_io->CS_Control(0); // Select
    if (sh1106_io->Transmit)
        sh1106_io->Transmit(&data, 1);
    if (sh1106_io->CS_Control)
        sh1106_io->CS_Control(1); // Deselect
#endif
}

void SH1106_Init(void)
{
    if (sh1106_io == NULL)
        return;

#if SH1106_MODE_SPI
    // SPI 模式下复位序列
    if (sh1106_io->RES_Control)
    {
        sh1106_io->RES_Control(0); // Reset
        if (sh1106_io->DelayMs)
            sh1106_io->DelayMs(10);
        sh1106_io->RES_Control(1); // Work
        if (sh1106_io->DelayMs)
            sh1106_io->DelayMs(10);
    }
#endif

    // 初始化序列
    SH1106_Write_Cmd(0xAE); // Display OFF

    SH1106_Write_Cmd(0x00); // Set Low Column Address
    SH1106_Write_Cmd(0x10); // Set High Column Address
    SH1106_Write_Cmd(0x40); // Set Display Start Line (0)
    SH1106_Write_Cmd(0xB0); // Set Page Address

    SH1106_Write_Cmd(0x81); // Contrast Control
    SH1106_Write_Cmd(0xCF); // 亮度值 (0x00~0xFF)
    SH1106_Write_Cmd(0xA1); // Segment Re-map (A0: Normal, A1: Remap/Flip H)
    SH1106_Write_Cmd(0xC8); // Common Output Scan Direction (C0: Normal, C8: Remap/Flip V)

    SH1106_Write_Cmd(0xA6); // Normal Display (A7: Inverse)

    SH1106_Write_Cmd(0xA8); // Multiplex Ratio
    SH1106_Write_Cmd(0x3F); // 1/64 Duty

    SH1106_Write_Cmd(0xD3); // Display Offset
    SH1106_Write_Cmd(0x00);

    SH1106_Write_Cmd(0xD5); // Display Clock Divide Ratio
    SH1106_Write_Cmd(0x80);

    SH1106_Write_Cmd(0xD9); // Dis-charge/Pre-charge Period
    SH1106_Write_Cmd(0xF1);

    SH1106_Write_Cmd(0xDA); // Common Pads Hardware Configuration
    SH1106_Write_Cmd(0x12);

    SH1106_Write_Cmd(0xDB); // VCOM Deselect Level
    SH1106_Write_Cmd(0x40);

    // 开启内置DC-DC (SH1106特有，部分模块可能不需要)
    SH1106_Write_Cmd(0xAD);
    SH1106_Write_Cmd(0x8B);
    SH1106_Write_Cmd(0xAF); // Display ON

    SH1106_Clear_All();
}

void SH1106_Clear_All(void)
{
    memset(SH1106_GRAM, 0x00, sizeof(SH1106_GRAM));
    SH1106_Update();
}

void SH1106_Clear_Part(uint8_t page, uint8_t start_col, uint8_t end_col)
{
    if (page >= SH1106_PAGES || start_col >= SH1106_WIDTH || end_col >= SH1106_WIDTH)
        return;

    for (uint8_t i = start_col; i <= end_col; i++)
    {
        SH1106_GRAM[page][i] = 0x00;
    }
}

void SH1106_Update(void)
{
    uint8_t i, n;
    for (i = 0; i < SH1106_PAGES; i++)
    {
        SH1106_Write_Cmd(0xB0 + i); // Set Page Address
        // 设置列地址，加上偏移量
        SH1106_Write_Cmd(0x00 + (SH1106_COL_OFFSET & 0x0F));        // Set Low Column Address
        SH1106_Write_Cmd(0x10 + ((SH1106_COL_OFFSET >> 4) & 0x0F)); // Set High Column Address

        for (n = 0; n < SH1106_WIDTH; n++)
        {
            SH1106_Write_Data(SH1106_GRAM[i][n]);
        }
    }
}

void SH1106_SetPixel(uint8_t x, uint8_t y, uint8_t color)
{
    if (x >= SH1106_WIDTH || y >= SH1106_HEIGHT)
        return;

    if (color)
        SH1106_GRAM[y / 8][x] |= (1 << (y % 8));
    else
        SH1106_GRAM[y / 8][x] &= ~(1 << (y % 8));
}
