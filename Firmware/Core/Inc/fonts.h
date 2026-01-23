/*
 * fonts.h
 *
 *  Created on: Jan 14, 2026
 *      Author: kevin
 */

#ifndef INC_FONTS_H_
#define INC_FONTS_H_

#include <stdint.h>

typedef struct {
    const uint8_t FontWidth;    /*!< Font width in pixels */
    const uint8_t FontHeight;   /*!< Font height in pixels */
    const uint16_t *data;       /*!< Pointer to data font data */
} FontDef;

extern FontDef Font_7x10;
extern FontDef Font_11x18;



#endif /* INC_FONTS_H_ */
