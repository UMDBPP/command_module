#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../../libraries/rp2040-drf1262-lib/SX1262.h"
#include "../BITSv5.h"
#include "hardware/gpio.h"
#include "pico/binary_info.h"
#include "pico/rand.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include "pico/unique_id.h"

DRF1262 radio(spi1, RADIO_CS, SCK_PIN, MOSI_PIN, MISO_PIN, TXEN_PIN, DIO1_PIN,
              BUSY_PIN, SW_PIN, RADIO_RST);

char id[2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1] = {0};

char radio_rx_buf[100] = {0};
uint8_t radio_tx_buf[100] = "_hello!";
uint8_t radio_ack_buf[100] = {0};
char ack[] = "ack - bitsv5 alive!";

volatile bool tx_done = false;
volatile bool transmit = false;
volatile bool rx_done = false;
volatile bool send_ack = false;

repeating_timer_t tx_timer;
alarm_pool_t *ack_alarm_pool;
alarm_id_t ack_alarm_id;

void rx_test(void);
void transmit_test(uint8_t *buf, size_t len);
void setup_led();
void led_on();
void led_off();
void gpio_callback(uint gpio, uint32_t events);
bool tx_timer_callback(repeating_timer_t *rt);

static int64_t ack_timer_callback(alarm_id_t id, void *user_data);
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

    ack_alarm_pool = alarm_pool_create_with_unused_hardware_alarm(4);

    // while (!stdio_usb_connected()) {
    //     sleep_ms(10);
    // }

    sleep_ms(5000);

    radio.debug_msg_en = 0;
    radio.radio_init();

    // negative timeout means exact delay (rather than delay between
    // callbacks)
    if (!add_repeating_timer_us(-30000000, tx_timer_callback, NULL,
                                &tx_timer)) {
        printf("Failed to add timer\n");
        return 1;
    }

    printf("\n%s %s\n", __DATE__, __TIME__);

    radio.radio_receive_cont();

    while (true) {
        if (transmit) {
            transmit_test((uint8_t *)radio_tx_buf, sizeof(radio_tx_buf));
            transmit = false;
        }

        if (rx_done) {
            printf("Received\n");
            printf("%s\n", radio_rx_buf);
            printf("RSSI: %d dBm Signal RSSI: %d SNR: %d dB\n",
                   radio.pkt_stat.rssi_pkt, radio.pkt_stat.signal_rssi_pkt,
                   radio.pkt_stat.snr_pkt);

            if (strncmp("ack", radio_rx_buf, 3) != 0) {
                ack_alarm_id = alarm_pool_add_alarm_in_ms(
                    ack_alarm_pool, 1000, ack_timer_callback, NULL, true);
            }

            rx_done = false;
        }

        if (send_ack) {
            printf("ACK\n");
            transmit_test((uint8_t *)ack, sizeof(ack));
            send_ack = false;
        }
    }
}

void transmit_test(uint8_t *buf, size_t len) {
    printf("Transmit\n");

    led_on();

    tx_done = false;

    // buf[0] = (char)get_rand_32();

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

static int64_t ack_timer_callback(alarm_id_t id, void *user_data) {
    send_ack = true;
    return 0;  // don't repeat
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

            radio.get_packet_status();
            radio.irqs.RX_DONE = false;
            rx_done = true;
        }

        radio.clear_irq_status();
    }
}
