#include <stdio.h>
#include <string.h>

#include "../BITS_common/BITSv5.h"
#include "../BITS_common/BITSv5_GPS.h"
#include "../BITS_common/BITSv5_Radio.h"
#include "../BITS_common/iridium-lib.h"

#include "hardware/uart.h"
#include "pico/binary_info.h"
#include "pico/stdlib.h"

IridiumSBD modem(uart0, IR_RX_PIN, IR_CTS_PIN, IR_RTS_PIN, IR_NETAV_PIN,
                 IR_RING_PIN, IR_TX_PIN);

int main() {

  setup(true);

  sleep_ms(5000);

  while (true) {
    // modem.get_info();
    modem.start_session();

    printf("starting sleep\n");
    sleep_ms(30000);
  }
}
