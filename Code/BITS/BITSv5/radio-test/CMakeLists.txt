cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
project(test_project C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()
add_executable(radio-test
    radio_test.cpp
    ../../../libraries/rp2040-drf1262-lib/SX1262.cpp
		../BITS_common/BITSv5.cpp
		../BITS_common/BITSv5_Radio.cpp
		../BITS_common/BITSv5_GPS.cpp

)
pico_enable_stdio_usb(radio-test 1)
pico_enable_stdio_uart(radio-test 1)
pico_add_extra_outputs(radio-test) 
target_link_libraries(radio-test pico_stdlib hardware_spi hardware_gpio hardware_sync pico_unique_id pico_rand hardware_irq hardware_flash hardware_i2c)
