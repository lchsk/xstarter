CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

project(xstarter)

set(CMAKE_CXX_FLAGS "-g -Wall -pedantic")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

find_package(PkgConfig)

set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

add_definitions(-std=gnu99)

find_package(Curses REQUIRED)
find_package(Glib REQUIRED)

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
    ${CURSES_LIBRARIES}
    -lmenu
)

install (TARGETS xstarter DESTINATION bin)
