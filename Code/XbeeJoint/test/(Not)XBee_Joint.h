#ifndef _NOTXBEEJOINT_H
#define _NOTXBEEJOINT_H

#define CS_PIN 21
#define SCK_PIN 18
#define MOSI_PIN 19
#define MISO_PIN 20
#define TXEN_PIN 1
#define DIO1_PIN 3
#define BUSY_PIN 6
#define SW_PIN 9

#define INCLUDE_DEBUG \
    1             // controls if debug conditionals are included at compile time
short debug = 1;  // controls if debug messages are printed

#endif