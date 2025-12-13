#include "main.h"
shell Shell = {
    .Shell_Init = false,               // Shell未初始化
    .c = 0,                            // 初始化接收字符
    .Res_len = 0,                      // 初始化接收长度
    .UART_NOTE = 0,                    // 初始化串口节点
    .RunStae = 0,                      // 初始化运行状态
    .Data_Receive = USART1_ReceiveChar // 数据接收函数指针
};

Sysfpoint Shell_Sysfpoint;

DeviceFamily STM32F103C8T6_Device = {.Architecture = "cortex-m3",
                                     .DeviceName = "STM32F103C8T6",
                                     .OS = "BareMetal",
                                     .Device = "STM32F1",
                                     .User = "Admin",
                                     .Password = "133990",
                                     .Version = "1.0.0"};

dev_info_t Dev_info_poor[] = {
    {.name = DEBUG_UART_NAME,
     .init = usart1_init,
     .enable = usart1_start,
     .disable = usart1_stop,
     .arg.s32 = 250000},

    {.name = "SysTick",
     .init = systick_init,
     .enable = NULL,  // SysTick不需要启用函数
     .disable = NULL, // SysTick不需要禁用函数
     .arg.ptr = NULL},

    {.name = "NVIC",
     .init = nvic_init,
     .enable = NULL,  // NVIC不需要启用函数
     .disable = NULL, // NVIC不需要禁用函数
     .arg.ptr = NULL},

    {.name = ONBOARD_LED_NAME,
     .init = led_init,
     .enable = led_on,
     .disable = led_off,
     .arg.ptr = NULL},

    {.name = ADC1_NAME,
     .init = adc1_init,
     .enable = adc1_enable,
     .disable = adc1_disable,
     .arg.ptr = NULL},

    {.name = "", // 空字符串表示数组结束
     .init = NULL,
     .enable = NULL,
     .disable = NULL,
     .arg.ptr = NULL}

};

EnvVar env_vars[] = {
    {NULL} // 环境变量列表结束标志
};