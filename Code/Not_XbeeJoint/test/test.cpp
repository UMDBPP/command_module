#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "(Not)XBee_Joint.h"

extern "C" {
#include "../../libraries/rp2040-console/console.h"
#include "../../libraries/rp2040-console/std-cmd/command.h"
}

#include "../../libraries/rp2040-config/MB85RS1MT.h"
#include "../../libraries/rp2040-config/config.h"
#include "../../libraries/rp2040-drf1262-lib/SX1262.h"
#include "hardware/flash.h"
#include "hardware/gpio.h"
// #include "hardware/irq.h"
#include "hardware/spi.h"
#include "pico/binary_info.h"
#include "pico/rand.h"
#include "pico/stdlib.h"
#include "pico/unique_id.h"

// Define the correct flash size for BITSv5 (16MB)
#undef PICO_FLASH_SIZE_BYTES
#define PICO_FLASH_SIZE_BYTES (16 * 1024 * 1024)

// Flash-based address of the last sector
#define FLASH_TARGET_OFFSET (PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE)

#ifndef RX_TEST
#define RX_TEST 1
#endif

#ifndef TX_TEST
#define TX_TEST 1
#endif

void rx_test(char *buf, short len);
void transmit_test(uint8_t *buf, short len);

DRF1262 radio(spi0, CS_PIN, SCK_PIN, MOSI_PIN, MISO_PIN, TXEN_PIN, DIO1_PIN,
              BUSY_PIN, SW_PIN);

char id[2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1] = {0};
char alive[] = "xbee joint alive!";
char ack[] = "ack - xbee joint alive!";

short debug_msgs = 0;  // controls if debug messages are printed

enum Commands_Ext { HELP = STAT + 1 };  // how to add extra op codes for fun

command cmd = {0x00, NOP, {0, 0, 0, 0, 0, 0, 0}, NULL};

Config not_xbee_test_config;

char radio_buf[100] = {0};

bool tx_done = true;
bool send_ack = false;

void help_handler(uint8_t *args);
void gpio_callback(uint gpio, uint32_t events);

// For the functionality of a (Not)Xbee Joint board
// NOT PRODUCTION CODE
int main() {
    stdio_init_all();

    // gpio_set_irq_callback(&gpio_callback);

    gpio_set_irq_enabled_with_callback(DIO1_PIN, GPIO_IRQ_EDGE_RISE, true,
                                       &gpio_callback);

    sleep_ms(5000);

    radio.debug_msg_en = debug_msgs;
    radio.radio_init();

    pico_get_unique_board_id_string(id,
                                    2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1);

    help_handler(NULL);
    while (true) {
        printf("\n\n\n\n\n\n\n\n\n\n\n\n");

        if (debug_msgs) {
            printf("============Xbee Joint: %s (DEBUG)============\n", id);
        } else {
            printf("============Xbee Joint: %s============\n", id);
        }

        get_command(&cmd);
        print_command(&cmd);
        cmd.handler(cmd.params);

        // sleep_ms(500);
    }
}

void transmit_test(uint8_t *buf, short len) {
    printf("Transmit Test\n");

    tx_done = false;

    radio.radio_send(buf, len);

    while (!tx_done) busy_wait_us_32(1);

        // sleep_ms(100);

#if INCLUDE_DEBUG
    radio.get_radio_errors();
    radio.get_irq_status();
#endif

    // radio.clear_irq_status();

#if INCLUDE_DEBUG
    radio.get_irq_status();
#endif
}

void rx_test(char *buf, short len) {
    printf("Receive Test, press 'c' to cancel\n");

    radio.radio_receive_single();

    // while (!gpio_get(DIO1_PIN)) {
    //     sleep_ms(1);
    //     if (getchar_timeout_us(0) == 'c') {
    //         radio.clear_irq_status();
    //         return;
    //     }
    // }

    // sleep_ms(10);

    // radio.get_rx_buffer_status();

    // radio.get_irq_status();
    // radio.clear_irq_status();
    // radio.get_irq_status();

    // radio.read_radio_buffer((uint8_t *)buf, len);
}

void no_op_handler(uint8_t *args) { printf("handler not implemented\n"); }
void test_handler(uint8_t *args) { printf("handler not implemented\n"); }

void vent_handler(uint8_t *args) { printf("handler not implemented\n"); }
void reset_handler(uint8_t *args) { printf("handler not implemented\n"); }
void pos_handler(uint8_t *args) { printf("handler not implemented\n"); }
void term_handler(uint8_t *args) { printf("handler not implemented\n"); }
void ack_handler(uint8_t *args) { printf("handler not implemented\n"); }
void nack_handler(uint8_t *args) { printf("handler not implemented\n"); }
void err_handler(uint8_t *args) { printf("handler not implemented\n"); }
void stat_handler(uint8_t *args) { printf("handler not implemented\n"); }
void get_handler(uint8_t *args) { printf("handler not implemented\n"); }
void set_handler(uint8_t *args) { printf("handler not implemented\n"); }

void help_handler(uint8_t *args) {
    printf(
        "Enter commands at the promp below\nCommand format: Op-Code "
        "args\n");
}

void send_handler(uint8_t *args) {
    char buf[100] = {'\0'};

    printf("\nEnter string to send: ");

    get_string(buf);

    transmit_test((uint8_t *)(buf), sizeof(buf));
}

void lstn_handler(uint8_t *args) {
    // char buf[100] = {0};

    // rx_test(buf, sizeof(buf));

    // printf("rx: %s\n", buf);

    radio.radio_receive_cont();

    while (true) {  //! gpio_get(DIO1_PIN)
        char c = getchar_timeout_us(1000);

        switch (c) {
            case 'c':
                return;
                break;
            case 's':
                transmit_test((uint8_t *)(alive), sizeof(alive));
        }
        // if (getchar_timeout_us(1000) == 'c') return;
        // sleep_ms(1);

        if (send_ack) {
            transmit_test((uint8_t *)(ack), sizeof(ack));
            sleep_ms(100);
            send_ack = false;
        }
    }

    radio.read_radio_buffer((uint8_t *)radio_buf, sizeof(radio_buf));

    printf("Got some data: %s\n", radio_buf);
}

void gpio_callback(uint gpio, uint32_t events) {
    if (gpio == DIO1_PIN) {
        printf("DIO1 ISR\n");
        radio.get_irq_status();

        if (radio.irqs.TX_DONE) {
            printf("TX ISR\n");
            tx_done = true;
            radio.disable_tx();
            radio.radio_receive_cont();
            radio.irqs.TX_DONE = false;
        }

        if (radio.irqs.RX_DONE) {
            printf("RX ISR\n");
            radio.read_radio_buffer((uint8_t *)radio_buf, sizeof(radio_buf));
            printf("%s\n", radio_buf);
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