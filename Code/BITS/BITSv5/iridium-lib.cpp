#include "iridium-lib.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hardware/uart.h"

#define SBDWT_CMD "AT+SBDWT="
#define CSQ_CMD "AT+CSQ\r"
#define SBDIX_CMD "AT+SBDIX\r"
#define SBDRB_CMD "AT+SBDRB\r"
#define SBDRT_CMD "AT+SBDRT\r"
#define CGMR_CMD "AT+CGMR\r"
#define CGMM_CMD "AT+CGMM\r"
#define SBDWB_CMD "AT+SBDWB="

char sbd_rx_buf[120] = {0};
char sbd_tx_buf[120] = {0};

void IridiumSBD::get_info() {
    printf("Writing CGMM\n");

    uart_write_blocking(uart, (uint8_t *)CGMM_CMD, strlen(CGMM_CMD));

    read_uart_until_return();
    printf("%s\n", sbd_rx_buf);
    read_uart_until_return();
    printf("%s\n", sbd_rx_buf);
    read_uart_until_return();
    printf("%s\n", sbd_rx_buf);
    read_uart_until_return();
    printf("%s\n", sbd_rx_buf);
}

void IridiumSBD::start_session() {
    int mo = 0;
    int momsn = 0;
    int mt = 0;
    int mtmsn = 0;
    int mt_len = 0;
    int mt_queued = 0;
    char trash[10] = {0};

    printf("Writing SBDIX\n");

    uart_write_blocking(uart, (uint8_t *)SBDIX_CMD, strlen(SBDIX_CMD));

    read_uart_until_return();
    printf("%s\n", sbd_rx_buf);
    read_uart_until_return();
    printf("%s\n", sbd_rx_buf);

    sscanf(sbd_rx_buf, "%s %d, %d, %d, %d, %d, %d", trash, mo, momsn, mt, mtmsn,
           mt_len, mt_queued);

    printf("Parsed values: %d, %d, %d, %d, %d, %d\n", mo, momsn, mt, mtmsn,
           mt_len, mt_queued);

    read_uart_until_return();
    printf("%s\n", sbd_rx_buf);

    read_uart_until_return();
    printf("%s\n", sbd_rx_buf);
}

void IridiumSBD::write_SBD_text(char *data, uint len) {
    strcpy(sbd_tx_buf, SBDWT_CMD);
    strcat(sbd_tx_buf, data);
    strcat(sbd_tx_buf, "\r");

    uart_write_blocking(uart, (uint8_t *)sbd_tx_buf, strlen(sbd_tx_buf));

    read_uart_until_return();

    printf("%s\n", sbd_rx_buf);
}

void IridiumSBD::get_SBD_status() {}

void IridiumSBD::read_SBD_text() {
    uart_write_blocking(uart, (uint8_t *)SBDRT_CMD, strlen(SBDRT_CMD));

    read_uart_until_return();

    printf("%s\n", sbd_rx_buf);
}

void IridiumSBD::read_uart_until_return() {
    char c = 0;
    uint pos = 0;

    c = uart_getc(uart);
    // printf("%c", c);
    while (c != '\n') {  // c != '\n' && c != '\r' //c != '\n'
        if (pos >= 119) {
            printf("SBD read buf full\n");
            pos = 119;
            break;
        }
        sbd_rx_buf[pos] = c;
        pos++;
        c = uart_getc(uart);
        // printf("%c", c);
    }
    sbd_rx_buf[pos] = '\0';
}