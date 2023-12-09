#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "(Not)XBee_Joint.h"
#include "SX1262.h"
#include "hardware/flash.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "pico/binary_info.h"
#include "pico/stdlib.h"

// Define the correct flash size for BITSv5 (16MB)
#undef PICO_FLASH_SIZE_BYTES
#define PICO_FLASH_SIZE_BYTES (16 * 1024 * 1024)

// Flash-based address of the last sector
#define FLASH_TARGET_OFFSET (PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE)

#define RX_TEST 1
#define TX_TEST 0

void rx_test(void);
void transmit_test(void);

// For the functionality of a (Not)Xbee Joint board
// NOT PRODUCTION CODE
int main() {
    uint8_t i = 0;

    stdio_init_all();

    sleep_ms(5000);

    radio_init();

    write_radio_buffer();

    while (true) {
        printf("======Hello, Xbee Joint!======\n");

        // printf("Enter char to Receive: ");
        // printf("%c\n", getchar_timeout_us(0));

#if TX_TEST
        transmit_test();
        sleep_ms(5000);
#endif

#if RX_TEST
        rx_test();
        get_radio_errors();
#endif

        printf("\n\n\n");

        sleep_ms(10);
    }
}

void transmit_test() {
    printf("Transmit Test\n");

    radio_send();

    sleep_ms(100);

    get_radio_errors();

    get_irq_status();
    clear_irq_status();
    get_irq_status();
}

void rx_test() {
    radio_receive_single();

    while (!gpio_get(DIO1_PIN)) {
        sleep_ms(1);
    }

    // sleep_ms(10);

    get_rx_buffer_status();

    get_irq_status();
    clear_irq_status();
    get_irq_status();

    read_radio_buffer();
}