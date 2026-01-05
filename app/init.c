#include "main.h"
#include "config.h"
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

#ifdef USE_DEVICE_SH1106
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
#elif USE_DEVICE_SSD1306
LCD_Handler_t lcd_ssd1306 = {
    .Width = SSD1306_WIDTH,
    .Height = SSD1306_HEIGHT,
    .SetPixel = SSD1306_SetPixel,
    .GetPixel = NULL, // 可选实现
    .FillRect = NULL, // 可选实现
    .Update = SSD1306_Update,
    .ScrollHard = NULL, // 可选实现
    .CursorX = 0,
    .CursorY = 0,
    .CurrentFont = &Consolas_Font_8x16, // 可选设置
    .TextColor = 0xFFFFFFFF,
    .BackColor = 0x00000000,
    .TerminalMode = true};
#elif USE_DEVICE_ST7789
LCD_Handler_t lcd_st7789 = {
    .Width = ST7789_WIDTH,
    .Height = ST7789_HEIGHT,
    .SetPixel = ST7789_SetPixel,
    .GetPixel = NULL, // 可选实现
    .FillRect = ST7789_FillRect,
    .Update = NULL,     // 可选实现
    .ScrollHard = NULL, // 可选实现
    .CursorX = 0,
    .CursorY = 0,
    .CurrentFont = &Consolas_Font_8x16, // 可选设置
    .TextColor = 0xFFFFFFFF,
    .BackColor = 0x00000000,
    .TerminalMode = true};
#endif

#ifdef USE_DEVICE_MPU6050
float mpu6050_sensor_data[3] = {0};
#endif


df_dev_t Dev_info_poor[] = {

    // {.name = OLED_NAME,
    //  .init = sh1106_dev_init,
    //  .enable = NULL,
    //  .disable = NULL,
    //  .arg.ptr = ptr(&lcd_sh1106)},
    {
        .name = LCD_NAME,
        .init = st7789_dev_init,
        .enable = NULL,
        .disable = NULL,
        .arg.ptr = ptr(&lcd_st7789)
    },
    // {.name = MPU6050_NAME,
    //  .init = mpu6050_dev_init,
    //  .enable = mpu6050_dev_enable,
    //  .disable = mpu6050_dev_disable,
    //  .read = mpu6050_dev_read,
    //  .arg.argv = argv(ptr(mpu6050_sensor_data), ptr(&lcd_sh1106))},

    //{.name = OLED_SSD1306_NAME,
    //     .init = ssd1306_dev_init,
    //     .enable = NULL,
    //     .disable = NULL,
    //     .arg.ptr = ptr(&lcd_ssd1306)},

    DF_DEV_END

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
static int df_interface_auto_init(void)
{
    // 初始化I2C框架
    Device_HAL_Init();
    // I2C框架暂无需特殊初始化，此函数用于日志记录
    #ifdef __SOFTI2C_
    LOG_I("IIC", "Soft I2C framework initialized");
    #elif __HARDI2C_
    LOG_I("IIC", "Hard I2C framework initialized");
    #endif
    #ifdef __SOFTSPI_
    LOG_I("SPI", "Soft SPI framework initialized");
    #elif __HARDSPI_
    LOG_I("SPI", "Hard SPI framework initialized");
    #endif
    return 0;
}

// 将I2C框架初始化注册到DEVICE级别
DF_INIT_EXPORT(df_interface_auto_init, DF_INIT_EXPORT_PREV);

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