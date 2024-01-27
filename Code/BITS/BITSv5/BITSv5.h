#ifndef _BITSv5_H
#define _BITSv5_H

#define CS_PIN 25
#define SCK_PIN 26
#define MOSI_PIN 27
#define MISO_PIN 24
#define TXEN_PIN 8
#define DIO1_PIN 10
#define BUSY_PIN 11
#define SW_PIN 9
#define SDA_PIN 20
#define SCL_PIN 21
#define GPS_ADDR 0x42

#define INCLUDE_DEBUG \
    1  // controls if debug conditionals are included at compile time

extern short debug_msgs;  // controls if debug messages are printed

#endif