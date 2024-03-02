cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(test_project C CXX ASM)

set(CMAKE_C_STANDARD 11)

set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(test
    test.cpp
    ../../../libraries/rp2040-drf1262-lib/SX1262.cpp
    ../../../libraries/rp2040-ms5607-lib/MS5607.cpp
    ../../../libraries/rp2040-config/MB85RS1MT.cpp
    ../../../libraries/rp2040-config/config.cpp
)

pico_enable_stdio_usb(test 1)

pico_add_extra_outputs(test)

target_link_libraries(test pico_stdlib hardware_flash hardware_sync hardware_spi hardware_i2c pico_unique_id pico_rand hardware_irq)