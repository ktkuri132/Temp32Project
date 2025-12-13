#ifndef __DF_DISPLAY_H
#define __DF_DISPLAY_H

#include <stdint.h>

/**
 * @brief 显示设备接口结构体
 * 定义了所有显示设备必须实现或可选实现的操作
 */
typedef struct DisplayDevice_t
{
    const char *name; // 设备名称 (如 "OLED", "UART_Term", "LCD")
    uint16_t width;   // 屏幕宽度 (像素或字符列数)
    uint16_t height;  // 屏幕高度 (像素或字符行数)

    // === 终端模式参数 ===
    uint8_t font_width;  // 字体宽度 (像素)
    uint8_t font_height; // 字体高度 (像素)
    uint16_t cursor_x;   // 当前光标X坐标 (由框架维护)
    uint16_t cursor_y;   // 当前光标Y坐标 (由框架维护)

    // === 基础接口 ===
    int (*Init)(void);                        // 初始化
    int (*Clear)(void);                       // 清屏
    int (*SetCursor)(uint16_t x, uint16_t y); // 设置光标位置
    int (*Scroll)(uint16_t lines);            // 向上滚动指定行数 (像素行)

    // === 字符输出接口 ===
    int (*WriteChar)(char c);            // 输出单个字符
    int (*WriteString)(const char *str); // 输出字符串

    // === 图形接口 (可选) ===
    int (*DrawPixel)(uint16_t x, uint16_t y, uint32_t color); // 画点
    int (*Fill)(uint32_t color);                              // 填充颜色

    // 链表指针，用于管理多个设备
    struct DisplayDevice_t *next;
} DisplayDevice_t;

/**
 * @brief 显示框架管理器结构体
 * 提供统一的对外调用接口
 */
typedef struct
{
    /**
     * @brief 注册一个新的显示设备
     * @param dev 设备结构体指针
     * @return 0:成功, -1:失败
     */
    int (*Register)(DisplayDevice_t *dev);

    /**
     * @brief 选择当前活动的显示设备
     * @param name 设备名称
     * @return 0:成功, -1:未找到
     */
    int (*Select)(const char *name);

    /**
     * @brief 向当前设备输出字符串
     * @param str 字符串
     * @return 0:成功, -1:失败
     */
    int (*Output)(const char *str);

    /**
     * @brief 格式化输出 (类似 printf)
     * @param fmt 格式化字符串
     * @param ... 参数
     * @return 输出的字符数
     */
    int (*Printf)(const char *fmt, ...);

    /**
     * @brief 清除当前设备显示
     * @return 0:成功, -1:失败
     */
    int (*Clear)(void);

} DisplayManager_t;

// 全局显示管理器实例
extern DisplayManager_t Display;

#endif // __DF_DISPLAY_H
