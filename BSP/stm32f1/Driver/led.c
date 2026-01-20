/**
 * @file led.c
 * @brief STM32F1 LED/GPIO 片上外设驱动
 * @note 使用 f103_gpio 底层驱动实现 df_led.h 和 df_gpio.h 接口
 * @note 片上外设通过 DF_BOARD_INIT 分散加载初始化
 */

#include "driver.h"
#include "df_led.h"
#include "df_gpio.h"
#include "df_init.h"

/*============================ 板载 LED 配置 ============================*/
/* 默认使用 PC13 (蓝色药丸板/最小系统板) */
#ifndef ONBOARD_LED_PORT
#define ONBOARD_LED_PORT F103_GPIOC
#endif

#ifndef ONBOARD_LED_PIN
#define ONBOARD_LED_PIN F103_PIN_13
#endif

/* LED 低电平点亮 */
#ifndef LED_ACTIVE_LOW
#define LED_ACTIVE_LOW 1
#endif

/*============================ 前向声明 ============================*/
int led_init(df_arg_t arg);
int led_on(df_arg_t arg);
int led_off(df_arg_t arg);
int led_toggle(df_arg_t arg);

/*============================ LED 设备实例 ============================*/
df_led_t led = {
    .init_flag = false,
    .num = 1,
    .state = false,
    .name = ONBOARD_LED_NAME,
    .init = led_init,
    .on = led_on,
    .off = led_off,
    .toggle = led_toggle};

/*============================ GPIO 设备实例 ============================*/
/* 可选：提供通用 GPIO 接口 */
static int gpio_pc13_init(df_arg_t arg);
static int gpio_pc13_set_high(df_arg_t arg);
static int gpio_pc13_set_low(df_arg_t arg);
static int gpio_pc13_toggle(df_arg_t arg);
static int gpio_pc13_read(df_arg_t arg);

df_gpio_t gpio_led = {
    .init_flag = false,
    .state = false,
    .group = F103_GPIOC,
    .pin = F103_PIN_13,
    .name = "gpio_pc13",
    .init = gpio_pc13_init,
    .deinit = NULL,
    .set_high = gpio_pc13_set_high,
    .set_low = gpio_pc13_set_low,
    .toggle = gpio_pc13_toggle,
    .read = gpio_pc13_read};

/*============================ LED 接口实现 ============================*/

/**
 * @brief 初始化板载 LED
 */
int led_init(df_arg_t arg)
{
    (void)arg;

    /* 使用 f103 驱动初始化 GPIO */
    f103_gpio_init_quick(ONBOARD_LED_PORT, ONBOARD_LED_PIN,
                         F103_GPIO_MODE_OUT_PP, F103_GPIO_SPEED_50MHZ);

    /* 初始状态：关闭 LED */
#if LED_ACTIVE_LOW
    f103_gpio_write(ONBOARD_LED_PORT, ONBOARD_LED_PIN, true); /* 高电平熄灭 */
#else
    f103_gpio_write(ONBOARD_LED_PORT, ONBOARD_LED_PIN, false); /* 低电平熄灭 */
#endif

    led.init_flag = true;
    led.state = false;

    return 0;
}

/**
 * @brief 点亮 LED
 */
int led_on(df_arg_t arg)
{
    (void)arg;

#if LED_ACTIVE_LOW
    f103_gpio_write(ONBOARD_LED_PORT, ONBOARD_LED_PIN, false); /* 低电平点亮 */
#else
    f103_gpio_write(ONBOARD_LED_PORT, ONBOARD_LED_PIN, true); /* 高电平点亮 */
#endif

    led.state = true;
    return 0;
}

/**
 * @brief 熄灭 LED
 */
int led_off(df_arg_t arg)
{
    (void)arg;

#if LED_ACTIVE_LOW
    f103_gpio_write(ONBOARD_LED_PORT, ONBOARD_LED_PIN, true); /* 高电平熄灭 */
#else
    f103_gpio_write(ONBOARD_LED_PORT, ONBOARD_LED_PIN, false); /* 低电平熄灭 */
#endif

    led.state = false;
    return 0;
}

/**
 * @brief 翻转 LED 状态
 */
int led_toggle(df_arg_t arg)
{
    (void)arg;

    f103_gpio_toggle(ONBOARD_LED_PORT, ONBOARD_LED_PIN);
    led.state = !led.state;

    return 0;
}

/*============================ GPIO 接口实现 ============================*/

static int gpio_pc13_init(df_arg_t arg)
{
    (void)arg;

    f103_gpio_init_quick(F103_GPIOC, F103_PIN_13,
                         F103_GPIO_MODE_OUT_PP, F103_GPIO_SPEED_50MHZ);
    gpio_led.init_flag = true;

    return 0;
}

static int gpio_pc13_set_high(df_arg_t arg)
{
    (void)arg;
    f103_gpio_write(F103_GPIOC, F103_PIN_13, true);
    gpio_led.state = true;
    return 0;
}

static int gpio_pc13_set_low(df_arg_t arg)
{
    (void)arg;
    f103_gpio_write(F103_GPIOC, F103_PIN_13, false);
    gpio_led.state = false;
    return 0;
}

static int gpio_pc13_toggle(df_arg_t arg)
{
    (void)arg;
    f103_gpio_toggle(F103_GPIOC, F103_PIN_13);
    gpio_led.state = !gpio_led.state;
    return 0;
}

static int gpio_pc13_read(df_arg_t arg)
{
    (void)arg;
    gpio_led.state = f103_gpio_read(F103_GPIOC, F103_PIN_13);
    return gpio_led.state ? 1 : 0;
}

/*============================ 片上外设自动初始化 ============================*/

/**
 * @brief LED 自动初始化函数
 * @note 通过 DF_BOARD_INIT 宏在系统启动时自动调用
 */
static int led_auto_init(void)
{
    LOG_I("LED","Initializing onboard LED...");
    return led_init(arg_null);
}
DF_PREV_INIT(led_auto_init);
