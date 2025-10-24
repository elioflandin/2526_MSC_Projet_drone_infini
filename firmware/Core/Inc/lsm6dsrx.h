#ifndef INC_LSM6DSRX_H_
#define INC_LSM6DSRX_H_

#include "stm32h7xx_hal.h"
#include <stdint.h>
#include <stdio.h>

#define LSM6DSRX_I2C_ADDR_7BIT   0x6A
#define LSM6DSRX_I2C_ADDR_HAL    (LSM6DSRX_I2C_ADDR_7BIT << 1)

#define LSM6DSRX_WHO_AM_I        0x0F
#define LSM6DSRX_CTRL1_XL        0x10
#define LSM6DSRX_CTRL2_G         0x11
#define LSM6DSRX_CTRL3_C         0x12
#define LSM6DSRX_OUTX_L_G        0x22

typedef struct {
    float accel_x;
    float accel_y;
    float accel_z;
    float gyro_x;
    float gyro_y;
    float gyro_z;
} LSM6DSRX_Data_t;

HAL_StatusTypeDef LSM6DSRX_Init(I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef LSM6DSRX_ReadData(I2C_HandleTypeDef *hi2c, LSM6DSRX_Data_t *data);
void LSM6DSRX_PrintData(LSM6DSRX_Data_t *data);

#endif /* INC_LSM6DSRX_H_ */
