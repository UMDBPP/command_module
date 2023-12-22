#ifndef _MS5607_H
#define _MS5607_H

#include "hardware/i2c.h"
#include "pico/stdlib.h"

#define MS5607_ADDR 0x77
#define MS5607_RESET_CMD 0x1E

#define MS5607_READ_C1_CMD 0xA2
#define MS5607_READ_C2_CMD 0xA4
#define MS5607_READ_C3_CMD 0xA6
#define MS5607_READ_C4_CMD 0xA8
#define MS5607_READ_C5_CMD 0xAA
#define MS5607_READ_C6_CMD 0xAC
#define MS5607_READ_CRC_CMD 0xAE

#define MS5607_PRESS_CONV_CMD 0x48  // OSR = 4096
#define MS5607_TEMP_CONV_CMD 0x58   // OSR = 4096
#define MS5607_READ_CMD 0x00

class MS5607 {
   public:
    i2c_inst_t *i2c = i2c0;
    uint sda_pin;
    uint scl_pin;
    uint8_t debug_msg_en = 0;

    /**
     * Sets MS5607 config information
     */
    MS5607(i2c_inst_t *i2c_p, const uint sda, const uint scl) {
        i2c = i2c_p;
        sda_pin = sda;
        scl_pin = scl;
    }

    /**
     * Reads pressure & temperature data from the sensor, performs necessary
     * compensation to find actual pressure and temperature data
     *
     * @param pressure pointer to location of final pressure value in mbar
     * @param temperature pointer to location of final temperature value in
     * degrees C
     */
    void ms5607_get_press_temp(double *pressure, double *temperature);

    void init(void);

    /**
     * Resets the sensor and reads the PROM values
     *
     * @return 1 if an error occurs, 0 otherwise
     */
    int reset(void);

   private:
    uint16_t c1 = 0;
    uint16_t c2 = 0;
    uint16_t c3 = 0;
    uint16_t c4 = 0;
    uint16_t c5 = 0;
    uint16_t c6 = 0;
    uint32_t d1 = 0;
    uint32_t d2 = 0;
    int32_t dT = 0;
    int64_t off = 0;
    int64_t sens = 0;

    int read_prom();
    int conversion(void);
};

#endif