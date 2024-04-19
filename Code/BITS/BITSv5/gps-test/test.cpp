#include <stdio.h>
#include <string.h>
#include <time.h>

#include "../../../libraries/libnmea/src/nmea/nmea.h"
#include "../../../libraries/rp2040-drf1262-lib/SX1262.h"
#include "../BITSv5.h"
#include "../BITSv5_GPS.h"
#include "../BITSv5_Radio.h"
#include "hardware/i2c.h"
#include "pico/binary_info.h"
#include "pico/stdlib.h"

DRF1262 radio(spi1, RADIO_CS, SCK_PIN, MOSI_PIN, MISO_PIN, TXEN_PIN, DIO1_PIN,
              BUSY_PIN, SW_PIN, RADIO_RST);

void ubx_inf_debug(void);
void ubx_cfg_prt(void);
void ubx_cfg_cfg(void);
void ubx_cfg_dat(void);

void transmit_test(uint8_t *buf, size_t len);
static int _split_string_by_comma(char *string, char **values, int max_values);
int nmea_position_parse(char *s, nmea_position *pos);
nmea_cardinal_t nmea_cardinal_direction_parse(char *s);
int nmea_date_parse(char *s, struct tm *date);
int nmea_time_parse(char *s, struct tm *time);

int main() {
    // Pins
    const uint sda_pin = SDA_PIN;
    const uint scl_pin = SCL_PIN;

    uint result1 = 0;
    uint result2 = 0;

    uint8_t msg[3] = {0x00, 0x00, 0x00};

    setup();

    gpio_init(RADIO_RST);
    gpio_set_dir(RADIO_RST, GPIO_OUT);
    gpio_put(RADIO_RST, 1);

    sleep_ms(5000);

    radio.debug_msg_en = 0;
    radio.radio_init();

    // Initialize I2C pins
    gpio_set_function(sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(scl_pin, GPIO_FUNC_I2C);

    uint8_t rx_msg = 0;

    while (true) {
        // printf("%c", uart_getc(uart1));

        // if (transmit) {
        //     transmit_test((uint8_t *)radio_tx_buf, sizeof(radio_tx_buf));
        //     transmit = false;
        // }

        get_gps_data();
    }
}

void ubx_inf_debug() {
    const uint8_t sync1 = 0xB5;
    const uint8_t sync2 = 0x62;
    const uint8_t msg_class = 0x04;
    const uint8_t id = 0x04;
    const char *payload = "hey";
    uint8_t CK_A = 0;
    uint8_t CK_B = 0;

    for (int i = 0; i < 10; i++) {
        CK_A = CK_A + payload[i];
        CK_B = CK_B + CK_A;
    }

    const uint8_t message[9] = {sync1,      sync2,      msg_class,
                                id,         payload[0], payload[1],
                                payload[2], CK_A,       CK_B};

    i2c_write_blocking(i2c, GPS_ADDR, message, 9, false);
}

void ubx_cfg_prt() {
    const uint8_t sync1 = 0xB5;
    const uint8_t sync2 = 0x62;
    const uint8_t msg_class = 0x06;
    const uint8_t id = 0x00;
    const uint8_t len = 0x01;
    const uint8_t payload = 0x00;
    uint8_t CK_A = 0;
    uint8_t CK_B = 0;
    uint result1 = 0;
    uint result2 = 0;
    uint8_t rx_msg = 0x00;

    const uint8_t scratch[8] = {sync1, sync2,   msg_class, id,
                                len,   payload, CK_A,      CK_B};

    for (int i = 2; i < 6; i++) {
        CK_A = CK_A + scratch[i];
        CK_B = CK_B + CK_A;
    }

    const uint8_t message[8] = {sync1, sync2,   msg_class, id,
                                len,   payload, CK_A,      CK_B};

    printf("Writing UBX-CFG-PRT Message...\n");

    i2c_write_blocking(i2c, GPS_ADDR, message, 8, false);

    sleep_ms(1500);

    // i2c_write_blocking(i2c, GPS_ADDR, &REG_NUM_BYTES_MSB, 1, true);

    printf("received msg:");

    for (int i = 0; i < 20; i++) {
        result2 = i2c_read_blocking(i2c, GPS_ADDR, &rx_msg, 1, false);
        if (result2 == PICO_ERROR_GENERIC)
            printf("\ni2c error occurred %x\n\n", result2);
        else
            printf(" %x", rx_msg);
    }

    printf("\n\n");
}

void ubx_cfg_cfg() {
    const uint8_t sync1 = 0xB5;
    const uint8_t sync2 = 0x62;
    const uint8_t msg_class = 0x06;
    const uint8_t id = 0x09;
    const uint8_t len = 0x0C;
    const uint8_t payload[12] = {0x00, 0x01, 0xF1, 0xFF, 0x00, 0x00,
                                 0x00, 0x00, 0x00, 0x01, 0xF1, 0xFF};
    uint8_t CK_A = 0;
    uint8_t CK_B = 0;
    uint result1 = 0;
    uint result2 = 0;
    uint8_t rx_msg = 0x00;

    const uint8_t scratch[19] = {
        sync1,       sync2,       msg_class,  id,         len,
        payload[0],  payload[1],  payload[2], payload[3], payload[4],
        payload[5],  payload[6],  payload[7], payload[8], payload[9],
        payload[10], payload[11], CK_A,       CK_B};

    for (int i = 2; i < 6; i++) {
        CK_A = CK_A + scratch[i];
        CK_B = CK_B + CK_A;
    }

    const uint8_t message[21] = {0xB5, 0x62, 0x06, 0x09, 0x0D, 0x00, 0xFF,
                                 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                 0xFF, 0xFF, 0x00, 0x00, 0x17, 0x2F, 0xAE};

    printf("Writing UBX-CFG-CFG Message...\n");

    i2c_write_blocking(i2c, GPS_ADDR, message, 21, false);

    sleep_ms(1500);

    // i2c_write_blocking(i2c, GPS_ADDR, &REG_NUM_BYTES_MSB, 1, true);

    printf("received msg:");

    for (int i = 0; i < 20; i++) {
        result2 = i2c_read_blocking(i2c, GPS_ADDR, &rx_msg, 1, false);
        if (result2 == PICO_ERROR_GENERIC)
            printf("\ni2c error occurred %x\n\n", result2);
        else
            printf(" %x", rx_msg);
    }

    printf("\n\n");
}

void ubx_cfg_dat() {
    uint result1 = 0;
    uint result2 = 0;
    uint8_t rx_msg = 0x00;

    const uint8_t message[10] = {0xB5, 0x62, 0x06, 0x06, 0x02,
                                 0x00, 0x00, 0x00, 0x0E, 0x4A};

    printf("Writing UBX-CFG-DAT Message...\n");

    i2c_write_blocking(i2c, GPS_ADDR, message, 10, false);

    sleep_ms(1500);

    printf("received msg:");

    for (int i = 0; i < 20; i++) {
        result2 = i2c_read_blocking(i2c, GPS_ADDR, &rx_msg, 1, false);
        if (result2 == PICO_ERROR_GENERIC)
            printf("\ni2c error occurred %x\n\n", result2);
        else
            printf(" %x", rx_msg);
    }

    printf("\n\n");
}

void transmit_test(uint8_t *buf, size_t len) {
    printf("Transmit Test\n");

    led_on();

    // tx_done = false;

    // buf[0] = (char)get_rand_32();

    radio.radio_send(buf, len);

    while (gpio_get(BUSY_PIN) && !gpio_get(DIO1_PIN))
        ;

    printf("Starting wait\n");

    sleep_ms(10000);

    led_off();
    printf("%s\n", (char *)buf);
    // radio.disable_tx();
    // radio.radio_receive_single();

    radio.get_radio_errors();

    radio.clear_irq_status();

    radio.radio_receive_single();

    radio.get_radio_errors();
}

void gpio_callback(uint gpio, uint32_t events) {
    if (gpio == DIO1_PIN) {
        printf("DIO1 ISR\n");
        radio.get_irq_status();

        if (radio.irqs.tx_done) {
            printf("TX ISR\n");
            radio.disable_tx();
            radio.radio_receive_cont();
            tx_done = true;
            radio.irqs.tx_done = false;
        }

        if (radio.irqs.rx_done) {
            printf("RX ISR\n");
            radio.read_radio_buffer((uint8_t *)radio_rx_buf,
                                    sizeof(radio_rx_buf));
            printf("%s\n", radio_rx_buf);
            radio.get_packet_status();
            printf("RSSI: %d dBm Signal RSSI: %d SNR: %d dB\n",
                   radio.pkt_stat.rssi_pkt, radio.pkt_stat.signal_rssi_pkt,
                   radio.pkt_stat.snr_pkt);

            radio.irqs.rx_done = false;
            send_ack = true;
        }

        radio.clear_irq_status();
    }
}
