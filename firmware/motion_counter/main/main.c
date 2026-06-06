#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_err.h"

#include "i2c_bus.h"
#include "imu.h"
#include "oled.h"

static const char *TAG = "main";

void app_main(void)
{
    ESP_LOGI(TAG, "Contador de abdominal e prancha iniciado");

    ESP_ERROR_CHECK(i2c_bus_init());
    i2c_bus_scan();

    esp_err_t ret = imu_init();

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "IMU nao inicializou: %s", esp_err_to_name(ret));
        return;
    }

    ret = oled_init();

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "OLED nao inicializou: %s", esp_err_to_name(ret));
    } else {
        oled_clear();
        oled_printf(0, 0, "Abdominal");
        oled_printf(0, 1, "IMU OK");
    }

    imu_start_task();

    while (1) {
        imu_data_t data = imu_get_data();

        ESP_LOGI(
            TAG,
            "Angulo atual: %.1f graus | AX:%6d AY:%6d AZ:%6d",
            data.angle_deg,
            data.ax,
            data.ay,
            data.az
        );

        oled_clear();
        oled_printf(0, 0, "Abdominal");
        oled_printf(0, 2, "Angulo:");
        oled_printf(0, 3, "%.1f graus", data.angle_deg);
        oled_printf(0, 5, "AX:%d", data.ax);
        oled_printf(0, 6, "AY:%d", data.ay);
        oled_printf(0, 7, "AZ:%d", data.az);

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}