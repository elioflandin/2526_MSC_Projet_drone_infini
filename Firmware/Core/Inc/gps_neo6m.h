/**
 ******************************************************************************
 * @file    gps_neo6m.h
 * @author  Kevin
 * @date    Decembre 2025
 * @brief   Header file for NEO-6M GPS Driver
 * This file contains the definitions and prototypes for the GPS module
 * interfacing with STM32G431.
 ******************************************************************************
 */

#ifndef INC_GPS_NEO6M_H_
#define INC_GPS_NEO6M_H_

#include "stm32g4xx_hal.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

/**
 * @brief Structure to hold processed GPS Data
 */
typedef struct {
    float latitude;         /*!< Latitude in Decimal Degrees */
    char lat_dir;           /*!< N or S */
    float longitude;        /*!< Longitude in Decimal Degrees */
    char lon_dir;           /*!< E or W */
    float altitude;         /*!< Altitude in meters */
    float speed_kmh;        /*!< Speed in km/h */
    uint8_t satellites;     /*!< Number of satellites used */
    uint8_t fix_quality;    /*!< 0=Invalid, 1=GPS Fix, 2=DGPS Fix */
    char time[12];          /*!< UTC Time string (HHMMSS.SS) */
    uint8_t is_valid;       /*!< Flag to indicate valid data reception */
} GPS_Data_t;

/* Exported functions prototypes ---------------------------------------------*/
void GPS_Init(UART_HandleTypeDef *huart);
void GPS_UART_RxCpltCallback(UART_HandleTypeDef *huart);
void GPS_Process(void);
GPS_Data_t* GPS_GetData(void);

#endif /* INC_GPS_NEO6M_H_ */
