#include "MS5607.h"

#include <stdlib.h>

#include "BITSv5.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "pico/stdlib.h"

void ms5607_init() {
    i2c_init(i2c0, 100000);

    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
}

void ms5607_reset() {
    uint8_t cmd = MS5607_RESET_CMD;

    i2c_write_blocking(i2c_p, MS5607_ADDR, &cmd, 1, false);
}

uint8_t* ms5607_read_prom(uint8_t* buf) {
    uint8_t cmd = MS5607_READ_PROM_CMD;

    i2c_write_blocking(i2c_p, MS5607_ADDR, &cmd, 1, false);

    i2c_read_blocking(i2c_p, MS5607_ADDR, &buf[0], 1, false);
    i2c_read_blocking(i2c_p, MS5607_ADDR, &buf[1], 1, false);

    return buf;
}

uint8_t* ms5607_conversion(uint8_t* buf) {
    uint8_t cmd = MS5607_PRESS_CONV_CMD;

    i2c_write_blocking(i2c_p, MS5607_ADDR, &cmd, 1, false);

    i2c_read_blocking(i2c_p, MS5607_ADDR, &buf[0], 1, false);
    i2c_read_blocking(i2c_p, MS5607_ADDR, &buf[1], 1, false);
    i2c_read_blocking(i2c_p, MS5607_ADDR, &buf[2], 1, false);

    return buf;
}