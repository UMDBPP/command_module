#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../../libraries/rp2040-config/MB85RS1MT.h"
#include "../../../libraries/rp2040-config/config.h"
#include "../../../libraries/rp2040-drf1262-lib/SX1262.h"
#include "../../../libraries/rp2040-ms5607-lib/MS5607.h"
#include "../BITSv5.h"
#include "../BITSv5_GPS.h"
#include "../BITSv5_Radio.h"
#include "hardware/flash.h"
#include "hardware/gpio.h"
#include "hardware/watchdog.h"
#include "pico/binary_info.h"
#include "pico/rand.h"
#include "pico/stdlib.h"
#include "pico/unique_id.h"

// logging
MB85RS1MT mem(spi1, FRAM_CS, SCK_PIN, MOSI_PIN, MISO_PIN);

static uint32_t log_addr = LOG_INIT_ADDR;
char log_str[] = "BITSv5,PRESS,TEMP,LAT,LONG,TIME\n";  // "a b c d ef"
char log_str1[] = "0,0,0,99.9999,99.9999,0000.00\n";   //"h i j k lm"
uint8_t log_buf = 0;

Config test_config;

// GPS
char gps_buf[90] = {0};
uint gps_buf_offset = 0;
void get_gps_data(void);

// radio
DRF1262 radio(spi1, RADIO_CS, SCK_PIN, MOSI_PIN, MISO_PIN, TXEN_PIN, DIO1_PIN,
              BUSY_PIN, SW_PIN, RADIO_RST);

void rx_test(void);
void transmit_test(uint8_t *buf, size_t len);

// misc
void gpio_callback(uint gpio, uint32_t events);
void setup_spi();
void write_name_config();

int main() {
    setup();

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

    printf("BITSv5 Test (Compiled %s %s)\n", __DATE__, __TIME__);
    printf("Device ID: %d\n", mem.device_id);
    read_config(NAME, test_config, (uint8_t *)test_config.name,
                sizeof(test_config.name), &mem);
    printf("DEVICE NAME: %s\n", test_config.name);

    printf("Dumping FRAM\n");
    for (log_addr = LOG_INIT_ADDR;
         log_addr <= LOG_INIT_ADDR + 1 + 20 * (sizeof(log_str1));
         log_addr++) {  //= log_addr + sizeof(log_buf)
        watchdog_update();
        mem.read_memory(log_addr, &log_buf, 1);  // sizeof(log_buf)
        if (log_buf != 0) printf("%c", log_buf);
        // printf("%d - %c\n", log_addr, log_buf);
        // memset(log_buf, 0, sizeof(log_buf));
        log_buf = 0;
    }

    log_addr = LOG_INIT_ADDR;
    printf("\n\nWriting FRAM\n");
    mem.write_memory(log_addr, (uint8_t *)log_str, sizeof(log_str));
    printf("%d - %s\n", log_addr, log_str);
    for (log_addr = LOG_INIT_ADDR + sizeof(log_str);
         log_addr <= LOG_INIT_ADDR + 1 + 20 * (sizeof(log_str1));
         log_addr = log_addr + sizeof(log_str1)) {
        // printf("%d\n", log_addr);
        watchdog_update();

        // log_str[1] = (char)get_rand_32();
        log_str1[0] = (char)get_rand_32();

        mem.write_memory(log_addr, (uint8_t *)log_str1, sizeof(log_str1));
        printf("%d - %s\n", log_addr, log_str1);

        // if (log_addr % 2 != 0) {
        //     mem.write_memory(log_addr, (uint8_t *)log_str, sizeof(log_str));
        //     printf("%d - %s\n", log_addr, log_str);
        // } else {
        //     mem.write_memory(log_addr, (uint8_t *)log_str1,
        //     sizeof(log_str1)); printf("%d - %s\n", log_addr, log_str1);
        // }
    }

    read_config(NAME, test_config, (uint8_t *)test_config.name,
                sizeof(test_config.name), &mem);
    printf("DEVICE NAME: %s\n", test_config.name);

    while (true) {
        // printf("Updating watchdog \n");
        // watchdog_update();

        // printf("DEVICE NAME: %s\n", test_config.name);

        // // Read and print the NAME config setting
        // read_config(NAME, test_config, (uint8_t *)test_config.name,
        //             sizeof(test_config.name), &mem);

        // printf("DEVICE NAME: %s\n", test_config.name);

        // // Write the time that this file was last compiled to the config
        // strcpy(test_config.name, __TIME__);

        // // Write the changed setting to the FRAM
        // write_config(NAME, test_config, (uint8_t *)test_config.name,
        //              sizeof(test_config.name), &mem);

        // // Read and print again
        // strcpy(test_config.name, "something broke");

        // read_config(NAME, test_config, (uint8_t *)test_config.name,
        //             sizeof(test_config.name), &mem);

        // printf("DEVICE NAME: %s\n", test_config.name);

        // transmit_test((uint8_t *)radio_tx_buf, sizeof(radio_tx_buf));

        sleep_ms(1000);
    }
}

void transmit_test(uint8_t *buf, size_t len) {
    printf("Transmit Test\n");

    led_on();

    tx_done = false;

    buf[0] = (char)get_rand_32();

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

void get_gps_data(void) {
    while (uart_is_readable(uart1) > 0) {
        char c = uart_getc(uart1);
        if (c == '\r') {
            strcpy((char *)radio_tx_buf, gps_buf);
            gps_buf_offset = 0;
        }

        if (gps_buf_offset >= 89) gps_buf_offset = 0;

        if (c == '$') gps_buf_offset = 0;

        if (c != '\r') gps_buf[gps_buf_offset] = c;
        gps_buf_offset++;
        // printf("%c", c);
    }
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