#include <stdio.h>
#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "led_strip.h"
#include "esp_log.h"

#define RGB_LED_GPIO 2
#define RGB_LED_COUNT 1

static const char *TAG = "RGB";

static uint32_t wheel(uint8_t pos)
{
    pos = 255 - pos;

    if (pos < 85) {
        return ((255 - pos * 3) << 16) | (0 << 8) | (pos * 3);
    }

    if (pos < 170) {
        pos -= 85;
        return (0 << 16) | ((pos * 3) << 8) | (255 - pos * 3);
    }

    pos -= 170;
    return ((pos * 3) << 16) | ((255 - pos * 3) << 8) | 0;
}

void app_main(void)
{
    ESP_LOGI(TAG, "Inicializando WS2812...");

    led_strip_handle_t led_strip;

    led_strip_config_t strip_config = {
        .strip_gpio_num = RGB_LED_GPIO,
        .max_leds = RGB_LED_COUNT,
    };

    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10 * 1000 * 1000,
        .mem_block_symbols = 64,
    };

    ESP_ERROR_CHECK(
        led_strip_new_rmt_device(
            &strip_config,
            &rmt_config,
            &led_strip
        )
    );

    uint8_t hue = 0;

    while (1) {

        uint32_t color = wheel(hue);

        uint8_t r = (color >> 16) & 0xFF;
        uint8_t g = (color >> 8) & 0xFF;
        uint8_t b = color & 0xFF;

        led_strip_set_pixel(led_strip, 0, r, g, b);
        led_strip_refresh(led_strip);

        hue++;

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}