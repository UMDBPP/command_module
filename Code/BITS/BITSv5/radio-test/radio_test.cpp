#include <stdio.h>
#include "pico/stdlib.h"
#include <string.h>
#include "pico/binary_info.h"
#include "hardware/spi.h"
#include "radio-lib/RadioLib.h"
#include "radio-lib/PiHal.h"



SX1262 radio = new Module(hal, 32, 5, RADIOLIB_NC, 8);

int main()
{
    stdio_init_all();

    // initialize just like with Arduino
    printf("[SX1261] Initializing ... ");
    int state = radio.begin();
    if (state != RADIOLIB_ERR_NONE)
    {
        printf("failed, code %d\n", state);
        return (1);
    }
    printf("success!\n");

    // loop forever
    for (;;)
    {
        // send a packet
        printf("[SX1261] Transmitting packet ... ");
        state = radio.transmit("Hello World!");
        if (state == RADIOLIB_ERR_NONE)
        {
            // the packet was successfully transmitted
            printf("success!\n");

            // wait for a second before transmitting again
            hal->delay(1000);
        }
        else
        {
            printf("failed, code %d\n", state);
        }
    }

    return 0;

    // while (true) {
    //     printf("Hello, BITS!\n");
    //     sleep_ms(1000);
    // }
}