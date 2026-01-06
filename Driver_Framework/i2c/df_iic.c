#include "df_iic.h"
#include "df_init.h"

/*===========================================================================*/
/*                         软件IIC非内联函数实现                              */
/*===========================================================================*/

/**
 * @brief 初始化IIC（只调用一次，不需要内联）
 */
void Soft_IIC_Init(df_soft_iic_t *i2c)
{
    if (i2c->gpio_init != NULL)
    {
        i2c->gpio_init();
    }
    i2c->scl(1);
    i2c->sda(1);
    i2c->init_flag = true;
}

/**
 * @brief IIC写一个字节到指定寄存器
 */
uint8_t Soft_IIC_Write_Byte(df_soft_iic_t *i2c, uint8_t addr, uint8_t reg,
                            uint8_t data)
{
    Soft_IIC_Start(i2c);
    Soft_IIC_Send_Byte(i2c, addr | 0);
    if (Soft_IIC_Wait_Ack(i2c))
    {
        Soft_IIC_Stop(i2c);
        return 1;
    }
    Soft_IIC_Send_Byte(i2c, reg);
    Soft_IIC_Wait_Ack(i2c);
    Soft_IIC_Send_Byte(i2c, data);
    if (Soft_IIC_Wait_Ack(i2c))
    {
        Soft_IIC_Stop(i2c);
        return 1;
    }
    Soft_IIC_Stop(i2c);
    return 0;
}

/**
 * @brief IIC从指定寄存器读一个字节
 */
uint8_t Soft_IIC_Read_Byte(df_soft_iic_t *i2c, uint8_t addr, uint8_t reg)
{
    uint8_t res;
    Soft_IIC_Start(i2c);
    Soft_IIC_Send_Byte(i2c, addr | 0);
    Soft_IIC_Wait_Ack(i2c);
    Soft_IIC_Send_Byte(i2c, reg);
    Soft_IIC_Wait_Ack(i2c);
    Soft_IIC_Start(i2c);
    Soft_IIC_Send_Byte(i2c, addr | 1);
    Soft_IIC_Wait_Ack(i2c);
    res = Soft_IIC_Receive_Byte(i2c, 0);
    Soft_IIC_Stop(i2c);
    return res;
}

/**
 * @brief IIC连续写多个字节
 */
uint8_t Soft_IIC_Write_Len(df_soft_iic_t *i2c, uint8_t addr, uint8_t reg,
                           uint8_t len, uint8_t *buf)
{
    Soft_IIC_Start(i2c);
    Soft_IIC_Send_Byte(i2c, addr | 0);
    if (Soft_IIC_Wait_Ack(i2c))
    {
        Soft_IIC_Stop(i2c);
        return 1;
    }
    Soft_IIC_Send_Byte(i2c, reg);
    Soft_IIC_Wait_Ack(i2c);
    for (uint8_t i = 0; i < len; i++)
    {
        Soft_IIC_Send_Byte(i2c, buf[i]);
        if (Soft_IIC_Wait_Ack(i2c))
        {
            Soft_IIC_Stop(i2c);
            return 1;
        }
    }
    Soft_IIC_Stop(i2c);
    return 0;
}

/**
 * @brief IIC连续读多个字节
 */
uint8_t Soft_IIC_Read_Len(df_soft_iic_t *i2c, uint8_t addr, uint8_t reg, uint8_t len,
                          uint8_t *buf)
{
    Soft_IIC_Start(i2c);
    Soft_IIC_Send_Byte(i2c, addr | 0);
    if (Soft_IIC_Wait_Ack(i2c))
    {
        Soft_IIC_Stop(i2c);
        return 1;
    }
    Soft_IIC_Send_Byte(i2c, reg);
    Soft_IIC_Wait_Ack(i2c);
    Soft_IIC_Start(i2c);
    Soft_IIC_Send_Byte(i2c, addr | 1);
    Soft_IIC_Wait_Ack(i2c);
    while (len)
    {
        if (len == 1)
            *buf = Soft_IIC_Receive_Byte(i2c, 0);
        else
            *buf = Soft_IIC_Receive_Byte(i2c, 1);
        len--;
        buf++;
    }
    Soft_IIC_Stop(i2c);
    return 0;
}

/**
 * @brief 检测IIC设备是否存在
 */
uint8_t Soft_IIC_Check(df_soft_iic_t *i2c, uint8_t addr)
{
    Soft_IIC_Start(i2c);
    Soft_IIC_Send_Byte(i2c, addr | 0);
    if (Soft_IIC_Wait_Ack(i2c))
    {
        Soft_IIC_Stop(i2c);
        return 1;
    }
    Soft_IIC_Stop(i2c);
    return 0;
}
