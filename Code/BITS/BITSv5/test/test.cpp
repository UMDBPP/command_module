#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "BITSv5.h"
#include "SX1262.h"
#include "hardware/flash.h"
#include "hardware/gpio.h"
#include "pico/binary_info.h"
#include "pico/stdlib.h"

// Define the correct flash size for BITSv5 (16MB)
#undef PICO_FLASH_SIZE_BYTES
#define PICO_FLASH_SIZE_BYTES (16 * 1024 * 1024)

// Flash-based address of the last sector
#define FLASH_TARGET_OFFSET (PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE)

void transmit_test(void);
void rx_test(void);

// For the functionality of a BITSv5 board
// NOT FLIGHT CODE
int main() {
    stdio_init_all();

    sleep_ms(5000);

    gpio_init(0);
    gpio_set_dir(0, GPIO_OUT);
    gpio_put(0, 0);

    radio_init();

    while (true) {
        printf("Hello, BITS! Transmitting Now!\n");

        transmit_test();

        printf("\n\n\n");

        gpio_put(0, true);
        sleep_ms(1000);
        gpio_put(0, false);
        sleep_ms(1000);
    }
}

void transmit_test() {
    printf("Transmit Test\n");

    radio_send();

    sleep_ms(3000);

    get_radio_errors();

    get_irq_status();
    clear_irq_status();
    get_irq_status();
}

void rx_test() {
    radio_receive_single();

    while (!gpio_get(DIO1_PIN)) {
        sleep_ms(10);
    }

    sleep_ms(1000);

    get_rx_buffer_status();

    get_irq_status();
    clear_irq_status();
    get_irq_status();

    read_radio_buffer();
}