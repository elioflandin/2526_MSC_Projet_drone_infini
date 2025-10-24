/*
 * mpl3115a2.c
 *
 *  Created on: Oct 24, 2025
 *      Author: kevin
 */

#include "mpl3115a2.h"
#include <math.h>
#include <stdio.h>

static HAL_StatusTypeDef mpl_write_reg(I2C_HandleTypeDef *hi2c, uint8_t reg, uint8_t val) {
    return HAL_I2C_Mem_Write(hi2c, MPL3115A2_I2C_ADDR_HAL, reg, I2C_MEMADD_SIZE_8BIT, &val, 1, HAL_MAX_DELAY);
}

static HAL_StatusTypeDef mpl_read_regs(I2C_HandleTypeDef *hi2c, uint8_t reg, uint8_t *buf, uint16_t len) {
    return HAL_I2C_Mem_Read(hi2c, MPL3115A2_I2C_ADDR_HAL, reg, I2C_MEMADD_SIZE_8BIT, buf, len, HAL_MAX_DELAY);
}

HAL_StatusTypeDef MPL3115A2_Init(I2C_HandleTypeDef *hi2c) {
    HAL_StatusTypeDef ret;
    uint8_t who = 0;

    /* Read WHO_AM_I */
    ret = mpl_read_regs(hi2c, MPL_REG_WHO_AM_I, &who, 1);
    if (ret != HAL_OK) return ret;

    if (who != MPL_WHO_AM_I_VALUE) {
        /* caller can print an error using printf if desired */
        return HAL_ERROR;
    }

    /* Put device in standby: clear SBYB (bit0) of CTRL_REG1 before writes */
    uint8_t ctrl1 = 0;
    ret = mpl_read_regs(hi2c, MPL_REG_CTRL_REG1, &ctrl1, 1);
    if (ret != HAL_OK) return ret;
    ctrl1 &= ~0x01; /* clear SBYB */
    ret = mpl_write_reg(hi2c, MPL_REG_CTRL_REG1, ctrl1);
    if (ret != HAL_OK) return ret;

    /* Enable data flags in PT_DATA_CFG: enable PDR, TDR, and event flags -> write 0x07 */
    ret = mpl_write_reg(hi2c, MPL_REG_PT_DATA_CFG, 0x07);
    if (ret != HAL_OK) return ret;

    /* Configure for barometer active mode with high oversampling:
       OS[2:0]=111 (128x) and SBYB=1 (active), ALT=0 (barometer)
       CTRL_REG1 = 0x39 is a common choice (0b00111001): OS=111, SBYB=1, ALT=0
       (datasheet examples & libraries use this). */
    ret = mpl_write_reg(hi2c, MPL_REG_CTRL_REG1, 0x39);
    if (ret != HAL_OK) return ret;

    /* Optionally CTRL_REG2 can be left default (0x00) or used to set auto reset etc. */

    /* At this point device is active in barometer mode */
    return HAL_OK;
}

/* Compute altitude from pressure using barometric formula (International Standard Atmosphere).
   p0 = 101325 Pa (sea level). Returns altitude in meters.
*/
static float pressure_to_altitude(float p_pa, float p0_pa) {
    /* altitude = 44330 * (1 - (p/p0)^(1/5.255)) */
    if (p_pa <= 0.0f) return 0.0f;
    return 44330.0f * (1.0f - powf(p_pa / p0_pa, 0.190294957f));
}

/* Read pressure (Pa), temperature (°C) and compute altitude (m).
   This function triggers a one-shot reading if PDR/TDR are not set. */
HAL_StatusTypeDef MPL3115A2_ReadAll(I2C_HandleTypeDef *hi2c, MPL3115A2_Data_t *out) {
    HAL_StatusTypeDef ret;
    uint8_t status = 0;

    /* Read status register */
    ret = mpl_read_regs(hi2c, MPL_REG_STATUS, &status, 1);
    if (ret != HAL_OK) return ret;

    /* PDR bit (pressure data ready) is bit 2 of STATUS. If not ready, toggle OST (one-shot) */
    if ((status & (1 << 2)) == 0) {
        /* Toggle OST: set bit 1 of CTRL_REG1 to 1 to start a measurement */
        uint8_t ctrl1 = 0;
        ret = mpl_read_regs(hi2c, MPL_REG_CTRL_REG1, &ctrl1, 1);
        if (ret != HAL_OK) return ret;
        /* set OST (bit 1) by writing ctrl1 | 0x02 */
        ret = mpl_write_reg(hi2c, MPL_REG_CTRL_REG1, (uint8_t)(ctrl1 | 0x02));
        if (ret != HAL_OK) return ret;

        /* wait until data ready - poll STATUS PDR bit */
        uint32_t t0 = HAL_GetTick();
        do {
            ret = mpl_read_regs(hi2c, MPL_REG_STATUS, &status, 1);
            if (ret != HAL_OK) return ret;
            if ((status & (1 << 2)) != 0) break;
            HAL_Delay(5);
        } while ((HAL_GetTick() - t0) < 500); /* timeout 500 ms */

        if ((status & (1 << 2)) == 0) return HAL_TIMEOUT;
    }

    /* Read pressure(3) + temp(2) in one transaction (auto-increment) registers 0x01..0x05 */
    uint8_t buf[5];
    ret = mpl_read_regs(hi2c, MPL_REG_OUT_P_MSB, buf, 5);
    if (ret != HAL_OK) return ret;

    uint32_t p_raw = ((uint32_t)buf[0] << 16) | ((uint32_t)buf[1] << 8) | (uint32_t)buf[2];
    uint16_t t_raw = ((uint16_t)buf[3] << 8) | (uint16_t)buf[4];

    /* According to datasheet & examples:
       - Pressure raw (24 bits) represents pressure in Pascals times 64 (i.e. value = P * 64).
         Therefore: pressure_pa = p_raw / 64.0
       - Temperature raw (16 bits) represents temperature in °C times 256 (i.e. value = T * 256).
         Therefore: temp_c = t_raw / 256.0
       See datasheet and common libraries for these conversions. */
    float pressure_pa = (float)p_raw / 64.0f;
    float temp_c = (float)t_raw / 256.0f;

    out->pressure_pa = pressure_pa;
    out->temperature_c = temp_c;
    out->altitude_m = pressure_to_altitude(pressure_pa, 101325.0f); /* use sea level default */

    return HAL_OK;
}

void MPL3115A2_Print(const MPL3115A2_Data_t *out) {
    printf("MPL Pressure: %.2f Pa | Temp: %.2f C | Alt: %.2f m\r\n",
           out->pressure_pa, out->temperature_c, out->altitude_m);
}
