/*
 * sh1106.c
 *
 *  Created on: Jan 14, 2026
 *      Author: kevin
 */

/**
 * @file sh1106.c
 * @brief Implémentation des primitives graphiques pour SH1106
 */
#include "sh1106.h"
#include <string.h>

extern I2C_HandleTypeDef hi2c1;

static void SH1106_WriteCommand(uint8_t command);

// Tampon mémoire pour l'écran (128*64 / 8 bits = 1024 octets)
static uint8_t SH1106_Buffer[1024];
static uint16_t CurrentX = 0;
static uint16_t CurrentY = 0;

void SH1106_Init(void) {
    HAL_Delay(100); // Attente matériel

    SH1106_WriteCommand(0xAE); // Display OFF
    SH1106_WriteCommand(0xA8); // Set Multiplex Ratio
    SH1106_WriteCommand(0x3F);
    SH1106_WriteCommand(0xD3); // Set Display Offset
    SH1106_WriteCommand(0x00);
    SH1106_WriteCommand(0x40); // Set Display Start Line
    SH1106_WriteCommand(0xA1); // Set Segment Re-map
    SH1106_WriteCommand(0xC8); // Set COM Output Scan Direction
    SH1106_WriteCommand(0xDA); // Set COM Pins Hardware Configuration
    SH1106_WriteCommand(0x12);
    SH1106_WriteCommand(0x81); // Set Contrast Control
    SH1106_WriteCommand(0x7F);
    SH1106_WriteCommand(0xA4); // Entire Display ON
    SH1106_WriteCommand(0xA6); // Set Normal Display
    SH1106_WriteCommand(0xD5); // Set Display Clock Divide Ratio
    SH1106_WriteCommand(0x80);
    SH1106_WriteCommand(0x8D); // Charge Pump Control
    SH1106_WriteCommand(0x14);
    SH1106_WriteCommand(0xAF); // Display ON

    SH1106_Clear();
    SH1106_UpdateScreen();
}

// Ajoute ici les fonctions GotoXY, DrawPixel et Puts si elles manquent encore
void SH1106_WriteCommand(uint8_t command) {
    HAL_I2C_Mem_Write(&hi2c1, SH1106_I2C_ADDR, 0x00, 1, &command, 1, 10);
}

void SH1106_Clear(void) {
    memset(SH1106_Buffer, 0, sizeof(SH1106_Buffer));
}

void SH1106_GotoXY(uint16_t x, uint16_t y) {
    CurrentX = x;
    CurrentY = y;
}

void SH1106_DrawPixel(uint16_t x, uint16_t y, uint8_t color) {
    if(x >= SH1106_WIDTH || y >= SH1106_HEIGHT) return;
    if(color) SH1106_Buffer[x + (y / 8) * SH1106_WIDTH] |= (1 << (y % 8));
    else      SH1106_Buffer[x + (y / 8) * SH1106_WIDTH] &= ~(1 << (y % 8));
}

void SH1106_Puts(char* str, FontDef* Font, uint8_t color) {
	if (Font->data == NULL) return; // Évite de lire n'importe quoi
    while (*str) {
        for (uint8_t i = 0; i < Font->FontHeight; i++) {
            uint16_t b = Font->data[(*str - 32) * Font->FontHeight + i];
            for (uint8_t j = 0; j < Font->FontWidth; j++) {
                if ((b << j) & 0x8000) SH1106_DrawPixel(CurrentX + j, CurrentY + i, color);
                else                   SH1106_DrawPixel(CurrentX + j, CurrentY + i, !color);
            }
        }
        CurrentX += Font->FontWidth;
        str++;
    }
}

void SH1106_DrawRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t color) {
    for(uint16_t i=x; i<x+w; i++) { SH1106_DrawPixel(i, y, color); SH1106_DrawPixel(i, y+h-1, color); }
    for(uint16_t i=y; i<y+h; i++) { SH1106_DrawPixel(x, i, color); SH1106_DrawPixel(x+w-1, i, color); }
}

void SH1106_DrawFilledRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t color) {
    for(uint16_t i=x; i<x+w; i++)
        for(uint16_t j=y; j<y+h; j++)
            SH1106_DrawPixel(i, j, color);
}

void SH1106_UpdateScreen(void) {
    for (uint8_t i = 0; i < 8; i++) {
        SH1106_WriteCommand(0xB0 + i);   // Page
        /* Ces deux lignes sont cruciales pour le SH1106 */
        SH1106_WriteCommand(0x00);       // Set Lower Column Address à 2
        SH1106_WriteCommand(0x10);       // Set Higher Column Address à 0

        HAL_I2C_Mem_Write(&hi2c1, SH1106_I2C_ADDR, 0x40, 1, &SH1106_Buffer[SH1106_WIDTH * i], SH1106_WIDTH, 25);
    }
}
