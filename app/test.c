/**
 * @file test_auto_init.c
 * @brief 测试文件：验证各个框架模块的自动初始化功能
 * @details 本文件用于验证各个框架模块的自动初始化是否正常执行
 * @date 2026-01-02
 */

#include "df_init.h"
#include "df_log.h"
#include <stdio.h>
#include <df_log.h>
#include <df_iic.h>
#include <sh1106/sh1106.h>
#include <device_hal.h>
#include <df_delay.h>
extern df_soft_iic_t i2c1_soft;

int i2c_writer_byte(uint8_t dev_addr, uint8_t reg_addr, uint8_t data){
    return Soft_IIC_Write_Byte(&i2c1_soft, dev_addr, reg_addr, data);
}

int i2c_reader_byte(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data){
    *data = Soft_IIC_Read_Byte(&i2c1_soft, dev_addr, reg_addr);
    return 0;
}

int i2c_reader_bytes(uint8_t dev_addr, uint8_t reg_addr, uint8_t len, uint8_t *buf){
    return Soft_IIC_Read_Len(&i2c1_soft, dev_addr, reg_addr, len, buf);
}

int i2c_writer_bytes(uint8_t dev_addr, uint8_t reg_addr, uint8_t len,
                                const uint8_t *buf){
    Soft_IIC_Write_Len(&i2c1_soft, dev_addr, reg_addr, len, (uint8_t *)buf);
    return 0;

}


device_i2c_hal_t test_i2c_bus = {
    .write_byte = i2c_writer_byte,
    .read_byte = i2c_reader_byte,
    .read_bytes = i2c_reader_bytes,
    .write_bytes = i2c_writer_bytes,
    .delay_ms = delay_ms,
    .delay_us = delay_us,
    .user_data = NULL,
    .initialized = false,
};

int __main(){

    Soft_IIC_Init(&i2c1_soft);

    SH1106_Init_HAL_I2C(NULL);

    while(1){
        log_flush();
    }
    return 0;
}
