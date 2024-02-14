cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(test_project C CXX ASM)

set(CMAKE_C_STANDARD 11)

set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(gps-test
    test.cpp
    ../../../libraries/rp2040-drf1262-lib/SX1262.cpp
)

pico_enable_stdio_usb(gps-test 1)

pico_add_extra_outputs(gps-test)

target_link_libraries(gps-test pico_stdlib hardware_flash hardware_sync hardware_spi hardware_i2c pico_unique_id pico_rand)

target_compile_definitions(test_rx_tx PRIVATE
    NMEA_STATIC=GPGLL,GPRMC,GPGGA,GPGSA,GPGSV,GPTXT,GPVTG
)