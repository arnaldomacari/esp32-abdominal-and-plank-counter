#include "imu.h"
#include "i2c_bus.h"

#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "esp_log.h"

#define IMU_ADDR            0x68

#define ICM_WHO_AM_I        0x75
#define ICM_REG_BANK_SEL    0x76
#define ICM_BANK_0          0x00
#define ICM_PWR_MGMT0       0x1F
#define ICM_ACCEL_DATA_X1   0x0B

#define IMU_TASK_STACK      4096
#define IMU_TASK_PRIORITY   5
#define IMU_READ_PERIOD_MS  100

static const char *TAG = "imu";

static SemaphoreHandle_t imu_data_mutex = NULL;
static imu_data_t imu_data = {0};
static TaskHandle_t imu_task_handle = NULL;

static void imu_update_data(const imu_data_t *new_data)
{
    if (xSemaphoreTake(imu_data_mutex, pdMS_TO_TICKS(20)) == pdTRUE) {
        imu_data = *new_data;
        xSemaphoreGive(imu_data_mutex);
    }
}

imu_data_t imu_get_data(void)
{
    imu_data_t copy = {0};

    if (imu_data_mutex != NULL &&
        xSemaphoreTake(imu_data_mutex, pdMS_TO_TICKS(20)) == pdTRUE) {
        copy = imu_data;
        xSemaphoreGive(imu_data_mutex);
    }

    return copy;
}

esp_err_t imu_init(void)
{
    imu_data_mutex = xSemaphoreCreateMutex();

    if (imu_data_mutex == NULL) {
        ESP_LOGE(TAG, "Falha ao criar mutex da IMU");
        return ESP_FAIL;
    }

    uint8_t whoami = 0;

    esp_err_t ret = i2c_bus_write_reg(IMU_ADDR, ICM_REG_BANK_SEL, ICM_BANK_0);
    ESP_LOGI(TAG, "Selecionando bank 0: %s", esp_err_to_name(ret));

    if (ret != ESP_OK) {
        return ret;
    }

    vTaskDelay(pdMS_TO_TICKS(20));

    ret = i2c_bus_read_reg(IMU_ADDR, ICM_WHO_AM_I, &whoami, 1);

    ESP_LOGI(
        TAG,
        "ICM-42670-P WHO_AM_I ret=%s valor=0x%02X",
        esp_err_to_name(ret),
        whoami
    );

    if (ret != ESP_OK) {
        return ret;
    }

    if (whoami != 0x67) {
        ESP_LOGW(TAG, "WHO_AM_I inesperado. Esperado 0x67");
    }

    ret = i2c_bus_write_reg(IMU_ADDR, ICM_PWR_MGMT0, 0x0F);

    ESP_LOGI(TAG, "Ligando accel/gyro PWR_MGMT0 ret=%s", esp_err_to_name(ret));

    if (ret != ESP_OK) {
        return ret;
    }

    vTaskDelay(pdMS_TO_TICKS(100));

    return ESP_OK;
}

static void imu_task(void *arg)
{
    uint8_t raw[12];
    imu_data_t local_data = {0};

    ESP_LOGI(TAG, "Task da IMU iniciada");

    while (1) {
        esp_err_t ret = i2c_bus_read_reg(
            IMU_ADDR,
            ICM_ACCEL_DATA_X1,
            raw,
            sizeof(raw)
        );

        if (ret == ESP_OK) {
            local_data.ax = (int16_t)((raw[0] << 8) | raw[1]);
            local_data.ay = (int16_t)((raw[2] << 8) | raw[3]);
            local_data.az = (int16_t)((raw[4] << 8) | raw[5]);

            local_data.gx = (int16_t)((raw[6] << 8) | raw[7]);
            local_data.gy = (int16_t)((raw[8] << 8) | raw[9]);
            local_data.gz = (int16_t)((raw[10] << 8) | raw[11]);

            local_data.ax_g = local_data.ax / 2048.0f;
            local_data.ay_g = local_data.ay / 2048.0f;
            local_data.az_g = local_data.az / 2048.0f;

            local_data.angle_deg = atan2f(
                local_data.ax_g,
                sqrtf(
                    (local_data.ay_g * local_data.ay_g) +
                    (local_data.az_g * local_data.az_g)
                )
            ) * 57.2957795f;

            imu_update_data(&local_data);
        } else {
            ESP_LOGW(TAG, "Erro lendo IMU: %s", esp_err_to_name(ret));
        }

        vTaskDelay(pdMS_TO_TICKS(IMU_READ_PERIOD_MS));
    }
}

void imu_start_task(void)
{
    if (imu_task_handle != NULL) {
        ESP_LOGW(TAG, "Task da IMU ja foi iniciada");
        return;
    }

    xTaskCreate(
        imu_task,
        "imu_task",
        IMU_TASK_STACK,
        NULL,
        IMU_TASK_PRIORITY,
        &imu_task_handle
    );
}