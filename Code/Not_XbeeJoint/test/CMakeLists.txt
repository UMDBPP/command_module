cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(test_project C CXX ASM)

set(CMAKE_C_STANDARD 11)

set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(test
    test.cpp
    ../../libraries/rp2040-drf1262-lib/SX1262.cpp
    ../../libraries/rp2040-console/console.c
    ../../libraries/rp2040-console/std-cmd/command.c
    ../../libraries/rp2040-config/MB85RS1MT.cpp
    ../../libraries/rp2040-config/config.cpp
)

pico_enable_stdio_usb(test 1)

pico_add_extra_outputs(test)

target_link_libraries(test pico_stdlib hardware_flash hardware_sync hardware_spi pico_rand pico_unique_id)

add_executable(test_rx_tx
    test.cpp
    ../../libraries/rp2040-drf1262-lib/SX1262.cpp
    ../../libraries/rp2040-console/console.c
    ../../libraries/rp2040-console/std-cmd/command.c
    ../../libraries/rp2040-config/MB85RS1MT.cpp
    ../../libraries/rp2040-config/config.cpp
)

pico_enable_stdio_usb(test_rx_tx 1)

pico_add_extra_outputs(test_rx_tx)

target_link_libraries(test_rx_tx pico_stdlib hardware_flash hardware_sync hardware_spi pico_rand pico_unique_id hardware_irq)

target_compile_definitions(test_rx_tx PRIVATE
    RX_TEST=1
    TX_TEST=1
)

add_executable(test_rx
    test.cpp
    ../../libraries/rp2040-drf1262-lib/SX1262.cpp
    ../../libraries/rp2040-console/console.c
    ../../libraries/rp2040-console/std-cmd/command.c
    ../../libraries/rp2040-config/MB85RS1MT.cpp
    ../../libraries/rp2040-config/config.cpp
)

pico_enable_stdio_usb(test_rx 1)

pico_add_extra_outputs(test_rx)

target_link_libraries(test_rx pico_stdlib hardware_flash hardware_sync hardware_spi pico_rand pico_unique_id hardware_irq)

target_compile_definitions(test_rx PRIVATE
    RX_TEST=1
    TX_TEST=0
)

add_executable(test_tx
    test.cpp
    ../../libraries/rp2040-drf1262-lib/SX1262.cpp
    ../../libraries/rp2040-console/console.c
    ../../libraries/rp2040-console/std-cmd/command.c
    ../../libraries/rp2040-config/MB85RS1MT.cpp
    ../../libraries/rp2040-config/config.cpp
)

pico_enable_stdio_usb(test_tx 1)

pico_add_extra_outputs(test_tx)

target_link_libraries(test_tx pico_stdlib hardware_flash hardware_sync hardware_spi pico_rand pico_unique_id hardware_irq)

target_compile_definitions(test_tx PRIVATE
    RX_TEST=0
    TX_TEST=1
)