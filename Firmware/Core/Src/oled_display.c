/*
 * oled_display.c
 *
 *  Created on: Jan 14, 2026
 *      Author: kevin
 */

/**
 * @file oled_display.c
 * @brief Fonctions d'affichage pour l'OLED.
 */
#include "oled_display.h"
#include "sh1106.h"
#include "fonts.h"
#include <stdio.h>


extern I2C_HandleTypeDef hi2c1;

void OLED_Update(uint8_t isArmed, uint32_t motorSpeedPercent, uint8_t isConnected) {
    SH1106_Clear();

    // 1. Affichage de la connexion (en haut à droite ou ligne dédiée)
    SH1106_GotoXY(0, 0);
    if (isConnected) {
        SH1106_Puts("BT: OK", &Font_7x10, 1);
    } else {
        SH1106_Puts("BT: DISCONNECTED", &Font_7x10, 1);
    }

    // 2. Affichage de l'état (décalé un peu plus bas)
    SH1106_GotoXY(0, 12);
    if (isArmed) {
        SH1106_Puts("STATUS: ARMED", &Font_7x10, 1);
    } else {
        SH1106_Puts("STATUS: LOCKED", &Font_7x10, 1);
    }

    // 3. Affichage de la vitesse
    SH1106_GotoXY(0, 25);
    char buffer[20];
    sprintf(buffer, "SPEED: %lu%%", motorSpeedPercent);
    SH1106_Puts(buffer, &Font_11x18, 1);

    // 4. Dessin d'une barre de progression
    SH1106_DrawRectangle(0, 50, 127, 10, 1);
    uint32_t barWidth = (motorSpeedPercent * 127) / 100;
    if (barWidth > 127) barWidth = 127;
    SH1106_DrawFilledRectangle(0, 50, barWidth, 10, 1);

    SH1106_UpdateScreen();
}
