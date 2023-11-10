#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

// For the functionality of a BITSv5 board
// NOT FLIGHT CODE
int main() {
    stdio_init_all();

    sleep_ms(5000);

    // set init GPIO0 and set as output
    // gpio_init(0);
    // gpio_set_dir(0, GPIO_OUT);

    // int *p, addr, value;

    // Compute the memory-mapped address, remembering to include the offset
    // for RAM (XIP_BASE)
    // addr = XIP_BASE + FLASH_TARGET_OFFSET;
    // p = (int *)addr;  // Place an int pointer at our memory-mapped address

    int input = 0;

    radio_receive_cont();

    while () {
        
    }

    while (true) {
        // value = *p;  // Store the value at this address for later use

        // printf("Value at %X is %X\n", p, value);
        // printf("Enter a number: ");
        // scanf("%d", input);

        printf("Hello, BITS!\n");

        get_radio_errors();

        gpio_put(0, true);
        sleep_ms(1000);
        gpio_put(0, false);
        sleep_ms(1000);
        // p++;
    }
}