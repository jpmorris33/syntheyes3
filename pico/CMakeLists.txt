cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(myproject_test C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(GIF_LIB_SRC giflib/giflib-5.2.1)

add_compile_definitions(PLATFORM_PICO)
#include_directories(../pico)
include_directories(../pico)
include_directories(${GIF_LIB_SRC})

pico_sdk_init()

add_executable(syntheyes3
	../syntheyes.cpp ../config.cpp ../gpio.cpp ../font.cpp ../gifload.cpp ../expression.cpp ../expressionlist.cpp
	../colourutils.cpp
	../platforms/pico.cpp ../platforms/Platform.cpp ../platforms/PicoPlatform.cpp
	../platforms/MemFS.cpp ../platforms/TarFS.cpp
	../drivers/driverutils.cpp ../drivers/PanelDriver.cpp
	../drivers/display/Unicorn.cpp ../drivers/display/MAX7219Panel.cpp ../drivers/display/WS2811PicoPanel.cpp
	../drivers/SerialDriver.cpp ../drivers/serial/VirtualSerialDriver.cpp
	../drivers/LightDriver.cpp  ../drivers/ServoDriver.cpp
	../drivers/Timing.cpp ../drivers/PicoTiming.cpp

	${GIF_LIB_SRC}/dgif_lib.c ${GIF_LIB_SRC}/gifalloc.c ${GIF_LIB_SRC}/gif_err.c ${GIF_LIB_SRC}/gif_font.c ${GIF_LIB_SRC}/gif_hash.c ${GIF_LIB_SRC}/openbsd-reallocarray.c

	)

# generate the header file into the source tree as it is included in the RP2040 datasheet
pico_generate_pio_header(syntheyes3 ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/generated)

target_link_libraries(syntheyes3 PRIVATE pico_stdlib hardware_pio hardware_spi)
pico_add_extra_outputs(syntheyes3)
