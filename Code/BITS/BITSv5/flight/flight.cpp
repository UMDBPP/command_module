#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../../libraries/rp2040-drf1262-lib/SX1262.h"
#include "../BITSv5.h"
#include "hardware/flash.h"
#include "hardware/gpio.h"
#include "pico/binary_info.h"
#include "pico/rand.h"
#include "pico/stdlib.h"
#include "pico/unique_id.h"
#include "pico/util/queue.h"

// Define the correct flash size for BITSv5 (16MB)
#undef PICO_FLASH_SIZE_BYTES
#define PICO_FLASH_SIZE_BYTES (16 * 1024 * 1024)

#define FIFO_LENGTH 32

queue_t gps_message_fifo;

DRF1262 radio(spi1, CS_PIN, SCK_PIN, MOSI_PIN, MISO_PIN, TXEN_PIN, DIO1_PIN,
              BUSY_PIN, SW_PIN);

char id[2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1] = {0};

short debug_msgs = 0;  // controls if debug messages are printed

char radio_rx_buf[100] = {0};
uint8_t radio_tx_buf[100] = {0};
uint8_t radio_ack_buf[100] = {0};
char gps_buf[90] = {0};
uint gps_buf_offset = 0;

volatile bool tx_done = false;
volatile bool transmit = false;
volatile bool rx_done = false;

repeating_timer_t tx_timer;

void rx_test(void);
void transmit_test(uint8_t *buf, size_t len);
void setup_led();
void led_on();
void led_off();
void gpio_callback(uint gpio, uint32_t events);
bool tx_timer_callback(repeating_timer_t *rt);

// FLIGHT CODE
int main() {
    stdio_init_all();

    // set_sys_clock_48mhz();

    gpio_set_irq_enabled_with_callback(DIO1_PIN, GPIO_IRQ_EDGE_RISE, true,
                                       &gpio_callback);

    uart_init(uart1, 9600);
    gpio_set_function(SCL_PIN, GPIO_FUNC_UART);

    setup_led();

    led_off();

    sleep_ms(5000);

    // negative timeout means exact delay (rather than delay between
    // callbacks)
    if (!add_repeating_timer_us(-60000000, tx_timer_callback, NULL,
                                &tx_timer)) {
        printf("Failed to add timer\n");
        return 1;
    }

    radio.debug_msg_en = debug_msgs;
    radio.radio_init();

    pico_get_unique_board_id_string(id,
                                    2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1);

    printf("\n%s %s\n", __DATE__, __TIME__);

    // radio.radio_receive_single();

    while (true) {
        // printf("\n\n\n\n\n\n\n\n\n\n\n\n\n");

        // if (debug_msgs) {
        //     printf("============BITSv5: %s (DEBUG)============\n", id);
        // } else {
        //     printf("============BITSv5: %s============\n", id);
        // }

        // radio.get_irq_status();

        // if (radio.irqs.RX_DONE) {
        //     radio.read_radio_buffer((uint8_t *)radio_rx_buf,
        //                             sizeof(radio_rx_buf));
        //     printf("%s\n", radio_rx_buf);
        //     rx_done = false;

        //     char ack_mesg[15] = "ack-__________";
        //     strncpy(ack_mesg + 4, (char *)radio_rx_buf, 9);
        //     strcpy((char *)radio_ack_buf + 1, ack_mesg);

        //     radio.clear_irq_status();

        //     transmit_test(radio_ack_buf, strlen((char *)radio_ack_buf));
        // }

        if (transmit) {
            transmit_test((uint8_t *)radio_tx_buf, sizeof(radio_tx_buf));
            transmit = false;
        }

        // if (tx_done) {
        // }

        while (uart_is_readable(uart1) > 0) {
            char c = uart_getc(uart1);
            if (c == '*') {
                strcpy((char *)radio_tx_buf, gps_buf);
                gps_buf_offset = 0;
            }

            if (gps_buf_offset >= 89) gps_buf_offset = 0;

            if (c == '$') gps_buf_offset = 0;

            gps_buf[gps_buf_offset] = c;
            gps_buf_offset++;
            printf("%c", c);
        }
    }
}

void transmit_test(uint8_t *buf, size_t len) {
    printf("Transmit Test\n");

    led_on();

    tx_done = false;

    buf[0] = (char)get_rand_32();

    radio.radio_send(buf, len);

    sleep_ms(10);

    led_off();
    printf("%s\n", (char *)buf);
    // radio.disable_tx();
    // radio.radio_receive_single();

#if INCLUDE_DEBUG
    radio.get_radio_errors();
    radio.get_irq_status();
#endif

    radio.clear_irq_status();

#if INCLUDE_DEBUG
    radio.get_irq_status();
#endif
}

void setup_led() {
    gpio_init(LED_PIN);
    gpio_set_dir(0, GPIO_OUT);
    gpio_put(LED_PIN, 0);
}

void led_on() { gpio_put(LED_PIN, true); }

void led_off() { gpio_put(LED_PIN, false); }

void gpio_callback(uint gpio, uint32_t events) {
    printf("i\n");
    if (gpio == DIO1_PIN) {
        radio.get_irq_status();

        if (radio.irqs.RX_DONE) {
            rx_done = true;
        }

        if (radio.irqs.TX_DONE) {
            tx_done = true;
        }

        radio.clear_irq_status();
    }
}

bool tx_timer_callback(repeating_timer_t *rt) {
    transmit = true;

    return true;  // keep repeating
}