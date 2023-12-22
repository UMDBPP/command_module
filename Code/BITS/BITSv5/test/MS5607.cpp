#include "MS5607.h"

#include <math.h>
#include <stdlib.h>

#include "BITSv5.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "pico/stdlib.h"

void MS5607::init() {
    i2c_init(i2c0, 100 * 1000);

    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);

    read_prom();
}

int MS5607::reset() {
    uint8_t cmd = MS5607_RESET_CMD;

    if (i2c_write_blocking(i2c, MS5607_ADDR, &cmd, 1, false) ==
        PICO_ERROR_GENERIC)
        return 1;
    else
        read_prom();
    return 0;
}

int MS5607::read_prom() {
    uint8_t cmd = MS5607_READ_C1_CMD;
    uint8_t buf[2] = {0, 0};

    if (i2c_write_blocking(i2c, MS5607_ADDR, &cmd, 1, false) ==
        PICO_ERROR_GENERIC)
        return 1;

    if (i2c_read_blocking(i2c, MS5607_ADDR, buf, 2, false) ==
        PICO_ERROR_GENERIC)
        return 1;

    c1 = ((uint16_t)buf[0] << 8) | buf[1];

    cmd = MS5607_READ_C2_CMD;

    if (i2c_write_blocking(i2c, MS5607_ADDR, &cmd, 1, false) ==
        PICO_ERROR_GENERIC)
        return 1;

    if (i2c_read_blocking(i2c, MS5607_ADDR, buf, 2, false) ==
        PICO_ERROR_GENERIC)
        return 1;

    c2 = ((uint16_t)buf[0] << 8) | buf[1];

    cmd = MS5607_READ_C3_CMD;

    if (i2c_write_blocking(i2c, MS5607_ADDR, &cmd, 1, false) ==
        PICO_ERROR_GENERIC)
        return 1;

    if (i2c_read_blocking(i2c, MS5607_ADDR, buf, 2, false) ==
        PICO_ERROR_GENERIC)
        return 1;

    c3 = ((uint16_t)buf[0] << 8) | buf[1];

    cmd = MS5607_READ_C4_CMD;

    if (i2c_write_blocking(i2c, MS5607_ADDR, &cmd, 1, false) ==
        PICO_ERROR_GENERIC)
        return 1;

    if (i2c_read_blocking(i2c, MS5607_ADDR, buf, 2, false) ==
        PICO_ERROR_GENERIC)
        return 1;

    c4 = ((uint16_t)buf[0] << 8) | buf[1];

    cmd = MS5607_READ_C5_CMD;

    if (i2c_write_blocking(i2c, MS5607_ADDR, &cmd, 1, false) ==
        PICO_ERROR_GENERIC)
        return 1;

    if (i2c_read_blocking(i2c, MS5607_ADDR, buf, 2, false) ==
        PICO_ERROR_GENERIC)
        return 1;

    c5 = ((uint16_t)buf[0] << 8) | buf[1];

    cmd = MS5607_READ_C6_CMD;

    if (i2c_write_blocking(i2c, MS5607_ADDR, &cmd, 1, false) ==
        PICO_ERROR_GENERIC)
        return 1;

    if (i2c_read_blocking(i2c, MS5607_ADDR, buf, 2, false) ==
        PICO_ERROR_GENERIC)
        return 1;

    c6 = ((uint16_t)buf[0] << 8) | buf[1];

    return 0;
}

int MS5607::conversion() {
    uint8_t cmd = MS5607_PRESS_CONV_CMD;
    uint8_t buf[3] = {0, 0, 0};

    i2c_write_blocking(i2c, MS5607_ADDR, &cmd, 1, false);
    i2c_read_blocking(i2c, MS5607_ADDR, &buf[0], 3, false);

    d1 = ((uint32_t)buf[0] << 16) | ((uint32_t)buf[1] << 8) | buf[2];

    cmd = MS5607_TEMP_CONV_CMD;

    i2c_write_blocking(i2c, MS5607_ADDR, &cmd, 1, false);
    i2c_read_blocking(i2c, MS5607_ADDR, &buf[0], 3, false);

    d2 = ((uint32_t)buf[0] << 16) | ((uint32_t)buf[1] << 8) | buf[2];

    return 0;
}

void MS5607::ms5607_get_press_temp(double* pressure, double* temperature) {
    int32_t press = 0;
    int32_t temp = 0;
    int32_t t2 = 0;
    int64_t off2 = 0;
    int64_t sens2 = 0;

    // Step 1: Read calibration data from PROM
    // Already did this in the init()

    // Step 2: Read pressure and temperature from the MS5607
    conversion();

    // The rest of this function mostly looks like random math, it is actually
    // the compensation calculations outline in the datasheet for the device!

    // Step 3: Calculate temperature
    dT = d2 - (c5 * ((uint16_t)256));
    temp = 2000 + (dT * (c6 / ((uint32_t)8388608)));

    // Step 4: Calculate temperature compensated pressure
    off = (c2 * 131072) + ((c4 * dT) / 64);
    sens = (c1 * 65536) + ((c3 * dT) / 128);

    // Second order compensation
    if ((((double)temp) / 100) < 20) {
        t2 = pow(dT, 2) / (int32_t)2147483648;
        off2 = (61 * pow((temp - 2000), 2)) / (int64_t)16;
        sens2 = (2 * pow((temp - 2000), 2));

        if ((((double)temp) / 100) < -15) {
            off2 = off2 + 15 * pow((temp + 1500), 2);
            sens2 = sens2 + 8 * pow((temp + 1500), 2);
        }

        temp = temp - t2;
        off = off - off2;
        sens - sens2;
    }

    press = (((d1 * sens) / 2097152) - off) / 32768;

    *temperature = (double)temp / 100;

    *pressure = (double)press / 100;
}