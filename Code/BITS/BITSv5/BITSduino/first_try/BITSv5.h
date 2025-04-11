#ifndef _BITSv5_H
#define _BITSv5_H

#include "BITSv5_GPS.h"
#include "BITSv5_Iridium.h"
#include "BITSv5_Log.h"
#include "BITSv5_Xbee.h"

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

#define IR_TX_PIN 12
#define IR_RX_PIN 1
#define IR_CTS_PIN 2
#define IR_RTS_PIN 3
#define IR_NETAV_PIN 4
#define IR_RING_PIN 5

#define OutputSerial Serial // USB debug

#define bat_cell1 22

typedef struct {
  uint32_t count;
  uint32_t pressure;
  struct {
    uint32_t integer;
    uint32_t decimal;
  } temperature;
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
  struct {
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
  } time;
  struct {
    uint8_t day;
    uint8_t month;
    uint8_t year;
  } date;
} log_item;

void setup_handler(void);
void loop_handler(void);

#endif
