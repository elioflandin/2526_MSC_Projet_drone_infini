/*
 * failsafe.c
 *
 *  Created on: Jan 15, 2026
 *      Author: kevin
 */

#include "failsafe.h"

void FAILSAFE_Init(void) {
    // On s'assure que tout est éteint au démarrage
    HAL_GPIO_WritePin(BT_LED_GPIO_Port, BT_LED_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(BT_BUZZ_GPIO_Port, BT_BUZZ_Pin, GPIO_PIN_RESET);
}

Failsafe_State_t FAILSAFE_CheckConnection(void) {
    static uint32_t lastStableTime = 0;
    static GPIO_PinState lastState = GPIO_PIN_RESET;
    GPIO_PinState currentState = HAL_GPIO_ReadPin(BT_STATE_GPIO_Port, BT_STATE_Pin);

    // Si l'état change, on réinitialise le compteur
    if (currentState != lastState) {
        lastStableTime = HAL_GetTick();
        lastState = currentState;
    }

    // On ne valide le changement que s'il est stable depuis plus de 200ms
    if ((HAL_GetTick() - lastStableTime) > 200) {
        if (currentState == GPIO_PIN_RESET) {
            // --- DÉCONNECTÉ (STABLE) ---
            HAL_GPIO_TogglePin(BT_LED_GPIO_Port, BT_LED_Pin);
            Failsafe_Buzzer_Play(1000, 10000);
            return SYSTEM_LOST;
        }
    }

    // --- CONNECTÉ (OU TRANSITION) ---
    HAL_GPIO_WritePin(BT_LED_GPIO_Port, BT_LED_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(BT_BUZZ_GPIO_Port, BT_BUZZ_Pin, GPIO_PIN_RESET);
    return SYSTEM_SAFE;
}

void Failsafe_Buzzer_Play(uint32_t duration_ms, uint32_t pitch_delay_us) {
	uint32_t start = HAL_GetTick();

	while((HAL_GetTick() - start) < duration_ms) {
		HAL_GPIO_TogglePin(BT_BUZZ_GPIO_Port, BT_BUZZ_Pin);

		/* Boucle de délai logiciel pour ajuster la tonalité */
		for(volatile int i=0; i < pitch_delay_us; i++);
	}

	/* Assure que la sortie est à l'état bas après le bip */
	HAL_GPIO_WritePin(BT_BUZZ_GPIO_Port, BT_BUZZ_Pin, GPIO_PIN_RESET);
}
