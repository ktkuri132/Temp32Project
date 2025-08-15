#include "main.h"
shell Shell = {
    .Shell_Init = false, // Shell未初始化
    .c = 0, // 初始化接收字符
    .Res_len = 0, // 初始化接收长度
    .UART_NOTE = 0, // 初始化串口节点
    .RunStae = 0, // 初始化运行状态
    .Data_Receive = USART1_ReceiveChar // 数据接收函数指针
};

Sysfpoint Shell_Sysfpoint;

DeviceFamily STM32F103C8T6_Device = {
    .Architecture = "cortex-m3",
    .DeviceName = "STM32F103C8T6",
    .OS = "BareMetal",
    .Device = "STM32F1",
    .User = "Admin",
    .Password = "133990",
    .Version = "1.0.0"
};

EnvVar env_vars[] = {
    {NULL} // 环境变量列表结束标志
};