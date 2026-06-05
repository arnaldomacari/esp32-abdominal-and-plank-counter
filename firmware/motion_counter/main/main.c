#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "esp_log.h"
#include "esp_err.h"
#include "driver/i2c.h"

#define I2C_PORT            I2C_NUM_0
#define I2C_SDA_GPIO        7
#define I2C_SCL_GPIO        8
#define I2C_FREQ_HZ         100000

#define IMU_ADDR            0x68

#define ICM_WHO_AM_I        0x75
#define ICM_REG_BANK_SEL    0x76
#define ICM_BANK_0          0x00
#define ICM_PWR_MGMT0       0x1F
#define ICM_ACCEL_DATA_X1   0x0B

#define IMU_TASK_STACK      4096
#define IMU_TASK_PRIORITY   5
#define IMU_READ_PERIOD_MS  100

static const char *TAG = "motion_counter";

static SemaphoreHandle_t i2c_mutex = NULL;

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

static imu_data_t imu_data = {0};

static esp_err_t i2c_shared_write_reg(uint8_t dev_addr, uint8_t reg, uint8_t value)
{
    esp_err_t ret;
    uint8_t data[2] = {reg, value};

    if (xSemaphoreTake(i2c_mutex, pdMS_TO_TICKS(200)) != pdTRUE) {
        return ESP_ERR_TIMEOUT;
    }

    ret = i2c_master_write_to_device(
        I2C_PORT,
        dev_addr,
        data,
        sizeof(data),
        pdMS_TO_TICKS(200)
    );

    xSemaphoreGive(i2c_mutex);
    return ret;
}

static esp_err_t i2c_shared_read_reg(uint8_t dev_addr, uint8_t reg, uint8_t *data, size_t len)
{
    esp_err_t ret;

    if (xSemaphoreTake(i2c_mutex, pdMS_TO_TICKS(200)) != pdTRUE) {
        return ESP_ERR_TIMEOUT;
    }

    ret = i2c_master_write_read_device(
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

static esp_err_t i2c_init(void)
{
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

    ESP_ERROR_CHECK(i2c_param_config(I2C_PORT, &conf));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_PORT, conf.mode, 0, 0, 0));

    ESP_LOGI(TAG, "I2C classico iniciado em SDA=%d SCL=%d", I2C_SDA_GPIO, I2C_SCL_GPIO);

    return ESP_OK;
}

static void i2c_scan(void)
{
    ESP_LOGI(TAG, "Escaneando barramento I2C...");

    for (uint8_t addr = 1; addr < 127; addr++) {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();

        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
        i2c_master_stop(cmd);

        esp_err_t ret = i2c_master_cmd_begin(
            I2C_PORT,
            cmd,
            pdMS_TO_TICKS(50)
        );

        i2c_cmd_link_delete(cmd);

        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "Encontrado dispositivo em 0x%02X", addr);
        }
    }
}

static esp_err_t imu_init(void)
{
    uint8_t whoami = 0;

    esp_err_t ret;

    ret = i2c_shared_write_reg(IMU_ADDR, ICM_REG_BANK_SEL, ICM_BANK_0);
    ESP_LOGI(TAG, "Selecionando bank 0: %s", esp_err_to_name(ret));

    vTaskDelay(pdMS_TO_TICKS(20));

    ret = i2c_shared_read_reg(IMU_ADDR, ICM_WHO_AM_I, &whoami, 1);

    ESP_LOGI(
        TAG,
        "ICM-42670-P WHO_AM_I ret=%s valor=0x%02X",
        esp_err_to_name(ret),
        whoami
    );

    if (ret != ESP_OK) {
        return ret;
    }

    ret = i2c_shared_write_reg(IMU_ADDR, ICM_PWR_MGMT0, 0x0F);

    ESP_LOGI(
        TAG,
        "Ligando accel/gyro PWR_MGMT0 ret=%s",
        esp_err_to_name(ret)
    );

    if (ret != ESP_OK) {
        return ret;
    }

    vTaskDelay(pdMS_TO_TICKS(100));

    return ESP_OK;
}

static void imu_task(void *arg)
{
    uint8_t raw[12];

    ESP_LOGI(TAG, "Task da IMU iniciada");

    while (1) {
        esp_err_t ret = i2c_shared_read_reg(
            IMU_ADDR,
            ICM_ACCEL_DATA_X1,
            raw,
            sizeof(raw)
        );

        if (ret == ESP_OK) {
            imu_data.ax = (int16_t)((raw[0] << 8) | raw[1]);
            imu_data.ay = (int16_t)((raw[2] << 8) | raw[3]);
            imu_data.az = (int16_t)((raw[4] << 8) | raw[5]);

            imu_data.gx = (int16_t)((raw[6] << 8) | raw[7]);
            imu_data.gy = (int16_t)((raw[8] << 8) | raw[9]);
            imu_data.gz = (int16_t)((raw[10] << 8) | raw[11]);

            imu_data.ax_g = imu_data.ax / 2048.0f;
            imu_data.ay_g = imu_data.ay / 2048.0f;
            imu_data.az_g = imu_data.az / 2048.0f;

            imu_data.angle_deg = atan2f(
                imu_data.ax_g,
                sqrtf(
                    (imu_data.ay_g * imu_data.ay_g) +
                    (imu_data.az_g * imu_data.az_g)
                )
            ) * 57.2957795f;

            ESP_LOGI(
                TAG,
                "AX:%6d AY:%6d AZ:%6d | GX:%6d GY:%6d GZ:%6d | angulo: %.1f",
                imu_data.ax,
                imu_data.ay,
                imu_data.az,
                imu_data.gx,
                imu_data.gy,
                imu_data.gz,
                imu_data.angle_deg
            );
        } else {
            ESP_LOGW(TAG, "Erro lendo IMU: %s", esp_err_to_name(ret));
        }

        vTaskDelay(pdMS_TO_TICKS(IMU_READ_PERIOD_MS));
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "Contador de abdominal e prancha iniciado");

    ESP_ERROR_CHECK(i2c_init());

    i2c_scan();

    esp_err_t ret = imu_init();

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "IMU nao inicializou: %s", esp_err_to_name(ret));
        return;
    }

    xTaskCreate(
        imu_task,
        "imu_task",
        IMU_TASK_STACK,
        NULL,
        IMU_TASK_PRIORITY,
        NULL
    );
}