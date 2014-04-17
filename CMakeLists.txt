cmake_minimum_required(VERSION 2.8.5)
project(genkfs)
set(CMAKE_CXX_FLAGS "-g")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "bin/")
add_definitions("-Wall")

add_executable(genkfs
    main.c
)

install(PROGRAMS bin/genkfs
    DESTINATION bin)

set(CMAKE_BUILD_TYPE Release)
