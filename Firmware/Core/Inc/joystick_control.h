/*
 * joystick_control.h
 *
 *  Created on: Jan 14, 2026
 *      Author: kevin
 */

/**
 * @file joystick_control.h
 * @brief Header pour la gestion de l'armement et du moteur via joystick.
 * @author kevin
 * @date Jan 14, 2026
 */

#ifndef INC_JOYSTICK_CONTROL_H_
#define INC_JOYSTICK_CONTROL_H_

#include "main.h"

/** @brief Valeur maximale de l'ADC (12 bits) */
#define ADC_MAX_VALUE 4095
/** @brief Valeur maximale du registre ARR du Timer (Période PWM) */
#define PWM_MAX_VALUE 499
/** @brief Temps requis pour l'appui long (en ms) */
#define ARM_TIME_MS   2000

/**
 * @brief Initialise les périphériques nécessaires au module (optionnel).
 */
void JC_Init(void);

/**
 * @brief Gère l'armement et le désarmement du système.
 * Effectue un toggle de l'état si le bouton est maintenu pendant ARM_TIME_MS.
 */
void JC_Handle_Arming(void);

/**
 * @brief Lit le joystick et pilote le moteur.
 * Mappe la plage ADC sur la plage PWM si le système est armé.
 */
void JC_Control_Motor(void);

/**
 * @brief Produit un signal sonore sur le buzzer passif.
 * @param duration_ms Durée du bip en millisecondes.
 * @param pitch_delay_us Délai pour la fréquence (plus petit = plus aigu).
 */
void JC_Buzzer_Play(uint32_t duration_ms, uint32_t pitch_delay_us);

void JC_Force_Safety_Speed(void);

void JC_Force_Disarm(void);

#endif /* INC_JOYSTICK_CONTROL_H_ */
