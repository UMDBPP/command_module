#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../../libraries/rp2040-drf1262-lib/SX1262.h"
#include "../BITSv5.h"
#include "hardware/gpio.h"
#include "pico/binary_info.h"
#include "pico/rand.h"
#include "pico/stdlib.h"
#include "pico/unique_id.h"

DRF1262 radio(spi1, RADIO_CS, SCK_PIN, MOSI_PIN, MISO_PIN, TXEN_PIN, DIO1_PIN,
              BUSY_PIN, SW_PIN, RADIO_RST);

char id[2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1] = {0};

char radio_rx_buf[100] = {0};
uint8_t radio_tx_buf[100] = "_hello!";
uint8_t radio_ack_buf[100] = {0};

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
void setup_spi();

int main() {
    stdio_init_all();

    gpio_set_irq_enabled_with_callback(DIO1_PIN, GPIO_IRQ_EDGE_RISE, true,
                                       &gpio_callback);

    setup_led();
    led_off();

    gpio_init(EXTINT_PIN);
    gpio_set_dir(EXTINT_PIN, GPIO_IN);
    gpio_init(TIMEPULSE_PIN);
    gpio_set_dir(TIMEPULSE_PIN, GPIO_IN);

    gpio_init(RADIO_RST);
    gpio_set_dir(RADIO_RST, GPIO_OUT);
    gpio_put(RADIO_RST, 1);

    sleep_ms(5000);

    radio.debug_msg_en = 0;
    radio.radio_init();

    // negative timeout means exact delay (rather than delay between
    // callbacks)
    if (!add_repeating_timer_us(-20000000, tx_timer_callback, NULL,
                                &tx_timer)) {
        printf("Failed to add timer\n");
        return 1;
    }

    printf("\n%s %s\n", __DATE__, __TIME__);

    while (true) {
        if (transmit) {
            transmit_test((uint8_t *)radio_tx_buf, sizeof(radio_tx_buf));
            transmit = false;
        }
    }
}

void transmit_test(uint8_t *buf, size_t len) {
    printf("Transmit Test\n");

    led_on();

    tx_done = false;

    buf[0] = (char)get_rand_32();

    radio.radio_send(buf, len);

    printf("%s\n", (char *)buf);

    radio.get_radio_errors();
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
            led_off();
        }

        if (radio.irqs.RX_DONE) {
            printf("RX ISR\n");
            radio.read_radio_buffer((uint8_t *)radio_rx_buf,
                                    sizeof(radio_rx_buf));
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