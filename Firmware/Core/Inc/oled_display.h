/*
 * oled_display.h
 *
 *  Created on: Jan 14, 2026
 *      Author: kevin
 */

/**
 * @file oled_display.h
 * @brief Gestion de l'affichage OLED SH1106.
 */
#ifndef INC_OLED_DISPLAY_H_
#define INC_OLED_DISPLAY_H_

#include "main.h"

/** @brief Initialise l'écran OLED */
void OLED_Init(void);
/** @brief Met à jour les informations affichées */
void OLED_Update(uint8_t isArmed, uint32_t motorSpeedPercent, uint8_t isConnected);

#endif
