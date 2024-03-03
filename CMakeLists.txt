cmake_minimum_required(VERSION 2.8.12)
project(AplikasiKasir)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

file(GLOB_RECURSE SOURCES "src/*.c" "src/**/*.c")
file(GLOB_RECURSE VENDOR_SOURCES "vendor/*.c" "vendor/**/*.c")

set(ALL_SOURCES ${SOURCES} ${VENDOR_SOURCES})

add_executable(AplikasiKasir ${ALL_SOURCES})

if(WIN32)
    target_link_libraries(AplikasiKasir PRIVATE ws2_32 winmm pthread)
elseif(UNIX)
    target_link_libraries(AplikasiKasir PRIVATE dl pthread)
endif()