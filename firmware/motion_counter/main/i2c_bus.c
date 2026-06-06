#include "i2c_bus.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "esp_log.h"
#include "driver/i2c.h"

#define I2C_PORT      I2C_NUM_0
#define I2C_SDA_GPIO  7
#define I2C_SCL_GPIO  8
#define I2C_FREQ_HZ   100000

static const char *TAG = "i2c_bus";
static SemaphoreHandle_t i2c_mutex = NULL;

esp_err_t i2c_bus_init(void)
{
    if (i2c_mutex != NULL) {
        return ESP_OK;
    }

    i2c_mutex = xSemaphoreCreateMutex();

    if (i2c_mutex == NULL) {
        ESP_LOGE(TAG, "Falha ao criar mutex I2C");
        return ESP_FAIL;
    }

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_SDA_GPIO,
        .scl_io_num = I2C_SCL_GPIO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_FREQ_HZ,
        .clk_flags = 0,
    };

    esp_err_t ret = i2c_param_config(I2C_PORT, &conf);
    if (ret != ESP_OK) {
        return ret;
    }

    ret = i2c_driver_install(I2C_PORT, conf.mode, 0, 0, 0);
    if (ret != ESP_OK) {
        return ret;
    }

    ESP_LOGI(TAG, "I2C iniciado em SDA=%d SCL=%d", I2C_SDA_GPIO, I2C_SCL_GPIO);
    return ESP_OK;
}

void i2c_bus_scan(void)
{
    ESP_LOGI(TAG, "Escaneando barramento I2C...");

    for (uint8_t addr = 1; addr < 127; addr++) {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();

        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
        i2c_master_stop(cmd);

        esp_err_t ret = i2c_master_cmd_begin(I2C_PORT, cmd, pdMS_TO_TICKS(50));
        i2c_cmd_link_delete(cmd);

        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "Encontrado dispositivo em 0x%02X", addr);
        }
    }
}

esp_err_t i2c_bus_write_reg(uint8_t dev_addr, uint8_t reg, uint8_t value)
{
    uint8_t data[2] = {reg, value};

    if (xSemaphoreTake(i2c_mutex, pdMS_TO_TICKS(200)) != pdTRUE) {
        return ESP_ERR_TIMEOUT;
    }

    esp_err_t ret = i2c_master_write_to_device(
        I2C_PORT,
        dev_addr,
        data,
        sizeof(data),
        pdMS_TO_TICKS(200)
    );

    xSemaphoreGive(i2c_mutex);
    return ret;
}

esp_err_t i2c_bus_read_reg(uint8_t dev_addr, uint8_t reg, uint8_t *data, size_t len)
{
    if (xSemaphoreTake(i2c_mutex, pdMS_TO_TICKS(200)) != pdTRUE) {
        return ESP_ERR_TIMEOUT;
    }

    esp_err_t ret = i2c_master_write_read_device(
        I2C_PORT,
        dev_addr,
        &reg,
        1,
        data,
        len,
        pdMS_TO_TICKS(200)
    );

    xSemaphoreGive(i2c_mutex);
    return ret;
}

esp_err_t i2c_bus_write_bytes(uint8_t dev_addr, const uint8_t *data, size_t len)
{
    if (xSemaphoreTake(i2c_mutex, pdMS_TO_TICKS(200)) != pdTRUE) {
        return ESP_ERR_TIMEOUT;
    }

    esp_err_t ret = i2c_master_write_to_device(
        I2C_PORT,
        dev_addr,
        data,
        len,
        pdMS_TO_TICKS(200)
    );

    xSemaphoreGive(i2c_mutex);
    return ret;
}