/*
  ______                              _
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: SX126x driver specific target board functions implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#include "sx126x-board.h"

#include <hardware/spi.h>
#include <hardware/sync.h>

#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "sx126x.h"

static uint8_t IrqNestLevel = 0;
static uint32_t ints;

void DelayMs(uint32_t ms) { sleep_ms(ms); }

void BoardDisableIrq(void) {
    // gpio_set_irq_enabled_with_callback(
    //     2, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, false, &gpio_callback);
    IrqNestLevel++;
    ints = save_and_disable_interrupts();
}

void BoardEnableIrq(void) {
    IrqNestLevel--;
    if (IrqNestLevel == 0) {
        // gpio_set_irq_enabled_with_callback(
        //     2, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true,
        //     &gpio_callback);
        restore_interrupts(ints);
    }
}

static uint16_t SpiInOut(uint16_t outData) {
    uint16_t rxData = 0;

    spi_write16_read16_blocking(spi0, &outData, &rxData, 1);

    return (rxData);
}

/*!
 * Antenna switch GPIO pins objects
 */

void SX126xIoInit(void) {
    gpio_init(NssPin);
    gpio_init(NResetPin);
    gpio_init(SwPin);
    gpio_init(BusyPin);
    gpio_init(Dio1Pin);

    gpio_set_dir(NssPin, GPIO_OUT);
    gpio_set_dir(NResetPin, GPIO_OUT);
    gpio_set_dir(SwPin, GPIO_OUT);

    gpio_set_dir(BusyPin, GPIO_IN);
    gpio_set_dir(Dio1Pin, GPIO_IN);

    gpio_put(NssPin, true);
    gpio_put(NResetPin, true);
    gpio_put(SwPin, true);
}

void SX126xReset(void) {
    sleep_ms(10);
    gpio_put(NResetPin, false);
    sleep_ms(20);
    gpio_put(NResetPin, true);
    sleep_ms(10);
}

void SX126xWaitOnBusy(void) {
    while (gpio_get(BusyPin) == 1)
        ;
}

void SX126xWakeup(void) {
    BoardDisableIrq();

    gpio_put(NssPin, false);

    SpiInOut(RADIO_GET_STATUS);
    SpiInOut(0x00);

    gpio_put(NssPin, true);

    // Wait for chip to be ready.
    SX126xWaitOnBusy();

    BoardEnableIrq();
}
void SX126xWriteCommand(uint8_t command, uint8_t *buffer, uint16_t size) {
    //   SX126xCheckDeviceReady( );    // undetermined

    gpio_put(NssPin, false);

    SpiInOut((uint8_t)command);

    for (uint16_t i = 0; i < size; i++) {
        SpiInOut(buffer[i]);
    }

    gpio_put(NssPin, true);

    if (command != RADIO_SET_SLEEP) {
        SX126xWaitOnBusy();
    }
}

void SX126xReadCommand(uint8_t command, uint8_t *buffer, uint16_t size) {
    //   SX126xCheckDeviceReady( );    // undetermined

    gpio_put(NssPin, false);

    SpiInOut((uint8_t)command);
    SpiInOut(0x00);
    for (uint16_t i = 0; i < size; i++) {
        buffer[i] = SpiInOut(0);
    }

    gpio_put(NssPin, true);

    SX126xWaitOnBusy();
}

void SX126xWriteRegisters(uint16_t address, uint8_t *buffer, uint16_t size) {
    //   SX126xCheckDeviceReady( );    // undetermined
    gpio_put(NssPin, false);
    SpiInOut(RADIO_WRITE_REGISTER);
    SpiInOut((address & 0xFF00) >> 8);
    SpiInOut(address & 0x00FF);

    for (uint16_t i = 0; i < size; i++) {
        SpiInOut(buffer[i]);
    }

    gpio_put(NssPin, true);

    SX126xWaitOnBusy();
}

void SX126xWriteRegister(uint16_t address, uint8_t value) {
    SX126xWriteRegisters(address, &value, 1);
}

void SX126xReadRegisters(uint16_t address, uint8_t *buffer, uint16_t size) {
    //   SX126xCheckDeviceReady( );    // undetermined

    gpio_put(NssPin, false);

    SpiInOut(RADIO_READ_REGISTER);
    SpiInOut((address & 0xFF00) >> 8);
    SpiInOut(address & 0x00FF);
    SpiInOut(0);
    for (uint16_t i = 0; i < size; i++) {
        buffer[i] = SpiInOut(0);
    }

    gpio_put(NssPin, true);

    SX126xWaitOnBusy();
}
uint8_t SX126xReadRegister(uint16_t address) {
    uint8_t data;
    SX126xReadRegisters(address, &data, 1);
    return data;
}

void SX126xWriteBuffer(uint8_t offset, uint8_t *buffer, uint8_t size) {
    //   SX126xCheckDeviceReady( );    // undetermined

    gpio_put(NssPin, false);

    SpiInOut(RADIO_WRITE_BUFFER);
    SpiInOut(offset);
    for (uint16_t i = 0; i < size; i++) {
        SpiInOut(buffer[i]);
    }

    gpio_put(NssPin, true);

    SX126xWaitOnBusy();
}
void SX126xReadBuffer(uint8_t offset, uint8_t *buffer, uint8_t size) {
    //   SX126xCheckDeviceReady( );    // undetermined

    gpio_put(NssPin, false);

    SpiInOut(RADIO_READ_BUFFER);
    SpiInOut(offset);
    SpiInOut(0);
    for (uint16_t i = 0; i < size; i++) {
        buffer[i] = SpiInOut(0);
    }

    gpio_put(NssPin, true);

    SX126xWaitOnBusy();
}
void SX126xSetRfTxPower(int8_t power) {
    SX126xSetTxParams(power, RADIO_RAMP_40_US);
}
bool SX126xCheckRfFrequency(uint32_t frequency) {
    // Implement check. Currently all frequencies are supported
    return true;
}