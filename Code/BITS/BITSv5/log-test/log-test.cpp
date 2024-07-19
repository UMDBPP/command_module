#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../../libraries/rp2040-config/MB85RS1MT.h"
#include "../../../libraries/rp2040-config/config.h"
#include "../BITSv5.h"
#include "../BITSv5_GPS.h"
#include "../BITSv5_Radio.h"
#include "hardware/flash.h"
#include "hardware/gpio.h"
#include "hardware/watchdog.h"
#include "pico/binary_info.h"
#include "pico/rand.h"
#include "pico/stdlib.h"
#include "pico/unique_id.h"

// logging
MB85RS1MT mem(spi1, FRAM_CS, SCK_PIN, MOSI_PIN, MISO_PIN);

static uint32_t log_addr = LOG_INIT_ADDR;
char log_str[] = "BITSv5,PRESS,TEMP,LAT,LONG,TIME\n";  // "a b c d ef"
char log_str1[] = "0,0,0,99.9999,99.9999,0000.00\n";   //"h i j k lm"
uint8_t log_buf = 0;

Config test_config;

void dump_mem(void);
void write_random_log(void);
void read_name(void);
void write_name_config(void);

int main() {
    setup();

    sleep_ms(5000);

    // setup devices
    mem.mem_init();

    // Enable the watchdog, requiring the watchdog to be updated every 2000ms or
    // the chip will reboot
    watchdog_enable(2000, 1);

    if (watchdog_caused_reboot()) {
        printf("Rebooted by Watchdog!\n");
    } else {
        printf("Clean boot\n");
    }

    printf("BITSv5 Test (Compiled %s %s)\n", __DATE__, __TIME__);
    printf("Device ID: %d\n", mem.device_id);
    read_config(NAME, test_config, (uint8_t *)test_config.name,
                sizeof(test_config.name), &mem);
    printf("DEVICE NAME: %s\n", test_config.name);

    dump_mem();
    write_random_log();

    read_name();

    while (true) {
        printf("Done\n");
        sleep_ms(1000);
    }
}

void dump_mem() {
    printf("Dumping FRAM\n");
    for (log_addr = LOG_INIT_ADDR;
         log_addr <= LOG_INIT_ADDR + 1 + 20 * (sizeof(log_str1)); log_addr++) {
        watchdog_update();
        mem.read_memory(log_addr, &log_buf, 1);
        if (log_buf != 0) printf("%c", log_buf);

        log_buf = 0;
    }
}

void write_random_log() {
    log_addr = LOG_INIT_ADDR;
    printf("\n\nWriting FRAM\n");
    mem.write_memory(log_addr, (uint8_t *)log_str, sizeof(log_str));
    printf("%d - %s\n", log_addr, log_str);
    for (log_addr = LOG_INIT_ADDR + sizeof(log_str);
         log_addr <= LOG_INIT_ADDR + 1 + 20 * (sizeof(log_str1));
         log_addr = log_addr + sizeof(log_str1)) {
        watchdog_update();

        log_str1[0] = (char)get_rand_32();

        mem.write_memory(log_addr, (uint8_t *)log_str1, sizeof(log_str1));
        printf("%d - %s\n", log_addr, log_str1);
    }
}

void read_name() {
    read_config(NAME, test_config, (uint8_t *)test_config.name,
                sizeof(test_config.name), &mem);
    printf("DEVICE NAME: %s\n", test_config.name);
}

void write_name_config() {
    strcpy(test_config.name, "BITSv5.2-0");
    write_config(NAME, test_config, (uint8_t *)test_config.name,
                 sizeof(test_config.name), &mem);
}