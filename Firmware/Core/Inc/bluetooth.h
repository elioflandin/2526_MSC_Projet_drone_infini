/*
 * bluetooth.h
 *
 *  Created on: Jan 15, 2026
 *      Author: kevin
 */

#ifndef INC_BLUETOOTH_H_
#define INC_BLUETOOTH_H_

#include "main.h"
#include <stdio.h>

// Prototype pour la boucle locale (Loopback)
void BT_Init_Loopback(UART_HandleTypeDef *huart_hm10, UART_HandleTypeDef *huart_hc06, UART_HandleTypeDef *huart_putty);

// Prototype pour le callback
void BT_Process_Data(UART_HandleTypeDef *huart);

void BT_Flush_Buffer(void);

#endif /* INC_BLUETOOTH_H_ */
