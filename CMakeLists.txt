# Generated Cmake Pico project file
cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 1.5.1)
set(toolchainVersion 13_2_Rel1)
set(picotoolVersion 2.0.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()

set(PICO_BOARD pico_w CACHE STRING "Board type")

include(pico_sdk_import.cmake)

project(Decibelimetro_Pico C CXX ASM)

pico_sdk_init()

add_subdirectory(libs/pico-lfs)

add_executable(Decibelimetro_Pico 
    main.c 
    libs/ssd1306.c 
    src/mic.c 
    src/display.c 
    src/wifi.c 
    src/mqtt.c 
    src/timertc.c
    src/flash.c
)

target_compile_definitions(Decibelimetro_Pico PRIVATE
    LFS_THREADSAFE=1
    LFS_NO_DEBUG=1
)

target_include_directories(Decibelimetro_Pico PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/libs
)

target_link_libraries(Decibelimetro_Pico PRIVATE
    pico_stdlib
    hardware_flash
    pico-lfs
    hardware_uart
    hardware_i2c
    hardware_rtc
    pico_cyw43_arch_lwip_threadsafe_background
    pico_lwip_mqtt
    pico_lwip_sntp
    pico_multicore
    pico_sync
)

pico_enable_stdio_uart(Decibelimetro_Pico 0)
pico_enable_stdio_usb(Decibelimetro_Pico 1)

pico_add_extra_outputs(Decibelimetro_Pico)