#include "shell.h"

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

void MCU_Shell_Init(shell *sh, DeviceFamily *log) {
    sh->Shell_Init = true;                  // 设置Shell初始化标志为true
    sh->c = 0;                              // 初始化接收字符
    sh->Res_len = 0;                        // 初始化接收长度
    sh->UART_NOTE = 0;                      // 初始化串口节点
    sh->RunStae = 0;                        // 初始化运行状态
    memset(sh->Data, 0, sizeof(sh->Data));  // 清空数据缓冲区
    printf(CLEAR_SCREEN);                   // 清屏
    printf(CURSOR_HOME);                    // 光标回到起始位置
    printf("SHELL_VERSION: %d.%d.%d\n", SHELL_VERSION_MAIN, SHELL_VERSION_RE,
           SHELL_VERSION_UPDATE);  // 显示版本信息
    if (log == NULL) {
        static DeviceFamily default_log = {.Architecture = "Unknown",
                                           .User = "User",
                                           .Password = "Unknown",
                                           .DeviceName = "UnknownMCU",
                                           .OS = "Unknown",
                                           .Device = "Unknown",
                                           .Version = "Unknown"};
        log = &default_log;  // 如果日志信息为空，使用默认值
    } else {
        if (log->Architecture != NULL) {
            printf("Architecture: %s\n", log->Architecture);  // 显示处理器架构
        } else {
            printf("Architecture:" FG_RED " Unknown" RESET_ALL "\n");
        }
        if (log->DeviceName != NULL) {
            printf("Device Name: %s\n", log->DeviceName);  // 显示设备名称
        } else {
            log->DeviceName = "Unknown";  // 如果设备名称为空，设置为"Unknown"
            printf("Device Name:" FG_RED " Unknown" RESET_ALL "\n");
        }
        if (log->OS != NULL) {
            printf("Operating System: %s\n", log->OS);  // 显示操作系统
        } else {
            printf("Operating System:" FG_RED "No OS" RESET_ALL "\n");
        }
        if (log->Device != NULL) {
            printf("Device Model: %s\n", log->Device);  // 显示设备型号
        } else {
            printf("Device Model:" FG_RED " Unknown" RESET_ALL "\n");
        }
        if (log->Version != NULL) {
            printf("Version: %s\n", log->Version);  // 显示版本信息
        } else {
            printf("Version:" FG_RED " Unknown" RESET_ALL "\n");
        }
        if (log->User != NULL) {
            printf("User: %s\n", log->User);  // 显示用户名
        } else {
            log->User = "Unknown";  // 如果用户名为空，设置为"Unknown"
            printf("User:" FG_RED " Unknown" RESET_ALL "\n");
        }
        if (log->Password != NULL) {
            printf("Password: %s\n", log->Password);  // 显示密码
        } else {
            printf("Password:" FG_RED " Unknown" RESET_ALL "\n");
        }
    }
    printf("The MCU Shell is start\n");  // 欢迎信息
    printf(
        "If you feel it is useful, please give me a star on GitHub <(^_^)> "
        "-> ");
    printf(
        TEXT_UNDERLINE
        "https:\/\/github.com\/ktkuri132\/driver_apis.git\n" RESET_ALL);  // GitHub提示
    printf(
        "Type 'help' for a list of commands.\n\n\n");  // 提示用户输入帮助命令
    printf("" FG_GREEN "%s" RESET_ALL "@%s> ", log->User,
           log->Device);  // 显示提示符
    fflush(stdout);       // 刷新输出缓冲区
}

// 串口1中断处理函数：检测数据格式，接收数据
void BIE_UART(void *Parameters, Sysfpoint *sfp, shell *sh, EnvVar *env,
              DeviceFamily *log) {
    printf(RESET_ALL);
    sh->Data_Receive(Parameters, &(sh->c));  // 接收数据
    // 如果是回车键
    if (sh->c == '\r' || sh->c == '\n') {
        sh->Data[sh->Res_len] = '\0';  // 添加字符串结束符
        printf("\n");                  // 换行
        Shell_Deal(sfp, sh, env);      // 解析并执行命令
        sh->Res_len = 0;               // 重置输入长度
        printf("" FG_GREEN "%s" RESET_ALL "@%s> ", log->User,
               log->Device);  // 显示提示符
        fflush(stdout);
    }
    // 如果是退格键
    else if (sh->c == '\b' || sh->c == 127) {
        if (sh->Res_len > 0) {
            sh->Res_len--;    // 删除最后一个字符
            printf("\b \b");  // 在终端上删除字符
            fflush(stdout);
        }
    }
    // 其他字符
    else {
        if (sh->Res_len < 19) {
            sh->Data[sh->Res_len++] = sh->c;  // 保存字符
            printf("%c", sh->c);              // 实时显示字符
            fflush(stdout);
        }
    }
}

// 待添加的命令
char *syscmd[20] = {
    "hello", "reset", "poweroff", "help", "exit", "clear", "test", "ls",
    NULL  // 命令列表结束标志
};

Cmd_PointerTypeDef Cmd;

extern void Sys_cmd_Init();  // 系统命令初始化函数声明

/**
 * @brief 系统默认处理命令
 * @param cmd 待处理的命令
 * @return 0: 成功，-1: 失败
 */
int8_t Cmd_match(shell *sh, char *cmd, int argc, void **argv) {
    // 处理命令
    if (strcmp(cmd, "hello") == 0) {
        printf("Hello, World!\n");
    } else if (strcmp(cmd, "reset") == 0) {
        printf("Rebooting...\n");
        if (Cmd.reset != NULL) {
            Cmd.reset(NULL, NULL);  // 调用重启函数
        } else {
            printf(FG_RED
                   "reset command not implemented.Cause is a NULL "
                   "point\n" RESET_ALL);
        }
    } else if (strcmp(cmd, "poweroff") == 0) {
        printf("Powering off...\n");
        if (Cmd.poweroff != NULL) {
            Cmd.poweroff(NULL, NULL);  // 调用关机函数
        } else {
            printf(FG_RED
                   "poweroff command not implemented.Cause is a NULL "
                   "point\n" RESET_ALL);
        }
    } else if (strcmp(cmd, "help") == 0) {
        printf("Available commands:\n");
        for (int i = 0; syscmd[i] != NULL; i++) {
            printf("- %s\n", syscmd[i]);
        }
    } else if (strcmp(cmd, "ls") == 0) {
        if (Cmd.ls != NULL) {
            Cmd.ls(argc, argv);  // 调用ls函数
        } else {
            printf(
                FG_RED
                "ls command not implemented.Cause is a NULL point\n" RESET_ALL);
        }
    } else if (strcmp(cmd, "clear") == 0) {
        printf("Clearing screen...\n");
        if (Cmd.clear != NULL) {
            Cmd.clear(NULL, NULL);  // 调用清屏函数
        } else {
            printf(FG_RED
                   "clear command not implemented.Cause is a NULL "
                   "point\n" RESET_ALL);
        }
    } else if (strcmp(cmd, "exit") == 0) {
        printf("Exiting...\n");
        sh->RunStae = 1;  // 设置运行状态为1，表示退出
        printf(CLEAR_SCREEN);
    } else if (strcmp(cmd, "test") == 0) {
        if (Cmd.test != NULL) {
            Cmd.test(argc, argv);  // 调用test函数
        } else {
            printf(FG_RED
                   "test command not implemented.Cause is a NULL "
                   "point\n" RESET_ALL);
        }
    } else {
        return -1;  // 命令未找到
    }
    return 0;  // 命令处理成功
}

#define MAX_ARGS 20     // 最大参数数量
#define MAX_ARG_LEN 50  // 每个参数的最大长度

/// 任务切换函数
/// @param userEnv: 用户环境变量数组
void Task_Switch(Sysfpoint *sfp, EnvVar *userEnv) {
    // 假如环境变量过长可采取其他的查找算法:如二分查找等
    // 这里采用线性查找
    int i;
    for (i = 0; userEnv[i].name != NULL; i++) {
        if (userEnv[i].RunStae) {
            // 执行命令
            sfp->syspfunc = userEnv[i].callback;  // 设置系统函数指针
            sfp->argc = userEnv[i].argc;          // 设置参数个数
            sfp->Parameters = userEnv[i].arg;     // 设置参数
            userEnv[i].RunStae = 0;               // 重置运行状态
            return;                               // 跳出循环，避免重复执行
        }
    }
    i = 0;  // 重置循环变量
}

/// @brief 主循环命令切换运行函数
/// @param sfp 指向系统函数指针结构体
void Task_Switch_Tick_Handler(Sysfpoint *sfp){
    if(sfp->syspfunc != NULL) {
        // 执行系统函数
        sfp->syspfunc(sfp->argc, sfp->Parameters);
        sfp->argc = 0;  // 重置参数个数
        sfp->Parameters = NULL;  // 清除参数指针
        sfp->syspfunc = NULL;  // 清除系统函数指针
    }
}


/// @brief 处理串口发送的指令
/// @param env_vars 环境变量列表
/// @param sh Shell协议结构体
/// @return 字符串指针
void Shell_Deal(Sysfpoint *sfp, shell *sh, EnvVar *env_vars) {
    if(sh->Shell_Init == false) {
        error("Shell not initialized.\n");
        return;  // 如果Shell未初始化，直接返回
    }
    char *input = (char *)(sh->Data);
    char *args[MAX_ARGS];
    int arg_count = 0;

    // 使用 strtok 分割输入字符串
    char *token = strtok(input, " ");
    while (token != NULL && arg_count < MAX_ARGS) {
        args[arg_count] = token;  // 保存参数
        arg_count++;
        token = strtok(NULL, " ");
    }

    if (arg_count == 0) {
        return;
    }

    char *cmd_part = args[0];                                    // 提取命令部分
    void *arg_part = (arg_count > 1) ? (void *)&args[1] : NULL;  // 提取参数部分
    // 遍历命令列表，匹配命令
    for (int i = 0; i < 20; i++) {
        if (syscmd[i] == NULL) break;  // 如果命令列表结束
        if (strcmp(cmd_part, syscmd[i]) == 0) {
            // 匹配到命令
            printf("Executing command: %s\n", syscmd[i]);
            if (Cmd_match(sh, syscmd[i], arg_count, arg_part) < 0) {
                printf(FG_RED "Command not found: %s\n" RESET_ALL, syscmd[i]);
            }
            return;
        }
    }
    // 如果未匹配到默认命令，检查环境变量
    for (int i = 0; env_vars[i].name != NULL; i++) {
        if (strcmp(cmd_part, env_vars[i].name) == 0) {
            // 匹配到环境变量
            printf("Executing environment variable command: %s\n",
                   env_vars[i].name);
            env_vars[i].RunStae = 1;       // 设置运行状态为1，表示执行命令
            env_vars[i].arg = arg_part;    // 设置参数
            env_vars[i].argc = arg_count;  // 设置参数个数
            Task_Switch(sfp, env_vars);    // 执行任务切换
            return;
        }
    }
    // 未匹配到命令
    if (cmd_part != NULL) {
        printf(FG_RED "Command not found: %s\n" RESET_ALL, cmd_part);
    }
}
