#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../../libraries/rp2040-config/MB85RS1MT.h"
#include "../../../libraries/rp2040-config/config.h"
#include "../../../libraries/rp2040-drf1262-lib/SX1262.h"
#include "../../../libraries/rp2040-ms5607-lib/MS5607.h"
#include "../BITS_common/BITSv5.h"
#include "../BITS_common/BITSv5_GPS.h"
#include "../BITS_common/BITSv5_Radio.h"
#include "hardware/flash.h"
#include "hardware/gpio.h"
#include "hardware/watchdog.h"
#include "pico/binary_info.h"
#include "pico/rand.h"
#include "pico/stdlib.h"
#include "pico/unique_id.h"

// logging
MB85RS1MT mem(spi1, FRAM_CS, SCK_PIN, MOSI_PIN, MISO_PIN);

log_item periodic_log_item;

Config test_config;

// Radio
DRF1262 radio(spi1, RADIO_CS, SCK_PIN, MOSI_PIN, MISO_PIN, TXEN_PIN, DIO1_PIN,
              BUSY_PIN, SW_PIN, RADIO_RST);

void rx_radio(void);
void tx_radio(uint8_t *buf, size_t len);

// misc
void gpio_callback(uint gpio, uint32_t events);
void write_name_config();

int main() {
    setup(&mem);

    radio.debug_msg_en = 0;

    sleep_ms(5000);

    // setup devices
    radio.radio_init();
    mem.mem_init();

    // Enable the watchdog, requiring the watchdog to be updated every 2000ms or
    // the chip will reboot
    watchdog_enable(2000, 1);

    if (watchdog_caused_reboot()) {
        printf("Rebooted by Watchdog!\n");
    } else {
        printf("Clean boot\n");
    }

    // read config info
    printf("BITSv5 Test (Compiled %s %s)\n", __DATE__, __TIME__);
    printf("Device ID: %d\n", mem.device_id);
    read_config(NAME, test_config, (uint8_t *)test_config.name,
                sizeof(test_config.name), &mem);
    printf("DEVICE NAME: %s\n", test_config.name);

    // Boot messages
    tx_radio((uint8_t *)"BITSv5.2 Booted", 15);
    // need some iridium transmit here

    while (true) {
        sleep_ms(1000);
    }
}

void tx_radio(const uint8_t *buf, size_t len) {
    printf("Transmit Test\n");

    led_on();

    tx_done = false;

    radio.radio_send(buf, len);

    // while (!tx_done) busy_wait_us_32(1);

    // printf("Starting wait\n");

    // sleep_ms(10000);

    led_off();
    printf("%s\n", (char *)buf);
    // radio.disable_tx();
    // radio.radio_receive_single();

    // radio.get_radio_errors();

    // radio.clear_irq_status();

    // radio.radio_receive_single();

    // radio.get_radio_errors();
    // radio.clear_radio_errors();
}

void rx_radio() {
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

    tx_radio((uint8_t *)ack_msg, sizeof(ack_msg));
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
        }

        if (radio.irqs.rx_done) {
            printf("RX ISR\n");
            radio.read_radio_buffer((uint8_t *)radio_rx_buf,
                                    sizeof(radio_rx_buf));
            printf("%s\n", radio_rx_buf);
            radio.get_packet_status();
            printf("RSSI: %d dBm Signal RSSI: %d SNR: %d dB\n",
                   radio.pkt_stat.rssi_pkt, radio.pkt_stat.signal_rssi_pkt,
                   radio.pkt_stat.snr_pkt);

            radio.irqs.rx_done = false;
            send_ack = true;
        }

        radio.clear_irq_status();
    }
}

void write_name_config() {
    strcpy(test_config.name, "BITSv5.2-0");
    write_config(NAME, test_config, (uint8_t *)test_config.name,
                 sizeof(test_config.name), &mem);
}
