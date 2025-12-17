#ifndef __DF_LCD_H
#define __DF_LCD_H

#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include "df_display.h" // 引入显示框架定义

/* ================= 颜色定义 (RGB565) ================= */
#define LCD_COLOR_BLACK 0x0000
#define LCD_COLOR_NAVY 0x000F
#define LCD_COLOR_DARKGREEN 0x03E0
#define LCD_COLOR_DARKCYAN 0x03EF
#define LCD_COLOR_MAROON 0x7800
#define LCD_COLOR_PURPLE 0x780F
#define LCD_COLOR_OLIVE 0x7BE0
#define LCD_COLOR_LIGHTGREY 0xC618
#define LCD_COLOR_DARKGREY 0x7BEF
#define LCD_COLOR_BLUE 0x001F
#define LCD_COLOR_GREEN 0x07E0
#define LCD_COLOR_CYAN 0x07FF
#define LCD_COLOR_RED 0xF800
#define LCD_COLOR_MAGENTA 0xF81F
#define LCD_COLOR_YELLOW 0xFFE0
#define LCD_COLOR_WHITE 0xFFFF
#define LCD_COLOR_ORANGE 0xFD20
#define LCD_COLOR_GREENYELLOW 0xAFE5
#define LCD_COLOR_PINK 0xF81F

/* ================= 字体结构定义 ================= */

typedef union {
    const uint8_t (*b8)[6]; // 字体数据表指针 (二维数组，2字节每字符)
    const uint8_t (*b16)[16]; // 字体数据表指针 (二维数组，16字节每字符)
} LCD_FontTable_t;

typedef struct
{
    LCD_FontTable_t table; // 字体数据表
    uint16_t Width;       // 字符宽度
    uint16_t Height;      // 字符高度
} LCD_Font_t;

/* ================= LCD 控制句柄 ================= */
typedef struct
{
    /* --- 硬件属性 --- */
    uint16_t Width;  // 屏幕宽度
    uint16_t Height; // 屏幕高度

    /* --- 底层接口 (必须实现 SetPixel) --- */
    // 画点函数: x, y, color
    void (*SetPixel)(uint16_t x, uint16_t y, uint32_t color);

    // 读点函数 (可选，用于软件滚动): x, y. 返回 color
    uint32_t (*GetPixel)(uint16_t x, uint16_t y);

    // 块填充函数 (可选，加速清屏/矩形): x, y, w, h, color
    void (*FillRect)(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t color);

    // 显存刷新 (可选，用于带缓存的屏幕)
    void (*Update)(void);

    // 硬件滚动 (可选，如果提供则优先使用硬件滚动)
    void (*ScrollHard)(uint16_t lines);

    /* --- 终端模式状态 (由框架维护) --- */
    uint16_t CursorX;
    uint16_t CursorY;
    LCD_Font_t *CurrentFont;
    uint32_t TextColor;
    uint32_t BackColor;
    bool TerminalMode; // 是否开启终端模式 (自动换行/滚动)

} LCD_Handler_t;

/* ================= API 声明 ================= */

// 初始化句柄
void LCD_Handler_Init(LCD_Handler_t *lcd, uint16_t width, uint16_t height);

// 基础绘图
void LCD_Clear(LCD_Handler_t *lcd, uint32_t color);
void LCD_DrawPoint(LCD_Handler_t *lcd, uint16_t x, uint16_t y, uint32_t color);
void LCD_DrawLine(LCD_Handler_t *lcd, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint32_t color);
void LCD_DrawRect(LCD_Handler_t *lcd, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t color);
void LCD_FillRect(LCD_Handler_t *lcd, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t color);
void LCD_DrawCircle(LCD_Handler_t *lcd, uint16_t x0, uint16_t y0, uint16_t r, uint32_t color);
void LCD_ShowImg(LCD_Handler_t *lcd, uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint8_t *img);
void LCD_WriteByte(LCD_Handler_t *lcd,uint16_t x,uint16_t y,uint8_t byte,bool mode);

// 文本显示 (固定位置)
void LCD_SetFont(LCD_Handler_t *lcd, LCD_Font_t *font);
void LCD_SetColors(LCD_Handler_t *lcd, uint32_t text, uint32_t back);
void LCD_ShowChar(LCD_Handler_t *lcd, uint16_t x, uint16_t y, char c);
void LCD_ShowString(LCD_Handler_t *lcd, uint16_t x, uint16_t y, const char *str);

// 终端模式 (流式输出)
void LCD_Terminal_Enable(LCD_Handler_t *lcd, bool enable);
void LCD_Terminal_Clear(LCD_Handler_t *lcd);
void LCD_Printf(LCD_Handler_t *lcd, const char *fmt, ...);

// 显存操作
void LCD_Update(LCD_Handler_t *lcd);

// 对接 Display 框架
// 将 LCD 句柄转换为 DisplayDevice_t 结构，以便注册到 Display 框架
// 注意：由于 Display 框架回调不带上下文，目前同一时间只能有一个 LCD 绑定到 Display 框架
void LCD_BindToDisplay(LCD_Handler_t *lcd, DisplayDevice_t *dev, const char *name);

#endif // __DF_LCD_H
