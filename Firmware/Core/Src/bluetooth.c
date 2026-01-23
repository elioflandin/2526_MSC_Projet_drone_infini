#include "bluetooth.h"
#include <string.h>

#define TX_BUFFER_SIZE 128

static uint8_t rx_byte_hm10;
static uint8_t rx_byte_pc;
static uint8_t tx_buffer[TX_BUFFER_SIZE];
static uint16_t head = 0;
static uint16_t tail = 0;

static UART_HandleTypeDef *hm10_uart;
static UART_HandleTypeDef *pc_uart;

void BT_Init_Loopback(UART_HandleTypeDef *huart_hm10, UART_HandleTypeDef *huart_hc06, UART_HandleTypeDef *huart_putty) {
    hm10_uart = huart_hm10;
    pc_uart   = huart_putty;

    HAL_Delay(500);
    // On reste à 9600 pour le moment pour stabiliser
    HAL_UART_Receive_IT(hm10_uart, &rx_byte_hm10, 1);
    HAL_UART_Receive_IT(pc_uart, &rx_byte_pc, 1);

    HAL_UART_Transmit(pc_uart, (uint8_t*)"Liaison PC-BT Active\r\n", 22, 100);
}

void BT_Process_Data(UART_HandleTypeDef *huart) {
    if (huart->Instance == hm10_uart->Instance) {
        // IPHONE -> PC
        HAL_UART_Transmit(pc_uart, &rx_byte_hm10, 1, 10);

        // AJOUT DU SAUT DE LIGNE AUTOMATIQUE si c'est un retour chariot
        if (rx_byte_hm10 == '\r' || rx_byte_hm10 == '\n') {
            uint8_t nl[] = "\n";
            HAL_UART_Transmit(pc_uart, nl, 1, 10);
        }

        HAL_UART_Receive_IT(hm10_uart, &rx_byte_hm10, 1);
    }
    else if (huart->Instance == pc_uart->Instance) {
        // PC -> IPHONE (Mise en buffer pour éviter les pertes)
        // On stocke l'octet reçu dans le tableau
        tx_buffer[head] = rx_byte_pc;
        head = (head + 1) % TX_BUFFER_SIZE;

        // On relance l'écoute PC immédiatement pour ne rien rater
        HAL_UART_Receive_IT(pc_uart, &rx_byte_pc, 1);
    }
}

// Nouvelle fonction à appeler dans le while(1) de main.c
void BT_Flush_Buffer(void) {
    if (head != tail) {
        // Si le buffer n'est pas vide, on envoie un octet au Bluetooth
        if (HAL_UART_Transmit(hm10_uart, &tx_buffer[tail], 1, 10) == HAL_OK) {
            tail = (tail + 1) % TX_BUFFER_SIZE;
        }
    }
}
