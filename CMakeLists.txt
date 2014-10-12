cmake_minimum_required(VERSION 2.8.5)
project(genkfs C)
set(CMAKE_CXX_FLAGS "-g")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "bin/")
add_definitions("-Wall")

add_executable(genkfs
    main.c
)

if(WIN32 OR CYGWIN)
  install(PROGRAMS bin/genkfs.exe
      DESTINATION bin)
else()
  install(PROGRAMS bin/genkfs
      DESTINATION bin)
endif()

set(CMAKE_BUILD_TYPE Release)

ADD_CUSTOM_TARGET(man ALL)

ADD_CUSTOM_COMMAND(
  TARGET man
  SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/genkfs.1.txt
  COMMAND a2x --no-xmllint --doctype manpage --format manpage ${CMAKE_CURRENT_SOURCE_DIR}/genkfs.1.txt 
  OUTPUTS ${CMAKE_CURRENT_BINARY_DIR}/genkfs.1
)

ADD_CUSTOM_COMMAND(
  TARGET man
  SOURCE man
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/genkfs.1
)

INSTALL(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/genkfs.1
    DESTINATION ${CMAKE_INSTALL_PREFIX}/man/man1
)
