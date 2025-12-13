#include "main.h"
#include <stdint.h>
#include <sh1106.h>
// 声明flash测试函数
void flash_info_query(void);
uint32_t a = 0;
int main()
{
    debug.send(arg_ptr("System Start!\r\n"));
    Device_Registration(Dev_info_poor); // 初始化设备模型
    // MCU_Shell_Init(&Shell, &STM32F103C8T6_Device);  // 初始化Shell
    SH1106_Init(); // 初始化SH1106显示屏
    // 运行flash测试
    // flash_info_query();

    while (1)
    {
        // Task_Switch_Tick_Handler(&Shell_Sysfpoint);
        led.toggle(arg_null);
        adc1.get_value(arg_u32(a));
        delay.ms(500);
    }
    return 0;
}
