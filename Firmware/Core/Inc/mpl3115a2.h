#ifndef INC_MPL3115A2_H_
#define INC_MPL3115A2_H_

#include "stm32h7xx_hal.h"
#include <stdint.h>

#define MPL3115A2_I2C_ADDR_7BIT  0x60
#define MPL3115A2_I2C_ADDR_HAL   (MPL3115A2_I2C_ADDR_7BIT << 1)

/* Registers (from datasheet) */
#define MPL_REG_STATUS        0x00
#define MPL_REG_OUT_P_MSB     0x01
#define MPL_REG_OUT_P_CSB     0x02
#define MPL_REG_OUT_P_LSB     0x03
#define MPL_REG_OUT_T_MSB     0x04
#define MPL_REG_OUT_T_LSB     0x05
#define MPL_REG_PT_DATA_CFG   0x13
#define MPL_REG_CTRL_REG1     0x26
#define MPL_REG_CTRL_REG2     0x27
#define MPL_REG_WHO_AM_I      0x0C

/* WHO_AM_I expected value */
#define MPL_WHO_AM_I_VALUE    0xC4

typedef struct {
    float pressure_pa;   /* pressure in Pascals */
    float temperature_c; /* temperature in °C */
    float altitude_m;    /* computed altitude in meters (from pressure) */
} MPL3115A2_Data_t;

/* API */
HAL_StatusTypeDef MPL3115A2_Init(I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef MPL3115A2_ReadAll(I2C_HandleTypeDef *hi2c, MPL3115A2_Data_t *out);
void MPL3115A2_Print(const MPL3115A2_Data_t *out);

#endif /* INC_MPL3115A2_H_ */
