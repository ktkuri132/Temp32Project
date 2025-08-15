#ifndef SERIAL_TOOL
#define SERIAL_TOOL

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



#define STDE_Serial
#define BIE_Serial

typedef struct Stde_DataTypeDef stde;

typedef struct Stde_DataTypeDef {
    void (*Data_Receive)(void *, uint8_t);
    void (*CMD_Callback)();       // 命令模式回调函数（可选）
    void (*DR_Eevet_Callback)();  // 结束帧回调函数（可选）
    void (*UART_DATA_TYPE_Callback)(
        stde *);      // 数据类型回调函数（可选
    uint8_t c;                    // 单次数据
    uint8_t UART_Strat;           // 数据传输起始标志位
    uint8_t UART_End;             // 数据传输结束标志位
    uint8_t Res_Data_type;        // 数据格式检测标志位
    uint8_t UART_DATA_TYPE;       // 真实数据格式
    uint8_t Res_note;             // 数据节点
    uint8_t Res_len;              // 接收数据的下标
    uint8_t UART_NOTE;            // 本次数据节点
    uint8_t UART_LEN;             // 本次接收数据的长度
    uint8_t UART_NOTE_LEN[20];    // 某次数据指定节点的数据长度
    uint8_t USART_array[10][10];  // 数据长度酌情调整
    uint8_t Data[10][10];         // 数据长度酌情调整

} stde;

void Stde_DataTypeDef_Init(stde *data);
uint8_t STDE_UART(void *Parameters, stde *Data);
uint16_t StdeUSART_Deal(stde *Data, uint8_t point_note);


#endif

