# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)
set(PICO_SDK_PATH "/Users/shuichi/pico/pico-sdk")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(saturn_pad C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(saturn_pad 
        main.cpp
        pad_state.cpp
        hid_app.cpp
        )

pico_set_program_name(saturn_pad "saturn_pad")
pico_set_program_version(saturn_pad "0.1")

pico_enable_stdio_uart(saturn_pad 1)
pico_enable_stdio_usb(saturn_pad 0)

# tinyusb
set(FAMILY rp2040)
set(BOARD pico_sdk)
set(TINYUSB_DEBUG_LEVE 0)
include(${PICO_TINYUSB_PATH}/hw/bsp/${FAMILY}/family.cmake)
family_configure_target(saturn_pad)
# add_compile_definitions(CFG_TUSB_DEBUG=0)

# Add the standard library to the build
target_link_libraries(saturn_pad
PRIVATE
        pico_stdlib
        
        tinyusb_host
        )

target_include_directories(saturn_pad
PRIVATE
    .
)

# Add any user requested libraries
target_link_libraries(saturn_pad
PRIVATE
        hardware_watchdog
        )

pico_add_extra_outputs(saturn_pad)

