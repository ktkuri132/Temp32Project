#include "driver.h"
// 毫秒延时函数 (基于while循环，72MHz系统时钟)
void delay_ms(uint32_t ms)
{
    uint32_t i, j;
    for(i = 0; i < ms; i++)
    {
        // 1ms的循环次数，根据72MHz调整
        // 经验值：约需要7200次循环实现1ms延时
        for(j = 0; j < 7200; j++)
        {
            __NOP(); // 空操作，防止编译器优化
        }
    }
}

// 微秒延时函数 (基于while循环，72MHz系统时钟)
void delay_us(uint32_t us)
{
    uint32_t i, j;
    for(i = 0; i < us; i++)
    {
        // 1us的循环次数，根据72MHz调整
        // 经验值：约需要7-8次循环实现1us延时
        for(j = 0; j < 8; j++)
        {
            __NOP(); // 空操作，防止编译器优化
        }
    }
}