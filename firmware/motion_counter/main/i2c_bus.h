#ifndef I2C_BUS_H
#define I2C_BUS_H

#include <stdint.h>
#include <stddef.h>
#include "esp_err.h"

esp_err_t i2c_bus_init(void);
void i2c_bus_scan(void);

esp_err_t i2c_bus_write_reg(uint8_t dev_addr, uint8_t reg, uint8_t value);
esp_err_t i2c_bus_read_reg(uint8_t dev_addr, uint8_t reg, uint8_t *data, size_t len);

esp_err_t i2c_bus_write_bytes(uint8_t dev_addr, const uint8_t *data, size_t len);

#endif

