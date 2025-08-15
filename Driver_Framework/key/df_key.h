#ifndef __DF_KEY_H
#define __DF_KEY_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

typedef struct KEY_TypeDef {
    uint8_t Key_Num;            // 按键编号
    bool Key_Pressed;           // 按键状态
    bool Key_LS_State;          // 按键长短按判断
    uint8_t Key_Press_Count;    // 按键按下计数
    uint32_t Key_Pressed_Time;  // 按键按下时间(s)
    bool Key_ok;                // 按键结束状态
} Kt;

typedef struct Key_Event_RunTime_TypeDef {
    uint32_t Sys_RunTime;             // 系统运行时间
    bool (*Key_Read)(Kt *);  // 按键读取函数指针
} Kert;

void Key_Tick(Kert *key_evet_handler);
void Key_IRQHandler(Kert *key_evet_handler, Kt *key);
bool Key_ReadBlocking(Kert *key_evet_handler, Kt *key, void (*func)(void));
uint8_t Key_Event(Kert *key_evet_handler, Kt *key, uint32_t TimeOut,
                  void (*Default_Pressed_CallBack)(void),
                  void (*TimeOut_Pressed_CallBack)(void),
                  void (*Default_Release_CallBack)(void),
                  void (*TimeOut_Release_CallBack)(void));
#endif