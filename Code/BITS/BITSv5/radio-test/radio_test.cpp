#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../../libraries/rp2040-drf1262-lib/SX1262.h"
#include "../BITSv5.h"
#include "../BITSv5_Radio.h"
#include "hardware/gpio.h"
#include "pico/binary_info.h"
#include "pico/rand.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include "pico/unique_id.h"

DRF1262 radio(spi1, RADIO_CS, SCK_PIN, MOSI_PIN, MISO_PIN, TXEN_PIN, DIO1_PIN,
              BUSY_PIN, SW_PIN, RADIO_RST);

void rx_test(void);
void transmit_test(uint8_t *buf, size_t len);

int main() {
    setup();
    led_off();

    // while (!stdio_usb_connected()) {
    //     sleep_ms(10);
    // }

    sleep_ms(5000);

    radio.debug_msg_en = 0;
    radio.radio_init();

    radio.radio_receive_cont();

    while (true) {
        if (transmit) {
            transmit_test((uint8_t *)radio_tx_buf, sizeof(radio_tx_buf));
            transmit = false;
        }

        if (rx_done) {
            printf("Received\n");
            printf("%s\n", radio_rx_buf);
            printf("RSSI: %d dBm Signal RSSI: %d SNR: %d dB\n",
                   radio.pkt_stat.rssi_pkt, radio.pkt_stat.signal_rssi_pkt,
                   radio.pkt_stat.snr_pkt);

            if (strncmp("ack", radio_rx_buf, 3) != 0) {
                ack_alarm_id = alarm_pool_add_alarm_in_ms(
                    ack_alarm_pool, 1000, ack_timer_callback, NULL, true);
            }

            rx_done = false;
        }

        if (send_ack) {
            printf("ACK\n");
            transmit_test((uint8_t *)ack, sizeof(ack));
            send_ack = false;
        }
    }
}

void transmit_test(uint8_t *buf, size_t len) {
    printf("Transmit\n");

    led_on();

    tx_done = false;

    // buf[0] = (char)get_rand_32();

    radio.radio_send(buf, len);

    printf("%s\n", (char *)buf);

    radio.get_radio_errors();
}

void rx_test() {
    char data[6] = {
        '\0', '\0', '\0', '\0', '\0', '\0',
    };

    char ack_msg[] = "_ack-__________";

    printf("Receive Test\n");

    radio.radio_receive_single();

    while (!gpio_get(DIO1_PIN) && !transmit) {
        sleep_ms(1);
    }

    radio.clear_irq_status();

    radio.read_radio_buffer((uint8_t *)data, 5);

    printf("Got some data: %s\n", data);

    strcpy(ack_msg + 4, data);

    transmit_test((uint8_t *)ack_msg, sizeof(ack_msg));
}

void gpio_callback(uint gpio, uint32_t events) {
    if (gpio == DIO1_PIN) {
        printf("DIO1 ISR\n");
        radio.get_irq_status();

        if (radio.irqs.tx_done) {
            printf("TX ISR\n");
            radio.disable_tx();
            radio.radio_receive_cont();
            tx_done = true;
            radio.irqs.tx_done = false;
            led_off();
        }

        if (radio.irqs.rx_done) {
            printf("RX ISR\n");
            radio.read_radio_buffer((uint8_t *)radio_rx_buf,
                                    sizeof(radio_rx_buf));

            radio.get_packet_status();
            radio.irqs.rx_done = false;
            rx_done = true;
        }

        radio.clear_irq_status();
    }
}
