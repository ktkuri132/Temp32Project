#ifndef __DF_LED_H
#define __DF_LED_H

#include <stdbool.h>
#include <stdint.h>

typedef struct LED_TypeDef {
    bool LED_Init_Flag;  // LED初始化标志
    uint8_t LED_Num;          // LED编号
    uint8_t LED_Color;        // LED颜色（0: 红色, 1: 绿色, 2: 蓝色）
    bool LED_State;           // LED状态（0: 关闭, 1: 打开）
    void (*Init)(void);       // LED初始化函数指针
    void (*On)(void);         // 打开LED的函数指针
    void (*Off)(void);        // 关闭LED的函数指针
    void (*Toggle)(void);     // 切换LED状态的函数指针
    uint32_t LED_Blink_Time;  // 闪烁时间间隔（毫秒）
} Lt;

typedef struct LED_Event_RunTime_TypeDef {
    uint32_t Sys_RunTime;  // 系统运行时间
    void (*LED_Read)(Lt *);  // LED读取函数指针
} Lert;


#endif