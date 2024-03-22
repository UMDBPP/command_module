#include <stdio.h>
#include <string.h>
#include <time.h>

#include "../../../libraries/libnmea/src/nmea/nmea.h"
#include "../../../libraries/rp2040-drf1262-lib/SX1262.h"
#include "../BITSv5.h"
#include "hardware/i2c.h"
#include "pico/binary_info.h"
#include "pico/stdlib.h"

#define NO_GPS_DATA 0xFF

// Registers
static const uint8_t REG_NUM_BYTES_MSB = 0xFD;
static const uint8_t REG_NUM_BYTES_LSB = 0xFE;
static const uint8_t REG_DATA = 0xFF;

volatile bool transmit;

repeating_timer_t tx_timer;

uint8_t radio_tx_buf[100] = "hello!";
unsigned char pos = 0;  // Keep track of current position in radio_tx_buf
char *values[10];       // Array of char pointers, for pointing to positons in
                        // radio_tx_buf

// These structs have been largely lifted from libnmea.
/* NMEA cardinal direction types */
typedef char nmea_cardinal_t;
#define NMEA_CARDINAL_DIR_NORTH (nmea_cardinal_t)'N'
#define NMEA_CARDINAL_DIR_EAST (nmea_cardinal_t)'E'
#define NMEA_CARDINAL_DIR_SOUTH (nmea_cardinal_t)'S'
#define NMEA_CARDINAL_DIR_WEST (nmea_cardinal_t)'W'
#define NMEA_CARDINAL_DIR_UNKNOWN (nmea_cardinal_t)'\0'

#define NMEA_TIME_FORMAT "%H%M%S"
#define NMEA_TIME_FORMAT_LEN 6

#define TM_YEAR_START 1900
#define RMC_YEAR_START 2000

/* GPS position struct */
// typedef struct _nmea_position {
//     double minutes;  // don't like that this is stored as a double
//     int degrees;
//     nmea_cardinal_t cardinal;
// } nmea_position;

/* Pared down struct for GGA sentence data */
typedef struct _nmea_gga {
    struct tm time;
    nmea_position longitude;
    nmea_position latitude;
    unsigned char position_fix;
} nmea_gga;

nmea_gga data;
tm date;
nmea_position lon;
nmea_position lat;

// Ports
i2c_inst_t *i2c = i2c0;

DRF1262 radio(spi1, RADIO_CS, SCK_PIN, MOSI_PIN, MISO_PIN, TXEN_PIN, DIO1_PIN,
              BUSY_PIN, SW_PIN, RADIO_RST);

void ubx_inf_debug(void);
void ubx_cfg_prt(void);
void ubx_cfg_cfg(void);
void ubx_cfg_dat(void);
bool tx_timer_callback(repeating_timer_t *rt);
void setup_led();
void led_on();
void led_off();
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

    stdio_init_all();

    set_sys_clock_48mhz();

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
    if (!add_repeating_timer_us(-60000000, tx_timer_callback, NULL,
                                &tx_timer)) {
        printf("Failed to add timer\n");
        return 1;
    }

    // Initialize I2C port at 100 kHz
    i2c_init(i2c, 100 * 1000);

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

        result2 = i2c_read_blocking(i2c, GPS_ADDR, &rx_msg, 1, false);
        if (result2 == PICO_ERROR_GENERIC)
            printf("\ni2c error occurred %x\n\n", result2);
        else {
            if (rx_msg != NO_GPS_DATA) {
                // printf("%c", rx_msg);
                // End sequence is "\r\n"
                radio_tx_buf[pos++] = rx_msg;
                if (rx_msg == '\n') {
                    // We've reached the end of the sentence/line.
                    radio_tx_buf[pos++] = '\0';  // NULL-terminate for safety
                    // Parse NMEA sentence, only if GNGGA for now
                    if (strlen((char *)radio_tx_buf) >= 6 &&
                        strncmp((char *)(&radio_tx_buf[3]), "GGA", 3) == 0) {
                        printf("%s\n", radio_tx_buf);
                        // Split msg into values, parse each value, then assign
                        // each result to appropriate place in struct
                        _split_string_by_comma((char *)radio_tx_buf, values, 7);
                        // nmea_gga data;
                        // tm date;
                        // nmea_position lon;
                        // nmea_position lat;
                        // nmea_date_parse(values[1], &date);
                        nmea_time_parse(values[1], &date);
                        data.time = date;
                        nmea_position_parse(values[4], &lon);
                        lon.cardinal = nmea_cardinal_direction_parse(values[5]);
                        data.longitude = lon;
                        nmea_position_parse(values[2], &lat);
                        lon.cardinal = nmea_cardinal_direction_parse(values[3]);
                        data.latitude = lat;
                        data.position_fix = atoi(values[6]);

                        printf("Hour: %d Min: %d Sec: %d\n", date.tm_hour,
                               date.tm_min, date.tm_sec);
                    }
                    pos = 0;  // Reset pos for reading in the next sentence
                }
            }
        }
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

bool tx_timer_callback(repeating_timer_t *rt) {
    transmit = true;

    return true;  // keep repeating
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

void setup_led() {
    gpio_init(LED_PIN);
    gpio_set_dir(0, GPIO_OUT);
    gpio_put(LED_PIN, 0);
}

void led_on() { gpio_put(LED_PIN, true); }

void led_off() { gpio_put(LED_PIN, false); }

/**
 * Splits a string by comma.
 *
 * string is the string to split, will be manipulated. Needs to be
 *        null-terminated.
 * values is a char pointer array that will be filled with pointers to the
 *        splitted values in the string.
 * max_values is the maximum number of values to be parsed.
 *
 * Returns the number of values found in string.
 *
 * Copied from libnmea/src/nmea/nmea.c.
 */
static int _split_string_by_comma(char *string, char **values, int max_values) {
    int i = 0;

    values[i++] = string;
    while (i < max_values && NULL != (string = strchr(string, ','))) {
        *string = '\0';
        values[i++] = ++string;
    }

    return i;
}

/**
 * Parses a position (latitude or longitude). TODO: Need to modify if want
 * minutes as int.
 *
 * s is the string containing the data in string form.
 * pos is a pointer to an nmea_position struct which will be filled with the
 *      data.
 *
 * Copied from libnmea/src/parsers/parse.c.
 */
int nmea_position_parse(char *s, nmea_position *pos) {
    char *cursor;

    pos->degrees = 0;
    pos->minutes = 0;

    if (s == NULL || *s == '\0') {
        return -1;
    }

    /* decimal minutes */
    if (NULL == (cursor = strchr(s, '.'))) {
        return -1;
    }

    /* minutes starts 2 digits before dot */
    cursor -= 2;
    pos->minutes = atof(cursor);
    *cursor = '\0';

    /* integer degrees */
    cursor = s;
    pos->degrees = atoi(cursor);

    return 0;
}

/**
 * Parses a cardinal direction.
 *
 * s is a pointer to a char, which contains the data to be parsed.
 *
 * Returns an nmea_cardinal_t (basically an enum).
 *
 * Copied from libnmea/src/parsers/parse.c.
 */
nmea_cardinal_t nmea_cardinal_direction_parse(char *s) {
    if (NULL == s || '\0' == *s) {
        return NMEA_CARDINAL_DIR_UNKNOWN;
    }

    switch (*s) {
        case NMEA_CARDINAL_DIR_NORTH:
            return NMEA_CARDINAL_DIR_NORTH;
        case NMEA_CARDINAL_DIR_EAST:
            return NMEA_CARDINAL_DIR_EAST;
        case NMEA_CARDINAL_DIR_SOUTH:
            return NMEA_CARDINAL_DIR_SOUTH;
        case NMEA_CARDINAL_DIR_WEST:
            return NMEA_CARDINAL_DIR_WEST;
        default:
            break;
    }

    return NMEA_CARDINAL_DIR_UNKNOWN;
}

/**
 * Parses a date.
 *
 * s is the string containng the date.
 * date is a pointer to a struct that gets filled with data.
 *
 * Returns 0 on success and -1 on failure.
 *
 * Copied from libnmea/src/parsers/parse.c.
 */
int nmea_date_parse(char *s, struct tm *date) {
    char *rv;
    uint32_t x;

    if (s == NULL || *s == '\0') {
        return -1;
    }

    x = strtoul(s, &rv, 10);
    date->tm_mday = x / 10000;
    date->tm_mon = ((x % 10000) / 100) - 1;
    date->tm_year = x % 100;

    // Normalize tm_year according to C standard library
    if (date->tm_year > 1900) {  // ZDA message case
        date->tm_year -= TM_YEAR_START;
    } else {  // RMC message case
        date->tm_year += (RMC_YEAR_START - TM_YEAR_START);
    }

    return 0;
}

int nmea_time_parse(char *s, struct tm *time) {
    char *rv;
    uint32_t x;

    if (s == NULL || *s == '\0') {
        return -1;
    }

    x = strtoul(s, &rv, 10);
    time->tm_hour = x / 10000;
    time->tm_min = (x % 10000) / 100;
    time->tm_sec = x % 100;
    if (time->tm_hour > 23 || time->tm_min > 59 || time->tm_sec > 59 ||
        (int)(rv - s) < NMEA_TIME_FORMAT_LEN) {
        return -1;
    }
    if (*rv == '.') {
        /* TODO There is a sub-second field. */
    }

    return 0;
}