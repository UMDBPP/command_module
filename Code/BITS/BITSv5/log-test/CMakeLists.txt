cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(test_project C CXX ASM)

set(CMAKE_C_STANDARD 11)

set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(log-test
    log-test.cpp
    ../BITSv5.cpp
    ../../../libraries/rp2040-config/MB85RS1MT.cpp
    ../../../libraries/rp2040-config/config.cpp
)

pico_enable_stdio_usb(log-test 1)

pico_add_extra_outputs(log-test)

target_link_libraries(log-test pico_stdlib hardware_flash hardware_sync hardware_spi hardware_i2c pico_unique_id pico_rand hardware_irq hardware_watchdog)