#ifndef IRIDIUM_LIB_H
#define IRIDIUM_LIB_H

#define IR_TX_PIN 12
#define IR_RX_PIN 1
#define IR_CTS_PIN 2
#define IR_RTS_PIN 3
#define IR_NETAV_PIN 4
#define IR_RING_PIN 5

#include "hardware/uart.h"
#include "pico/stdlib.h"

// AT+SBDIX - Initiate an Short Burst Data session, i.e. talk to the satellites.
// Make sure you have loaded message data first. AT+SBDWT - Write text message
// into outbound buffer AT+SBDWB - Write binary data into outbound buffer
// AT+SBDRT - Read text message from inbound buffer
// AT+SBDRB - Read binary data in from inbound buffer
// AT+SBDSX - Status

// Default UART settings:
// Baud Rate = 19200
// Data Bits = 8
// Parity = N
// Stop Bits = 1
class IridiumSBD {
public:
  uint rx_pin;
  uint cts_pin;
  uint rts_pin;
  uint netav_pin;
  uint ring_pin;
  uint tx_pin;
  uart_inst_t *uart;

  struct sbd_status {
    bool out_stat;
    int next_seq_num;
    bool in_stat;
    int last_seq_num;
    bool ring_alert;
    int msg_waiting;
  };

  IridiumSBD(uart_inst_t *p, const uint rx, const uint cts, const uint rts,
             const uint netav, const uint ring, const uint tx) {
    rx_pin = rx;
    cts_pin = cts;
    rts_pin = rts;
    netav_pin = netav;
    ring_pin = ring;
    tx_pin = tx;
    uart = p;

    uart_init(uart0, 19200);

    // Set the GPIO pin mux to the UART - 0 is TX, 1 is RX
    gpio_set_function(IR_RX_PIN, GPIO_FUNC_UART);
    gpio_set_function(IR_TX_PIN, GPIO_FUNC_UART);

    uart_set_format(uart0, 8, 1, UART_PARITY_NONE);
  }

  void start_session(void);
  void write_SBD_text(char *data, uint len);
  void write_SBD_binary(void);
  void read_SBD_text(void);
  void read_SBD_binary(void);
  void get_SBD_status(void);
  void get_info(void);
};

#endif
