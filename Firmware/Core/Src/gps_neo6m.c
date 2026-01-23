/**
 ******************************************************************************
 * @file    gps_neo6m.c
 * @author  Kevin
 * @date    Decembre 2025
 * @brief   Driver implementation for NEO-6M GPS
 * Handles NMEA parsing (GPGGA, GPRMC) and UART buffer management.
 ******************************************************************************
 */

#include "gps_neo6m.h"

/* Private variables ---------------------------------------------------------*/
#define GPS_BUFFER_SIZE 256
static uint8_t rx_buffer[GPS_BUFFER_SIZE]; // Buffer circulaire (DMA ou IT)
static uint8_t rx_byte;                    // Byte temporaire pour IT
static uint16_t rx_index = 0;
static char line_buffer[GPS_BUFFER_SIZE];  // Ligne complète à parser
static volatile uint8_t line_received = 0; // Flag

static GPS_Data_t gps_data = {0};
static UART_HandleTypeDef *gps_huart;

/* Private function prototypes -----------------------------------------------*/
static void GPS_Parse_GPGGA(char *line);
static void GPS_Parse_GPRMC(char *line);
static float GPS_NMEA_To_Decimal(float nmea_coord);

/**
 * @brief  Initializes the GPS module reception
 * @param  huart: Pointer to UART handle (e.g., &huart1)
 */
void GPS_Init(UART_HandleTypeDef *huart) {
	gps_huart = huart;
	// Démarre la réception caractère par caractère
	HAL_UART_Receive_IT(gps_huart, &rx_byte, 1);
}

/**
 * @brief  Returns the pointer to the latest GPS data struct
 * @retval Pointer to GPS_Data_t
 */
GPS_Data_t* GPS_GetData(void) {
	return &gps_data;
}

/**
 * @brief  Main processing function, call inside while(1)
 */
void GPS_Process(void) {
	if (line_received) {
		// Dispatcher basé sur l'en-tête NMEA
		if (strncmp(line_buffer, "$GPGGA", 6) == 0) {
			GPS_Parse_GPGGA(line_buffer);
		}
		else if (strncmp(line_buffer, "$GPRMC", 6) == 0) {
			GPS_Parse_GPRMC(line_buffer);
		}
		else if (strncmp(line_buffer, "$GNGGA", 6) == 0) {
			// Support pour les modules GNSS mixtes
			GPS_Parse_GPGGA(line_buffer);
		}

		line_received = 0;
	}
}

/**
 * @brief  Callback for UART Interrupt
 * Must be called from HAL_UART_RxCpltCallback in main.c
 * @param  huart: UART handle that triggered the interrupt
 */
void GPS_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart->Instance == gps_huart->Instance) {

		// ***************************************
		// LIGNE DE DEBUG A AJOUTER TEMPORAIREMENT
		// ***************************************
		printf("%c", rx_byte);
		// ***************************************
		// Ancienne vérification : if (rx_byte != '\n' && rx_index < GPS_BUFFER_SIZE - 1)
		// Nouvelle vérification :
		if (rx_byte != '\r' && rx_byte != '\n' && rx_index < GPS_BUFFER_SIZE - 1) {
			rx_buffer[rx_index++] = rx_byte;
		} else if (rx_byte == '\n') { // On attend spécifiquement le LF pour valider la ligne
			// Fin de ligne détectée (avec LF)
			rx_buffer[rx_index] = '\0'; // Terminons la chaîne juste avant le \r ou \n
			// Assurez-vous d'avoir une chaîne valide même si le \r n'a pas été capturé
			// Si rx_index > 0 et le dernier caractère n'était pas un \r, c'est bon.
			if (rx_index > 0 && rx_buffer[rx_index-1] == '\r') {
				rx_buffer[rx_index-1] = '\0'; // Retirer le CR avant de copier
			} else {
				rx_buffer[rx_index] = '\0';
			}

			strcpy(line_buffer, (char *)rx_buffer);
			rx_index = 0;
			line_received = 1;
		}
		// Relance la réception (à placer en dehors du 'if' principal, comme vous l'avez fait)
		HAL_UART_Receive_IT(gps_huart, &rx_byte, 1);
	}
}

/* Private functions implementation ------------------------------------------*/

/**
 * @brief Parse GPGGA sentence (Global Positioning System Fix Data)
 */
static void GPS_Parse_GPGGA(char *line) {
	char *ptr = line;
	char field[20];
	int field_index = 0;
	float raw_lat = 0.0f, raw_lon = 0.0f;

	while((ptr = strchr(ptr, ',')) != NULL) {
		ptr++; // Skip comma
		field_index++;

		char* next_comma = strchr(ptr, ',');
		if(!next_comma) next_comma = strchr(ptr, '*');
		if(!next_comma) break;

		int len = next_comma - ptr;
		if(len >= sizeof(field)) len = sizeof(field)-1;
		strncpy(field, ptr, len);
		field[len] = '\0';

		if (len > 0) {
			switch(field_index) {
			case 1: strcpy(gps_data.time, field); break;       // Time
			case 2: raw_lat = atof(field); break;              // Lat
			case 3: gps_data.lat_dir = field[0]; break;        // N/S
			case 4: raw_lon = atof(field); break;              // Lon
			case 5: gps_data.lon_dir = field[0]; break;        // E/W
			case 6: gps_data.fix_quality = atoi(field); break; // Fix
			case 7: gps_data.satellites = atoi(field); break;  // Sats
			case 9: gps_data.altitude = atof(field); break;    // Alt
			}
		}
	}

	if (gps_data.fix_quality > 0) {
		gps_data.latitude = GPS_NMEA_To_Decimal(raw_lat);
		gps_data.longitude = GPS_NMEA_To_Decimal(raw_lon);
		gps_data.is_valid = 1;
	} else {
		gps_data.is_valid = 0;
	}
}

/**
 * @brief Parse GPRMC sentence (Recommended Minimum Specific GPS/TRANSIT Data)
 */
static void GPS_Parse_GPRMC(char *line) {
	char* ptr = line;
	char field[20];
	int field_index = 0;

	while((ptr = strchr(ptr, ',')) != NULL) {
		ptr++;
		field_index++;

		char* next_comma = strchr(ptr, ',');
		if(!next_comma) next_comma = strchr(ptr, '*');
		if(!next_comma) break;

		int len = next_comma - ptr;
		if(len >= sizeof(field)) len = sizeof(field)-1;
		strncpy(field, ptr, len);
		field[len] = '\0';

		// Champ 7 dans RMC est la vitesse en nœuds
		if (len > 0 && field_index == 7) {
			float speed_knots = atof(field);
			gps_data.speed_kmh = speed_knots * 1.852f;
		}
	}
}

/**
 * @brief Converts NMEA format (DDMM.MMMM) to Decimal Degrees (DD.DDDD)
 * @param nmea_coord: Coordinate in NMEA format
 * @retval Coordinate in Decimal Degrees
 */
static float GPS_NMEA_To_Decimal(float nmea_coord) {
	int degrees = (int)(nmea_coord / 100);
	float minutes = nmea_coord - (degrees * 100);
	return degrees + (minutes / 60.0f);
}
