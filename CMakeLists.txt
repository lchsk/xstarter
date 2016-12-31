# This file described build process for xstarter
# Requires Curses and Glib

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

project(xstarter)

set(PROJECT_VERSION "0.4.0")
set(CMAKE_CXX_FLAGS "-g -Wall -pedantic")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

find_package(PkgConfig)

set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

add_definitions(-std=gnu99)

find_package(Curses REQUIRED)
find_package(Glib REQUIRED)

message (${CURSES_INCLUDE_DIR})

include_directories(
  ${Glib_INCLUDE_DIRS}
  ${CURSES_INCLUDE_DIR}
)

add_executable(xstarter
  src/main.c
  src/scan.c
  src/settings.c
  src/term.c
  src/utils.c
  src/utils_string.c
)

target_link_libraries(xstarter
  ${Glib_LIBRARIES}
  # /lib/libncursesw.so.6
  # /usr/lib/libformw.so.6
  # /usr/lib/libmenuw.so.6
  #/lib/libformw.so
  #${CURSES_LIBRARIES}
  -lncurses
  -pthread
)

install (TARGETS xstarter DESTINATION bin)

# uninstall target
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY
)

add_custom_target(uninstall
  COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
)

add_custom_target(check ./tests/xstarter_tests ${CMAKE_CTEST_COMMAND})
