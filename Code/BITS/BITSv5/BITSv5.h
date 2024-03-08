#ifndef _BITSv5_H
#define _BITSv5_H

#define SCK_PIN 26
#define MOSI_PIN 27
#define MISO_PIN 24

#define RADIO_CS 25
#define TXEN_PIN 8
#define DIO1_PIN 10
#define BUSY_PIN 11
#define SW_PIN 9
#define RADIO_RST 13

#define SDA_PIN 20
#define SCL_PIN 21

#define GPS_ADDR 0x42
#define EXTINT_PIN 18
#define TIMEPULSE_PIN 17

#define LED_PIN 0

#define FRAM_CS 29

#define LOG_INIT_ADDR 8192
#define LOG_MAX_ADDR 131072

#define INCLUDE_DEBUG \
    1  // controls if debug conditionals are included at compile time

extern short debug_msgs;  // controls if debug messages are printed

/* NMEA cardinal direction types */
typedef char nmea_cardinal_t;
#define NMEA_CARDINAL_DIR_NORTH (nmea_cardinal_t)'N'
#define NMEA_CARDINAL_DIR_EAST (nmea_cardinal_t)'E'
#define NMEA_CARDINAL_DIR_SOUTH (nmea_cardinal_t)'S'
#define NMEA_CARDINAL_DIR_WEST (nmea_cardinal_t)'W'
#define NMEA_CARDINAL_DIR_UNKNOWN (nmea_cardinal_t)'\0'

/* GPS position struct */
typedef struct {
    double minutes;
    int degrees;
    nmea_cardinal_t cardinal;
} nmea_position;

typedef struct _log_config {
    int pressure;
    int temperature;
    nmea_position pos;
    uint log_addr;
} log_config;

extern void log_to_string(log_config *log, char *dst);

#endif