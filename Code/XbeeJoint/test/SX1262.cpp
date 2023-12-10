#include "SX1262.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "pico/stdlib.h"

// spi_inst_t *spi = spi0;

const uint8_t read_reg_cmd = SX126X_CMD_READ_REGISTER;
const uint8_t get_status_cmd = SX126X_CMD_GET_STATUS;
const uint8_t nop_cmd = 0x00;
const uint8_t addr2 = 0x07;
const uint8_t addr1 = 0x40;
uint8_t msg = 0x00;
const uint8_t StdbyConfig = 0x01;
const uint8_t set_standby_cmd = SX126X_CMD_SET_STANDBY;
const uint8_t get_err_cmd = SX126X_CMD_GET_DEVICE_ERRORS;
const uint8_t set_packet_type_cmd = SX126X_CMD_SET_PACKET_TYPE;
const uint8_t packet_type_lora = 0x01;
const uint8_t packet_type_fsk = 0x00;
const uint8_t pa_config_cmd = SX126X_CMD_SET_PA_CONFIG;
const uint8_t set_rf_freq_cmd = SX126X_CMD_SET_RF_FREQUENCY;
const uint8_t set_tx_params_cmd = SX126X_CMD_SET_TX_PARAMS;
const uint8_t set_buffer_base_addr_cmd = SX126X_CMD_SET_BUFFER_BASE_ADDRESS;
const uint8_t write_radio_buffer_cmd = SX126X_CMD_WRITE_BUFFER;
const uint8_t set_modulation_param_cmd = SX126X_CMD_SET_MODULATION_PARAMS;
const uint8_t write_radio_register_cmd = SX126X_CMD_WRITE_REGISTER;
const uint8_t tx_continuous_wave_cmd = SX126X_CMD_SET_TX_CONTINUOUS_WAVE;
const uint8_t set_tx_cmd = SX126X_CMD_SET_TX;
const uint8_t set_dio2_rf_ctrl_cmd = SX126X_CMD_SET_DIO2_AS_RF_SWITCH_CTRL;
const uint8_t set_packet_param_cmd = SX126X_CMD_SET_PACKET_PARAMS;
const uint8_t clear_radio_err_cmd = SX126X_CMD_CLEAR_DEVICE_ERRORS;
const uint8_t set_dio3_as_tcxo_cmd = SX126X_CMD_SET_DIO3_AS_TCXO_CTRL;
const uint8_t set_regulator_mode_cmd = SX126X_CMD_SET_REGULATOR_MODE;
const uint8_t set_radio_rx_cmd = SX126X_CMD_SET_RX;
const uint8_t set_radio_dio_irq_cmd = SX126X_CMD_SET_DIO_IRQ_PARAMS;
const uint8_t set_radio_clear_irq_cmd = SX126X_CMD_CLEAR_IRQ_STATUS;
const uint8_t read_buffer_cmd = SX126X_CMD_READ_BUFFER;
const uint8_t get_irq_status_cmd = SX126X_CMD_GET_IRQ_STATUS;
const uint8_t get_rx_buffer_cmd = SX126X_CMD_GET_RX_BUFFER_STATUS;
const uint8_t set_lora_symb_timeout_cmd = SX126X_CMD_SET_LORA_SYMB_NUM_TIMEOUT;
const uint8_t calibrate_image_cmd = SX126X_CMD_CALIBRATE_IMAGE;

// extern short xbee_joint_debug_msgs;  // controls if debug messages are
// printed

void DRF1262::radio_init() {
    printf("Initializing Radio\n");

    radio_spi_init();

    // Step 1: Enter STDBY_RC
    set_radio_standby();

    set_dio3_as_tcxo();
    set_dio2_rf_switch();
    set_dio_irq();
    set_regulator_mode();

    clear_radio_errors();

    get_radio_errors();

    // Step 2: Set Packet Type
    if (PACKET_LORA) {
        set_radio_packet_type_lora();
    } else {
        set_radio_packet_type_fsk();
    }

    // Step 3: Set RF Frequency
    set_radio_rf_freq();

    // Step 4: Set PA Config
    set_radio_pa_config();

    // Step 5: Set TX Parameters
    set_tx_params();

    // Step 6: Set Buffer Base Address
    set_buffer_base_address();

    // Step 8: Set Modulation Parameters
    set_radio_lora_modulation_param();

    // Step 9: Set Packet Parameters
    set_lora_packet_parameters();

    // Step 10: Configure DIO
    // set_dio2_rf_switch();

    // Step 11: Define Sync Word
    set_radio_sync_word();
    set_lora_symb_timeout();

    // TODO calibrate image

    read_radio_registers();
}

void DRF1262::get_radio_status() {
    gpio_put(cs_pin, 0);
    spi_write_blocking(spi, &get_status_cmd, 1);
    spi_write_read_blocking(spi, &nop_cmd, &msg, 1);
    gpio_put(cs_pin, 1);
    printf("radio status: %x\n", msg);
}

void DRF1262::set_radio_standby() {
    gpio_put(cs_pin, 0);
    spi_write_blocking(spi, &set_standby_cmd, 1);
    spi_write_blocking(spi, &StdbyConfig, 1);
    spi_write_blocking(spi, &nop_cmd, 1);
    spi_write_blocking(spi, &nop_cmd, 1);
    spi_write_blocking(spi, &nop_cmd, 1);
    gpio_put(cs_pin, 1);
}

void DRF1262::get_radio_errors() {
    gpio_put(cs_pin, 0);
    spi_write_blocking(spi, &get_err_cmd, 1);
    spi_write_read_blocking(spi, &nop_cmd, &msg, 1);
    printf("status: %x\n", msg);
    spi_write_read_blocking(spi, &nop_cmd, &msg, 1);
    printf("err: %x\n", msg);
    spi_write_read_blocking(spi, &nop_cmd, &msg, 1);
    gpio_put(cs_pin, 1);
    printf("err: %x\n", msg);
}

void DRF1262::read_radio_registers() {
    printf("reg: %x%x\n", addr2, addr1);

    gpio_put(cs_pin, 0);
    spi_write_read_blocking(spi, &read_reg_cmd, &msg, 1);

    spi_write_read_blocking(spi, &addr2, &msg, 1);

    spi_write_read_blocking(spi, &addr1, &msg, 1);

    spi_write_read_blocking(spi, &nop_cmd, &msg, 1);
    printf("status: %x\n", msg);

    for (int j = 0; j <= 1; j++) {
        spi_write_read_blocking(spi, &nop_cmd, &msg, 1);
        printf("read: %x\n", msg);
    }
    gpio_put(cs_pin, 1);
}

void DRF1262::radio_spi_init() {
    printf("Init radio SPI\n");

    gpio_init(cs_pin);
    gpio_set_dir(cs_pin, GPIO_OUT);
    gpio_put(cs_pin, 1);

    gpio_init(sw_pin);
    gpio_set_dir(sw_pin, GPIO_OUT);
    gpio_put(sw_pin, 1);

    gpio_init(txen_pin);
    gpio_set_dir(txen_pin, GPIO_OUT);
    gpio_put(txen_pin, 0);

    gpio_init(busy_pin);
    gpio_set_dir(busy_pin, GPIO_IN);

    gpio_init(dio1_pin);
    gpio_set_dir(dio1_pin, GPIO_IN);

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

void DRF1262::set_radio_packet_type_lora() {
#if INCLUDE_DEBUG
    if (xbee_joint_debug_msgs) {
        printf("Setting Packet Type to LoRa\n");
    }
#endif

    gpio_put(cs_pin, 0);
    spi_write_blocking(spi, &set_packet_type_cmd, 1);
    spi_write_blocking(spi, &packet_type_lora, 1);
    gpio_put(cs_pin, 1);
}

void DRF1262::set_radio_pa_config() {
    const uint8_t pa_duty = 0x04;
    const uint8_t hp_max = 0x07;
    const uint8_t device_sel = 0x00;
    const uint8_t pa_lut = 0x01;

#if INCLUDE_DEBUG
    if (xbee_joint_debug_msgs) {
        printf("Setting PA Config\n");
    }
#endif

    gpio_put(cs_pin, 0);
    spi_write_blocking(spi, &pa_config_cmd, 1);
    spi_write_blocking(spi, &pa_duty, 1);
    spi_write_blocking(spi, &hp_max, 1);
    spi_write_blocking(spi, &device_sel, 1);
    spi_write_blocking(spi, &pa_lut, 1);
    gpio_put(cs_pin, 1);
}

void DRF1262::set_radio_rf_freq() {
    const uint32_t frequency = 915000000;

    uint8_t buf[4];
    uint32_t freq = 0;

    freq = (uint32_t)((double)frequency / (double)FREQ_STEP);
    buf[0] = (uint8_t)((freq >> 24) & 0xFF);
    buf[1] = (uint8_t)((freq >> 16) & 0xFF);
    buf[2] = (uint8_t)((freq >> 8) & 0xFF);
    buf[3] = (uint8_t)(freq & 0xFF);

    printf("Setting Frequency to %d\n", frequency);

    gpio_put(cs_pin, 0);
    spi_write_blocking(spi, &set_rf_freq_cmd, 1);
    spi_write_blocking(spi, &buf[0], 1);
    spi_write_blocking(spi, &buf[1], 1);
    spi_write_blocking(spi, &buf[2], 1);
    spi_write_blocking(spi, &buf[3], 1);
    gpio_put(cs_pin, 1);
}

void DRF1262::set_tx_params() {
    const uint8_t power = 0x00;
    const uint8_t ramp_time = SX126X_PA_RAMP_200U;  // 200us ramp time

    gpio_put(cs_pin, 0);
    spi_write_blocking(spi, &set_tx_params_cmd, 1);
    spi_write_blocking(spi, &power, 1);
    spi_write_blocking(spi, &ramp_time, 1);
    gpio_put(cs_pin, 1);
}

void DRF1262::set_buffer_base_address() {
    gpio_put(cs_pin, 0);
    spi_write_blocking(spi, &set_buffer_base_addr_cmd, 1);
    spi_write_blocking(spi, &tx_buffer, 1);
    spi_write_blocking(spi, &rx_buffer, 1);
    gpio_put(cs_pin, 1);
}

short DRF1262::write_radio_buffer(uint8_t offset, uint8_t *data,
                                  short num_bytes) {
    if (num_bytes > 255) return -1;

    gpio_put(cs_pin, 0);
    spi_write_blocking(spi, &write_radio_buffer_cmd, 1);
    spi_write_blocking(spi, &offset, 1);
    spi_write_blocking(spi, data, num_bytes);
    gpio_put(cs_pin, 1);

    return 0;
}

void DRF1262::set_radio_lora_modulation_param() {
    const uint8_t spreading_factor = 0x07;
    const uint8_t bandwidth = 0x04;
    const uint8_t coding_rate = 0x04;
    const uint8_t low_data_rate = 0x00;

    gpio_put(cs_pin, 0);
    spi_write_blocking(spi, &set_modulation_param_cmd, 1);
    spi_write_blocking(spi, &spreading_factor, 1);
    spi_write_blocking(spi, &bandwidth, 1);
    spi_write_blocking(spi, &coding_rate, 1);
    spi_write_blocking(spi, &low_data_rate, 1);
    gpio_put(cs_pin, 1);
}

void DRF1262::set_lora_packet_parameters() {
    const uint8_t preamble2 = 0x00;
    const uint8_t preamble1 = 0x0F;
    const uint8_t header = 0x00;
    const uint8_t length = 0x05;
    const uint8_t crc = 0x01;
    const uint8_t iq = 0x00;

    printf("Setting LoRa Packet Parameters\n");

    gpio_put(cs_pin, 0);
    spi_write_blocking(spi, &set_packet_param_cmd, 1);
    spi_write_blocking(spi, &preamble2, 1);
    spi_write_blocking(spi, &preamble1, 1);
    spi_write_blocking(spi, &header, 1);
    spi_write_blocking(spi, &length, 1);
    spi_write_blocking(spi, &crc, 1);
    spi_write_blocking(spi, &iq, 1);
    gpio_put(cs_pin, 1);
}

void DRF1262::set_fsk_packet_parameters() {
    const uint8_t preamble2 = 0x00;
    const uint8_t preamble1 = 0x0F;
    const uint8_t preamble_det_len = 0x00;
    const uint8_t sync_len = 0x08;
    const uint8_t addr_comp = 0x00;
    const uint8_t packet_type = 0x01;  // variable packet size
    const uint8_t payload_len = 0x01;
    const uint8_t crc = 0x01;        // CRC off
    const uint8_t whitening = 0x00;  // no encoding

    printf("Setting FSK Packet Parameters\n");

    gpio_put(cs_pin, 0);
    spi_write_blocking(spi, &set_packet_param_cmd, 1);
    spi_write_blocking(spi, &preamble2, 1);
    spi_write_blocking(spi, &preamble1, 1);
    spi_write_blocking(spi, &preamble_det_len, 1);
    spi_write_blocking(spi, &sync_len, 1);
    spi_write_blocking(spi, &addr_comp, 1);
    spi_write_blocking(spi, &packet_type, 1);
    spi_write_blocking(spi, &payload_len, 1);
    spi_write_blocking(spi, &crc, 1);
    spi_write_blocking(spi, &whitening, 1);
    gpio_put(cs_pin, 1);
}

void DRF1262::set_dio2_rf_switch() {
    const uint8_t enable = 1;

#if INCLUDE_DEBUG
    if (xbee_joint_debug_msgs) {
        printf("Setting DIO2 as RF Switch\n");
    }
#endif

    gpio_put(cs_pin, 0);
    spi_write_blocking(spi, &set_dio2_rf_ctrl_cmd, 1);
    spi_write_blocking(spi, &enable, 1);
    gpio_put(cs_pin, 1);
}

void DRF1262::set_radio_sync_word() {
    const uint8_t msb2 = 0x07;
    const uint8_t msb1 = 0x40;
    const uint8_t lsb2 = 0x07;
    const uint8_t lsb1 = 0x41;
    const uint8_t data2 = 0x34;
    const uint8_t data1 = 0x44;

#if INCLUDE_DEBUG
    if (xbee_joint_debug_msgs) {
        printf("Setting Radio Sync Word\n");
    }
#endif

    gpio_put(cs_pin, 0);
    spi_write_blocking(spi, &write_radio_register_cmd, 1);
    spi_write_blocking(spi, &msb2, 1);
    spi_write_blocking(spi, &msb1, 1);
    spi_write_blocking(spi, &data2, 1);
    gpio_put(cs_pin, 1);

    sleep_ms(10);

    gpio_put(cs_pin, 0);
    spi_write_blocking(spi, &write_radio_register_cmd, 1);
    spi_write_blocking(spi, &lsb2, 1);
    spi_write_blocking(spi, &lsb1, 1);
    spi_write_blocking(spi, &data1, 1);
    gpio_put(cs_pin, 1);
}

void DRF1262::set_tx_continuous_wave() {
#if INCLUDE_DEBUG
    if (xbee_joint_debug_msgs) {
        printf("Setting Mode TX Tone\n");
    }
#endif

    gpio_put(cs_pin, 0);
    spi_write_blocking(spi, &tx_continuous_wave_cmd, 1);
    gpio_put(cs_pin, 1);
}

void DRF1262::set_tx() {
    const uint8_t timeout3 = 0x00;
    const uint8_t timeout2 = 0x00;
    const uint8_t timeout1 = 0x00;

#if INCLUDE_DEBUG
    if (xbee_joint_debug_msgs) {
        printf("Setting Mode TX\n");
    }
#endif

    gpio_put(cs_pin, 0);
    spi_write_blocking(spi, &set_tx_cmd, 1);
    spi_write_blocking(spi, &timeout3, 1);
    spi_write_blocking(spi, &timeout2, 1);
    spi_write_blocking(spi, &timeout1, 1);
    gpio_put(cs_pin, 1);
}

void DRF1262::set_dio3_as_tcxo() {
    const uint8_t tcxoVoltage = 0x07;
    const uint8_t timeout3 = 0x01;
    const uint8_t timeout2 = 0x02;
    const uint8_t timeout1 = 0x80;

#if INCLUDE_DEBUG
    if (xbee_joint_debug_msgs) {
        printf("Setting DIO3 as TCXO CTRL\n");
    }
#endif

    gpio_put(cs_pin, 0);
    spi_write_blocking(spi, &set_dio3_as_tcxo_cmd, 1);
    spi_write_blocking(spi, &tcxoVoltage, 1);
    spi_write_blocking(spi, &timeout3, 1);
    spi_write_blocking(spi, &timeout2, 1);
    spi_write_blocking(spi, &timeout1, 1);
    gpio_put(cs_pin, 1);

    // don't delete this unless you want to play with the timeout value again
    sleep_ms(10);
}

void DRF1262::set_regulator_mode() {
    const uint8_t mode = 0x01;
#if INCLUDE_DEBUG
    if (xbee_joint_debug_msgs) {
        printf("Setting Regulator Mode to DC DC\n");
    }
#endif

    gpio_put(cs_pin, 0);
    spi_write_blocking(spi, &set_regulator_mode_cmd, 1);
    spi_write_blocking(spi, &mode, 1);
    gpio_put(cs_pin, 1);
}

void DRF1262::clear_radio_errors() {
#if INCLUDE_DEBUG
    if (xbee_joint_debug_msgs) {
        printf("Clearing radio errors\n");
    }
#endif

    gpio_put(cs_pin, 0);
    spi_write_blocking(spi, &clear_radio_err_cmd, 1);
    spi_write_read_blocking(spi, &nop_cmd, &msg, 1);
    spi_write_read_blocking(spi, &nop_cmd, &msg, 1);
    gpio_put(cs_pin, 1);
}

void DRF1262::radio_send(uint8_t *data, short len) {
    uint8_t payload = write_radio_buffer(tx_buffer, data, len);
    set_tx();
}

void DRF1262::radio_receive_cont() {
    uint8_t timeout3 = 0xFF;
    uint8_t timeout2 = 0xFF;
    uint8_t timeout1 = 0xFF;

#if INCLUDE_DEBUG
    if (xbee_joint_debug_msgs) {
        printf("Entering Radio Receive Mode (Continuous)\n");
    }
#endif

    gpio_put(cs_pin, 0);
    spi_write_blocking(spi, &set_radio_rx_cmd, 1);
    spi_write_blocking(spi, &timeout3, 1);
    spi_write_blocking(spi, &timeout2, 1);
    spi_write_blocking(spi, &timeout1, 1);
    gpio_put(cs_pin, 1);
}

void DRF1262::radio_receive_single() {
    uint8_t timeout3 = 0x00;
    uint8_t timeout2 = 0x00;
    uint8_t timeout1 = 0x00;

#if INCLUDE_DEBUG
    if (xbee_joint_debug_msgs) {
        printf("Entering Radio Receive Mode (Single)\n");
    }
#endif

    gpio_put(cs_pin, 0);
    spi_write_blocking(spi, &set_radio_rx_cmd, 1);
    spi_write_blocking(spi, &timeout3, 1);
    spi_write_blocking(spi, &timeout2, 1);
    spi_write_blocking(spi, &timeout1, 1);
    gpio_put(cs_pin, 1);
}

void DRF1262::set_dio_irq() {
    uint8_t irq_mask2 = 0xFF;
    uint8_t irq_mask1 = 0xFF;

    uint8_t dio1_mask2 = 0x00;
    uint8_t dio1_mask1 = 0x16;
    uint8_t dio2_mask2 = 0x00;
    uint8_t dio2_mask1 = 0x00;
    uint8_t dio3_mask2 = 0x00;
    uint8_t dio3_mask1 = 0x00;

    printf("Setting DIO1 IRQ\n");
    gpio_put(cs_pin, 0);
    spi_write_blocking(spi, &set_radio_dio_irq_cmd, 1);
    spi_write_blocking(spi, &irq_mask2, 1);
    spi_write_blocking(spi, &irq_mask1, 1);
    spi_write_blocking(spi, &dio1_mask2, 1);
    spi_write_blocking(spi, &dio1_mask1, 1);
    spi_write_blocking(spi, &dio2_mask2, 1);
    spi_write_blocking(spi, &dio2_mask1, 1);
    spi_write_blocking(spi, &dio3_mask2, 1);
    spi_write_blocking(spi, &dio3_mask1, 1);
    gpio_put(cs_pin, 1);
}

void DRF1262::clear_irq_status() {
    uint8_t irq_mask2 = 0xFF;
    uint8_t irq_mask1 = 0xFF;

#if INCLUDE_DEBUG
    if (xbee_joint_debug_msgs) {
        printf("Clearing IRQ\n");
    }
#endif

    gpio_put(cs_pin, 0);
    spi_write_blocking(spi, &set_radio_clear_irq_cmd, 1);
    spi_write_blocking(spi, &irq_mask2, 1);
    spi_write_blocking(spi, &irq_mask1, 1);
    gpio_put(cs_pin, 1);
}

short DRF1262::read_radio_buffer(uint8_t *data, short num_bytes) {
    get_rx_buffer_status();

    if (num_bytes > 255 || num_bytes < length) return -1;

    printf("Reading Radio Buffer\n");
    gpio_put(cs_pin, 0);
    spi_write_read_blocking(spi, &read_buffer_cmd, &msg, 1);

    // these next two return the status of the radio
    spi_write_read_blocking(spi, &rx_buffer_start, &msg, 1);
    spi_write_read_blocking(spi, &nop_cmd, &msg, 1);

    spi_write_read_blocking(spi, &nop_cmd, data, num_bytes);
    gpio_put(cs_pin, 1);

#if INCLUDE_DEBUG
    if (xbee_joint_debug_msgs) {
        printf("Received data:");

        for (short i = 0; i < num_bytes; i++) {
            printf(" %x", data[i]);
        }

        printf("\n");
    }
#endif

    return 0;
}

void DRF1262::get_irq_status() {
    uint8_t status2 = 0x00;
    uint8_t status1 = 0x00;

#if INCLUDE_DEBUG
    if (xbee_joint_debug_msgs) {
        printf("Getting IRQ Status\n");
    }
#endif

    gpio_put(cs_pin, 0);
    spi_write_blocking(spi, &get_irq_status_cmd, 1);
    spi_write_blocking(spi, &nop_cmd, 1);
    spi_write_read_blocking(spi, &nop_cmd, &status2, 1);
    spi_write_read_blocking(spi, &nop_cmd, &status1, 1);
    gpio_put(cs_pin, 1);

    printf("IRQ Status Register %x %x\n", status2, status1);
}

void DRF1262::get_rx_buffer_status() {
#if INCLUDE_DEBUG
    if (xbee_joint_debug_msgs) {
        printf("Getting RX Buffer Status\n");
    }
#endif

    gpio_put(cs_pin, 0);
    spi_write_blocking(spi, &get_rx_buffer_cmd, 1);
    spi_write_blocking(spi, &nop_cmd, 1);
    spi_write_read_blocking(spi, &nop_cmd, &length, 1);
    spi_write_read_blocking(spi, &nop_cmd, &rx_buffer_start, 1);
    gpio_put(cs_pin, 1);

#if INCLUDE_DEBUG
    if (xbee_joint_debug_msgs) {
        printf("Payload Length %x\n", length);
        printf("Buffer Pointer %x\n", rx_buffer_start);
    }
#endif
}

void DRF1262::set_lora_symb_timeout() {
    uint8_t symb_num = 0x0F;

    spi_write_blocking(spi, &set_lora_symb_timeout_cmd, 1);
    spi_write_blocking(spi, &symb_num, 1);
}

void DRF1262::calibrate_image() {
    uint8_t freq1 = 0xE1;
    uint8_t freq2 = 0xE9;

    spi_write_blocking(spi, &calibrate_image_cmd, 1);
    spi_write_blocking(spi, &freq1, 1);
    spi_write_blocking(spi, &freq2, 1);
}

void DRF1262::set_radio_packet_type_fsk() {
#if INCLUDE_DEBUG
    if (xbee_joint_debug_msgs) {
        printf("Setting Packet Type to FSK\n");
    }
#endif

    gpio_put(cs_pin, 0);
    spi_write_blocking(spi, &set_packet_type_cmd, 1);
    spi_write_blocking(spi, &packet_type_fsk, 1);
    gpio_put(cs_pin, 1);
}