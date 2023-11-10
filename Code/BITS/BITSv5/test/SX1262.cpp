#include "SX1262.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "pico/stdlib.h"

spi_inst_t *spi = spi1;

const uint cs_pin = 25;
const uint sck_pin = 26;
const uint mosi_pin = 27;
const uint miso_pin = 24;
const uint txen_pin = 8;
const uint dio1_pin = 10;
const uint busy_pin = 11;
const uint sw_pin = 9;

const uint8_t read_reg_cmd = 0x1D;
const uint8_t get_status_cmd = 0xC0;
const uint8_t nop_cmd = 0x00;
const uint8_t addr2 = 0x06;
const uint8_t addr1 = 0xB8;
uint8_t msg = 0x00;
const uint8_t StdbyConfig = 0x01;
const uint8_t set_standby_cmd = 0x80;
const uint8_t get_err_cmd = 0x17;
const uint8_t set_packet_type_cmd = 0x8A;
const uint8_t packet_type_lora = 0x01;
const uint8_t pa_config_cmd = 0x95;
const uint8_t set_rf_freq_cmd = 0x95;
const uint8_t set_tx_params_cmd = 0x8E;
const uint8_t set_buffer_base_addr_cmd = 0x8F;
const uint8_t write_radio_buffer_cmd = 0x0E;
const uint8_t set_modulation_param_cmd = 0x8B;
const uint8_t write_radio_register_cmd = 0x0D;
const uint8_t tx_continuous_wave_cmd = 0xD1;
const uint8_t set_tx_cmd = 0x83;
const uint8_t set_dio2_rf_ctrl_cmd = 0x9D;
const uint8_t set_packet_param_cmd = 0x8C;
const uint8_t clear_radio_err_cmd = 0x07;
const uint8_t set_dio3_as_tcxo_cmd = 0x97;
const uint8_t set_regulator_mode_cmd = 0x96;
const uint8_t set_radio_rx_cmd = SX126X_CMD_SET_RX;

void radio_init() {
    printf("Initializing Radio");

    radio_spi_init();

    printf("BUSY Pin: %d\n", gpio_get(busy_pin));

    // Step 1: Enter STDBY_RC
    set_radio_standby();

    set_dio3_as_tcxo();
    set_dio2_rf_switch();
    set_regulator_mode();

    clear_radio_errors();

    get_radio_errors();

    // Step 2: Set Packet Type to LoRa
    set_radio_packet_type_lora();

    // Step 3: Set RF Frequency
    set_radio_rf_freq();

    // Step 4: Set PA Config
    set_radio_pa_config();

    // Step 5: Set TX Parameters
    set_tx_params();

    // Step 6: Set Buffer Base Address
    set_buffer_base_address();

    // Step 7: Write Buffer
    write_radio_buffer();

    // Step 8: Set Modulation Parameters
    set_radio_modulation_param();

    // Step 9: Set Packet Parameters
    set_packet_parameters();

    // Step 10: Configure DIO
    set_dio2_rf_switch();

    // Step 11: Define Sync Word
    set_radio_sync_word();
}

void get_radio_status() {
    gpio_put(CS_PIN, 0);
    spi_write_blocking(spi, &get_status_cmd, 1);
    spi_write_read_blocking(spi, &nop_cmd, &msg, 1);
    gpio_put(CS_PIN, 1);
    printf("radio status: %x\n", msg);
}

void set_radio_standby() {
    gpio_put(CS_PIN, 0);
    spi_write_blocking(spi, &set_standby_cmd, 1);
    spi_write_blocking(spi, &StdbyConfig, 1);
    spi_write_blocking(spi, &nop_cmd, 1);
    spi_write_blocking(spi, &nop_cmd, 1);
    spi_write_blocking(spi, &nop_cmd, 1);
    gpio_put(CS_PIN, 1);
}

void get_radio_errors() {
    gpio_put(CS_PIN, 0);
    spi_write_blocking(spi, &get_err_cmd, 1);
    spi_write_read_blocking(spi, &nop_cmd, &msg, 1);
    printf("status: %x\n", msg);
    spi_write_read_blocking(spi, &nop_cmd, &msg, 1);
    printf("err: %x\n", msg);
    spi_write_read_blocking(spi, &nop_cmd, &msg, 1);
    gpio_put(CS_PIN, 1);
    printf("err: %x\n", msg);
}

void read_radio_registers() {
    printf("reg: %x%x\n", addr2, addr1);
    gpio_put(CS_PIN, 0);
    spi_write_read_blocking(spi, &read_reg_cmd, &msg, 1);
    printf("data: %x\n", msg);

    spi_write_read_blocking(spi, &addr2, &msg, 1);
    printf("data: %x\n", msg);

    spi_write_read_blocking(spi, &addr1, &msg, 1);
    printf("data: %x\n", msg);

    spi_write_read_blocking(spi, &nop_cmd, &msg, 1);
    printf("data: %x\n", msg);

    for (int j = 0; j < 4; j++) {
        spi_write_read_blocking(spi, &nop_cmd, &msg, 1);
        printf("read: %x\n", msg);
    }
    gpio_put(CS_PIN, 0);
}

void radio_spi_init() {
    printf("Init radio SPI\n");

    gpio_init(CS_PIN);
    gpio_set_dir(CS_PIN, GPIO_OUT);
    gpio_put(CS_PIN, 1);

    gpio_init(sw_pin);
    gpio_set_dir(sw_pin, GPIO_OUT);
    gpio_put(sw_pin, 1);

    gpio_init(txen_pin);
    gpio_set_dir(txen_pin, GPIO_OUT);
    gpio_put(txen_pin, 0);

    gpio_init(busy_pin);
    gpio_set_dir(busy_pin, GPIO_IN);

    spi_init(spi, 500000);

    spi_set_format(spi,            // SPI instance
                   8,              // Number of bits per transfer
                   (spi_cpol_t)0,  // Polarity (CPOL)
                   (spi_cpha_t)0,  // Phase (CPHA)
                   SPI_MSB_FIRST);

    gpio_set_function(sck_pin, GPIO_FUNC_SPI);
    gpio_set_function(mosi_pin, GPIO_FUNC_SPI);
    gpio_set_function(miso_pin, GPIO_FUNC_SPI);
}

void set_radio_packet_type_lora() {
    printf("Setting Packet Type to LoRa\n");

    gpio_put(CS_PIN, 0);
    spi_write_blocking(spi, &set_packet_type_cmd, 1);
    spi_write_blocking(spi, &packet_type_lora, 1);
    gpio_put(CS_PIN, 1);
}

void set_radio_pa_config() {
    const uint8_t pa_duty = 0x04;
    const uint8_t hp_max = 0x07;
    const uint8_t device_sel = 0x00;
    const uint8_t pa_lut = 0x01;

    printf("Setting PA Config\n");

    gpio_put(CS_PIN, 0);
    spi_write_blocking(spi, &pa_config_cmd, 1);
    spi_write_blocking(spi, &pa_duty, 1);
    spi_write_blocking(spi, &hp_max, 1);
    spi_write_blocking(spi, &device_sel, 1);
    spi_write_blocking(spi, &pa_lut, 1);
    gpio_put(CS_PIN, 1);
}

void set_radio_rf_freq() {
    const uint32_t frequency = 915000000;

    uint8_t buf[4];
    uint32_t freq = 0;

    freq = (uint32_t)((double)frequency / (double)FREQ_STEP);
    buf[0] = (uint8_t)((freq >> 24) & 0xFF);
    buf[1] = (uint8_t)((freq >> 16) & 0xFF);
    buf[2] = (uint8_t)((freq >> 8) & 0xFF);
    buf[3] = (uint8_t)(freq & 0xFF);

    printf("Setting Frequency to %d\n", frequency);

    gpio_put(CS_PIN, 0);
    spi_write_blocking(spi, &set_rf_freq_cmd, 1);
    spi_write_blocking(spi, &buf[0], 1);
    spi_write_blocking(spi, &buf[1], 1);
    spi_write_blocking(spi, &buf[2], 1);
    spi_write_blocking(spi, &buf[3], 1);
    gpio_put(CS_PIN, 1);
}

void set_tx_params() {
    const uint8_t power = 0x16;
    const uint8_t ramp_time = 0x04;

    gpio_put(CS_PIN, 0);
    spi_write_blocking(spi, &set_tx_params_cmd, 1);
    spi_write_blocking(spi, &power, 1);
    spi_write_blocking(spi, &ramp_time, 1);
    gpio_put(CS_PIN, 1);
}

void set_buffer_base_address() {
    const uint8_t tx_buffer = 0x00;
    const uint8_t rx_buffer = 0x00;

    gpio_put(CS_PIN, 0);
    spi_write_blocking(spi, &set_buffer_base_addr_cmd, 1);
    spi_write_blocking(spi, &tx_buffer, 1);
    spi_write_blocking(spi, &rx_buffer, 1);
    gpio_put(CS_PIN, 1);
}

void write_radio_buffer() {
    const uint8_t offset = 0x00;
    const uint8_t data = 0x69;

    gpio_put(CS_PIN, 0);
    spi_write_blocking(spi, &set_buffer_base_addr_cmd, 1);
    spi_write_blocking(spi, &offset, 1);
    spi_write_blocking(spi, &data, 1);
    gpio_put(CS_PIN, 1);
}

void set_radio_modulation_param() {
    const uint8_t spreading_factor = 11;
    const uint8_t bandwidth = 1;
    const uint8_t coding_rate = 1;

    gpio_put(CS_PIN, 0);
    spi_write_blocking(spi, &set_modulation_param_cmd, 1);
    spi_write_blocking(spi, &spreading_factor, 1);
    spi_write_blocking(spi, &bandwidth, 1);
    spi_write_blocking(spi, &coding_rate, 1);
    gpio_put(CS_PIN, 1);
}

void set_packet_parameters() {
    const uint8_t preamble2 = 0;
    const uint8_t preamble1 = 8;
    const uint8_t header = 0;
    const uint8_t length = 1;
    const uint8_t crc = 0;
    const uint8_t iq = 0;

    printf("Setting Packet Parameters\n");

    gpio_put(CS_PIN, 0);
    spi_write_blocking(spi, &set_buffer_base_addr_cmd, 1);
    spi_write_blocking(spi, &preamble2, 1);
    spi_write_blocking(spi, &preamble1, 1);
    spi_write_blocking(spi, &header, 1);
    spi_write_blocking(spi, &length, 1);
    spi_write_blocking(spi, &crc, 1);
    spi_write_blocking(spi, &iq, 1);
    gpio_put(CS_PIN, 1);
}

void set_dio2_rf_switch() {
    const uint8_t enable = 1;

    printf("Setting DIO2 as RF Switch\n");

    gpio_put(CS_PIN, 0);
    spi_write_blocking(spi, &set_dio2_rf_ctrl_cmd, 1);
    spi_write_blocking(spi, &enable, 1);
    gpio_put(CS_PIN, 1);
}

void set_radio_sync_word() {
    const uint8_t msb2 = 0x07;
    const uint8_t msb1 = 0x40;
    const uint8_t lsb2 = 0x07;
    const uint8_t lsb1 = 0x41;
    const uint8_t data2 = 0x34;
    const uint8_t data1 = 0x44;

    printf("Setting Radio Sync Word\n");

    gpio_put(CS_PIN, 0);
    spi_write_blocking(spi, &write_radio_register_cmd, 1);
    spi_write_blocking(spi, &msb2, 1);
    spi_write_blocking(spi, &msb1, 1);
    spi_write_blocking(spi, &data2, 1);
    spi_write_blocking(spi, &lsb2, 1);
    spi_write_blocking(spi, &lsb1, 1);
    spi_write_blocking(spi, &data1, 1);
    gpio_put(CS_PIN, 1);
}

void set_tx_continuous_wave() {
    printf("Setting Mode TX Tone\n");

    gpio_put(CS_PIN, 0);
    spi_write_blocking(spi, &tx_continuous_wave_cmd, 1);
    gpio_put(CS_PIN, 1);
}

void set_tx() {
    const uint8_t timeout3 = 0x00;
    const uint8_t timeout2 = 0x7D;
    const uint8_t timeout1 = 0x00;

    printf("Setting Mode TX\n");

    gpio_put(CS_PIN, 0);
    spi_write_blocking(spi, &set_tx_cmd, 1);
    spi_write_blocking(spi, &timeout3, 1);
    spi_write_blocking(spi, &timeout2, 1);
    spi_write_blocking(spi, &timeout1, 1);
    gpio_put(CS_PIN, 1);
}

void set_dio3_as_tcxo() {
    const uint8_t tcxoVoltage = 0x07;
    const uint8_t timeout3 = 0x00;
    const uint8_t timeout2 = 0x01;
    const uint8_t timeout1 = 0x40;

    printf("Setting DIO3 as TCXO CTRL\n");

    gpio_put(CS_PIN, 0);
    spi_write_blocking(spi, &set_dio3_as_tcxo_cmd, 1);
    spi_write_blocking(spi, &tcxoVoltage, 1);
    spi_write_blocking(spi, &timeout3, 1);
    spi_write_blocking(spi, &timeout2, 1);
    spi_write_blocking(spi, &timeout1, 1);
    gpio_put(CS_PIN, 1);
}

void set_regulator_mode() {
    const uint8_t mode = 0x01;

    printf("Setting Regulator Mode to DC DC\n");

    gpio_put(CS_PIN, 0);
    spi_write_blocking(spi, &set_regulator_mode_cmd, 1);
    spi_write_blocking(spi, &mode, 1);
    gpio_put(CS_PIN, 1);
}

void clear_radio_errors() {
    printf("Clearing radio errors\n");
    gpio_put(cs_pin, 0);
    spi_write_blocking(spi, &clear_radio_err_cmd, 1);
    spi_write_read_blocking(spi, &nop_cmd, &msg, 1);
    spi_write_read_blocking(spi, &nop_cmd, &msg, 1);
    gpio_put(cs_pin, 1);
}

void radio_send() {
    write_radio_buffer();
    set_tx();
}

void radio_receive_cont() {
    uint8_t timeout3 = 0x00;
    uint8_t timeout2 = 0x00;
    uint8_t timeout1 = 0x00;

    printf("Entering Radio Receive Mode\n");
    gpio_put(cs_pin, 0);
    spi_write_blocking(spi, &set_radio_rx_cmd, 1);
    spi_write_blocking(spi, &timeout3, 1);
    spi_write_blocking(spi, &timeout2, 1);
    spi_write_blocking(spi, &timeout1, 1);
    gpio_put(cs_pin, 1);
}