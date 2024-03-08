#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../../libraries/rp2040-config/MB85RS1MT.h"
#include "../../../libraries/rp2040-config/config.h"
#include "../BITSv5.h"
#include "hardware/gpio.h"
#include "pico/binary_info.h"
#include "pico/rand.h"
#include "pico/stdlib.h"

MB85RS1MT mem(spi1, FRAM_CS, SCK_PIN, MOSI_PIN, MISO_PIN);

uint32_t log_addr = LOG_INIT_ADDR;
char log_str[] = "BITSv5,PRESS,TEMP,LAT,LONG,TIME\n";  // "a b c d ef"
char log_str1[] = "0,0,0,99.9999,99.9999,0000.00\n";   //"h i j k lm"
uint8_t log_buf = 0;

Config test_config;

char c = 0;

void setup_led();
void led_on();
void led_off();
void setup_spi();
void write_name_config();
void dump_fram();
void write_fram();

int main() {
    stdio_init_all();

    setup_spi();

    setup_led();
    led_on();

    mem.mem_init();

    sleep_ms(5000);

    printf("BITSv5 Test (Compiled %s %s)\n", __DATE__, __TIME__);
    printf("Device ID: %d\n", mem.device_id);
    read_config(NAME, test_config, (uint8_t *)test_config.name,
                sizeof(test_config.name), &mem);
    printf("DEVICE NAME: %s\n", test_config.name);

    while (true) {
        c = getchar_timeout_us(0);
        printf("Press \"d\" to dump memory, \"w\" to write memory\n");
        if (c == 'd') {
            sleep_ms(1000);
            dump_fram();
        } else if (c == 'w') {
            sleep_ms(1000);
            write_fram();
        }
    }
}

void setup_led() {
    gpio_init(LED_PIN);
    gpio_set_dir(0, GPIO_OUT);
    gpio_put(LED_PIN, 0);
}

void led_on() { gpio_put(LED_PIN, true); }

void led_off() { gpio_put(LED_PIN, false); }

void setup_spi() {
    spi_init(spi1, 500000);
    spi_set_format(spi1, 8, (spi_cpol_t)0, (spi_cpha_t)0, SPI_MSB_FIRST);
}

void write_name_config() {
    strcpy(test_config.name, "BITSv5.2-0");
    write_config(NAME, test_config, (uint8_t *)test_config.name,
                 sizeof(test_config.name), &mem);
}

void dump_fram() {
    printf("Dumping FRAM\n");
    for (log_addr = LOG_INIT_ADDR; log_addr <= LOG_MAX_ADDR; log_addr++) {
        mem.read_memory(log_addr, &log_buf, 1);  // sizeof(log_buf)
        if (log_buf != 0) printf("%c", log_buf);
        // printf("%d - %c\n", log_addr, log_buf);
        // memset(log_buf, 0, sizeof(log_buf));
        log_buf = 0;
    }
}

void write_fram() {
    log_addr = LOG_INIT_ADDR;
    printf("\n\nWriting FRAM\n");
    mem.write_memory(log_addr, (uint8_t *)log_str, sizeof(log_str));
    printf("%d - %s\n", log_addr, log_str);
    for (log_addr = LOG_INIT_ADDR + sizeof(log_str);
         log_addr <= LOG_INIT_ADDR + 1 + 20 * (sizeof(log_str1));
         log_addr = log_addr + sizeof(log_str1)) {
        // printf("%d\n", log_addr);

        // log_str[1] = (char)get_rand_32();
        log_str1[0] = (char)get_rand_32();

        mem.write_memory(log_addr, (uint8_t *)log_str1, sizeof(log_str1));
        printf("%d - %s\n", log_addr, log_str1);
    }
}