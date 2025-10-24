#ifndef INC_TESEO_LIV3R_H_
#define INC_TESEO_LIV3R_H_

#include "stm32h7xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

#define TESEO_UART_TIMEOUT 1000 // ms

typedef struct {
    double latitude;   // degrés
    double longitude;  // degrés
    double altitude;   // mètres
    double speed;      // km/h
    double course;     // degrés
    bool fix;          // true si fix GPS disponible
} TESEO_LIV3R_Data_t;

/* API */
HAL_StatusTypeDef TESEO_Init(UART_HandleTypeDef *huart);
HAL_StatusTypeDef TESEO_ReadData(UART_HandleTypeDef *huart, TESEO_LIV3R_Data_t *data);
void TESEO_Print(const TESEO_LIV3R_Data_t *data);

#endif /* INC_TESEO_LIV3R_H_ */
