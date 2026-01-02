/**
 * @file shell.c
 * @brief MCU Shell 命令行接口实现
 * @note 使用 Driver_Framework 的 UART 接口进行输入输出
 */

#include "df_shell.h"
#include "df_uart.h"

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/*===========================================================================*/
/*                         Shell UART 输出接口                                */
/*===========================================================================*/

/** @brief Shell 使用的 UART 设备指针 */
static df_uart_t *shell_uart = NULL;

/**
 * @brief 设置 Shell 使用的 UART 设备
 * @param uart UART 设备指针
 */
void shell_set_uart(df_uart_t *uart)
{
    shell_uart = uart;
}

/**
 * @brief Shell 输出字符串
 * @param str 要输出的字符串
 */
static void shell_puts(const char *str)
{
    if (shell_uart != NULL && shell_uart->send != NULL)
    {
        shell_uart->send(arg_ptr((void *)str));
    }
    else
    {
        /* 回退到标准输出 */
        printf("%s", str);
        fflush(stdout);
    }
}

/**
 * @brief Shell 输出单个字符
 * @param ch 要输出的字符
 */
static void shell_putchar(char ch)
{
    char buf[2] = {ch, '\0'};
    shell_puts(buf);
}

/**
 * @brief Shell 格式化输出
 * @param fmt 格式字符串
 * @param ... 可变参数
 */
static void shell_printf(const char *fmt, ...)
{
    static char shell_print_buf[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(shell_print_buf, sizeof(shell_print_buf), fmt, args);
    va_end(args);
    shell_puts(shell_print_buf);
}

/*===========================================================================*/
/*                         Shell 初始化                                       */
/*===========================================================================*/

void MCU_Shell_Init(shell *sh, DeviceFamily *log)
{
    sh->Shell_Init = true;                 // 设置Shell初始化标志为true
    sh->c = 0;                             // 初始化接收字符
    sh->Res_len = 0;                       // 初始化接收长度
    sh->UART_NOTE = 0;                     // 初始化串口节点
    sh->RunStae = 0;                       // 初始化运行状态
    memset(sh->Data, 0, sizeof(sh->Data)); // 清空数据缓冲区

    shell_printf("SHELL_VERSION: %d.%d.%d\n", SHELL_VERSION_MAIN, SHELL_VERSION_RE,
                 SHELL_VERSION_UPDATE); // 显示版本信息
    if (log == NULL)
    {
        static DeviceFamily default_log = {.Architecture = "Unknown",
                                           .User = "User",
                                           .Password = "Unknown",
                                           .DeviceName = "UnknownMCU",
                                           .OS = "Unknown",
                                           .Device = "Unknown",
                                           .Version = "Unknown"};
        log = &default_log; // 如果日志信息为空，使用默认值
    }
    else
    {
        if (log->Architecture != NULL)
        {
            shell_printf("Architecture: %s\n", log->Architecture);
        }
        else
        {
            shell_printf("Architecture:" FG_RED " Unknown" RESET_ALL "\n");
        }
        if (log->DeviceName != NULL)
        {
            shell_printf("Device Name: %s\n", log->DeviceName);
        }
        else
        {
            log->DeviceName = "Unknown";
            shell_printf("Device Name:" FG_RED " Unknown" RESET_ALL "\n");
        }
        if (log->OS != NULL)
        {
            shell_printf("Operating System: %s\n", log->OS);
        }
        else
        {
            shell_printf("Operating System:" FG_RED "No OS" RESET_ALL "\n");
        }
        if (log->Device != NULL)
        {
            shell_printf("Device Model: %s\n", log->Device);
        }
        else
        {
            shell_printf("Device Model:" FG_RED " Unknown" RESET_ALL "\n");
        }
        if (log->Version != NULL)
        {
            shell_printf("Version: %s\n", log->Version);
        }
        else
        {
            shell_printf("Version:" FG_RED " Unknown" RESET_ALL "\n");
        }
        if (log->User != NULL)
        {
            shell_printf("User: %s\n", log->User);
        }
        else
        {
            log->User = "Unknown";
            shell_printf("User:" FG_RED " Unknown" RESET_ALL "\n");
        }
        if (log->Password != NULL)
        {
            shell_printf("Password: %s\n", log->Password);
        }
        else
        {
            shell_printf("Password:" FG_RED " Unknown" RESET_ALL "\n");
        }
    }
    shell_printf("The MCU Shell is start\n");
    shell_printf("If you feel it is useful, please give me a star on GitHub <(^_^)> -> ");
    shell_printf(TEXT_UNDERLINE "https://github.com/ktkuri132/driver_apis.git\n" RESET_ALL);
    shell_printf("Type 'help' for a list of commands.\n\n\n");
    shell_printf(FG_GREEN "%s" RESET_ALL "@%s> ", log->User, log->Device);
}

/**
 * @brief 串口中断处理函数：检测数据格式，接收数据
 * @param Parameters 接收到的字符
 * @param sfp 系统函数指针
 * @param sh Shell结构体
 * @param env 环境变量
 * @param log 设备信息
 */
void BIE_UART(uint8_t Parameters, Sysfpoint *sfp, shell *sh, EnvVar *env,
              DeviceFamily *log)
{
    shell_puts(RESET_ALL);
    sh->c = Parameters;

    // 如果是回车键
    if (sh->c == '\r' || sh->c == '\n')
    {
        sh->Data[sh->Res_len] = '\0'; // 添加字符串结束符
        shell_puts("\n");
        Shell_Deal(sfp, sh, env); // 解析并执行命令
        sh->Res_len = 0;          // 重置输入长度
        shell_printf(FG_GREEN "%s" RESET_ALL "@%s> ", log->User, log->Device);
    }
    // 如果是退格键
    else if (sh->c == '\b' || sh->c == 127)
    {
        if (sh->Res_len > 0)
        {
            sh->Res_len--;       // 删除最后一个字符
            shell_puts("\b \b"); // 在终端上删除字符
        }
    }
    // 其他字符
    else
    {
        if (sh->Res_len < 19)
        {
            sh->Data[sh->Res_len++] = sh->c; // 保存字符
            shell_putchar(sh->c);            // 实时显示字符
        }
    }
}

// 待添加的命令
char *syscmd[20] = {
    "hello", "reset", "poweroff", "help", "exit", "clear", "test", "ls",
    NULL // 命令列表结束标志
};

Cmd_PointerTypeDef Cmd;

extern void Sys_cmd_Init(); // 系统命令初始化函数声明

/**
 * @brief 系统默认处理命令
 * @param cmd 待处理的命令
 * @return 0: 成功，-1: 失败
 */
int8_t Cmd_match(shell *sh, char *cmd, int argc, void **argv)
{
    // 处理命令
    if (strcmp(cmd, "hello") == 0)
    {
        shell_printf("Hello, World!\n");
    }
    else if (strcmp(cmd, "reset") == 0)
    {
        shell_printf("Rebooting...\n");
        if (Cmd.reset != NULL)
        {
            Cmd.reset(NULL, NULL);
        }
        else
        {
            shell_printf(FG_RED "reset command not implemented. Cause is a NULL point\n" RESET_ALL);
        }
    }
    else if (strcmp(cmd, "poweroff") == 0)
    {
        shell_printf("Powering off...\n");
        if (Cmd.poweroff != NULL)
        {
            Cmd.poweroff(NULL, NULL);
        }
        else
        {
            shell_printf(FG_RED "poweroff command not implemented. Cause is a NULL point\n" RESET_ALL);
        }
    }
    else if (strcmp(cmd, "help") == 0)
    {
        shell_printf("Available commands:\n");
        for (int i = 0; syscmd[i] != NULL; i++)
        {
            shell_printf("- %s\n", syscmd[i]);
        }
    }
    else if (strcmp(cmd, "ls") == 0)
    {
        if (Cmd.ls != NULL)
        {
            Cmd.ls(argc, argv);
        }
        else
        {
            shell_printf(FG_RED "ls command not implemented. Cause is a NULL point\n" RESET_ALL);
        }
    }
    else if (strcmp(cmd, "clear") == 0)
    {
        shell_printf("Clearing screen...\n");
        if (Cmd.clear != NULL)
        {
            Cmd.clear(NULL, NULL);
        }
        else
        {
            shell_printf(FG_RED "clear command not implemented. Cause is a NULL point\n" RESET_ALL);
        }
    }
    else if (strcmp(cmd, "exit") == 0)
    {
        shell_printf("Exiting...\n");
        sh->RunStae = 1;
        shell_puts(CLEAR_SCREEN);
    }
    else if (strcmp(cmd, "test") == 0)
    {
        if (Cmd.test != NULL)
        {
            Cmd.test(argc, argv);
        }
        else
        {
            shell_printf(FG_RED "test command not implemented. Cause is a NULL point\n" RESET_ALL);
        }
    }
    else
    {
        return -1; // 命令未找到
    }
    return 0; // 命令处理成功
}

#define MAX_ARGS 20    // 最大参数数量
#define MAX_ARG_LEN 50 // 每个参数的最大长度

/// 任务切换函数
/// @param userEnv: 用户环境变量数组
void Task_Switch(Sysfpoint *sfp, EnvVar *userEnv)
{
    // 假如环境变量过长可采取其他的查找算法:如二分查找等
    // 这里采用线性查找
    int i;
    for (i = 0; userEnv[i].name != NULL; i++)
    {
        if (userEnv[i].RunStae)
        {
            // 执行命令
            sfp->syspfunc = userEnv[i].callback; // 设置系统函数指针
            sfp->argc = userEnv[i].argc;         // 设置参数个数
            sfp->Parameters = userEnv[i].arg;    // 设置参数
            userEnv[i].RunStae = 0;              // 重置运行状态
            return;                              // 跳出循环，避免重复执行
        }
    }
    i = 0; // 重置循环变量
}

/// @brief 主循环命令切换运行函数
/// @param sfp 指向系统函数指针结构体
void Task_Switch_Tick_Handler(Sysfpoint *sfp)
{
    if (sfp->syspfunc != NULL)
    {
        // 执行系统函数
        sfp->syspfunc(sfp->argc, sfp->Parameters);
        sfp->argc = 0;          // 重置参数个数
        sfp->Parameters = NULL; // 清除参数指针
        sfp->syspfunc = NULL;   // 清除系统函数指针
    }
}

/// @brief 处理串口发送的指令
/// @param env_vars 环境变量列表
/// @param sh Shell协议结构体
/// @return 字符串指针
void Shell_Deal(Sysfpoint *sfp, shell *sh, EnvVar *env_vars)
{
    if (sh->Shell_Init == false)
    {
        error("Shell not initialized.\n");
        return; // 如果Shell未初始化，直接返回
    }
    char *input = (char *)(sh->Data);
    char *args[MAX_ARGS];
    int arg_count = 0;

    // 使用 strtok 分割输入字符串
    char *token = strtok(input, " ");
    while (token != NULL && arg_count < MAX_ARGS)
    {
        args[arg_count] = token; // 保存参数
        arg_count++;
        token = strtok(NULL, " ");
    }

    if (arg_count == 0)
    {
        return;
    }

    char *cmd_part = args[0];                                   // 提取命令部分
    void *arg_part = (arg_count > 1) ? (void *)&args[1] : NULL; // 提取参数部分

    // 遍历命令列表，匹配命令
    for (int i = 0; i < 20; i++)
    {
        if (syscmd[i] == NULL)
            break;
        if (strcmp(cmd_part, syscmd[i]) == 0)
        {
            shell_printf("Executing command: %s\n", syscmd[i]);
            if (Cmd_match(sh, syscmd[i], arg_count, arg_part) < 0)
            {
                shell_printf(FG_RED "Command not found: %s\n" RESET_ALL, syscmd[i]);
            }
            return;
        }
    }

    // 如果未匹配到默认命令，检查环境变量
    for (int i = 0; env_vars[i].name != NULL; i++)
    {
        if (strcmp(cmd_part, env_vars[i].name) == 0)
        {
            shell_printf("Executing environment variable command: %s\n", env_vars[i].name);
            env_vars[i].RunStae = 1;
            env_vars[i].arg = arg_part;
            env_vars[i].argc = arg_count;
            Task_Switch(sfp, env_vars);
            return;
        }
    }

    // 未匹配到命令
    if (cmd_part != NULL)
    {
        shell_printf(FG_RED "Command not found: %s\n" RESET_ALL, cmd_part);
    }
}
