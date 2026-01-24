/*
 * imu.h
 *
 *  Created on: 24 janv. 2026
 *      Author: kevin
 */

#ifndef INC_IMU_H_
#define INC_IMU_H_

#define BMP280_ADDR 	0x76
#define BMP280_ID_REG 	0xD0

#define MPU9250_ADDR  		0x68
#define MPU_WHO_AM_I      	0x75
#define MPU_PWR_MGMT_1    	0x6B
#define MPU_ACCEL_CONFIG  	0x1C
#define MPU_GYRO_CONFIG   	0x1B

#define RAD_TO_DEG (57.2957795f)
#define DT_TO_SEC 1000.0f

#define PID_MAX  10.0f
#define ANGLE_DEADBAND 2.0f

typedef struct {
    uint16_t dig_T1;
    int16_t  dig_T2;
    int16_t  dig_T3;
    uint16_t dig_P1;
    int16_t  dig_P2;
    int16_t  dig_P3;
    int16_t  dig_P4;
    int16_t  dig_P5;
    int16_t  dig_P6;
    int16_t  dig_P7;
    int16_t  dig_P8;
    int16_t  dig_P9;
} BMP280_CalibData;

typedef struct {
    float kp;
    float ki;
    float kd;

    float prev_error;
    float integral;
} PID_t;

BMP280_CalibData bmp_calib;
PID_t pid_roll;
PID_t pid_pitch;



float gyro_offset_x = 0.0f;
float gyro_offset_y = 0.0f;
float gyro_offset_z = 0.0f;

float accel_offset_x = 0.0f;
float accel_offset_y = 0.0f;
float accel_offset_z = 0.0f;

float roll = 0.0f;
float pitch = 0.0f;

float alpha = 0.98f;   // coefficient du filtre
uint32_t last_tick = 0;
int32_t t_fine;

float roll_ref;   // valeur de référence obtenues par la télécommande
float pitch_ref;  // en degrés
float servo_left = 0.0f;	// Valeurs de sorties
float servo_right = 0.0f;	// pour la commande des servos

float bmp280_compensate_T(int32_t adc_T);
float bmp280_compensate_P(int32_t adc_P);
void mpu9250_calibrate_gyro(uint16_t samples);
void mpu9250_calibrate_accel(uint16_t samples);
void compute_roll_pitch(float ax, float ay, float az, float *roll, float *pitch);
float pid_update(PID_t *pid, float error, float dt);
uint16_t angle_to_pwm(float angle_deg);

#endif /* INC_IMU_H_ */
