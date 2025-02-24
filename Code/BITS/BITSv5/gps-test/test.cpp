#include <stdio.h>
#include <string.h>
#include <time.h>

#include "../../../libraries/libnmea/src/nmea/nmea.h"
#include "../../../libraries/rp2040-drf1262-lib/SX1262.h"
#include "../BITS_common/BITSv5.h"
#include "../BITS_common/BITSv5_GPS.h"
#include "../BITS_common/BITSv5_Radio.h"
#include "hardware/i2c.h"
#include "pico/binary_info.h"
#include "pico/stdlib.h"

static int _split_string_by_comma(char *string, char **values, int max_values);
int nmea_position_parse(char *s, nmea_position *pos);
nmea_cardinal_t nmea_cardinal_direction_parse(char *s);
int nmea_date_parse(char *s, struct tm *date);
int nmea_time_parse(char *s, struct tm *time);

int main() {

  setup(true);

  sleep_ms(5000);

  // Initialize I2C pins
  gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
  gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);

  uint8_t rx_msg = 0;

  while (true) {
    get_gps_data();
  }
}
