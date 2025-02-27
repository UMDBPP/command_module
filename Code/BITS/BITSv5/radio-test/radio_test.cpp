#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../../libraries/rp2040-drf1262-lib/SX1262.h"
#include "../BITS_common/BITSv5.h"
#include "../BITS_common/BITSv5_GPS.h"
#include "../BITS_common/BITSv5_Radio.h"
#include "hardware/gpio.h"
#include "pico/binary_info.h"
#include "pico/rand.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include "pico/unique_id.h"

DRF1262 radio(spi1, RADIO_CS, SCK_PIN, MOSI_PIN, MISO_PIN, TXEN_PIN, DIO1_PIN,
              BUSY_PIN, SW_PIN, RADIO_RST);

void rx_test(void);
void transmit_buffer(uint8_t *buf, size_t len);
void radio_task(void);

int main() {
  setup(true);
  led_off();

  sleep_ms(5000);

  radio.debug_msg_en = 0; // disable debug messages
  radio.radio_init();
  radio.radio_receive_cont(); // enter continuous receive mode

  while (true) {
    radio_task();
  }
}

// This task checks 3 flags to determine what, if anything, needs to be done. It
// is designed to be run on some regular interval, or as frequently as possible
// in order to ensure reliable message servicing.
void radio_task() {
  if (transmit) {

    radio.set_cad();

    uint32_t timeout = to_ms_since_boot(delayed_by_ms(get_absolute_time(), 10));

    // poll before timeout
    while (to_ms_since_boot(get_absolute_time()) < timeout) {
      if (cad_done == true) {
        if (channel_active != true) {
          transmit_buffer((uint8_t *)radio_tx_buf, sizeof(radio_tx_buf));
          transmit = false;
          cad_done = false;
        }
        break;
      }
    }
  }

  if (rx_done) {
    printf("Received\n");
    printf("%s\n", radio_rx_buf);
    printf("RSSI: %d dBm Signal RSSI: %d SNR: %d dB\n", radio.pkt_stat.rssi_pkt,
           radio.pkt_stat.signal_rssi_pkt, radio.pkt_stat.snr_pkt);

    // if an acknowledgement was received
    if (strncmp("ack", radio_rx_buf, 3) != 0) {
      ack_alarm_id = alarm_pool_add_alarm_in_ms(ack_alarm_pool, 1000,
                                                ack_timer_callback, NULL, true);
    }

    rx_done = false;
  }

  if (send_ack) {
    printf("sending ACK\n");
    transmit_buffer((uint8_t *)ack, sizeof(ack));
    send_ack = false;
  }
}

void transmit_buffer(uint8_t *buf, size_t len) {
  printf("Transmit\n");

  led_on();

  tx_done = false;

  buf[0] =
      (char)((get_rand_32() % 93) + 33); // random character between 33 and 126

  radio.radio_send(buf, len);

  printf("sent: %s\n", (char *)buf);

  radio.get_radio_errors();
  printf("status: %x\n", radio.status);
  printf("status: %x\n", radio.err1);
  printf("status: %x\n", radio.err2);
}

// user defined, strongly bound, GPIO callback for servicing the radio
void gpio_callback(uint gpio, uint32_t events) {
  if (gpio == DIO1_PIN) {
    printf("DIO1 ISR\n");
    radio.get_irq_status();

    if (radio.irqs.tx_done) {
      printf("TX ISR\n");
      radio.disable_tx();
      radio.radio_receive_cont();
      radio.irqs.tx_done = false;
      tx_done = true;

      led_off();
    }

    if (radio.irqs.rx_done) {
      printf("RX ISR\n");
      radio.read_radio_buffer((uint8_t *)radio_rx_buf, sizeof(radio_rx_buf));

      radio.get_packet_status();
      radio.irqs.rx_done = false;
      rx_done = true;
    }

    if (radio.irqs.cad_done) {
      printf("CAD ISR\n");
      radio.irqs.cad_done = false;
      cad_done = true;
      channel_active = radio.irqs.cad_det;
    }

    radio.clear_irq_status();
  }
}
