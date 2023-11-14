#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

const uint cs_pin = 21;
const uint sck_pin = 18;
const uint mosi_pin = 19;
const uint miso_pin = 20;
const uint txen_pin = 1;
const uint dio1_pin = 3;
const uint busy_pin = 6;
const uint sw_pin = 9;

void transmit_test(void);

// For the functionality of a BITSv5 board
// NOT FLIGHT CODE
int main() {
    uint8_t i = 0;

    stdio_init_all();

    sleep_ms(5000);

    radio_init();

    while (true) {
        printf("======Hello, Xbee Joint!======\n");

        printf("Enter char to Receive: ");
        printf("%c\n", getchar_timeout_us(0));

        transmit_test();
        printf("\n\n\n");
    }
}

void transmit_test() {
    printf("Transmit Test");

    radio_send();

    sleep_ms(3000);

    get_radio_errors();

    get_irq_status();
    clear_irq_status();
    get_irq_status();
}

void rx_test() {
    radio_receive_cont();

    while (!gpio_get(dio1_pin)) {
        sleep_ms(10);
    }

    sleep_ms(1000);

    get_irq_status();

    read_radio_buffer();
}