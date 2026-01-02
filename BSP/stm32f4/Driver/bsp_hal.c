/**
 * @file bsp_hal.c
 * @brief BSP硬件抽象层 - POSIX风格接口实现
 * @details 桥接板级底层驱动(vet6_xxx.c)到Driver_Framework
 */

#include "bsp_hal.h"
#include "driver.h"
#include <stdarg.h>
#include <stdio.h>

/*===========================================================================*/
/*                              GPIO实现                                      */
/*===========================================================================*/

#if defined(CHIP_PACKAGE_VET6) || !defined(CHIP_PACKAGE)

int bsp_gpio_config(uint8_t port, uint8_t pin, bsp_gpio_dir_t dir, bsp_gpio_pull_t pull)
{
    if (port > 4 || pin > 15)
        return BSP_EINVAL;

    vet6_gpio_id_t gpio_id = (vet6_gpio_id_t)(port * 16 + pin);
    vet6_gpio_pull_t vet6_pull;

    switch (pull)
    {
    case BSP_GPIO_PULL_UP:
        vet6_pull = VET6_GPIO_PULL_UP;
        break;
    case BSP_GPIO_PULL_DOWN:
        vet6_pull = VET6_GPIO_PULL_DOWN;
        break;
    default:
        vet6_pull = VET6_GPIO_NOPULL;
        break;
    }

    if (dir == BSP_GPIO_DIR_OUTPUT)
    {
        return vet6_gpio_config_output(gpio_id, VET6_GPIO_SPEED_MEDIUM,
                                       VET6_GPIO_PUSH_PULL, vet6_pull);
    }
    else
    {
        return vet6_gpio_config_input(gpio_id, vet6_pull);
    }
}

int bsp_gpio_set(uint8_t port, uint8_t pin)
{
    if (port > 4 || pin > 15)
        return BSP_EINVAL;
    vet6_gpio_id_t gpio_id = (vet6_gpio_id_t)(port * 16 + pin);
    vet6_gpio_set(gpio_id);
    return BSP_OK;
}

int bsp_gpio_reset(uint8_t port, uint8_t pin)
{
    if (port > 4 || pin > 15)
        return BSP_EINVAL;
    vet6_gpio_id_t gpio_id = (vet6_gpio_id_t)(port * 16 + pin);
    vet6_gpio_reset(gpio_id);
    return BSP_OK;
}

int bsp_gpio_toggle(uint8_t port, uint8_t pin)
{
    if (port > 4 || pin > 15)
        return BSP_EINVAL;
    vet6_gpio_id_t gpio_id = (vet6_gpio_id_t)(port * 16 + pin);
    vet6_gpio_toggle(gpio_id);
    return BSP_OK;
}

int bsp_gpio_read(uint8_t port, uint8_t pin)
{
    if (port > 4 || pin > 15)
        return BSP_EINVAL;
    vet6_gpio_id_t gpio_id = (vet6_gpio_id_t)(port * 16 + pin);
    return vet6_gpio_read(gpio_id);
}

int bsp_gpio_write(uint8_t port, uint8_t pin, uint8_t value)
{
    if (value)
    {
        return bsp_gpio_set(port, pin);
    }
    else
    {
        return bsp_gpio_reset(port, pin);
    }
}

#endif

/*===========================================================================*/
/*                              UART实现                                      */
/*===========================================================================*/

#if defined(CHIP_PACKAGE_VET6) || !defined(CHIP_PACKAGE)

/* USART句柄数组 */
static vet6_usart_handle_t uart_handles[6];
static bool uart_initialized[6] = {false};

int bsp_uart_open(uint8_t uart_num, const bsp_uart_config_t *config)
{
    if (uart_num < 1 || uart_num > 6)
        return BSP_EINVAL;

    int idx = uart_num - 1;
    if (uart_initialized[idx])
        return BSP_EBUSY;

    uint32_t baudrate = config ? config->baudrate : 115200;

    int ret = vet6_usart_init_simple(&uart_handles[idx], (vet6_usart_t)idx, baudrate);
    if (ret == 0)
    {
        vet6_usart_enable(&uart_handles[idx]);
        uart_initialized[idx] = true;
        return BSP_OK;
    }
    return BSP_ERROR;
}

int bsp_uart_close(uint8_t uart_num)
{
    if (uart_num < 1 || uart_num > 6)
        return BSP_EINVAL;

    int idx = uart_num - 1;
    if (!uart_initialized[idx])
        return BSP_ENODEV;

    vet6_usart_disable(&uart_handles[idx]);
    vet6_usart_deinit(&uart_handles[idx]);
    uart_initialized[idx] = false;
    return BSP_OK;
}

int bsp_uart_write(uint8_t uart_num, const void *data, size_t len)
{
    if (uart_num < 1 || uart_num > 6)
        return BSP_EINVAL;

    int idx = uart_num - 1;
    if (!uart_initialized[idx])
        return BSP_ENODEV;

    return vet6_usart_send(&uart_handles[idx], (const uint8_t *)data, len);
}

int bsp_uart_read(uint8_t uart_num, void *buf, size_t len)
{
    if (uart_num < 1 || uart_num > 6)
        return BSP_EINVAL;

    int idx = uart_num - 1;
    if (!uart_initialized[idx])
        return BSP_ENODEV;

    return vet6_usart_receive(&uart_handles[idx], (uint8_t *)buf, len, 1000);
}

int bsp_uart_putc(uint8_t uart_num, char c)
{
    if (uart_num < 1 || uart_num > 6)
        return BSP_EINVAL;

    int idx = uart_num - 1;
    if (!uart_initialized[idx])
        return BSP_ENODEV;

    vet6_usart_send_byte(&uart_handles[idx], (uint8_t)c);
    return BSP_OK;
}

int bsp_uart_getc(uint8_t uart_num)
{
    if (uart_num < 1 || uart_num > 6)
        return BSP_EINVAL;

    int idx = uart_num - 1;
    if (!uart_initialized[idx])
        return BSP_ENODEV;

    return vet6_usart_receive_byte(&uart_handles[idx], 1000);
}

int bsp_uart_printf(uint8_t uart_num, const char *fmt, ...)
{
    if (uart_num < 1 || uart_num > 6)
        return BSP_EINVAL;

    int idx = uart_num - 1;
    if (!uart_initialized[idx])
        return BSP_ENODEV;

    va_list args;
    va_start(args, fmt);
    int ret = vet6_usart_vprintf(&uart_handles[idx], fmt, args);
    va_end(args);
    return ret;
}

#endif

/*===========================================================================*/
/*                              I2C实现                                       */
/*===========================================================================*/

#if defined(CHIP_PACKAGE_VET6) || !defined(CHIP_PACKAGE)

/* I2C句柄数组 */
static vet6_i2c_handle_t i2c_handles[3];
static vet6_soft_i2c_handle_t soft_i2c_handles[3];
static bool i2c_initialized[3] = {false};
static bool i2c_use_hw[3] = {true, true, true};

int bsp_i2c_open(uint8_t bus_num, const bsp_i2c_config_t *config)
{
    if (bus_num < 1 || bus_num > 3)
        return BSP_EINVAL;

    int idx = bus_num - 1;
    if (i2c_initialized[idx])
        return BSP_EBUSY;

    bool use_hw = config ? config->use_hw : true;
    uint32_t speed = config ? config->speed : 100000;

    i2c_use_hw[idx] = use_hw;

    int ret;
    if (use_hw)
    {
        ret = vet6_i2c_init_simple(&i2c_handles[idx], (vet6_i2c_t)idx, speed);
    }
    else
    {
        /* 使用默认软件I2C引脚配置 */
        ret = vet6_soft_i2c_init_quick(&soft_i2c_handles[idx], VET6_PB8, VET6_PB9);
    }

    if (ret == 0)
    {
        i2c_initialized[idx] = true;
        return BSP_OK;
    }
    return BSP_ERROR;
}

int bsp_i2c_close(uint8_t bus_num)
{
    if (bus_num < 1 || bus_num > 3)
        return BSP_EINVAL;

    int idx = bus_num - 1;
    if (!i2c_initialized[idx])
        return BSP_ENODEV;

    if (i2c_use_hw[idx])
    {
        vet6_i2c_deinit(&i2c_handles[idx]);
    }
    i2c_initialized[idx] = false;
    return BSP_OK;
}

int bsp_i2c_write(uint8_t bus_num, uint8_t dev_addr, const void *data, size_t len)
{
    if (bus_num < 1 || bus_num > 3)
        return BSP_EINVAL;

    int idx = bus_num - 1;
    if (!i2c_initialized[idx])
        return BSP_ENODEV;

    if (i2c_use_hw[idx])
    {
        return vet6_i2c_write(&i2c_handles[idx], dev_addr, (const uint8_t *)data, len);
    }
    else
    {
        return vet6_soft_i2c_write(&soft_i2c_handles[idx], dev_addr, (const uint8_t *)data, len);
    }
}

int bsp_i2c_read(uint8_t bus_num, uint8_t dev_addr, void *buf, size_t len)
{
    if (bus_num < 1 || bus_num > 3)
        return BSP_EINVAL;

    int idx = bus_num - 1;
    if (!i2c_initialized[idx])
        return BSP_ENODEV;

    if (i2c_use_hw[idx])
    {
        return vet6_i2c_read(&i2c_handles[idx], dev_addr, (uint8_t *)buf, len);
    }
    else
    {
        return vet6_soft_i2c_read(&soft_i2c_handles[idx], dev_addr, (uint8_t *)buf, len);
    }
}

int bsp_i2c_mem_write(uint8_t bus_num, uint8_t dev_addr, uint8_t reg, const void *data, size_t len)
{
    if (bus_num < 1 || bus_num > 3)
        return BSP_EINVAL;

    int idx = bus_num - 1;
    if (!i2c_initialized[idx])
        return BSP_ENODEV;

    if (i2c_use_hw[idx])
    {
        return vet6_i2c_mem_write(&i2c_handles[idx], dev_addr, reg, (const uint8_t *)data, len);
    }
    else
    {
        return vet6_soft_i2c_mem_write(&soft_i2c_handles[idx], dev_addr, reg, 1, (const uint8_t *)data, len);
    }
}

int bsp_i2c_mem_read(uint8_t bus_num, uint8_t dev_addr, uint8_t reg, void *buf, size_t len)
{
    if (bus_num < 1 || bus_num > 3)
        return BSP_EINVAL;

    int idx = bus_num - 1;
    if (!i2c_initialized[idx])
        return BSP_ENODEV;

    if (i2c_use_hw[idx])
    {
        return vet6_i2c_mem_read(&i2c_handles[idx], dev_addr, reg, (uint8_t *)buf, len);
    }
    else
    {
        return vet6_soft_i2c_mem_read(&soft_i2c_handles[idx], dev_addr, reg, 1, (uint8_t *)buf, len);
    }
}

#endif

/*===========================================================================*/
/*                              SPI实现                                       */
/*===========================================================================*/

#if defined(CHIP_PACKAGE_VET6) || !defined(CHIP_PACKAGE)

/* SPI句柄数组 */
static vet6_spi_handle_t spi_handles[3];
static bool spi_initialized[3] = {false};

int bsp_spi_open(uint8_t spi_num, const bsp_spi_config_t *config)
{
    if (spi_num < 1 || spi_num > 3)
        return BSP_EINVAL;

    int idx = spi_num - 1;
    if (spi_initialized[idx])
        return BSP_EBUSY;

    vet6_spi_config_t spi_cfg = {
        .spi = (vet6_spi_t)idx,
        .mode = config ? config->mode : 0,
        .bits = (config && config->bits == 16) ? VET6_SPI_BITS_16 : VET6_SPI_BITS_8,
        .lsb_first = config ? config->lsb_first : false,
        .prescaler = VET6_SPI_PRESCALER_16, /* 默认分频 */
        .pin_map = 0};

    int ret = vet6_spi_init(&spi_handles[idx], &spi_cfg);
    if (ret == 0)
    {
        vet6_spi_enable(&spi_handles[idx]);
        spi_initialized[idx] = true;
        return BSP_OK;
    }
    return BSP_ERROR;
}

int bsp_spi_close(uint8_t spi_num)
{
    if (spi_num < 1 || spi_num > 3)
        return BSP_EINVAL;

    int idx = spi_num - 1;
    if (!spi_initialized[idx])
        return BSP_ENODEV;

    vet6_spi_disable(&spi_handles[idx]);
    vet6_spi_deinit(&spi_handles[idx]);
    spi_initialized[idx] = false;
    return BSP_OK;
}

int bsp_spi_transfer(uint8_t spi_num, const void *tx_buf, void *rx_buf, size_t len)
{
    if (spi_num < 1 || spi_num > 3)
        return BSP_EINVAL;

    int idx = spi_num - 1;
    if (!spi_initialized[idx])
        return BSP_ENODEV;

    return vet6_spi_transfer(&spi_handles[idx], (const uint8_t *)tx_buf, (uint8_t *)rx_buf, len);
}

int bsp_spi_write(uint8_t spi_num, const void *data, size_t len)
{
    if (spi_num < 1 || spi_num > 3)
        return BSP_EINVAL;

    int idx = spi_num - 1;
    if (!spi_initialized[idx])
        return BSP_ENODEV;

    return vet6_spi_send(&spi_handles[idx], (const uint8_t *)data, len);
}

int bsp_spi_read(uint8_t spi_num, void *buf, size_t len)
{
    if (spi_num < 1 || spi_num > 3)
        return BSP_EINVAL;

    int idx = spi_num - 1;
    if (!spi_initialized[idx])
        return BSP_ENODEV;

    return vet6_spi_receive(&spi_handles[idx], (uint8_t *)buf, len);
}

#endif

/*===========================================================================*/
/*                              ADC实现                                       */
/*===========================================================================*/

#if defined(CHIP_PACKAGE_VET6) || !defined(CHIP_PACKAGE)

/* ADC句柄数组 */
static vet6_adc_handle_t adc_handles[3];
static bool adc_initialized[3] = {false};

int bsp_adc_open(uint8_t adc_num, const bsp_adc_config_t *config)
{
    if (adc_num < 1 || adc_num > 3)
        return BSP_EINVAL;

    int idx = adc_num - 1;
    if (adc_initialized[idx])
        return BSP_EBUSY;

    vet6_adc_config_t adc_cfg = {
        .adc = (vet6_adc_t)idx,
        .resolution = VET6_ADC_RES_12BIT,
        .mode = VET6_ADC_MODE_SINGLE,
        .align_left = false,
        .use_dma = false};

    if (config)
    {
        switch (config->resolution)
        {
        case 6:
            adc_cfg.resolution = VET6_ADC_RES_6BIT;
            break;
        case 8:
            adc_cfg.resolution = VET6_ADC_RES_8BIT;
            break;
        case 10:
            adc_cfg.resolution = VET6_ADC_RES_10BIT;
            break;
        default:
            adc_cfg.resolution = VET6_ADC_RES_12BIT;
            break;
        }
    }

    int ret = vet6_adc_init(&adc_handles[idx], &adc_cfg);
    if (ret == 0)
    {
        adc_initialized[idx] = true;
        return BSP_OK;
    }
    return BSP_ERROR;
}

int bsp_adc_close(uint8_t adc_num)
{
    if (adc_num < 1 || adc_num > 3)
        return BSP_EINVAL;

    int idx = adc_num - 1;
    if (!adc_initialized[idx])
        return BSP_ENODEV;

    vet6_adc_deinit(&adc_handles[idx]);
    adc_initialized[idx] = false;
    return BSP_OK;
}

int bsp_adc_read(uint8_t adc_num, uint8_t channel)
{
    if (adc_num < 1 || adc_num > 3)
        return BSP_EINVAL;
    if (channel > 15)
        return BSP_EINVAL;

    int idx = adc_num - 1;
    if (!adc_initialized[idx])
        return BSP_ENODEV;

    return vet6_adc_read_channel(&adc_handles[idx], (vet6_adc_channel_t)channel);
}

int bsp_adc_read_voltage(uint8_t adc_num, uint8_t channel)
{
    int adc_value = bsp_adc_read(adc_num, channel);
    if (adc_value < 0)
        return adc_value;

    /* 12位ADC, 3.3V参考电压 */
    return (adc_value * 3300) / 4095;
}

int bsp_adc_read_temperature(void)
{
    /* 仅ADC1支持温度传感器 */
    if (!adc_initialized[0])
    {
        /* 自动初始化ADC1 */
        bsp_adc_open(1, NULL);
    }

    return vet6_adc_read_temperature(&adc_handles[0]);
}

#endif

/*===========================================================================*/
/*                              DAC实现                                       */
/*===========================================================================*/

#if defined(CHIP_PACKAGE_VET6) || !defined(CHIP_PACKAGE)

/* DAC句柄数组 */
static vet6_dac_handle_t dac_handles[2];
static bool dac_initialized[2] = {false};

int bsp_dac_open(uint8_t channel)
{
    if (channel < 1 || channel > 2)
        return BSP_EINVAL;

    int idx = channel - 1;
    if (dac_initialized[idx])
        return BSP_EBUSY;

    int ret = vet6_dac_init_simple(&dac_handles[idx], (vet6_dac_channel_t)idx);
    if (ret == 0)
    {
        vet6_dac_enable(&dac_handles[idx]);
        dac_initialized[idx] = true;
        return BSP_OK;
    }
    return BSP_ERROR;
}

int bsp_dac_close(uint8_t channel)
{
    if (channel < 1 || channel > 2)
        return BSP_EINVAL;

    int idx = channel - 1;
    if (!dac_initialized[idx])
        return BSP_ENODEV;

    vet6_dac_disable(&dac_handles[idx]);
    vet6_dac_deinit(&dac_handles[idx]);
    dac_initialized[idx] = false;
    return BSP_OK;
}

int bsp_dac_write(uint8_t channel, uint16_t value)
{
    if (channel < 1 || channel > 2)
        return BSP_EINVAL;

    int idx = channel - 1;
    if (!dac_initialized[idx])
        return BSP_ENODEV;

    vet6_dac_set_value(&dac_handles[idx], value);
    return BSP_OK;
}

int bsp_dac_write_voltage(uint8_t channel, uint16_t voltage_mv)
{
    if (channel < 1 || channel > 2)
        return BSP_EINVAL;

    int idx = channel - 1;
    if (!dac_initialized[idx])
        return BSP_ENODEV;

    vet6_dac_set_voltage(&dac_handles[idx], voltage_mv);
    return BSP_OK;
}

#endif

/*===========================================================================*/
/*                              定时器实现                                     */
/*===========================================================================*/

#if defined(CHIP_PACKAGE_VET6) || !defined(CHIP_PACKAGE)

/* 定时器句柄数组 */
static vet6_tim_handle_t tim_handles[14];
static bool tim_initialized[14] = {false};

int bsp_timer_init(uint8_t tim_num, uint32_t period_us, bsp_timer_callback_t callback, void *arg)
{
    if (tim_num < 1 || tim_num > 14)
        return BSP_EINVAL;

    int idx = tim_num - 1;
    if (tim_initialized[idx])
        return BSP_EBUSY;

    int ret = vet6_tim_init_us(&tim_handles[idx], (vet6_tim_t)idx, period_us);
    if (ret == 0)
    {
        /* TODO: 设置中断回调 */
        tim_initialized[idx] = true;
        return BSP_OK;
    }
    return BSP_ERROR;
}

int bsp_timer_start(uint8_t tim_num)
{
    if (tim_num < 1 || tim_num > 14)
        return BSP_EINVAL;

    int idx = tim_num - 1;
    if (!tim_initialized[idx])
        return BSP_ENODEV;

    vet6_tim_enable(&tim_handles[idx]);
    return BSP_OK;
}

int bsp_timer_stop(uint8_t tim_num)
{
    if (tim_num < 1 || tim_num > 14)
        return BSP_EINVAL;

    int idx = tim_num - 1;
    if (!tim_initialized[idx])
        return BSP_ENODEV;

    vet6_tim_disable(&tim_handles[idx]);
    return BSP_OK;
}

int bsp_pwm_init(uint8_t tim_num, uint8_t channel, uint32_t freq, uint16_t duty)
{
    if (tim_num < 1 || tim_num > 14)
        return BSP_EINVAL;
    if (channel < 1 || channel > 4)
        return BSP_EINVAL;

    int idx = tim_num - 1;

    /* 使用快速PWM初始化 */
    vet6_gpio_id_t pwm_pin = VET6_PA0; /* 默认引脚，实际应根据定时器和通道确定 */

    int ret = vet6_pwm_init_quick(&tim_handles[idx], (vet6_tim_t)idx, channel, pwm_pin, freq, duty);
    if (ret == 0)
    {
        tim_initialized[idx] = true;
        return BSP_OK;
    }
    return BSP_ERROR;
}

int bsp_pwm_set_duty(uint8_t tim_num, uint8_t channel, uint16_t duty)
{
    if (tim_num < 1 || tim_num > 14)
        return BSP_EINVAL;
    if (channel < 1 || channel > 4)
        return BSP_EINVAL;

    int idx = tim_num - 1;
    if (!tim_initialized[idx])
        return BSP_ENODEV;

    vet6_pwm_set_duty(&tim_handles[idx], channel, duty);
    return BSP_OK;
}

#endif

/*===========================================================================*/
/*                              延时实现                                       */
/*===========================================================================*/

void bsp_delay_us(uint32_t us)
{
    delay_us(us);
}

void bsp_delay_ms(uint32_t ms)
{
    delay_ms(ms);
}
