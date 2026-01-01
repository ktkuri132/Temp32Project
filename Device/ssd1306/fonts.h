#ifndef __SSD1306_FRONT_H
#define __SSD1306_FRONT_H

#include <stdint.h>
#include <ssd1306/ssd1306.h>
#ifdef USE_DEVICE_SSD1306

/*字符集定义*/
/*以下两个宏定义只可解除其中一个的注释*/
#define SSD1306_CHARSET_UTF8 // 定义字符集为UTF8
// #define SSD1306_CHARSET_GB2312		//定义字符集为GB2312

/*字模基本单元*/
typedef struct
{

#ifdef SSD1306_CHARSET_UTF8 // 定义字符集为UTF8
    char Index[5];          // 汉字索引，空间为5字节
#endif

#ifdef SSD1306_CHARSET_GB2312 // 定义字符集为GB2312
    char Index[3];            // 汉字索引，空间为3字节
#endif

    uint8_t Data[32]; // 字模数据
} ChineseCell_t;

/*ASCII字模数据声明*/
extern const uint8_t SSD1306_F8x16[][16];
extern const uint8_t SSD1306_F6x8[][6];

/*汉字字模数据声明*/
extern const ChineseCell_t SSD1306_CF16x16[];

/*图像数据声明*/
extern const uint8_t Diode[];
/*按照上面的格式，在这个位置加入新的图像数据声明*/
//...

#endif
#endif

/*****************江协科技|版权所有****************/
/*****************jiangxiekeji.com*****************/
