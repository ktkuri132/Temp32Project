#include "sh1106.h"
#include <math.h>

#ifdef SH1106

uint8_t SH1106_DisplayBuf[8][128];

#ifdef Peripheral_SPI
#include <stm32f4xx_gpio.h>
/**
 * 函    数：SH1106写D0（CLK）高低电平
 * 参    数：要写入D0的电平值，范围：0/1
 * 返 回 值：无
 * 说    明：当上层函数需要写D0时，此函数会被调用
 *           用户需要根据参数传入的值，将D0置为高电平或者低电平
 *           当参数传入0时，置D0为低电平，当参数传入1时，置D0为高电平
 */
void SH1106_W_D0(uint8_t BitValue)
{
    /*根据BitValue的值，将D0置高电平或者低电平*/
    GPIO_WriteBit(GPIOB, GPIO_Pin_8, (BitAction)BitValue);
}

/**
 * 函    数：SH1106写D1（MOSI）高低电平
 * 参    数：要写入D1的电平值，范围：0/1
 * 返 回 值：无
 * 说    明：当上层函数需要写D1时，此函数会被调用
 *           用户需要根据参数传入的值，将D1置为高电平或者低电平
 *           当参数传入0时，置D1为低电平，当参数传入1时，置D1为高电平
 */
void SH1106_W_D1(uint8_t BitValue)
{
    /*根据BitValue的值，将D1置高电平或者低电平*/
    GPIO_WriteBit(GPIOB, GPIO_Pin_9, (BitAction)BitValue);
}

/**
 * 函    数：SH1106写RES高低电平
 * 参    数：要写入RES的电平值，范围：0/1
 * 返 回 值：无
 * 说    明：当上层函数需要写RES时，此函数会被调用
 *           用户需要根据参数传入的值，将RES置为高电平或者低电平
 *           当参数传入0时，置RES为低电平，当参数传入1时，置RES为高电平
 */
void SH1106_W_RES(uint8_t BitValue)
{
    /*根据BitValue的值，将RES置高电平或者低电平*/
    GPIO_WriteBit(GPIOB, GPIO_Pin_7, (BitAction)BitValue);
}

/**
 * 函    数：SH1106写DC高低电平
 * 参    数：要写入DC的电平值，范围：0/1
 * 返 回 值：无
 * 说    明：当上层函数需要写DC时，此函数会被调用
 *           用户需要根据参数传入的值，将DC置为高电平或者低电平
 *           当参数传入0时，置DC为低电平，当参数传入1时，置DC为高电平
 */
void SH1106_W_DC(uint8_t BitValue)
{
    /*根据BitValue的值，将DC置高电平或者低电平*/
    GPIO_WriteBit(GPIOB, GPIO_Pin_10, (BitAction)BitValue);
}

/**
 * 函    数：SH1106写CS高低电平
 * 参    数：要写入CS的电平值，范围：0/1
 * 返 回 值：无
 * 说    明：当上层函数需要写CS时，此函数会被调用
 *           用户需要根据参数传入的值，将CS置为高电平或者低电平
 *           当参数传入0时，置CS为低电平，当参数传入1时，置CS为高电平
 */
void SH1106_W_CS(uint8_t BitValue)
{
    /*根据BitValue的值，将CS置高电平或者低电平*/
    GPIO_WriteBit(GPIOB, GPIO_Pin_11, (BitAction)BitValue);
}

/**
 * 函    数：SH1106引脚初始化
 * 参    数：无
 * 返 回 值：无
 * 说    明：当上层函数需要初始化时，此函数会被调用
 *           用户需要将D0、D1、RES、DC和CS引脚初始化为推挽输出模式
 */
void SH1106_GPIO_Init(void)
{
    uint32_t i, j;

    /*在初始化前，加入适量延时，待SH1106供电稳定*/
    for (i = 0; i < 1000; i++)
    {
        for (j = 0; j < 1000; j++)
            ;
    }

    /*将D0、D1、RES、DC和CS引脚初始化为推挽输出模式*/
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    // RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /*置引脚默认电平*/
    SH1106_W_D0(0);
    SH1106_W_D1(1);
    SH1106_W_RES(1);
    SH1106_W_DC(1);
    SH1106_W_CS(1);
}

/*********************引脚配置*/

/*通信协议*********************/

/**
 * 函    数：SPI发送一个字节
 * 参    数：Byte 要发送的一个字节数据，范围：0x00~0xFF
 * 返 回 值：无
 */
void SH1106_SPI_SendByte(uint8_t Byte)
{
    uint8_t i;

    /*循环8次，主机依次发送数据的每一位*/
    for (i = 0; i < 8; i++)
    {
        /*使用掩码的方式取出Byte的指定一位数据并写入到D1线*/
        /*两个!的作用是，让所有非零的值变为1*/
        SH1106_W_D1(!!(Byte & (0x80 >> i)));
        SH1106_W_D0(1); // 拉高D0，从机在D0上升沿读取SDA
        SH1106_W_D0(0); // 拉低D0，主机开始发送下一位数据
    }
}

/**
 * 函    数：SH1106写命令
 * 参    数：Command 要写入的命令值，范围：0x00~0xFF
 * 返 回 值：无
 */
void SH1106_WriteCommand(uint8_t Command)
{
    SH1106_W_CS(0);               // 拉低CS，开始通信
    SH1106_W_DC(0);               // 拉低DC，表示即将发送命令
    SH1106_SPI_SendByte(Command); // 写入指定命令
    SH1106_W_CS(1);               // 拉高CS，结束通信
}

/**
 * 函    数：SH1106写数据
 * 参    数：Data 要写入数据的起始地址
 * 参    数：Count 要写入数据的数量
 * 返 回 值：无
 */
void SH1106_WriteData(uint8_t *Data, uint8_t Count)
{
    uint8_t i;

    SH1106_W_CS(0); // 拉低CS，开始通信
    SH1106_W_DC(1); // 拉高DC，表示即将发送数据
    /*循环Count次，进行连续的数据写入*/
    for (i = 0; i < Count; i++)
    {
        SH1106_SPI_SendByte(Data[i]); // 依次发送Data的每一个数据
    }
    SH1106_W_CS(1); // 拉高CS，结束通信
}

#endif

uint8_t SH1106_Init(void)
{
    if(i2c_Dev.soft_iic_init_flag == 0){
        SH1106_GPIO_Init(); // 先调用底层的端口初始化
    }
    /*写入一系列的命令，对SH1106进行初始化配置*/
    if(SH1106_WriteCommand(0xAE)) // 设置显示开启/关闭，0xAE关闭，0xAF开启
        return 1;
    SH1106_WriteCommand(0xD5); // 设置显示时钟分频比/振荡器频率
    SH1106_WriteCommand(0x80); // 0x00~0xFF

    SH1106_WriteCommand(0xA8); // 设置多路复用率
    SH1106_WriteCommand(0x3F); // 0x0E~0x3F

    SH1106_WriteCommand(0xD3); // 设置显示偏移
    SH1106_WriteCommand(0x00); // 0x00~0x7F

    SH1106_WriteCommand(0x40); // 设置显示开始行，0x40~0x7F

    SH1106_WriteCommand(0xA1); // 设置左右方向，0xA1正常，0xA0左右反置

    SH1106_WriteCommand(0xC8); // 设置上下方向，0xC8正常，0xC0上下反置

    SH1106_WriteCommand(0xDA); // 设置COM引脚硬件配置
    SH1106_WriteCommand(0x12);

    SH1106_WriteCommand(0x81); // 设置对比度
    SH1106_WriteCommand(0xCF); // 0x00~0xFF

    SH1106_WriteCommand(0xD9); // 设置预充电周期
    SH1106_WriteCommand(0xF1);

    SH1106_WriteCommand(0xDB); // 设置VCOMH取消选择级别
    SH1106_WriteCommand(0x30);

    SH1106_WriteCommand(0xA4); // 设置整个显示打开/关闭

    SH1106_WriteCommand(0xA6); // 设置正常/反色显示，0xA6正常，0xA7反色

    SH1106_WriteCommand(0x8D); // 设置充电泵
    SH1106_WriteCommand(0x14);

    SH1106_WriteCommand(0xAF); // 开启显示

    SH1106_Clear();  // 清空显存数组
    SH1106_Update(); // 更新显示，清屏，防止初始化后未显示内容时花屏
    return 0;
}

/// @brief 检测SH1106设备是否存在
/// @param none
/// @return 0：设备存在，-1：设备不存在
uint8_t SH1106_CheakDevice(void)
{
    static uint8_t initialized = 0;
    if (!initialized)
    {
        if(SH1106_Init()){
            return -1; // 设备不存在
        }
        initialized = 1;
    } else {
        if(SH1106_Device_AckCheak()){
            initialized = 0; // 如果写命令失败，标记为未初始化
            return 0; // 设备不存在
        }
    }
    return 0; // 假设设备总是存在
}

/**
 * 函    数：SH1106设置显示光标位置
 * 参    数：Page 指定光标所在的页，范围：0~7
 * 参    数：X 指定光标所在的X轴坐标，范围：0~127
 * 返 回 值：无
 * 说    明：SH1106默认的Y轴，只能8个Bit为一组写入，即1页等于8个Y轴坐标
 */
void SH1106_SetCursor(uint8_t Page, uint8_t X)
{
    /*如果使用此程序驱动1.3寸的SH1106显示屏，则需要解除此注释*/
    /*因为1.3寸的SH1106驱动芯片（SH1106）有132列*/
    /*屏幕的起始列接在了第2列，而不是第0列*/
    /*所以需要将X加2，才能正常显示*/
    X += 2;

    /*通过指令设置页地址和列地址*/
    SH1106_WriteCommand(0xB0 | Page);              // 设置页位置
    SH1106_WriteCommand(0x10 | ((X & 0xF0) >> 4)); // 设置X位置高4位
    SH1106_WriteCommand(0x00 | (X & 0x0F));        // 设置X位置低4位
}


/**
 * 函    数：将SH1106显存数组更新到SH1106屏幕
 * 参    数：无
 * 返 回 值：无
 * 说    明：所有的显示函数，都只是对SH1106显存数组进行读写
 *           随后调用SH1106_Update函数或SH1106_UpdateArea函数
 *           才会将显存数组的数据发送到SH1106硬件，进行显示
 *           故调用显示函数后，要想真正地呈现在屏幕上，还需调用更新函数
 */
void SH1106_Update(void)
{
    uint8_t j;
    /*遍历每一页*/
    for (j = 0; j < 8; j++)
    {
        /*设置光标位置为每一页的第一列*/
        SH1106_SetCursor(j, 0);
        /*连续写入128个数据，将显存数组的数据写入到SH1106硬件*/
        SH1106_WriteData(SH1106_DisplayBuf[j], 128);
    }
}

/**
 * 函    数：将SH1106显存数组部分更新到SH1106屏幕
 * 参    数：X 指定区域左上角的横坐标，范围：-32768~32767，屏幕区域：0~127
 * 参    数：Y 指定区域左上角的纵坐标，范围：-32768~32767，屏幕区域：0~63
 * 参    数：Width 指定区域的宽度，范围：0~128
 * 参    数：Height 指定区域的高度，范围：0~64
 * 返 回 值：无
 * 说    明：此函数会至少更新参数指定的区域
 *           如果更新区域Y轴只包含部分页，则同一页的剩余部分会跟随一起更新
 * 说    明：所有的显示函数，都只是对SH1106显存数组进行读写
 *           随后调用SH1106_Update函数或SH1106_UpdateArea函数
 *           才会将显存数组的数据发送到SH1106硬件，进行显示
 *           故调用显示函数后，要想真正地呈现在屏幕上，还需调用更新函数
 */
void SH1106_UpdateArea(int16_t X, int16_t Y, uint8_t Width, uint8_t Height)
{
    int16_t j;
    int16_t Page, Page1;

    /*负数坐标在计算页地址时需要加一个偏移*/
    /*(Y + Height - 1) / 8 + 1的目的是(Y + Height) / 8并向上取整*/
    Page = Y / 8;
    Page1 = (Y + Height - 1) / 8 + 1;
    if (Y < 0)
    {
        Page -= 1;
        Page1 -= 1;
    }

    /*遍历指定区域涉及的相关页*/
    for (j = Page; j < Page1; j++)
    {
        if (X >= 0 && X <= 127 && j >= 0 && j <= 7) // 超出屏幕的内容不显示
        {
            /*设置光标位置为相关页的指定列*/
            SH1106_SetCursor(j, X);
            /*连续写入Width个数据，将显存数组的数据写入到SH1106硬件*/
            SH1106_WriteData(&SH1106_DisplayBuf[j][X], Width);
        }
    }
}

/**
 * 函    数：将SH1106显存数组全部清零
 * 参    数：无
 * 返 回 值：无
 * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
 */
void SH1106_Clear(void)
{
    uint8_t i, j;
    for (j = 0; j < 8; j++) // 遍历8页
    {
        for (i = 0; i < 128; i++) // 遍历128列
        {
            SH1106_DisplayBuf[j][i] = 0x00; // 将显存数组数据全部清零
        }
    }
}

/**
 * 函    数：将SH1106显存数组部分清零
 * 参    数：X 指定区域左上角的横坐标，范围：-32768~32767，屏幕区域：0~127
 * 参    数：Y 指定区域左上角的纵坐标，范围：-32768~32767，屏幕区域：0~63
 * 参    数：Width 指定区域的宽度，范围：0~128
 * 参    数：Height 指定区域的高度，范围：0~64
 * 返 回 值：无
 * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
 */
void SH1106_ClearArea(int16_t X, int16_t Y, uint8_t Width, uint8_t Height)
{
    int16_t i, j;

    for (j = Y; j < Y + Height; j++) // 遍历指定页
    {
        for (i = X; i < X + Width; i++) // 遍历指定列
        {
            if (i >= 0 && i <= 127 && j >= 0 && j <= 63) // 超出屏幕的内容不显示
            {
                SH1106_DisplayBuf[j / 8][i] &= ~(0x01 << (j % 8)); // 将显存数组指定数据清零
            }
        }
    }
}

/**
 * 函    数：将SH1106显存数组全部取反
 * 参    数：无
 * 返 回 值：无
 * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
 */
void SH1106_Reverse(void)
{
    uint8_t i, j;
    for (j = 0; j < 8; j++) // 遍历8页
    {
        for (i = 0; i < 128; i++) // 遍历128列
        {
            SH1106_DisplayBuf[j][i] ^= 0xFF; // 将显存数组数据全部取反
        }
    }
}

/**
 * 函    数：将SH1106显存数组部分取反
 * 参    数：X 指定区域左上角的横坐标，范围：-32768~32767，屏幕区域：0~127
 * 参    数：Y 指定区域左上角的纵坐标，范围：-32768~32767，屏幕区域：0~63
 * 参    数：Width 指定区域的宽度，范围：0~128
 * 参    数：Height 指定区域的高度，范围：0~64
 * 返 回 值：无
 * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
 */
void SH1106_ReverseArea(int16_t X, int16_t Y, uint8_t Width, uint8_t Height)
{
    int16_t i, j;

    for (j = Y; j < Y + Height; j++) // 遍历指定页
    {
        for (i = X; i < X + Width; i++) // 遍历指定列
        {
            if (i >= 0 && i <= 127 && j >= 0 && j <= 63) // 超出屏幕的内容不显示
            {
                SH1106_DisplayBuf[j / 8][i] ^= 0x01 << (j % 8); // 将显存数组指定数据取反
            }
        }
    }
}


/**
 * 函    数：SH1106显示图像
 * 参    数：X 指定图像左上角的横坐标，范围：-32768~32767，屏幕区域：0~127
 * 参    数：Y 指定图像左上角的纵坐标，范围：-32768~32767，屏幕区域：0~63
 * 参    数：Width 指定图像的宽度，范围：0~128
 * 参    数：Height 指定图像的高度，范围：0~64
 * 参    数：Image 指定要显示的图像
 * 返 回 值：无
 * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
 */
void SH1106_ShowImage(int16_t X, int16_t Y, uint8_t Width, uint8_t Height, const uint8_t *Image)
{
    uint8_t i = 0, j = 0;
    int16_t Page, Shift;

    /*将图像所在区域清空*/
    SH1106_ClearArea(X, Y, Width, Height);

    /*遍历指定图像涉及的相关页*/
    /*(Height - 1) / 8 + 1的目的是Height / 8并向上取整*/
    for (j = 0; j < (Height - 1) / 8 + 1; j++)
    {
        /*遍历指定图像涉及的相关列*/
        for (i = 0; i < Width; i++)
        {
            if (X + i >= 0 && X + i <= 127) // 超出屏幕的内容不显示
            {
                /*负数坐标在计算页地址和移位时需要加一个偏移*/
                Page = Y / 8;
                Shift = Y % 8;
                if (Y < 0)
                {
                    Page -= 1;
                    Shift += 8;
                }

                if (Page + j >= 0 && Page + j <= 7) // 超出屏幕的内容不显示
                {
                    /*显示图像在当前页的内容*/
                    SH1106_DisplayBuf[Page + j][X + i] |= Image[j * Width + i] << (Shift);
                }

                if (Page + j + 1 >= 0 && Page + j + 1 <= 7) // 超出屏幕的内容不显示
                {
                    /*显示图像在下一页的内容*/
                    SH1106_DisplayBuf[Page + j + 1][X + i] |= Image[j * Width + i] >> (8 - Shift);
                }
            }
        }
    }
}


/**
 * 函    数：SH1106在指定位置画一个点
 * 参    数：X 指定点的横坐标，范围：-32768~32767，屏幕区域：0~127
 * 参    数：Y 指定点的纵坐标，范围：-32768~32767，屏幕区域：0~63
 * 返 回 值：无
 * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
 */
void SH1106_DrawPoint(int16_t X, int16_t Y)
{
    if (X >= 0 && X <= 127 && Y >= 0 && Y <= 63) // 超出屏幕的内容不显示
    {
        /*将显存数组指定位置的一个Bit数据置1*/
        SH1106_DisplayBuf[Y / 8][X] |= 0x01 << (Y % 8);
    }
}

/**
 * 函    数：SH1106获取指定位置点的值
 * 参    数：X 指定点的横坐标，范围：-32768~32767，屏幕区域：0~127
 * 参    数：Y 指定点的纵坐标，范围：-32768~32767，屏幕区域：0~63
 * 返 回 值：指定位置点是否处于点亮状态，1：点亮，0：熄灭
 */
uint32_t SH1106_GetPoint(uint16_t X, uint16_t Y)
{
    if (X >= 0 && X <= 127 && Y >= 0 && Y <= 63) // 超出屏幕的内容不读取
    {
        /*判断指定位置的数据*/
        if (SH1106_DisplayBuf[Y / 8][X] & 0x01 << (Y % 8))
        {
            return 1; // 为1，返回1
        }
    }

    return 0; // 否则，返回0
}



#endif