#ifndef OLED_H
#define OLED_H

#include "esp_err.h"

esp_err_t oled_init(void);
void oled_clear(void);
void oled_set_cursor(int col, int page);
void oled_write_text(const char *text);
void oled_printf(int col, int page, const char *fmt, ...);

#endif