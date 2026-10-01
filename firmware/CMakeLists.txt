# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.3.0)
set(toolchainVersion 15_2_Rel1)
set(picotoolVersion 2.3.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
cmake_minimum_required(VERSION 3.13)

# the board is a Pico 2 (RP2350), see docs/hardware-design.md
set(PICO_BOARD pico2 CACHE STRING "Board type")
set(PICO_PLATFORM rp2350 CACHE STRING "Platform")

include(pico_sdk_import.cmake)

project(motorecopico C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

# FatFs is vendored rather than fetched, so a build never depends on the network. see
# third_party/fatfs/README.md for which release is expected and where it comes from
set(FATFS_DIR ${CMAKE_CURRENT_LIST_DIR}/third_party/fatfs)

if(NOT EXISTS ${FATFS_DIR}/ff.c)
    message(FATAL_ERROR
        "FatFs sources are missing. put ff.c, ff.h, ffsystem.c and ffunicode.c from the ChaN "
        "FatFs release into ${FATFS_DIR} (see its README.md). ffconf.h lives in src/ and is "
        "already configured for this board.")
endif()

add_executable(motorecopico
    src/main.c
    src/logger.c
    src/led.c
    src/mcp25625.c
    src/sd_spi.c
    src/diskio.c
    src/nmea.c
    src/record.c
    src/power.c
    src/settings.c
    src/settings_file.c
    src/wallclock.c
    ${FATFS_DIR}/ff.c
    ${FATFS_DIR}/ffunicode.c
    ${FATFS_DIR}/ffsystem.c
)

target_include_directories(motorecopico PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src
    ${FATFS_DIR}
)

target_link_libraries(motorecopico
    pico_stdlib
    hardware_spi
    hardware_uart
    hardware_adc
    hardware_gpio
    hardware_irq
)

# the debug log goes out over USB serial. stdio must stay off UART0: since v4.1 the GPS sits on
# GP0/GP1, which are also the SDK's default stdio UART pins, so enabling it would put printf into
# the GPS receiver's ear
pico_enable_stdio_usb(motorecopico 1)
pico_enable_stdio_uart(motorecopico 0)

target_compile_definitions(motorecopico PRIVATE
    # printf("%.1f") in the status line needs the floating point formatter
    PICO_PRINTF_SUPPORT_FLOAT=1

    # a USB host that enumerates the port but never reads it (a phone charger, a laptop that went
    # to sleep) would otherwise stall every debug line for half a second, and the CAN ring drains
    # in this loop. logging matters more than the log line does
    PICO_STDIO_USB_STDOUT_TIMEOUT_US=100000
)

target_compile_options(motorecopico PRIVATE -Wall -Wextra)

pico_add_extra_outputs(motorecopico)

# bench build: puts the CAN controller in loopback so the driver can be exercised without a bus
option(CAN_SELF_TEST "build with the MCP25625 loopback self test" OFF)

if(CAN_SELF_TEST)
    target_compile_definitions(motorecopico PRIVATE CAN_SELF_TEST=1)
endif()

# the same tests test/build.ps1 runs on a PC, built to run on the target instead. useful because
# the toolchain for this board does not imply a host compiler, and because only the target build
# proves struct CANData really is 16 bytes on arm. runs on any Pico 2, no MotoRecoPico board needed
add_executable(motorecopico_tests
    test/test_pico_main.c
    test/test_nmea.c
    src/nmea.c
    src/record.c
    src/power.c
    src/settings.c
    src/wallclock.c
)

target_include_directories(motorecopico_tests PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src)
target_compile_definitions(motorecopico_tests PRIVATE TEST_ON_PICO=1 PICO_PRINTF_SUPPORT_FLOAT=1)
target_compile_options(motorecopico_tests PRIVATE -Wall -Wextra)
target_link_libraries(motorecopico_tests pico_stdlib)

pico_enable_stdio_usb(motorecopico_tests 1)
pico_enable_stdio_uart(motorecopico_tests 0)
pico_add_extra_outputs(motorecopico_tests)
