#ifndef _BITSv5_RADIO_H
#define _BITSv5_RADIO_H

#include "pico/rand.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include "pico/unique_id.h"

int64_t ack_timer_callback(alarm_id_t id, void *user_data);
bool tx_timer_callback(repeating_timer_t *rt);
int start_tx_repeating(int interval_ms);

extern char radio_rx_buf[100];
extern uint8_t radio_tx_buf[100];
extern uint8_t radio_ack_buf[100];
extern char ack[100];

extern volatile bool tx_done;
extern volatile bool transmit;
extern volatile bool rx_done;
extern volatile bool send_ack;

extern repeating_timer_t tx_timer;
extern alarm_pool_t *ack_alarm_pool;
extern alarm_id_t ack_alarm_id;

void radio_setup();

#endif