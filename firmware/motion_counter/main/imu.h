#ifndef IMU_H
#define IMU_H

#include <stdint.h>
#include "esp_err.h"

typedef struct {
    int16_t ax;
    int16_t ay;
    int16_t az;

    int16_t gx;
    int16_t gy;
    int16_t gz;

    float ax_g;
    float ay_g;
    float az_g;

    float angle_deg;
} imu_data_t;

esp_err_t imu_init(void);
void imu_start_task(void);
imu_data_t imu_get_data(void);

#endif