/*
 * joystick_control.c
 *
 *  Created on: Jan 14, 2026
 *      Author: kevin
 */

/**
 * @file joystick_control.c
 * @brief Implémentation de la logique métier pour le contrôle du moteur et la sécurité.
 * * Ce fichier gère l'acquisition analogique, le pilotage PWM du moteur et la
 * machine à états du bouton d'armement avec retour sonore sur buzzer passif.
 * * @author kevin
 * @date Jan 14, 2026
 */

#include "joystick_control.h"
#include "main.h"

/* Variables externes --------------------------------------------------------*/
/** @brief Handler de l'ADC1 défini dans main.c */
extern ADC_HandleTypeDef hadc2;
/** @brief Handler du Timer 1 défini dans main.c */
extern TIM_HandleTypeDef htim1;

/* Variables privées ---------------------------------------------------------*/
/** * @brief État d'armement du système.
 * @details 0 = Système verrouillé (moteur coupé), 1 = Système armé (contrôle actif).
 */
static uint8_t isArmed = 0;

/* Fonctions privées (Prototypes) --------------------------------------------*/
static void Sound_Armed(void);
static void Sound_Disarmed(void);

/* Fonctions privées (Définitions) -------------------------------------------*/

/**
 * @brief Produit un signal sonore aigu pour confirmer l'armement.
 * @details Utilise JC_Buzzer_Play avec un délai court pour une fréquence haute.
 */
static void Sound_Armed(void)    { JC_Buzzer_Play(600, 2000); }

/**
 * @brief Produit un signal sonore grave pour confirmer le désarmement.
 * @details Utilise JC_Buzzer_Play avec un délai long pour une fréquence basse.
 */
static void Sound_Disarmed(void) { JC_Buzzer_Play(600, 8000); }

/* Fonctions publiques -------------------------------------------------------*/

/**
 * @brief Gère la machine à états du bouton d'armement.
 * @details Détecte un appui maintenu sur le joystick. Si la durée dépasse
 * ARM_TIME_MS, l'état isArmed est inversé et un signal sonore est émis.
 * L'utilisation de la variable actionDone empêche les rebonds logiques
 * si le bouton reste pressé après l'action.
 */
void JC_Handle_Arming(void) {
	static uint32_t pressStartTime = 0;
	static uint8_t buttonPressed = 0;
	static uint8_t actionDone = 0;

	/* Lecture de l'état du bouton (Configuration Pull-up : actif à l'état BAS) */
	if (HAL_GPIO_ReadPin(JS_BUTTON_GPIO_Port, JS_BUTTON_Pin) == GPIO_PIN_RESET) {
		if (!buttonPressed) {
			buttonPressed = 1;
			pressStartTime = HAL_GetTick();
			actionDone = 0;
		}

		/* Vérification du seuil de temps pour l'appui long */
		if (!actionDone && (HAL_GetTick() - pressStartTime >= ARM_TIME_MS)) {
			isArmed = !isArmed; // Bascule de l'état de sécurité

			if (isArmed) {
				Sound_Armed();
			} else {
				Sound_Disarmed();
			}
			actionDone = 1; // Marqueur pour éviter de répéter l'action
		}
	} else {
		/* Réinitialisation lors du relâchement du bouton */
		buttonPressed = 0;
		actionDone = 0;
	}
}

/**
 * @brief Lit la position du joystick et ajuste la vitesse du moteur.
 * @details Si le système est armé, la valeur de l'ADC (0-4095) est convertie
 * linéairement en rapport cyclique PWM (0-499). Si le système est
 * désarmé, le moteur est maintenu à l'arrêt.
 */
void JC_Control_Motor(void) {
    uint32_t joy_val = 0;
    uint32_t current_speed = 0;

    // Valeurs relevées sur PuTTY
    const uint32_t VAL_BAS    = 0;
    const uint32_t VAL_MILIEU = 2730; // Moyenne de tes relevés
    const uint32_t VAL_HAUT   = 4095;
    const uint32_t DEADZONE   = 50;   // Petite marge pour stabiliser le milieu

    HAL_ADC_Start(&hadc2);
    if (HAL_ADC_PollForConversion(&hadc2, 10) == HAL_OK) {
        joy_val = HAL_ADC_GetValue(&hadc2);
    }
    HAL_ADC_Stop(&hadc2);

    if (isArmed) {
        // ZONE 1 : Du bas vers le milieu
        if (joy_val < (VAL_MILIEU - DEADZONE)) {
            // Mappe [VAL_BAS à VAL_MILIEU] -> [0 à PWM_MAX/2]
            current_speed = (joy_val * (PWM_MAX_VALUE / 2)) / (VAL_MILIEU - VAL_BAS);
        }
        // ZONE MORTE : On force le 50% quand on est proche du milieu
        else if (joy_val >= (VAL_MILIEU - DEADZONE) && joy_val <= (VAL_MILIEU + DEADZONE)) {
            current_speed = PWM_MAX_VALUE / 2;
        }
        // ZONE 2 : Du milieu vers le haut
        else {
            // Mappe [VAL_MILIEU à VAL_HAUT] -> [PWM_MAX/2 à PWM_MAX]
            uint32_t input_offset = joy_val - VAL_MILIEU;
            uint32_t input_range  = VAL_HAUT - VAL_MILIEU;
            uint32_t output_start = PWM_MAX_VALUE / 2;
            uint32_t output_range = PWM_MAX_VALUE / 2;

            current_speed = output_start + (input_offset * output_range) / input_range;
        }

        // Sécurité finale
        if (current_speed > PWM_MAX_VALUE) current_speed = PWM_MAX_VALUE;

        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, current_speed);
    } else {
        current_speed = 0;
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
    }

    uint8_t connected = (HAL_GPIO_ReadPin(BT_STATE_GPIO_Port, BT_STATE_Pin) == GPIO_PIN_SET);
    uint32_t speed_percent = (current_speed * 100) / PWM_MAX_VALUE;
    OLED_Update(isArmed, speed_percent,connected);
}

// Ajoute cette fonction pour forcer l'état verrouillé
void JC_Force_Disarm(void) {
    isArmed = 0;
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
    OLED_Update(0, 0);
}

/**
 * @brief Génère une oscillation logicielle pour piloter un buzzer passif.
 * @param duration_ms Durée du son en millisecondes.
 * @param pitch_delay_us Temps d'attente entre deux inversions d'état (définit la fréquence).
 * @note Cette fonction utilise une boucle active (bloquante) pour générer le signal.
 */
void JC_Buzzer_Play(uint32_t duration_ms, uint32_t pitch_delay_us) {
	uint32_t start = HAL_GetTick();

	while((HAL_GetTick() - start) < duration_ms) {
		HAL_GPIO_TogglePin(BUZZER_GPIO_Port, BUZZER_Pin);

		/* Boucle de délai logiciel pour ajuster la tonalité */
		for(volatile int i=0; i < pitch_delay_us; i++);
	}

	/* Assure que la sortie est à l'état bas après le bip */
	HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_RESET);
}

/**
 * @brief Force le moteur à 50% de sa puissance en cas de perte de signal.
 * @details Cette fonction ignore la position du joystick pour la sécurité.
 */
void JC_Force_Safety_Speed(void) {
    // 50% de PWM_MAX_VALUE (499 / 2 = 249)
    uint32_t safety_speed = PWM_MAX_VALUE / 2;

    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, safety_speed);

    // Mise à jour de l'écran pour indiquer le mode sécurité
    OLED_Update(isArmed, 50);
}
