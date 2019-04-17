cmake_minimum_required( VERSION 3.5.2 )
project (spi)

set (ARM_OPTIONS "-mtune=cortex-a8 -march=armv7-a")
set (LIBDIR_APP_LOADER "/usr/local/lib")
set (INCDIR_APP_LOADER "/usr/local/include")
set (PRUSS_LIBS "-Wl,-rpath=$(LIBDIR_APP_LOADER) -L$(LIBDIR_APP_LOADER) -lprussdrv")

set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -I${INCDIR_APP_LOADER} -std=c99 -D_XOPEN_SOURCE=500 -O2 ${ARM_OPTIONS}")
set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${PRUSS_LIBS}")

add_custom_command(
	POST_BUILD
	OUTPUT SPIv1_bin.h
	COMMAND pasm -V3 -c SPIv1.p
	DEPENDS SPIv1.p SPI.hp
	COMMENT "Generating SPIv1_bin.h"
)
add_custom_target(pru ALL DEPENDS SPIv1_bin.h)
file(GLOB SOURCES "*.c")
add_library(spi SHARED ${SOURCES})
add_dependencies(spi pru)
#add_library(spi SHARED ${SOURCES})
install(TARGETS spi DESTINATION "/usr/local/lib")
install(FILES SPIv1.h DESTINATION "/usr/local/include")
