#include "df_iic.h"
#include "df_init.h"

// 初始化IIC
void Soft_IIC_Init(SIAS *i2c_dev)
{
    if (i2c_dev->Soft_IIC_GPIO_Port_Init != NULL)
    {
        i2c_dev->Soft_IIC_GPIO_Port_Init(); // 初始化IIC的GPIO端口
    }
    i2c_dev->Soft_IIC_SCL(1);
    i2c_dev->Soft_IIC_SDA(1);
    i2c_dev->soft_iic_init_flag = true;
}

// 产生IIC起始信号
void Soft_IIC_Start(SIAS *i2c_dev)
{
    i2c_dev->Soft_SDA_OUT(); // sda线输出
    i2c_dev->Soft_IIC_SDA(1);
    i2c_dev->Soft_IIC_SCL(1);
    // i2c_dev->delay_us(4);
    i2c_dev->Soft_IIC_SDA(
        0);                   // START:when CLK is high,DATA change form high to low
                              //    i2c_dev->delay_us(1);
    i2c_dev->Soft_IIC_SCL(0); // 钳住I2C总线，准备发送或接收数据
}
// 产生IIC停止信号
void Soft_IIC_Stop(SIAS *i2c_dev)
{
    i2c_dev->Soft_SDA_OUT(); // sda线输出
    i2c_dev->Soft_IIC_SCL(0);
    i2c_dev->Soft_IIC_SDA(
        0); // STOP:when CLK is high DATA change form low to high
    // delay_us(4);
    i2c_dev->Soft_IIC_SCL(1);
    i2c_dev->Soft_IIC_SDA(1); // 发送I2C总线结束信号
    // delay_us(4);
}
// 等待应答信号到来
// 返回值：1，接收应答失败
//         0，接收应答成功
uint8_t Soft_IIC_Wait_Ack(SIAS *i2c_dev)
{
    uint8_t ucErrTime = 0;
    i2c_dev->Soft_SDA_IN(); // SDA设置为输入
    i2c_dev->Soft_IIC_SDA(1);
    //    i2c_dev->delay_us(1);
    i2c_dev->Soft_IIC_SCL(1);
    //    i2c_dev->delay_us(1);
    while (i2c_dev->Soft_READ_SDA())
    {
        ucErrTime++;
        if (ucErrTime > 250)
        {
            Soft_IIC_Stop(i2c_dev);
            return 1;
        }
    }
    i2c_dev->Soft_IIC_SCL(0); // 时钟输出0
    return 0;
}
// 产生ACK应答
void Soft_IIC_Ack(SIAS *i2c_dev)
{
    i2c_dev->Soft_IIC_SCL(0);
    i2c_dev->Soft_SDA_OUT();
    i2c_dev->Soft_IIC_SDA(0);
    //    i2c_dev->delay_us(1);
    i2c_dev->Soft_IIC_SCL(1);
    // i2c_dev->delay_us(2);
    i2c_dev->Soft_IIC_SCL(0);
}
// 不产生ACK应答
void Soft_IIC_NAck(SIAS *i2c_dev)
{
    i2c_dev->Soft_IIC_SCL(0);
    i2c_dev->Soft_SDA_OUT();
    i2c_dev->Soft_IIC_SDA(1);
    //    i2c_dev->delay_us(1);
    i2c_dev->Soft_IIC_SCL(1);
    // delay_us(2);
    i2c_dev->Soft_IIC_SCL(0);
}
// IIC发送一个字节
// 返回从机有无应答
// 1，有应答
// 0，无应答
void Soft_IIC_Send_Byte(SIAS *i2c_dev, uint8_t txd)
{
    uint8_t t;
    i2c_dev->Soft_SDA_OUT();
    i2c_dev->Soft_IIC_SCL(0); // 拉低时钟开始数据传输
    for (t = 0; t < 8; t++)
    {
        i2c_dev->Soft_IIC_SDA((txd & 0x80) >> 7);
        txd <<= 1;
        //    i2c_dev->delay_us(1);
        i2c_dev->Soft_IIC_SCL(1);
        //    i2c_dev->delay_us(1);
        i2c_dev->Soft_IIC_SCL(0);
        //    i2c_dev->delay_us(1);
    }
}
// 读1个字节，ack=1时，发送ACK，ack=0，发送nACK
uint8_t Soft_IIC_Receive_Byte(SIAS *i2c_dev, unsigned char ack)
{
    unsigned char i, receive = 0;
    i2c_dev->Soft_SDA_IN(); // SDA设置为输入
    for (i = 0; i < 8; i++)
    {
        i2c_dev->Soft_IIC_SCL(0);
        //    i2c_dev->delay_us(1);
        i2c_dev->Soft_IIC_SCL(1);
        receive <<= 1;
        if (i2c_dev->Soft_READ_SDA())
            receive++;
        //    i2c_dev->delay_us(1);
    }
    if (!ack)
        Soft_IIC_NAck(i2c_dev); // 发送nACK
    else
        Soft_IIC_Ack(i2c_dev); // 发送ACK
    return receive;
}

uint8_t Soft_IIC_Write_Byte(SIAS *i2c_dev, uint8_t addr, uint8_t reg,
                            uint8_t data)
{
    Soft_IIC_Start(i2c_dev);
    Soft_IIC_Send_Byte(i2c_dev, addr | 0); // 发送器件地址+写命令
    if (Soft_IIC_Wait_Ack(i2c_dev))        // 等待应答
    {
        Soft_IIC_Stop(i2c_dev);
        return 1;
    }
    Soft_IIC_Send_Byte(i2c_dev, reg);  // 写寄存器地址
    Soft_IIC_Wait_Ack(i2c_dev);        // 等待应答
    Soft_IIC_Send_Byte(i2c_dev, data); // 发送数据
    if (Soft_IIC_Wait_Ack(i2c_dev))    // 等待ACK
    {
        Soft_IIC_Stop(i2c_dev);
        return 1;
    }
    Soft_IIC_Stop(i2c_dev);
    return 0;
}

uint8_t Soft_IIC_Read_Byte(SIAS *i2c_dev, uint8_t addr, uint8_t reg)
{
    uint8_t res;
    Soft_IIC_Start(i2c_dev);
    Soft_IIC_Send_Byte(i2c_dev, addr | 0); // 发送器件地址+写命令
    Soft_IIC_Wait_Ack(i2c_dev);            // 等待应答
    Soft_IIC_Send_Byte(i2c_dev, reg);      // 写寄存器地址
    Soft_IIC_Wait_Ack(i2c_dev);            // 等待应答
    Soft_IIC_Start(i2c_dev);
    Soft_IIC_Send_Byte(i2c_dev, addr | 1);   // 发送器件地址+读命令
    Soft_IIC_Wait_Ack(i2c_dev);              // 等待应答
    res = Soft_IIC_Receive_Byte(i2c_dev, 0); // 读取数据,发送nACK
    Soft_IIC_Stop(i2c_dev);                  // 产生一个停止条件
    return res;
}

uint8_t Soft_IIC_Write_Len(SIAS *i2c_dev, uint8_t addr, uint8_t reg,
                           uint8_t len, uint8_t *buf)
{
    uint8_t i;
    Soft_IIC_Start(i2c_dev);
    Soft_IIC_Send_Byte(i2c_dev, addr | 0); // 发送器件地址+写命令
    if (Soft_IIC_Wait_Ack(i2c_dev))        // 等待应答
    {
        Soft_IIC_Stop(i2c_dev);
        return 1;
    }
    Soft_IIC_Send_Byte(i2c_dev, reg); // 写寄存器地址
    Soft_IIC_Wait_Ack(i2c_dev);       // 等待应答
    for (i = 0; i < len; i++)
    {
        Soft_IIC_Send_Byte(i2c_dev, buf[i]); // 发送数据
        if (Soft_IIC_Wait_Ack(i2c_dev))      // 等待ACK
        {
            Soft_IIC_Stop(i2c_dev);
            return 1;
        }
    }
    Soft_IIC_Stop(i2c_dev);
    return 0;
}

uint8_t Soft_IIC_Read_Len(SIAS *i2c_dev, uint8_t addr, uint8_t reg, uint8_t len,
                          uint8_t *buf)
{
    Soft_IIC_Start(i2c_dev);
    Soft_IIC_Send_Byte(i2c_dev, addr | 0); // 发送器件地址+写命令
    if (Soft_IIC_Wait_Ack(i2c_dev))        // 等待应答
    {
        // printf("未检测到IIC设备");
        Soft_IIC_Stop(i2c_dev);
        return 1;
    }
    Soft_IIC_Send_Byte(i2c_dev, reg); // 写寄存器地址
    Soft_IIC_Wait_Ack(i2c_dev);       // 等待应答
    Soft_IIC_Start(i2c_dev);
    Soft_IIC_Send_Byte(i2c_dev, addr | 1); // 发送器件地址+读命令
    Soft_IIC_Wait_Ack(i2c_dev);            // 等待应答
    while (len)
    {
        if (len == 1)
            *buf = Soft_IIC_Receive_Byte(i2c_dev, 0); // 读数据,发送nACK
        else
            *buf = Soft_IIC_Receive_Byte(i2c_dev, 1); // 读数据,发送ACK
        len--;
        buf++;
    }
    Soft_IIC_Stop(i2c_dev); // 产生一个停止条件
    return 0;
}

uint8_t Soft_IIC_Cheak(SIAS *i2c_dev, uint8_t addr)
{
    Soft_IIC_Start(i2c_dev);
    Soft_IIC_Send_Byte(i2c_dev, addr | 0);
    if (Soft_IIC_Wait_Ack(i2c_dev)) // 等待应答
    {
        Soft_IIC_Stop(i2c_dev);
        return 1;
    }
    Soft_IIC_Stop(i2c_dev);
    return 0;
}

#include <df_log.h>

// ============ 自动初始化 ============
/**
 * @brief I2C框架自动初始化函数
 * @details 在框架初始化时自动调用，初始化I2C通信框架
 * @return 0表示成功
 */
static int df_iic_auto_init(void)
{
    // I2C框架暂无需特殊初始化，此函数用于日志记录
    LOG_I("IIC", "I2C framework initialized\n");
    return 0;
}

// 将I2C框架初始化注册到DEVICE级别
DF_INIT_EXPORT(df_iic_auto_init, DF_INIT_EXPORT_DEVICE);
