cmake_minimum_required(VERSION 2.8.5)
project(genkfs)
set(CMAKE_CXX_FLAGS "-g")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "bin/")
add_definitions("-Wall")

add_executable(genkfs
    kfs/directory.c
    kfs/file.c
    kfs/filesystem.c
    main.c
)

include_directories(
    kfs/
)

set(CMAKE_BUILD_TYPE Release)
