#ifndef __SEHLL_H
#define __SEHLL_H

#include <df_log.h>  // 终端样式和日志
#include <df_uart.h> // UART 设备接口
#include <stdint.h>
#include <stdbool.h>

// 主版本号
#define SHELL_VERSION_MAIN 1
// 确认发行版号
#define SHELL_VERSION_RE 0
// 更新号
#define SHELL_VERSION_UPDATE 2

typedef struct
{
    char *Architecture; // 处理器架构
    char *User;         // 用户名
    char *Password;     // 密码
    char *DeviceName;   // 设备名称
    char *OS;           // 操作系统
    char *Device;       // 设备型号
    char *Version;      // 版本信息
} DeviceFamily;

typedef struct ShellTypeDef
{
    bool Shell_Init; // Shell初始化标志
    uint8_t c;
    uint8_t Res_len;   // 接收数据的下标
    uint8_t UART_NOTE; // 本次数据节点
    uint8_t RunStae;   // 运行状态
    uint8_t Data[20];  // 数据长度酌情调整
    int (*Data_Receive)(df_arg_t);
} shell; // Shell协议结构体

/**
 * @brief Sysfpoint(系统函数指针)结构体
 * @note  该结构体用于存储系统函数指针和参数
 * @param  syspfunc: 系统函数指针
 * @param  Parameters: 系统函数指针参数
 * @param  argc: 参数个数
 */
typedef struct
{
    void (*syspfunc)(int, void *[]); // 系统函数指针
    void **Parameters;
    int argc;
} Sysfpoint;

typedef struct
{
    char *name;                      // 命令名称
    uint8_t RunStae;                 // 运行状态
    void **arg;                      // 命令参数指针
    int argc;                        // 参数个数
    void (*callback)(int, void *[]); // 命令回调函数
} EnvVar;                            // 环境变量结构体

/*===========================================================================*/
/*                         Shell API 函数声明                                 */
/*===========================================================================*/

/**
 * @brief 设置 Shell 使用的 UART 设备
 * @param uart UART 设备指针 (df_uart_t*)
 * @note 必须在 MCU_Shell_Init 之前调用
 */
void shell_set_uart(df_uart_t *uart);

/**
 * @brief 初始化 Shell
 * @param ShellTypeStruct Shell 结构体指针
 * @param log 设备信息结构体指针
 */
void MCU_Shell_Init(shell *ShellTypeStruct, DeviceFamily *log);

/**
 * @brief 处理 Shell 命令
 * @param sfp 系统函数指针结构体
 * @param sh Shell 结构体指针
 * @param env_vars 环境变量数组
 */
void Shell_Deal(Sysfpoint *sfp, shell *sh, EnvVar *env_vars);

/**
 * @brief UART 接收中断处理函数
 * @param Parameters 接收到的字符
 * @param sfp 系统函数指针结构体
 * @param ShellTypeStruct Shell 结构体指针
 * @param env 环境变量数组
 * @param log 设备信息结构体指针
 */
void BIE_UART(uint8_t Parameters, Sysfpoint *sfp, shell *ShellTypeStruct,
              EnvVar *env, DeviceFamily *log);

/**
 * @brief 主循环命令切换运行函数
 * @param sfp 系统函数指针结构体
 */
void Task_Switch_Tick_Handler(Sysfpoint *sfp);

/**
 * @brief 系统默认配置命令指针结构体
 */
typedef struct
{
    void (*ls)(int, void *[]);       // ls命令回调函数
    void (*reset)(int, void *[]);    // reboot命令回调函数
    void (*poweroff)(int, void *[]); // poweroff命令回调函数
    void (*help)(int, void *[]);     // help命令回调函数
    void (*clear)(int, void *[]);    // clear命令回调函数
    void (*test)(int, void *[]);     // test命令回调函数
} Cmd_PointerTypeDef;

#endif