#ifndef _BITSv5_GPS_H
#define _BITSv5_GPS_H

#include <time.h>

// #include "../../libraries/libnmea/src/nmea/nmea.h"
#include "hardware/i2c.h"

#define NO_GPS_DATA 0xFF

// Registers
extern const uint8_t REG_NUM_BYTES_MSB;
extern const uint8_t REG_NUM_BYTES_LSB;
extern const uint8_t REG_DATA;
extern i2c_inst_t *i2c;
extern uint8_t gps_buf[100];

// These structs have been largely lifted from libnmea.
/* NMEA cardinal direction types */
typedef char nmea_cardinal_t;
#define NMEA_CARDINAL_DIR_NORTH (nmea_cardinal_t)'N'
#define NMEA_CARDINAL_DIR_EAST (nmea_cardinal_t)'E'
#define NMEA_CARDINAL_DIR_SOUTH (nmea_cardinal_t)'S'
#define NMEA_CARDINAL_DIR_WEST (nmea_cardinal_t)'W'
#define NMEA_CARDINAL_DIR_UNKNOWN (nmea_cardinal_t)'\0'

#define NMEA_TIME_FORMAT "%H%M%S"
#define NMEA_TIME_FORMAT_LEN 6

#define TM_YEAR_START 1900
#define RMC_YEAR_START 2000

enum { INVALID = 0, GPS, DIFF, NA, RTK_FIXED, RTK_FLOAT, INS } fix_quality;

typedef struct {
    struct {
        uint8_t hours;
        uint8_t minutes;
        uint8_t seconds;
    } time;

    struct {
        uint32_t integer;
        uint32_t decimal;
        char cardinal;
    } latitude;

    struct {
        uint32_t integer;
        uint32_t decimal;
        char cardinal;
    } longitude;

    uint8_t fix_quality;

    uint8_t sv;

    struct {
        uint32_t integer;
        uint32_t decimal;
    } hdop;

    struct {
        uint32_t integer;
        uint32_t decimal;
    } altitude;
} nmea_gga_msg;

typedef struct {
    struct {
        uint8_t day;
        uint8_t month;
        uint8_t year;
    } date;
} nmea_rmc_msg;

extern nmea_gga_msg gga_data;
extern nmea_rmc_msg rmc_data;

static int _split_string_by_comma(char *string, char **values, int max_values);
// int nmea_position_parse(char *s, nmea_position *pos);
nmea_cardinal_t nmea_cardinal_direction_parse(char *s);
int nmea_date_parse(char *s, struct tm *date);
int nmea_time_parse(char *s, struct tm *time);
void get_gps_data(void);
void gps_setup(void);

#endif