#ifndef _BITSv5_H
#define _BITSv5_H

#include "pico/time.h"
#include "pico/unique_id.h"

// Define the correct flash size for BITSv5 (16MB)
#undef PICO_FLASH_SIZE_BYTES
#define PICO_FLASH_SIZE_BYTES (16 * 1024 * 1024)

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

#define INCLUDE_DEBUG                                                          \
  1 // controls if debug conditionals are included at compile time

extern short debug_msgs; // controls if debug messages are printed

// extern void log_to_string(log_config *log, char *dst);

extern char id[2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1];

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

typedef struct {
  uint start_addr;
  uint end_addr;
} log_file;

extern log_item periodic_log_item;

/**
 * @brief Basic setup functions for the RP2040 in general as well as the BITS
 * platform. This function sets up the on-board LED, enables I2C and SPI, gets
 * unique board ID, and runs the Radio and GPS setup functions.
 *
 * @param periodic_log enables or disables periodic logging
 *
 * @return int - maybe at somepoint this can be a status code although frankly
 * the program should probably crash if this function encounters an issue
 */
int setup(bool periodic_log);

void led_on();
void led_off();
void gpio_callback(uint gpio, uint32_t events);

// logging
bool log_timer_callback(repeating_timer_t *rt);
void dump_fram(log_file *file);
void write_fram(log_file *file, uint8_t *buf, uint len);

#endif
