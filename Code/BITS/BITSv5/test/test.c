#include <stdio.h>
#include "pico/stdlib.h"
#include <string.h>
#include "pico/binary_info.h"
#include "hardware/gpio.h"

int main() {
    stdio_init_all();

    gpio_init(0);
    gpio_set_dir(0, GPIO_OUT);

    while (true) {
        printf("Hello, BITS!\n");
        gpio_put(0, true);
        sleep_ms(1000);
        gpio_put(0, false);
        sleep_ms(1000);
    }
}