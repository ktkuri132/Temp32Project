#include "main.h"

shell Shell = {
    .Shell_Init = false, // Shell未初始化
    .c = 0,              // 初始化接收字符
    .Res_len = 0,        // 初始化接收长度
    .UART_NOTE = 0,      // 初始化串口节点
    .RunStae = 0,        // 初始化运行状态
    .Data_Receive = NULL // 数据接收函数指针（需要适配新接口）
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

df_dev_t Dev_info_poor[] = {

        {.name = OLED_SH1106_NAME,
      .init = sh1106_dev_init,
      .enable = NULL,
      .disable = NULL,
      .arg.ptr = (void *)&lcd_sh1106},

    // {.name = ONBOARD_LED_NAME,
    //  .init = led_init,
    //  .enable = led_on,
    //  .disable = led_off,
    //  .arg.ptr = NULL},

    //{.name = OLED_SSD1306_NAME,
    //     .init = ssd1306_dev_init,
    //     .enable = NULL,
    //     .disable = NULL,
    //     .arg.ptr = (void *)&lcd_ssd1306},

    {.name = "", // 空字符串表示数组结束
     .init = NULL,
     .enable = NULL,
     .disable = NULL,
     .arg.ptr = NULL}

};

EnvVar env_vars[] = {
    {NULL} // 环境变量列表结束标志
};

// ============ 自动初始化 ============
/**
 * @brief I2C框架自动初始化函数
 * @details 在框架初始化时自动调用，初始化I2C通信框架
 * @return 0表示成功
 */
static int df_iic_auto_init(void)
{
    // 初始化I2C框架
    Device_HAL_Init();
    // I2C框架暂无需特殊初始化，此函数用于日志记录
    LOG_I("IIC", "I2C framework initialized");
    return 0;
}

// 将I2C框架初始化注册到DEVICE级别
DF_INIT_EXPORT(df_iic_auto_init, DF_INIT_EXPORT_PREV);


// ============ 自动初始化 ============
/**
 * @brief 设备框架自动初始化函数
 * @details 在框架初始化时自动调用，初始化设备管理框架
 * @return 0表示成功
 */
static int df_device_auto_init(void)
{

    df_dev_register(Dev_info_poor); // 初始化设备模型
    LOG_I("DEV", "Device framework initialized\n");
    return 0;
}

// 将设备框架初始化注册到PREV级别（在BOARD之后）
DF_INIT_EXPORT(df_device_auto_init, DF_INIT_EXPORT_DEVICE);