#include "stde.h"

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// 初始化函数，模拟类的构造函数
void Stde_DataTypeDef_Init(stde *data) {
    data->CMD_Callback = NULL;
    data->DR_Eevet_Callback = NULL;
    data->UART_DATA_TYPE_Callback = NULL;
    data->c = 0;
    data->UART_Strat = 0;
    data->UART_End = 0;
    data->Res_Data_type = 0;
    data->UART_DATA_TYPE = 0;
    data->Res_note = 0;
    data->Res_len = 0;
    data->UART_NOTE = 0;
    data->UART_LEN = 0;
    memset(data->UART_NOTE_LEN, 0, sizeof(data->UART_NOTE_LEN));
    memset(data->USART_array, 0, sizeof(data->USART_array));
    memset(data->Data, 0, sizeof(data->Data));
}

/// @brief STDE串口协议处理函数
uint8_t STDE_UART(void *Parameters, stde *Data) {
    Data->Data_Receive(Parameters, Data->c);  // 接收数据
    if (Data->c == 's')                        // 如果是开始字符
    {
        Data->UART_Strat = 1;     // 开始接收
        Data->UART_End = 0;       // 关闭结束标志
        Data->Res_Data_type = 1;  // 开始检测数据格式
        Data->Res_len = 0;        // 下标清零
        Data->Res_note = 0;       // 数据节点清零
        return 0;
    } else if (Data->c == 'e')  // 如果是结束字符
    {
        Data->UART_NOTE_LEN[Data->Res_note] =
            Data->Res_len;                     // 保存最后一次数据节点的长度
        Data->UART_Strat = 0;                  // 重新开始
        Data->UART_End = 1;                    // 打开结束标志
        Data->UART_LEN = Data->Res_len;        // 保存本次数据长度
        Data->UART_NOTE = Data->Res_note + 1;  // 保存本次数据节点
        Data->Res_note = 0;                    // 数据节点清零
        Data->Res_len = 0;                     // 下标清零
        memcpy(Data->Data, Data->USART_array,
               sizeof(Data->USART_array));  // 将数据拷贝到Data数组
        memset(Data->USART_array, 0,
               sizeof(Data->USART_array));  // 清空USART_array数组
        // 数据帧结束，产生回调事件
        return 0;
    } else if (Data->c == ',') {
        Data->UART_NOTE_LEN[Data->Res_note] =
            Data->Res_len;  // 保存本次数据节点的长度
        Data->Res_note++;   // 数据节点加1
        Data->Res_len = 0;  // 下标清零
    } else {
        if (Data->Res_Data_type)  // 先检测数据格式
        {
            switch (Data->c) {
                // 检测数据格式
                case '1':
                    Data->UART_DATA_TYPE = 1;
                    break;
                case '5':
                    Data->UART_DATA_TYPE = 5;
                    break;
                case '6':
                    Data->UART_DATA_TYPE = 6;
                    break;
                case '3':
                    Data->UART_DATA_TYPE = 3;
                    break;
                case '4':
                    Data->UART_DATA_TYPE = 4;
                    break;
                case '2':
                    Data->UART_DATA_TYPE = 2;
                    break;
                    // 检测：命令数据格式

                case 'p':
                    Data->UART_DATA_TYPE = 'p';
                    break;

                default:
                    break;
            }
            // (Data->UART_DATA_TYPE_Callback != NULL)
            //     ? Data->UART_DATA_TYPE_Callback(Data)
            //     : 0;
            Data->Res_Data_type = 0;  // 关闭数据格式检测
        }

        if (Data->UART_Strat)  // 开始接收后，数组的第一个位置存储的是数据格式，读数据要从第二个位置开始
        {
            Data->USART_array[Data->Res_note][Data->Res_len] =
                Data->c;      // 存储数据
            Data->Res_len++;  // 下标加1
        }
    }
}

/// @brief 处理串口数据针对于数字
/// @param point_note 待处理的指定节点
uint16_t StdeUSART_Deal(stde *Data, uint8_t point_note) {
    // 检查是否越界
    if (point_note > Data->UART_NOTE) {
        return 0;
    }
    uint16_t sum = 0;
    uint8_t len = Data->UART_NOTE_LEN[point_note];

    for (uint8_t i = 0; i < len; i++) {
        uint8_t temp_target = Data->Data[point_note][i];
        // 判断临时目标数据的ascll码值
        if (!(temp_target >= '0' && temp_target <= '9'))  // 如果不是数字字符
        {
            return temp_target;  // 直接返回
        }

        sum = sum * 10 + (temp_target - '0');
    }

    return sum;
}



