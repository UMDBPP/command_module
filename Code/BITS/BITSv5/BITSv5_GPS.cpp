#include "BITSv5_GPS.h"

#include <stdio.h>

#include "BITSv5.h"
#include "hardware/i2c.h"
#include "pico/stdlib.h"
// #include <time.h>

#include "../../libraries/libnmea/src/nmea/nmea.h"

// Registers
const uint8_t REG_NUM_BYTES_MSB = 0xFD;
const uint8_t REG_NUM_BYTES_LSB = 0xFE;
const uint8_t REG_DATA = 0xFF;
i2c_inst_t *i2c = i2c0;
uint8_t gps_buf[100] = {0};
uint8_t rx_msg = 0;

int result = 0;

unsigned char pos = 0;  // Keep track of current position in gps_buf
char *values[10];       // Array of char pointers, for pointing to positons in
                        // gps_buf

nmea_gga data;
tm date;
nmea_position lon;
nmea_position lat;

void get_gps_data(void) {
    result = i2c_read_blocking(i2c, GPS_ADDR, &rx_msg, 1, false);
    if (result == PICO_ERROR_GENERIC)
        printf("\ni2c error occurred %x\n\n", result);
    else {
        if (rx_msg != NO_GPS_DATA) {
            // printf("%c", rx_msg);
            // End sequence is "\r\n"
            gps_buf[pos++] = rx_msg;
            if (rx_msg == '\n') {
                // We've reached the end of the sentence/line.
                gps_buf[pos++] = '\0';  // NULL-terminate for safety
                // Parse NMEA sentence, only if GNGGA for now
                if (strlen((char *)gps_buf) >= 6 &&
                    strncmp((char *)(&gps_buf[3]), "GGA", 3) == 0) {
                    printf("%s\n", gps_buf);
                    // Split msg into values, parse each value, then assign
                    // each result to appropriate place in struct
                    _split_string_by_comma((char *)gps_buf, values, 7);
                    // nmea_gga data;
                    // tm date;
                    // nmea_position lon;
                    // nmea_position lat;
                    // nmea_date_parse(values[1], &date);
                    nmea_time_parse(values[1], &date);
                    data.time = date;
                    nmea_position_parse(values[4], &lon);
                    lon.cardinal = nmea_cardinal_direction_parse(values[5]);
                    data.longitude = lon;
                    nmea_position_parse(values[2], &lat);
                    lon.cardinal = nmea_cardinal_direction_parse(values[3]);
                    data.latitude = lat;
                    data.position_fix = atoi(values[6]);

                    printf("Hour: %d Min: %d Sec: %d\n", date.tm_hour,
                           date.tm_min, date.tm_sec);
                }
                pos = 0;  // Reset pos for reading in the next sentence
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

/**
 * Parses a position (latitude or longitude). TODO: Need to modify if want
 * minutes as int.
 *
 * s is the string containing the data in string form.
 * pos is a pointer to an nmea_position struct which will be filled with the
 *      data.
 *
 * Copied from libnmea/src/parsers/parse.c.
 */
int nmea_position_parse(char *s, nmea_position *pos) {
    char *cursor;

    pos->degrees = 0;
    pos->minutes = 0;

    if (s == NULL || *s == '\0') {
        return -1;
    }

    /* decimal minutes */
    if (NULL == (cursor = strchr(s, '.'))) {
        return -1;
    }

    /* minutes starts 2 digits before dot */
    cursor -= 2;
    pos->minutes = atof(cursor);
    *cursor = '\0';

    /* integer degrees */
    cursor = s;
    pos->degrees = atoi(cursor);

    return 0;
}

/**
 * Parses a cardinal direction.
 *
 * s is a pointer to a char, which contains the data to be parsed.
 *
 * Returns an nmea_cardinal_t (basically an enum).
 *
 * Copied from libnmea/src/parsers/parse.c.
 */
nmea_cardinal_t nmea_cardinal_direction_parse(char *s) {
    if (NULL == s || '\0' == *s) {
        return NMEA_CARDINAL_DIR_UNKNOWN;
    }

    switch (*s) {
        case NMEA_CARDINAL_DIR_NORTH:
            return NMEA_CARDINAL_DIR_NORTH;
        case NMEA_CARDINAL_DIR_EAST:
            return NMEA_CARDINAL_DIR_EAST;
        case NMEA_CARDINAL_DIR_SOUTH:
            return NMEA_CARDINAL_DIR_SOUTH;
        case NMEA_CARDINAL_DIR_WEST:
            return NMEA_CARDINAL_DIR_WEST;
        default:
            break;
    }

    return NMEA_CARDINAL_DIR_UNKNOWN;
}

/**
 * Parses a date.
 *
 * s is the string containng the date.
 * date is a pointer to a struct that gets filled with data.
 *
 * Returns 0 on success and -1 on failure.
 *
 * Copied from libnmea/src/parsers/parse.c.
 */
int nmea_date_parse(char *s, struct tm *date) {
    char *rv;
    uint32_t x;

    if (s == NULL || *s == '\0') {
        return -1;
    }

    x = strtoul(s, &rv, 10);
    date->tm_mday = x / 10000;
    date->tm_mon = ((x % 10000) / 100) - 1;
    date->tm_year = x % 100;

    // Normalize tm_year according to C standard library
    if (date->tm_year > 1900) {  // ZDA message case
        date->tm_year -= TM_YEAR_START;
    } else {  // RMC message case
        date->tm_year += (RMC_YEAR_START - TM_YEAR_START);
    }

    return 0;
}

int nmea_time_parse(char *s, struct tm *time) {
    char *rv;
    uint32_t x;

    if (s == NULL || *s == '\0') {
        return -1;
    }

    x = strtoul(s, &rv, 10);
    time->tm_hour = x / 10000;
    time->tm_min = (x % 10000) / 100;
    time->tm_sec = x % 100;
    if (time->tm_hour > 23 || time->tm_min > 59 || time->tm_sec > 59 ||
        (int)(rv - s) < NMEA_TIME_FORMAT_LEN) {
        return -1;
    }
    if (*rv == '.') {
        /* TODO There is a sub-second field. */
    }

    return 0;
}