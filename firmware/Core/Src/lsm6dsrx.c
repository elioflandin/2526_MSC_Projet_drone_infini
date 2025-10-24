/*
 * lsm6dsrx.c (IMU)
 *
 *  Created on: Oct 24, 2025
 *      Author: kevin
 */

#include "lsm6dsrx.h"

static float accel_lsb_to_g = 0.000061f;   // ±2g
static float gyro_lsb_to_dps = 0.00875f;   // ±245 dps
static float g_to_ms2 = 9.80665f;

HAL_StatusTypeDef LSM6DSRX_Init(I2C_HandleTypeDef *hi2c) {
    uint8_t who_am_i = 0;
    HAL_StatusTypeDef ret;

    ret = HAL_I2C_Mem_Read(hi2c, LSM6DSRX_I2C_ADDR_HAL, LSM6DSRX_WHO_AM_I, I2C_MEMADD_SIZE_8BIT, &who_am_i, 1, HAL_MAX_DELAY);
    if (ret != HAL_OK) return ret;

    if (who_am_i != 0x6B) {
        printf("Erreur : WHO_AM_I = 0x%02X (attendu 0x6B)\r\n", who_am_i);
        return HAL_ERROR;
    }

    // Accéléromètre : 104 Hz, ±2g
    uint8_t ctrl1_xl = 0b01000000;
    ret = HAL_I2C_Mem_Write(hi2c, LSM6DSRX_I2C_ADDR_HAL, LSM6DSRX_CTRL1_XL, I2C_MEMADD_SIZE_8BIT, &ctrl1_xl, 1, HAL_MAX_DELAY);
    if (ret != HAL_OK) return ret;

    // Gyroscope : 104 Hz, 245 dps
    uint8_t ctrl2_g = 0b01000000;
    ret = HAL_I2C_Mem_Write(hi2c, LSM6DSRX_I2C_ADDR_HAL, LSM6DSRX_CTRL2_G, I2C_MEMADD_SIZE_8BIT, &ctrl2_g, 1, HAL_MAX_DELAY);
    if (ret != HAL_OK) return ret;

    // Activation auto-increment et BDU
    uint8_t ctrl3_c = 0b01000100;
    ret = HAL_I2C_Mem_Write(hi2c, LSM6DSRX_I2C_ADDR_HAL, LSM6DSRX_CTRL3_C, I2C_MEMADD_SIZE_8BIT, &ctrl3_c, 1, HAL_MAX_DELAY);

    printf("LSM6DSRX initialisé (WHO_AM_I = 0x%02X)\r\n", who_am_i);
    return ret;
}

HAL_StatusTypeDef LSM6DSRX_ReadData(I2C_HandleTypeDef *hi2c, LSM6DSRX_Data_t *data) {
    uint8_t raw[12];
    HAL_StatusTypeDef ret;

    ret = HAL_I2C_Mem_Read(hi2c, LSM6DSRX_I2C_ADDR_HAL, LSM6DSRX_OUTX_L_G, I2C_MEMADD_SIZE_8BIT, raw, 12, HAL_MAX_DELAY);
    if (ret != HAL_OK) return ret;

    int16_t gx = (int16_t)(raw[1] << 8 | raw[0]);
    int16_t gy = (int16_t)(raw[3] << 8 | raw[2]);
    int16_t gz = (int16_t)(raw[5] << 8 | raw[4]);
    int16_t ax = (int16_t)(raw[7] << 8 | raw[6]);
    int16_t ay = (int16_t)(raw[9] << 8 | raw[8]);
    int16_t az = (int16_t)(raw[11] << 8 | raw[10]);

    data->gyro_x = gx * gyro_lsb_to_dps;
    data->gyro_y = gy * gyro_lsb_to_dps;
    data->gyro_z = gz * gyro_lsb_to_dps;

    data->accel_x = ax * accel_lsb_to_g * g_to_ms2;
    data->accel_y = ay * accel_lsb_to_g * g_to_ms2;
    data->accel_z = az * accel_lsb_to_g * g_to_ms2;

    return HAL_OK;
}

void LSM6DSRX_PrintData(LSM6DSRX_Data_t *data) {
    printf("Acc[m/s2]: X=%.2f Y=%.2f Z=%.2f | Gyro[dps]: X=%.2f Y=%.2f Z=%.2f\r\n",
           data->accel_x, data->accel_y, data->accel_z,
           data->gyro_x, data->gyro_y, data->gyro_z);
}



