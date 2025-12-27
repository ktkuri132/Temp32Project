#ifndef __FRONT_H
#define __FRONT_H

// 光标控制
#define CURSOR_UP(n) "\033[" #n "A"                     // 光标上移 n 行
#define CURSOR_DOWN(n) "\033[" #n "B"                   // 光标下移 n 行
#define CURSOR_RIGHT(n) "\033[" #n "C"                  // 光标右移 n 列
#define CURSOR_LEFT(n) "\033[" #n "D"                   // 光标左移 n 列
#define CURSOR_POS(row, col) "\033[" #row ";" #col "H"  // 光标移动到 (row, col)
#define CURSOR_HOME "\033[H"     // 光标移动到左上角 (1,1)
#define CURSOR_SAVE "\033[s"     // 保存光标位置
#define CURSOR_RESTORE "\033[u"  // 恢复光标位置
// 清屏和清除行
#define CLEAR_SCREEN "\033[2J"      // 清屏
#define CLEAR_LINE "\033[K"         // 清除从光标到行尾
#define CLEAR_LINE_START "\033[1K"  // 清除从行首到光标
#define CLEAR_LINE_ALL "\033[2K"    // 清除整行
// 文本样式
#define RESET_ALL "\033[0m"           // 重置所有样式
#define TEXT_BOLD "\033[1m"           // 粗体
#define TEXT_DIM "\033[2m"            // 暗色（通常无效）
#define TEXT_ITALIC "\033[3m"         // 斜体
#define TEXT_UNDERLINE "\033[4m"      // 下划线
#define TEXT_BLINK "\033[5m"          // 闪烁
#define TEXT_REVERSE "\033[7m"        // 反转前景色和背景色
#define TEXT_HIDDEN "\033[8m"         // 隐藏文本（通常无效）
#define TEXT_STRIKETHROUGH "\033[9m"  // 删除线
// 颜色控制
// 前景色
#define FG_BLACK "\033[30m"    // 黑色
#define FG_RED "\033[31m"      // 红色
#define FG_GREEN "\033[32m"    // 绿色
#define FG_YELLOW "\033[33m"   // 黄色
#define FG_BLUE "\033[34m"     // 蓝色
#define FG_MAGENTA "\033[35m"  // 洋红色
#define FG_CYAN "\033[36m"     // 青色
#define FG_WHITE "\033[37m"    // 白色

// 背景色
#define BG_BLACK "\033[40m"    // 黑色背景
#define BG_RED "\033[41m"      // 红色背景
#define BG_GREEN "\033[42m"    // 绿色背景
#define BG_YELLOW "\033[43m"   // 黄色背景
#define BG_BLUE "\033[44m"     // 蓝色背景
#define BG_MAGENTA "\033[45m"  // 洋红色背景
#define BG_CYAN "\033[46m"     // 青色背景
#define BG_WHITE "\033[47m"    // 白色背景

// 256 色模式
#define FG_COLOR_256(n) "\033[38;5;" #n "m"  // 前景色，n 为 0-255
#define BG_COLOR_256(n) "\033[48;5;" #n "m"  // 背景色，n 为 0-255

// RGB 颜色模式
#define FG_COLOR_RGB(r, g, b) "\033[38;2;" #r ";" #g ";" #b "m"  // 前景色
#define BG_COLOR_RGB(r, g, b) "\033[48;2;" #r ";" #g ";" #b "m"  // 背景色
// 其他控制码
#define CURSOR_HIDE "\033[?25l"   // 隐藏光标
#define CURSOR_SHOW "\033[?25h"   // 显示光标
#define CURSOR_GET_POS "\033[6n"  // 获取光标位置
#define TERM_IDENTIFY "\033[c"    // 查询终端类型

#define out_log(fmt, ...) \
    printf(FG_MAGENTA "[LOG][%s:%d][%s] " fmt "\n" RESET_ALL, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define error(fmt, ...) \
    printf(FG_RED "[ERROR][%s:%d][%s] " fmt "\n" RESET_ALL, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define warning(fmt, ...) \
    printf(FG_YELLOW "[WARN][%s:%d][%s] " fmt "\n" RESET_ALL, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define info(fmt, ...) \
    printf(FG_GREEN "[INFO][%s:%d][%s] " fmt "\n" RESET_ALL, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define debug(fmt, ...) \
    printf(FG_BLUE "[DEBUG][%s:%d][%s] " fmt "\n" RESET_ALL, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define success(fmt, ...) \
    printf(FG_CYAN "[SUCCESS][%s:%d][%s] " fmt "\n" RESET_ALL, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#endif