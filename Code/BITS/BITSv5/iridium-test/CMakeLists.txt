cmake_minimum_required(VERSION 3.13...3.27)

# initialize pico-sdk from submodule
# note: this must happen before project()
include(../pico-sdk/pico_sdk_init.cmake)

project(test_project C CXX ASM)

set(CMAKE_C_STANDARD 11)

set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(test
    test.cpp
    ../BITS_common/BITSv5.cpp
    ../BITS_common/BITSv5_Radio.cpp
		../bits_common/BITSv5_GPS.cpp
		../bits_common/iridium-lib.cpp

)

pico_enable_stdio_usb(test 1)

pico_add_extra_outputs(test)

target_link_libraries(test pico_stdlib hardware_flash hardware_sync hardware_spi hardware_i2c pico_unique_id pico_rand hardware_irq hardware_watchdog hardware_gpio)
