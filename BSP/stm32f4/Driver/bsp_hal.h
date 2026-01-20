/**
 * @file bsp_hal.h
 * @brief BSP硬件抽象层 - POSIX风格接口定义
 * @details 定义统一的POSIX风格接口，作为板级底层驱动和Driver_Framework之间的桥接层
 *          支持不同封装型号(vet6, vgt6等)的驱动适配
 *
 * 架构层次:
 *   Driver_Framework (df_xxx.h)  <- 应用层框架接口
 *          |
 *   BSP/Driver/xxx.c             <- 本层: 桥接层/适配层
 *          |
 *   BSP/Driver/vet6/vet6_xxx.c   <- 板级底层驱动
 */

#ifndef __BSP_HAL_H
#define __BSP_HAL_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /*===========================================================================*/
    /*                              POSIX风格错误码                               */
    /*===========================================================================*/

#define BSP_OK 0          /**< 操作成功 */
#define BSP_ERROR (-1)    /**< 通用错误 */
#define BSP_ENODEV (-2)   /**< 设备不存在 */
#define BSP_EBUSY (-3)    /**< 设备忙 */
#define BSP_EINVAL (-4)   /**< 无效参数 */
#define BSP_ENOMEM (-5)   /**< 内存不足 */
#define BSP_ETIMEOUT (-6) /**< 超时 */
#define BSP_ENOTSUP (-7)  /**< 不支持的操作 */
#define BSP_EIO (-8)      /**< I/O错误 */

/*===========================================================================*/
/*                              封装型号选择                                   */
/*===========================================================================*/

/* 根据编译宏选择对应的板级驱动 */
#if defined(CHIP_PACKAGE_VET6)
#include "vet6/vet6_bsp.h"
#define BSP_PACKAGE_NAME "VET6"
#elif defined(CHIP_PACKAGE_VGT6)
#include "vgt6/vgt6_bsp.h"
#define BSP_PACKAGE_NAME "VGT6"
#elif defined(CHIP_PACKAGE_ZET6)
#include "zet6/zet6_bsp.h"
#define BSP_PACKAGE_NAME "ZET6"
#elif defined(CHIP_PACKAGE_ZGT6)
#include "zgt6/zgt6_bsp.h"
#define BSP_PACKAGE_NAME "ZGT6"
#else
/* 默认使用VET6 */
#include "vet6/vet6_bsp.h"
#define BSP_PACKAGE_NAME "VET6"
#warning "No CHIP_PACKAGE defined, using VET6 as default"
#endif

    /*===========================================================================*/
    /*                              GPIO接口 (POSIX风格)                          */
    /*===========================================================================*/

    /**
     * @brief GPIO方向枚举
     */
    typedef enum
    {
        BSP_GPIO_DIR_INPUT = 0, /**< 输入模式 */
        BSP_GPIO_DIR_OUTPUT = 1 /**< 输出模式 */
    } bsp_gpio_dir_t;

    /**
     * @brief GPIO上下拉模式
     */
    typedef enum
    {
        BSP_GPIO_PULL_NONE = 0, /**< 无上下拉 */
        BSP_GPIO_PULL_UP = 1,   /**< 上拉 */
        BSP_GPIO_PULL_DOWN = 2  /**< 下拉 */
    } bsp_gpio_pull_t;

    /**
     * @brief 配置GPIO引脚
     * @param port 端口号 (0=A, 1=B, ...)
     * @param pin  引脚号 (0-15)
     * @param dir  方向
     * @param pull 上下拉模式
     * @return BSP_OK成功, <0失败
     */
    int bsp_gpio_config(uint8_t port, uint8_t pin, bsp_gpio_dir_t dir, bsp_gpio_pull_t pull);

    /**
     * @brief 设置GPIO输出高电平
     */
    int bsp_gpio_set(uint8_t port, uint8_t pin);

    /**
     * @brief 设置GPIO输出低电平
     */
    int bsp_gpio_reset(uint8_t port, uint8_t pin);

    /**
     * @brief 翻转GPIO输出
     */
    int bsp_gpio_toggle(uint8_t port, uint8_t pin);

    /**
     * @brief 读取GPIO电平
     * @return 0低电平, 1高电平, <0错误
     */
    int bsp_gpio_read(uint8_t port, uint8_t pin);

    /**
     * @brief 写入GPIO电平
     */
    int bsp_gpio_write(uint8_t port, uint8_t pin, uint8_t value);

    /*===========================================================================*/
    /*                              UART接口 (POSIX风格)                          */
    /*===========================================================================*/

    /**
     * @brief UART配置结构体
     */
    typedef struct
    {
        uint32_t baudrate; /**< 波特率 */
        uint8_t databits;  /**< 数据位 (7, 8, 9) */
        uint8_t stopbits;  /**< 停止位 (1, 2) */
        uint8_t parity;    /**< 校验 (0=无, 1=奇, 2=偶) */
        bool flow_ctrl;    /**< 硬件流控 */
    } bsp_uart_config_t;

    /**
     * @brief 打开UART设备
     * @param uart_num UART编号 (1-6)
     * @param config   配置参数
     * @return BSP_OK成功, <0失败
     */
    int bsp_uart_open(uint8_t uart_num, const bsp_uart_config_t *config);

    /**
     * @brief 关闭UART设备
     */
    int bsp_uart_close(uint8_t uart_num);

    /**
     * @brief 发送数据 (阻塞)
     * @param uart_num UART编号
     * @param data     数据指针
     * @param len      数据长度
     * @return 实际发送字节数, <0错误
     */
    int bsp_uart_write(uint8_t uart_num, const void *data, size_t len);

    /**
     * @brief 接收数据 (阻塞)
     * @param uart_num UART编号
     * @param buf      接收缓冲区
     * @param len      期望接收长度
     * @return 实际接收字节数, <0错误
     */
    int bsp_uart_read(uint8_t uart_num, void *buf, size_t len);

    /**
     * @brief 发送单个字符
     */
    int bsp_uart_putc(uint8_t uart_num, char c);

    /**
     * @brief 接收单个字符
     */
    int bsp_uart_getc(uint8_t uart_num);

    /**
     * @brief 格式化输出
     */
    int bsp_uart_printf(uint8_t uart_num, const char *fmt, ...);

    /*===========================================================================*/
    /*                              I2C接口 (POSIX风格)                           */
    /*===========================================================================*/

    /**
     * @brief I2C配置结构体
     */
    typedef struct
    {
        uint32_t speed; /**< I2C速度 (Hz) */
        bool use_hw;    /**< 使用硬件I2C */
    } bsp_i2c_config_t;

    /**
     * @brief 打开I2C总线
     * @param bus_num I2C总线编号 (1-3)
     * @param config  配置参数
     * @return BSP_OK成功, <0失败
     */
    int bsp_i2c_open(uint8_t bus_num, const bsp_i2c_config_t *config);

    /**
     * @brief 关闭I2C总线
     */
    int bsp_i2c_close(uint8_t bus_num);

    /**
     * @brief I2C写入数据
     * @param bus_num  总线编号
     * @param dev_addr 设备地址 (7位)
     * @param data     数据指针
     * @param len      数据长度
     * @return 实际写入字节数, <0错误
     */
    int bsp_i2c_write(uint8_t bus_num, uint8_t dev_addr, const void *data, size_t len);

    /**
     * @brief I2C读取数据
     * @param bus_num  总线编号
     * @param dev_addr 设备地址 (7位)
     * @param buf      接收缓冲区
     * @param len      期望读取长度
     * @return 实际读取字节数, <0错误
     */
    int bsp_i2c_read(uint8_t bus_num, uint8_t dev_addr, void *buf, size_t len);

    /**
     * @brief I2C寄存器写入
     */
    int bsp_i2c_mem_write(uint8_t bus_num, uint8_t dev_addr, uint8_t reg, const void *data, size_t len);

    /**
     * @brief I2C寄存器读取
     */
    int bsp_i2c_mem_read(uint8_t bus_num, uint8_t dev_addr, uint8_t reg, void *buf, size_t len);

    /*===========================================================================*/
    /*                              SPI接口 (POSIX风格)                           */
    /*===========================================================================*/

    /**
     * @brief SPI配置结构体
     */
    typedef struct
    {
        uint32_t speed; /**< SPI速度 (Hz) */
        uint8_t mode;   /**< SPI模式 (0-3) */
        uint8_t bits;   /**< 数据位宽 (8, 16) */
        bool lsb_first; /**< LSB优先 */
    } bsp_spi_config_t;

    /**
     * @brief 打开SPI设备
     */
    int bsp_spi_open(uint8_t spi_num, const bsp_spi_config_t *config);

    /**
     * @brief 关闭SPI设备
     */
    int bsp_spi_close(uint8_t spi_num);

    /**
     * @brief SPI传输 (全双工)
     */
    int bsp_spi_transfer(uint8_t spi_num, const void *tx_buf, void *rx_buf, size_t len);

    /**
     * @brief SPI写入
     */
    int bsp_spi_write(uint8_t spi_num, const void *data, size_t len);

    /**
     * @brief SPI读取
     */
    int bsp_spi_read(uint8_t spi_num, void *buf, size_t len);

    /*===========================================================================*/
    /*                              ADC接口 (POSIX风格)                           */
    /*===========================================================================*/

    /**
     * @brief ADC配置结构体
     */
    typedef struct
    {
        uint8_t resolution;  /**< 分辨率 (6, 8, 10, 12) */
        uint8_t sample_time; /**< 采样时间选择 */
    } bsp_adc_config_t;

    /**
     * @brief 打开ADC设备
     */
    int bsp_adc_open(uint8_t adc_num, const bsp_adc_config_t *config);

    /**
     * @brief 关闭ADC设备
     */
    int bsp_adc_close(uint8_t adc_num);

    /**
     * @brief 读取ADC通道值
     * @param adc_num ADC编号 (1-3)
     * @param channel 通道号 (0-15)
     * @return ADC值, <0错误
     */
    int bsp_adc_read(uint8_t adc_num, uint8_t channel);

    /**
     * @brief 读取ADC并转换为电压 (mV)
     */
    int bsp_adc_read_voltage(uint8_t adc_num, uint8_t channel);

    /**
     * @brief 读取内部温度传感器
     * @return 温度值 (0.1°C为单位), <0错误
     */
    int bsp_adc_read_temperature(void);

    /*===========================================================================*/
    /*                              DAC接口 (POSIX风格)                           */
    /*===========================================================================*/

    /**
     * @brief 打开DAC通道
     */
    int bsp_dac_open(uint8_t channel);

    /**
     * @brief 关闭DAC通道
     */
    int bsp_dac_close(uint8_t channel);

    /**
     * @brief 设置DAC输出值
     * @param channel 通道号 (1, 2)
     * @param value   输出值 (0-4095)
     * @return BSP_OK成功, <0失败
     */
    int bsp_dac_write(uint8_t channel, uint16_t value);

    /**
     * @brief 设置DAC输出电压 (mV)
     */
    int bsp_dac_write_voltage(uint8_t channel, uint16_t voltage_mv);

    /*===========================================================================*/
    /*                              定时器接口                                     */
    /*===========================================================================*/

    /**
     * @brief 定时器回调函数类型
     */
    typedef void (*bsp_timer_callback_t)(void *arg);

    /**
     * @brief 初始化定时器 (周期中断)
     * @param tim_num  定时器编号
     * @param period_us 周期 (微秒)
     * @param callback 中断回调函数
     * @param arg      回调参数
     * @return BSP_OK成功, <0失败
     */
    int bsp_timer_init(uint8_t tim_num, uint32_t period_us, bsp_timer_callback_t callback, void *arg);

    /**
     * @brief 启动定时器
     */
    int bsp_timer_start(uint8_t tim_num);

    /**
     * @brief 停止定时器
     */
    int bsp_timer_stop(uint8_t tim_num);

    /**
     * @brief 初始化PWM输出
     * @param tim_num  定时器编号
     * @param channel  通道号 (1-4)
     * @param freq     频率 (Hz)
     * @param duty     占空比 (0-10000 = 0%-100%)
     * @return BSP_OK成功, <0失败
     */
    int bsp_pwm_init(uint8_t tim_num, uint8_t channel, uint32_t freq, uint16_t duty);

    /**
     * @brief 设置PWM占空比
     */
    int bsp_pwm_set_duty(uint8_t tim_num, uint8_t channel, uint16_t duty);

    /*===========================================================================*/
    /*                              延时接口                                       */
    /*===========================================================================*/

    /**
     * @brief 微秒延时
     */
    void bsp_delay_us(uint32_t us);

    /**
     * @brief 毫秒延时
     */
    void bsp_delay_ms(uint32_t ms);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_HAL_H */
