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

#define INCLUDE_DEBUG \
    1  // controls if debug conditionals are included at compile time

extern short debug_msgs;  // controls if debug messages are printed

#endif