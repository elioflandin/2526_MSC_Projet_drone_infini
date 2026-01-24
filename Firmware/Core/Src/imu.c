/*
 * imu.c
 *
 *  Created on: 24 janv. 2026
 *      Author: kevin
 */

// Compensation de la température
float bmp280_compensate_T(int32_t adc_T)
{
    float var1, var2, T;
    var1 = (((float)adc_T)/16384.0f - ((float)bmp_calib.dig_T1)/1024.0f)
           * ((float)bmp_calib.dig_T2);
    var2 = ((((float)adc_T)/131072.0f - ((float)bmp_calib.dig_T1)/8192.0f) *
           (((float)adc_T)/131072.0f - ((float)bmp_calib.dig_T1)/8192.0f))
           * ((float)bmp_calib.dig_T3);
    t_fine = (int32_t)(var1 + var2);
    T = (var1 + var2) / 5120.0f;
    return T;
}

// Compensation de la pression
float bmp280_compensate_P(int32_t adc_P)
{
    float var1, var2, p;

    var1 = ((float)t_fine / 2.0f) - 64000.0f;
    var2 = var1 * var1 * ((float)bmp_calib.dig_P6) / 32768.0f;
    var2 = var2 + var1 * ((float)bmp_calib.dig_P5) * 2.0f;
    var2 = (var2 / 4.0f) + (((float)bmp_calib.dig_P4) * 65536.0f);
    var1 = (((float)bmp_calib.dig_P3) * var1 * var1 / 524288.0f
           + ((float)bmp_calib.dig_P2) * var1) / 524288.0f;
    var1 = (1.0f + var1 / 32768.0f) * ((float)bmp_calib.dig_P1);

    if (var1 == 0.0f) return 0;

    p = 1048576.0f - (float)adc_P;
    p = (p - (var2 / 4096.0f)) * 6250.0f / var1;
    var1 = ((float)bmp_calib.dig_P9) * p * p / 2147483648.0f;
    var2 = p * ((float)bmp_calib.dig_P8) / 32768.0f;
    p = p + (var1 + var2 + ((float)bmp_calib.dig_P7)) / 16.0f;

    return p;
}

void mpu9250_calibrate_gyro(uint16_t samples)
{
    int32_t gx_sum = 0, gy_sum = 0, gz_sum = 0;
    uint8_t gyro_raw[6];
    int16_t gx, gy, gz;

    printf("Calibration du gyroscope... Gardez le immobile!\r\n");

    for (uint16_t i = 0; i < samples; i++)
    {
        HAL_I2C_Mem_Read(&hi2c3, MPU9250_ADDR << 1,
                         0x43, I2C_MEMADD_SIZE_8BIT,
                         gyro_raw, 6, HAL_MAX_DELAY);

        gx = (gyro_raw[0] << 8) | gyro_raw[1];
        gy = (gyro_raw[2] << 8) | gyro_raw[3];
        gz = (gyro_raw[4] << 8) | gyro_raw[5];

        gx_sum += gx;
        gy_sum += gy;
        gz_sum += gz;

        HAL_Delay(2); // 500 mesures par secondes
    }

    gyro_offset_x = (gx_sum / (float)samples) / 131.0f;
    gyro_offset_y = (gy_sum / (float)samples) / 131.0f;
    gyro_offset_z = (gz_sum / (float)samples) / 131.0f;

    printf("Gyro offsets [dps]: %.2f %.2f %.2f\r\n",
           gyro_offset_x, gyro_offset_y, gyro_offset_z);
}

void mpu9250_calibrate_accel(uint16_t samples)
{
    int32_t ax_sum = 0, ay_sum = 0, az_sum = 0;
    uint8_t accel_raw[6];
    int16_t ax, ay, az;

    printf("Calibration de l'accéléromètre... Gardez le immobile!\r\n");

    for (uint16_t i = 0; i < samples; i++)
    {
        HAL_I2C_Mem_Read(&hi2c3, MPU9250_ADDR << 1,
                         0x3B, I2C_MEMADD_SIZE_8BIT,
                         accel_raw, 6, HAL_MAX_DELAY);

        ax = (accel_raw[0] << 8) | accel_raw[1];
        ay = (accel_raw[2] << 8) | accel_raw[3];
        az = (accel_raw[4] << 8) | accel_raw[5];

        ax_sum += ax;
        ay_sum += ay;
        az_sum += az;

        HAL_Delay(2);
    }

    accel_offset_x = (ax_sum / (float)samples) / 16384.0f;
    accel_offset_y = (ay_sum / (float)samples) / 16384.0f;
    accel_offset_z = (az_sum / (float)samples) / 16384.0f;
    // on ne retire pas 1g finalement car cela donnait une calibration fausse
    // (les 1g etaient retirés 2 fois)

    printf("Accel offsets [g]: %.3f %.3f %.3f\r\n",
           accel_offset_x, accel_offset_y, accel_offset_z);
}

void compute_roll_pitch(float ax, float ay, float az,
                        float *roll, float *pitch)
{
    *roll  = atan2f(ay, az) * RAD_TO_DEG;
    *pitch = atan2f(-ax, sqrtf(ay*ay + az*az)) * RAD_TO_DEG;
}

float pid_update(PID_t *pid, float error, float dt)
{
    pid->integral += error * dt;

    float derivative = (error - pid->prev_error) / dt;
    pid->prev_error = error;

    return pid->kp * error
         + pid->ki * pid->integral
         + pid->kd * derivative;
}

uint16_t angle_to_pwm(float angle_deg)
{
    // Saturation à ±PID_MAX
    if (angle_deg > PID_MAX)  angle_deg = PID_MAX;
    if (angle_deg < -PID_MAX) angle_deg = -PID_MAX;

    // Conversion angle → PWM
    // -PID_MAX  -> 1000 µs
    //  0        -> 1500 µs
    // +PID_MAX  -> 2000 µs
    float k = 500.0f / PID_MAX;
    float pulse = 1500.0f + angle_deg * k;

    // Sécurité finale
    if (pulse < 1000.0f) pulse = 1000.0f;
    if (pulse > 2000.0f) pulse = 2000.0f;

    return (uint16_t)pulse;
}
