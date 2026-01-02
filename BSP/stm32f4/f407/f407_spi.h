/**
 * @file f407_spi.h
 * @brief STM32F407系列通用SPI板级驱动头文件
 * @details 提供SPI1-SPI3的初始化和操作API
 *          适用于所有STM32F407封装型号 (VET6, ZET6, ZGT6等)
 *          - SPI1: APB2 (最高42MHz)
 *          - SPI2: APB1 (最高21MHz)
 *          - SPI3: APB1 (最高21MHz)
 *
 * 默认引脚配置:
 *   SPI1: PA5(SCK), PA6(MISO), PA7(MOSI) 或 PB3(SCK), PB4(MISO), PB5(MOSI)
 *   SPI2: PB13(SCK), PB14(MISO), PB15(MOSI) 或 PC2(MISO), PC3(MOSI)
 *   SPI3: PB3(SCK), PB4(MISO), PB5(MOSI) 或 PC10(SCK), PC11(MISO), PC12(MOSI)
 */

#ifndef __F407_SPI_H
#define __F407_SPI_H

#include <stm32f4xx.h>
#include <stdint.h>
#include <stdbool.h>
#include "f407_gpio.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /*===========================================================================*/
    /*                              类型定义                                      */
    /*===========================================================================*/

    /**
     * @brief SPI编号枚举
     */
    typedef enum
    {
        F407_SPI1 = 0,
        F407_SPI2,
        F407_SPI3,
        F407_SPI_MAX
    } f407_spi_t;

    /**
     * @brief SPI模式 (CPOL和CPHA组合)
     */
    typedef enum
    {
        F407_SPI_MODE0 = 0, // CPOL=0, CPHA=0
        F407_SPI_MODE1 = 1, // CPOL=0, CPHA=1
        F407_SPI_MODE2 = 2, // CPOL=1, CPHA=0
        F407_SPI_MODE3 = 3  // CPOL=1, CPHA=1
    } f407_spi_mode_t;

    /**
     * @brief SPI数据位宽
     */
    typedef enum
    {
        F407_SPI_DATASIZE_8BIT = 0,
        F407_SPI_DATASIZE_16BIT = 1
    } f407_spi_datasize_t;

    /**
     * @brief SPI波特率分频
     */
    typedef enum
    {
        F407_SPI_BAUDRATE_DIV2 = 0,
        F407_SPI_BAUDRATE_DIV4 = 1,
        F407_SPI_BAUDRATE_DIV8 = 2,
        F407_SPI_BAUDRATE_DIV16 = 3,
        F407_SPI_BAUDRATE_DIV32 = 4,
        F407_SPI_BAUDRATE_DIV64 = 5,
        F407_SPI_BAUDRATE_DIV128 = 6,
        F407_SPI_BAUDRATE_DIV256 = 7
    } f407_spi_baudrate_t;

    /**
     * @brief SPI主从模式
     */
    typedef enum
    {
        F407_SPI_SLAVE = 0,
        F407_SPI_MASTER = 1
    } f407_spi_role_t;

    /**
     * @brief SPI位序
     */
    typedef enum
    {
        F407_SPI_MSB_FIRST = 0, // 高位在前
        F407_SPI_LSB_FIRST = 1  // 低位在前
    } f407_spi_bitorder_t;

    /**
     * @brief SPI NSS管理
     */
    typedef enum
    {
        F407_SPI_NSS_HARD = 0, // 硬件NSS管理
        F407_SPI_NSS_SOFT = 1  // 软件NSS管理
    } f407_spi_nss_t;

    /**
     * @brief SPI引脚映射
     */
    typedef enum
    {
        F407_SPI1_PINS_PA5_PA6_PA7 = 0, // SPI1: PA5/PA6/PA7
        F407_SPI1_PINS_PB3_PB4_PB5,     // SPI1: PB3/PB4/PB5
        F407_SPI2_PINS_PB13_PB14_PB15,  // SPI2: PB13/PB14/PB15
        F407_SPI2_PINS_PB13_PC2_PC3,    // SPI2: PB13/PC2/PC3
        F407_SPI3_PINS_PB3_PB4_PB5,     // SPI3: PB3/PB4/PB5
        F407_SPI3_PINS_PC10_PC11_PC12   // SPI3: PC10/PC11/PC12
    } f407_spi_pinmap_t;

    /**
     * @brief SPI配置结构体
     */
    typedef struct
    {
        f407_spi_t spi;               // SPI编号
        f407_spi_mode_t mode;         // SPI模式
        f407_spi_datasize_t datasize; // 数据位宽
        f407_spi_baudrate_t baudrate; // 波特率分频
        f407_spi_role_t role;         // 主从模式
        f407_spi_bitorder_t bitorder; // 位序
        f407_spi_nss_t nss;           // NSS管理方式
        f407_spi_pinmap_t pinmap;     // 引脚映射
    } f407_spi_config_t;

    /**
     * @brief SPI句柄结构体
     */
    typedef struct
    {
        SPI_TypeDef *instance;    // SPI外设指针
        f407_spi_config_t config; // 配置信息
        bool initialized;         // 初始化标志
        f407_gpio_id_t cs_pin;    // 软件CS引脚
        bool cs_active_low;       // CS低电平有效
    } f407_spi_handle_t;

/*===========================================================================*/
/*                              宏定义                                        */
/*===========================================================================*/

/* SPI时钟频率 */
#define F407_SPI1_CLK 84000000UL // APB2时钟
#define F407_SPI2_CLK 42000000UL // APB1时钟
#define F407_SPI3_CLK 42000000UL // APB1时钟

/* 超时时间 */
#define F407_SPI_TIMEOUT 1000

    /*===========================================================================*/
    /*                              初始化API                                     */
    /*===========================================================================*/

    /**
     * @brief 使能SPI时钟
     * @param spi SPI编号
     */
    void f407_spi_clk_enable(f407_spi_t spi);

    /**
     * @brief 禁用SPI时钟
     * @param spi SPI编号
     */
    void f407_spi_clk_disable(f407_spi_t spi);

    /**
     * @brief 获取SPI外设基地址
     * @param spi SPI编号
     * @return SPI外设指针
     */
    SPI_TypeDef *f407_spi_get_base(f407_spi_t spi);

    /**
     * @brief 初始化SPI引脚
     * @param pinmap 引脚映射
     * @return 0:成功, -1:失败
     */
    int f407_spi_pins_init(f407_spi_pinmap_t pinmap);

    /**
     * @brief 初始化SPI
     * @param handle SPI句柄指针
     * @param config 配置结构体指针
     * @return 0:成功, -1:失败
     */
    int f407_spi_init(f407_spi_handle_t *handle, const f407_spi_config_t *config);

    /**
     * @brief 快速初始化SPI为主机模式
     * @param handle SPI句柄指针
     * @param spi SPI编号
     * @param baudrate 波特率分频
     * @param pinmap 引脚映射
     * @return 0:成功, -1:失败
     */
    int f407_spi_init_master(f407_spi_handle_t *handle, f407_spi_t spi,
                             f407_spi_baudrate_t baudrate, f407_spi_pinmap_t pinmap);

    /**
     * @brief 反初始化SPI
     * @param handle SPI句柄指针
     */
    void f407_spi_deinit(f407_spi_handle_t *handle);

    /*===========================================================================*/
    /*                              基本操作API                                   */
    /*===========================================================================*/

    /**
     * @brief 使能SPI
     * @param handle SPI句柄指针
     */
    void f407_spi_enable(f407_spi_handle_t *handle);

    /**
     * @brief 禁用SPI
     * @param handle SPI句柄指针
     */
    void f407_spi_disable(f407_spi_handle_t *handle);

    /**
     * @brief 设置软件CS引脚
     * @param handle SPI句柄指针
     * @param cs_pin CS引脚
     * @param active_low 是否低电平有效
     */
    void f407_spi_set_cs_pin(f407_spi_handle_t *handle, f407_gpio_id_t cs_pin, bool active_low);

    /**
     * @brief 选中从设备 (CS有效)
     * @param handle SPI句柄指针
     */
    void f407_spi_cs_select(f407_spi_handle_t *handle);

    /**
     * @brief 取消选中从设备 (CS无效)
     * @param handle SPI句柄指针
     */
    void f407_spi_cs_deselect(f407_spi_handle_t *handle);

    /*===========================================================================*/
    /*                              数据传输API                                   */
    /*===========================================================================*/

    /**
     * @brief 发送单字节
     * @param handle SPI句柄指针
     * @param data 发送数据
     * @return 0:成功, -1:失败
     */
    int f407_spi_send_byte(f407_spi_handle_t *handle, uint8_t data);

    /**
     * @brief 接收单字节
     * @param handle SPI句柄指针
     * @param data 接收数据指针
     * @return 0:成功, -1:失败
     */
    int f407_spi_receive_byte(f407_spi_handle_t *handle, uint8_t *data);

    /**
     * @brief 发送并接收单字节
     * @param handle SPI句柄指针
     * @param tx_data 发送数据
     * @return 接收到的数据
     */
    uint8_t f407_spi_transfer_byte(f407_spi_handle_t *handle, uint8_t tx_data);

    /**
     * @brief 发送数据缓冲区
     * @param handle SPI句柄指针
     * @param data 发送数据指针
     * @param len 数据长度
     * @return 0:成功, -1:失败
     */
    int f407_spi_send(f407_spi_handle_t *handle, const uint8_t *data, uint16_t len);

    /**
     * @brief 接收数据到缓冲区
     * @param handle SPI句柄指针
     * @param data 接收数据指针
     * @param len 数据长度
     * @return 0:成功, -1:失败
     */
    int f407_spi_receive(f407_spi_handle_t *handle, uint8_t *data, uint16_t len);

    /**
     * @brief 全双工传输
     * @param handle SPI句柄指针
     * @param tx_data 发送数据指针
     * @param rx_data 接收数据指针
     * @param len 数据长度
     * @return 0:成功, -1:失败
     */
    int f407_spi_transfer(f407_spi_handle_t *handle, const uint8_t *tx_data,
                          uint8_t *rx_data, uint16_t len);

    /**
     * @brief 发送16位数据
     * @param handle SPI句柄指针
     * @param data 发送数据
     * @return 0:成功, -1:失败
     */
    int f407_spi_send_16bit(f407_spi_handle_t *handle, uint16_t data);

    /**
     * @brief 发送并接收16位数据
     * @param handle SPI句柄指针
     * @param tx_data 发送数据
     * @return 接收到的数据
     */
    uint16_t f407_spi_transfer_16bit(f407_spi_handle_t *handle, uint16_t tx_data);

    /*===========================================================================*/
    /*                              辅助功能API                                   */
    /*===========================================================================*/

    /**
     * @brief 获取SPI波特率
     * @param handle SPI句柄指针
     * @return 波特率(Hz)
     */
    uint32_t f407_spi_get_baudrate(f407_spi_handle_t *handle);

    /**
     * @brief 设置SPI波特率
     * @param handle SPI句柄指针
     * @param baudrate 波特率分频
     */
    void f407_spi_set_baudrate(f407_spi_handle_t *handle, f407_spi_baudrate_t baudrate);

    /**
     * @brief 检查SPI是否忙
     * @param handle SPI句柄指针
     * @return true:忙, false:空闲
     */
    bool f407_spi_is_busy(f407_spi_handle_t *handle);

    /**
     * @brief 等待传输完成
     * @param handle SPI句柄指针
     * @param timeout 超时时间(ms)
     * @return 0:成功, -1:超时
     */
    int f407_spi_wait_complete(f407_spi_handle_t *handle, uint32_t timeout);

    /**
     * @brief 清除SPI所有错误标志
     * @param handle SPI句柄指针
     */
    void f407_spi_clear_errors(f407_spi_handle_t *handle);

#ifdef __cplusplus
}
#endif

#endif /* __F407_SPI_H */
