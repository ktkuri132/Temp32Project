#include "display/df_display.h"
#include "lcd/df_lcd.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

// 内部变量：设备链表头和当前设备指针
static DisplayDevice_t *dev_head = NULL;
static DisplayDevice_t *current_dev = NULL;

// 注册设备
static int Display_Register(DisplayDevice_t *dev)
{
    if (dev == NULL)
        return -1;

    // 简单的链表插入（头插法）
    dev->next = dev_head;
    dev_head = dev;

    // 如果当前没有选中设备，默认选中第一个注册的设备
    if (current_dev == NULL)
    {
        current_dev = dev;
    }

    // 尝试初始化设备
    if (dev->Init)
    {
        dev->Init();
    }

    return 0;
}

// 注册 LCD 设备
static int Display_RegisterLCD(DisplayDevice_t *dev, LCD_Handler_t *lcd, const char *name)
{
    if (dev == NULL || lcd == NULL)
        return -1;

    memset(dev, 0, sizeof(DisplayDevice_t));
    dev->name = name;
    dev->type = DISPLAY_TYPE_LCD;
    // dev->lcd_handler = (void *)lcd;

    // 填充一些基本信息
    // dev->width = ((LCD_Handler_t *)dev->lcd_handler)->Width;
    // dev->height = ((LCD_Handler_t *)dev->lcd_handler)->Height;

    // 启用 LCD 的终端模式
    // LCD_Terminal_Enable(dev->lcd_handler, true);
    return Display_Register(dev);
}

// 选择设备
static int Display_Select(const char *name)
{
    DisplayDevice_t *p = dev_head;
    while (p != NULL)
    {
        if (strcmp(p->name, name) == 0)
        {
            current_dev = p;
            return 0;
        }
        p = p->next;
    }
    return -1; // 未找到
}

// 输出字符串 (带终端逻辑)
static int Display_Output(const char *str)
{
    if (current_dev == NULL)
        return -1;

    // 如果是 LCD 设备，直接调用 LCD 框架的 Printf (它支持流式输出和滚动)
    if (current_dev->type == DISPLAY_TYPE_LCD && current_dev->lcd_handler)
    {
        LCD_Printf((LCD_Handler_t *)current_dev->lcd_handler, "%s", str);
        return 0;
    }

    // 如果没有配置字体信息，回退到简单透传模式
    if (current_dev->font_width == 0 || current_dev->font_height == 0)
    {
        if (current_dev->WriteString)
        {
            return current_dev->WriteString(str);
        }
        return -1;
    }

    // 终端模式逻辑
    while (*str)
    {
        char c = *str++;

        // 处理换行符
        if (c == '\n')
        {
            current_dev->cursor_x = 0;
            current_dev->cursor_y += current_dev->font_height;
        }
        // 处理回车符 (通常与 \n 一起，这里简单处理为归零)
        else if (c == '\r')
        {
            current_dev->cursor_x = 0;
        }
        // 普通字符
        else
        {
            // 如果当前行已满，自动换行
            if (current_dev->cursor_x + current_dev->font_width > current_dev->width)
            {
                current_dev->cursor_x = 0;
                current_dev->cursor_y += current_dev->font_height;
            }

            // 检查是否需要滚动
            if (current_dev->cursor_y + current_dev->font_height > current_dev->height)
            {
                if (current_dev->Scroll)
                {
                    // 滚动一行
                    current_dev->Scroll(current_dev->font_height);
                    current_dev->cursor_y -= current_dev->font_height;
                }
                else
                {
                    // 不支持滚动，清屏回顶
                    if (current_dev->Clear)
                        current_dev->Clear();
                    current_dev->cursor_x = 0;
                    current_dev->cursor_y = 0;
                }
            }

            // 输出字符
            if (current_dev->SetCursor)
                current_dev->SetCursor(current_dev->cursor_x, current_dev->cursor_y);
            if (current_dev->WriteChar)
                current_dev->WriteChar(c);

            current_dev->cursor_x += current_dev->font_width;
        }
    }

    return 0;
}

// 格式化输出
static int Display_Printf(const char *fmt, ...)
{
    if (current_dev == NULL)
        return -1;

    char buf[256]; // 缓冲区，注意栈溢出风险
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    if (len > 0)
    {
        Display_Output(buf); // 调用统一的 Output 接口
    }
    return len;
}

// 清屏
static int Display_Clear(void)
{
    if (current_dev)
    {
        if (current_dev->type == DISPLAY_TYPE_LCD && current_dev->lcd_handler)
        {
            LCD_Terminal_Clear((LCD_Handler_t *)current_dev->lcd_handler);
            return 0;
        }

        // 重置光标
        current_dev->cursor_x = 0;
        current_dev->cursor_y = 0;
        if (current_dev->Clear)
            return current_dev->Clear();
    }
    return -1;
}

// 实例化全局管理器
DisplayManager_t Display = {
    .Register = Display_Register,
    .RegisterLCD = NULL,
    .Select = Display_Select,
    .Output = Display_Output,
    .Printf = Display_Printf,
    .Clear = Display_Clear};
