cmake_minimum_required(VERSION 2.8.5)

project(genkfs C)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic -Werror")                                          

if (NOT DEFINED CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RELEASE)
endif()

if(EMSCRIPTEN)
  set(CMAKE_EXE_LINKER_FLAGS "--pre-js ${CMAKE_CURRENT_SOURCE_DIR}/module.js -s EXPORTED_FUNCTIONS=\"['_main', '_emscripten_main']\"")
  set(CMAKE_C_FLAGS "-O3")
endif()

add_executable(genkfs main.c)

install(TARGETS genkfs DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

ADD_CUSTOM_TARGET(man ALL)
ADD_CUSTOM_COMMAND(
  TARGET man
  COMMAND a2x --no-xmllint --doctype manpage --format manpage ${CMAKE_CURRENT_SOURCE_DIR}/genkfs.1.txt -D ${CMAKE_CURRENT_BINARY_DIR}
  OUTPUTS ${CMAKE_CURRENT_BINARY_DIR}/genkfs.1
)

if (NOT DEFINED CMAKE_INSTALL_MANDIR)
    set(CMAKE_INSTALL_MANDIR ${CMAKE_INSTALL_PREFIX}/share/man)
endif()
INSTALL(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/genkfs.1
    DESTINATION ${CMAKE_INSTALL_MANDIR}/man1
)
