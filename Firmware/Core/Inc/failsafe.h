/*
 * failsafe.h
 *
 *  Created on: Jan 15, 2026
 *      Author: kevin
 */

#ifndef INC_FAILSAFE_H_
#define INC_FAILSAFE_H_

#include "main.h"

// États du système
typedef enum {
    SYSTEM_SAFE = 0,
    SYSTEM_LOST = 1
} Failsafe_State_t;

/**
 * @brief Initialise les périphériques liés à la sécurité
 */
void FAILSAFE_Init(void);

/**
 * @brief Vérifie l'état de la connexion et gère l'alerte
 * @return État actuel (SAFE ou LOST)
 */
Failsafe_State_t FAILSAFE_CheckConnection(void);

void Failsafe_Buzzer_Play(uint32_t duration_ms, uint32_t pitch_delay_us);

#endif /* INC_FAILSAFE_H_ */
