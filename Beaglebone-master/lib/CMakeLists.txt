cmake_minimum_required( VERSION 3.5.2 )
project (cc1200)

set (ARM_OPTIONS "-mtune=cortex-a8 -march=armv7-a")
set (LIBDIR_APP_LOADER "/usr/local/lib")
set (INCDIR_APP_LOADER "/usr/local/include")

set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -I${INCDIR_APP_LOADER} -std=c99 -D_XOPEN_SOURCE=500 -O2 ${ARM_OPTIONS}")
set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")

file(GLOB SOURCES "src/*.c")
add_library(cc1200 SHARED ${SOURCES})
include_directories (cc1200 include)

install(TARGETS cc1200 DESTINATION "/usr/local/lib")
install(FILES include/cc1200.h DESTINATION "/usr/local/include")
