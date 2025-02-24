#include "BITSv5_GPS.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "BITSv5.h"
#include "hardware/i2c.h"
#include "pico/stdlib.h"
// #include <time.h>

// #include "../../libraries/libnmea/src/nmea/nmea.h"

// Registers
const uint8_t REG_NUM_BYTES_MSB = 0xFD;
const uint8_t REG_NUM_BYTES_LSB = 0xFE;
const uint8_t REG_DATA = 0xFF;
i2c_inst_t *i2c = i2c0;
uint8_t gps_buf[100] = {0};
uint8_t rx_msg = 0;

int result = 0;

unsigned char pos = 0; // Keep track of current position in gps_buf
char *values[16];      // Array of char pointers, for pointing to positons in
                       // gps_buf

nmea_gga_msg gga_data;
nmea_rmc_msg rmc_data;

void gps_setup() {
  gpio_init(EXTINT_PIN);
  gpio_set_dir(EXTINT_PIN, GPIO_IN);
  gpio_init(TIMEPULSE_PIN);
  gpio_set_dir(TIMEPULSE_PIN, GPIO_IN);
}

static void gga_parse() {
  printf("%s\n", gps_buf);

  _split_string_by_comma((char *)gps_buf, values, 7);

  uint32_t temp_time = 0;
  sscanf(values[1], "%d.", &temp_time);
  gga_data.time.hours = temp_time / 10000;
  gga_data.time.minutes = (temp_time % 10000) / 100;
  gga_data.time.seconds = temp_time % 100;

  sscanf(values[4], "%d.%d", gga_data.longitude.integer,
         gga_data.longitude.decimal);
  gga_data.longitude.cardinal = *values[5];

  sscanf(values[2], "%d.%d", gga_data.latitude.integer,
         gga_data.latitude.decimal);
  gga_data.latitude.cardinal = *values[3];

  gga_data.fix_quality = atoi(values[6]);

  sscanf(values[9], "%d.%d", gga_data.altitude.integer,
         gga_data.altitude.decimal);

  printf("Hour: %d Min: %d Sec: %d\n", gga_data.time.hours,
         gga_data.time.minutes, gga_data.time.seconds);
  printf("lat: %d.%d lon: %d.%d\n", gga_data.latitude.integer,
         gga_data.latitude.decimal, gga_data.longitude.integer,
         gga_data.longitude.decimal);
  printf("altitude: %d.%d\n", gga_data.altitude.integer,
         gga_data.altitude.decimal);
  printf("fix quality: %d\n", gga_data.fix_quality);
}

static void rmc_parse() {
  printf("%s\n", gps_buf);

  _split_string_by_comma((char *)gps_buf, values, 12);

  uint32_t temp_date = 0;
  sscanf(values[9], "%d", &temp_date);
  rmc_data.date.day = temp_date / 10000;
  rmc_data.date.month = (temp_date % 10000) / 100;
  rmc_data.date.year = (temp_date % 100) + 2000;

  printf("Day: %d Month: %d Year: %d\n", rmc_data.date.day, rmc_data.date.month,
         rmc_data.date.year);
}

void get_gps_data(void) {
  result = i2c_read_blocking(i2c, GPS_ADDR, &rx_msg, 1, false);
  if (result == PICO_ERROR_GENERIC)
    printf("\ni2c error occurred %x\n\n", result);
  else {
    if (rx_msg != NO_GPS_DATA) {

#ifdef TEST
      printf("%c", rx_msg);
#endif

      // End sequence is "\r\n"
      gps_buf[pos++] = rx_msg;
      if (rx_msg == '\n') {
        // We've reached the end of the sentence/line.
        gps_buf[pos++] = '\0'; // NULL-terminate for safety
        // Parse NMEA sentence, only if GNGGA for now
        if (strlen((char *)gps_buf) >= 6 &&
            strncmp((char *)(&gps_buf[3]), "GGA", 3) == 0) {
          gga_parse();
        } else if (strlen((char *)gps_buf) >= 6 &&
                   strncmp((char *)(&gps_buf[3]), "RMC", 3) == 0) {
          rmc_parse();
        }
        pos = 0; // Reset pos for reading in the next sentence
      }
    }
  }
}

/**
 * Splits a string by comma.
 *
 * string is the string to split, will be manipulated. Needs to be
 *        null-terminated.
 * values is a char pointer array that will be filled with pointers to the
 *        splitted values in the string.
 * max_values is the maximum number of values to be parsed.
 *
 * Returns the number of values found in string.
 *
 * Copied from libnmea/src/nmea/nmea.c.
 */
static int _split_string_by_comma(char *string, char **values, int max_values) {
  int i = 0;

  values[i++] = string;
  while (i < max_values && NULL != (string = strchr(string, ','))) {
    *string = '\0';
    values[i++] = ++string;
  }

  return i;
}
