CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

project(xstarter)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
endif()

set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")

set(PROJECT_VERSION "0.8.1")
if (CMAKE_BUILD_TYPE STREQUAL Debug)
    set(CMAKE_C_FLAGS "-g -Wall -pedantic")
else()
    set(CMAKE_C_FLAGS "-Wall -pedantic -O3")
endif()

# cpack
set(CPACK_PACKAGE_CONTACT "Maciej Lechowski")
set(CPACK_GENERATOR "DEB;RPM;ZIP;TGZ")
set(CPACK_PACKAGE_DESCRIPTION, "Application launcher for Linux")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})

# deb stuff
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://lchsk.com/xstarter.html")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libncurses5 (>= 5.0)")

# rpm stuff
set(CPACK_RPM_PACKAGE_LICENSE "GPL")
set(CPACK_RPM_PACKAGE_URL "https://lchsk.com/xstarter.html")
set(CPACK_RPM_PACKAGE_DESCRIPTION "Application launcher for Linux")
set(CPACK_RPM_PACKAGE_REQUIRES "ncurses >= 5.0")

include(CPack)

# end cpack

add_definitions(-std=c99)

find_package(PkgConfig REQUIRED)

message(STATUS "Cmake build type: " ${CMAKE_BUILD_TYPE})
message(STATUS "Curses include directory: " ${CURSES_INCLUDE_DIR})

pkg_check_modules(CURSES REQUIRED ncurses)

add_library(lib-inih STATIC
    external/inih/ini.c
    external/inih/ini.h
)
set_target_properties(lib-inih PROPERTIES LINKER_LANGUAGE C)

include_directories(
    ${CURSES_INCLUDE_DIRS}
    external/
)

add_executable(xstarter
    src/main.c
    src/scan.c
    src/settings.c
    src/term.c
    src/utils.c
    src/utils_string.c
    src/list.c
    xstarter.1
)

target_link_libraries(xstarter
    ${CURSES_LIBRARIES}
    -pthread
    -linih
)

install(TARGETS xstarter DESTINATION bin)
install(FILES xstarter.1 DESTINATION share/man/man1)

add_executable(xstarter_tests
    src/utils.c
    src/utils_string.c
    src/settings.c

    tests/test.c
)

target_link_libraries(xstarter_tests
    -linih
)