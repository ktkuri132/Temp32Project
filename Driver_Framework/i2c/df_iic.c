#include "df_iic.h"
#include "df_init.h"

// 初始化IIC
void Soft_IIC_Init(df_soft_iic_t *i2c)
{
    if (i2c->gpio_init != NULL)
    {
        i2c->gpio_init(); // 初始化IIC的GPIO端口
    }
    i2c->scl(1);
    i2c->sda(1);
    i2c->init_flag = true;
}

// 产生IIC起始信号
void Soft_IIC_Start(df_soft_iic_t *i2c)
{
    i2c->sda_out(); // sda线输出
    i2c->sda(1);
    i2c->scl(1);
    // i2c->delay_us(4);
    i2c->sda(
        0);          // START:when CLK is high,DATA change form high to low
                     //    i2c->delay_us(1);
    i2c->scl(0); // 钳住I2C总线，准备发送或接收数据
}
// 产生IIC停止信号
void Soft_IIC_Stop(df_soft_iic_t *i2c)
{
    i2c->sda_out(); // sda线输出
    i2c->scl(0);
    i2c->sda(
        0); // STOP:when CLK is high DATA change form low to high
    // delay_us(4);
    i2c->scl(1);
    i2c->sda(1); // 发送I2C总线结束信号
    // delay_us(4);
}
// 等待应答信号到来
// 返回值：1，接收应答失败
//         0，接收应答成功
uint8_t Soft_IIC_Wait_Ack(df_soft_iic_t *i2c)
{
    uint8_t ucErrTime = 0;
    i2c->sda_in(); // SDA设置为输入
    i2c->sda(1);
    //    i2c->delay_us(1);
    i2c->scl(1);
    //    i2c->delay_us(1);
    while (i2c->read_sda())
    {
        ucErrTime++;
        if (ucErrTime > 250)
        {
            Soft_IIC_Stop(i2c);
            return 1;
        }
    }
    i2c->scl(0); // 时钟输出0
    return 0;
}
// 产生ACK应答
void Soft_IIC_Ack(df_soft_iic_t *i2c)
{
    i2c->scl(0);
    i2c->sda_out();
    i2c->sda(0);
    //    i2c->delay_us(1);
    i2c->scl(1);
    // i2c->delay_us(2);
    i2c->scl(0);
}
// 不产生ACK应答
void Soft_IIC_NAck(df_soft_iic_t *i2c)
{
    i2c->scl(0);
    i2c->sda_out();
    i2c->sda(1);
    //    i2c->delay_us(1);
    i2c->scl(1);
    // delay_us(2);
    i2c->scl(0);
}
// IIC发送一个字节
// 返回从机有无应答
// 1，有应答
// 0，无应答
void Soft_IIC_Send_Byte(df_soft_iic_t *i2c, uint8_t txd)
{
    uint8_t t;
    i2c->sda_out();
    i2c->scl(0); // 拉低时钟开始数据传输
    for (t = 0; t < 8; t++)
    {
        i2c->sda((txd & 0x80) >> 7);
        txd <<= 1;
        //    i2c->delay_us(1);
        i2c->scl(1);
        //    i2c->delay_us(1);
        i2c->scl(0);
        //    i2c->delay_us(1);
    }
}
// 读1个字节，ack=1时，发送ACK，ack=0，发送nACK
uint8_t Soft_IIC_Receive_Byte(df_soft_iic_t *i2c, unsigned char ack)
{
    unsigned char i, receive = 0;
    i2c->sda_in(); // SDA设置为输入
    for (i = 0; i < 8; i++)
    {
        i2c->scl(0);
        //    i2c->delay_us(1);
        i2c->scl(1);
        receive <<= 1;
        if (i2c->read_sda())
            receive++;
        //    i2c->delay_us(1);
    }
    if (!ack)
        Soft_IIC_NAck(i2c); // 发送nACK
    else
        Soft_IIC_Ack(i2c); // 发送ACK
    return receive;
}

uint8_t Soft_IIC_Write_Byte(df_soft_iic_t *i2c, uint8_t addr, uint8_t reg,
                            uint8_t data)
{
    Soft_IIC_Start(i2c);
    Soft_IIC_Send_Byte(i2c, addr | 0); // 发送器件地址+写命令
    if (Soft_IIC_Wait_Ack(i2c))        // 等待应答
    {
        Soft_IIC_Stop(i2c);
        return 1;
    }
    Soft_IIC_Send_Byte(i2c, reg);  // 写寄存器地址
    Soft_IIC_Wait_Ack(i2c);        // 等待应答
    Soft_IIC_Send_Byte(i2c, data); // 发送数据
    if (Soft_IIC_Wait_Ack(i2c))    // 等待ACK
    {
        Soft_IIC_Stop(i2c);
        return 1;
    }
    Soft_IIC_Stop(i2c);
    return 0;
}

uint8_t Soft_IIC_Read_Byte(df_soft_iic_t *i2c, uint8_t addr, uint8_t reg)
{
    uint8_t res;
    Soft_IIC_Start(i2c);
    Soft_IIC_Send_Byte(i2c, addr | 0); // 发送器件地址+写命令
    Soft_IIC_Wait_Ack(i2c);            // 等待应答
    Soft_IIC_Send_Byte(i2c, reg);      // 写寄存器地址
    Soft_IIC_Wait_Ack(i2c);            // 等待应答
    Soft_IIC_Start(i2c);
    Soft_IIC_Send_Byte(i2c, addr | 1);   // 发送器件地址+读命令
    Soft_IIC_Wait_Ack(i2c);              // 等待应答
    res = Soft_IIC_Receive_Byte(i2c, 0); // 读取数据,发送nACK
    Soft_IIC_Stop(i2c);                  // 产生一个停止条件
    return res;
}

uint8_t Soft_IIC_Write_Len(df_soft_iic_t *i2c, uint8_t addr, uint8_t reg,
                           uint8_t len, uint8_t *buf)
{
    uint8_t i;
    Soft_IIC_Start(i2c);
    Soft_IIC_Send_Byte(i2c, addr | 0); // 发送器件地址+写命令
    if (Soft_IIC_Wait_Ack(i2c))        // 等待应答
    {
        Soft_IIC_Stop(i2c);
        return 1;
    }
    Soft_IIC_Send_Byte(i2c, reg); // 写寄存器地址
    Soft_IIC_Wait_Ack(i2c);       // 等待应答
    for (i = 0; i < len; i++)
    {
        Soft_IIC_Send_Byte(i2c, buf[i]); // 发送数据
        if (Soft_IIC_Wait_Ack(i2c))      // 等待ACK
        {
            Soft_IIC_Stop(i2c);
            return 1;
        }
    }
    Soft_IIC_Stop(i2c);
    return 0;
}

uint8_t Soft_IIC_Read_Len(df_soft_iic_t *i2c, uint8_t addr, uint8_t reg, uint8_t len,
                          uint8_t *buf)
{
    Soft_IIC_Start(i2c);
    Soft_IIC_Send_Byte(i2c, addr | 0); // 发送器件地址+写命令
    if (Soft_IIC_Wait_Ack(i2c))        // 等待应答
    {
        // printf("未检测到IIC设备");
        Soft_IIC_Stop(i2c);
        return 1;
    }
    Soft_IIC_Send_Byte(i2c, reg); // 写寄存器地址
    Soft_IIC_Wait_Ack(i2c);       // 等待应答
    Soft_IIC_Start(i2c);
    Soft_IIC_Send_Byte(i2c, addr | 1); // 发送器件地址+读命令
    Soft_IIC_Wait_Ack(i2c);            // 等待应答
    while (len)
    {
        if (len == 1)
            *buf = Soft_IIC_Receive_Byte(i2c, 0); // 读数据,发送nACK
        else
            *buf = Soft_IIC_Receive_Byte(i2c, 1); // 读数据,发送ACK
        len--;
        buf++;
    }
    Soft_IIC_Stop(i2c); // 产生一个停止条件
    return 0;
}

uint8_t Soft_IIC_Check(df_soft_iic_t *i2c, uint8_t addr)
{
    Soft_IIC_Start(i2c);
    Soft_IIC_Send_Byte(i2c, addr | 0);
    if (Soft_IIC_Wait_Ack(i2c)) // 等待应答
    {
        Soft_IIC_Stop(i2c);
        return 1;
    }
    Soft_IIC_Stop(i2c);
    return 0;
}



