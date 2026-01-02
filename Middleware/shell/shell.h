#ifndef __SEHLL_H
#define __SEHLL_H
#include <df_log.h> // 终端样式和日志已合并到 df_log.h
#include <stdint.h>
#include <stdbool.h>

// 主版本号
#define SHELL_VERSION_MAIN 1
// 确认发行版号
#define SHELL_VERSION_RE 0
// 更新号
#define SHELL_VERSION_UPDATE 1

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
    uint8_t (*Data_Receive)(void *, uint8_t *);
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

void MCU_Shell_Init(shell *ShellTypeStruct, DeviceFamily *log);
void Shell_Deal(Sysfpoint *sfp, shell *sh, EnvVar *env_vars);
void BIE_UART(uint8_t Parameters, Sysfpoint *sfp, shell *ShellTypeStruct,
              EnvVar *env, DeviceFamily *log);
void Task_Switch_Tick_Handler(Sysfpoint *sfp);

typedef struct
{
    void (*ls)(int, void *[]);       // ls命令回调函数
    void (*reset)(int, void *[]);    // reboot命令回调函数
    void (*poweroff)(int, void *[]); // poweroff命令回调函数
    void (*help)(int, void *[]);     // help命令回调函数
    void (*clear)(int, void *[]);    // clear命令回调函数
    void (*test)(int, void *[]);     // test命令回调函数
} Cmd_PointerTypeDef;                // 系统默认配置命令指针结构体

#endif