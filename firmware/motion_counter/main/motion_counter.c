#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"
#include "esp_log.h"

#define I2C_SDA_GPIO 7
#define I2C_SCL_GPIO 8
#define I2C_PORT     I2C_NUM_0

static const char *TAG = "I2C_SCAN";

void app_main(void)
{
    ESP_LOGI(TAG, "Iniciando scan I2C...");

    i2c_master_bus_handle_t bus_handle;

    i2c_master_bus_config_t bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_PORT,
        .sda_io_num = I2C_SDA_GPIO,
        .scl_io_num = I2C_SCL_GPIO,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &bus_handle));

    while (1) {
        ESP_LOGI(TAG, "Procurando dispositivos I2C...");

        for (uint8_t addr = 1; addr < 127; addr++) {
            esp_err_t ret = i2c_master_probe(bus_handle, addr, 100);

            if (ret == ESP_OK) {
                ESP_LOGI(TAG, "Dispositivo encontrado em 0x%02X", addr);
            }
        }

        ESP_LOGI(TAG, "Scan finalizado.\n");
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}