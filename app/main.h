#ifndef __MAIN_H
#define __MAIN_H

#include <stm32f10x.h>
#include <driver.h>
#include <shell.h>
#include <dev_model_core.h>

extern shell Shell; // Shell协议结构体实例
extern Sysfpoint Shell_Sysfpoint; // 系统函数指针结构体实例
extern EnvVar env_vars[]; // 环境变量数组
extern DeviceFamily STM32F103C8T6_Device; // 设备信息结构体实例
extern dev_model_t Drivers_model_poor[];
#endif