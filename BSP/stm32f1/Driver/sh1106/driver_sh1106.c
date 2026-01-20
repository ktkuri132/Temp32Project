#include "driver_sh1106.h"
#ifdef USE_DEVICE_SH1106

#define SH1106_DC_PORT F103_GPIOB
#define SH1106_DC_PIN F103_PIN_2
#define SH1106_RES_PORT F103_GPIOA
#define SH1106_RES_PIN F103_PIN_7
#define SH1106_CS_PORT F103_GPIOB
#define SH1106_CS_PIN F103_PIN_12

/**
 * @brief SH1106 SPI GPIO初始化
 */
void sh1106_pin_init(void)
{
    f103_gpio_init_quick(SH1106_DC_PORT, SH1106_DC_PIN, F103_GPIO_MODE_OUT_PP, F103_GPIO_SPEED_50MHZ);
    f103_gpio_init_quick(SH1106_RES_PORT, SH1106_RES_PIN, F103_GPIO_MODE_OUT_PP, F103_GPIO_SPEED_50MHZ);
    f103_gpio_init_quick(SH1106_CS_PORT, SH1106_CS_PIN, F103_GPIO_MODE_OUT_PP, F103_GPIO_SPEED_50MHZ);
}

/**
 * @brief SH1106 复位控制
 */
void sh1106_res_set(bool level)
{
    f103_gpio_write(SH1106_RES_PORT, SH1106_RES_PIN, level);
}

/**
 * @brief SH1106 数据/命令线控制
 */
void sh1106_dc_set(bool level)
{
    f103_gpio_write(SH1106_DC_PORT, SH1106_DC_PIN, level);
}

/**
 * @brief SH1106 片选控制
 */
void sh1106_cs_set(bool level)
{
    f103_gpio_write(SH1106_CS_PORT, SH1106_CS_PIN, level);
}

private_sh1106_t sh1106_private_hal = {
    .pin_init = sh1106_pin_init,
    .dc_control = sh1106_dc_set,
    .res_control = sh1106_res_set
};


int sh1106_dev_init(df_arg_t arg)
{
    LCD_Handler_t *lcd = (LCD_Handler_t *)arg.ptr;
    if (lcd == NULL)
    {
        LOG_E("SH1106", "sh1106_dev_init: lcd handler is NULL!");
        return -1;
    }
    if (lcd->SetPixel == NULL)
    {
        LOG_E("SH1106", "sh1106_dev_init: lcd SetPixel function is NULL!");
        return -1;
    }
    if (lcd->Width != 128 || lcd->Height != 64)
    {
        LOG_E("SH1106", "sh1106_dev_init: lcd size mismatch! Expected 128x64.");
        return -1;
    }
    if (lcd->Update == NULL)
    {
        LOG_E("SH1106", "sh1106_dev_init: lcd Update function is NULL!");
        return -1;
    }
    delay.ms(arg_u32(100)); // 等待电源稳定
    int8_t ret = Device_SH1106_Init(&sh1106_private_hal);
    if (ret != 0)
    {
        LOG_E("SH1106", "sh1106_dev_init: Device_SH1106_Init failed!");
        LOG_E("SH1106", "Device_SH1106_Init: return code %d", ret);
        return -1;
    }
    LCD_Clear(lcd, 0); // 清屏，黑色背景
    LCD_Printf(lcd, "System Start\n");
    LCD_Printf(lcd, "SH1106 OLED Initialized.\n");
    return 0;
}

#endif