#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../../libraries/rp2040-drf1262-lib/SX1262.h"
#include "BITSv5.h"
#include "hardware/flash.h"
#include "hardware/gpio.h"
#include "pico/binary_info.h"
#include "pico/rand.h"
#include "pico/stdlib.h"
#include "pico/unique_id.h"

// Define the correct flash size for BITSv5 (16MB)
#undef PICO_FLASH_SIZE_BYTES
#define PICO_FLASH_SIZE_BYTES (16 * 1024 * 1024)

// Flash-based address of the last sector
#define FLASH_TARGET_OFFSET (PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE)

#define RX_TEST 0
#define TX_TEST 1

void rx_test(void);
void transmit_test(void);

DRF1262 radio(spi0, CS_PIN, SCK_PIN, MOSI_PIN, MISO_PIN, TXEN_PIN, DIO1_PIN,
              BUSY_PIN, SW_PIN);

char id[2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1] = {0};

short debug_msgs = 1;  // controls if debug messages are printed

// For the functionality of a BITSv5 board
// NOT FLIGHT CODE
int main() {
    stdio_init_all();

    sleep_ms(5000);

    gpio_init(0);
    gpio_set_dir(0, GPIO_OUT);
    gpio_put(0, 0);

    radio.radio_init();

    while (true) {
        printf("\n\n\n\n\n\n\n\n\n\n\n\n");

        if (debug_msgs) {
            printf("============BITSv5: %s (DEBUG)============\n", id);
        } else {
            printf("============BITSv5: %s============\n", id);
        }

#if TX_TEST
        sleep_ms(4500);
        transmit_test();
#endif

#if RX_TEST
        rx_test();
        get_radio_errors();
#endif

        gpio_put(0, true);
        sleep_ms(1000);
        gpio_put(0, false);
        sleep_ms(1000);
    }
}

void transmit_test() {
    printf("Transmit Test\n");

    char data[] = "bits5";

    data[4] = (char)get_rand_32();

    printf("Sending payload: %s", data);

    radio.radio_send((uint8_t *)data, 5);

    sleep_ms(100);

#if DEBUG
    get_radio_errors();
    get_irq_status();
#endif

    radio.clear_irq_status();

#if DEBUG
    get_irq_status();
#endif
}

void rx_test() {
    char data[6] = {
        '\0', '\0', '\0', '\0', '\0', '\0',
    };

    radio.radio_receive_single();

    while (!gpio_get(DIO1_PIN)) {
        sleep_ms(1);
    }

    // sleep_ms(10);

    // radio.get_rx_buffer_status();

    radio.get_irq_status();
    radio.clear_irq_status();
    radio.get_irq_status();

    radio.read_radio_buffer((uint8_t *)data, 5);

    printf("Got some data: %s | %x%x", data, data[4]);
}