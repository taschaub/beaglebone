cmake_minimum_required(VERSION 2.8.9)
project (CC1200)

set (CMAKE_EXE_LINKER_FLAGS "-L/usr/local/lib")
set (CC1200_LIBS "-lprussdrv -lspi -lcc1200")
set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${CC1200_LIBS}")

add_compile_options(-std=gnu11)

include_directories(include)
file(GLOB SOURCES "src/*.c")
add_executable(cc1200d ${SOURCES})
