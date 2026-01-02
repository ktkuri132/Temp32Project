/**
 * @file led.c
 * @brief STM32F4 LED/GPIO 片上外设驱动
 * @note 使用 f407_gpio 底层驱动实现 df_led.h 和 df_gpio.h 接口
 * @note 片上外设通过 DF_BOARD_INIT 分散加载初始化
 */

#include "driver.h"
#include "df_led.h"
#include "df_gpio.h"
#include "df_init.h"
#include "f407_gpio.h"

/*============================ 板载 LED 配置 ============================*/
/* 默认使用 PC0 */
#ifndef ONBOARD_LED_PORT
#define ONBOARD_LED_PORT F407_GPIOC
#endif

#ifndef ONBOARD_LED_PIN
#define ONBOARD_LED_PIN F407_PIN_0
#endif

/* LED 低电平点亮 */
#ifndef LED_ACTIVE_LOW
#define LED_ACTIVE_LOW 1
#endif

/* LED GPIO ID */
#define ONBOARD_LED_ID F407_GPIO_ID(ONBOARD_LED_PORT, ONBOARD_LED_PIN)

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
static int gpio_led_init(df_arg_t arg);
static int gpio_led_set_high(df_arg_t arg);
static int gpio_led_set_low(df_arg_t arg);
static int gpio_led_toggle(df_arg_t arg);
static int gpio_led_read(df_arg_t arg);

df_gpio_t gpio_led = {
    .init_flag = false,
    .state = false,
    .group = ONBOARD_LED_PORT,
    .pin = ONBOARD_LED_PIN,
    .name = "gpio_pc0",
    .init = gpio_led_init,
    .deinit = NULL,
    .set_high = gpio_led_set_high,
    .set_low = gpio_led_set_low,
    .toggle = gpio_led_toggle,
    .read = gpio_led_read};

/*============================ LED 接口实现 ============================*/

/**
 * @brief 初始化板载 LED
 */
int led_init(df_arg_t arg)
{
    (void)arg;

    /* 使用 f407 驱动初始化 GPIO */
    f407_gpio_config_output_pp(ONBOARD_LED_ID);

    /* 初始状态：关闭 LED */
#if LED_ACTIVE_LOW
    f407_gpio_set(ONBOARD_LED_ID); /* 高电平熄灭 */
#else
    f407_gpio_reset(ONBOARD_LED_ID); /* 低电平熄灭 */
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
    f407_gpio_reset(ONBOARD_LED_ID); /* 低电平点亮 */
#else
    f407_gpio_set(ONBOARD_LED_ID); /* 高电平点亮 */
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
    f407_gpio_set(ONBOARD_LED_ID); /* 高电平熄灭 */
#else
    f407_gpio_reset(ONBOARD_LED_ID); /* 低电平熄灭 */
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

    f407_gpio_toggle(ONBOARD_LED_ID);
    led.state = !led.state;

    return 0;
}

/*============================ GPIO 接口实现 ============================*/

static int gpio_led_init(df_arg_t arg)
{
    (void)arg;

    f407_gpio_config_output_pp(ONBOARD_LED_ID);
    gpio_led.init_flag = true;

    return 0;
}

static int gpio_led_set_high(df_arg_t arg)
{
    (void)arg;
    f407_gpio_set(ONBOARD_LED_ID);
    gpio_led.state = true;
    return 0;
}

static int gpio_led_set_low(df_arg_t arg)
{
    (void)arg;
    f407_gpio_reset(ONBOARD_LED_ID);
    gpio_led.state = false;
    return 0;
}

static int gpio_led_toggle(df_arg_t arg)
{
    (void)arg;
    f407_gpio_toggle(ONBOARD_LED_ID);
    gpio_led.state = !gpio_led.state;
    return 0;
}

static int gpio_led_read(df_arg_t arg)
{
    (void)arg;
    gpio_led.state = f407_gpio_read(ONBOARD_LED_ID);
    return gpio_led.state ? 1 : 0;
}

/*============================ 片上外设自动初始化 ============================*/

/**
 * @brief LED 自动初始化函数
 * @note 通过 DF_BOARD_INIT 宏在系统启动时自动调用
 */
static int led_auto_init(void)
{
    return led_init(arg_null);
}
DF_BOARD_INIT(led_auto_init);
