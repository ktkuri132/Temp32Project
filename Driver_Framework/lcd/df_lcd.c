#include "df_lcd.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

extern const uint8_t Font8x16_Table[]; // 字体数据表声明


/* ================= 实现 ================= */

void LCD_Handler_Init(LCD_Handler_t *lcd, uint16_t width, uint16_t height)
{
    if (!lcd)
        return;
    lcd->Width = width;
    lcd->Height = height;
    lcd->CursorX = 0;
    lcd->CursorY = 0;
    lcd->CurrentFont = &Font8x16; // 默认字体
    lcd->TextColor = LCD_COLOR_WHITE;
    lcd->BackColor = LCD_COLOR_BLACK;
    lcd->TerminalMode = false;
}

void LCD_Clear(LCD_Handler_t *lcd, uint32_t color)
{
    if (!lcd)
        return;
    if (lcd->FillRect)
    {
        lcd->FillRect(0, 0, lcd->Width, lcd->Height, color);
    }
    else if (lcd->SetPixel)
    {
        for (uint16_t y = 0; y < lcd->Height; y++)
        {
            for (uint16_t x = 0; x < lcd->Width; x++)
            {
                lcd->SetPixel(x, y, color);
            }
        }
    }
    if (lcd->Update)
        lcd->Update();
}

void LCD_DrawPoint(LCD_Handler_t *lcd, uint16_t x, uint16_t y, uint32_t color)
{
    if (lcd && lcd->SetPixel)
    {
        if (x < lcd->Width && y < lcd->Height)
        {
            lcd->SetPixel(x, y, color);
        }
    }
}

void LCD_DrawLine(LCD_Handler_t *lcd, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint32_t color)
{
    if (!lcd)
        return;
    int xerr = 0, yerr = 0, delta_x, delta_y, distance;
    int incx, incy, uRow, uCol;
    delta_x = x2 - x1;
    delta_y = y2 - y1;
    uRow = x1;
    uCol = y1;
    if (delta_x > 0)
        incx = 1;
    else if (delta_x == 0)
        incx = 0;
    else
    {
        incx = -1;
        delta_x = -delta_x;
    }
    if (delta_y > 0)
        incy = 1;
    else if (delta_y == 0)
        incy = 0;
    else
    {
        incy = -1;
        delta_y = -delta_y;
    }
    if (delta_x > delta_y)
        distance = delta_x;
    else
        distance = delta_y;
    for (int t = 0; t <= distance + 1; t++)
    {
        LCD_DrawPoint(lcd, uRow, uCol, color);
        xerr += delta_x;
        yerr += delta_y;
        if (xerr > distance)
        {
            xerr -= distance;
            uRow += incx;
        }
        if (yerr > distance)
        {
            yerr -= distance;
            uCol += incy;
        }
    }
}

void LCD_DrawRect(LCD_Handler_t *lcd, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t color)
{
    LCD_DrawLine(lcd, x, y, x + w - 1, y, color);
    LCD_DrawLine(lcd, x, y + h - 1, x + w - 1, y + h - 1, color);
    LCD_DrawLine(lcd, x, y, x, y + h - 1, color);
    LCD_DrawLine(lcd, x + w - 1, y, x + w - 1, y + h - 1, color);
}

void LCD_FillRect(LCD_Handler_t *lcd, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t color)
{
    if (!lcd)
        return;
    if (lcd->FillRect)
    {
        lcd->FillRect(x, y, w, h, color);
    }
    else
    {
        for (uint16_t i = 0; i < h; i++)
        {
            for (uint16_t j = 0; j < w; j++)
            {
                LCD_DrawPoint(lcd, x + j, y + i, color);
            }
        }
    }
}

void LCD_DrawCircle(LCD_Handler_t *lcd, uint16_t x0, uint16_t y0, uint16_t r, uint32_t color)
{
    int a = 0, b = r;
    int di = 3 - (r << 1);
    while (a <= b)
    {
        LCD_DrawPoint(lcd, x0 + a, y0 - b, color);
        LCD_DrawPoint(lcd, x0 + b, y0 - a, color);
        LCD_DrawPoint(lcd, x0 + b, y0 + a, color);
        LCD_DrawPoint(lcd, x0 + a, y0 + b, color);
        LCD_DrawPoint(lcd, x0 - a, y0 + b, color);
        LCD_DrawPoint(lcd, x0 - b, y0 + a, color);
        LCD_DrawPoint(lcd, x0 - a, y0 - b, color);
        LCD_DrawPoint(lcd, x0 - b, y0 - a, color);
        a++;
        if (di < 0)
            di += 4 * a + 6;
        else
        {
            di += 10 + 4 * (a - b);
            b--;
        }
    }
}

void LCD_SetFont(LCD_Handler_t *lcd, LCD_Font_t *font)
{
    if (lcd && font)
        lcd->CurrentFont = font;
}

void LCD_SetColors(LCD_Handler_t *lcd, uint32_t text, uint32_t back)
{
    if (lcd)
    {
        lcd->TextColor = text;
        lcd->BackColor = back;
    }
}

// 修改显示方向
void LCD_WriteByte(LCD_Handler_t *lcd, uint16_t x, uint16_t y, uint8_t byte, bool mode)
{
    for (uint8_t bit = 0; bit < 8; bit++)
    {
        if (byte & (1 << bit))
        {
            if (mode)
                LCD_DrawPoint(lcd, x + bit, y, lcd->TextColor);
            else
                LCD_DrawPoint(lcd, x, y + bit, lcd->BackColor);
        }
    }
}

void LCD_ShowImg(LCD_Handler_t *lcd, uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint8_t *img)
{
    if (!lcd || !img)
        return;
    // 行增加
    for (uint16_t j = 0; j < w; j++)
    {
        // 列增加
        for (uint16_t i = 0; i < h; i++)
        {
            uint8_t byte = img[j * w + i];
            LCD_WriteByte(lcd, x + j, y + i, byte, true);
        }
    }
}

void LCD_ShowChar(LCD_Handler_t *lcd, uint16_t x, uint16_t y, char c)
{
    if (!lcd || !lcd->CurrentFont)
        return;
    LCD_ShowImg(lcd, x, y, 1, lcd->CurrentFont->Height,
                lcd->CurrentFont->table[c - 32]);
}

void LCD_ShowString(LCD_Handler_t *lcd, uint16_t x, uint16_t y, const char *str)
{
    if (!lcd || !lcd->CurrentFont)
        return;
    while (*str)
    {
        if (x + lcd->CurrentFont->Width > lcd->Width)
        {
            x = 0;
            y += lcd->CurrentFont->Height;
        }
        if (y + lcd->CurrentFont->Height > lcd->Height)
            break;
        LCD_ShowChar(lcd, x, y, *str);
        x += lcd->CurrentFont->Width;
        str++;
    }
}

/* ================= 终端模式实现 ================= */

void LCD_Terminal_Enable(LCD_Handler_t *lcd, bool enable)
{
    if (lcd)
        lcd->TerminalMode = enable;
}

void LCD_Terminal_Clear(LCD_Handler_t *lcd)
{
    if (!lcd)
        return;
    LCD_Clear(lcd, lcd->BackColor);
    lcd->CursorX = 0;
    lcd->CursorY = 0;
}

// 软件滚动实现 (需要 GetPixel 支持，否则只能清屏)
static void LCD_Scroll_Software(LCD_Handler_t *lcd, uint16_t lines)
{
    if (!lcd || !lcd->GetPixel)
    {
        // 不支持读点，无法软件滚动，只能清屏
        LCD_Clear(lcd, lcd->BackColor);
        return;
    }

    // 将屏幕内容向上搬运
    for (uint16_t y = 0; y < lcd->Height - lines; y++)
    {
        for (uint16_t x = 0; x < lcd->Width; x++)
        {
            uint32_t color = lcd->GetPixel(x, y + lines);
            lcd->SetPixel(x, y, color);
        }
    }
    // 清除底部区域
    LCD_FillRect(lcd, 0, lcd->Height - lines, lcd->Width, lines, lcd->BackColor);
}

void LCD_Terminal_Scroll(LCD_Handler_t *lcd, uint16_t lines)
{
    if (!lcd)
        return;

    if (lcd->ScrollHard)
    {
        lcd->ScrollHard(lines);
    }
    else
    {
        LCD_Scroll_Software(lcd, lines);
    }

    if (lcd->Update)
        lcd->Update();
}

void LCD_Printf(LCD_Handler_t *lcd, const char *fmt, ...)
{
    if (!lcd || !lcd->CurrentFont)
        return;

    char buf[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    char *str = buf;
    while (*str)
    {
        char c = *str++;

        if (c == '\n')
        {
            lcd->CursorX = 0;
            lcd->CursorY += lcd->CurrentFont->Height;
        }
        else if (c == '\r')
        {
            lcd->CursorX = 0;
        }
        else
        {
            // 自动换行
            if (lcd->CursorX + lcd->CurrentFont->Width > lcd->Width)
            {
                lcd->CursorX = 0;
                lcd->CursorY += lcd->CurrentFont->Height;
            }

            // 检查滚动
            if (lcd->CursorY + lcd->CurrentFont->Height > lcd->Height)
            {
                LCD_Terminal_Scroll(lcd, lcd->CurrentFont->Height);
                lcd->CursorY -= lcd->CurrentFont->Height;
            }

            LCD_ShowChar(lcd, lcd->CursorX, lcd->CursorY, c);
            lcd->CursorX += lcd->CurrentFont->Width;
        }
    }

    if (lcd->Update)
        lcd->Update();
}

void LCD_Update(LCD_Handler_t *lcd)
{
    if (lcd && lcd->Update)
        lcd->Update();
}

/* ================= Display 框架适配层 ================= */

static LCD_Handler_t *g_AdapterLCD = NULL;

static int Adapter_Init(void)
{
    // LCD 已经在外部初始化，这里不需要做太多事情
    // 或者可以在这里调用 LCD_Clear
    if (g_AdapterLCD)
        LCD_Clear(g_AdapterLCD, g_AdapterLCD->BackColor);
    return 0;
}

static int Adapter_Clear(void)
{
    if (g_AdapterLCD)
        LCD_Clear(g_AdapterLCD, g_AdapterLCD->BackColor);
    return 0;
}

static int Adapter_SetCursor(uint16_t x, uint16_t y)
{
    if (g_AdapterLCD)
    {
        g_AdapterLCD->CursorX = x;
        g_AdapterLCD->CursorY = y;
    }
    return 0;
}

static int Adapter_WriteChar(char c)
{
    if (g_AdapterLCD)
    {
        // 使用当前光标位置显示
        LCD_ShowChar(g_AdapterLCD, g_AdapterLCD->CursorX, g_AdapterLCD->CursorY, c);
        // 注意：Display 框架会维护光标移动，这里不需要手动增加 CursorX
        // 除非 Display 框架处于非终端模式
    }
    return 0;
}

static int Adapter_WriteString(const char *str)
{
    if (g_AdapterLCD)
    {
        // 如果 Display 框架没有配置字体，它会调用 WriteString
        // 此时我们使用 LCD 自己的终端逻辑
        LCD_Printf(g_AdapterLCD, "%s", str);
    }
    return 0;
}

static int Adapter_Scroll(uint16_t lines)
{
    if (g_AdapterLCD)
    {
        LCD_Terminal_Scroll(g_AdapterLCD, lines);
    }
    return 0;
}

void LCD_BindToDisplay(LCD_Handler_t *lcd, DisplayDevice_t *dev, const char *name)
{
    if (!lcd || !dev)
        return;

    g_AdapterLCD = lcd;

    dev->name = name;
    dev->width = lcd->Width;
    dev->height = lcd->Height;

    // 填充字体信息，启用 Display 框架的终端模式
    if (lcd->CurrentFont)
    {
        dev->font_width = lcd->CurrentFont->Width;
        dev->font_height = lcd->CurrentFont->Height;
    }
    else
    {
        dev->font_width = 0;
        dev->font_height = 0;
    }

    dev->Init = Adapter_Init;
    dev->Clear = Adapter_Clear;
    dev->SetCursor = Adapter_SetCursor;
    dev->WriteChar = Adapter_WriteChar;
    dev->WriteString = Adapter_WriteString; // 用于非终端模式回退
    dev->Scroll = Adapter_Scroll;

    // 还可以适配 DrawPixel 等
}
