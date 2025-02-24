#include "BITSv5_Radio.h"

#include <stdio.h>

#include "BITSv5.h"
#include "hardware/spi.h"
#include "pico/rand.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include "pico/unique_id.h"

volatile bool tx_done = false;
volatile bool transmit = false;
volatile bool rx_done = false;
volatile bool send_ack = false;

repeating_timer_t tx_timer;
alarm_pool_t *ack_alarm_pool;
alarm_id_t ack_alarm_id;

char radio_rx_buf[100] = {0};
uint8_t radio_tx_buf[100] = "_hello!";
uint8_t radio_ack_buf[100] = {0};
char ack[100] = "ack - bitsv5 alive!";

void radio_setup() {
  gpio_init(RADIO_RST);
  gpio_set_dir(RADIO_RST, GPIO_OUT);
  gpio_put(RADIO_RST, 1);
  gpio_set_irq_enabled_with_callback(DIO1_PIN, GPIO_IRQ_EDGE_RISE, true,
                                     &gpio_callback);
  // ack_alarm_pool = alarm_pool_create_with_unused_hardware_alarm(4);

  // negative timeout means exact delay (rather than delay between
  // callbacks)
  start_tx_repeating(30000);
}

bool tx_timer_callback(repeating_timer_t *rt) {
  transmit = true;
  return true; // keep repeating
}

int64_t ack_timer_callback(alarm_id_t id, void *user_data) {
  send_ack = true;
  return 0; // don't repeat
}

int start_tx_repeating(int interval_ms) {
  // negative timeout means exact delay (rather than delay between
  // callbacks)
  if (!add_repeating_timer_ms(-interval_ms, tx_timer_callback, NULL,
                              &tx_timer)) {
    printf("Failed to add timer\n");
    return 1;
  }
  return 0;
}

// Weakly bound GPIO callback for servicing the radio, user should define their
// own if the radio needs to be used
void __attribute__((weak)) gpio_callback(uint gpio, uint32_t events) { return; }
