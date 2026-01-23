/*
 * sh1106.h
 *
 *  Created on: Jan 14, 2026
 *      Author: kevin
 */

#ifndef INC_SH1106_H_
#define INC_SH1106_H_

#include "main.h"
#include "fonts.h"

#define SH1106_I2C_ADDR        (0x3C << 1)
#define SH1106_WIDTH           128
#define SH1106_HEIGHT          64

void SH1106_Init(void);
void SH1106_Clear(void);
void SH1106_UpdateScreen(void);
void SH1106_GotoXY(uint16_t x, uint16_t y);
void SH1106_DrawPixel(uint16_t x, uint16_t y, uint8_t color);
void SH1106_Puts(char* str, FontDef* Font, uint8_t color);
void SH1106_DrawRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t color);
void SH1106_DrawFilledRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t color);

#endif /* INC_SH1106_H_ */
