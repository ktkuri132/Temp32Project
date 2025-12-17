#ifndef __FONTS_H
#define __FONTS_H
#include <lcd/df_lcd.h>

// 字体选择宏定义
// #define SONG_TI_Font8x16
#define JetBrains_Mono_8x16
// #define Consolas_8x16
// #define LI_SHU_Font6x8

#ifdef SONG_TI_Font8x16
extern const uint8_t SONG_TI_Font8x16_Table[][16];
extern LCD_Font_t SONG_TI_Font_8x16;
#elif defined JetBrains_Mono_8x16
extern const uint8_t JetBrains_Mono_Font8x16_Table[][16];
extern LCD_Font_t JetBrains_Mono_Font_8x16;
#elif defined Consolas_Font8x16
extern const uint8_t Consolas_Font8x16[][16];
extern LCD_Font_t Consolas_Font_8x16;
#elif defined LI_SHU_Font6x8
extern const uint8_t LI_SHU_Font6x8_Table[][8];
extern LCD_Font_t LI_SHU_Font_6x8;
#endif

#endif