#ifndef _MS5607_H
#define _MS5607_H

#include "pico/stdlib.h"

#define MS5607_ADDR 0x77
#define i2c_p i2c0
#define MS5607_RESET_CMD 0x1E
#define MS5607_READ_PROM_CMD 0xA6
#define MS5607_PRESS_CONV_CMD 0x48
#define MS5607_TEMP_CONV_CMD 0x58
#define MS5607_READ_CMD 0x00

void ms5607_init(void);
uint8_t* ms5607_read_prom(uint8_t*);
uint8_t* ms5607_conversion(uint8_t*);

#endif