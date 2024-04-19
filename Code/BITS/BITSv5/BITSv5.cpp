#include "BITSv5.h"

#include <stdio.h>

#include "BITSv5_GPS.h"
#include "BITSv5_Radio.h"
#include "hardware/spi.h"
#include "pico/stdlib.h"

char id[2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1];

int setup() {
    stdio_init_all();

    gpio_init(LED_PIN);
    gpio_set_dir(0, GPIO_OUT);
    gpio_put(LED_PIN, 0);

    gpio_set_irq_enabled_with_callback(DIO1_PIN, GPIO_IRQ_EDGE_RISE, true,
                                       &gpio_callback);

    gpio_init(EXTINT_PIN);
    gpio_set_dir(EXTINT_PIN, GPIO_IN);
    gpio_init(TIMEPULSE_PIN);
    gpio_set_dir(TIMEPULSE_PIN, GPIO_IN);

    gpio_init(RADIO_RST);
    gpio_set_dir(RADIO_RST, GPIO_OUT);
    gpio_put(RADIO_RST, 1);

    // Initialize I2C port at 100 kHz
    i2c_init(i2c1, 100 * 1000);

    spi_init(spi1, 500000);
    spi_set_format(spi1,           // SPI instance
                   8,              // Number of bits per transfer
                   (spi_cpol_t)0,  // Polarity (CPOL)
                   (spi_cpha_t)0,  // Phase (CPHA)
                   SPI_MSB_FIRST);

    pico_get_unique_board_id_string(id,
                                    2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1);

    radio_setup();

    // negative timeout means exact delay (rather than delay between
    // callbacks)
    if (!add_repeating_timer_us(-30000000, tx_timer_callback, NULL,
                                &tx_timer)) {
        printf("Failed to add timer\n");
        return 1;
    }

    printf("\n%s %s\n", __DATE__, __TIME__);
    return 0;
}

void led_on() { gpio_put(LED_PIN, true); }

void led_off() { gpio_put(LED_PIN, false); }