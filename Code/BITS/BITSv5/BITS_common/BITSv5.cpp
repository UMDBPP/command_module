#include "BITSv5.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../../libraries/rp2040-config/MB85RS1MT.h"
#include "BITSv5_GPS.h"
#include "BITSv5_Radio.h"
#include "hardware/spi.h"
#include "hardware/watchdog.h"
#include "pico/stdlib.h"
#include "pico/time.h"

char id[2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1];
repeating_timer_t log_timer;

static uint32_t log_addr = LOG_INIT_ADDR;
char log_header[] = "BITSv5,PRESS,TEMP,LAT,LONG,DATE,TIME\n";
char log_str1[] = "0000,0000.00,00.0,99.9999,99.9999,00/00/0000,00:00.00\n";
uint8_t log_buf = 0;
extern MB85RS1MT mem;
log_item periodic_log_item;

int setup(bool periodic_log) {
  stdio_init_all();

  // init LED pin
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);
  gpio_put(LED_PIN, 0);

  // Initialize I2C port at 100 kHz
  i2c_init(i2c1, 100 * 1000);
  gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
  gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);

  // init SPI
  spi_init(spi1, 500000);
  spi_set_format(spi1,          // SPI instance
                 8,             // Number of bits per transfer
                 (spi_cpol_t)0, // Polarity (CPOL)
                 (spi_cpha_t)0, // Phase (CPHA)
                 SPI_MSB_FIRST);

  pico_get_unique_board_id_string(id, 2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1);

  if (periodic_log != 0) {
    memset(&periodic_log_item, 0, sizeof(log_item));
    if (!add_repeating_timer_ms(-60000, log_timer_callback, NULL, &log_timer)) {
      printf("Failed to add timer\n");
    }
  }

  radio_setup();
  gps_setup();
  return 0;
}

bool log_timer_callback(repeating_timer_t *rt) {
  return true; // keep repeating
}

void led_on() { gpio_put(LED_PIN, true); }

void led_off() { gpio_put(LED_PIN, false); }

void dump_fram(log_file *file) {
  uint log_addr = 0;
  uint8_t log_buf = 0;

  printf("\n\nDumping FRAM\n");
  for (log_addr = file->start_addr; log_addr <= file->end_addr;
       log_addr++) { //= log_addr + sizeof(log_buf)
    watchdog_update();
    mem.read_memory(log_addr, &log_buf, 1); // sizeof(log_buf)
    if (log_buf != 0)
      printf("%c", log_buf);
    // printf("%d - %c\n", log_addr, log_buf);
    // memset(log_buf, 0, sizeof(log_buf));
    log_buf = 0;
  }
}

void write_fram(log_file *file, uint8_t *buf, uint len) {
  uint log_addr = file->end_addr;

  if (len > LOG_MAX_ADDR - LOG_INIT_ADDR) {
    printf("Buffer too long\n");
    return;
  }

  printf("\n\nWriting FRAM\n");

  mem.write_memory(log_addr, buf, len);

  file->end_addr = log_addr + len;

  //     printf("%d - %s\n", log_addr, log_str);
  //     for (log_addr = LOG_INIT_ADDR + sizeof(log_str);
  //          log_addr <= LOG_INIT_ADDR + 1 + 20 * (sizeof(log_str1));
  //          log_addr = log_addr + sizeof(log_str1)) {
  //         // printf("%d\n", log_addr);
  //         watchdog_update();

  //         // log_str[1] = (char)get_rand_32();
  //         log_str1[0] = (char)get_rand_32();

  //         mem.write_memory(log_addr, (uint8_t *)log_str1,
  //         sizeof(log_str1)); printf("%d - %s\n", log_addr, log_str1);

  //         // if (log_addr % 2 != 0) {
  //         //     mem.write_memory(log_addr, (uint8_t *)log_str,
  //         sizeof(log_str));
  //         //     printf("%d - %s\n", log_addr, log_str);
  //         // } else {
  //         //     mem.write_memory(log_addr, (uint8_t *)log_str1,
  //         //     sizeof(log_str1)); printf("%d - %s\n", log_addr,
  //         log_str1);
  //         // }
  //     }
}
