#include "radio.h"

#include <stdio.h>
#include <string.h>

#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "pico/binary_info.h"
#include "pico/stdlib.h"
#include "sx126x-board.h"
#include "sx126x.h"

/*
 * if you want to use TCXO please open sx1262.h line 29
 */

uint8_t mode = USER_MODE_TX; /*mode  SET current work mode: TX or RX*/
/*Default frequency：868MHZ Bandwidth：125KHZ，RF_FACTOR：11 */
#define RF_FREQUENCY 868000000  // Hz  center frequency
#define TX_OUTPUT_POWER 22      // dBm tx output power
#define LORA_BANDWIDTH \
    1  // bandwidth=125khz   0:250kHZ,1:125kHZ,2:62kHZ,3:20kHZ.... lookfor radio
       // line 392
#define LORA_SPREADING_FACTOR 11  // spreading factor=11 [SF5..SF12]
#define LORA_CODINGRATE \
    1                                     // [1: 4/5,
                                          //  2: 4/6,
                                          //  3: 4/7,
                                          //  4: 4/8]
#define LORA_PREAMBLE_LENGTH 8            // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT 0             // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON false  // variable data payload
#define LORA_IQ_INVERSION_ON false

typedef enum {
    LOWPOWER,
    RX,
    RX_TIMEOUT,
    RX_ERROR,
    TX,
    TX_TIMEOUT,
} States_t;

#define RX_TIMEOUT_VALUE 1000
#define BUFFER_SIZE 64  // Define the payload size here
States_t State = LOWPOWER;

RadioEvents_t RadioEvents;
uint16_t BufferSize = BUFFER_SIZE;
uint8_t Buffer[BUFFER_SIZE];
int8_t RssiValue = 0;
int8_t SnrValue = 0;

void OnTxDone(void);
void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr);
void OnTxTimeout(void);
void OnRxTimeout(void);
void OnRxError(void);

uint8_t i = 0;

int main() {
    stdio_init_all();

    SX126xIoInit();  // Initializes the radio I/Os pins

    while (true) {
        printf("Hello, BITS!\n");
        sleep_ms(1000);
    }
}