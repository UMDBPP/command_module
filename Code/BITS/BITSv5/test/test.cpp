#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../../libraries/rp2040-config/MB85RS1MT.h"
#include "../../../libraries/rp2040-config/config.h"
#include "../../../libraries/rp2040-drf1262-lib/SX1262.h"
#include "../../../libraries/rp2040-ms5607-lib/MS5607.h"
#include "../BITSv5.h"
#include "hardware/flash.h"
#include "hardware/gpio.h"
#include "pico/binary_info.h"
#include "pico/rand.h"
#include "pico/stdlib.h"
#include "pico/unique_id.h"

// Define the correct flash size for BITSv5 (16MB)
#undef PICO_FLASH_SIZE_BYTES
#define PICO_FLASH_SIZE_BYTES (16 * 1024 * 1024)

DRF1262 radio(spi1, RADIO_CS, SCK_PIN, MOSI_PIN, MISO_PIN, TXEN_PIN, DIO1_PIN,
              BUSY_PIN, SW_PIN);

MB85RS1MT mem(spi1, FRAM_CS, SCK_PIN, MOSI_PIN, MISO_PIN);

static uint32_t log_addr = LOG_INIT_ADDR;
char log_str[] = "log item ";
uint32_t log_cnt = 0;

Config test_config;

char id[2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1] = {0};

char radio_rx_buf[100] = {0};
uint8_t radio_tx_buf[100] = "_hello!";
uint8_t radio_ack_buf[100] = {0};
char ack[] = "ack - xbee joint alive!";

char gps_buf[90] = {0};
uint gps_buf_offset = 0;

volatile bool tx_done = false;
volatile bool transmit = false;
volatile bool rx_done = false;
volatile bool send_ack = false;

repeating_timer_t tx_timer;

void rx_test(void);
void transmit_test(uint8_t *buf, size_t len);
void setup_led();
void led_on();
void led_off();
void gpio_callback(uint gpio, uint32_t events);
bool tx_timer_callback(repeating_timer_t *rt);
void get_gps_data(void);
int start_tx_repeating(void);

int main() {
    stdio_init_all();

    gpio_set_irq_enabled_with_callback(DIO1_PIN, GPIO_IRQ_EDGE_RISE, true,
                                       &gpio_callback);

    setup_led();
    led_off();

    spi_init(spi1, 500000);

    spi_set_format(spi1,           // SPI instance
                   8,              // Number of bits per transfer
                   (spi_cpol_t)0,  // Polarity (CPOL)
                   (spi_cpha_t)0,  // Phase (CPHA)
                   SPI_MSB_FIRST);

    gpio_init(RADIO_RST);
    gpio_set_dir(RADIO_RST, GPIO_OUT);
    gpio_put(RADIO_RST, 1);

    pico_get_unique_board_id_string(id,
                                    2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1);

    sleep_ms(5000);

    radio.debug_msg_en = 0;
    radio.radio_init();

    mem.mem_init();

    while (true) {
        printf("\n%s %s\n", __DATE__, __TIME__);

        printf("Config Lib Test Compiled %s %s\n", __DATE__, __TIME__);

        printf("Device ID: %d\n", mem.device_id);

        printf("DEVICE NAME: %s\n", test_config.name);

        // Read and print the NAME config setting
        read_config(NAME, test_config, (uint8_t *)test_config.name,
                    sizeof(test_config.name), &mem);

        printf("DEVICE NAME: %s\n", test_config.name);

        // Write the time that this file was last compiled to the config
        strcpy(test_config.name, __TIME__);

        // Write the changed setting to the FRAM
        write_config(NAME, test_config, (uint8_t *)test_config.name,
                     sizeof(test_config.name), &mem);

        // Read and print again
        strcpy(test_config.name, "something broke");

        read_config(NAME, test_config, (uint8_t *)test_config.name,
                    sizeof(test_config.name), &mem);

        printf("DEVICE NAME: %s\n", test_config.name);

        mem.write_memory(log_addr, (uint8_t *)log_str, strlen(log_str));
        log_addr = log_addr + strlen(log_str);
        mem.write_memory(log_addr, (uint8_t *)(&log_cnt), sizeof(log_cnt));
        log_addr = log_addr + sizeof(log_str);
        log_cnt++;

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

    while (!tx_done) busy_wait_us_32(1);

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

void setup_led() {
    gpio_init(LED_PIN);
    gpio_set_dir(0, GPIO_OUT);
    gpio_put(LED_PIN, 0);
}

void led_on() { gpio_put(LED_PIN, true); }

void led_off() { gpio_put(LED_PIN, false); }

bool tx_timer_callback(repeating_timer_t *rt) {
    transmit = true;

    return true;  // keep repeating
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

int start_tx_repeating() {
    // negative timeout means exact delay (rather than delay between
    // callbacks)
    if (!add_repeating_timer_us(-20000000, tx_timer_callback, NULL,
                                &tx_timer)) {
        printf("Failed to add timer\n");
        return 1;
    }
    return 0;
}

void gpio_callback(uint gpio, uint32_t events) {
    if (gpio == DIO1_PIN) {
        printf("DIO1 ISR\n");
        radio.get_irq_status();

        if (radio.irqs.TX_DONE) {
            printf("TX ISR\n");
            radio.disable_tx();
            radio.radio_receive_cont();
            tx_done = true;
            radio.irqs.TX_DONE = false;
        }

        if (radio.irqs.RX_DONE) {
            printf("RX ISR\n");
            radio.read_radio_buffer((uint8_t *)radio_rx_buf, sizeof(radio_rx_buf));
            printf("%s\n", radio_rx_buf);
            radio.get_packet_status();
            printf("RSSI: %d dBm Signal RSSI: %d SNR: %d dB\n",
                   radio.pkt_stat.rssi_pkt, radio.pkt_stat.signal_rssi_pkt,
                   radio.pkt_stat.snr_pkt);

            radio.irqs.RX_DONE = false;
            send_ack = true;
        }

        radio.clear_irq_status();
    }
}