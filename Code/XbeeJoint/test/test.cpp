#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SX126x.h"
#include "hardware/flash.h"
#include "hardware/gpio.h"
#include "pico/binary_info.h"
#include "pico/stdlib.h"

#define LORA_BANDWIDTH \
    4  // bandwidth=125khz  0:250kHZ,1:125kHZ,2:62kHZ,3:20kHZ.... look for radio
       // line 392
#define LORA_SPREADING_FACTOR 7  // spreading factor=11 [SF5..SF12]
#define LORA_CODINGRATE \
    1  // [1: 4/5,
       //  2: 4/6,
       //  3: 4/7,
       //  4: 4/8]

#define LORA_PREAMBLE_LENGTH 8            // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT 0             // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON false  // variable data payload
#define LORA_IQ_INVERSION_ON false
#define LORA_PAYLOADLENGTH \
    0  // 0: variable receive length
       // 1..255 payloadlength

SX126x lora(21,  // Port-Pin Output: SPI select
            0,   // Port-Pin Output: Reset
            6,   // Port-Pin Input:  Busy
            3    // Port-Pin Input:  Interrupt DIO1
);

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

    printf("0\n");

    // set init GPIO0 and set as output
    // gpio_init(0);
    // gpio_set_dir(0, GPIO_OUT);

    // int *p, addr, value;

    // Compute the memory-mapped address, remembering to include the offset
    // for RAM (XIP_BASE)
    // addr = XIP_BASE + FLASH_TARGET_OFFSET;
    // p = (int *)addr;  // Place an int pointer at our memory-mapped address

    printf("1\n");

    lora.begin(
        SX126X_PACKET_TYPE_LORA,  // LoRa or FSK, FSK currently not supported
        915000000,                // frequency in Hz
        20);                      // tx power in dBm

    printf("2\n");

    lora.LoRaConfig(LORA_SPREADING_FACTOR, LORA_BANDWIDTH, LORA_CODINGRATE,
                    LORA_PREAMBLE_LENGTH, LORA_PAYLOADLENGTH,
                    false,   // crcOn
                    false);  // invertIrq

    printf("3\n");

    uint8_t i = 0;

    int input = 0;

    while (true) {
        // value = *p;  // Store the value at this address for later use

        // printf("Value at %X is %X\n", p, value);
        // printf("Enter a number: ");
        // scanf("%d", input);

        printf("Hello, BITS! Transmitting %u now\n", i);

        lora.Send(&i, 1, SX126x_TXMODE_SYNC);
        i++;

        // gpio_put(0, true);
        sleep_ms(1000);
        // gpio_put(0, false);
        sleep_ms(1000);
        // p++;
    }
}