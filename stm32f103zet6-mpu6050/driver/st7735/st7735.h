#ifndef __ST7735_H
#define __ST7735_H

#include <stdbool.h>
#include "stfonts.h"


// 1.44" display, default orientation
#define ST7735_IS_128X128 1
#define ST7735_WIDTH  128
#define ST7735_HEIGHT 128
#define ST7735_XSTART 2
#define ST7735_YSTART 3
#define ST7735_ROTATION (ST7735_MADCTL_MX | ST7735_MADCTL_MY | ST7735_MADCTL_BGR)

// Color definitions
#define	ST7735_BLACK   0x0000
#define	ST7735_BLUE    0x001F
#define	ST7735_RED     0xF800
#define	ST7735_GREEN   0x07E0
#define ST7735_CYAN    0x07FF
#define ST7735_MAGENTA 0xF81F
#define ST7735_YELLOW  0xFFE0
#define ST7735_WHITE   0xFFFF
#define ST7735_COLOR565(r, g, b) (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3))


void ST7735_Init(void);
void ST7735_Fill_Rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void ST7735_Fill_Screen(uint16_t color);
void ST7735_Draw_Pixel(uint16_t x, uint16_t y, uint16_t color);
void ST7735_Write_Char(uint16_t x, uint16_t y, char ch, st_fonts_t *font, uint16_t color, uint16_t bgcolor);
void ST7735_Write_Font(uint16_t x, uint16_t y, st_fonts_t *font, uint32_t index, uint16_t color, uint16_t bgcolor);
void ST7735_Write_Fonts(uint16_t x, uint16_t y, st_fonts_t *font, uint32_t index, uint32_t count, uint16_t color, uint16_t bgcolor);
void ST7735_Write_String(uint16_t x, uint16_t y, const char *str, st_fonts_t *font, uint16_t color, uint16_t bgcolor);
void ST7735_Draw_Image(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t *data);
#endif /* __ST7735_H  */
