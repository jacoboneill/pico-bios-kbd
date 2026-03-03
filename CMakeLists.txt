cmake_minimum_required(VERSION 3.13)

set(PICO_BOARD pico_w)
set(PICO_SDK_FETCH_FROM_GIT on)
set(PICO_SDK_FETCH_FROM_GIT_TAG 2.1.1)

include(pico_sdk_import.cmake)

project(pico_keyboard C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(pico_keyboard
    main.cpp
    usb_descriptors.c
)

target_include_directories(pico_keyboard PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(pico_keyboard
    pico_stdlib
    pico_cyw43_arch_none
    tinyusb_device
    tinyusb_board
)

pico_enable_stdio_usb(pico_keyboard 1)
pico_enable_stdio_uart(pico_keyboard 0)

pico_add_extra_outputs(pico_keyboard)
