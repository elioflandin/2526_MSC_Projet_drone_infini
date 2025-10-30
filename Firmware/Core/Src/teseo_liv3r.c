/*
 * teseo_liv3r.c
 *
 *  Created on: Oct 24, 2025
 *      Author: kevin
 */

#include "teseo_liv3r.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Exemple de buffer pour stocker une ligne NMEA */
#define TESEO_RX_BUFFER_SIZE 128
static char rx_buffer[TESEO_RX_BUFFER_SIZE];

/* Initialisation simple : on peut envoyer des commandes NMEA si besoin */
HAL_StatusTypeDef TESEO_Init(UART_HandleTypeDef *huart) {
    // rien de spécial pour initialiser, UART déjà configuré
    return HAL_OK;
}

/* Lecture simple : on lit une ligne NMEA, ici $GNGGA pour la position */
HAL_StatusTypeDef TESEO_ReadData(UART_HandleTypeDef *huart, TESEO_LIV3R_Data_t *data) {
    memset(rx_buffer, 0, TESEO_RX_BUFFER_SIZE);
    uint8_t c;
    uint16_t idx = 0;

    // Lecture caractère par caractère jusqu'au \n ou timeout
    uint32_t start = HAL_GetTick();
    while (idx < TESEO_RX_BUFFER_SIZE - 1) {
        if (HAL_UART_Receive(huart, &c, 1, 10) == HAL_OK) {
            if (c == '\n') break;
            rx_buffer[idx++] = c;
        }
        if (HAL_GetTick() - start > TESEO_UART_TIMEOUT) {
            return HAL_TIMEOUT;
        }
    }

    rx_buffer[idx] = '\0';

    // Vérifier si c'est une trame GGA
    if (strncmp(rx_buffer, "$GNGGA", 6) != 0) {
        return HAL_ERROR; // autre trame
    }

    // Découper la trame NMEA par les virgules
    char *tokens[15] = {0};
    char *ptr = strtok(rx_buffer, ",");
    int tok_idx = 0;
    while (ptr != NULL && tok_idx < 15) {
        tokens[tok_idx++] = ptr;
        ptr = strtok(NULL, ",");
    }

    if (tok_idx < 10) return HAL_ERROR;

    // Status fix : 0=invalid, 1=GPS fix, 2=DGPS fix
    data->fix = (tokens[6][0] > '0');

    if (data->fix) {
        // Latitude : ddmm.mmmm
        double lat = atof(tokens[2]);
        int lat_deg = (int)(lat / 100);
        double lat_min = lat - lat_deg * 100;
        data->latitude = lat_deg + lat_min / 60.0;
        if (tokens[3][0] == 'S') data->latitude = -data->latitude;

        // Longitude : dddmm.mmmm
        double lon = atof(tokens[4]);
        int lon_deg = (int)(lon / 100);
        double lon_min = lon - lon_deg * 100;
        data->longitude = lon_deg + lon_min / 60.0;
        if (tokens[5][0] == 'W') data->longitude = -data->longitude;

        // Altitude en mètres
        data->altitude = atof(tokens[9]);

        // vitesse et cap non disponibles dans GGA (option : lire RMC)
        data->speed = 0;
        data->course = 0;
    } else {
        data->latitude = 0;
        data->longitude = 0;
        data->altitude = 0;
        data->speed = 0;
        data->course = 0;
    }

    return HAL_OK;
}

void TESEO_Print(const TESEO_LIV3R_Data_t *data) {
    if (data->fix) {
        printf("GPS Fix: Lat %.6f | Lon %.6f | Alt %.2f m\r\n",
               data->latitude, data->longitude, data->altitude);
    } else {
        printf("GPS No Fix\r\n");
    }
}
