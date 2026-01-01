#include "main.h"

// SH1106适配函数（因为SH1106的接口与LCD_Handler_t不完全兼容）
static void SH1106_SetPixel_Adapter(uint16_t x, uint16_t y, uint32_t color)
{
    // SH1106是单色屏，只要color非0就点亮
    if (color)
    {
        SH1106_DrawPoint(x, y);
    }
}

static uint32_t SH1106_GetPixel_Adapter(uint16_t x, uint16_t y)
{
    return SH1106_GetPoint(x, y);
}

shell Shell = {
    .Shell_Init = false,               // Shell未初始化
    .c = 0,                            // 初始化接收字符
    .Res_len = 0,                      // 初始化接收长度
    .UART_NOTE = 0,                    // 初始化串口节点
    .RunStae = 0,                      // 初始化运行状态
    .Data_Receive = USART1_ReceiveChar // 数据接收函数指针
};

Sysfpoint Shell_Sysfpoint;

DeviceFamily STM32F103C8T6_Device = {
    .Architecture = "cortex-m3",
    .DeviceName = "STM32F103C8T6",
    .OS = "BareMetal",
    .Device = "STM32F1",
    .User = "Admin",
    .Password = "133990",
    .Version = "1.0.0"};

LCD_Handler_t lcd_sh1106 = {
    .Width = SH1106_WIDTH,
    .Height = SH1106_HEIGHT,
    .SetPixel = SH1106_SetPixel,
    .GetPixel = SH1106_GetPoint,
    .FillRect = SH1106_FillRect, // SH1106没有硬件块填充，由框架模拟
    .Update = SH1106_Update,
    .ScrollHard = NULL, // 可选实现
    .CursorX = 0,
    .CursorY = 0,
    .CurrentFont = &Consolas_Font_8x16, // 可选设置
    .TextColor = 0xFFFFFFFF,
    .BackColor = 0x00000000,
    .TerminalMode = true};

// LCD_Handler_t lcd_ssd1306 = {
//     .Width = SSD1306_WIDTH,
//     .Height = SSD1306_HEIGHT,
//     .SetPixel = SSD1306_SetPixel,
//     .GetPixel = NULL, // 可选实现
//     .FillRect = NULL, // 可选实现
//     .Update = SSD1306_Update,
//     .ScrollHard = NULL, // 可选实现
//     .CursorX = 0,
//     .CursorY = 0,
//     .CurrentFont = &JetBrains_Mono_Font_8x16, // 可选设置
//     .TextColor = 0xFFFFFFFF,
//     .BackColor = 0x00000000,
//     .TerminalMode = true};

dev_info_t Dev_info_poor[] = {

    //     {.name = OLED_SH1106_NAME,
    //   .init = sh1106_dev_init,
    //   .enable = NULL,
    //   .disable = NULL,
    //   .arg.ptr = (void *)&lcd_sh1106},

    {.name = DEBUG_UART_NAME,
     .init = usart1_init,
     .enable = usart1_start,
     .disable = usart1_stop,
     .arg.ptr = (void *)&debug},

    // {.name = "SysTick",
    //  .init = systick_init,
    //  .enable = NULL,  // SysTick不需要启用函数
    //  .disable = NULL, // SysTick不需要禁用函数
    //  .arg.ptr = NULL},

    {.name = "NVIC",
     .init = nvic_init,
     .enable = NULL,  // NVIC不需要启用函数
     .disable = NULL, // NVIC不需要禁用函数
     .arg.ptr = NULL},

    {.name = ONBOARD_LED_NAME,
     .init = led_init,
     .enable = led_on,
     .disable = led_off,
     .arg.ptr = NULL},

    //{.name = OLED_SSD1306_NAME,
    //     .init = ssd1306_dev_init,
    //     .enable = NULL,
    //     .disable = NULL,
    //     .arg.ptr = (void *)&lcd_ssd1306},
    // {.name = ADC1_NAME,
    //  .init = adc1_init,
    //  .enable = adc1_enable,
    //  .disable = adc1_disable,
    //  .arg.ptr = NULL},

    {.name = "", // 空字符串表示数组结束
     .init = NULL,
     .enable = NULL,
     .disable = NULL,
     .arg.ptr = NULL}

};

EnvVar env_vars[] = {
    {NULL} // 环境变量列表结束标志
};